#include "duplex_pipe_buffer.h"
#include <utils/pipe_buffer.h>
#include <utils/logging.h>
#include <utils/ptr.h>

#include <linux/types.h>
#include <linux/slab.h>

typedef struct _duplex_pipe_buffer_impl
{
    pipe_buffer_t *buffer_a, *buffer_b;
} _duplex_pipe_buffer_impl_t;

typedef struct duplex_pipe_end
{
    const pipe_buffer_t* read_buffer;
    pipe_buffer_t* write_buffer;
} duplex_pipe_end_t;



static _duplex_pipe_buffer_impl_t* _get__duplex_pipe_buffer_impl(const unsigned int size)
{
    _duplex_pipe_buffer_impl_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(_duplex_pipe_buffer_impl_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((newbuf->buffer_a = get_pipe_buffer(size/2)) == NULL)
    {
        ERROR("Could not allocate buffer.");
        goto r_buf;
    }
    else if ((newbuf->buffer_b = get_pipe_buffer(size/2)) == NULL)
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
        put_pipe_buffer(newbuf->buffer_a);
    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;
}

static void _put__duplex_pipe_buffer_impl(_duplex_pipe_buffer_impl_t* bufimpl)
{
    TRACE("");
    put_pipe_buffer(bufimpl->buffer_a);
    put_pipe_buffer(bufimpl->buffer_b);
    kfree(bufimpl);
}
static duplex_pipe_end_t* _get_duplex_pipe_end(pipe_buffer_t* wr_buf, const pipe_buffer_t* rd_buf)
{
    duplex_pipe_end_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(duplex_pipe_end_t), GFP_KERNEL)) == NULL)
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

static void _put_duplex_pipe_end(duplex_pipe_end_t* bufend)
{
    TRACE("");
    kfree(bufend);
}

duplex_pipe_buffer_t* get_duplex_pipe_buffer(const unsigned int size)
{
    _duplex_pipe_buffer_impl_t* impl;
    duplex_pipe_end_t* left_end, *right_end;
    duplex_pipe_buffer_t* newbuf;
    TRACE("");

    if ((newbuf = kmalloc(sizeof(duplex_pipe_buffer_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((impl = _get__duplex_pipe_buffer_impl(size)) == NULL)
    {
        ERROR("Could not create buffer imp");
        goto r_buf;
    }
    else if ((left_end = _get_duplex_pipe_end(impl->buffer_a, impl->buffer_b)) == NULL)
    {
        ERROR("Could not create pipe end");
        goto r_buf_impl;
    }
    else if ((right_end = _get_duplex_pipe_end(impl->buffer_b, impl->buffer_a)) == NULL)
    {
        ERROR("Could not create pipe end");
        goto r_buf_end;
    }
    else
    {
        duplex_pipe_buffer_t tmpbuf =
            {.size = size,
             .left_end = left_end,
             .right_end = right_end,
             ._impl_p = impl
            };
        memcpy(newbuf, &tmpbuf, sizeof(duplex_pipe_buffer_t));
        DEBUG("Allocated new pipe_buffer");
    }

    return newbuf;
    r_buf_end:
        _put_duplex_pipe_end(left_end);
    r_buf_impl:
        _put__duplex_pipe_buffer_impl(impl);
    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;
}

void put_duplex_pipe_buffer(duplex_pipe_buffer_t* buf)
{
    TRACE("");
    RETURN_VOID_ON_NULL(buf);

    _put__duplex_pipe_buffer_impl(buf->_impl_p);
    kfree(buf);
}

unsigned int duplex_pipe_end_copy_from_user(struct duplex_pipe_end* duplex_pipe_end, const void* buffer_ext, const unsigned int buflen)
{
    TRACE("");
    RETURN_ON_NULL(duplex_pipe_end, buflen);
    return pipe_buffer_copy_from_user(duplex_pipe_end->write_buffer, buffer_ext, buflen);
}

unsigned int duplex_pipe_end_copy_to_user(const struct duplex_pipe_end* duplex_pipe_end, void* buffer_ext, const unsigned int buflen, const unsigned int off)
{
    TRACE("");
    RETURN_ON_NULL(duplex_pipe_end, buflen);
    return pipe_buffer_copy_to_user(duplex_pipe_end->read_buffer, buffer_ext, buflen, off);
    
}

unsigned int duplex_pipe_end_n_bytes_available(const duplex_pipe_end_t* duplex_pipe_end)
{
    RETURN_ON_NULL(duplex_pipe_end, 0);
    return pipe_buffer_n_bytes_available(duplex_pipe_end->read_buffer);
}

unsigned int duplex_pipe_end_read_start(const duplex_pipe_end_t* duplex_pipe_end, void** o_buffer_ptr)
{
    RETURN_ON_NULL(duplex_pipe_end, 0);
    return pipe_buffer_read_start(duplex_pipe_end->read_buffer, o_buffer_ptr);
}

unsigned int duplex_pipe_end_read_start_waiting(const duplex_pipe_end_t* duplex_pipe_end, void** o_buffer_ptr)
{
    RETURN_ON_NULL(duplex_pipe_end, 0);
    return pipe_buffer_read_start_waiting(duplex_pipe_end->read_buffer, o_buffer_ptr);
}

void duplex_pipe_end_read_end(const duplex_pipe_end_t* duplex_pipe_end)
{
    RETURN_VOID_ON_NULL(duplex_pipe_end);
    return pipe_buffer_read_end(duplex_pipe_end->read_buffer);
}

unsigned int duplex_pipe_end_write_start(duplex_pipe_end_t* duplex_pipe_end, void** o_buffer_ptr, const unsigned int n_bytes_requested)
{
    RETURN_ON_NULL(duplex_pipe_end, 0);
    return pipe_buffer_write_start(duplex_pipe_end->write_buffer, o_buffer_ptr, n_bytes_requested);
}

void duplex_pipe_end_write_end(duplex_pipe_end_t* duplex_pipe_end)
{
    RETURN_VOID_ON_NULL(duplex_pipe_end);
    return pipe_buffer_write_end(duplex_pipe_end->write_buffer);
}
