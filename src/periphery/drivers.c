#include "drivers.h"
#include "devicetree.h"
#include <utils/logging.h>

#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>


static struct
{
    t_onProbe_cb*  onProbeCallbacks;
    unsigned int nOnProbeCallbacks;
    t_onRemove_cb* onRemoveCallbacks;
    unsigned int nOnRemoveCallbacks;
} _pcm3060_driver_i = {NULL,0,NULL,0};

static int probe_pcm3060_device(struct platform_device *pdev)
{
    int ret = 0;
    unsigned int i;
    INFO("Initializing pcm3060 device with %d callbacks", _pcm3060_driver_i.nOnProbeCallbacks);
    for (i = 0; i < _pcm3060_driver_i.nOnProbeCallbacks; ++i)
    {
        t_onProbe_cb onProbe_cb =  _pcm3060_driver_i.onProbeCallbacks[i];
        ret = (onProbe_cb != NULL) && (onProbe_cb( &(pdev->dev)) );
        if (ret)
        {
            ERROR("Failed pcm3060 init");
            break;
        }
    }
    
    return ret;
}

static int remove_pcm3060_device(struct platform_device *pdev)
{
    int ret = 0;
    unsigned int i;
    INFO("Removing pcm3060 device with %d callbacks",  _pcm3060_driver_i.nOnRemoveCallbacks);
    for (i = 0; i <  _pcm3060_driver_i.nOnRemoveCallbacks; ++i)
    {
        t_onRemove_cb onRemove_cb =  _pcm3060_driver_i.onRemoveCallbacks[i];
        ret &= (onRemove_cb != NULL) && (onRemove_cb( &(pdev->dev)) );
    }

    if (ret)
    {
        ERROR("Failed pcm3060 remove!");
        // break;
    }
    
    return ret;
}

#ifdef CONFIG_OF
static struct of_device_id pcm3060_device_matchtable[] = {
     {
             .compatible = DEVICETREE_PCM3060_COMPATIBLE,
     },
     {},
};
#endif // CONFIG_OF

MODULE_DEVICE_TABLE(of, pcm3060_device_matchtable);

static struct platform_driver driv_ext_pcm3060 = {
    .probe = probe_pcm3060_device,
    .remove = remove_pcm3060_device,
    .driver = {
        .name = DEV_PCM3060_DRIVERNAME,
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(pcm3060_device_matchtable),
    },
};

int register_driver_pcm3060_plain(void)
{
    return platform_driver_register(&driv_ext_pcm3060);
}

int register_driver_pcm3060(t_onProbe_cb* callbacks, const unsigned int nCallbacks)
{
    unsigned int it;
    INFO("Registering PCM3060 device driver");
    if (callbacks)
    {
        _pcm3060_driver_i.nOnProbeCallbacks = nCallbacks;
         _pcm3060_driver_i.onProbeCallbacks = kmalloc(_pcm3060_driver_i.nOnProbeCallbacks*sizeof(t_onProbe_cb), GFP_KERNEL);
        for (it = 0; it < _pcm3060_driver_i.nOnProbeCallbacks; it++)
        {
             _pcm3060_driver_i.onProbeCallbacks[it] = callbacks[it];
        }
    }
    
    return register_driver_pcm3060_plain();
}

void unregister_driver_pcm3060(t_onRemove_cb* callbacks, const unsigned int nCallbacks)
{
    unsigned int it;
    INFO("Unregistering PCM3060 device driver");

    if (callbacks)
    {
         _pcm3060_driver_i.nOnRemoveCallbacks = nCallbacks;
         _pcm3060_driver_i.onRemoveCallbacks = kmalloc(_pcm3060_driver_i.nOnRemoveCallbacks*sizeof(t_onRemove_cb), GFP_KERNEL);
        for (it = 0; it < _pcm3060_driver_i.nOnRemoveCallbacks; it++)
        {
             _pcm3060_driver_i.onRemoveCallbacks[it] = callbacks[it];
        }
    }
    platform_driver_unregister(&driv_ext_pcm3060);
}

void unregister_driver_pcm3060_plain(void)
{
    INFO("Unregistering PCM3060 device driver");
    unregister_driver_pcm3060(NULL,0);
}