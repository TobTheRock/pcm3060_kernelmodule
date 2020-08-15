#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>

#include <periphery/drivers.h>
#include <periphery/clock_generator.h>
#include <periphery/transceiver.h>
#include <utils/logging.h>
#include <periphery/pcm3060.h>
#include <config.h>
// #include <ut>

// #include <periphery/clock_generator.h>
static pcm3060_t* pcmdev;

static int __init hello_world_init(void)
{
    pcm3060_config_t cfg = {
        .sck_f =CONFIG_ADC_FS_HZ
        };

	INFO("Welcome to TobiTronicX\n");
    INFO("Kernel Module Inserted Successfully...\n");
    INFO("CFG  %d\n", cfg.sck_f);

    // t_onProbe_cb cbs[] = {&clock_generator_init, &tx_init};
    // t_onProbe_cb cbs[] = {&tx_init};
    // register_driver_pcm3060(cbs, ARRAY_SIZE(cbs));
    // clock_generator_init();

    pcmdev = get_pcm3060();
    pcmdev->init(&cfg);

	return 0;
}

void __exit hello_world_exit(void)
{
    // clock_generator_cleanup();

    // t_onRemove_cb fcbs[] = {&tx_cleanup};
    // unregister_driver_pcm3060(fcbs, ARRAY_SIZE(fcbs));
    put_pcm3060(pcmdev);
	printk(KERN_INFO "Kernel Module Removed Successfully...\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com or admin@embetronicx.com>");
MODULE_DESCRIPTION("A simple hello world driver");
MODULE_VERSION("2:1.0");
