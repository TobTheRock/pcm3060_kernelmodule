/** @file pcm3060
 *  @brief definition of pcm3060 device
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_PERIPHERY_PCM3060_H
#define KERNELMODULE_PCM3060_PERIPHERY_PCM3060_H

#include <linux/types.h>
#include <utils/duplex_ring_buffer.h>

#define PCM3060_CHANNEL_ID_LEFT 0
#define PCM3060_CHANNEL_ID_RIGHT 1

typedef struct pcm3060_config
{
    u32 fs; // sample rate frequency in Hz
    u32 buf_size; // i/o buffer size in byte
} pcm3060_config_t;

typedef struct pcm3060
{
    int (*init) (const pcm3060_config_t* const cfg);
    duplex_ring_end_t* (*get_channel_buffer_end) (unsigned int channel);
} pcm3060_t;

//alloacate and setup the struct
pcm3060_t* get_pcm3060(void);

void put_pcm3060(pcm3060_t* dev_pcm3060);


#endif // !KERNELMODULE_PCM3060_PERIPHERY_PCM3060_H