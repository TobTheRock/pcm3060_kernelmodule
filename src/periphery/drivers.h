/** @file devices.h
 *  @brief
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_PERIPHERY_DRIVERS_H
#define KERNELMODULE_PCM3060_PERIPHERY_DRIVERS_H

#include <linux/platform_device.h>

#define DEV_PCM3060_DRIVERNAME "driv_ext_pcm3060"

typedef int (*t_onProbe_cb) (struct device *pdev);
typedef int (*t_onRemove_cb) (struct device *pdev);

int register_driver_pcm3060_plain(void);
int register_driver_pcm3060(t_onProbe_cb* callbacks, const unsigned int nCallbacks);
void unregister_driver_pcm3060(t_onRemove_cb* callbacks, const unsigned int nCallbacks);
void unregister_driver_pcm3060_plain(void);

#endif // !KERNELMODULE_PCM3060_PERIPHERY_DRIVERS_H