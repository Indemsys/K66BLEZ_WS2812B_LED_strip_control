#ifndef MKW40_CHANNEL_H
  #define MKW40_CHANNEL_H

// Идентификаторы подписчиков приемников пакетов из канала связи MKW40
// Эти же идентификаторы являются индексами в массиве подписки
  #define MKW40_SUBS_VT100    1  // Идентификатор пакета для драйвера терминала VT100
  #define MKW40_SUBS_FILEMAN  2  // Идентификатор пакета для файлового менеджера
  #define MKW40_SUBS_CMDMAN   3  // Идентификатор пакета для менеджера команд

  #define MKW_SUBSCR_NAX_CNT  3 // Колическтво элементов в массиве подписок на прием пакетов из канала свзя с MKW40

// Коды протокола управления плейером
//
// Коды ответов
  #define  REPLY_DEVICE_READY_FOR_CMD   0x0000AA00  // Устройство готово к обмену
  #define  REPLY_FILE_PREPARED          0x0000AA01  // Файл подготовлен
  #define  REPLY_FILE_ERROR             0x0000AA03  // Ошибка файла
  #define  REPLY_PLAYING_END            0x0000AA04  // Воспроизведение окончено
  #define  REPLY_CMD_ERROR              0x01010101  // Ошибка команды

// Коды команд
  #define  CMD_START                0x00000002
  #define  CMD_STOP                0x00000003


typedef void (*T_MKW40_receiver)(uint8_t *data, uint32_t sz, void *ptr);

typedef struct
{
  T_MKW40_receiver   receiv_func;
  void               *pcbl; // Указатель на вспомогательную управляющую структуру для функции приемника
} T_MKW40_recv_subsc;  // Запись подуписки на прием из канала MKW40


void        Task_MKW40(uint32_t parameter);
_mqx_uint   MKW40_subscibe(uint8_t llid, T_MKW40_receiver receiv_func, void *pcbl);
_mqx_uint   MKW40_send_buf(uint8_t llid, uint8_t *data, uint32_t sz);

#endif // MKW40_CHANNEL_H



