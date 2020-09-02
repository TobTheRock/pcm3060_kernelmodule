/** @file ringbuffer.h
 *  @brief Definition of a ringbuffer
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_UTILS_RING_BUFFER_H
#define KERNELMODULE_PCM3060_UTILS_RING_BUFFER_H

struct _ring_buffer_impl;


typedef struct ring_buffer
{
    const unsigned int size;
    struct _ring_buffer_impl* const _impl_p;
} ring_buffer_t;

ring_buffer_t* get_ring_buffer(const unsigned int size);
void put_ring_buffer(ring_buffer_t* buf);

// return NOF bytes dropped
unsigned int ring_buffer_write (ring_buffer_t* ring, const void* buf, unsigned int buflen);
unsigned int ring_buffer_copy_from_user (ring_buffer_t* ring, const void* buf, unsigned int buflen);

unsigned int  ring_buffer_read (const ring_buffer_t* ring, void* buf, unsigned int buflen);
unsigned int ring_buffer_copy_to_user (const ring_buffer_t* ring, void* buf, unsigned int buflen);
unsigned int ring_buffer_n_bytes_readable(const ring_buffer_t* ring);
unsigned int ring_buffer_n_bytes_writable(const ring_buffer_t* ring);

#endif // !KERNELMODULE_PCM3060_UTILS_RING_BUFFER_H