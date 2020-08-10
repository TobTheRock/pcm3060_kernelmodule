/** @file spi.h
 *  @brief Spi utilities
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_UTILS_SPI_H
#define KERNELMODULE_PCM3060_UTILS_SPI_H

#include <linux/spi/spi.h>
struct spi_device *spi_get(struct device *dev, const char *con_id);

#endif // !KERNELMODULE_PCM3060_UTILS_SPI_H