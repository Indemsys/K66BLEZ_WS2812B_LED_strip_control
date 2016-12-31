// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.09.06
// 22:51:23
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "BSP.h"


/*------------------------------------------------------------------------------
  ������������� Periodic interrupt timer (PIT) � ��������� ������
  ������ ����������� �� 16 ���

 \param void
 ------------------------------------------------------------------------------*/
void BSP_Init_PIT(void)
{
  SIM_SCGC6 |= BIT(23);  // ��������� ������������ PIT


  PIT_MCR = 0x00; // �������� ������ PIT
                  // �������������� Timer 1
  PIT_LDVAL1 = 0xFFFFFFFF; // setup timer 1 for maximum counting period
  PIT_TCTRL1 = 0x0; // disable timer 1 interrupts
  PIT_TCTRL1 |= BIT(2); // chain timer 1 to timer 0
  PIT_TCTRL1 |= BIT(0); // start timer 1
                        // �������������� Timer 0
  PIT_LDVAL0 = 0xFFFFFFFF; // setup timer 0 for maximum counting period
  PIT_TCTRL0 = BIT(0); // start timer 0
}


/*------------------------------------------------------------------------------
 ��������� 64-� ������ ������� PIT
 ������� ����������� �� 16 ���

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
