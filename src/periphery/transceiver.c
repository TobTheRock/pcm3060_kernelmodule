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

int tx_init(struct device *pdev)
{
    unsigned char ch = 0xaa;
    unsigned int test = 0;
    int ret = 0;
    struct spi_device* sdev;
    INFO("Requesting SPI for ADC");

    sdev = spi_get(pdev, "spi_pcm3060_adc");
    if (!sdev)
    {
        ERROR("No spi device %s found", "spi_pcm3060_adc");
        return ENXIO;
    }
    else
    {
        INFO("Got %p", sdev);
        // INFO("Controller %p", sdev->controller); // not yet in kernel 4.9!
        INFO("SPI device  mode %d", sdev->mode);
        INFO("SPI device Master %p", sdev->master);
        INFO("SPI device Master trans func %p", sdev->master->transfer_one);
        INFO("SPI device  Master busnum  %d", sdev->master->bus_num);
        INFO("SPI device  Master freq rang  %d - %d HZ", sdev->master->min_speed_hz, sdev->master->max_speed_hz);

        sdev->max_speed_hz = CONFIG_ADC_CLOCK_BCK1_F_HZ;
        sdev->mode = SPI_MODE_1; // CPOL = 0, CPHA = 1
        sdev->bits_per_word = CONFIG_WORD_SIZE_PER_TX;

        spi_setup(sdev);
    }

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