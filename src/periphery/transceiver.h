/** @file transceiver.h
 *  @brief transceive data
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H
#define KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H

#include "drivers.h"


int tx_init(struct device *pdev);
int tx_cleanup(struct device *pdev);

#endif // !KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H