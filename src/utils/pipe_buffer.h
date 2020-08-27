/** @file pipe_buffer.h
 *  @brief pipe data from writter(s) to reader(s)
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_PIPE_BUFFER_H
#define KERNELMODULE_PCM3060_PIPE_BUFFER_H

struct _pipe_buffer_impl;

typedef struct pipe_buffer
{
    const unsigned int size;
    struct _pipe_buffer_impl*const _impl_p;
} pipe_buffer_t;


pipe_buffer_t* get_pipe_buffer(const unsigned int size);
void put_pipe_buffer(pipe_buffer_t* buf);

unsigned int pipe_buffer_copy_from_user(struct pipe_buffer* pipe_buffer, const void* buffer_ext, const unsigned int buflen);
unsigned int pipe_buffer_copy_to_user(const struct pipe_buffer* pipe_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off);

// READER

unsigned int pipe_buffer_n_bytes_available(const pipe_buffer_t* pipe_buffer);
// start reading the next available bytes, return NOF bytes readable, sets pointer to memory region
unsigned int pipe_buffer_read_start(const pipe_buffer_t* pipe_buffer, void** o_buffer_ptr);
//waits for data:
unsigned int pipe_buffer_read_start_waiting(const pipe_buffer_t* pipe_buffer, void** o_buffer_ptr);
//terminate reading
void pipe_buffer_read_end(const pipe_buffer_t* pipe_buffer);

//WRITER
//
unsigned int pipe_buffer_write_start(pipe_buffer_t* pipe_buffer, void** o_buffer_ptr, const unsigned int n_bytes_requested);
//terminate writing
void pipe_buffer_write_end(pipe_buffer_t* pipe_buffer);


#endif // !KERNELMODULE_PCM3060_PIPE_BUFFER_H