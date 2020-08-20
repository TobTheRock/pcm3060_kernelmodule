/** @file transceiver.h
 *  @brief transceive data
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H
#define KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H

#include "drivers.h"
#include <utils/dualbuffer.h>


int tx_init(struct device *pdev, dualbuffer_t const* bufin, dualbuffer_t const* bufout);
int tx_cleanup(struct device *pdev);

#endif // !KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H