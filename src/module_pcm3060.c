#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>

// #include <periphery/drivers.h>
// #include <periphery/clock_generator.h>
// #include <periphery/transceiver.h>
#include <utils/logging.h>
#include <chrdev_pcm3060.h>
#include <config.h>
// #include <ut>

// #include <periphery/clock_generator.h>
#include <periphery/pcm3060.h>
static pcm3060_t* pcmdev;

static int __init _pcm3060_module_init(void)
{
    INFO("Kernel Module Inserted Successfully...");
    // pcmdev = get_pcm3060();
	return chrdev_pcm3060_register("pcm3060");
}

void __exit _pcm3060_module_exit(void)
{
    INFO("Removing kernel module ");
    // put_pcm3060(pcmdev);
    chrdev_pcm3060_unregister();
}

module_init(_pcm3060_module_init);
module_exit(_pcm3060_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tobias Waurick");
MODULE_DESCRIPTION("Kernel module to steer the PCM3060");
MODULE_VERSION("0.1");
