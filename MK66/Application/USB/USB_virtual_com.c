#include "App.h"
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device_stack_interface.h"
#include "USB_Virtual_com.h"
#include "usb_descriptor.h"



#if HIGH_SPEED
  #define CONTROLLER_ID         USB_CONTROLLER_EHCI_0
  #define DATA_BUFF_SIZE        (HS_DIC_BULK_OUT_ENDP_PACKET_SIZE)
#else
  #define CONTROLLER_ID         USB_CONTROLLER_KHCI_0
  #define DATA_BUFF_SIZE        (FS_DIC_BULK_OUT_ENDP_PACKET_SIZE)
#endif


#define VCOM_FLAG_RECEIVED    BIT( 0 )
#define VCOM_FLAG_TRANSMITED  BIT( 1 )

/* Implementation Specific Macros */
#define LINE_CODING_SIZE              (0x07)
#define COMM_FEATURE_DATA_SIZE        (0x02)

#define LINE_CODE_DTERATE_IFACE      (115200) /*e.g 9600 is 0x00002580 */
#define LINE_CODE_CHARFORMAT_IFACE   (0x00)   /* 1 stop bit */
#define LINE_CODE_PARITYTYPE_IFACE   (0x00)  /* No Parity */
#define LINE_CODE_DATABITS_IFACE     (0x08)  /* Data Bits Format */

#define STATUS_ABSTRACT_STATE_IFACE  (0x0000) /* Disable Multiplexing ENDP in this interface will continue to accept/offer data*/
#define COUNTRY_SETTING_IFACE        (0x0000) /* Country Code in the format as defined in [ISO3166]-- PLEASE CHECK THESE VALUES*/



#if USBCFG_DEV_COMPOSITE
  #error This application requires USBCFG_DEV_COMPOSITE defined zero in usb_device_config.h. Please recompile usbd with this option.
#endif

extern   usb_desc_request_notify_struct_t   desc_callback;
extern   uint8_t                            USB_Desc_Set_Speed(uint32_t handle, uint16_t speed);

cdc_handle_t                                g_app_handle;


void USB_App_Device_Callback(uint8_t event_type, void *val, void *arg);
uint8_t USB_App_Class_Callback(uint8_t event, uint16_t value, uint8_t **data, uint32_t *size, void *arg);


uint8_t g_line_coding[LINE_CODING_SIZE] =
{
  /*e.g. 0x00,0x10,0x0E,0x00 : 0x000E1000 is 921600 bits per second */
  (LINE_CODE_DTERATE_IFACE >> 0) & 0x000000FF,
  (LINE_CODE_DTERATE_IFACE >> 8) & 0x000000FF,
  (LINE_CODE_DTERATE_IFACE >> 16) & 0x000000FF,
  (LINE_CODE_DTERATE_IFACE >> 24) & 0x000000FF,
  LINE_CODE_CHARFORMAT_IFACE,
  LINE_CODE_PARITYTYPE_IFACE,
  LINE_CODE_DATABITS_IFACE
};

uint8_t g_abstract_state[COMM_FEATURE_DATA_SIZE] =
{
  (STATUS_ABSTRACT_STATE_IFACE >> 0) & 0x00FF,
  (STATUS_ABSTRACT_STATE_IFACE >> 8) & 0x00FF
};

uint8_t g_country_code[COMM_FEATURE_DATA_SIZE] =
{
  (COUNTRY_SETTING_IFACE >> 0) & 0x00FF,
  (COUNTRY_SETTING_IFACE >> 8) & 0x00FF
};
static bool vport_need_change_speed = FALSE; // Флаг необходимости корректировки скорости при считывании дескриптора
static bool vport_configured = FALSE; // Флаг выполненного конфигурирования COM  порта
static bool vport_opened = FALSE;     // Флаг открытого для коммуникации виртуального порта

// Ведем прием циклически в N приемных буферов
typedef struct
{
  uint8_t   *buff; // Буфер с пакетов
  uint32_t  len;  // Длина пакета
  uint32_t  pos;  // Текущая позиция считывания байта

} T_recv_cbl;


