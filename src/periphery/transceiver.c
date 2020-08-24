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

static struct _thread_in_data
{
    struct spi_device* spiDev;
    pipe_buffer_t const* bufin;
    pipe_buffer_t * bufout;
} _internal_data = {NULL, NULL, NULL};

static int _tx_run(void* unused)
{
    static int offset = 0;
    TRACE("Starting while %p %p...", _internal_data.spiDev, _internal_data.bufin,_internal_data.bufout);

    while(!kthread_should_stop())
    {
        unsigned int n_bytes_to_write = 0, offset = 0;
        void* mem = NULL;
        TRACE("Reading");
        if ( pipe_buffer_n_bytes_available(_internal_data.bufin) > 0)
        {
            n_bytes_to_write = pipe_buffer_read_start(_internal_data.bufin, &mem);
            TRACE("Writing %d bytes...", n_bytes_to_write);
            spi_write(_internal_data.spiDev, mem, n_bytes_to_write);
            TRACE("End reading");
            pipe_buffer_read_end(_internal_data.bufin);
            TRACE("FIN Reading");

        }

         //TODO spi_read (sync) all the time... OR better : async...

        // TODO rm
        TRACE("Sleeping, should have written %d bytes", n_bytes_to_write);
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(msecs_to_jiffies(1000));
        // msleep(1000);
        // schedule();
    }
    TRACE("Exiting...");
    complete(&tx_completion);
    return 0;
}

int tx_init(struct device *pdev, pipe_buffer_t const* bufin, pipe_buffer_t* bufout)
{
    struct spi_device* ext_spi_dev;

    if ((pdev == NULL) || (bufin == NULL) || (bufout == NULL))
    {
        ERROR("Invalid (nullptr) arguments!");
        return 1;
    }
    _internal_data.bufin = bufin;
    _internal_data.bufout = bufout;

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