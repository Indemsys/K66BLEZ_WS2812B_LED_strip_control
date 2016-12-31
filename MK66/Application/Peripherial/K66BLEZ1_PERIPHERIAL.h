#ifndef __K66BLEZ_H
  #define __K66BLEZ_H

#include "K66BLEZ1_INIT_SYS.h"
#include "K66BLEZ1_ADC.h"
#include "K66BLEZ1_DMA.h"
#include "K66BLEZ1_FTM.h"
#include "K66BLEZ1_DAC.h"
#include "K66BLEZ1_PIT.h"
#include "K66BLEZ1_SPI.h"
#include "K66BLEZ1_CAN.h"
#include "K66BLEZ1_VBAT_RAM.h"
#include "K66BLEZ1_MKW40_Channel.h"

// ����� ������� � ���� ����� ��� ��������� ���� ������� DMA 
// ��������� DMA ��������:
//   - ����� ������ DMA. ������� ����� ���� 32. ����� ���������� �����������, �� � ������� ���� ���������� 
//   - ����� �������������� DMA ���� �� � ���� ������ ������ 
//   - ����� ����� � ��������������. ����� ��������������� ������ ��������� � ������������ ���������
//   - ����� ������ ���������� DMA ���� ������������ ��������� ������� DMA ��� ������ �������� ������  
//


// -----------------------------------------------------------------------------
//  ����������� ���������������� ������� DMA � DMA MUX ��� ������ � ADC
#define ADC_SCAN_SZ       4

#define DMA_ADC0_RES_CH   20               // ������� ����� DMA ��� ������������ ADC
#define DMA_ADC1_RES_CH   21               // ������� ����� DMA ��� ������������ ADC
#define DMA_ADC0_CFG_CH   16               // ������� ����� DMA ��� ������������ ADC
#define DMA_ADC1_CFG_CH   17               // ������� ����� DMA ��� ������������ ADC
#define DMA_ADC_DMUX_PTR  DMAMUX_BASE_PTR  // ��������� �� ������ DMUX ������� ������������ ��� �������� �������� �� ADC � DMA
#define DMA_ADC0_DMUX_SRC DMUX_SRC_ADC0    // ����� DMUX ������������ ��� ���������� ADC
#define DMA_ADC1_DMUX_SRC DMUX_SRC_ADC1    // ����� DMUX ������������ ��� ���������� ADC
#define DMA_ADC_INT_NUM   INT_DMA5_DMA21   // ����� ������� ���������� ������������ � DMA ��� ������������ ADC

// -----------------------------------------------------------------------------
//  ����������� ���������������� ������� DMA � DMA MUX ��� ������ � ����������� SPI ���� MKW40

#define MKW40_SPI              SPI2              // ����� SPI ������ ������������ ��� ������������ � ������� MKW40
#define DMA_MKW40_FM_CH        0                 // ����� DMA ��� ������������ ������ ��   SPI. FIFO->������.
#define DMA_MKW40_MF_CH        1                 // ����� DMA ��� ������������ �������� �� SPI. ������->FIFO.
#define MKW40_SPI_CS           0                 // ����� ����������� �������� ������� CS ��� ���������� MKW40 SPI
#define DMA_MKW40_DMUX_PTR     DMAMUX_BASE_PTR  // ��������� �� ������ DMUX ������� ������������ ��� �������� �������� �� ����������� SPI � DMA
#define DMA_MKW40_DMUX_TX_SRC  DMUX_SRC_FTM3_CH7_SPI2_TX // ���� DMUX ������������ ��� ������ �������� �� DMA
#define DMA_MKW40_DMUX_RX_SRC  DMUX_SRC_FTM3_CH6_SPI2_RX // ���� DMUX ������������ ��� ������ ������ �� DMA

#define DMA_MKW40_RX_INT_NUM   INT_DMA0_DMA16    // ����� ������� ���������� ������������ � DMA ��� ������������ ������ �� SPI
#define DMA_MKW40_ISR          DMA0_DMA16_IRQHandler

// -----------------------------------------------------------------------------
//  ����������� ���������������� ������� DMA � DMA MUX ��� ������ � ����������� SPI ���������� DRV8305 � ������� DMC01

#define DRV8305_SPI             SPI1              // ����� SPI ������ ������������ ��� ������������ � DRV8305
#define DMA_DRV8305_FM_CH       2                 // ����� DMA ��� ������������ ������ ��   SPI. FIFO->������.
#define DMA_DRV8305_MF_CH       3                 // ����� DMA ��� ������������ �������� �� SPI. ������->FIFO.
#define DRV8305_SPI_CS          0                 // ����� ����������� �������� ������� CS ��� ���������� LDC1000
#define DMA_DRV8305_DMUX_PTR    DMAMUX_BASE_PTR  // ��������� �� ������ DMUX ������� ������������ ��� �������� �������� �� ����������� SPI � DMA
#define DMA_DRV8305_DMUX_TX_SRC DMUX_SRC_SPI1_TX // ���� DMUX ������������ ��� ������ �������� �� DMA
#define DMA_DRV8305_DMUX_RX_SRC DMUX_SRC_SPI1_RX // ���� DMUX ������������ ��� ������ ������ �� DMA

#define DMA_DRV8305_RX_INT_NUM  INT_DMA2_DMA18    // ����� ������� ���������� ������������ � DMA ��� ������������ ������ �� SPI
#define DMA_DRV8305_ISR         DMA2_DMA18_IRQHandler

// -----------------------------------------------------------------------------
//  ����������� ���������������� ������� DMA � DMA MUX ��� ������ � �� ����� � LED ����� �� ������ WS2812B
//  ���������� DMA �� ������������ 

#define DMA_WS2812B_DMUX_PTR    DMAMUX_BASE_PTR   // ��������� �� ������ DMUX ������� ������������ ��� �������� �������� �� ����������� SPI � DMA
#define DMA_WS2812B_DMUX_SRC    DMUX_SRC_FTM0_CH2 // ���� DMUX ������������ ��� ������ �������� �� DMA
#define DMA_WS2812B_CH          4                 // ����� DMA ��� ������������ �������� � WS2812B


#ifdef ADC_GLOBAL

T_ADC_res   adcs;

#else

extern T_ADC_res   adcs;

#endif


void Set_MKW40_CS_state(int state);


#endif
