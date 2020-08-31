/** @file duplex_pipe_buffer.h
 *  @brief wrapper fur duplex pipe buffers
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_DUPLEX_PIPE_BUFFER_H
#define KERNELMODULE_PCM3060_DUPLEX_PIPE_BUFFER_H

struct _duplex_duplex_pipe_buffer_impl;
typedef struct duplex_pipe_end duplex_pipe_end_t;

typedef struct duplex_pipe_buffer
{
    const unsigned int size;
    struct duplex_pipe_end* left_end,* right_end;
    struct _duplex_duplex_pipe_buffer_impl*const _impl_p;
} duplex_pipe_buffer_t;

duplex_pipe_buffer_t* get_duplex_pipe_buffer(const unsigned int size);
void put_duplex_pipe_buffer(duplex_pipe_buffer_t* buf);

unsigned int duplex_pipe_end_copy_from_user(struct duplex_pipe_end* duplex_pipe_end, const void* buffer_ext, const unsigned int buflen);
unsigned int duplex_pipe_end_copy_to_user(const struct duplex_pipe_end* duplex_pipe_end, void* buffer_ext, const unsigned int buflen, const unsigned int off);

// READER

unsigned int duplex_pipe_end_n_bytes_available(const duplex_pipe_end_t* duplex_pipe_end);
// start reading the next available bytes, return NOF bytes readable, sets pointer to memory region
unsigned int duplex_pipe_end_read_start(const duplex_pipe_end_t* duplex_pipe_end, void** o_buffer_ptr);
//waits for data:
unsigned int duplex_pipe_end_read_start_waiting(const duplex_pipe_end_t* duplex_pipe_end, void** o_buffer_ptr);
//terminate reading
void duplex_pipe_end_read_end(const duplex_pipe_end_t* duplex_pipe_end);

//WRITER
//
unsigned int duplex_pipe_end_write_start(duplex_pipe_end_t* duplex_pipe_end, void** o_buffer_ptr, const unsigned int n_bytes_requested);
//terminate writing
void duplex_pipe_end_write_end(duplex_pipe_end_t* duplex_pipe_end);


#endif // !KERNELMODULE_PCM3060_DUPLEX_PIPE_BUFFER_H