#define  IN_BUF_QUANTITY   2
static T_recv_cbl          rcbl[IN_BUF_QUANTITY]; //  Массив управляющих структур приема-обработки входящих пакетов
static uint8_t            *g_recv_buf;            //  Указатель на область с буферами приема
static volatile uint8_t    g_recv_head_indx;      //  Индекс головы циклической очереди буферов приема
static volatile uint8_t    g_recv_tail_indx;      //  Индекс головы циклической очереди буферов приема

static uint32_t            g_recv_error_cnt;      //  Счетчик ошибок процедур приема
static volatile uint32_t   g_recv_buf_queue_full; //  Флаг заполненнной очереди приемных буфферов

static LWEVENT_STRUCT      vcm_lwev;              //  Флаг выполненного приема пакета

static uint8_t            *g_curr_send_buf;

static uint16_t            g_cdc_device_speed;
static uint16_t            g_bulk_out_max_packet_size;
static uint16_t            g_bulk_in_max_packet_size;


/*****************************************************************************
 * Local Functions
 *****************************************************************************/

/**************************************************************************//*!
 *
 * @name  USB_Get_Line_Coding
 *
 * @brief The function returns the Line Coding/Configuration
 *
 * @param handle:        handle
 * @param interface:     interface number
 * @param coding_data:   output line coding data
 *
 * @return USB_OK                              When Success
 *         USBERR_INVALID_REQ_TYPE             when Error
 *****************************************************************************/
uint8_t USB_Get_Line_Coding(uint32_t handle, uint8_t interface, uint8_t **coding_data)
{
#ifdef _USB_DEBUG
  USB_PRINTF("USB_Get_Line_Coding (%08X, %d)\r\n", handle, interface);
#endif

  /* if interface valid */
  if (interface < USB_MAX_SUPPORTED_INTERFACES)
  {
    /* get line coding data*/
    *coding_data = g_line_coding;
    return USB_OK;
  }

  return USBERR_INVALID_REQ_TYPE;
}

/**************************************************************************//*!
 *
 * @name  USB_Set_Line_Coding
 *
 * @brief The function sets the Line Coding/Configuration
 *
 * @param handle: handle
 * @param interface:     interface number
 * @param coding_data:   output line coding data
 *
 * @return USB_OK                              When Success
 *         USBERR_INVALID_REQ_TYPE             when Error
 *****************************************************************************/
uint8_t USB_Set_Line_Coding(uint32_t handle, uint8_t interface, uint8_t **coding_data)
{
  uint8_t count;

#ifdef _USB_DEBUG
  USB_PRINTF("USB_Set_Line_Coding (%08X, %d)\r\n", handle, interface);
#endif

  /* if interface valid */
  if (interface < USB_MAX_SUPPORTED_INTERFACES)
  {
    /* set line coding data*/
    for (count = 0; count < LINE_CODING_SIZE; count++)
    {
      g_line_coding[count] = *((*coding_data + USB_SETUP_PKT_SIZE) + count);
    }
    return USB_OK;
  }

  return USBERR_INVALID_REQ_TYPE;
}

/**************************************************************************//*!
 *
 * @name  USB_Get_Abstract_State
 *
 * @brief The function gets the current setting for communication feature
 *                                                  (ABSTRACT_STATE)
 * @param handle:        handle
 * @param interface:     interface number
 * @param feature_data:   output comm feature data
 *
 * @return USB_OK                              When Success
 *         USBERR_INVALID_REQ_TYPE             when Error
 *****************************************************************************/
uint8_t USB_Get_Abstract_State(uint32_t handle, uint8_t interface, uint8_t **feature_data)
{
#ifdef _USB_DEBUG
  USB_PRINTF("USB_Get_Abstract_State (%08X, %d)\r\n", handle, interface);
#endif
  /* if interface valid */
  if (interface < USB_MAX_SUPPORTED_INTERFACES)
  {
    /* get line coding data*/
    *feature_data = g_abstract_state;
    return USB_OK;
  }

  return USBERR_INVALID_REQ_TYPE;
}

/**************************************************************************//*!
 *
 * @name  USB_Get_Country_Setting
 *
 * @brief The function gets the current setting for communication feature
 *                                                  (COUNTRY_CODE)
 * @param handle:        handle
 * @param interface:     interface number
 * @param feature_data:   output comm feature data
 *
 * @return USB_OK                              When Success
 *         USBERR_INVALID_REQ_TYPE             when Error
 *****************************************************************************/
