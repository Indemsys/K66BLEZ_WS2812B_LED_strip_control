// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2015.11.11
// 08:20:38
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

volatile unsigned int wdt_snapshot;
/*------------------------------------------------------------------------------
  ���������� �� watchdog-�

 \param user_isr_ptr
 ------------------------------------------------------------------------------*/
static void WDT_isr(void *user_isr_ptr)
{
  wdt_snapshot = WatchDog_get_timeout_reg();
}

/*------------------------------------------------------------------------------
  ��������� ���������� �� watchdog

 ------------------------------------------------------------------------------*/
void ivINT_Watchdog(void)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;

  wdt_snapshot = WatchDog_get_timeout_reg();

  WDOG->UNLOCK = 0xC520; // ������� ������ �� ������ � ������� ���������� WDOG
  WDOG->UNLOCK = 0xD928;
  __no_operation();
  WDOG->STCTRLL = BIT(15);
}
/*------------------------------------------------------------------------------
  �������������� watchdog

    ��������� ������ WDT.
    ��������� �� LPO (1 kHz).
    ����� ����� �������������� ��� ���������������� ����������
    �������� ����� ����.

 \param wdt_timeout - ����� (� �� )�� ��������� �������� watchdog �������� ����� �������
 \param wdt_win     - ����� (� ��) �� ��������� ��������� ������ ������ refresh watchdog-�
 ------------------------------------------------------------------------------*/
void WatchDog_init(unsigned int wdt_timeout, unsigned int wdt_win)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;

  _int_disable();

  _bsp_int_init(INT_Watchdog, WDT_ISR_PRIO, 0, TRUE);

  WDOG->UNLOCK = 0xC520; // ������� ������ �� ������ � ������� ���������� WDOG
  WDOG->UNLOCK = 0xD928;
  __no_operation();

  WDOG->STCTRLH = 0
                  + LSHIFT(0x01, 14) // DISTESTWDOG | Allows the WDOG�s functional test mode to be disabled permanently| 0 WDOG functional test mode is not disabled.
                  + LSHIFT(0x00, 12) // BYTESEL[1:0]| This 2-bit field select the byte to be tested ...                | 00 Byte 0 selected
                  + LSHIFT(0x00, 11) // TESTSEL     | Selects the test to be run on the watchdog timer                 | 0 Quick test
                  + LSHIFT(0x00, 10) // TESTWDOG    | Puts the watchdog in the functional test mode                    |
                  + LSHIFT(0x01, 8)  // Reserved    |
                  + LSHIFT(0x01, 7)  // WAITEN      | Enables or disables WDOG in wait mode.                           | 1 WDOG is enabled in CPU wait mode.
                  + LSHIFT(0x01, 6)  // STOPEN      | Enables or disables WDOG in stop mode                            | 1 WDOG is enabled in CPU stop mode.
                  + LSHIFT(0x00, 5)  // DBGEN       | Enables or disables WDOG in Debug mode                           | 0 WDOG is disabled in CPU Debug mode.
                  + LSHIFT(0x01, 4)  // ALLOWUPDATE | Enables updates to watchdog write once registers                 | 1 WDOG write once registers can be unlocked for updating
                  + LSHIFT(0x01, 3)  // WINEN       | Enable windowing mode.                                           | 0 Windowing mode is disabled.
                  + LSHIFT(0x01, 2)  // IRQRSTEN    | Used to enable the debug breadcrumbs feature                     | 0 WDOG time-out generates reset only.
                  + LSHIFT(0x00, 1)  // CLKSRC      | Selects clock source for the WDOG                                | 1 WDOG clock sourced from alternate clock source
                  + LSHIFT(0x01, 0)  // WDOGEN      | Enables or disables the WDOG�s operation                         | 1 WDOG is enabled.
  ;
  // ������������ ������������� � 0
  WDOG->PRESC = 0;

  // ������������� ����-��� WDT
  WDOG->TOVALH = (wdt_timeout >> 16);
  WDOG->TOVALL = wdt_timeout & 0xFFFF;

  // ������������� ����-��� WDT
  WDOG->WINH = (wdt_win >> 16);
  WDOG->WINL = wdt_win & 0xFFFF;


  _int_enable();

}

/*------------------------------------------------------------------------------
  ����� ������� WDT
 ------------------------------------------------------------------------------*/
void WatchDog_refresh(void)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;

  // �� ����� ������ WDT ������ ���� ��������� ��� ���������� � RTOS � ������ ����
  __disable_interrupt();
  WDOG->REFRESH = 0xA602;
  WDOG->REFRESH = 0xB480;
  __enable_interrupt();
}

/*------------------------------------------------------------------------------
  �������� ������� ������� ��������������� WDT
 ------------------------------------------------------------------------------*/
unsigned short WatchDog_get_counter(void)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;
  return WDOG->RSTCNT;
}

/*------------------------------------------------------------------------------
  �������� ������� �������� WDT
 ------------------------------------------------------------------------------*/
unsigned int WatchDog_get_timeout_reg(void)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;
  return (WDOG->TMROUTH << 16) | WDOG->TMROUTL ;
}
