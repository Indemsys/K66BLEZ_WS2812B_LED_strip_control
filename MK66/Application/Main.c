#define _MAIN_GLOBALS_
#include "App.h"



void Main_task(unsigned int initial_data);



const TASK_TEMPLATE_STRUCT  MQX_template_list[] =
{
  /* Task Index,     Function,              Stack,  Priority,                   Name,        Attributes,                                                          Param, Time Slice */
  { MAIN_TASK_IDX,      Main_task,          2000,   MAIN_TASK_PRIO,            "Main",       MQX_FLOATING_POINT_TASK + MQX_AUTO_START_TASK,                       0,     0 },
  { VT100_IDX,          Task_VT100,         2000,   VT100_ID_PRIO,             "VT100",      MQX_FLOATING_POINT_TASK + MQX_TIME_SLICE_TASK,                       0,     2 },
  { CAN_TX_IDX,         Task_CAN_Tx,        500,    CAN_TX_ID_PRIO,            "CAN_TX",     0,                                                                   0,     0 },
  { CAN_RX_IDX,         Task_CAN_Rx,        500,    CAN_RX_ID_PRIO,            "CAN_RX",     0,                                                                   0,     0 },
  { MKW40_IDX,          Task_MKW40,         1000,   MKW_ID_PRIO,               "MKW40",      MQX_TIME_SLICE_TASK,                                                 0,     2 },
  { FILELOG_IDX,        Task_file_log,      1500,   FILELOG_ID_PRIO,           "FileLog",    MQX_TIME_SLICE_TASK,                                                 0,     2 },
  { SHELL_IDX,          Task_shell,         2000,   SHELL_ID_PRIO,             "Shell",      MQX_TIME_SLICE_TASK,                                                 0,     2 },
  { SUPERVISOR_IDX,     Task_supervisor,    500,    SUPRVIS_ID_PRIO,           "SUPRVIS",    MQX_TIME_SLICE_TASK,                                                 0,     2 },
  { BACKGR_IDX,         Task_background,    1000,   BACKGR_ID_PRIO,            "BACKGR",     MQX_FLOATING_POINT_TASK,                                             0,     0 },
  { 0 }
};

volatile uint32_t tdelay, tmin, tmax;


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Main_task(unsigned int initial_data)
{
  unsigned int   cycle_cnt;
  uint32_t       rtc_time;
  TIME_STRUCT    mqx_time;

  // Устанавливаем время операционной системы
  //
  //
  //
  _rtc_get_time(&rtc_time);
  mqx_time.SECONDS = rtc_time;
  mqx_time.MILLISECONDS = 0;
  _time_set(&mqx_time); // Установка времени RTOS

  // Таймер не вызывает свою функцию. Причина не ясна
  // _timer_create_component(TIMERS_ID_PRIO, 500); 
  Set_LED_pattern(LED_BLINK, 0);


  AppLogg_init();
  Write_start_log_rec();
  Get_reference_time();

  Inputs_create_sync_obj();     // Создаем объект синхронизации задачи обслуживания входов до того как запустили измерения
  ADC_calibr_config_start();    // Инициализируем работу многоканального АЦП
  //FTM3_init_motor_PWM();      // Артефакт предыдущего приложения 
  //Init_DAC0();                // Артефакт предыдущего приложения 
  //Init_DAC1();                // Артефакт предыдущего приложения 
  //Init_QuadratureDecoder();   // Артефакт предыдущего приложения 

#ifdef MQX_MFS
  if (Init_mfs() == MQX_OK)
  {
    _task_create(0, FILELOG_IDX, 0);
  }
#endif

  Restore_parameters_and_settings();


  _task_create(0, BACKGR_IDX, 0);                  // Фоновая задача. Измеряет загрузку процессора
  _task_create(0, SUPERVISOR_IDX, 0);              // Задача сброса watchdog и наблюдения за работоспособностью системы
  WatchDog_init(WATCHDOG_TIMEOUT, WATCHDOG_WIN);   // Инициализируем Watchdog

// CAN не используем    
//  CAN_init(CAN0_BASE_PTR, CAN_SPEED);
//  _task_create(0, CAN_RX_IDX, (uint32_t)CAN_RX_PROC); // В качестве параметра передаем указатель на обработчик CAN пакетов
//  _task_create(0, CAN_TX_IDX, (uint32_t)CAN_TX_PROC);


  _task_create(0, MKW40_IDX, 0);

  Init_USB();
#ifdef USB_VT100
  _task_create(0, VT100_IDX, (uint32_t)Mnudrv_get_USB_vcom_driver1());       // Создаем задачу VT100_task с дайвером  интерфейса и делаем ее активной
#else
  _task_create(0, VT100_IDX, (uint32_t)Mnsdrv_get_ser_std_driver());         // Создаем задачу VT100_task с дайвером  интерфейса и делаем ее активной
#endif


  APPLICATION_TASK();

}


