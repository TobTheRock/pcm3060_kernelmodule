/** @file transceiver.c
 *  @brief transceive data
 *
 *  @author Tobias Waurick
 */

#include "transceiver.h"
#include <utils/spi.h>
#include <config.h>
#include <utils/logging.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/completion.h>

#include <linux/delay.h> // todo RM
#include <linux/jiffies.h> // todo RM

static struct task_struct* etx_spi_thread;
DECLARE_COMPLETION(tx_completion);

//TODO keep this static ?
static struct _thread_in_data
{
    struct spi_device* spiDev;
    duplex_pipe_end_t* leftchan_buf;
    duplex_pipe_end_t* rightchan_buf;
} _internal_data = {NULL, NULL, NULL};

static int _tx_run(void* unused)
{
    // unsigned int n_bytes_writtable = 0;
    static const unsigned int n_bytes_per_read = 16;  //TODO  from config or so
    duplex_pipe_end_t* active_channel = _internal_data.leftchan_buf;
    TRACE("Starting run loop");


    while(!kthread_should_stop())
    {
        unsigned int n_bytes_to_write_from_spi = 0, n_bytes_to_read_from_spi = 0;
        void *mem_tx = NULL, *mem_rx = NULL;
        if ( duplex_pipe_end_n_bytes_available(active_channel) > 0)
        {
            TRACE("Requesting read from pipe");
            n_bytes_to_write_from_spi = duplex_pipe_end_read_start(active_channel, &mem_tx);

        }
        
        if ((n_bytes_to_read_from_spi = duplex_pipe_end_write_start(active_channel, &mem_rx, n_bytes_per_read)) > 0)
        {
            TRACE("Requesting write to pipe...");
        }
        else
        {
            WARNING("RX buffer is full, data might be lost!");
        }

        if ((mem_rx != NULL) && (mem_tx != NULL))
        {
            TRACE("Reading %d and writting %d bytes...", n_bytes_to_read_from_spi, n_bytes_to_write_from_spi);
            spi_write_then_read(_internal_data.spiDev, mem_tx, n_bytes_to_write_from_spi, mem_rx, n_bytes_to_read_from_spi);
            duplex_pipe_end_read_end(active_channel); // TODO read only x byte from here so we can have a fixed cycle lenth
            TRACE("FIN Reading from pipe");
            duplex_pipe_end_write_end(active_channel);
            TRACE("FIN writting from pipe");
        }
        else if (mem_rx != NULL)
        {
            TRACE("Reading %d bytes to %p...", n_bytes_to_read_from_spi, mem_rx);
            TRACE("Before %d %d...", * (unsigned int*)mem_rx, *(unsigned int*)(mem_rx+1));
            spi_read(_internal_data.spiDev, mem_rx, n_bytes_to_read_from_spi);
            TRACE("Got %d %d...", *(unsigned int*)mem_rx, *(unsigned int*)(mem_rx+1));
            duplex_pipe_end_write_end(active_channel);
            TRACE("FIN Reading from pipe");
            
        }
        else if (mem_tx != NULL)
        {
            TRACE("Writting %d bytes...", n_bytes_to_write_from_spi);
            spi_write(_internal_data.spiDev, mem_tx, n_bytes_to_write_from_spi);
            duplex_pipe_end_read_end(active_channel);
            TRACE("FIN Reading from pipe");
        }
        else
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

int tx_init(struct device *pdev, duplex_pipe_end_t* leftchan_buf, duplex_pipe_end_t* rightchan_buf)
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
    ext_spi_dev = spi_get(pdev, "spi_pcm3060_adc");
    if (!ext_spi_dev)
    {
        ERROR("No spi device %s found", "spi_pcm3060_adc");
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
        ext_spi_dev->bits_per_word = 16;
        // ext_spi_dev->max_speed_hz = CONFIG_ADC_CLOCK_BCK1_F_HZ;
        ext_spi_dev->mode = SPI_MODE_1; // CPOL = 0, CPHA = 1
        // ext_spi_dev->bits_per_word = CONFIG_WORD_SIZE_PER_TX;

        spi_setup(ext_spi_dev);
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