uint8_t USB_Get_Country_Setting(uint32_t handle, uint8_t interface, uint8_t **feature_data)
{
#ifdef _USB_DEBUG
  USB_PRINTF("USB_Get_Country_Setting (%08X, %d)\r\n", handle, interface);
#endif
  /* if interface valid */
  if (interface < USB_MAX_SUPPORTED_INTERFACES)
  {
    /* get line coding data*/
    *feature_data = g_country_code;
    return USB_OK;
  }

  return USBERR_INVALID_REQ_TYPE;
}

/**************************************************************************//*!
 *
 * @name  USB_Set_Abstract_State
 *
 * @brief The function gets the current setting for communication feature
 *                                                  (ABSTRACT_STATE)
 * @param handle:        handle
 * @param interface:     interface number
 * @param feature_data:   output comm feature data
 *
 * @return USB_OK                              When Success
 *         USBERR_INVALID_REQ_TYPE             when Error
 *****************************************************************************/
uint8_t USB_Set_Abstract_State(uint32_t handle, uint8_t interface, uint8_t **feature_data)
{
  uint8_t count;

#ifdef _USB_DEBUG
  USB_PRINTF("USB_Set_Abstract_State (%08X, %d)\r\n", handle, interface);
#endif
  /* if interface valid */
  if (interface < USB_MAX_SUPPORTED_INTERFACES)
  {
    /* set Abstract State Feature*/
    for (count = 0; count < COMM_FEATURE_DATA_SIZE; count++)
    {
      g_abstract_state[count] = *(*feature_data + count);
    }
    return USB_OK;
  }

  return USBERR_INVALID_REQ_TYPE;
}

/**************************************************************************//*!
 *
 * @name  USB_Set_Country_Setting
 *
 * @brief The function gets the current setting for communication feature
 *                                                  (COUNTRY_CODE)
 * @param handle: handle
 * @param interface:     interface number
 * @param feature_data:   output comm feature data
 *
 * @return USB_OK                              When Success
 *         USBERR_INVALID_REQ_TYPE             when Error
 *****************************************************************************/
uint8_t USB_Set_Country_Setting(uint32_t handle, uint8_t interface, uint8_t **feature_data)
{
  uint8_t count;


#ifdef _USB_DEBUG
  USB_PRINTF("USB_Set_Country_Setting (%08X, %d)\r\n", handle, interface);
#endif
  /* if interface valid */
  if (interface < USB_MAX_SUPPORTED_INTERFACES)
  {
    for (count = 0; count < COMM_FEATURE_DATA_SIZE; count++)
    {
      g_country_code[count] = *(*feature_data + count);
    }
    return USB_OK;
  }

  return USBERR_INVALID_REQ_TYPE;
}

/*------------------------------------------------------------------------------



 \param handle
 ------------------------------------------------------------------------------*/
void Virtual_com_change_usb_speed(uint32_t handle)
{
  if (vport_need_change_speed == TRUE)
  {
    if (USB_OK == USB_Class_CDC_Get_Speed(handle, &g_cdc_device_speed))
    {
      USB_Desc_Set_Speed(handle, g_cdc_device_speed);
      if (USB_SPEED_HIGH == g_cdc_device_speed)
      {
        g_bulk_out_max_packet_size = HS_DIC_BULK_OUT_ENDP_PACKET_SIZE;
        g_bulk_in_max_packet_size = HS_DIC_BULK_IN_ENDP_PACKET_SIZE;
#ifdef _USB_DEBUG
        USB_PRINTF("Speed = HIGH\r\n");
#endif
      }
      else
      {
        g_bulk_out_max_packet_size = FS_DIC_BULK_OUT_ENDP_PACKET_SIZE;
        g_bulk_in_max_packet_size = FS_DIC_BULK_IN_ENDP_PACKET_SIZE;
#ifdef _USB_DEBUG
        USB_PRINTF("SPEED = FULL\r\n");
#endif
      }
    }
    vport_need_change_speed = FALSE;
  }
}



