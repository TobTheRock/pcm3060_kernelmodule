#include "buffer.h"
#include <utils/logging.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/err.h>


typedef struct _buffer_impl
{
    void* buf;
    const unsigned int bufsize;
    atomic_t write_off, n_bytes_written, n_bytes_written_tmp, n_active_writters;
    struct spinlock slock;
} _buffer_impl_t;


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
        DEBUG("Allocated new buffer");
    }

    spin_lock_init(&newbuf->slock);
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

static inline void _put_buffer_impl (_buffer_impl_t* bufimpl)
{
    TRACE("");
    if (!bufimpl)
    {
        ERROR("Invalid buffer pointer!");
        return;
    }

    TRACE("Freeing...");
    kfree(bufimpl->buf);
    kfree(bufimpl);
    bufimpl = NULL;
}

static inline unsigned int _write_impl (struct buffer* this_buffer, const void* buffer_ext, const unsigned int buflen, const bool fromUser)
{
    unsigned int n_bytes_dropped = 0, this_bufsize;
    TRACE("");
    if ((buffer_ext == NULL) || (this_buffer == NULL))
    {
        ERROR("Invalid (ring)buffer");
        n_bytes_dropped = buflen;
    }
    else if (buflen == 0)
    {
        DEBUG("Buflen is 0, nothing todo");
    }
    else if (atomic_read(&this_buffer->_impl_p->write_off) >= (this_bufsize = this_buffer->_impl_p->bufsize))
    {
        ERROR("Buffer is already full!");
        n_bytes_dropped = buflen;
    }
    // else if (buflen > this_buffer->_impl_p->bufsize)
    // {
    //     ERROR("Ringbuffer with size %d is to small for write with %d bytes", ringbufsize, buflen);
    //     return buflen;
    // }
    else
    {
        unsigned int write_off_new, write_off_old, n_bytes_to_write = 0;
        //first thread
        if (atomic_inc_return(&this_buffer->_impl_p->n_active_writters) == 1)
        {
            spin_lock(&this_buffer->_impl_p->slock);
        }
        // Synchronized block
        //increase offset first to dissallow other reads getting the memory range
        write_off_new = atomic_add_return(buflen, &this_buffer->_impl_p->write_off);
        write_off_old = write_off_new - buflen;
        //in case offset is greater than the buffersize not all bytes can be copied!
        if (write_off_old >= this_bufsize)
        {
            ERROR("Buffer is already full!");
            n_bytes_dropped = buflen;
        }
        else
        {
            n_bytes_dropped = (this_bufsize <= write_off_new) ? min( (write_off_new - this_bufsize+1), buflen) : 0;
            n_bytes_to_write = buflen - n_bytes_dropped;
            if (fromUser)
            {
                n_bytes_dropped += copy_from_user(this_buffer->_impl_p->buf + write_off_old, buffer_ext, n_bytes_to_write);
            }
            else
            {
                memcpy( this_buffer->_impl_p->buf + write_off_old, buffer_ext, n_bytes_to_write);
            }
        }
        // finish wait for other reads and increase n_bytes_written(TODO)
        atomic_add(n_bytes_to_write, &this_buffer->_impl_p->n_bytes_written_tmp);
        if (atomic_dec_and_test(&this_buffer->_impl_p->n_active_writters))
        {
            spin_unlock(&this_buffer->_impl_p->slock);
        }
        else
        {
            spin_lock(&this_buffer->_impl_p->slock);
        }
        atomic_add(atomic_read(&this_buffer->_impl_p->n_bytes_written_tmp), &this_buffer->_impl_p->n_bytes_written);
        atomic_set(&this_buffer->_impl_p->n_bytes_written_tmp, 0);
    }

    return n_bytes_dropped;
}

static unsigned int _write (struct buffer* this_buffer, const void* buffer_ext, const unsigned int buflen)
{
    TRACE("");
    return _write_impl(this_buffer, buffer_ext, buflen, 0);
}

static unsigned int _write_from_user (struct buffer* this_buffer, const void* buffer_ext, const unsigned int buflen)
{
    TRACE("");
    return _write_impl(this_buffer, buffer_ext, buflen, 1);
}

