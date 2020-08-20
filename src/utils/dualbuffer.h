/** @file dualbuffer.h
 *  @brief Buffer with one buffer for reading and one for writting
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_UTILS_DUALBUFFER_H
#define KERNELMODULE_PCM3060_UTILS_DUALBUFFER_H

struct _dualbuffer_impl;


typedef struct dualbuffer
{
    // return NOF bytes dropped/ not copied
    unsigned int (* write) (struct dualbuffer* this_buffer, const void* buffer_ext, const unsigned int buflen);
    unsigned int (* write_from_user) (struct dualbuffer* this_buffer, const void* buffer_ext, const unsigned int buflen);

    // return NOF bytes not copied
    unsigned int (* copy) (const struct dualbuffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off);
    unsigned int (* copy_to_user) (const struct dualbuffer* this_buffer, void* buffer_ext, const unsigned int buflen, const unsigned int off);

    //direct read, returns NOF bytes readible,
    unsigned int (* read) (const struct dualbuffer* this_buffer, void** buffer_ext, const unsigned int off);
    void (* release_read) (const struct dualbuffer* this_buffer, void** buffer_ext);
    
    //get nof readable bytes
    unsigned int (* get_n_bytes_readable) (const struct dualbuffer* this_buffer);

    //resets the dualbuffer, pointers from read are still valied, but may contain outdated data
    void (* reset) (struct dualbuffer* this_buffer);

    struct _dualbuffer_impl* const _impl_p;
} dualbuffer_t;

dualbuffer_t* get_dualbuffer(const unsigned int size);

void put_dualbuffer(dualbuffer_t* buf);

#endif // !KERNELMODULE_PCM3060_UTILS_DUALBUFFER_H