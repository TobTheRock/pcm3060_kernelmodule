#include "spi.h"


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
	struct spi_device *spi = NULL;
	struct of_phandle_args args;
	struct spi_device *sdev;
	int index = 0;
	int err;

	if (con_id) {
		index = of_property_match_string(np, "spi-names", con_id);
		if (index < 0)
			return ERR_PTR(index);
	}

	err = of_parse_phandle_with_args(np, "spis", "#list-cells", index,
					 &args);
	if (err) {
		/* pr_debug */pr_warn("%s(): can't parse \"spis\" property\n", __func__);
		return ERR_PTR(err);
	}

	sdev = of_node_to_pwmchip(args.np);
	if (IS_ERR(sdev)) {
		/* pr_debug */pr_warn("%s(): SPI device not found\n", __func__);
		goto put;
	}

	/*
	 * If a consumer name was not given, try to look it up from the
	 * "pwm-names" property if it exists. Otherwise use the name of
	 * the user device node.
	 */
	if (!con_id) {
		err = of_property_read_string_index(np, "pwm-names", index,
						    &con_id);
		if (err < 0)
			con_id = np->name;
	}

	pwm->label = con_id;

put:
	of_node_put(args.np);

	return sdev;
}