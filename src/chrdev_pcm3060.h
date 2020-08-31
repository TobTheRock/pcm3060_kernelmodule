/** @file device
 *  @brief Create a device file
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_CHARDEV_PCM3060__H
#define KERNELMODULE_CHARDEV_PCM3060__H

//#define FS_DEVICE_NAME "PCM3060_DEV"

int chrdev_pcm3060_register(const char* dev_name);
void chrdev_pcm3060_unregister(void);

#endif // !KERNELMODULE_CHARDEV_PCM3060__H