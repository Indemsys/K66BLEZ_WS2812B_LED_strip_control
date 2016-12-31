// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.09.25
// 20:49:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "K66BLEZ.h"

// Коды ответов
#define  REPLY_DEVICE_READY_FOR_CMD   0x0000AA00  // Устройство готово к обмену
#define  REPLY_FILE_PREPARED          0x0000AA01  // Файл подготовлен
#define  REPLY_FILE_ERROR             0x0000AA03  // Ошибка файла

// Коды команд
#define  CMD_PLAY_FILE                0x00000002
#define  CMD_STOP_PLAY                0x00000003

static deviceId_t        g_peer_device_id;
/*------------------------------------------------------------------------------
  Перехватили запись в характеристику сервиса команд


 \param peer_device_id
 \param handle
 \param value
 \param len

 \return uint8_t
 ------------------------------------------------------------------------------*/
uint8_t  CommandService_write(deviceId_t peer_device_id, uint16_t handle, uint8_t *value, uint16_t len)
{
  uint32_t cmd;

  DEBUG_CMDSERV_PRINT_ARG("Intercepted write cmd len=%d\r\n", len);

  // Запоминаем устройство от которого перехватили запрос на запись
  g_peer_device_id = peer_device_id;
  // Проверяем получили ли мы команду или имя файла
  if (len == 4)
  {
    memcpy(&cmd, value, 4);
  }
  else cmd = 0;

  if (cmd == CMD_PLAY_FILE)
  {
    // Команда начать воспроизведение файла
    DEBUG_CMDSERV_PRINT_ARG("Command to play file\r\n", 0);
    // Отправляем команду хосту
    Host_channel_push(MKW40_SUBS_CMDMAN, (uint8_t *)&cmd, sizeof(cmd));
  }
  else if (cmd == CMD_STOP_PLAY)
  {
    // Команда остановки воспроизведение файла
    DEBUG_CMDSERV_PRINT_ARG("Command to stop playing\r\n", 0);
    // Отправляем команду хосту
    Host_channel_push(MKW40_SUBS_CMDMAN, (uint8_t *)&cmd, sizeof(cmd));
  }
  else
  {
    // Считаем что это имя файла
    DEBUG_CMDSERV_PRINT_ARG("File name received\r\n", 0);
    DEBUG_CMDSERV_PRINT_ARG("%s\r\n", value);

    // Отправляем команду хосту
    Host_channel_push(MKW40_SUBS_CMDMAN, value, len);


    // Здесь имитируем ответ.
    // Клиент к этому моменту уже должен быть подписан на характеристику поэтом готовность к оповещению не проверяем
    //{
    //  uint32_t val = REPLY_FILE_PREPARED;
    //  GattDb_WriteAttribute(value_k66_cmd_read, sizeof(val), (uint8_t *)&val);
    //  GattServer_SendNotification(g_peer_device_id, value_k66_cmd_read);
    //}

  }


  return gAttErrCodeNoError_c;

}

/*------------------------------------------------------------------------------
   Прием данных их канала хоста
   Выполняется в контексте задачи Host_Chan_Task


 \param data
 \param sz
 ------------------------------------------------------------------------------*/
void CommandService_receive_from_host(uint8_t *data, uint32_t sz)
{
  // Записываем ответ от хост контроллера в характеристику
  GattDb_WriteAttribute(value_k66_cmd_read, sz, data);
  // Сразу же высылаем нотификацию
  GattServer_SendNotification(g_peer_device_id, value_k66_cmd_read);

}
/*------------------------------------------------------------------------------
  Перехватили чтение характеристики севиса команд
  Явно читается в этом варианте сервиса только готовность сервиса к обмену


 \param void
 ------------------------------------------------------------------------------*/
void CommandService_read(void)
{
  uint32_t val = REPLY_DEVICE_READY_FOR_CMD;

  DEBUG_CMDSERV_PRINT_ARG("Intercepted red cmd\r\n", 0);

  GattDb_WriteAttribute(value_k66_cmd_read, sizeof(val), (uint8_t *)&val); // Записываем данные в характеристику

}
