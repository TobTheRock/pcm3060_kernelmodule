#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>

#include <periphery/devices.h>
#include <periphery/clock_generator.h>
#include <periphery/transceiver.h>
#include <utils/logging.h>
// #include <ut>

// #include <periphery/clock_generator.h>


static int __init hello_world_init(void)
{
	INFO("Welcome to TobiTronicX\n");
    INFO("Kernel Module Inserted Successfully...\n");

    // t_onProbe_cb cbs[] = {&clock_generator_init, &tx_init};
    t_onProbe_cb cbs[] = {&tx_init};
    register_device_pcm3060(cbs, ARRAY_SIZE(cbs));
    // clock_generator_init();
	return 0;
}

void __exit hello_world_exit(void)
{
    // clock_generator_cleanup();

    t_onRemove_cb fcbs[] = {&tx_cleanup};
    unregister_device_pcm3060(fcbs, ARRAY_SIZE(fcbs));
	printk(KERN_INFO "Kernel Module Removed Successfully...\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com or admin@embetronicx.com>");
MODULE_DESCRIPTION("A simple hello world driver");
MODULE_VERSION("2:1.0");
