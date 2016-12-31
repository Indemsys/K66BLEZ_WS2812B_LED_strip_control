// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.09.06
// 22:51:23
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "BSP.h"


/*------------------------------------------------------------------------------
  Инициализация Periodic interrupt timer (PIT) в связанном режиме
  Модуль тактируется от 16 МГц

 \param void
 ------------------------------------------------------------------------------*/
void BSP_Init_PIT(void)
{
  SIM_SCGC6 |= BIT(23);  // Разрешить тактирование PIT


  PIT_MCR = 0x00; // Вллючаем модуль PIT
                  // Инициализируем Timer 1
  PIT_LDVAL1 = 0xFFFFFFFF; // setup timer 1 for maximum counting period
  PIT_TCTRL1 = 0x0; // disable timer 1 interrupts
  PIT_TCTRL1 |= BIT(2); // chain timer 1 to timer 0
  PIT_TCTRL1 |= BIT(0); // start timer 1
                        // Инициализируем Timer 0
  PIT_LDVAL0 = 0xFFFFFFFF; // setup timer 0 for maximum counting period
  PIT_TCTRL0 = BIT(0); // start timer 0
}


/*------------------------------------------------------------------------------
 Прочитать 64-х битный счетчик PIT
 Счетчик тактируется от 16 МГц

 \param void

 \return uint64_t
 ------------------------------------------------------------------------------*/
uint64_t BSP_Read_PIT(void)
{
  uint64_t val;
  val = (uint64_t)PIT_LTMR64H << 32;
  val = val + PIT_LTMR64L;
  return 0xFFFFFFFFFFFFFFFFull - val;
}
