#include "drivers.h"
#include <utils/logging.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <config.h>

static t_onProbe_cb* onProbeCallbacks;
static unsigned int nOnProbeCallbacks;
static t_onRemove_cb* onRemoveCallbacks;
static unsigned int nOnRemoveCallbacks;

static int probe_pcm3060_device(struct platform_device *pdev)
{
    int ret = 0;
    unsigned int i;
    INFO("Initializing pcm3060 device with %d callbacks", nOnProbeCallbacks);
    for (i = 0; i < nOnProbeCallbacks; ++i)
    {
        t_onProbe_cb onProbe_cb = onProbeCallbacks[i];
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
    INFO("Removing pcm3060 device with %d callbacks", nOnRemoveCallbacks);
    for (i = 0; i < nOnRemoveCallbacks; ++i)
    {
        t_onRemove_cb onRemove_cb = onRemoveCallbacks[i];
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
             .compatible = CONFIG_DT_PCM3060_COMPATIBLE,
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
    return register_device_pcm3060(0,0);
}

int register_driver_pcm3060(t_onProbe_cb* callbacks, const unsigned int nCallbacks)
{
    unsigned int it;
    INFO("Registering PCM3060 device driver");
    if (callbacks)
    {
        nOnProbeCallbacks = nCallbacks;
        onProbeCallbacks = malloc(nOnProbeCallbacks*sizeof(t_onProbe_cb));
        for (it = 0; it < nOnProbeCallbacks; it++)
        {
            onProbeCallbacks = callbacks[it];
        }
    }
    
    return platform_driver_register(&driv_ext_pcm3060);
}

void unregister_driver_pcm3060(t_onRemove_cb* callbacks, const unsigned int nCallbacks)
{
    unsigned int it;
    INFO("Unregistering PCM3060 device driver");

    if (callbacks)
    {
        nOnRemoveCallbacks = nCallbacks;
        onRemoveCallbacks = malloc(nOnRemoveCallbacks*sizeof(t_onProbe_cb));
        for (it = 0; it < nOnRemoveCallbacks; it++)
        {
            onRemoveCallbacks = callbacks[it];
        }
    }
    platform_driver_unregister(&driv_ext_pcm3060);
}

void unregister_driver_pcm3060_plain(void)
{
    INFO("Unregistering PCM3060 device driver");
    platform_driver_unregister(&driv_ext_pcm3060);
}