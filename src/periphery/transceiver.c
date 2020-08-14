/** @file transceiver.c
 *  @brief transceive data
 *
 *  @author Tobias Waurick
 */

#include "transceiver.h"
#include <utils/spi.h>
#include <config.h>
#include <utils/logging.h>

static struct spi_device* sdev;
static struct spi_master	*master;
static struct CONTROLLER	*c;

int tx_init(struct device *pdev)
{
    unsigned char ch = 0xff;
    INFO("Requesting SPI for ADC");
    struct spi_device* sdev= spi_get(pdev, "spi_pcm3060_adc");
    INFO("Got %p", sdev);
    // INFO("Controller %p", sdev->controller); // not yet in kernel 4.9!
    INFO("SPI device  mode %d", sdev->mode);
    INFO("SPI device Master %p", sdev->master);
    INFO("SPI device Master trans func %p", sdev->master->transfer_one);
    INFO("SPI device  Master busnum  %d", sdev->master->bus_num);
    INFO("SPI device  Master freq rang  %d - %d HZ", sdev->master->min_speed_hz, sdev->master->max_speed_hz);


    // master = spi_alloc_master(dev, sizeof *c);
    // if (!master)
    //     return -ENODEV;
    // c = spi_master_get_devdata(master);
    spi_write(sdev, &ch, sizeof(ch));


//pdev->dev->->of_node
// then read the property---
    return 0;
}

int tx_cleanup(struct device *pdev)
{
    INFO("Cleaning up transceiver....");
    spi_dev_put(sdev);
    return 0;
}