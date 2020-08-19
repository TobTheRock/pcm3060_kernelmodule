/** @file devicetree.h
 *  @brief device tree definitons
 *
 *  @author Tobias Waurick
 */

#ifndef KERNELMODULE_PCM3060_PERIPHERY_DEVICETREE_H
#define KERNELMODULE_PCM3060_PERIPHERY_DEVICETREE_H

#include <linux/kconfig.h>

#if IS_ENABLED(CONFIG_OF)
#include <linux/device.h>

/*-------------------------------- Device Tree Settings ------------------------------------*/
#define DEVICETREE_PCM3060_COMPATIBLE "ext_pcm3060"
#define DEVICETREE_PCM3060_SCK2_PWMNAME "pwm_pcm3060_sck2"
#define DEVICETREE_PCM3060_SCK1_PWMNAME "pwm_pcm3060_sck1"

struct device* dt_find_pcm3060_device(void);

#endif /* IS_ENABLED(CONFIG_OF) */
#endif // !KERNELMODULE_PCM3060_PERIPHERY_DEVICETREE_H