/** @file buffer.h
 *  @brief Definition of a simple buffer
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_UTILS_RINGBUFFER_H
#define KERNELMODULE_PCM3060_UTILS_RINGBUFFER_H

struct _buffer_impl;


typedef struct buffer
{
    // return NOF bytes dropped/ not copied
    unsigned int (* write) (struct buffer* this_buffer, const void* buffer_ext, const unsigned int buflen);
    unsigned int (* write_from_user) (struct buffer* this_buffer, const void* buffer_ext, const unsigned int buflen);

    // return NOF bytes not copied
    unsigned int (* copy) (struct buffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off);
    unsigned int (* copy_to_user) (struct buffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off);

    //direct read, returns NOF bytes readible,
    unsigned int (* read) (struct buffer* this_buffer, void* buffer_ext, const unsigned int off);
    
    //get nof readable bytes
    unsigned int (* get_n_bytes_readable) (struct buffer* this_buffer);

    //resets the buffer, pointers from read are still valied, but may contain outdated data
    void (*reset) (struct buffer* this_buffer);

    struct _buffer_impl* const _impl_p;
} buffer_t;

buffer_t* get_buffer(const unsigned int size);

void put_buffer(buffer_t* buf);

#endif // !KERNELMODULE_PCM3060_UTILS_RINGBUFFER_H