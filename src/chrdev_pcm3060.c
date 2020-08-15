#include "chrdev_pcm3060.h"
#include <utils/logging.h>
#include <periphery/pcm3060.h>

#include <linux/cdev.h>
// #include <linux/kdev_t.h>
#include <linux/fs.h>

#define CHRDEV_PCM3060_MINOR_START 0

typedef struct _chrdev_pcm3060_data
{
    struct cdev cdev;
    pcm3060* pcm3060_p;
} _chrdev_pcm3060_data_t;

static _chrdev_pcm3060_data_t _chrdev_pcm3060_data_array[CHRDEV_PCM3060_MAX_DEVICES];

static int _chrdev_pcm3060_dev_major = 0;
static struct class *_chrdev_pcm3060_class = NULL;

// static int my_read(struct file *file, char __user *user_buffer,
//                    size_t size, loff_t *offset)
// {
//     struct my_device_data *my_data = (struct my_device_data *) file->private_data;
//     ssize_t len = min(my_data->size - *offset, size);

//     if (len <= 0)
//         return 0;

//     /* read data from my_data->buffer to user buffer */
//     if (copy_to_user(user_buffer, my_data->buffer + *offset, len))
//         return -EFAULT;

//     *offset += len;
//     return len;
// }

static int chrdev_pcm3060_open(struct inode *, struct file *);
{
    _chrdev_pcm3060_data_t *pcm3060_data =
             container_of(inode->i_cdev, _chrdev_pcm3060_data_t, cdev);

    /* validate access to device */
    file->private_data = pcm3060_data;

    /* initialize pcm3060 */

}

const struct file_operations chrdev_pcm3060_fops = {
    .owner = THIS_MODULE,
    .open = chrdev_pcm3060_open
    // .read = my_read,
    // .write = my_write,
    // .release = my_release,
    // .unlocked_ioctl = my_ioctl
};


int chrdev_pcm3060_register(const char* dev_name)
{
    unsigned int i;
    
    if( (alloc_chrdev_region(&dev, CHRDEV_PCM3060_MINOR_START, CHRDEV_PCM3060_MAX_DEVICES, dev_name)) < 0)
    {
            INFO("Cannot allocate major number for device 1\n");
            return -1;
    }
    _chrdev_pcm3060_dev_major = MAJOR(dev);

    INFO("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    // create sysfs class
    _chrdev_pcm3060_class = class_create(THIS_MODULE, dev_name);

    // Create necessary number of the devices
    for (i = 0; i < CHRDEV_PCM3060_MAX_DEVICES; i++)
    {
        // init new device
        cdev_init(&_chrdev_pcm3060_data_array[i].cdev, &chrdev_pcm3060_fops);
        _chrdev_pcm3060_data_array[i].cdev.owner = THIS_MODULE;

        // add device to the system where "i" is a Minor number of the new device
        cdev_add(&_chrdev_pcm3060_data_array[i].cdev, MKDEV(_chrdev_pcm3060_dev_major, i), 1);

        // create device node /dev/mychardev-x where "x" is "i", equal to the Minor number
        device_create(_chrdev_pcm3060_class, NULL, MKDEV(_chrdev_pcm3060_dev_major, i), NULL, "%s-%d", dev_name, i);
    }

    return 0;
}

void chrdev_pcm3060_unregister(void)
{
    int i;

    for (i = 0; i < CHRDEV_PCM3060_MAX_DEVICES; i++)
    {
        device_destroy(_chrdev_pcm3060_class, MKDEV(_chrdev_pcm3060_dev_major, i));
    }

    class_unregister(_chrdev_pcm3060_class);
    class_destroy(_chrdev_pcm3060_class);

    unregister_chrdev_region(MKDEV(_chrdev_pcm3060_dev_major, 0), MINORMASK);
}
