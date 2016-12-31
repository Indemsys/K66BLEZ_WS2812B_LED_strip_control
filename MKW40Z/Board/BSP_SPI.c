// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.09.11
// 13:29:50
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "BSP.h"


/*------------------------------------------------------------------------------

  Частота шины = 16 МГц

 \param void
 ------------------------------------------------------------------------------*/
void BSP_Init_SPI1(void)
{
  uint8_t dbr    = 1;
  uint8_t pbr    = 3;
  uint8_t br     = 2;
  uint8_t pcssck = 1;
  uint8_t cssck  = 1;

//  fsz   - размер передаваемых данных ( 3 < fsz < 17 )
//  cpol  - начальный уровень. 0 The inactive state value of SCK is low.
//                            1 The inactive state value of SCK is high.
//
//  cpha  - выбор среза. 0 Data is captured on the leading edge of SCK and changed on the following edge.
//                      1 Data is changed on the leading edge of SCK and captured on the following edge.
  T_SPI_bits fsz = SPI_8_BITS;
  uint8_t cpol = 0;
  uint8_t cpha = 0;


  // Конфигурируем выводы SPI1
  // PTA16 - SOUT
  // PTA17 - SIN
  // PTA18 - SCK
  // PTA19 - PCS0
  BSP_Init_SPI1_Pins();

  SIM_SCGC6 |= BIT(13); // Разрешить тактирование SPI1

  SPI1_MCR = 0
             + LSHIFT(1,    31) // MSTR      | Master/Slave Mode Select                     | 1 Enables Master mode
             + LSHIFT(0,    30) // CONT_SCKE | Continuous SCK Enable                        | 0 Continuous SCK disabled
             + LSHIFT(0,    28) // DCONF     | SPI Configuration.                           | 00 SPI
             + LSHIFT(0,    27) // FRZ       | Freeze                                       | 0 Do not halt serial transfers in Debug mode.
             + LSHIFT(0,    26) // MTFE      | Modified Timing Format Enable                | 0 Modified SPI transfer format disabled.
             + LSHIFT(0,    24) // ROOE      | Receive FIFO Overflow Overwrite Enable       | 0 Incoming data is ignored.
             + LSHIFT(0x3F, 16) // PCSIS     | Peripheral Chip Select x Inactive State      | 1 The inactive state of PCSx is high.
             + LSHIFT(0,    15) // DOZE      | Doze Enable                                  | 0 Doze mode has no effect on the module.
             + LSHIFT(0,    14) // MDIS      | Module Disable                               | 0 Enables the module clocks.
             + LSHIFT(0,    13) // DIS_TXF   | Disable Transmit FIFO                        | 0 TX FIFO is enabled.
             + LSHIFT(0,    12) // DIS_RXF   | Disable Receive FIFO                         | 0 RX FIFO is enabled.
             + LSHIFT(1,    11) // CLR_TXF   | Writing a 1 to CLR_TXF clears the TX FIFO Counter
             + LSHIFT(1,    10) // CLR_RXF   | Writing a 1 to CLR_RXF clears the RX FIFO Counter
             + LSHIFT(0,    8)  // SMPL_PT   | Controls when the module master samples SIN in Modified Transfer Format
             + LSHIFT(1,    0)  // HALT      | The HALT bit starts and stops frame transfers| 1 Stop transfers.
  ;

// Поиск делителей для обеспечения частоты тактирования baud = (CPU_BUS_CLK_HZ /pbr) x [(1+dbr)/br]
// dbr примем = 1
// Задержка задержка подачи тактового сигнала после установки сигнала выборки должна быть равна половине длительности тактового сигнала

  // Установки для скорости 8 Mbit/s
  dbr    = 1;
  pbr    = PBR_2;
  br     = BR_2;
  pcssck = PCSSCK_1;
  cssck  = CSSCK_2;


  // Программируем первый набор аттрибутов, будем использовать только его
  SPI1_CTAR0 = 0
               + LSHIFT(dbr,     31) // DBR    | Double Baud Rate                       | 0 The baud rate is computed normally with a 50/50 duty cycle.
               + LSHIFT(fsz - 1, 27) // FMSZ   | Frame Size                             |
               + LSHIFT(cpol,    26) // CPOL   | Clock Polarity
               + LSHIFT(cpha,    25) // CPHA   | Clock Phase
               + LSHIFT(0,       24) // LSBFE  | LSB First                              | 0 Data is transferred MSB first.
               + LSHIFT(pcssck,  22) // PCSSCK | PCS to SCK Delay Prescaler             | 01 PCS to SCK Prescaler value is 3.
               + LSHIFT(0,       20) // PASC   | After SCK Delay Prescaler              | 00 Delay after Transfer Prescaler value is 1.
               + LSHIFT(0,       18) // PDT    | Delay after Transfer Prescaler         | 00 Delay after Transfer Prescaler value is 1.
               + LSHIFT(pbr,     16) // PBR    | Baud Rate Prescaler. The baud rate is the frequency of the SCK (0 -> 2, 1 -> 3, 2 -> 5, 3 -> 7)
               + LSHIFT(cssck,   12) // CSSCK  | PCS to SCK Delay Scaler. The PCS to SCK Delay is the delay between the assertion of PCS and the first edge of the SCK
               + LSHIFT(0,       8)  // ASC    | After SCK Delay Scaler. The After SCK Delay is the delay between the last edge of SCK and the negation of PCS
               + LSHIFT(0,       4)  // DT     | Delay After Transfer Scaler. The Delay after Transfer is the time between the negation of the PCS signal at the end
                                     //        |   of a frame and the assertion of PCS at the beginning of the next frame.
               + LSHIFT(br,      0)  // BR     | Baud Rate Scaler. The baud rate is computed according to the following equation:
                                     //        |  SCK baud rate = (fP /PBR) x [(1+DBR)/BR]
  ;

  // Status Register
  SPI1_SR = 0
            + LSHIFT(1,   31) // TCF       | Transfer Complete Flag
            + LSHIFT(1,   30) // TXRXS     | TX and RX Status
            + LSHIFT(1,   28) // EOQF      | End of Queue Flag
            + LSHIFT(1,   27) // TFUF      | Transmit FIFO Underflow Flag
            + LSHIFT(1,   25) // TFFF      | Transmit FIFO Fill Flag
            + LSHIFT(1,   19) // RFOF      | Receive FIFO Overflow Flag
            + LSHIFT(1,   17) // RFDF      | Receive FIFO Drain Flag
            + LSHIFT(0,   12) // TXCTR     | TX FIFO Counter (read only)
            + LSHIFT(0,    8) // TXNXTPTR  | Transmit Next Pointer
            + LSHIFT(0,    4) // RXCTR     | RX FIFO Counter
            + LSHIFT(0,    0) // POPNXTPTR | Pop Next Pointer
  ;

  // DMA/Interrupt Request Select and Enable Register
  SPI1_RSER = 0
              + LSHIFT(0,   31) // TCF_RE    | Transmission Complete Request Enable. Enables TCF flag in the SR to generate an interrupt request                   | 0 TCF interrupt requests are disabled.
              + LSHIFT(0,   28) // EOQF_RE   | Finished Request Enable. Enables the EOQF flag in the SR to generate an interrupt request                           | 0 EOQF interrupt requests are disabled.
              + LSHIFT(0,   27) // TFUF_RE   | Transmit FIFO Underflow Request Enable. Enables the TFUF flag in the SR to generate an interrupt request            | 0 TFUF interrupt requests are disabled.
              + LSHIFT(1,   25) // TFFF_RE   | Transmit FIFO Fill Request Enable. Enables the TFFF flag in the SR to generate a request                            | 1 TFFF interrupts or DMA requests are enabled
              + LSHIFT(1,   24) // TFFF_DIRS | Transmit FIFO Fill DMA or Interrupt Request Select. Selects between generating a DMA request or an interrupt request| 1 TFFF flag generates DMA requests.
              + LSHIFT(1,   19) // RFOF_RE   | Receive FIFO Overflow Request Enable. Enables the RFOF flag in the SR to generate an interrupt request              | 1 RFOF interrupt requests are enabled.
              + LSHIFT(1,   17) // RFDF_RE   | Receive FIFO Drain Request Enable. Enables the RFDF flag in the SR to generate a request.                           | 1 RFDF interrupt or DMA requests are enabled.
              + LSHIFT(1,   16) // RFDF_DIRS | Receive FIFO Drain DMA or Interrupt Request Select                                                                  | 1 DMA request.
//              + LSHIFT(0,   25) // TFFF_RE   | Transmit FIFO Fill Request Enable. Enables the TFFF flag in the SR to generate a request                            | 1 TFFF interrupts or DMA requests are enabled
//              + LSHIFT(0,   24) // TFFF_DIRS | Transmit FIFO Fill DMA or Interrupt Request Select. Selects between generating a DMA request or an interrupt request| 1 TFFF flag generates DMA requests.
//              + LSHIFT(0,   19) // RFOF_RE   | Receive FIFO Overflow Request Enable. Enables the RFOF flag in the SR to generate an interrupt request              | 1 RFOF interrupt requests are enabled.
//              + LSHIFT(0,   17) // RFDF_RE   | Receive FIFO Drain Request Enable. Enables the RFDF flag in the SR to generate a request.                           | 1 RFDF interrupt or DMA requests are enabled.
//              + LSHIFT(0,   16) // RFDF_DIRS | Receive FIFO Drain DMA or Interrupt Request Select                                                                  | 1 DMA request.


  ;


  SPI1_MCR &= ~BIT(HALT); // Запустить
}


