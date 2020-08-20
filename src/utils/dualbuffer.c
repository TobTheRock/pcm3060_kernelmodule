#include "dualbuffer.h"
#include <utils/logging.h>
#include <utils/buffer.h>

#include <linux/types.h>
#include <linux/slab.h>

typedef struct _dualbuffer_impl
{
    atomic_t n_active_readers;
    buffer_t *read_buf, *write_buf;
} _dualbuffer_impl_t;

static inline _dualbuffer_impl_t* _get_dualbuffer_impl(const unsigned int size)
{
    _dualbuffer_impl_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(_dualbuffer_impl_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((newbuf->read_buf = get_buffer(size)) == NULL)
    {
        ERROR("Could not allocate read buffer.");
        goto r_buf;
    }
    else if ((newbuf->write_buf = get_buffer(size)) == NULL)
    {
        ERROR("Could not allocate write buffer.");
        goto r_rd_buf;
    }
    else
    {
        DEBUG("Allocated new dual buffer");
    }

    atomic_set(&newbuf->n_active_readers, 0);

    return newbuf;
    //ERROR cleanups
    r_rd_buf:
        kfree(newbuf->read_buf);
    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;
}

static inline void _put_buffer_impl (_dualbuffer_impl_t* bufimpl)
{
    TRACE("");
    if (bufimpl == NULL)
    {
        ERROR("Invalid buffer pointer!");
        return;
    }

    TRACE("Freeing...");
    put_buffer(bufimpl->read_buf);
    put_buffer(bufimpl->write_buf);
    kfree(bufimpl);
    bufimpl = NULL;
}

static unsigned int _write (struct dualbuffer* this_buffer, const void* buffer_ext, const unsigned int buflen)
{
    TRACE("");
    if (this_buffer == NULL)
    {
        ERROR("Invalid buffer pointer");
        return buflen;
    }
    return this_buffer->_impl_p->write_buf->write(this_buffer->_impl_p->write_buf, buffer_ext, buflen);
}

static unsigned int _write_from_user (struct dualbuffer* this_buffer, const void* buffer_ext, const unsigned int buflen)
{
    TRACE("");
    if (this_buffer == NULL)
    {
        ERROR("Invalid buffer pointer");
        return buflen;
    }
    return this_buffer->_impl_p->write_buf->write_from_user(this_buffer->_impl_p->write_buf, buffer_ext, buflen);
}

static void _switch_buf_start (const struct dualbuffer* this_buffer)
{
    if ( (atomic_inc_return(&this_buffer->_impl_p->n_active_readers) == 1) &&  //first registered reader
         (this_buffer->_impl_p->write_buf->get_n_bytes_readable(this_buffer->_impl_p->write_buf)) > 0 ) // Only switch if bytes have been written
    {
        //swap write and read buffers
        buffer_t* tmp = this_buffer->_impl_p->write_buf;
        DEBUG("Swapping buffers");
        TRACE("Write buffer addr: %p, Read buffer addr: %p", this_buffer->_impl_p->write_buf, this_buffer->_impl_p->read_buf);
        this_buffer->_impl_p->read_buf->reset(this_buffer->_impl_p->read_buf);
        this_buffer->_impl_p->write_buf = this_buffer->_impl_p->read_buf;
        this_buffer->_impl_p->read_buf = tmp;
        this_buffer->_impl_p->read_buf->sync(this_buffer->_impl_p->read_buf); //wait for possible writers to finish before continuing
        TRACE("Write buffer addr: %p, Read buffer addr: %p", this_buffer->_impl_p->write_buf, this_buffer->_impl_p->read_buf);
    }
    return;
}

static inline void _switch_buf_end(const struct dualbuffer* this_buffer)
{
    TRACE("");
    atomic_dec(&this_buffer->_impl_p->n_active_readers);
}

static unsigned int _copy (const struct dualbuffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off)
{
    unsigned int n_bytes_dropped;
    TRACE("");
    if (this_buffer == NULL)
    {
        ERROR("Invalid buffer pointer");
        return buflen;
    }
    _switch_buf_start(this_buffer);
    n_bytes_dropped = this_buffer->_impl_p->read_buf->copy(this_buffer->_impl_p->read_buf, buffer_ext, buflen, off);
    _switch_buf_end(this_buffer);
    return n_bytes_dropped;
}
static unsigned int _copy_to_user (const struct dualbuffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off)
{
    unsigned int n_bytes_dropped;
    TRACE("");
    if (this_buffer == NULL)
    {
        ERROR("Invalid buffer pointer");
        return buflen;
    }
    _switch_buf_start(this_buffer);
    n_bytes_dropped = this_buffer->_impl_p->read_buf->copy_to_user(this_buffer->_impl_p->read_buf, buffer_ext, buflen, off);
    _switch_buf_end(this_buffer);
    return n_bytes_dropped;
}

static unsigned int _read (const struct dualbuffer* this_buffer, void** out_buffer_p, const unsigned int off)
{
    unsigned int n_bytes_dropped;
    TRACE("");
    if (this_buffer == NULL)
    {
        ERROR("Invalid buffer pointer");
        return 0;
    }
    _switch_buf_start(this_buffer);
    return this_buffer->_impl_p->read_buf->read(this_buffer->_impl_p->read_buf, out_buffer_p, off);
}

static void _release_read (const struct dualbuffer* this_buffer, void** buffer_ext)
{
    if (this_buffer == NULL)
    {
        ERROR("Invalid buffer pointer");
    }
    // TODO check that buffer_ext is in  valid range
    else
    {
        buffer_ext = NULL;
        _switch_buf_end(this_buffer);
    }
}


static unsigned int _get_n_bytes_readable (const struct dualbuffer* this_buffer)
{
    if (this_buffer == NULL)
    {
        ERROR("Invalid buffer pointer");
        return 0;
    }
    return this_buffer->_impl_p->read_buf->get_n_bytes_readable(this_buffer->_impl_p->read_buf);
}

static void _reset (struct dualbuffer* this_buffer)
{
    if ((this_buffer == NULL))
    {
        ERROR("Invalid buffer");
        return;
    }

    this_buffer->_impl_p->read_buf->reset(this_buffer->_impl_p->read_buf);
    this_buffer->_impl_p->write_buf->reset(this_buffer->_impl_p->write_buf);
}

dualbuffer_t* get_dualbuffer(const unsigned int size)
{
    dualbuffer_t* newbuf;

    TRACE("");
    if ((newbuf = kmalloc(sizeof(dualbuffer_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((*(_dualbuffer_impl_t**)&newbuf->_impl_p = _get_dualbuffer_impl(size)) == NULL)
    {
        goto r_buf;
    }
    else
    {
        DEBUG("Allocated new dualbuffer");
    }

    newbuf->write = &_write;
    newbuf->copy = &_copy;
    newbuf->write_from_user = &_write_from_user;
    newbuf->copy_to_user = &_copy_to_user;
    newbuf->reset = &_reset;
    newbuf->read = &_read;
    newbuf->release_read = &_release_read;
    newbuf->get_n_bytes_readable = &_get_n_bytes_readable;

    return newbuf;

    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;

}

void put_dualbuffer(dualbuffer_t* buf)
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
