/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if !defined(__BOARD_H__)
#define __BOARD_H__

#include <stdint.h>
#include <string.h>
#include "pin_mux.h"
#include "BSP.h"


/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

/* The board name */
#define BOARD_NAME                      "K66BLEZv1"

#define CLOCK_VLPR 1U
#define CLOCK_RUN  2U
#define CLOCK_NUMBER_OF_CONFIGURATIONS 3U

#ifndef CLOCK_INIT_CONFIG
#define CLOCK_INIT_CONFIG CLOCK_RUN
#endif

#if (CLOCK_INIT_CONFIG == CLOCK_RUN)
#define CORE_CLOCK_FREQ 32000000U
#else
#define CORE_CLOCK_FREQ 4000000U
#endif

#define BOARD_USE_LPUART 1

/* Connectivity */
#ifndef APP_SERIAL_INTERFACE_TYPE
#define APP_SERIAL_INTERFACE_TYPE (gSerialMgrLpuart_c)
#endif

#ifndef APP_SERIAL_INTERFACE_INSTANCE
#define APP_SERIAL_INTERFACE_INSTANCE (0)
#endif

/* EXTAL0 PTA18 */
#define EXTAL0_PORT   PORTB
#define EXTAL0_PIN    16
#define EXTAL0_PINMUX kPortPinDisabled

/* XTAL0 PTA19 */
#define XTAL0_PORT   PORTB
#define XTAL0_PIN    17
#define XTAL0_PINMUX kPortPinDisabled

/* RTC external clock configuration. */
#define RTC_XTAL_FREQ   32768U
#define RTC_SC2P_ENABLE_CONFIG       false
#define RTC_SC4P_ENABLE_CONFIG       false
#define RTC_SC8P_ENABLE_CONFIG       false
#define RTC_SC16P_ENABLE_CONFIG      false
#define RTC_OSC_ENABLE_CONFIG        true

/* RTC_CLKIN PTB16 */
#define RTC_CLKIN_PORT   PORTB
#define RTC_CLKIN_PIN    16
#define RTC_CLKIN_PINMUX kPortMuxAsGpio

/* The UART to use for debug messages. */
#ifndef BOARD_DEBUG_UART_INSTANCE
    #define BOARD_DEBUG_UART_INSTANCE   0
    #define BOARD_DEBUG_UART_BASEADDR   LPUART0
#endif
#ifndef BOARD_DEBUG_UART_BAUD
    #define BOARD_DEBUG_UART_BAUD       115200
#endif
#ifndef BOARD_LPUART_CLOCK_SOURCE
    #define BOARD_LPUART_CLOCK_SOURCE   kClockLpuartSrcMcgFllClk
#endif



#define BOARD_I2C_GPIO_SCL              GPIO_MAKE_PIN(GPIOC_IDX, 2)
#define BOARD_I2C_GPIO_SDA              GPIO_MAKE_PIN(GPIOC_IDX, 3)



/* The i2c instance used for i2c connection by default */
#define BOARD_I2C_INSTANCE              1

/* The dspi instance used for dspi example */
#define BOARD_DSPI_INSTANCE             1

/* The TPM instance/channel used for board */
#define BOARD_TPM_INSTANCE              0
#define BOARD_TPM_CHANNEL               1




/* The rtc instance used for rtc_func */
#define BOARD_RTC_FUNC_INSTANCE         0

#define BOARD_LTC_INSTANCE              0

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

void hardware_init(void);
void dbg_uart_init(void);
/* Function to initialize clock base on board configuration. */
void BOARD_ClockInit(void);

/* Function to initialize OSC0 base on board configuration. */
void BOARD_InitOsc0(void);

/* Function to initialize RTC external clock base on board configuration. */
void BOARD_InitRtcOsc(void);


#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __BOARD_H__ */
