/** @file transceiver.h
 *  @brief transceive data
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H
#define KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H

void tx_init(struct device *pdev);
void tx_cleanup(void);

#endif // !KERNELMODULE_PCM3060_PERIPHERY_TRANSCEIVER_H