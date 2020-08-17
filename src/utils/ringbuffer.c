#include "ringbuffer.h"

#include <utils/logging.h>

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/err.h>

typedef struct _ringbuffer_impl
{
    void* buf;
    const unsigned int bufsize;
    atomic_t readcnt, writecnt, refcnt;
    struct mutex mx;
} _ringbuffer_impl_t;


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


static inline _ringbuffer_impl_t* get_ringbuffer_impl(const unsigned int size)
{
    _ringbuffer_impl_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(_ringbuffer_impl_t), GFP_KERNEL)) == NULL)
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
        DEBUG("Allocated new ringbuffer");
    }

    mutex_init(&newbuf->mx);
    *(unsigned int*)&newbuf->bufsize = size;
    atomic_set(&newbuf->readcnt, 0);
    atomic_set(&newbuf->writecnt, 0);
    atomic_set(&newbuf->refcnt, 1);

    return newbuf;

    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;
}

static inline void _put_ringbuffer_impl(_ringbuffer_impl_t* bufimpl)
{
    TRACE("");
    if (!bufimpl)
    {
        ERROR("Invalid buffer pointer!");
        return;
    }

    TRACE("Decreasing refcount...");
    if (atomic_dec_and_test(&bufimpl->refcnt))
    {
        TRACE("Freeing...");
        kfree(bufimpl->buf);
        kfree(bufimpl);
    }
}

static inline int _write_impl (struct ringbuffer* ring, const void* buf, const unsigned int buflen, bool fromUser)
{
    unsigned int ringbufsize;
    TRACE("");
    if ((buf == NULL) || (ring == NULL))
    {
        ERROR("Invalid (ring)buffer");
        return ENXIO;
    }
    else if (buflen == 0)
    {
        DEBUG("Buflen is 0, nothing todo");
        return 0;
    }

    ringbufsize = ring->_impl_p->bufsize;

    if (buflen > ringbufsize)
    {
        ERROR("Ringbuffer with size %d is to small for write with %d bytes", ringbufsize, buflen);
        return ENOMEM;
    }

    {
        int ret = 0;
        unsigned int bytesToEnd, bytesToWriteAtStart, bytesToWriteAtEnd,
                     bytesWritten, bytesRead, maxBytesWritable;

        bytesWritten = atomic_read(&ring->_impl_p->writecnt);
        bytesRead = atomic_read(&ring->_impl_p->readcnt);
        maxBytesWritable = _get_ring_distance(bytesWritten, bytesRead, ringbufsize);

        if (buflen >  maxBytesWritable )
        {
            ERROR("Ringbuffer overflow! Cannot write %d bytes, only space left for %d", buflen, maxBytesWritable);
            return ENOMEM;
        }

        bytesToEnd = ringbufsize - bytesWritten;

        bytesToWriteAtStart = (bytesToEnd < buflen) ? (buflen-bytesToEnd) : (0);
        bytesToWriteAtEnd = (bytesToWriteAtStart)? buflen : bytesToEnd;


        mutex_lock(&ring->_impl_p->mx);
        if (fromUser)
        {
            if (copy_from_user( ring->_impl_p->buf + bytesWritten,
                        buf, bytesToWriteAtEnd) != 0)
            {
                ERROR("Could not copy memory at end");
                ret = ENOMEM;
            }
            else if ( bytesToWriteAtStart &&
                    (copy_from_user( ring->_impl_p->buf, buf, bytesToWriteAtStart) != 0) )
            {
                ERROR("Could not copy memory at front");
                ret = ENOMEM;
            }

        }
        else
        {
            if ( memcpy( ring->_impl_p->buf + bytesWritten, buf, bytesToWriteAtEnd) == NULL)
            {
                ERROR("Could not copy memory at end");
                ret = ENOMEM;
            }
            else if ( bytesToWriteAtStart &&
                    (memcpy( ring->_impl_p->buf, buf, bytesToWriteAtStart) == NULL) )
            {
                ERROR("Could not copy memory at front");
                ret = ENOMEM;
            }
        }
    
        if (!ret)
        {
            TRACE("Setting new writecnt %d", (bytesWritten+buflen)%ringbufsize);
            atomic_set(&ring->_impl_p->writecnt,
                    (bytesWritten+buflen)%ringbufsize);
        }

        mutex_unlock(&ring->_impl_p->mx);

        return ret;
    }
}

