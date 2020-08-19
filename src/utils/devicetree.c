#include "devicetree.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,7,9)

int device_match_of_node(struct device *dev, void *np)
{
	return dev->of_node == np;
}

#endif