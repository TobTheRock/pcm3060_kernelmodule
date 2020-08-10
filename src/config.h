#ifndef KERNELMODULE_PCM3060_CONFIG_H
#define KERNELMODULE_PCM3060_CONFIG_H

/*-------------------------------- Settings ------------------------------------*/

#define CONFIG_PLATFORM_HK_ODROID_N2 // Platform to be used

#define CONFIG_MASTER_GENERATE_CLK 1 // Master platform: Generate clock signals (SCK,BCK,LRCK) for the PCM3060

#define CONFIG_LOGGING_NAME "mod_pcm3060"
#define CONFIG_DEBUG_LOGGING_ENABLE

/*-------------------------------- ADC Settings ------------------------------------*/
#define CONFIG_ADC_FS_HZ  1// Sampling frequency from 16 to 192 KHz
#define CONFIG_ADC_RATIO_SCK1_FS_HZ 128 //Sck1 must be multiples of sampling frequency:  128/192/256/384/512/768 fS
#define CONFIG_ADC_RATIO_BCK1_FS_HZ 64 //Sck1 must be multiples of sampling frequency:  48/64 fS
#define CONFIG_ADC_RATIO_BCK1_LRCK_HZ 2 // TODO: Check

#define CONFIG_ADC_CLOCK_SCK1_F_HZ ((CONFIG_ADC_FS_HZ)*(CONFIG_ADC_RATIO_SCK1_FS_HZ))  // ADC SCK1 clock speed in Hz
#define CONFIG_ADC_CLOCK_BCK1_F_KHZ ((CONFIG_ADC_FS_HZ)*(CONFIG_ADC_RATIO_SCK1_FS_HZ))  // ADC BCK clock speed in Hz
#define CONFIG_ADC_CLOCK_LRCK1_F_KHZ ((CONFIG_ADC_RATIO_BCK1_LRCK_HZ)*(CONFIG_ADC_CLOCK_BCK1_F_KHZ))

/*-------------------------------- DAC Settings ------------------------------------*/
//#define CONFIG_DAC_FS_HZ  44100// Sampling frequency from 16 to 192 KHz
#define CONFIG_DAC_FS_HZ  1// Sampling frequency from 16 to 192 KHz
#define CONFIG_DAC_RATIO_SCK2_FS_HZ 128 //Sck2 must be multiples of sampling frequency:  128/192/256/384/512/768 fS
#define CONFIG_DAC_RATIO_BCK2_FS_HZ 64 //Sck2 must be multiples of sampling frequency:  48/64 fS
#define CONFIG_DAC_RATIO_BCK2_LRCK_HZ 2 // TODO: Check

#define CONFIG_DAC_CLOCK_SCK2_F_HZ ((CONFIG_DAC_FS_HZ)*(CONFIG_DAC_RATIO_SCK2_FS_HZ))  // DAC SCK2 clock speed in Hz
#define CONFIG_DAC_CLOCK_BCK2_F_KHZ ((CONFIG_DAC_FS_HZ)*(CONFIG_DAC_RATIO_SCK2_FS_HZ))  // DAC BCK clock speed in Hz
#define CONFIG_DAC_CLOCK_LRCK2_F_KHZ ((CONFIG_DAC_RATIO_BCK2_LRCK_HZ)*(CONFIG_DAC_CLOCK_BCK2_F_KHZ))


/*-------------------------------- Device Tree Settings ------------------------------------*/
#define CONFIG_DT_PCM3060_COMPATIBLE "ext_pcm3060"
#define CONFIG_DT_PCM3060_SCK2_PWMNAME "pwm_pcm3060_sck2"
#define CONFIG_DT_PCM3060_SCK1_PWMNAME "pwm_pcm3060_sck1"


/*-------------------------------- Platform Settings ------------------------------------*/

// #ifdef (CONFIG_PLATFORM_HK_ODROID_N2)
//     #include <config/config_hk_odroid_n2.h>
// #else
// #error Unkown Platform!
// #endif // (PLATFORM == HK_ODROID_N2)

#endif // KERNELMODULE_PCM3060_CONFIG_H