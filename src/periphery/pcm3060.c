#include "pcm3060.h"
#include <utils/logging.h>
#include <periphery/drivers.h>
#include <periphery/clock_generator.h>

#include <linux/slab.h>
#include <linux/mutex.h>
//#include <linx/types.h>
// #include <linux/string.h>

DEFINE_MUTEX(_pcm3060_mutex);

typedef struct
{
    pcm3060_t* pcm3060_ext;
    pcm3060_config_t* config;
    atomic_t refcount;
} _pcm3060_internal_t;

static _pcm3060_internal_t _pcm3060_i =
{
    NULL,
    NULL,
    ATOMIC_INIT(0)
};

static int _probe_pcm3060(struct device *pdev)
{
    int ret = 0;
    DEBUG("");

    if ( (ret = clock_generator_init(pdev, _pcm3060_i.config->sck_f)) )
    {
        ERROR("Failed to initialize CLOCK!");
    }

    return ret;
}

static int _remove_pcm3060(struct device *pdev)
{
    int ret = 0;
    DEBUG("");
    ret |= clock_generator_cleanup(pdev);

    return ret;
}

static int _configure_pcm3060(const pcm3060_config_t* const cfg)
{
    t_onProbe_cb cbs[] = {&_probe_pcm3060};
    DEBUG("");

    if (_pcm3060_i.config)
    {
        ERROR("Reconfigure not implemented yet");
        return 1;
    }

    _pcm3060_i.config = kmalloc(sizeof(pcm3060_config_t), GFP_KERNEL);
    *(_pcm3060_i.config) = *cfg;

    register_driver_pcm3060(cbs, ARRAY_SIZE(cbs));
    return 0;
}


pcm3060_t* get_pcm3060()
{
    atomic_inc(&_pcm3060_i.refcount);

    mutex_lock(&_pcm3060_mutex);
    if (_pcm3060_i.pcm3060_ext)
    {
        DEBUG("Already created a pcm3060, increasing ref count");
    }
    else
    {
        DEBUG("Creating new pcm3060");
        _pcm3060_i.pcm3060_ext = kmalloc(sizeof(pcm3060_t), GFP_KERNEL);
        _pcm3060_i.pcm3060_ext->init = &_configure_pcm3060;
    }
    mutex_unlock(&_pcm3060_mutex);

    return _pcm3060_i.pcm3060_ext;
}


void put_pcm3060(pcm3060_t* dev_pcm3060)
{
    if (atomic_read(&_pcm3060_i.refcount) == 0)
    {
        WARNING("No pcm3060 was created yet!");
        return;
    }
    else if (_pcm3060_i.pcm3060_ext != dev_pcm3060)
    {
        WARNING("Invalid pcm3060!");
    }
    else
    {
        DEBUG("Decreasing refcount...");
        if (atomic_dec_and_test(&_pcm3060_i.refcount))
        {
            t_onRemove_cb fcbs[] = {&_remove_pcm3060};
            DEBUG("Freeing pcm3060");
            kfree(_pcm3060_i.pcm3060_ext);
            kfree(_pcm3060_i.config);
            unregister_driver_pcm3060(fcbs, ARRAY_SIZE(fcbs));
        }
    }

}