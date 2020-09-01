#include "pipe_buffer.h"
#include <utils/logging.h>
#include <utils/buffer.h>
#include <utils/ptr.h>

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/completion.h>

#define  WAIT_FOR_DATA_WRITE(ptr_bufimpl)\
{\
    if (!buffer_get_n_bytes_readable(ptr_bufimpl->buf))\
    {\
        DEBUG("Waiting for data...");\
        wait_for_completion_killable(&ptr_bufimpl->write_completion);\
    }\
}

#define WAIT_FOR_READ_COMPLETE(ptr_bufimpl)\
{\
    if (atomic_read(&ptr_bufimpl->n_active_readers))\
    {\
        TRACE("Waiting for read to complete...");\
        wait_for_completion_killable(&ptr_bufimpl->read_completion);\
    }\
}

typedef struct _pipe_buffer_impl
{
    atomic_t n_active_readers, n_bytes_read;
    buffer_t *buf;
    struct completion read_completion, write_completion;
} _pipe_buffer_impl_t;


static inline void _notify_data_write(_pipe_buffer_impl_t* bufimpl, unsigned int n_data)
{
    TRACE("");
    if (n_data)
    {
        DEBUG("Notifying waiting readers.");
        complete_all(&bufimpl->write_completion);
    }
}

static inline void _check_reset_after_read(_pipe_buffer_impl_t* bufimpl, unsigned int n_bytes_read)
{
    if (atomic_add_return(n_bytes_read, &bufimpl->n_bytes_read) >= buffer_get_n_bytes_readable(bufimpl->buf))
    {
        TRACE("Resetting buffer");
        atomic_set(&bufimpl->n_bytes_read, 0);
        buffer_reset(bufimpl->buf);
    }
}

static inline _pipe_buffer_impl_t* _get_pipe_buffer_impl(const unsigned int size)
{
    _pipe_buffer_impl_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(_pipe_buffer_impl_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((newbuf->buf = get_buffer(size)) == NULL)
    {
        ERROR("Could not allocate read buffer.");
        goto r_buf;
    }
    else
    {
        DEBUG("Allocated new pipe buffer impl");
    }

    atomic_set(&newbuf->n_active_readers, 0);
    atomic_set(&newbuf->n_bytes_read, 0);

    init_completion(&newbuf->write_completion);
    init_completion(&newbuf->read_completion);
    complete(&newbuf->read_completion);
    return newbuf;
    //ERROR cleanups
    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;
}

static inline void _put_pipe_buffer_impl (_pipe_buffer_impl_t* bufimpl)
{
    TRACE("");
    RETURN_VOID_ON_NULL(bufimpl);

    complete_all(&bufimpl->write_completion);
    WAIT_FOR_READ_COMPLETE(bufimpl);

    TRACE("Freeing...");
    put_buffer(bufimpl->buf);
    kfree(bufimpl);
    bufimpl = NULL;
}



pipe_buffer_t* get_pipe_buffer(const unsigned int size)
{
    _pipe_buffer_impl_t* impl;
    pipe_buffer_t* newbuf;
    TRACE("");
    if ((newbuf = kmalloc(sizeof(pipe_buffer_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Could not allocate kernel memory.");
        goto r_null;
    }
    else if ((impl = _get_pipe_buffer_impl(size)) == NULL) // CONST_CAST(_pipe_buffer_impl_t*)?
    {
        ERROR("Could not create buffer imp");
        goto r_buf;
    }
    else
    {
        pipe_buffer_t tmpbuf = {.size = size, ._impl_p = impl};
        memcpy(newbuf, &tmpbuf, sizeof(pipe_buffer_t));
        DEBUG("Allocated new pipe_buffer");
    }


    return newbuf;

    r_buf:
        kfree(newbuf);
    r_null:
        return NULL;
}

void put_pipe_buffer(pipe_buffer_t* buf)
{
    TRACE("");
    RETURN_VOID_ON_NULL(buf);


    _put_pipe_buffer_impl(buf->_impl_p);
    kfree(buf);
    buf = NULL;
}

unsigned int pipe_buffer_copy_from_user(struct pipe_buffer* pipe_buffer, const void* buffer_ext, const unsigned int buflen)
{
    unsigned int n_bytes_dropped;
    TRACE("");
    RETURN_ON_NULL(pipe_buffer, buflen);
    WAIT_FOR_READ_COMPLETE(pipe_buffer->_impl_p);
    n_bytes_dropped = buffer_copy_from_user(pipe_buffer->_impl_p->buf, buffer_ext, buflen);
    _notify_data_write(pipe_buffer->_impl_p, buflen);
    return n_bytes_dropped;
}

unsigned int pipe_buffer_copy_to_user(const struct pipe_buffer* pipe_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off)
{
    unsigned int n_bytes_dropped, n_bytes_read;
    RETURN_ON_NULL(pipe_buffer, buflen);
    n_bytes_dropped = buffer_copy_to_user(pipe_buffer->_impl_p->buf, buffer_ext, buflen, off);

    n_bytes_read = (buflen + off - n_bytes_dropped); // mark everything before off as read...
    _check_reset_after_read(pipe_buffer->_impl_p, n_bytes_read);
    return n_bytes_dropped;
}

unsigned int pipe_buffer_n_bytes_available(const struct pipe_buffer* pipe_buffer)
{
    TRACE("");
    RETURN_ON_NULL(pipe_buffer, 0);
    return buffer_get_n_bytes_readable(pipe_buffer->_impl_p->buf);
}

unsigned int pipe_buffer_read_start(const pipe_buffer_t* pipe_buffer, void** o_buffer_ptr)
{
    TRACE("");
    RETURN_ON_NULL(pipe_buffer, 0);
    if(atomic_inc_return(&pipe_buffer->_impl_p->n_active_readers) == 1) //first reader
    {
        TRACE("Locking");
        reinit_completion(&pipe_buffer->_impl_p->read_completion);
    }
    return buffer_read(pipe_buffer->_impl_p->buf, o_buffer_ptr, 0);
}

unsigned int pipe_buffer_read_start_waiting(const pipe_buffer_t* pipe_buffer, void** o_buffer_ptr)
{
    TRACE("");
    RETURN_ON_NULL(pipe_buffer, 0);
    WAIT_FOR_DATA_WRITE(pipe_buffer->_impl_p);
    return pipe_buffer_read_start(pipe_buffer, o_buffer_ptr);
}

void pipe_buffer_read_end(const pipe_buffer_t* pipe_buffer, const unsigned int n_bytes_read)
{
    TRACE("");
    RETURN_VOID_ON_NULL(pipe_buffer);

    if (atomic_dec_and_test(&pipe_buffer->_impl_p->n_active_readers))
    {
        _check_reset_after_read(pipe_buffer->_impl_p, n_bytes_read);
        TRACE("Unlocking");
        complete_all(&pipe_buffer->_impl_p->read_completion);
    }
}

unsigned int pipe_buffer_write_start(pipe_buffer_t* pipe_buffer, void** o_buffer_ptr, const unsigned int n_bytes_requested)
{
    TRACE("");
    RETURN_ON_NULL(pipe_buffer, 0);
    return buffer_write_reserve(pipe_buffer->_impl_p->buf, o_buffer_ptr, n_bytes_requested);
}


void pipe_buffer_write_end(pipe_buffer_t* pipe_buffer)
{
    TRACE("");
    RETURN_VOID_ON_NULL(pipe_buffer);
    buffer_write_finish(pipe_buffer->_impl_p->buf);
}
