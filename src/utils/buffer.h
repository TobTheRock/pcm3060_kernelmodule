/** @file buffer.h
 *  @brief Definition of a simple threadsafe buffer
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_UTILS_BUFFER_H
#define KERNELMODULE_PCM3060_UTILS_BUFFER_H

struct _buffer_impl;


typedef struct buffer
{
    const unsigned int size;
    struct _buffer_impl* const _impl_p;
} buffer_t;



// return NOF bytes dropped(not copied)
unsigned int buffer_write_copy (struct buffer* this_buffer, const void* buffer_ext, const unsigned int buflen);
unsigned int buffer_read_copy (const struct buffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off);

unsigned int buffer_copy_from_user (struct buffer* this_buffer, const void* buffer_ext, const unsigned int buflen);
unsigned int buffer_copy_to_user (const struct buffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off);

//direct read, returns NOF bytes readible,
unsigned int buffer_read (const struct buffer* this_buffer, void** out_buffer_p, const unsigned int off);

//get nof readable bytes
unsigned int buffer_get_n_bytes_readable (const struct buffer* this_buffer);

//resets the buffer, pointers from read are still valied, but may contain outdated data
void buffer_reset (struct buffer* this_buffer);
//wait for writers to finish
void buffer_sync (struct buffer* this_buffer);

buffer_t* get_buffer(const unsigned int size);

void put_buffer(buffer_t* buf);

#endif // !KERNELMODULE_PCM3060_UTILS_BUFFER_H