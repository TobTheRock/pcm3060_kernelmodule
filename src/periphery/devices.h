/** @file devices.h
 *  @brief
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_PERIPHERY_DEVICES_HPP
#define KERNELMODULE_PCM3060_PERIPHERY_DEVICES_HPP

#include <linux/platform_device.h>

#define DEV_PCM3060_DRIVERNAME "driv_ext_pcm3060"

typedef int (*t_onProbe_cb) (struct device *pdev);
typedef void (*t_onRemove_cb) (void);

int register_device_pcm3060_plain(void);
int register_device_pcm3060(t_onProbe_cb* callbacks, unsigned int nCallbacks);
void unregister_device_pcm3060(t_onRemove_cb* callbacks, unsigned int nCallbacks);
void unregister_device_pcm3060_plain(void);

#endif // !KERNELMODULE_PCM3060_PERIPHERY_DEVICES_HPP