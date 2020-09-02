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
#define DEVICETREE_PCM3060_SCK_PWM_NAME "pwm_pcm3060"
#define DEVICETREE_PCM3060_SPI_NAME "spi_pcm3060"

struct device* dt_find_pcm3060_device(void);

#endif /* IS_ENABLED(CONFIG_OF) */
#endif // !KERNELMODULE_PCM3060_PERIPHERY_DEVICETREE_H