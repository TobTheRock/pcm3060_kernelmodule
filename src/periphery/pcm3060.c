#include "pcm3060.h"
#include <config.h>
#include <utils/logging.h>
#include <utils/ptr.h>
#include <periphery/devicetree.h>
#include <periphery/clock_generator.h>
#include <periphery/transceiver.h>

#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/device.h>
//#include <linx/types.h>
// #include <linux/string.h>

DEFINE_MUTEX(_pcm3060_mutex);

typedef struct
{
    pcm3060_t* pcm3060_ext;
    struct device* pdev;
    pcm3060_config_t* config;
    duplex_ring_buffer_t* left_chan_buffer;
    duplex_ring_buffer_t* right_chan_buffer;
    atomic_t refcount;
} _pcm3060_internal_t;

static _pcm3060_internal_t _pcm3060_i =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ATOMIC_INIT(0)
};

static int _probe_pcm3060_device(struct device *pdev)
{
    int ret = 0;
    TRACE("");
    // if ( (ret = clock_generator_init(pdev, CONFIG_GET_CLOCK_SCK_F_HZ(_pcm3060_i.config->fs))) )
    // {
    //     ERROR("Failed to initialize CLOCK!");
    // }
    // else
    if ( (ret = tx_init(pdev, _pcm3060_i.left_chan_buffer->right_end, _pcm3060_i.right_chan_buffer->right_end)) )
    {
        ERROR("Failed to initialize TRANSCEIVER!");
    }
    

    return ret;
}

static int _remove_pcm3060_device(struct device *pdev)
{
    int ret = 0;
    TRACE("");
    // ret |= clock_generator_cleanup(pdev);
    ret |= tx_cleanup(pdev);
    put_device(_pcm3060_i.pdev);
    _pcm3060_i.pdev = NULL;
    return ret;
}

static int _configure_pcm3060(const pcm3060_config_t* const cfg)
{
    TRACE("");
    RETURN_ON_NULL(cfg, -1);
    
    if (_pcm3060_i.config)
    {
        WARNING("Reconfigure not implemented yet");
        return 0;
    }

    if ( (_pcm3060_i.config = kmalloc(sizeof(pcm3060_config_t), GFP_KERNEL)) == NULL)
    {
        ERROR("Failed to allocate memory in the kernel");
        return -1;
    }
    else if ( (_pcm3060_i.left_chan_buffer = get_duplex_ring_buffer(cfg->buf_size)) == NULL )
    {
        ERROR("Failed to get Input buffer");
        goto r_conf;
    }
    else if ( (_pcm3060_i.right_chan_buffer = get_duplex_ring_buffer(cfg->buf_size)) == NULL )
    {
        ERROR("Failed to get Output buffer");
        goto r_buf;
    }

    *(_pcm3060_i.config) = *cfg;

    return _probe_pcm3060_device(_pcm3060_i.pdev);
    r_conf:
        kfree(_pcm3060_i.config);
    r_buf:
        put_duplex_ring_buffer(_pcm3060_i.left_chan_buffer);
    return -1;
}

static duplex_ring_end_t* _pcm3060_get_channel_buffer_end(unsigned int channel)
{
    duplex_ring_end_t* pipe_end = NULL;
    
    switch (channel)
    {
    case PCM3060_CHANNEL_ID_LEFT:
        pipe_end = _pcm3060_i.left_chan_buffer->left_end;
        break;
        
    case PCM3060_CHANNEL_ID_RIGHT:
        pipe_end = _pcm3060_i.right_chan_buffer->left_end;
        break;
    
    default:
        WARNING("Invalid Channel id %d", channel);
        break;
    }

    return pipe_end;
}

pcm3060_t* get_pcm3060()
{
    TRACE("");
    if (atomic_inc_return(&_pcm3060_i.refcount) > 1)
    {
        DEBUG("Already created a pcm3060, increasing ref count");
    }
    else
    {
        DEBUG("Creating new pcm3060");
        mutex_lock(&_pcm3060_mutex);
        if ( (_pcm3060_i.pcm3060_ext = kmalloc(sizeof(pcm3060_t), GFP_KERNEL)) == NULL)
        {
            ERROR("Failed to allocate memory in the kernel");
            goto r_null;
        }
        else if ((_pcm3060_i.pdev = dt_find_pcm3060_device()) == NULL)
        {
            ERROR("Failed to get a matching device");
            goto r_str;
        }
        else
        {
            _pcm3060_i.left_chan_buffer = NULL;
            _pcm3060_i.right_chan_buffer = NULL;
            _pcm3060_i.config = NULL;
            _pcm3060_i.pcm3060_ext->init = &_configure_pcm3060;
            _pcm3060_i.pcm3060_ext->get_channel_buffer_end = &_pcm3060_get_channel_buffer_end;
        }
        mutex_unlock(&_pcm3060_mutex);
    }

    return _pcm3060_i.pcm3060_ext;
    r_str:
        kfree(_pcm3060_i.pcm3060_ext);
    r_null:
        mutex_unlock(&_pcm3060_mutex);
        return NULL;
}


void put_pcm3060(pcm3060_t* dev_pcm3060)
{
    TRACE("");
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
            DEBUG("Freeing pcm3060");
            _remove_pcm3060_device(_pcm3060_i.pdev);
            TRACE("...Done");
            
            if (_pcm3060_i.pcm3060_ext != NULL)
            {
                TRACE("Freeing external interface");
                kfree(_pcm3060_i.pcm3060_ext);
            }

            if (_pcm3060_i.left_chan_buffer != NULL)
            {
                TRACE("Freeing left channel buffer");
                put_duplex_ring_buffer(_pcm3060_i.left_chan_buffer);
                _pcm3060_i.left_chan_buffer = NULL;
            }
            if (_pcm3060_i.right_chan_buffer != NULL)
            {
                TRACE("Freeing right channel buffer");
                put_duplex_ring_buffer(_pcm3060_i.right_chan_buffer);
                _pcm3060_i.right_chan_buffer = NULL;
            }
            if (_pcm3060_i.config != NULL)
            {
                TRACE("Freeing config");
                kfree(_pcm3060_i.config);
            }

            _pcm3060_i.pcm3060_ext = NULL;
            _pcm3060_i.config = NULL;
            dev_pcm3060 = NULL;
            TRACE("FIN");
        }
    }

}