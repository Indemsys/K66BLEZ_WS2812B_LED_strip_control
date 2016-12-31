// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-06
// 15:30:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define EVENT_START           BIT( 0 )
#define EVENT_STOP            BIT( 1 )


extern uint32_t  enable_led_strip;


static void LEDSC_cmd_receiver(uint8_t *data, uint32_t sz, void *ptr);
static LWEVENT_STRUCT player_lwev;

/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
_mqx_uint LEDSC_create_sync_obj(void)
{
  _mqx_uint res;

  if (player_lwev.VALID != LWEVENT_VALID)
  {
    res = _lwevent_create(&player_lwev, LWEVENT_AUTO_CLEAR); // ��� ������� ������������� ������������
  }
  return res;
}



/*------------------------------------------------------------------------------


 \return _mqx_uint
 ------------------------------------------------------------------------------*/
void LEDSC_set_events(uint32_t evt)
{
  _lwevent_set(&player_lwev, evt);
}
/*-----------------------------------------------------------------------------------------------------
  �������� �������
  ���� ticks = 0 �� ���� ����������
-----------------------------------------------------------------------------------------------------*/
_mqx_uint LEDSC_wait_events(uint32_t evt, uint32_t ticks)
{
  return _lwevent_wait_ticks(&player_lwev, evt, FALSE, ticks);
}

/*------------------------------------------------------------------------------
 ����� ������� � ���������� ������� �������� �������


 \param evt    - ��������� �� ��������� ������� � ���������
 \param ticks

 \return _mqx_uint
 ------------------------------------------------------------------------------*/
_mqx_uint LEDSC_wait_get_events(uint32_t *pevt, uint32_t ticks)
{
  _mqx_uint res;
  res = _lwevent_wait_ticks(&player_lwev, *pevt, FALSE, ticks);
  *pevt = _lwevent_get_signalled();
  return res;
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void LEDSC_task(void)
{
  uint32_t evt;

  LEDSC_create_sync_obj();
  FTM_init_PWM_DMA(FTM0_BASE_PTR); // �������������� PWM ��������� ��� ������ �� ������������ ������ �� WS2812B
  WS2812B_Demo_DMA();

  MKW40_subscibe(MKW40_SUBS_CMDMAN, LEDSC_cmd_receiver, 0);

  enable_led_strip =1;
  do
  {
    evt = EVENT_START + EVENT_STOP;
    LEDSC_wait_get_events(&evt, 0); // ������� ����� ���������

    if (evt & EVENT_START)
    {
      enable_led_strip = 1; 
    }
    if (evt & EVENT_STOP)
    {
      enable_led_strip = 0; 
    }
  }
  while (1);

}



/*------------------------------------------------------------------------------
  �������� ������ �� ������ MKW40
  ���������� � ��������� ������ Task_MKW40


 \param data
 \param sz
 \param ptr - �������������� ��������� ������������ ��� ��������
 ------------------------------------------------------------------------------*/
static void LEDSC_cmd_receiver(uint8_t *data, uint32_t sz, void *ptr)
{
  uint32_t cmd;
  // �������������� ��������� �������

  if (sz == 4) memcpy(&cmd, data, 4);
  else cmd = 0;

  switch (cmd)
  {
  case CMD_START:
    LEDSC_set_events(EVENT_START);
    break;

  case CMD_STOP:
    LEDSC_set_events(EVENT_STOP);
    break;

  case 0:
    // ������ �������

    break;
  }
}

