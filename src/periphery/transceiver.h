/** @file transceiver.h
 *  @brief transceive data
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H
#define KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H

#include "drivers.h"
#include <utils/duplex_ring_buffer.h>


int tx_init(struct device *pdev, duplex_ring_end_t* leftchan_buf, duplex_ring_end_t* rightchan_buf);
int tx_cleanup(struct device *pdev);

#endif // !KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H