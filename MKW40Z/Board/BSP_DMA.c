// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.09.18
// 14:45:10
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "BSP.h"



static uint8_t DMA_SPI_tx_buf[SPI_DMA_BUF_SZ];
static uint8_t DMA_SPI_rx_buf[SPI_DMA_BUF_SZ];

/*------------------------------------------------------------------------------



 \param void
 ------------------------------------------------------------------------------*/
static void BSP_DMA_Mux_reset(void)
{
  SIM_SCGC6 |= BIT(1); // Включаем тактироовнаие DMA Mux
  DMAMUX0_CHCFG0 = 0;
  DMAMUX0_CHCFG1 = 0;
  DMAMUX0_CHCFG2 = 0;
  DMAMUX0_CHCFG3 = 0;

}
/*------------------------------------------------------------------------------



 \param void
 ------------------------------------------------------------------------------*/
static void BSP_DMA_Mux_init(void)
{

  // Конфигурируем мультиплексирование источников сигнало для DMA
  // В наличии имеем только 4-е канала DMA
  // Два из них занимаем приемом-передачей по SPI1
  DMAMUX0_CHCFG0 = 0
                   + LSHIFT(1,     7) // ENBL      | DMA Channel Enable           | 1 DMA channel is enabled
                   + LSHIFT(0,     6) // TRIG      | DMA Channel Trigger Enable   | 0 Triggering is disabled. If triggering is disabled and ENBL is set, the DMA Channel will simply route the specified source to the DMA channel. (Normal mode)
                   + LSHIFT(19,    0) // SOURCE    | DMA Channel Source (Slot)    | SPI1 Transmit
  ;
  DMAMUX0_CHCFG1 = 0
                   + LSHIFT(1,     7) // ENBL      | DMA Channel Enable           | 1 DMA channel is enabled
                   + LSHIFT(0,     6) // TRIG      | DMA Channel Trigger Enable   | 0 Triggering is disabled. If triggering is disabled and ENBL is set, the DMA Channel will simply route the specified source to the DMA channel. (Normal mode)
                   + LSHIFT(18,    0) // SOURCE    | DMA Channel Source (Slot)    | SPI1 Receive
  ;

}

/*------------------------------------------------------------------------------



 \param void
 ------------------------------------------------------------------------------*/
void BSP_DMA_init(void)
{
  SIM_SCGC7 |= BIT(8); // Включаем тактироовнаие DMA

  BSP_DMA_Mux_reset();
  BSP_DMA_Mux_init();

}


/*------------------------------------------------------------------------------



 \param void
 ------------------------------------------------------------------------------*/

