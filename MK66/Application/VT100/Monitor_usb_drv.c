// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.06.23
// 11:31:24
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "usb_error.h"

#define  VCOM_PORT_NUM 2

static int Mnudrv_init(void **pcbl, void *pdrv);
static int Mnudrv_send_buf(const void *buf, unsigned int len);
static int Mnudrv_wait_ch(unsigned char *b, int ticks);
static int Mnudrv_printf(const char  *fmt_ptr, ...);
static int Mnudrv_deinit(void *pcbl);


static T_monitor_driver vcom_driver1 =
{
  MN_USB_VCOM_DRIVER0,
  Mnudrv_init,
  Mnudrv_send_buf,
  Mnudrv_wait_ch,
  Mnudrv_printf,
  Mnudrv_deinit,
};

static T_monitor_driver vcom_driver2 =
{
  MN_USB_VCOM_DRIVER1,
  Mnudrv_init,
  Mnudrv_send_buf,
  Mnudrv_wait_ch,
  Mnudrv_printf,
  Mnudrv_deinit,
};

#define MAX_STR_SZ  128
typedef struct
{
  uint8_t          str[MAX_STR_SZ];
} T_vcom_drv_cbl;



/*------------------------------------------------------------------------------
  Инициализация драйвера.
  Вызывается из задачи терминала при ее старте


 \param pcbl  -  указатель на указатель на внутреннюю управляющую структуру драйвера
 \param pdrv   - указатель  на структуру драйвера типа T_monitor_driver

 \return int
 ------------------------------------------------------------------------------*/
static int Mnudrv_init(void **pcbl, void *pdrv)
{
  _mqx_uint result;
  uint8_t   b;

  T_vcom_drv_cbl *p;

  // Выделяем память для управляющей структуры драйвера
  p = (T_vcom_drv_cbl *)_mem_alloc_zero(sizeof(T_vcom_drv_cbl));



  if ((((T_monitor_driver *)pdrv)->driver_type == MN_USB_VCOM_DRIVER0) || (((T_monitor_driver *)pdrv)->driver_type == MN_USB_VCOM_DRIVER1))
  {
    *pcbl = p; //  Устанавливаем в управляющей структуре драйвера задачи указатель на управляющую структуру драйвера
  }
  else
  {
    LOG("Incorrect driver type", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    _mem_free(p);
    return MQX_ERROR;
  }

  // Ждем подключения по  Virtual COM
  return Mnudrv_wait_ch(&b, INT_MAX);

}

/*------------------------------------------------------------------------------



 \param pcbl - указатель на внутреннюю управляющую структуру драйвера

 \return int
 ------------------------------------------------------------------------------*/
static int Mnudrv_deinit(void *pcbl)
{
  //  ToDo!!! Требуется реализация для 2-х virtual com

  // Освобождаем память упраляющей структуры
  _mem_free(pcbl);
  return MQX_OK;
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_monitor_driver* Mnudrv_get_USB_vcom_driver1(void)
{
  return &vcom_driver1;
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_monitor_driver* Mnudrv_get_USB_vcom_driver2(void)
{
  return &vcom_driver2;
}

/*-------------------------------------------------------------------------------------------------------------
  Вывод форматированной строки в коммуникационный канал порта
-------------------------------------------------------------------------------------------------------------*/
static int Mnudrv_send_buf(const void *buf, unsigned int len)
{
  //  ToDo!!! Требуется реализация для 2-х virtual com  T_vcom_drv_cbl *p;

//  T_monitor_cbl    *pvt100_cb;
//  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());
//  p = (T_vcom_drv_cbl *)(pvt100_cb->pdrvcbl);

  return Virtual_com_send_data((uint8_t *)buf, len, 100);
}

/*------------------------------------------------------------------------------



 \param b
 \param timeout

 \return int возвращает MQX_OK в случае состоявшегося приема байта
 ------------------------------------------------------------------------------*/
static int Mnudrv_wait_ch(unsigned char *b, int ticks)
{
  //  ToDo!!! Требуется реализация для 2-х virtual com

//  T_vcom_drv_cbl     *p;
//  T_monitor_cbl     *pvt100_cb;
//
//  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());
//  p = (T_vcom_drv_cbl *)(pvt100_cb->pdrvcbl);

  return Virtual_com_get_data(b, ticks);
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mnudrv_printf(const char  *fmt_ptr, ...)
{
  _mqx_int          res;
  va_list           ap;
  T_vcom_drv_cbl     *p;
  T_monitor_cbl     *pvt100_cb;

  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  p = (T_vcom_drv_cbl *)(pvt100_cb->pdrvcbl);

  va_start(ap, fmt_ptr);
  res = vsnprintf((char*)p->str, MAX_STR_SZ - 1, fmt_ptr, ap);
  va_end(ap);
  if (res < 0) return MQX_ERROR;

  res = Mnudrv_send_buf(p->str, res);

  return res;
}

