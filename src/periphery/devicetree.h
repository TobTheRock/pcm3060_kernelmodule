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
#define DEVICETREE_PCM3060_SCK_PWM_NAME "pwm_pcm3060_sck"
#define DEVICETREE_PCM3060_SPI_NAME "spi_pcm3060"
#define DEVICETREE_PCM3060_GPIO_LRCK_NAME "gpio_lrck_pcm3060"

struct device* dt_find_pcm3060_device(void);
// call gpio_free when finished!
int get_gpio(struct device *dev, const char * name, int * pGPIOnum_o);

#endif /* IS_ENABLED(CONFIG_OF) */
#endif // !KERNELMODULE_PCM3060_PERIPHERY_DEVICETREE_H