void BSP_ClearFIFO_SPI1(void)
{
  SPI1_MCR |= BIT(11) | BIT(10);

}
/*------------------------------------------------------------------------------



 \param data
 \param len
 ------------------------------------------------------------------------------*/
void BSP_SPI_Write(uint8_t *data, uint32_t len)
{
  uint32_t i;

  for (i = 0; i < len; i++)
  {
    while ((SPI1_SR & BIT(25)) == 0); // Ожидаем флага готовности SPI принять данные для передачи
    SPI1_PUSHR = *data;
    data++;
    SPI1_SR = BIT(25);
  }

}

/*------------------------------------------------------------------------------



 \param void
 ------------------------------------------------------------------------------*/
void BSP_SPI_DMA_read_write(void)
{
  Set_SPI_CS_state(0);
  BSP_DMA_SPI_init_transfer();
  BSP_SPI_DMA_wait_transfer_done();
  Set_SPI_CS_state(1);
}
/*------------------------------------------------------------------------------
  Инициализация канала передачи по SPI с использованием DMA


 \param void
 ------------------------------------------------------------------------------*/
void BSP_SPI_DMA_test_write(void)
{



//***************************************
// Заполняем буфер тестовыми данными

  static uint32_t counter = 0;
  uint8_t *data;

  data = BSP_SPI_DMA_Get_tx_buf(  );
  memset(data, 0,  SPI_DMA_BUF_SZ);
  data[0] = 0x55;
  memcpy(&data[1], &counter, sizeof(counter));
  data[SPI_DMA_BUF_SZ - 1] = 0xAA;
//***************************************

  Set_SPI_CS_state(0);
  BSP_DMA_SPI_init_transfer();
  BSP_SPI_DMA_wait_transfer_done();
  Set_SPI_CS_state(1);

  counter++;


}
