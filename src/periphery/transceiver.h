/** @file transceiver.h
 *  @brief transceive data
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H
#define KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H

#include "drivers.h"
#include <utils/pipe_buffer.h>


int tx_init(struct device *pdev, pipe_buffer_t const* bufin, pipe_buffer_t* bufout);
int tx_cleanup(struct device *pdev);

#endif // !KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H