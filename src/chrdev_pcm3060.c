#include "chrdev_pcm3060.h"
#include <utils/logging.h>
#include <utils/ptr.h>
#include <periphery/pcm3060.h>

#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>

#define CHRDEV_PCM3060_MINOR_START 0

typedef struct _chrdev_pcm3060_file_data
{
    struct cdev cdev;
    unsigned int channel;
    pcm3060_t* pcm3060;
} _chrdev_pcm3060_file_data_t;

typedef struct _chrdev_pcm3060_dev_data
{
    pcm3060_config_t pcm3060_cfg;
} _chrdev_pcm3060_dev_data_t;

static ssize_t nchannels_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
    // struct chrdev_device *chrdev = dev_get_drvdata(dev);

    // return sprintf(buf, "%d\n", chrdev->id);
     return sprintf(buf, "BLA\n");
}
static DEVICE_ATTR_RO(nchannels);


static struct attribute *_chrdev_pcm3060_attrs[] = {
    &dev_attr_nchannels.attr,
    NULL,
};

static const struct attribute_group _chrdev_pcm3060_group = {
    .attrs = _chrdev_pcm3060_attrs,
};

static const struct attribute_group *_chrdev_pcm3060_groups[] = {
    &_chrdev_pcm3060_group,
    NULL,
};

static _chrdev_pcm3060_file_data_t _chrdev_pcm3060_file_data_array[CONFIG_NCHANNELS];


static dev_t _chrdev_pcm3060_dev = 0;
static struct class *_chrdev_pcm3060_class = NULL;

static int chrdev_pcm3060_open(struct inode * node, struct file * file)
{
    // TODO read from SYS CTL
    pcm3060_config_t cfg = {
        .sck_f =CONFIG_ADC_FS_HZ,
        .buf_size = 1000
        };
    _chrdev_pcm3060_file_data_t* pcm3060_data;
    TRACE("");
    RETURN_ON_NULL(node, -1);
    RETURN_ON_NULL(file, -1);

    pcm3060_data = container_of(node->i_cdev, _chrdev_pcm3060_file_data_t, cdev);
    RETURN_ON_NULL(pcm3060_data, -1);

    TRACE("Opened channel %d", pcm3060_data->channel);
    file->private_data = pcm3060_data;
    /* initialize pcm3060 */
    if ( (pcm3060_data->pcm3060 = get_pcm3060()) == NULL)
    {
        ERROR("Failed to get PCM3060");
        goto r_fail;
    }

    return pcm3060_data->pcm3060->init(&cfg); // TODO check this and release in case of error

    r_fail:
        return -1;
}
static int chrdev_pcm3060_release(struct inode *node, struct file *file)
{
    TRACE("");
    if (file->private_data)
    {
        _chrdev_pcm3060_file_data_t *pcm3060_data = (_chrdev_pcm3060_file_data_t*) file->private_data;
        if (pcm3060_data->pcm3060 == NULL)
        {
            ERROR("No pcm3060 struct!");
        }
        else
        {
            put_pcm3060(pcm3060_data->pcm3060);
        }
    }
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
    TRACE("Reading %ld bytes from offset offset", count, *offset);
    if (file->private_data)
    {
        _chrdev_pcm3060_file_data_t* pcm3060_data = (_chrdev_pcm3060_file_data_t*) file->private_data;
        duplex_pipe_end_t* pipe_end;
        if (pcm3060_data->pcm3060 == NULL)
        {
            ERROR("No pcm3060 struct!");
        }
        else if ( (pipe_end = pcm3060_data->pcm3060->get_channel_buffer_end(pcm3060_data->channel)) == NULL)
        {
            ERROR("No buffer for channel %d!", pcm3060_data->channel);
        }
        else
        {
            ret = duplex_pipe_end_copy_to_user(pipe_end, buf, count, *offset);
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
        _chrdev_pcm3060_file_data_t* pcm3060_data = (_chrdev_pcm3060_file_data_t*) file->private_data;
        duplex_pipe_end_t* pipe_end;
        if (pcm3060_data->pcm3060 == NULL)
        {
            ERROR("No pcm3060 struct!");
        }
        else if ( (pipe_end = pcm3060_data->pcm3060->get_channel_buffer_end(pcm3060_data->channel)) == NULL)
        {
            ERROR("No buffer for channel %d!", pcm3060_data->channel);
        }
        else if (duplex_pipe_end_copy_from_user(pipe_end, buf, count))
        {
            ERROR("Writing to input buffer failed!");
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
    
    if( (alloc_chrdev_region(&_chrdev_pcm3060_dev, CHRDEV_PCM3060_MINOR_START, CONFIG_NCHANNELS, dev_name)) < 0)
    {
            INFO("Cannot allocate major number for device 1\n");
            return -1;
    }

    INFO("Major = %d Minor = %d \n",MAJOR(_chrdev_pcm3060_dev), MINOR(_chrdev_pcm3060_dev));

    // create sysfs class
    _chrdev_pcm3060_class = class_create(THIS_MODULE, dev_name);
    _chrdev_pcm3060_class->dev_groups = _chrdev_pcm3060_groups;

    // Create necessary number of the devices
    for (i = 0; i < CONFIG_NCHANNELS; i++)
    {
        // init new device
        cdev_init(&_chrdev_pcm3060_file_data_array[i].cdev, &chrdev_pcm3060_fops);
        _chrdev_pcm3060_file_data_array[i].cdev.owner = THIS_MODULE;
        _chrdev_pcm3060_file_data_array[i].channel = i;

        // add device to the system where "i" is a Minor number of the new device
        cdev_add(&_chrdev_pcm3060_file_data_array[i].cdev, MKDEV(MAJOR(_chrdev_pcm3060_dev), i), 1);

        // create device node /dev/mychardev-x where "x" is "i", equal to the Minor number
        device_create(_chrdev_pcm3060_class, NULL, MKDEV(MAJOR(_chrdev_pcm3060_dev), i), NULL, "%s-%d", dev_name, i);
    }

    return 0;
}

void chrdev_pcm3060_unregister(void)
{
    int i;
    TRACE("");

    for (i = 0; i < CONFIG_NCHANNELS; i++)
    {
        device_destroy(_chrdev_pcm3060_class, MKDEV(MAJOR(_chrdev_pcm3060_dev), i));
    }

    class_unregister(_chrdev_pcm3060_class);
    class_destroy(_chrdev_pcm3060_class);

    unregister_chrdev_region(MKDEV(MAJOR(_chrdev_pcm3060_dev), 0), MINORMASK);
}
