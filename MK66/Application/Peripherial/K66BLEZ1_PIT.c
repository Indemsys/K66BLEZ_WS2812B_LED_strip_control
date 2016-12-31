#include "App.h"

/*-------------------------------------------------------------------------------------------------------------
  ������������� ������ Periodic Interrupt Timer (PIT)
  ���� �� ������� PIT ������������ ��� �������� �������� �������� ��� ������ ADC

  ����������� ������ �� Bus Clock -> CPU_BUS_CLK_HZ = 60 ���
-------------------------------------------------------------------------------------------------------------*/
void Init_PIT_module(void)
{
  SIM_MemMapPtr  SIM  = SIM_BASE_PTR;
  PIT_MemMapPtr  PIT = PIT_BASE_PTR;


  SIM->SCGC6 |= BIT(23); // PIT | PIT clock gate control | 1 Clock is enabled.
  PIT->MCR = 0
    + LSHIFT(0, 1) // MDIS | Module Disable  | 1 Clock for PIT Timers is disabled.
    + LSHIFT(1, 0) // FRZ  | Freeze          | 1 Timers are stopped in debug mode.
;
}

/*-------------------------------------------------------------------------------------------------------------
   ������������� ������� 0 ������ PIT
   ������������ ��� ������� ADC

   period - ������ ������ ������� � ���

   ����������� ������ �� Bus Clock -> CPU_BUS_CLK_HZ = 60 ���
 -------------------------------------------------------------------------------------------------------------*/
void Init_PIT0(uint32_t period)
{
  PIT_MemMapPtr PIT = PIT_BASE_PTR;

  // Timer Load Value Register
  PIT->CHANNEL[0].LDVAL = ((CPU_BUS_CLK_HZ/1000000ul) * period)/ADC_SCAN_SZ; // ��������� ������ �������

  PIT->CHANNEL[0].TCTRL = 0
    + LSHIFT(0, 1) // TIE | Timer Interrupt Enable Bit. | 0 Interrupt requests from Timer n are disabled.
    + LSHIFT(1, 0) // TEN | Timer Enable Bit.           | 1 Timer n is active.
;
}


/*-------------------------------------------------------------------------------------------------------------
   �������� ������� �������� �������. ������ ����� ���� �� 0 � ��������� ���� �������� ��������� �� PIT->CHANNEL[0].LDVAL
 -------------------------------------------------------------------------------------------------------------*/
uint32_t Get_PIT0_curr_val(void)
{
  PIT_MemMapPtr PIT = PIT_BASE_PTR;
  return PIT->CHANNEL[0].CVAL;
}

/*-------------------------------------------------------------------------------------------------------------
   �������� �����������  �������� �������.
 -------------------------------------------------------------------------------------------------------------*/
uint32_t Get_PIT0_load_val(void)
{
  PIT_MemMapPtr PIT = PIT_BASE_PTR;
  return PIT->CHANNEL[0].LDVAL;
}


