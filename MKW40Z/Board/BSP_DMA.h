#ifndef BSP_DMA_H
  #define BSP_DMA_H

#define SPI_DMA_BUF_SZ  22

void      BSP_DMA_init(void);
void      BSP_DMA_SPI_init_transfer();
void      BSP_SPI_DMA_wait_transfer_done(void);
uint8_t*  BSP_SPI_DMA_Get_tx_buf(void);
uint8_t*  BSP_SPI_DMA_Get_rx_buf(void);
#endif // BSP_DMA_H



