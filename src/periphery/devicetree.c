#include "devicetree.h"

#if IS_ENABLED(CONFIG_OF)

#include <utils/logging.h>

#include <linux/version.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <utils/devicetree.h>


struct device* dt_find_pcm3060_device(void)
{

    struct device_node* dev_node;
    struct device *dev = NULL;
    TRACE("");

    dev_node = of_find_compatible_node(NULL, NULL, DEVICETREE_PCM3060_COMPATIBLE);
    if (dev_node == NULL)
    {
        ERROR("Device node with compatible property: %s not found in device tree", DEVICETREE_PCM3060_COMPATIBLE);
    }
    else
    {
        TRACE("Found device node");
        if ((dev = bus_find_device(&platform_bus_type, NULL, dev_node, &device_match_of_node)) == NULL)
        {
            ERROR("Device not found on platform bus!");
        }
    }
    return dev;
}

#endif /* IS_ENABLED(CONFIG_OF) */