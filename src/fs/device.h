/** @file device
 *  @brief Create a device file
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_FS_DEVICE_H
#define KERNELMODULE_PCM3060_FS_DEVICE_H

#define FS_DEVICE_NAME "PCM3060_DEV"

    int fs_device_register(void);
    void fs_device_unregister(void);

#endif // !KERNELMODULE_PCM3060_FS_DEVICE_H