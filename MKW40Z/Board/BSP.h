#ifndef BSP_H
  #define BSP_H

#include   <stdint.h>
#include   <string.h>

#include   "MKW40Z4.h"
#include   "BSP_pins.h"
#include   "BSP_SPI.h"
#include   "BSP_PIT.h"
#include   "BSP_DMA.h"

#define BIT(n) (1u << n)
#define LSHIFT(v,n) (((unsigned int)(v) << n))

#endif // BSP_H