/******************************************************************************
 *
 *    @name        USB_App_Device_Callback
 *
 *    @brief       This function handles the callback
 *
 *    @param       handle : handle to Identify the controller
 *    @param       event_type : value of the event
 *    @param       val : gives the configuration value
 *
 *    @return      None
 *
 *    Функция вызывается в контексте задачи с именем USB_DEV (функцимя _usb_dev_task_stun -> _usb_dev_task)
 *****************************************************************************/
void USB_App_Device_Callback(uint8_t event_type, void *val, void *arg)
{
  uint32_t handle;

#ifdef _USB_DEBUG
  USB_PRINTF("USB_App_Device_Callback: ", event_type);
  switch (event_type)
  {
  case USB_DEV_EVENT_BUS_RESET:
    USB_PRINTF("USB_DEV_EVENT_BUS_RESET             "); break;
  case USB_DEV_EVENT_CONFIG_CHANGED:
    USB_PRINTF("USB_DEV_EVENT_CONFIG_CHANGED        "); break;
  case USB_DEV_EVENT_INTERFACE_CHANGED        :
    USB_PRINTF("USB_DEV_EVENT_INTERFACE_CHANGED     "); break;
  case USB_DEV_EVENT_ENUM_COMPLETE            :
    USB_PRINTF("USB_DEV_EVENT_ENUM_COMPLETE         "); break;
  case USB_DEV_EVENT_SEND_COMPLETE            :
    USB_PRINTF("USB_DEV_EVENT_SEND_COMPLETE         "); break;
  case USB_DEV_EVENT_DATA_RECEIVED            :
    USB_PRINTF("USB_DEV_EVENT_DATA_RECEIVED         "); break;
  case USB_DEV_EVENT_ERROR                    :
    USB_PRINTF("USB_DEV_EVENT_ERROR                 "); break;
  case USB_DEV_EVENT_GET_DATA_BUFF            :
    USB_PRINTF("USB_DEV_EVENT_GET_DATA_BUFF         "); break;
  case USB_DEV_EVENT_EP_STALLED               :
    USB_PRINTF("USB_DEV_EVENT_EP_STALLED            "); break;
  case USB_DEV_EVENT_EP_UNSTALLED             :
    USB_PRINTF("USB_DEV_EVENT_EP_UNSTALLED          "); break;
  case USB_DEV_EVENT_GET_TRANSFER_SIZE        :
    USB_PRINTF("USB_DEV_EVENT_GET_TRANSFER_SIZE     "); break;
  case USB_DEV_EVENT_TYPE_SET_REMOTE_WAKEUP   :
    USB_PRINTF("USB_DEV_EVENT_TYPE_SET_REMOTE_WAKEUP"); break;
  case USB_DEV_EVENT_TYPE_CLR_REMOTE_WAKEUP   :
    USB_PRINTF("USB_DEV_EVENT_TYPE_CLR_REMOTE_WAKEUP"); break;
  case USB_DEV_EVENT_TYPE_SET_EP_HALT         :
    USB_PRINTF("USB_DEV_EVENT_TYPE_SET_EP_HALT      "); break;
  case USB_DEV_EVENT_TYPE_CLR_EP_HALT         :
    USB_PRINTF("USB_DEV_EVENT_TYPE_CLR_EP_HALT      "); break;
  case USB_DEV_EVENT_DETACH                   :
    USB_PRINTF("USB_DEV_EVENT_DETACH                "); break;
  }
  USB_PRINTF("\r\n");
#endif

  handle = *((uint32_t *)arg);
  if (event_type == USB_DEV_EVENT_BUS_RESET)
  {
    vport_configured = FALSE;
    vport_need_change_speed = TRUE;
    Virtual_com_change_usb_speed(handle);
    vport_need_change_speed = TRUE;
  }
  else if (event_type == USB_DEV_EVENT_CONFIG_CHANGED)
  {
    // Указаваем конечной  точке DIC_BULK_OUT_ENDPOINT начать принимать  данные
    USB_Class_CDC_Recv_Data(handle, DIC_BULK_OUT_ENDPOINT, rcbl[g_recv_head_indx].buff, g_bulk_out_max_packet_size);

    vport_configured = TRUE;
  }
  else if (event_type == USB_DEV_EVENT_ERROR)
  {
    /* add user code for error handling */
  }
  else if (event_type == USB_DEV_EVENT_ENUM_COMPLETE)
  {
    // Приходит после события USB_DEV_EVENT_CONFIG_CHANGED
  }
  return;
}

