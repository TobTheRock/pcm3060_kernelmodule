/** @file transceiver.c
 *  @brief transceive data
 *
 *  @author Tobias Waurick
 */

#include "transceiver.h"
#include "devicetree.h"
#include <config.h>
#include <utils/spi.h>
#include <utils/ptr.h>
#include <utils/logging.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/completion.h>

#include <linux/delay.h> // todo RM
#include <linux/jiffies.h> // todo RM

#define SPIMODE_LEFT_CHANNEL  SPI_CS_HIGH | SPI_MODE_3  // CHIP select high when active, CPOL = 1, CPHA = 1
#define SPIMODE_RIGHT_CHANNEL SPI_MODE_3                // CPOL = 1, CPHA = 1

static struct task_struct* etx_spi_thread;
DECLARE_COMPLETION(tx_completion);

//TODO keep this static ?
static struct _thread_in_data
{
    struct spi_device* spiDev;
    duplex_ring_end_t* leftchan_buf;
    duplex_ring_end_t* rightchan_buf;
} _internal_data = {NULL, NULL, NULL};

// typedef union
// {
//     u32 word;
//     u8 bytes[4];
// } ;


static int _tx_rx_on_channel(duplex_ring_end_t* active_channel, unsigned int lrck_low)
{
    unsigned int n_bytes_to_write_over_spi = 0, n_bytes_to_read_from_spi = 0;
    int ret = 0;
    // u8 mem_tx[CONFIG_SPI_N_BYTE_PER_TX] = {0}, mem_rx[CONFIG_SPI_N_BYTE_PER_TX] = {0};
     u32 mem_tx[1] = {0}, mem_rx[1] = {0};

    if ( (n_bytes_to_write_over_spi = duplex_ring_end_n_bytes_readable(active_channel)) > 0)
    {
        // unsigned int byte_it;
        TRACE("Bytes readable in TX ring buffer %d", n_bytes_to_write_over_spi);
        n_bytes_to_write_over_spi = min(n_bytes_to_write_over_spi, CONFIG_N_BYTE_SIZE_PER_TX);
        duplex_ring_end_read(active_channel, mem_tx, n_bytes_to_write_over_spi); // TODO bytes dropped?
        // for (byte_it = 0; byte_it < (CONFIG_N_BYTE_SIZE_PER_TX-n_bytes_to_write_over_spi); byte_it++)
        // {
        //     *(mem_tx+byte_it) = 0;
        // }
        
    }

    if ((n_bytes_to_read_from_spi = duplex_ring_end_n_bytes_writable(active_channel)) > 0)
    {
        TRACE("Bytes writable in RX ring buffer %d", n_bytes_to_read_from_spi);
        n_bytes_to_read_from_spi = min(n_bytes_to_read_from_spi, CONFIG_N_BYTE_SIZE_PER_TX);
    }
    else
    {
        WARNING("RX buffer is full, data might be lost!");
    }

    if ((n_bytes_to_write_over_spi == 0) && (n_bytes_to_read_from_spi == 0))
    {
        WARNING("Cannot do anything!");
        ret = -1;
    }
    else
    {
        // _internal_data.spiDev-> setCs ? directly?
        TRACE("Reading %d and writting %d bytes...", n_bytes_to_read_from_spi, n_bytes_to_write_over_spi);
        // spi_write_then_read(_internal_data.spiDev, mem_tx, CONFIG_SPI_N_BYTE_PER_TX, mem_rx, CONFIG_SPI_N_BYTE_PER_TX);
        struct spi_transfer	t = {
        .tx_buf		= mem_tx,
        .rx_buf		= mem_rx,
        .len		= CONFIG_SPI_N_BYTE_PER_TX,
         .bits_per_word = CONFIG_SPI_WORD_LEN,
        // .cs_change = lrck_low,          //TODO rm this does not work
		};

	    spi_sync_transfer(_internal_data.spiDev, &t, 1);  //TODO async speedup
        TRACE("Got  %d %d %d %d...", * (u8*)mem_rx, *(u8*)(mem_rx+1), *(u8*)(mem_rx+2), *(u8*)(mem_rx+3));
        // TRACE("Got  %d ...", *mem_rx);
        if (n_bytes_to_read_from_spi)
        {
            TRACE("Writing to RX ring buffer...");
            duplex_ring_end_write(active_channel, mem_rx, n_bytes_to_read_from_spi); // TODO bytes dropped?
        }
    }

    return ret;
}