static int _write (struct ringbuffer* ring, const void* buf, unsigned int buflen)
{
    TRACE("");
    return _write_impl(ring, buf, buflen, 0);
}

static int _write_from_user (struct ringbuffer* ring, const void* buf, unsigned int buflen)
{
    TRACE("");
    return _write_impl(ring, buf, buflen, 1);
}

static inline unsigned int _read_impl (struct ringbuffer* ring, void* buf, unsigned int buflen, bool fromUser)
{
    unsigned int ret_bytes_read = 0;
    TRACE("");
    if ((buf == NULL) || (ring == NULL))
    {
        ERROR("Invalid (ring)buffer");
    }
    else
    {
        unsigned int bytesToEnd, bytesToReadAtStart, bytesToReadAtEnd,
                     bytesRead, bytesWritten, ringbufsize, nbytes_toread, maxBytesReadable;


        ringbufsize = ring->_impl_p->bufsize;
        mutex_lock(&ring->_impl_p->mx);

        bytesRead = atomic_read(&ring->_impl_p->readcnt);
        bytesWritten = atomic_read(&ring->_impl_p->writecnt);

        maxBytesReadable = _get_ring_distance(bytesRead, bytesWritten , ringbufsize);

        bytesToEnd = ringbufsize - bytesRead;
        nbytes_toread = min(buflen, maxBytesReadable);

        bytesToReadAtStart = (bytesToEnd < nbytes_toread) ? (nbytes_toread-bytesToEnd) : (0);
        bytesToReadAtEnd = (bytesToReadAtStart)? nbytes_toread : bytesToEnd;
        if (fromUser)
        {
            if (copy_to_user(buf, ring->_impl_p->buf + bytesRead,
                            bytesToReadAtEnd) != 0)
            {
                ERROR("Could not copy memory from end");
            }
            else if ( bytesToReadAtStart &&
                    (copy_to_user(buf, ring->_impl_p->buf, bytesToReadAtStart) != 0) )
            {
                ERROR("Could not copy memory from front");
            }
            else
            {
                ret_bytes_read = nbytes_toread;
            }

        }
        else
        {
            if ( memcpy(buf, ring->_impl_p->buf + bytesRead, bytesToReadAtEnd) == NULL)
            {
                ERROR("Could not copy memory from end");
            }
            else if ( bytesToReadAtStart &&
                    (memcpy(buf, ring->_impl_p->buf, bytesToReadAtStart) == NULL) )
            {
                ERROR("Could not copy memory from front");
            }
            else
            {
                ret_bytes_read = nbytes_toread;
            }
        }
    
        if (ret_bytes_read)
        {
            TRACE("Setting new readcnt %d", (bytesRead+nbytes_toread)%ringbufsize);
            atomic_set(&ring->_impl_p->writecnt,
                        (bytesRead+nbytes_toread)%ringbufsize);
        }

        mutex_unlock(&ring->_impl_p->mx);
    }

    return ret_bytes_read;
}



static unsigned int _read (struct ringbuffer* ring, void* buf, unsigned int buflen)
{
    TRACE("");
    return _read_impl(ring, buf, buflen, 0);
}

static unsigned int _read_from_user (struct ringbuffer* ring, void* buf, unsigned int buflen)
{
    TRACE("");
    return _read_impl(ring, buf, buflen, 1);
}


ringbuffer_t* get_ringbuffer(const unsigned int size)
{
    ringbuffer_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(ringbuffer_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((*(_ringbuffer_impl_t**)&newbuf->_impl_p = get_ringbuffer_impl(size)) == NULL)
    {
        goto r_buf;
    }
    else
    {
        DEBUG("Allocated new ringbuffer");
    }

    newbuf->write = &_write;
    newbuf->read = &_read;
    newbuf->write_from_user = &_write_from_user;
    newbuf->read_from_user = &_read_from_user;

    return newbuf;

    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;

}

void put_ringbuffer(ringbuffer_t* buf)
{
    TRACE("");
    if (buf == NULL)
    {
        ERROR("Invalid ringbuffer");
        return;
    }

    _put_ringbuffer_impl(buf->_impl_p);
    kfree(buf);
    buf = NULL;
}
