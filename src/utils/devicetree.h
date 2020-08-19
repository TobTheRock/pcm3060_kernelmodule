
#ifndef KERNELMODULE_PCM3060_UTILS_DEVICETREE_H
#define KERNELMODULE_PCM3060_UTILS_DEVICETREE_H

#include <linux/kconfig.h>
#include <linux/version.h>
#include <linux/device.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,7,9)
int device_match_of_node(struct device *dev, void *np);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(5,7,9) */

#endif // !KERNELMODULE_PCM3060_UTILS_DEVICETREE_H