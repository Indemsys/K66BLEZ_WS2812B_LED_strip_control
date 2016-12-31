// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.09.11
// 13:45:59
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "BSP.h"

typedef struct
{
  GPIO_MemMapPtr gpio;
  PORT_MemMapPtr port;
  unsigned char  pin_num;
  unsigned char  irqc; //  Interrupt Configuration
                       //  0000 Interrupt/DMA Request disabled.
                       //  0001 DMA Request on rising edge.
                       //  0010 DMA Request on falling edge.
                       //  0011 DMA Request on either edge.
                       //  0100 Reserved.
                       //  1000 Interrupt when logic zero.
                       //  1001 Interrupt on rising edge.
                       //  1010 Interrupt on falling edge.
                       //  1011 Interrupt on either edge.
                       //  1100 Interrupt when logic one.
  unsigned char  lock; //  if 1 Pin Control Register bits [15:0] are locked and cannot be updated until the next System Reset.
  unsigned char  mux;  //  Pin Mux Control
                       //  000 Pin Disabled (Analog).
                       //  001 Alternative 1 (GPIO).
                       //  010 Alternative 2 (chip specific).
                       //  011 Alternative 3 (chip specific).
                       //  100 Alternative 4 (chip specific).
                       //  101 Alternative 5 (chip specific).
                       //  110 Alternative 6 (chip specific).
                       //  111 Alternative 7 (chip specific / JTAG / NMI).
  unsigned char  DSE; // 0 Low drive strength is configured on the corresponding pin, if pin is configured as a digital output.
                      // 1 High drive strength is configured on the corresponding pin, if pin is configured as a digital output.
  unsigned char  SRE;  // 0 Fast slew rate is configured on the corresponding pin, if pin is configured as a digital output.
                       // 1 Slow slew rate is configured on the corresponding pin, if pin is configured as a digital output.
  unsigned char  ODE;  // 0 Open Drain output is disabled on the corresponding pin.
                       // 1 Open Drain output is enabled on the corresponding pin, provided pin is configured as a digital output.
  unsigned char  PFE;  // 0 Passive Input Filter is disabled on the corresponding pin.
                       // 1 Passive Input Filter is enabled on the corresponding pin.
  unsigned char  PUPD; // 00 Internal pull-up or pull-down resistor is not enabled on the corresponding pin.
                       // 10 Internal pull-down resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.
                       // 11 Internal pull-up resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.
  unsigned char  dir;  // 0 Pin is configured as general purpose input, if configured for the GPIO function
                       // 1 Pin is configured for general purpose output, if configured for the GPIO function
  unsigned char  init; // Init state

} T_IO_pins_configuration;

#define   ANAL          0  // Pin Disabled (Analog).
#define   ALT0          0  // Pin Disabled (Analog).
#define   GPIO          1  // Alternative 1 (GPIO).
#define   ALT1          1  // Alternative 1 (GPIO).
#define   ALT2          2  //
#define   ALT3          3  //
#define   ALT4          4  //
#define   ALT5          5  //
#define   ALT6          6  //
#define   ALT7          7  //

#define   DSE_LO        0 // 0 Low drive strength is configured on the corresponding pin, if pin is configured as a digital output.
#define   DSE_HI        1 // 1 High drive strength is configured on the corresponding pin, if pin is configured as a digital output.

#define   OD_DIS        0 // 0 Open Drain output is disabled on the corresponding pin.
#define   OD__EN        1 // 1 Open Drain output is enabled on the corresponding pin, provided pin is configured as a digital output.

#define   PFE_DIS       0 // 0 Passive Input Filter is disabled on the corresponding pin.
#define   PFE__EN       1 // 1 Passive Input Filter is enabled on the corresponding pin.

#define   FAST_SLEW     0 // 0 Fast slew rate is configured on the corresponding pin, if pin is configured as a digital output.
#define   SLOW_SLEW     1 // 1 Slow slew rate is configured on the corresponding pin, if pin is configured as a digital output.