static int _tx_run(void* unused)
{
    // unsigned int n_bytes_writtable = 0;
    // duplex_ring_end_t* active_channel = _internal_data.leftchan_buf;
    // u8 *mem_tx = NULL, *mem_rx = NULL;

    // mem_tx = kcalloc(CONFIG_SPI_N_BYTE_PER_TX, sizeof(u8), GFP_KERNEL);
    // RETURN_ON_NULL(mem_tx,-1);q
    // mem_rx = kcalloc(CONFIG_SPI_N_BYTE_PER_TX, sizeof(u8), GFP_KERNEL);
    // RETURN_ON_NULL(mem_rx,-1);

    TRACE("Starting run loop");
    TRACE("RX/TX at most %d byte per LRCK half cycle", CONFIG_SPI_N_BYTE_PER_TX);
    while(!kthread_should_stop())
    {
        int err;
        // TRACE("SETUP");
        // _internal_data.spiDev->mode = SPIMODE_LEFT_CHANNEL;
        // if (spi_setup(_internal_data.spiDev))
        // {
        //     ERROR("Could not setup SPI device!");
        //     return -ENXIO;
        // }
        TRACE("LCHAN");
        err = _tx_rx_on_channel(_internal_data.leftchan_buf,1);
        //TOGGLE SPI

        // TRACE("SETUP");
        // _internal_data.spiDev->mode = SPIMODE_RIGHT_CHANNEL;
        // if (spi_setup(_internal_data.spiDev))
        // {
        //     ERROR("Could not setup SPI device!");
        //     return -ENXIO;
        // }
        TRACE("RCHAN");
        err |= _tx_rx_on_channel(_internal_data.rightchan_buf,0);

        if(err)
        {
            set_current_state(TASK_INTERRUPTIBLE);
            WARNING("Cannot do anything sleeping!");
            schedule_timeout(msecs_to_jiffies(1000)); // Todo rm magic number
        }
    }
    TRACE("Exiting...");
    complete(&tx_completion);
    return 0;
}

int tx_init(struct device *pdev, duplex_ring_end_t* leftchan_buf, duplex_ring_end_t* rightchan_buf)
{
    
    struct spi_device* ext_spi_dev;

    if ((pdev == NULL) || (leftchan_buf == NULL) || (rightchan_buf == NULL))
    {
        ERROR("Invalid (nullptr) arguments!");
        return 1;
    }
    _internal_data.leftchan_buf = leftchan_buf;
    _internal_data.rightchan_buf = rightchan_buf;

    INFO("Requesting SPI for ADC");
    ext_spi_dev = spi_get(pdev, DEVICETREE_PCM3060_SPI_NAME);
    if (!ext_spi_dev)
    {
        ERROR("No spi device %s found", DEVICETREE_PCM3060_SPI_NAME);
        return ENXIO;
    }
    else
    {
        INFO("Got %p", ext_spi_dev);
        // INFO("Controller %p", ext_spi_dev->controller); // not yet in kernel 4.9!
        INFO("SPI device  mode %d", ext_spi_dev->mode);
        INFO("SPI device Master %p", ext_spi_dev->master);
        INFO("SPI device Master trans func %p", ext_spi_dev->master->transfer_one);
        INFO("SPI device  Master busnum  %d", ext_spi_dev->master->bus_num);
        INFO("SPI device  Master freq rang  %d - %d HZ", ext_spi_dev->master->min_speed_hz, ext_spi_dev->master->max_speed_hz);

        _internal_data.spiDev = ext_spi_dev;
        ext_spi_dev->max_speed_hz = 50000;
        ext_spi_dev->bits_per_word = CONFIG_SPI_WORD_LEN;
        // ext_spi_dev->max_speed_hz = CONFIG_ADC_CLOCK_BCK1_F_HZ;
        ext_spi_dev->mode = SPI_MODE_3; // CPOL = 1, CPHA = 1
        // ext_spi_dev->bits_per_word = CONFIG_WORD_SIZE_PER_TX;

        if (spi_setup(ext_spi_dev))
        {
            ERROR("Could not setup SPI device!");
            return  ENXIO;
        }
    }


    etx_spi_thread = kthread_run(_tx_run,NULL,"Transceiver Thread");
    if(IS_ERR(etx_spi_thread))
    {
        printk(KERN_ERR "Cannot create kthread\n");
        etx_spi_thread = NULL;
        goto r_device;
    }

//pdev->dev->->of_node
// then read the property---
    return 0;
r_device:
    spi_dev_put(ext_spi_dev);
    return ENXIO;
}

int tx_cleanup(struct device *pdev)
{
    //TODO checks...
    INFO("Cleaning up transceiver....");
    if (etx_spi_thread != NULL)
    {
        INFO("Stopping thread %p ...", etx_spi_thread);
        kthread_stop(etx_spi_thread);
        TRACE("Waiting to join...");
        wait_for_completion(&tx_completion);
    }
    else
    {
        WARNING("No kernel thread...");
    }
    if (_internal_data.spiDev != NULL)
    {
        TRACE("Removing SPI device..");
        spi_dev_put(_internal_data.spiDev);
    }
    else
    {
        TRACE("No SPI device..");
    }
    
    
    return 0;
}