void BSP_DMA_SPI_init_transfer()
{
  BSP_ClearFIFO_SPI1();
  // Канал на прием из SPI
  DMA_SAR1 = (uint32_t)&SPI1_POPR;
  DMA_DAR1 = (uint32_t)DMA_SPI_rx_buf;
  DMA_DSR_BCR1 =  SPI_DMA_BUF_SZ; //  Устанавливаем количество принимаемых байт

  DMA_DCR1 = 0
             + LSHIFT(0,     31) // EINT    | Enable Interrupt on Completion of Transfer | 0 No interrupt is generated
             + LSHIFT(1,     30) // ERQ     | Enable Peripheral Request                  | 1 Enables peripheral request to initiate transfer
             + LSHIFT(1,     29) // CS      | Cycle Steal                                | 1 Forces a single read/write transfer per request.
             + LSHIFT(0,     28) // AA      | Auto-align                                 | 0 Auto-align disabled
             + LSHIFT(0,     23) // EADREQ  | Enable asynchronous DMA requests           | 0 Disabled
             + LSHIFT(0,     22) // SINC    | Source Increment                           | 0 No change to SAR after a successful transfer.
             + LSHIFT(1,     20) // SSIZE   | Source Size                                | 01 8-bit
             + LSHIFT(1,     19) // DINC    | Destination Increment                      | 1 The DAR increments by 1, 2, 4 depending upon the size of the transfer.
             + LSHIFT(1,     17) // DSIZE   | Destination Size                           | 01 8-bit
             + LSHIFT(0,     16) // START   | Start Transfer                             | 0 DMA inactive
             + LSHIFT(0,     12) // SMOD    | Source Address Modulo                      | 0000 Buffer disabled
             + LSHIFT(0,     8)  // DMOD    | Destination Address Modulo                 | 0000 Buffer disabled
             + LSHIFT(1,     7)  // D_REQ   | Disable Request                            | 1 ERQ bit is cleared when the BCR is exhausted
             + LSHIFT(0,     4)  // LINKCC  | Link Channel Control                       | 00 No channel-to-channel linking
             + LSHIFT(0,     2)  // LCH1    | Link Channel 1                             |
             + LSHIFT(0,     0)  // LCH2    | Link Channel 2                             |
  ;


  // Канал на передачу в SPI
  DMA_SAR0 = (uint32_t)DMA_SPI_tx_buf;
  DMA_DAR0 = (uint32_t)&SPI1_PUSHR;
  DMA_DSR_BCR0 =  SPI_DMA_BUF_SZ; // Устанавливаем количество передаваемых байт

  DMA_DCR0 = 0
             + LSHIFT(0,     31) // EINT    | Enable Interrupt on Completion of Transfer | 0 No interrupt is generated
             + LSHIFT(1,     30) // ERQ     | Enable Peripheral Request                  | 1 Enables peripheral request to initiate transfer
             + LSHIFT(1,     29) // CS      | Cycle Steal                                | 1 Forces a single read/write transfer per request.
             + LSHIFT(0,     28) // AA      | Auto-align                                 | 0 Auto-align disabled
             + LSHIFT(0,     23) // EADREQ  | Enable asynchronous DMA requests           | 0 Disabled
             + LSHIFT(1,     22) // SINC    | Source Increment                           | 1 The SAR increments by 1, 2, 4 as determined by the transfer size.
             + LSHIFT(1,     20) // SSIZE   | Source Size                                | 01 8-bit
             + LSHIFT(0,     19) // DINC    | Destination Increment                      | 0 No change to the DAR after a successful transfer.
             + LSHIFT(1,     17) // DSIZE   | Destination Size                           | 01 8-bit
             + LSHIFT(0,     16) // START   | Start Transfer                             | 0 DMA inactive
             + LSHIFT(0,     12) // SMOD    | Source Address Modulo                      | 0000 Buffer disabled
             + LSHIFT(0,     8)  // DMOD    | Destination Address Modulo                 | 0000 Buffer disabled
             + LSHIFT(1,     7)  // D_REQ   | Disable Request                            | 1 ERQ bit is cleared when the BCR is exhausted
             + LSHIFT(0,     4)  // LINKCC  | Link Channel Control                       | 00 No channel-to-channel linking
             + LSHIFT(0,     2)  // LCH1    | Link Channel 1                             |
             + LSHIFT(0,     0)  // LCH2    | Link Channel 2                             |
  ;


}


/*------------------------------------------------------------------------------



 \param void
 ------------------------------------------------------------------------------*/
void BSP_SPI_DMA_wait_transfer_done(void)
{
//  while ((DMA_DSR_BCR0 & BIT(24)) == 0);
  while ((DMA_DSR_BCR1 & BIT(24)) == 0); // Ожидаем окончание приема
  DMA_DSR_BCR0 = BIT(24);
  DMA_DSR_BCR1 = BIT(24);

}

/*------------------------------------------------------------------------------



 \param void

 \return uint8_t*
 ------------------------------------------------------------------------------*/
uint8_t* BSP_SPI_DMA_Get_tx_buf(void)
{
  return DMA_SPI_tx_buf;
}


/*------------------------------------------------------------------------------



 \param void

 \return uint8_t*
 ------------------------------------------------------------------------------*/
uint8_t* BSP_SPI_DMA_Get_rx_buf(void)
{
  return DMA_SPI_rx_buf;
}
