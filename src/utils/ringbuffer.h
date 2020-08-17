/** @file ringbuffer.h
 *  @brief Definition of a ringbuffer
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_UTILS_RINGBUFFER_H
#define KERNELMODULE_PCM3060_UTILS_RINGBUFFER_H

struct _ringbuffer_impl;


typedef struct ringbuffer
{
    int (*write) (struct ringbuffer* ring, const void* buf, unsigned int buflen);
    int (*write_from_user) (struct ringbuffer* ring, const void* buf, unsigned int buflen);
    // return NOF bytes read
    unsigned int (*read) (struct ringbuffer* ring, void* buf, unsigned int buflen);
    unsigned int (*read_from_user) (struct ringbuffer* ring, void* buf, unsigned int buflen);
    struct _ringbuffer_impl* const _impl_p;
} ringbuffer_t;

ringbuffer_t* get_ringbuffer(const unsigned int size);

void put_ringbuffer(ringbuffer_t* buf);

#endif // !KERNELMODULE_PCM3060_UTILS_RINGBUFFER_H