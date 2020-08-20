#include "spi.h"

#include <linux/printk.h>
#include <linux/list.h>
//#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/kconfig.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,7,9)
#if IS_ENABLED(CONFIG_OF)

#include <utils/devicetree.h>
#include <linux/device.h>
/* must call put_device() when done with returned spi_device device */
struct spi_device *of_find_spi_device_by_node(struct device_node *node)
{

	// /* pr_debug */pr_warn("%s(): Searching on bus %s \n", __func__, node->full_name);
	// struct device *dev = bus_find_device_by_name(&spi_bus_type, NULL, node->name);

	struct device *dev = bus_find_device(&spi_bus_type, NULL, node, &device_match_of_node);

	/* pr_debug */pr_warn("%s(): Found dev %p \n", __func__, dev);

	return dev ? to_spi_device(dev) : NULL;
}
// EXPORT_SYMBOL_GPL(of_find_spi_device_by_node);
#else
struct spi_device *of_find_spi_device_by_node(struct device_node *node)
{
	return NULL;
}
#endif /* IS_ENABLED(CONFIG_OF) */
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(5,7,9) */

struct spi_device *of_spi_get(struct device_node *np, const char *con_id);

struct spi_device *spi_get(struct device *dev, const char *con_id)
{
	struct spi_device *spi = ERR_PTR(-EPROBE_DEFER);
	const char *dev_id = dev ? dev_name(dev) : NULL;

	/* look up via DT first */
	if (IS_ENABLED(CONFIG_OF) && dev && dev->of_node)
		return of_spi_get(dev->of_node, con_id);

	return spi;
}


struct spi_device *of_spi_get(struct device_node *np, const char *con_id)
{
	// struct spi_device *spi = NULL;
	struct of_phandle_args args;
	struct spi_device *sdev;
	int index = 0;
	int err;

	/* pr_debug */pr_warn("%s(): \n", __func__);

	if (con_id) {

	/* pr_debug */pr_warn("%s(): conid \n", __func__);
		index = of_property_match_string(np, "spi-names", con_id);
		if (index < 0){
			/* pr_debug */pr_warn("%s(): pr match fail \n", __func__);
			return ERR_PTR(index);
		}
	}

	/* pr_debug */pr_warn("%s(): parse \n", __func__);
	err = of_parse_phandle_with_args(np, "spis", "#list-cells", index,
					 &args);
	if (err) {
		/* pr_debug */pr_warn("%s(): can't parse \"spis\" property\n", __func__);
		return ERR_PTR(err);
	}

	/* pr_debug */pr_warn("%s(): find \n", __func__);
	sdev = of_find_spi_device_by_node(args.np);
	if (IS_ERR(sdev)) {
		/* pr_debug */pr_warn("%s(): SPI device not found\n", __func__);
		goto put;
	}

	/*
	 * If a consumer name was not given, try to look it up from the
	 * "pwm-names" property if it exists. Otherwise use the name of
	 * the user device node.
	 */
	// if (!con_id) {
	// 	err = of_property_read_string_index(np, "pwm-names", index,
	// 					    &con_id);
	// 	if (err < 0)
	// 		con_id = np->name;
	// }

	// sdev->label = con_id;

put:
	of_node_put(args.np);

	return sdev;
}

