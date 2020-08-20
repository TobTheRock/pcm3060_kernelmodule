#include "chrdev_pcm3060.h"
#include <utils/logging.h>
#include <periphery/pcm3060.h>

#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>

#define CHRDEV_PCM3060_MINOR_START 0

typedef struct _chrdev_pcm3060_data
{
    struct cdev cdev;
    pcm3060_t* pcm3060;
} _chrdev_pcm3060_data_t;

static _chrdev_pcm3060_data_t _chrdev_pcm3060_data_array[CHRDEV_PCM3060_MAX_DEVICES];


static dev_t _chrdev_pcm3060_dev = 0;
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

static int chrdev_pcm3060_open(struct inode * node, struct file * file)
{
    TRACE("");
    // TODO read from SYS CTL
    pcm3060_config_t cfg = {
        .sck_f =CONFIG_ADC_FS_HZ,
        .buf_size = 10
        };
    _chrdev_pcm3060_data_t* pcm3060_data;
    TRACE("");

    if ( (node == NULL) || (file == NULL) )
    {
        ERROR("Invalid node/file pointer");
        return -1;
    }

    pcm3060_data = container_of(node->i_cdev, _chrdev_pcm3060_data_t, cdev);
    file->private_data = pcm3060_data;

    /* initialize pcm3060 */
    if ( (pcm3060_data->pcm3060 = get_pcm3060()) == NULL)
    {
        ERROR("Failed to get PCM3060");
        goto r_fail;
    }

    return pcm3060_data->pcm3060->init(&cfg); // TODO check this and release in case of error

    // r_pcm:
    //     put_pcm3060(pcm3060_data->pcm3060);
    r_fail:
        return -1;
}
static int chrdev_pcm3060_release(struct inode *node, struct file *file)
{
    TRACE("");
    if (file->private_data)
    {
        _chrdev_pcm3060_data_t *pcm3060_data = (_chrdev_pcm3060_data_t*) file->private_data;
        if (pcm3060_data->pcm3060 == NULL)
        {
            ERROR("No pcm3060 struct!");
        }
        else
        {
            put_pcm3060(pcm3060_data->pcm3060);
        }
        
    }

    // kfree(f->private_data);

    return 0;
}


static long chrdev_pcm3060_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    TRACE("");
    return 0;
}

static ssize_t chrdev_pcm3060_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    ssize_t ret = 0;
    TRACE("");
    if (file->private_data)
    {
        _chrdev_pcm3060_data_t* pcm3060_data = (_chrdev_pcm3060_data_t*) file->private_data;
        if (pcm3060_data->pcm3060 == NULL)
        {
            ERROR("No pcm3060 struct!");
        }
        else if (pcm3060_data->pcm3060->input_buffer == NULL)
        {
            ERROR("No input buffer!");
        }
        else
        {
            ret = pcm3060_data->pcm3060->input_buffer->copy_to_user(pcm3060_data->pcm3060->input_buffer, buf, count, 0); // todo this must be an output buffer
            *offset += ret;
        }
    }


    return ret;
}

static ssize_t chrdev_pcm3060_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    ssize_t ret = 0;
    TRACE("");
    if (file->private_data)
    {
        _chrdev_pcm3060_data_t* pcm3060_data = (_chrdev_pcm3060_data_t*) file->private_data;
        if (pcm3060_data->pcm3060 == NULL)
        {
            ERROR("No pcm3060 struct!");
        }
        else if (pcm3060_data->pcm3060->input_buffer == NULL)
        {
            ERROR("No output buffer!");
        }
        else if (pcm3060_data->pcm3060->input_buffer->write_from_user(pcm3060_data->pcm3060->input_buffer, buf, count))
        {
            ERROR("Writing to input wring buffer failed!");
        }
        else
        {
            ret = count;
            *offset += count;
        }
    }


    return ret;
}

const struct file_operations chrdev_pcm3060_fops = {
    .owner = THIS_MODULE,
    .open = chrdev_pcm3060_open,
    .read = chrdev_pcm3060_read,
    .write = chrdev_pcm3060_write,
    .release = chrdev_pcm3060_release,
    .unlocked_ioctl = chrdev_pcm3060_ioctl
};


int chrdev_pcm3060_register(const char* dev_name)
{
    unsigned int i;
    
    if( (alloc_chrdev_region(&_chrdev_pcm3060_dev, CHRDEV_PCM3060_MINOR_START, CHRDEV_PCM3060_MAX_DEVICES, dev_name)) < 0)
    {
            INFO("Cannot allocate major number for device 1\n");
            return -1;
    }

    INFO("Major = %d Minor = %d \n",MAJOR(_chrdev_pcm3060_dev), MINOR(_chrdev_pcm3060_dev));

    // create sysfs class
    _chrdev_pcm3060_class = class_create(THIS_MODULE, dev_name);

    // Create necessary number of the devices
    for (i = 0; i < CHRDEV_PCM3060_MAX_DEVICES; i++)
    {
        // init new device
        cdev_init(&_chrdev_pcm3060_data_array[i].cdev, &chrdev_pcm3060_fops);
        _chrdev_pcm3060_data_array[i].cdev.owner = THIS_MODULE;

        // add device to the system where "i" is a Minor number of the new device
        cdev_add(&_chrdev_pcm3060_data_array[i].cdev, MKDEV(MAJOR(_chrdev_pcm3060_dev), i), 1);

        // create device node /dev/mychardev-x where "x" is "i", equal to the Minor number
        device_create(_chrdev_pcm3060_class, NULL, MKDEV(MAJOR(_chrdev_pcm3060_dev), i), NULL, "%s-%d", dev_name, i);
    }

    return 0;
}

void chrdev_pcm3060_unregister(void)
{
    int i;
    TRACE("");

    for (i = 0; i < CHRDEV_PCM3060_MAX_DEVICES; i++)
    {
        device_destroy(_chrdev_pcm3060_class, MKDEV(MAJOR(_chrdev_pcm3060_dev), i));
    }

    class_unregister(_chrdev_pcm3060_class);
    class_destroy(_chrdev_pcm3060_class);

    unregister_chrdev_region(MKDEV(MAJOR(_chrdev_pcm3060_dev), 0), MINORMASK);
}
