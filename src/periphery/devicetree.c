#include "devicetree.h"

#if IS_ENABLED(CONFIG_OF)

#include <utils/logging.h>

#include <linux/version.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
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

int get_gpio(struct device *dev, const char * name, int * pGPIOnum_o)
{
    int n_pins,pin_id_it;
    int err = 0;
    struct device_node * pDN  = dev->of_node;
    TRACE("Searching gpio named %s", name);

    *pGPIOnum_o = 0;

    // parse pins
    n_pins = of_gpio_named_count(pDN, name);
    if (n_pins <= 0)
    {
        TRACE("no gpios found");
        return -1;
    }

    for (pin_id_it = 0; pin_id_it < n_pins; pin_id_it++)
    {
        // get pin number
        *pGPIOnum_o = of_get_named_gpio(pDN,name, pin_id_it);
        if (*pGPIOnum_o == -EPROBE_DEFER)
        {
            return err;
        }
        // check if pin number is valid
        if (gpio_is_valid(*pGPIOnum_o))
        {
            // yes
            // request pin
            if ( (err = devm_gpio_request(dev, *pGPIOnum_o, name)) )
            {
                return err;
            }
            break;
        }
    }
    return err;
}

#endif