/******************************************************************************
 *
 *    @name        USB_App_Class_Callback
 *
 *    @brief       This function handles the callback for Get/Set report req
 *
 *    @param       request  :  request type
 *    @param       value    :  give report type and id
 *    @param       data     :  pointer to the data
 *    @param       size     :  size of the transfer
 *
 *    @return      status
 *                  USB_OK  :  if successful
 *                  else return error
 *
 *    Функция вызывается в контексте задачи с именем USB_DEV (функцимя _usb_dev_task_stun -> _usb_dev_task)
 *
 *****************************************************************************/
uint8_t USB_App_Class_Callback(uint8_t event, uint16_t value, uint8_t **data, uint32_t *size, void *arg)
{
  cdc_handle_t handle;
  uint8_t error = USB_OK;
  int32_t res;

#ifdef _USB_DEBUG
  USB_PRINTF("USB_App_Class_Callback: ");
  switch (event)
  {
  case SEND_ENCAPSULATED_COMMAND       :
    USB_PRINTF("SEND_ENCAPSULATED_COMMAND      "); break;
  case GET_ENCAPSULATED_RESPONSE       :
    USB_PRINTF("GET_ENCAPSULATED_RESPONSE      "); break;
  case SET_COMM_FEATURE                :
    USB_PRINTF("SET_COMM_FEATURE               "); break;
  case GET_COMM_FEATURE                :
    USB_PRINTF("GET_COMM_FEATURE               "); break;
  case CLEAR_COMM_FEATURE              :
    USB_PRINTF("CLEAR_COMM_FEATURE             "); break;
  case SET_AUX_LINE_STATE              :
    USB_PRINTF("SET_AUX_LINE_STATE             "); break;
  case SET_HOOK_STATE                  :
    USB_PRINTF("SET_HOOK_STATE                 "); break;
  case PULSE_SETUP                     :
    USB_PRINTF("PULSE_SETUP                    "); break;
  case SEND_PULSE                      :
    USB_PRINTF("SEND_PULSE                     "); break;
  case SET_PULSE_TIME                  :
    USB_PRINTF("SET_PULSE_TIME                 "); break;
  case RING_AUX_JACK                   :
    USB_PRINTF("RING_AUX_JACK                  "); break;
  case SET_LINE_CODING                 :
    USB_PRINTF("SET_LINE_CODING                "); break;
  case GET_LINE_CODING                 :
    USB_PRINTF("GET_LINE_CODING                "); break;
  case SET_CONTROL_LINE_STATE          :
    USB_PRINTF("SET_CONTROL_LINE_STATE         "); break;
  case SEND_BREAK                      :
    USB_PRINTF("SEND_BREAK                     "); break;
  case SET_RINGER_PARAMS               :
    USB_PRINTF("SET_RINGER_PARAMS              "); break;
  case GET_RINGER_PARAMS               :
    USB_PRINTF("GET_RINGER_PARAMS              "); break;
  case SET_OPERATION_PARAM             :
    USB_PRINTF("SET_OPERATION_PARAM            "); break;
  case GET_OPERATION_PARAM             :
    USB_PRINTF("GET_OPERATION_PARAM            "); break;
  case SET_LINE_PARAMS                 :
    USB_PRINTF("SET_LINE_PARAMS                "); break;
  case GET_LINE_PARAMS                 :
    USB_PRINTF("GET_LINE_PARAMS                "); break;
  case DIAL_DIGITS                     :
    USB_PRINTF("DIAL_DIGITS                    "); break;
  case SET_UNIT_PARAMETER              :
    USB_PRINTF("SET_UNIT_PARAMETER             "); break;
  case GET_UNIT_PARAMETER              :
    USB_PRINTF("GET_UNIT_PARAMETER             "); break;
  case CLEAR_UNIT_PARAMETER            :
    USB_PRINTF("CLEAR_UNIT_PARAMETER           "); break;
  case GET_PROFILE                     :
    USB_PRINTF("GET_PROFILE                    "); break;
  case SET_ETHERNET_MULTICAST_FILTERS  :
    USB_PRINTF("SET_ETHERNET_MULTICAST_FILTERS "); break;
  case SET_ETHERNET_POW_PATTER_FILTER  :
    USB_PRINTF("SET_ETHERNET_POW_PATTER_FILTER "); break;
  case GET_ETHERNET_POW_PATTER_FILTER  :
    USB_PRINTF("GET_ETHERNET_POW_PATTER_FILTER "); break;
  case SET_ETHERNET_PACKET_FILTER      :
    USB_PRINTF("SET_ETHERNET_PACKET_FILTER     "); break;
  case GET_ETHERNET_STATISTIC          :
    USB_PRINTF("GET_ETHERNET_STATISTIC         "); break;
  case SET_ATM_DATA_FORMAT             :
    USB_PRINTF("SET_ATM_DATA_FORMAT            "); break;
  case GET_ATM_DEVICE_STATISTICS       :
    USB_PRINTF("GET_ATM_DEVICE_STATISTICS      "); break;
  case SET_ATM_DEFAULT_VC              :
    USB_PRINTF("SET_ATM_DEFAULT_VC             "); break;
  case GET_ATM_VC_STATISTICS           :
    USB_PRINTF("GET_ATM_VC_STATISTICS          "); break;
  case MDLM_SPECIFIC_REQUESTS_MASK     :
    USB_PRINTF("MDLM_SPECIFIC_REQUESTS_MASK    "); break;
  case GET_COUNTRY_SETTING             :
    USB_PRINTF("GET_COUNTRY_SETTING            "); break;
  case SET_ABSTRACT_STATE              :
    USB_PRINTF("SET_ABSTRACT_STATE             "); break;
  case SET_COUNTRY_SETTING             :
    USB_PRINTF("SET_COUNTRY_SETTING            "); break;
  case USB_APP_CDC_CARRIER_DEACTIVATED :
    USB_PRINTF("USB_APP_CDC_CARRIER_DEACTIVATED"); break;
  case USB_APP_CDC_CARRIER_ACTIVATED   :
    USB_PRINTF("USB_APP_CDC_CARRIER_ACTIVATED  "); break;
  case USB_APP_CDC_DTE_DEACTIVATED     :
    USB_PRINTF("USB_APP_CDC_DTE_DEACTIVATED    "); break;
  case USB_APP_CDC_DTE_ACTIVATED       :
    USB_PRINTF("USB_APP_CDC_DTE_ACTIVATED      "); break;
  case USB_APP_GET_LINK_SPEED          :
    USB_PRINTF("USB_APP_GET_LINK_SPEED         "); break;
  case USB_APP_GET_LINK_STATUS         :
    USB_PRINTF("USB_APP_GET_LINK_STATUS        "); break;
  case USB_APP_CDC_SERIAL_STATE_NOTIF  :
    USB_PRINTF("USB_APP_CDC_SERIAL_STATE_NOTIF "); break;
  }
  USB_PRINTF("\r\n");
#endif

  handle = *((cdc_handle_t *)arg);
  switch (event)
  {
  case GET_LINE_CODING:
    error = USB_Get_Line_Coding(handle, value, data);
    break;
  case GET_ABSTRACT_STATE:
    error = USB_Get_Abstract_State(handle, value, data);
    break;
  case GET_COUNTRY_SETTING:
    error = USB_Get_Country_Setting(handle, value, data);
    break;
  case SET_LINE_CODING:
    error = USB_Set_Line_Coding(handle, value, data);
    break;
  case SET_ABSTRACT_STATE:
    error = USB_Set_Abstract_State(handle, value, data);
    break;
  case SET_COUNTRY_SETTING:
    error = USB_Set_Country_Setting(handle, value, data);
    break;
  case USB_APP_CDC_DTE_ACTIVATED:
    if (vport_configured == TRUE)
    {
      // Происходит когда открывают порт на PC
      vport_opened = TRUE;
      _lwevent_set(&vcm_lwev, VCOM_FLAG_TRANSMITED);  // Объявляем готовность к передаче
    }
    break;
  case USB_APP_CDC_DTE_DEACTIVATED:
    if (vport_configured == TRUE)
    {
      // Происходит когда закрывают порт на PC
      vport_opened = FALSE;
      _lwevent_clear(&vcm_lwev, VCOM_FLAG_TRANSMITED);
    }
    break;
  case USB_DEV_EVENT_DATA_RECEIVED:
    {
      if ((vport_configured == TRUE) && (vport_opened == TRUE))
      {
        // Если *size = 0xFFFFFFFF то данные не приняты
        if ((*size != 0xFFFFFFFF) && (*size != 0))
        {
          uint32_t tmp_indx;
          // Пакет принят

          rcbl[g_recv_head_indx].len = *size;
          rcbl[g_recv_head_indx].pos = 0;


          //  Перевести указатель на следующий свободный для приема буфер
          tmp_indx =  g_recv_head_indx;
          tmp_indx++;
          if (tmp_indx >= IN_BUF_QUANTITY) tmp_indx = 0;



          if (tmp_indx != g_recv_tail_indx)
          {
            // Если есть еще свободный буффер в очереди то назначить прием в него
            g_recv_head_indx = tmp_indx;
            res = USB_Class_CDC_Recv_Data(handle, DIC_BULK_OUT_ENDPOINT, rcbl[g_recv_head_indx].buff, g_bulk_out_max_packet_size);
            if (res != USB_OK) g_recv_error_cnt++;
          }
          else
          {
            // Буферов свободных для пиема не обнаружено.
            // Сигнализируем флагом о том что не смогли зарегистрировать режим прием для конечной точки
            g_recv_buf_queue_full = 1;
          }

          //ITM_EVENT8_WITH_PC(2,1);
          _lwevent_set(&vcm_lwev, VCOM_FLAG_RECEIVED); // Установливаем флаг выполненного приема

#ifdef DEBUG_USB_APP
          DEBUG_PRINT_USP_APP("Recv %d\r\n", *size);
#endif

        }
        else
        {
          // Заново взводим прием на текущй буфер поскольку была ошибка приема
          res = USB_Class_CDC_Recv_Data(handle, DIC_BULK_OUT_ENDPOINT, rcbl[g_recv_head_indx].buff, g_bulk_out_max_packet_size);
          if (res != USB_OK) g_recv_error_cnt++;
        }
      }
    }
    break;
  case USB_DEV_EVENT_SEND_COMPLETE:
    {
      //ITM_EVENT8_WITH_PC(1,1);
      _lwevent_set(&vcm_lwev, VCOM_FLAG_TRANSMITED);

      if ((size != NULL) && (*size != 0) && (!(*size % g_bulk_in_max_packet_size)))
      {
        /* If the last packet is the size of endpoint, then send also zero-ended packet,
         ** meaning that we want to inform the host that we do not have any additional
         ** data, so it can flush the output.
         */
        USB_Class_CDC_Send_Data(g_app_handle, DIC_BULK_IN_ENDPOINT, NULL, 0);
      }
      else if ((vport_configured == TRUE) && (vport_opened == TRUE))
      {
        if ((*data != NULL) || ((*data == NULL) && (*size == 0)))
        {
          /* User: add your own code for send complete event */
          /* Schedule buffer for next receive event */

        }
      }
    }
    break;
  case USB_APP_CDC_SERIAL_STATE_NOTIF:
    {
      /* User: add your own code for serial_state notify event */

      // Происходит когда открывают порт на PC
    }
    break;
  case USB_APP_CDC_CARRIER_ACTIVATED:

    // Происходит когда открывают порт на PC или закрывают

    break;
  default:
    {
      error = USBERR_INVALID_REQ_TYPE;
      break;
    }

  }

  return error;
}

