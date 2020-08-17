/** @file pcm3060
 *  @brief definition of pcm3060 device
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_PERIPHERY_PCM3060_H
#define KERNELMODULE_PCM3060_PERIPHERY_PCM3060_H

#include <linux/types.h>

typedef struct pcm3060_config
{
    u32 sck_f; // frequency for sck in Hz
} pcm3060_config_t;

typedef struct pcm3060
{
    //write
    //read
    int (*init) (const pcm3060_config_t* const cfg);
} pcm3060_t;

//alloacate and setup the struct
pcm3060_t* get_pcm3060(void);

void put_pcm3060(pcm3060_t* dev_pcm3060);


#endif // !KERNELMODULE_PCM3060_PERIPHERY_PCM3060_H