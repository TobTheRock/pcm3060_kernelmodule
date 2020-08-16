#include "ringbuffer.h"

#include <utils/logging.h>

#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/types.h>

typedef static struct _ringbuffer_impl
{
    void* buf;
    const unsigned int bufsize;
    atomic_t readcnt, writecnt;
    mutex mx;
} _ringbuffer_impl_t;





static _ringbuffer_impl_t* get_ringbuffer_impl(const unsigned int size)
{

}

static void put_ringbuffer_impl(_ringbuffer_impl_t* bufimpl)
{

}

static int _write (struct ringbuffer* ring, const void* buf, unsigned int buflen)
{

}

static int _write_from_user (struct ringbuffer* ring, const void* buf, unsigned int buflen)
{

}
static unsigned int _read (struct ringbuffer* ring, void* buf, unsigned int buflen)
{
    
}
static unsigned int _read_from_user (struct ringbuffer* ring, void* buf, unsigned int buflen)
{

}


ringbuffer_t* get_ringbuffer(const unsigned int size)
{
    ringbuffer_t* newbuf;
    if ( ((newbuf = kmalloc(sizeof(ringbuffer_t), GFP_KERNEL)) != NULL) &&
         ((newbuf->_impl_p = get_ringbuffer_impl(size)) != NULL ))
    {
        DEBUG("Allocate new ringbuffer")
    }
    else
    {
        ERROR("Could not allocate kernel memory.");
        return NULL;
    }



    return newbuf;
}

void put_ringbuffer(ringbuffer_t* buf)
{


}
