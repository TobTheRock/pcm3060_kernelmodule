
#ifndef KERNELMODULE_PCM3060_CONFIG_H
#define KERNELMODULE_PCM3060_CONFIG_H

/*-------------------------------- Settings ------------------------------------*/

#define CONFIG_PLATFORM_HK_ODROID_N2 // Platform to be used

//#define CONFIG_MASTER_GENERATE_CLK 1 // Master platform: Generate clock signals (SCK,BCK,LRCK) for the PCM3060

#define CONFIG_LOGGING_NAME "mod_pcm3060"
#define CONFIG_DEBUG_LOGGING_ENABLE

#define CONFIG_NCHANNELS 2

/*-------------------------------- Clock Settings (ADC=DAC) ------------------------------------*/
#define CONFIG_DEFAULT_FS_HZ  44100// Sampling frequency from 16 to 192 KHz
#define CONFIG_IS_VALID_FS(fs_hz) (((fs_hz) >= 16000) && ((fs_hz) <= 192000))

#define CONFIG_RATIO_SCK_FS_HZ 128 //Sck1 must be multiples of sampling frequency:  128/192/256/384/512/768 fS
#define CONFIG_RATIO_BCK_FS_HZ 64 //Sck1 must be multiples of sampling frequency:  48/64 fS
#define CONFIG_RATIO_LRCK_FS_HZ 2 // TODO: Check

#define CONFIG_GET_CLOCK_SCK_F_HZ(fs) ((fs)*(CONFIG_RATIO_SCK_FS_HZ))  // ADC SCK1 clock speed in Hz
#define CONFIG_GET_CLOCK_BCK_F_HZ(fs) ((fs)*(CONFIG_RATIO_BCK_FS_HZ))  // ADC BCK clock speed in Hz
#define CONFIG_GET_CLOCK_LRCK_F_HZ(fs) ((fs)*(CONFIG_RATIO_LRCK_FS_HZ))

/*-------------------------------- Treansceiver Settings ------------------------------------*/
/* Available formats
24-bit I2S format
24-bit left-justified format -> that's what we use for now
24-bit right-justified format
16-bit right-justified format
 */
#define CONFIG_N_BIT_PER_TX 24 // TODO check how this works with SPU
#define CONFIG_N_BYTE_SIZE_PER_TX (1 + (((CONFIG_N_BIT_PER_TX) - 1) / (8)))


#define CONFIG_DEFAULT_BUF_SIZE 512 // buf size of each duplex buffer

/*-------------------------------- Platform Settings ------------------------------------*/

// #ifdef (CONFIG_PLATFORM_HK_ODROID_N2)
//     #include <config/config_hk_odroid_n2.h>
// #else
// #error Unkown Platform!
// #endif // (PLATFORM == HK_ODROID_N2)

#endif // KERNELMODULE_PCM3060_CONFIG_H