/*------------------------------------------------------------------------------



 \param initial_data
 ------------------------------------------------------------------------------*/
void Init_USB(void)
{
  uint32_t i;
  cdc_config_struct_t cdc_config;
  cdc_config.cdc_application_callback.callback = USB_App_Device_Callback;
  cdc_config.cdc_application_callback.arg = &g_app_handle;
  cdc_config.vendor_req_callback.callback = NULL;
  cdc_config.vendor_req_callback.arg = NULL;
  cdc_config.class_specific_callback.callback = USB_App_Class_Callback;
  cdc_config.class_specific_callback.arg = &g_app_handle;
  cdc_config.desc_callback_ptr = &desc_callback;
  /* Always happen in control endpoint hence hard coded in Class layer*/

  g_recv_buf = OS_Mem_alloc_uncached_align(DATA_BUFF_SIZE * IN_BUF_QUANTITY, 32);
  g_recv_head_indx = 0;
  g_recv_tail_indx = g_recv_head_indx;
  g_recv_buf_queue_full = 0;
  for (i = 0; i < IN_BUF_QUANTITY; i++)
  {
    // Подготавдиваем массив управляющих структур для приема пакетов
    rcbl[i].buff = &g_recv_buf[DATA_BUFF_SIZE * i];
    rcbl[i].len = 0;
    rcbl[i].pos = 0;
  }

  g_curr_send_buf = OS_Mem_alloc_uncached_align(DATA_BUFF_SIZE, 32);

  g_cdc_device_speed         = USB_SPEED_HIGH; //USB_SPEED_FULL;
  g_bulk_out_max_packet_size = FS_DIC_BULK_OUT_ENDP_PACKET_SIZE;
  g_bulk_in_max_packet_size  = FS_DIC_BULK_IN_ENDP_PACKET_SIZE;

  _lwevent_create(&vcm_lwev, LWEVENT_AUTO_CLEAR); // Все события автоматически сбрасываемые
  /* Initialize the USB interface */
  USB_Class_CDC_Init(CONTROLLER_ID, &cdc_config, &g_app_handle);
}