#define   PUPD_DIS      0 // 00 Internal pull-up or pull-down resistor is not enabled on the corresponding pin.
#define   PULL__DN      2 // 10 Internal pull-down resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.
#define   PULL__UP      3 // 11 Internal pull-up resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.

#define   GP_INP        0 // 0 Pin is configured as general purpose input, if configured for the GPIO function
#define   GP_OUT        1 // 1 Pin is configured for general purpose output, if configured for the GPIO function

#define   IRQ_DIS       0  // Interrupt Status Flag (ISF) is disabled.
#define   IRQ_IDR       1  // ISF flag and DMA request on rising edge.
#define   IRQ_IDF       2  // ISF flag and DMA request on falling edge.
#define   IRQ_IDE       3  // ISF flag and DMA request on either edge.
#define   IRQ_IL0       8  // ISF flag and Interrupt when logic 0.
#define   IRQ_IRE       9  // ISF flag and Interrupt on rising-edge.
#define   IRQ_IFE       10 // ISF flag and Interrupt on falling-edge.
#define   IRQ_IEE       11 // ISF flag and Interrupt on either edge.
#define   IRQ_IL1       12 // ISF flag and Interrupt when logic 1.


static const T_IO_pins_configuration SPI1_pins_conf[] =
{
  { GPIOA_BASE_PTR, PORTA_BASE_PTR,   16,   IRQ_DIS,   0,   ALT2, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //  PTA16 - SOUT
  { GPIOA_BASE_PTR, PORTA_BASE_PTR,   17,   IRQ_DIS,   0,   ALT2, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //  PTA17 - SIN
  { GPIOA_BASE_PTR, PORTA_BASE_PTR,   18,   IRQ_DIS,   0,   ALT2, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //  PTA18 - SCK
  { GPIOA_BASE_PTR, PORTA_BASE_PTR,   19,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   1 }, //  PTA19 - PCS0
  { GPIOC_BASE_PTR, PORTC_BASE_PTR,   7 ,   IRQ_DIS,   0,   GPIO, DSE_HI, FAST_SLEW, OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   1 }, //  PTC7  - тест или URT0_TX
};



/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void Config_pin(const T_IO_pins_configuration pinc)
{
  pinc.port->PCR[pinc.pin_num] = LSHIFT(pinc.irqc, 16) |
                                 LSHIFT(pinc.lock, 15) |
                                 LSHIFT(pinc.mux, 8) |
                                 LSHIFT(pinc.DSE, 6) |
                                 LSHIFT(pinc.ODE, 5) |
                                 LSHIFT(pinc.PFE, 4) |
                                 LSHIFT(pinc.SRE, 2) |
                                 LSHIFT(pinc.PUPD, 0);

  if (pinc.init == 0) pinc.gpio->PCOR = LSHIFT(1, pinc.pin_num);
  else pinc.gpio->PSOR = LSHIFT(1, pinc.pin_num);
  pinc.gpio->PDDR = (pinc.gpio->PDDR & ~LSHIFT(1, pinc.pin_num)) | LSHIFT(pinc.dir, pinc.pin_num);
}

/*------------------------------------------------------------------------------



 \param void
 ------------------------------------------------------------------------------*/
void BSP_Init_SPI1_Pins(void)
{
  int i;

  for (i = 0; i < (sizeof(SPI1_pins_conf) / sizeof(SPI1_pins_conf[0])); i++)
  {
    Config_pin(SPI1_pins_conf[i]);
  }
}

/*------------------------------------------------------------------------------



 \param state
 ------------------------------------------------------------------------------*/
void Set_SPI_CS_state(int state)
{
  if (state == 0) GPIOA->PCOR = LSHIFT(1, 19); // Port Clear Output Register
  else GPIOA->PSOR = LSHIFT(1, 19);            // Port Set Output Register
}

/*------------------------------------------------------------------------------



 \param state
 ------------------------------------------------------------------------------*/
void Set_PTC7_state(int state)
{
  if (state == 0) GPIOC->PCOR = LSHIFT(1, 7); // Port Clear Output Register
  else GPIOC->PSOR = LSHIFT(1, 7);            // Port Set Output Register
}
