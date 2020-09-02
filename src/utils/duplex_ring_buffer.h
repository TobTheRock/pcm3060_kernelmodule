/** @file duplex_ring_buffer.h
 *  @brief wrapper fur duplex ring buffers
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_DUPLEX_RING_BUFFER_H
#define KERNELMODULE_PCM3060_DUPLEX_RING_BUFFER_H

struct _duplex_ring_buffer_impl;
typedef struct duplex_ring_end duplex_ring_end_t;

typedef struct duplex_ring_buffer
{
    const unsigned int size;
    struct duplex_ring_end* left_end,* right_end;
    struct _duplex_ring_buffer_impl*const _impl_p;
} duplex_ring_buffer_t;

duplex_ring_buffer_t* get_duplex_ring_buffer(const unsigned int size);
void put_duplex_ring_buffer(duplex_ring_buffer_t* buf);

unsigned int duplex_ring_end_copy_from_user(struct duplex_ring_end* duplex_ring_end, const void* buffer_ext, const unsigned int buflen);
unsigned int duplex_ring_end_copy_to_user(const struct duplex_ring_end* duplex_ring_end, void* buffer_ext, const unsigned int buflen);

// READER

unsigned int duplex_ring_end_n_bytes_readable(const duplex_ring_end_t* duplex_ring_end);
unsigned int duplex_ring_end_n_bytes_writable(const duplex_ring_end_t* duplex_ring_end);
// start reading by copy, return NOF bytes dropped
unsigned int duplex_ring_end_read(const duplex_ring_end_t* duplex_ring_end, void* o_buffer_ptr, const unsigned int buflen);
//WRITER
//
unsigned int duplex_ring_end_write(duplex_ring_end_t* duplex_ring_end, const void* o_buffer_ptr, const unsigned int buflen);


#endif // !KERNELMODULE_PCM3060_DUPLEX_RING_BUFFER_H