/*------------------------------------------------------------------------------



 \param buff
 \param size
 \param ticks

 \return uint32_t MQX_OK
 ------------------------------------------------------------------------------*/
uint32_t Virtual_com_send_data(uint8_t *buff, uint32_t size, uint32_t ticks)
{
  uint32_t res;
#ifdef DEBUG_USB_APP
  DEBUG_PRINT_USP_APP("Send %d\r\n", size);
#endif

  if (ticks != 0)
  {
    if (_lwevent_wait_ticks(&vcm_lwev, VCOM_FLAG_TRANSMITED, FALSE, ticks)!=MQX_OK) return MQX_ERROR;
  }

  if (USB_Class_CDC_Send_Data(g_app_handle, DIC_BULK_IN_ENDPOINT, buff, size) == USB_OK) return MQX_OK;
  return MQX_ERROR;
}

/*------------------------------------------------------------------------------
  Получить данные из виртуального порта в буфер buff, количество байт данных - size
  В случае невозможности выполнения происходит возврат из функиции без задержки

 \param buff
 \param size

 \return uint32_t. Возвращает MQX_OK в случае выполнения функции и MQX_ERROR в случае невыполнения функции

 ------------------------------------------------------------------------------*/
uint32_t Virtual_com_get_data(uint8_t *b, uint32_t ticks)
{
  int32_t  res;
  // Если индексы буферов равны то это значит отсутствие принятых пакетов
  if (g_recv_tail_indx == g_recv_head_indx)
  {
    if (ticks != 0)
    {
      if (_lwevent_wait_ticks(&vcm_lwev, VCOM_FLAG_RECEIVED, FALSE, ticks)!=MQX_OK) return MQX_ERROR;
    }
    else return MQX_ERROR;
  }


  *b = rcbl[g_recv_tail_indx].buff[rcbl[g_recv_tail_indx].pos];
  rcbl[g_recv_tail_indx].pos++;

  // Если позиция достигла конца данных в текущем буфере, то буфер освобождается для приема
  if (rcbl[g_recv_tail_indx].pos >= rcbl[g_recv_tail_indx].len)
  {
    // Смещаем указатель хвоста очереди приемных буфферов
    // Появляется место для движения головы очереди приемных буфферов

    if ((g_recv_tail_indx + 1) >= IN_BUF_QUANTITY) g_recv_tail_indx = 0;
    else g_recv_tail_indx++;

    if (g_recv_buf_queue_full == 1) // Если очередь была заполнена, то посылаем запрос приема , так как в обработчике собыйтий запрос приема был пропущен
    {
      g_recv_buf_queue_full = 0;

      if ((g_recv_head_indx + 1) >= IN_BUF_QUANTITY) g_recv_head_indx = 0;
      else g_recv_head_indx++;

      res = USB_Class_CDC_Recv_Data(g_app_handle, DIC_BULK_OUT_ENDPOINT, rcbl[g_recv_head_indx].buff, g_bulk_out_max_packet_size);
      if (res != USB_OK) g_recv_error_cnt++;
    }
  }

  return MQX_OK;

}
