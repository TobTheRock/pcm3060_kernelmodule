#include "device.h"
#include <utils/logging.h>

#include <linux/cdev.h>
// #include <linux/kdev_t.h>
#include <linux/fs.h>

#define MINOR_START 0
#define DEV_CNT 1

struct cdev cdev;




int fs_device_register(void)
{
    // if( (alloc_chrdev_region(&dev, MINOR_START, DEV_CNT, FS_DEVICE_NAME)) < 0)
    // {
            // INFO("Cannot allocate major number for device 1\n");
            // return -1;
    // }
    // INFO("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
    return 0;
}

void fs_device_unregister(void)
{
    // unregister_chrdev_region(dev, DEV_CNT);
}

// const struct file_operations my_fops = {
//     .owner = THIS_MODULE,
//     .open = my_open,
//     .read = my_read,
//     .write = my_write,
//     .release = my_release,
//     .unlocked_ioctl = my_ioctl
// };
