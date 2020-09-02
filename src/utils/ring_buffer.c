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


static inline unsigned int _get_ring_distance(const unsigned int start, const unsigned end, const unsigned ringsize)
{
    unsigned int distance;

    if (end < start)
    {
        distance = ringsize - (start - end);
    }
    else
    {
        distance = end - start;
    }

    return distance;
}


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
    TRACE("");
    RETURN_ON_NULL(ring, buflen);
    RETURN_ON_NULL(buf, buflen);

    if (buflen > ringbufsize)
    {
        ERROR("Ringbuffer with size %d is to small for write with %d bytes", ringbufsize, buflen);
        return buflen;
    }
    else
    {
        unsigned int bytesToEnd, bytesToWriteAtStart, bytesToWriteAtEnd,
                     bytesWritten, bytesRead, maxBytesWritable;

        mutex_lock(&ring->_impl_p->mx);     // TODO: probably no mutex is needed, when increasing the write cnt first
        bytesWritten = atomic_read(&ring->_impl_p->write_idx); // increase write count for next reader
        bytesRead = atomic_read(&ring->_impl_p->read_idx);
        maxBytesWritable = _get_ring_distance(bytesWritten, bytesRead, ringbufsize);

        if (buflen >  maxBytesWritable )
        {
            WARNING("Ringbuffer overflow! Cannot write %d bytes, only space left for %d", buflen, maxBytesWritable);
            n_bytes_dropped = buflen - maxBytesWritable;
        }

        bytesToEnd = ringbufsize - bytesWritten;

        bytesToWriteAtStart = (bytesToEnd < buflen) ? (buflen-bytesToEnd) : (0);
        bytesToWriteAtEnd = (bytesToWriteAtStart)? buflen : bytesToEnd;

        if (fromUser)
        {
            n_bytes_dropped += copy_from_user( ring->_impl_p->buf + bytesWritten, buf, bytesToWriteAtEnd);
            if (bytesToWriteAtStart)
            {
                n_bytes_dropped += copy_from_user( ring->_impl_p->buf, buf, bytesToWriteAtStart);
            }

        }
        else
        {
            memcpy(ring->_impl_p->buf + bytesWritten, buf, bytesToWriteAtEnd);
            if ( bytesToWriteAtStart )
            {
                memcpy( ring->_impl_p->buf, buf, bytesToWriteAtStart);
            }
        }

        mutex_unlock(&ring->_impl_p->mx);

    }

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
    unsigned int ret_bytes_read = 0, n_bytes_dropped = 0;
    unsigned int bytesToEnd, bytesToReadAtStart, bytesToReadAtEnd,
                    bytesRead, bytesWritten, ringbufsize, nbytes_toread, maxBytesReadable;
    TRACE("");
    RETURN_ON_NULL(ring, buflen);
    RETURN_ON_NULL(buf, buflen);



    ringbufsize = ring->size;
    mutex_lock(&ring->_impl_p->mx);    // TODO: probably no mutex is needed, when increasing the rd cnt first

    bytesRead = atomic_read(&ring->_impl_p->read_idx);
    bytesWritten = atomic_read(&ring->_impl_p->write_idx);

    maxBytesReadable = _get_ring_distance(bytesRead, bytesWritten , ringbufsize);

    bytesToEnd = ringbufsize - bytesRead;
    nbytes_toread = min(buflen, maxBytesReadable);

    bytesToReadAtStart = (bytesToEnd < nbytes_toread) ? (nbytes_toread-bytesToEnd) : (0);
    bytesToReadAtEnd = (bytesToReadAtStart)? nbytes_toread : bytesToEnd;
    if (fromUser)
    {
        n_bytes_dropped += copy_to_user(buf, ring->_impl_p->buf + bytesRead, bytesToReadAtEnd);
        if (bytesToReadAtStart)
        {
            n_bytes_dropped += copy_to_user(buf, ring->_impl_p->buf, bytesToReadAtStart);
        }
    }
    else
    {
        memcpy(buf, ring->_impl_p->buf + bytesRead, bytesToReadAtEnd);
        if (bytesToReadAtStart)
        {
            memcpy(buf, ring->_impl_p->buf, bytesToReadAtStart);
        }
    }

    if (ret_bytes_read)
    {
        TRACE("Setting new read_idx %d", (bytesRead+nbytes_toread)%ringbufsize);
        atomic_set(&ring->_impl_p->write_idx,
                    (bytesRead+nbytes_toread)%ringbufsize);
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
    unsigned int bytesRead, bytesWritten;
    TRACE("");
    bytesRead = atomic_read(&ring->_impl_p->read_idx);
    bytesWritten = atomic_read(&ring->_impl_p->write_idx);

    return _get_ring_distance(bytesRead, bytesWritten , ring->size);
}

unsigned int ring_buffer_n_bytes_writable(const ring_buffer_t* ring)
{
    unsigned int bytesRead, bytesWritten;
    TRACE("");
    bytesRead = atomic_read(&ring->_impl_p->read_idx);
    bytesWritten = atomic_read(&ring->_impl_p->write_idx);

    return _get_ring_distance(bytesWritten, bytesRead, ring->size);
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
