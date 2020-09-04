#include "ring_buffer.h"

#include <utils/logging.h>
#include <utils/ptr.h>


#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/err.h>

typedef struct _ring_buffer_impl
{
    void* buf;
    atomic_t read_idx, write_idx;
    struct mutex mx;
} _ring_buffer_impl_t;

static inline _ring_buffer_impl_t* get_ringbuffer_impl(const unsigned int size)
{
    _ring_buffer_impl_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(_ring_buffer_impl_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((newbuf->buf = kmalloc(size, GFP_KERNEL)) == NULL)
    {
        goto r_buf;
    }
    else
    {
        TRACE("Allocated new byte buffer %p", newbuf->buf);
    }

    mutex_init(&newbuf->mx);
    atomic_set(&newbuf->read_idx, 0);
    atomic_set(&newbuf->write_idx, 0);

    return newbuf;

    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;
}

static inline void _put_ringbuffer_impl(_ring_buffer_impl_t* bufimpl)
{
    TRACE("");
    RETURN_VOID_ON_NULL(bufimpl);
    TRACE("Freeing...");
    kfree(bufimpl->buf);
    kfree(bufimpl);
}

static inline unsigned int _write_impl (ring_buffer_t* ring, const void* buf, const unsigned int buflen, bool fromUser)
{
    const unsigned int ringbufsize = ring->size;
    unsigned int n_bytes_dropped = 0;
    unsigned int bytes_to_write_from_start, bytes_to_write_from_idx, wr_idx, rd_idx;
    TRACE("");
    RETURN_ON_NULL(ring, buflen);
    RETURN_ON_NULL(buf, buflen);


    mutex_lock(&ring->_impl_p->mx);     // TODO: probably no mutex is needed, when increasing the write cnt first

    
    wr_idx = atomic_read(&ring->_impl_p->write_idx); // increase write count for next reader
    rd_idx = atomic_read(&ring->_impl_p->read_idx);

    if (rd_idx > wr_idx)
    {
        bytes_to_write_from_idx = min((rd_idx - wr_idx), buflen);
        bytes_to_write_from_start = 0;
    }
    else
    {
        bytes_to_write_from_idx = min(buflen, (ringbufsize-wr_idx));
        bytes_to_write_from_start = (bytes_to_write_from_idx < buflen)? min((buflen-bytes_to_write_from_idx), (wr_idx-rd_idx)) : 0;
    }

    n_bytes_dropped = buflen - bytes_to_write_from_idx - bytes_to_write_from_start;
    TRACE("Setting new write_idx %d", (wr_idx + bytes_to_write_from_idx + bytes_to_write_from_start)%ringbufsize);
    atomic_set(&ring->_impl_p->write_idx, (wr_idx + bytes_to_write_from_idx + bytes_to_write_from_start) % ringbufsize);

    TRACE("Copying at index %d and at begin %d bytes", bytes_to_write_from_idx, bytes_to_write_from_start);
    if (fromUser)
    {
        n_bytes_dropped += copy_from_user( ring->_impl_p->buf + wr_idx, buf, bytes_to_write_from_idx);
        if (bytes_to_write_from_start)
        {
            n_bytes_dropped += copy_from_user( ring->_impl_p->buf, buf, bytes_to_write_from_start);
        }

    }
    else
    {
        memcpy(ring->_impl_p->buf + wr_idx, buf, bytes_to_write_from_idx);
        if ( bytes_to_write_from_start )
        {
            memcpy( ring->_impl_p->buf, buf, bytes_to_write_from_start);
        }
    }

    mutex_unlock(&ring->_impl_p->mx);

    return n_bytes_dropped;
}

unsigned int ring_buffer_write (ring_buffer_t* ring, const void* buf, unsigned int buflen)
{
    TRACE("");
    return _write_impl(ring, buf, buflen, 0);
}

unsigned int ring_buffer_copy_from_user (ring_buffer_t* ring, const void* buf, unsigned int buflen)
{
    TRACE("");
    return _write_impl(ring, buf, buflen, 1);
}

static inline unsigned int _read_impl (const ring_buffer_t* ring, void* buf, unsigned int buflen, bool fromUser)
{
    const unsigned int ringbufsize = ring->size;
    unsigned int n_bytes_dropped = 0;
    unsigned int bytes_to_read_from_start, bytes_to_read_from_idx,  wr_idx, rd_idx;
    TRACE("");
    RETURN_ON_NULL(ring, buflen);
    RETURN_ON_NULL(buf, buflen);


    mutex_lock(&ring->_impl_p->mx);     // TODO: probably no mutex is needed, when increasing the write cnt first

    
    wr_idx = atomic_read(&ring->_impl_p->write_idx); // increase write count for next reader
    rd_idx = atomic_read(&ring->_impl_p->read_idx);

    if (rd_idx > wr_idx)
    {
        bytes_to_read_from_idx = min(buflen, (ringbufsize-rd_idx));
        bytes_to_read_from_start = (bytes_to_read_from_idx < buflen) ? min((buflen-bytes_to_read_from_idx), (wr_idx-rd_idx)) : 0;
    }
    else
    {
        bytes_to_read_from_idx = min((rd_idx - wr_idx), buflen);
        bytes_to_read_from_start = 0;
    }
    n_bytes_dropped = buflen - bytes_to_read_from_idx - bytes_to_read_from_start;
    TRACE("Setting new read_idx %d", (rd_idx + bytes_to_read_from_idx + bytes_to_read_from_start)%ringbufsize);
    atomic_set(&ring->_impl_p->read_idx, (rd_idx + bytes_to_read_from_idx + bytes_to_read_from_start)%ringbufsize);
    TRACE("Copying from index %d and from begin %d bytes", bytes_to_read_from_idx, bytes_to_read_from_start);

    if (fromUser)
    {
        TRACE("Copy to user %d %d %d %d...", * (u8*)ring->_impl_p->buf, *(u8*)(ring->_impl_p->buf+1), *(u8*)(ring->_impl_p->buf+2), *(u8*)(ring->_impl_p->buf+3));
        n_bytes_dropped += copy_to_user(buf, ring->_impl_p->buf + rd_idx, bytes_to_read_from_idx);
        if (bytes_to_read_from_start)
        {
            n_bytes_dropped += copy_to_user(buf, ring->_impl_p->buf, bytes_to_read_from_start);
        }
    }
    else
    {
        memcpy(buf, ring->_impl_p->buf + rd_idx, bytes_to_read_from_idx);
        if (bytes_to_read_from_start)
        {
            memcpy(buf, ring->_impl_p->buf, bytes_to_read_from_start);
        }
    }


    mutex_unlock(&ring->_impl_p->mx);
    

    return n_bytes_dropped;
}



unsigned int ring_buffer_read (const ring_buffer_t* ring, void* buf, unsigned int buflen)
{
    TRACE("");
    return _read_impl(ring, buf, buflen, 0);
}

unsigned int ring_buffer_copy_to_user (const ring_buffer_t* ring, void* buf, unsigned int buflen)
{
    TRACE("");
    return _read_impl(ring, buf, buflen, 1);
}

unsigned int ring_buffer_n_bytes_readable(const ring_buffer_t* ring)
{
    unsigned int wr_idx, rd_idx, n_bytes_readable;
    TRACE("");
    wr_idx = atomic_read(&ring->_impl_p->write_idx);
    rd_idx = atomic_read(&ring->_impl_p->read_idx);
    if (rd_idx > wr_idx)
    {
        n_bytes_readable = ring->size - (rd_idx - wr_idx); // = size - bytes_not_readable
    }
    else
    {
        n_bytes_readable = wr_idx - rd_idx;
    }

    return n_bytes_readable;
}

unsigned int ring_buffer_n_bytes_writable(const ring_buffer_t* ring)
{
    unsigned int wr_idx, rd_idx, n_bytes_writable = 0;
    TRACE("");
    wr_idx = atomic_read(&ring->_impl_p->write_idx);
    rd_idx = atomic_read(&ring->_impl_p->read_idx);

    if (rd_idx > wr_idx)
    {
        n_bytes_writable = rd_idx - wr_idx;
    }
    else
    {
        n_bytes_writable = ring->size - (wr_idx - rd_idx); // = size - bytes_not_writable
    }

    return n_bytes_writable;
}

ring_buffer_t* get_ring_buffer(const unsigned int size)
{
    ring_buffer_t* newbuf;
    _ring_buffer_impl_t* impl;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(ring_buffer_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((impl = get_ringbuffer_impl(size)) == NULL)
    {
        goto r_buf;
    }
    else
    {
        ring_buffer_t tmpbuf = {.size = size, ._impl_p = impl};
        memcpy(newbuf, &tmpbuf, sizeof(ring_buffer_t));
        DEBUG("Allocated new ringbuffer");
    }

    return newbuf;

    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;

}

void put_ring_buffer(ring_buffer_t* buf)
{
    TRACE("");
    RETURN_VOID_ON_NULL(buf);

    _put_ringbuffer_impl(buf->_impl_p);
    kfree(buf);
    buf = NULL;
}
