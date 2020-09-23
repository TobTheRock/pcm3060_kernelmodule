
#ifndef KERNELMODULE_PCM3060_PERIPHERY_ODROIDN2_PINS_H
#define KERNELMODULE_PCM3060_PERIPHERY_ODROIDN2_PINS_H

// #ifdef (CONFIG_MASTER_GENERATE_CLK) && (CONFIG_MASTER_GENERATE_CLK == 1)
// #warning Odroid N2 will generate the SLCK, BCK & LRCK clocks.
// #endif // (CONFIG_MASTER_GENERATE_CLK) && (CONFIG_MASTER_GENERATE_CLK == 1)

/*FOR LATER*/
// ODROIDN2 J2 - 2x20 PINS
//
//                  +-\/-+
//           3.3V  1|    |2 5v
//                 3|    |4
//                 5|    |6
//                 7|    |8
//                 9|    |10
//                11|    |12
//                13|    |14
//                15|    |16
//                17|    |18
//                19|    |20
// DIN  (SPI MOSI)21|    |22
// DOUT (SPI MISO)23|    |24
// BCK (SPI SCLK) 25|    |26 LRCK (GPIO A04)
//                27|    |28
//                29|    |30
//                31|    |32
//   SCK1 (PWMD)  33|    |34
//                35|    |36
//                37|    |38
//           GND  39|    |40
//                  +----+
#define PINS_ODROIDN2_BCK2 33

#endif // !KERNELMODULE_PCM3060_PERIPHERY_ODROIDN2_PINS_H