static inline unsigned int  _copy_impl (struct buffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off, bool fromUser)
{
    unsigned int n_bytes_dropped = 0, n_bytes_available, n_bytes_to_copy;
    TRACE("");

    if ((this_buffer == NULL) || (buffer_ext == NULL))
    {
        ERROR("Invalid (external) buffer");
        n_bytes_dropped = buflen;
    }
    else if (buflen == 0)
    {
        DEBUG("Buflen is 0, nothing todo");
    }
    else if (off >= (n_bytes_available = atomic_read(&this_buffer->_impl_p->n_bytes_written)))
    {
        ERROR("Offset %d exceeds nof buffered values %d", off, n_bytes_available);
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

static unsigned int _copy (struct buffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off)
{
    TRACE("");
    return _copy_impl(this_buffer, buffer_ext, buflen, off, 0);
}
static unsigned int _copy_to_user (struct buffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off)
{
    TRACE("");
    return _copy_impl(this_buffer, buffer_ext, buflen, off, 1);
}

static unsigned int _read (struct buffer* this_buffer, void* out_buffer_p, const unsigned int off)
{
    unsigned int n_bytes_available = 0;
    if ((this_buffer == NULL))
    {
        ERROR("Invalid buffer");
        return 0;
    }
    else if (out_buffer_p != NULL)
    {
        WARNING("Overwriting existing buffer pointer %p", out_buffer_p);
    }

    if (off >= (n_bytes_available = atomic_read(&this_buffer->_impl_p->n_bytes_written)))
    {
        ERROR("Offset %d exceeds nof buffered values %d", off, n_bytes_available);
    }
    else
    {
        n_bytes_available -= off;
    }
    
    out_buffer_p = this_buffer->_impl_p->buf + off;
    
    return n_bytes_available;
}

static unsigned int _get_n_bytes_readable (struct buffer* this_buffer)
{
    if ((this_buffer == NULL))
    {
        ERROR("Invalid buffer");
        return 0;
    }
    return atomic_read(&this_buffer->_impl_p->n_bytes_written);
}

static void _reset (struct buffer* this_buffer)
{
    TRACE("");

    if (this_buffer == NULL)
    {
        ERROR("Invalid buffer");
        return;
    }
    //so no new writters can't join.
    atomic_set(&this_buffer->_impl_p->write_off, this_buffer->_impl_p->bufsize);

    //wait for writers to finish
    if (atomic_read(&this_buffer->_impl_p->n_active_writters))
    {
            spin_lock(&this_buffer->_impl_p->slock);
    }

    atomic_set(&this_buffer->_impl_p->write_off, 0);
    atomic_set(&this_buffer->_impl_p->n_bytes_written, 0);
    atomic_set(&this_buffer->_impl_p->n_bytes_written_tmp, 0);

    return;
}

static void _sync (struct buffer* this_buffer)
{
    TRACE("");
    if (this_buffer == NULL)
    {
        ERROR("Invalid buffer");
        return;
    }

    //wait for writers to finish
    if (atomic_read(&this_buffer->_impl_p->n_active_writters))
    {
        TRACE("Waiting...");
        spin_lock(&this_buffer->_impl_p->slock);
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
    else if ((*(_buffer_impl_t**)&newbuf->_impl_p = _get_buffer_impl(size)) == NULL)
    {
        goto r_buf;
    }
    else
    {
        DEBUG("Allocated new buffer");
    }

    newbuf->write = &_write;
    newbuf->copy = &_copy;
    newbuf->write_from_user = &_write_from_user;
    newbuf->copy_to_user = &_copy_to_user;
    newbuf->reset = &_reset;
    newbuf->sync = &_sync;
    newbuf->read = &_read;
    newbuf->get_n_bytes_readable = &_get_n_bytes_readable;

    return newbuf;

    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;

}

void put_buffer(buffer_t* buf)
{
    TRACE("");
    if (buf == NULL)
    {
        ERROR("Invalid buffer pointer");
        return;
    }

    _put_buffer_impl(buf->_impl_p);
    kfree(buf);
    buf = NULL;
}
