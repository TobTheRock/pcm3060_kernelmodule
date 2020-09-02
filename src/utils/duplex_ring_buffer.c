#include "duplex_ring_buffer.h"
#include <utils/ring_buffer.h>
#include <utils/logging.h>
#include <utils/ptr.h>

#include <linux/types.h>
#include <linux/slab.h>

typedef struct _duplex_ring_buffer_impl
{
    ring_buffer_t *buffer_a, *buffer_b;
} _duplex_ring_buffer_impl_t;


// TODO invalidate ends when ringbuffer is deleted...
typedef struct duplex_ring_end
{
    const ring_buffer_t* read_buffer;
    ring_buffer_t* write_buffer;
} duplex_ring_end_t;



static _duplex_ring_buffer_impl_t* _get__duplex_ring_buffer_impl(const unsigned int size)
{
    _duplex_ring_buffer_impl_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(_duplex_ring_buffer_impl_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((newbuf->buffer_a = get_ring_buffer(size/2)) == NULL)
    {
        ERROR("Could not allocate buffer.");
        goto r_buf;
    }
    else if ((newbuf->buffer_b = get_ring_buffer(size/2)) == NULL)
    {
        ERROR("Could not allocate buffer.");
        goto r_buf_a;
    }
    else
    {
        DEBUG("Allocated new duplex pipe buffer impl");
    }

    return newbuf;
    //ERROR cleanups
    r_buf_a:
        put_ring_buffer(newbuf->buffer_a);
    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;
}

static void _put__duplex_ring_buffer_impl(_duplex_ring_buffer_impl_t* bufimpl)
{
    TRACE("");
    put_ring_buffer(bufimpl->buffer_a);
    put_ring_buffer(bufimpl->buffer_b);
    kfree(bufimpl);
}
static duplex_ring_end_t* _get_duplex_ring_end(ring_buffer_t* wr_buf, const ring_buffer_t* rd_buf)
{
    duplex_ring_end_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(duplex_ring_end_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        return NULL;
    }
    else
    {
        DEBUG("Allocated new duplex pipe buffer end");
        newbuf->write_buffer = wr_buf;
        newbuf->read_buffer = rd_buf;
    }

    return newbuf;
}

static void _put_duplex_ring_end(duplex_ring_end_t* bufend)
{
    TRACE("");
    kfree(bufend);
}

duplex_ring_buffer_t* get_duplex_ring_buffer(const unsigned int size)
{
    _duplex_ring_buffer_impl_t* impl;
    duplex_ring_end_t* left_end, *right_end;
    duplex_ring_buffer_t* newbuf;
    TRACE("");

    if ((newbuf = kmalloc(sizeof(duplex_ring_buffer_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((impl = _get__duplex_ring_buffer_impl(size)) == NULL)
    {
        ERROR("Could not create buffer imp");
        goto r_buf;
    }
    else if ((left_end = _get_duplex_ring_end(impl->buffer_a, impl->buffer_b)) == NULL)
    {
        ERROR("Could not create pipe end");
        goto r_buf_impl;
    }
    else if ((right_end = _get_duplex_ring_end(impl->buffer_b, impl->buffer_a)) == NULL)
    {
        ERROR("Could not create pipe end");
        goto r_buf_end;
    }
    else
    {
        duplex_ring_buffer_t tmpbuf =
            {.size = size,
             .left_end = left_end,
             .right_end = right_end,
             ._impl_p = impl
            };
        memcpy(newbuf, &tmpbuf, sizeof(duplex_ring_buffer_t));
        DEBUG("Allocated new ring_buffer");
    }

    return newbuf;
    r_buf_end:
        _put_duplex_ring_end(left_end);
    r_buf_impl:
        _put__duplex_ring_buffer_impl(impl);
    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;
}

void put_duplex_ring_buffer(duplex_ring_buffer_t* buf)
{
    TRACE("");
    RETURN_VOID_ON_NULL(buf);

    _put__duplex_ring_buffer_impl(buf->_impl_p);
    kfree(buf);
}

unsigned int duplex_ring_end_copy_from_user(struct duplex_ring_end* duplex_ring_end, const void* buffer_ext, const unsigned int buflen)
{
    TRACE("");
    RETURN_ON_NULL(duplex_ring_end, buflen);
    return ring_buffer_copy_from_user(duplex_ring_end->write_buffer, buffer_ext, buflen);
}

unsigned int duplex_ring_end_copy_to_user(const struct duplex_ring_end* duplex_ring_end, void* buffer_ext, const unsigned int buflen)
{
    TRACE("");
    RETURN_ON_NULL(duplex_ring_end, buflen);
    return ring_buffer_copy_to_user(duplex_ring_end->read_buffer, buffer_ext, buflen);
    
}

unsigned int duplex_ring_end_n_bytes_readable(const duplex_ring_end_t* duplex_ring_end)
{
    TRACE("");
    RETURN_ON_NULL(duplex_ring_end, 0);
    return ring_buffer_n_bytes_readable(duplex_ring_end->read_buffer);
}

unsigned int duplex_ring_end_n_bytes_writable(const duplex_ring_end_t* duplex_ring_end)
{
    TRACE("");
    RETURN_ON_NULL(duplex_ring_end, 0);
    return ring_buffer_n_bytes_writable(duplex_ring_end->write_buffer);
}

unsigned int duplex_ring_end_read(const duplex_ring_end_t* duplex_ring_end, void* o_buffer_ptr, const unsigned int buflen)
{
    TRACE("");
    RETURN_ON_NULL(duplex_ring_end, 0);
    return ring_buffer_read(duplex_ring_end->read_buffer, o_buffer_ptr, buflen);
}


unsigned int duplex_ring_end_write(duplex_ring_end_t* duplex_ring_end, const void* o_buffer_ptr, const unsigned int buflen)
{
    TRACE("");
    RETURN_ON_NULL(duplex_ring_end, 0);
    return ring_buffer_write(duplex_ring_end->write_buffer, o_buffer_ptr, buflen);
}

