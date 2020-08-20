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
#include <linux/mutex.h>

static struct task_struct* etx_spi_thread;

struct _thread_in_data
{
    struct spi_device* spiDev;
    dualbuffer_t const* bufin;
    dualbuffer_t const* bufout;
};

static _tx_run(void* input_data)
{
    struct _thread_in_data data;
    TRACE("%p", input_data);
    
    if (input_data == NULL)
    {
        ERROR("Invalid input data");
        return 1;
    }
    
    data = *((struct _thread_in_data*)input_data);

    TRACE("Starting while %p %p...", data.spiDev, data.bufin,data.bufout);
    while(!kthread_should_stop())
    {
        unsigned int n_bytes_to_write = 0;
        void* mem = NULL;
        if ( (n_bytes_to_write = (data.bufin->read(data.bufin, mem, 0)) > 0) && (mem != NULL))
        {
            TRACE("Writing %d bytes...", n_bytes_to_write);
            spi_write(data.spiDev, mem, n_bytes_to_write);
        }
         //TODO spi_read (sync) all the time... OR better : async...

        // TODO rm
        TRACE("Sleeping");
        schedule_timeout(1000);
    }


    spi_dev_put(data.spiDev);
    return 0;
}

int tx_init(struct device *pdev, dualbuffer_t const* bufin, dualbuffer_t const* bufout)
{
    struct spi_device* ext_spi_dev;
    struct _thread_in_data thread_data = { .bufin = bufin,  .bufout = bufout };

    if ((pdev == NULL) || (bufin == NULL) || (bufout == NULL))
    {
        ERROR("Invalid (nullptr) arguments!");
        return 1;
    }

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

        thread_data.spiDev = ext_spi_dev;
        // ext_spi_dev->max_speed_hz = CONFIG_ADC_CLOCK_BCK1_F_HZ;
        ext_spi_dev->mode = SPI_MODE_1; // CPOL = 0, CPHA = 1
        // ext_spi_dev->bits_per_word = CONFIG_WORD_SIZE_PER_TX;

        spi_setup(ext_spi_dev);
    }


    etx_spi_thread = kthread_run(_tx_run,&thread_data,"Transceiver Thread");
    if(IS_ERR(etx_spi_thread))
    {
        printk(KERN_ERR "Cannot create kthread\n");
        goto r_device;
    }
    TRACE("Exit %p", &thread_data);

//pdev->dev->->of_node
// then read the property---
    return 0;
r_device:
    spi_dev_put(ext_spi_dev);
    return ENXIO;
}

int tx_cleanup(struct device *pdev)
{
    INFO("Cleaning up transceiver....");
    kthread_stop(etx_spi_thread);
    return 0;
}