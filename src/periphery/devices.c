#include "devices.h"
#include <utils/logging.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <config.h>

static t_onProbe_cb* onProbeCallbacks;
static unsigned int nOnProbeCallbacks;

static int probe_pcm3060_device(struct platform_device *pdev)
{
    int ret = 0;
    unsigned int i;
    INFO("Initializing pcm3060 device with %d callbacks", nOnProbeCallbacks);
    for (i = 0; i < nOnProbeCallbacks; ++i)
    {
        t_onProbe_cb onProbe_cb = onProbeCallbacks[i];
        ret = (onProbe_cb != NULL) && (onProbe_cb(pdev));
        if (ret)
        {
            ERROR("Failed pcm3060 init");
            break;
        }
    }
    
    return ret;
}

static int remove_pcm3060_device(struct platform_device *dev)
{
    INFO("Removing  pcm3060 device");
    return 0;
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
int register_device_pcm3060_plain(void)
{
    return register_device_pcm3060(0,0);
}

int register_device_pcm3060(t_onProbe_cb* callbacks, unsigned int nCallbacks)
{
    INFO("Registering PCM3060 device driver");
    onProbeCallbacks = callbacks;
    nOnProbeCallbacks = nCallbacks;
    return platform_driver_register(&driv_ext_pcm3060);
}

void unregister_device_pcm3060(t_onRemove_cb* callbacks, unsigned int nCallbacks)
{
    INFO("Unregistering PCM3060 device driver");
    platform_driver_unregister(&driv_ext_pcm3060);
}

void unregister_device_pcm3060_plain(void)
{
    INFO("Unregistering PCM3060 device driver");
    platform_driver_unregister(&driv_ext_pcm3060);
}