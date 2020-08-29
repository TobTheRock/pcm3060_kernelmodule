#include "buffer.h"
#include <utils/logging.h>
#include <utils/ptr.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/err.h>


typedef struct _buffer_impl
{
    void* buf;
    const unsigned int bufsize;
    atomic_t write_off, n_bytes_written, n_bytes_written_tmp, n_active_writters;
    struct mutex mlock;
} _buffer_impl_t;

typedef struct _wr_offset_range
{
    unsigned int wr_off_start, n_bytes_writable, n_bytes_dropped;
} _wr_offset_range_t;

static inline  _buffer_impl_t* _get_buffer_impl(const unsigned int size)
{
    _buffer_impl_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(_buffer_impl_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((newbuf->buf = kmalloc(size, GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_buf;
    }
    else
    {
        TRACE("Allocated new byte buffer %p", newbuf->buf);
    }

    mutex_init(&newbuf->mlock);
    *(unsigned int*)&newbuf->bufsize = size;
    atomic_set(&newbuf->write_off, 0);
    atomic_set(&newbuf->n_active_writters, 0);
    atomic_set(&newbuf->n_bytes_written, 0);
    atomic_set(&newbuf->n_bytes_written_tmp, 0);

    return newbuf;

    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;
}

static void _sync_buffer_impl (_buffer_impl_t* bufimpl)
{
    //so no new writters can't join.
    atomic_set(&bufimpl->write_off, bufimpl->bufsize);

    //wait for writers to finish
    if (atomic_read(&bufimpl->n_active_writters))
    {
        mutex_lock(&bufimpl->mlock);
    }
}

static inline void _put_buffer_impl (_buffer_impl_t* bufimpl)
{
    TRACE("");
    RETURN_VOID_ON_NULL(bufimpl);
    _sync_buffer_impl(bufimpl);

    TRACE("Freeing byte buffer %p", bufimpl->buf);
    kfree(bufimpl->buf);
    TRACE("Freeing buffer implementation %p", bufimpl);
    kfree(bufimpl);
    bufimpl = NULL;
}

static int _write_check_full(struct buffer* this_buffer)
{
    int ret = 0;
    if (atomic_read(&this_buffer->_impl_p->write_off) >= this_buffer->_impl_p->bufsize)
    {
        ERROR("Buffer is already full!");
        ret = -1;
    }
    return ret;
}

static _wr_offset_range_t _write_request_off_range_sync(struct buffer* this_buffer, unsigned int buflen)
{
    _wr_offset_range_t range_ret = {0,0,0};
    unsigned int wr_off_end;
    //Starting synchronized write block
    if (atomic_inc_return(&this_buffer->_impl_p->n_active_writters) == 1) //first thread
    {
        TRACE("Locking");
        mutex_lock(&this_buffer->_impl_p->mlock);
    }
    //increase offset first to dissallow other reads getting the memory range
    wr_off_end = atomic_add_return(buflen, &this_buffer->_impl_p->write_off);
    range_ret.wr_off_start = wr_off_end - buflen;
    //in case offset is greater than the buffersize not all bytes can be copied!
    if (range_ret.wr_off_start >= this_buffer->_impl_p->bufsize)
    {
        ERROR("Buffer is already full!");
        range_ret.n_bytes_writable = 0;
        range_ret.n_bytes_dropped = buflen;
    }
    else
    {
        range_ret.n_bytes_dropped = (this_buffer->_impl_p->bufsize <= wr_off_end) ? min( (wr_off_end - this_buffer->_impl_p->bufsize+1), buflen) : 0;
        range_ret.n_bytes_writable = buflen - range_ret.n_bytes_dropped;
        
        // will be added to n_bytes_written on _write_finish_sync
        atomic_add(range_ret.n_bytes_writable, &this_buffer->_impl_p->n_bytes_written_tmp);
    }
    
    return range_ret;
}

static void _write_finish_sync(struct buffer* this_buffer)
{
    //Ending synchronized write block
    if (atomic_dec_and_test(&this_buffer->_impl_p->n_active_writters))
    {
        TRACE("Unlocking");
        mutex_unlock(&this_buffer->_impl_p->mlock);
        TRACE("Increasing n_bytes_written %d", atomic_read(&this_buffer->_impl_p->n_bytes_written_tmp));
        atomic_add(atomic_read(&this_buffer->_impl_p->n_bytes_written_tmp), &this_buffer->_impl_p->n_bytes_written);
        atomic_set(&this_buffer->_impl_p->n_bytes_written_tmp, 0);
    }
    else
    {
        TRACE("Waiting for other threads...");
        mutex_lock(&this_buffer->_impl_p->mlock);
    }
}

unsigned int buffer_write_copy (struct buffer* this_buffer, const void* buffer_ext, const unsigned int buflen)
{
    unsigned int n_bytes_dropped = 0;
    TRACE("");
    RETURN_ON_NULL(this_buffer, buflen);
    RETURN_ON_NULL(buffer_ext, buflen);
    if (buflen == 0)
    {
        DEBUG("Buflen is 0, nothing todo");
    }
    else if (_write_check_full(this_buffer))
    {
        n_bytes_dropped =  buflen;
    }
    else
    {
        _wr_offset_range_t range = _write_request_off_range_sync(this_buffer, buflen);
        n_bytes_dropped = range.n_bytes_dropped;
        if (range.n_bytes_writable > 0 )
        {
            memcpy( this_buffer->_impl_p->buf + range.wr_off_start, buffer_ext, range.n_bytes_writable);
        }
        _write_finish_sync(this_buffer);
    }

    return n_bytes_dropped;
}

unsigned int buffer_copy_from_user (struct buffer* this_buffer, const void* buffer_ext, const unsigned int buflen)
{
    unsigned int n_bytes_dropped = 0;
    TRACE("");
    RETURN_ON_NULL(this_buffer, buflen);
    RETURN_ON_NULL(buffer_ext, buflen);
    if (buflen == 0)
    {
        DEBUG("Buflen is 0, nothing todo");
    }
    else if (_write_check_full(this_buffer))
    {
        n_bytes_dropped =  buflen;
    }
    else
    {
        _wr_offset_range_t range = _write_request_off_range_sync(this_buffer, buflen);
        n_bytes_dropped = range.n_bytes_dropped;
        if (range.n_bytes_writable > 0 )
        {
            n_bytes_dropped += copy_from_user(this_buffer->_impl_p->buf + range.wr_off_start, buffer_ext, range.n_bytes_writable);
        }
        _write_finish_sync(this_buffer);
    }

    return n_bytes_dropped;
}

static inline unsigned int  _copy_impl (const struct buffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off, bool fromUser)
{
    unsigned int n_bytes_dropped = 0, n_bytes_available, n_bytes_to_copy;
    TRACE("");
    
    RETURN_ON_NULL(this_buffer, buflen);
    RETURN_ON_NULL(buffer_ext, buflen);
    if (buflen == 0)
    {
        DEBUG("Buflen is 0, nothing todo");
    }
    else if (off >= (n_bytes_available = atomic_read(&this_buffer->_impl_p->n_bytes_written)))
    {
        WARN("Offset %d exceeds nof buffered values %d", off, n_bytes_available);
        n_bytes_dropped = buflen;
    }
    else if ( (n_bytes_to_copy = min(n_bytes_available - off, buflen) ) > 0 )
    {
        n_bytes_dropped = (buflen - n_bytes_to_copy);
        if (fromUser)
        {
            n_bytes_dropped += copy_to_user(buffer_ext, this_buffer->_impl_p->buf + off, n_bytes_to_copy);
        }
        else
        {
            memcpy(buffer_ext, this_buffer->_impl_p->buf + off, n_bytes_to_copy);
        }
    }
    else
    {
        ERROR("Could not read any bytes starting from %d!", off);
        n_bytes_dropped = buflen;
    }
    
    return n_bytes_dropped;
}

unsigned int buffer_read_copy (const struct buffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off)
{
    TRACE("");
    return _copy_impl(this_buffer, buffer_ext, buflen, off, 0);
}
unsigned int buffer_copy_to_user (const struct buffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off)
{
    TRACE("");
    return _copy_impl(this_buffer, buffer_ext, buflen, off, 1);
}

unsigned int buffer_read (const struct buffer* this_buffer, void** out_buffer_p, const unsigned int off)
{
    unsigned int n_bytes_available = 0;
    TRACE("");
    RETURN_ON_NULL(this_buffer, 0);
    RETURN_ON_NULL(out_buffer_p, 0);

    if (off > (n_bytes_available = atomic_read(&this_buffer->_impl_p->n_bytes_written)))
    {
        ERROR("Offset %d exceeds nof buffered values %d", off, n_bytes_available);
    }
    else
    {
        TRACE("avaiable bytes %d", n_bytes_available);
        n_bytes_available -= off;
        TRACE("avaiable bytes after offset %d", n_bytes_available);
        *out_buffer_p = this_buffer->_impl_p->buf + off;
        TRACE("Internal buffer %p, buf out %p", this_buffer->_impl_p->buf, *out_buffer_p);
    }
    
    return n_bytes_available;
}

unsigned int buffer_write_reserve (struct buffer* this_buffer, void** out_buffer_p, const unsigned int n_bytes_requested)
{
    unsigned int n_bytes_to_write = 0;
    TRACE("");
    RETURN_ON_NULL(this_buffer, 0);
    if (n_bytes_requested == 0)
    {
        DEBUG("n_bytes_requested is 0, nothing todo");
    }
    else if (_write_check_full(this_buffer))
    {
        WARNING("Buffer is already full");
    }
    else
    {
        _wr_offset_range_t range = _write_request_off_range_sync(this_buffer, n_bytes_requested);
        if(range.n_bytes_writable)
        {
            *out_buffer_p = this_buffer->_impl_p->buf + range.wr_off_start;
        }
        n_bytes_to_write = range.n_bytes_writable;
    }
    
    return n_bytes_to_write;
}

void buffer_write_finish (struct buffer* this_buffer)
{
    TRACE("");
    RETURN_VOID_ON_NULL(this_buffer);

    if (atomic_read(&this_buffer->_impl_p->n_active_writters) < 1)
    {
        ERROR("No writters, did you call buffer_write_reserve?");
    }
    else
    {
        _write_finish_sync(this_buffer);
    }
}



unsigned int buffer_get_n_bytes_readable (const struct buffer* this_buffer)
{
    RETURN_ON_NULL(this_buffer, 0);
    return atomic_read(&this_buffer->_impl_p->n_bytes_written);
}

void buffer_reset (struct buffer* this_buffer)
{
    TRACE("");
    RETURN_VOID_ON_NULL(this_buffer);

    _sync_buffer_impl(this_buffer->_impl_p);

    atomic_set(&this_buffer->_impl_p->write_off, 0);
    atomic_set(&this_buffer->_impl_p->n_bytes_written, 0);
    atomic_set(&this_buffer->_impl_p->n_bytes_written_tmp, 0);

    return;
}

void buffer_sync (struct buffer* this_buffer)
{
    TRACE("");
    RETURN_VOID_ON_NULL(this_buffer);

    //wait for writers to finish
    if (atomic_read(&this_buffer->_impl_p->n_active_writters))
    {
        TRACE("Waiting...");
        mutex_lock(&this_buffer->_impl_p->mlock);
    }
}

buffer_t* get_buffer(const unsigned int size)
{
    buffer_t* newbuf;

    TRACE("");
    if ((newbuf = kmalloc(sizeof(buffer_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((CONST_CAST(_buffer_impl_t*)newbuf->_impl_p = _get_buffer_impl(size)) == NULL)
    {
        goto r_buf;
    }
    else
    {
        DEBUG("Allocated new buffer");
        CONST_CAST(unsigned int) newbuf->size = size;
    }

    return newbuf;

    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;

}

void put_buffer(buffer_t* buf)
{
    TRACE("");
    RETURN_VOID_ON_NULL(buf);
    //TODO MAKE THREAD SAFE SYNC
    _put_buffer_impl(buf->_impl_p);
    kfree(buf);
    buf = NULL;
}
