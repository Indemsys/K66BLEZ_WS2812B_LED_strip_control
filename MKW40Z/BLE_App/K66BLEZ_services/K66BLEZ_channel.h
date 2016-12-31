#ifndef BLE_HOST_CHANNEL_H
  #define BLE_HOST_CHANNEL_H

// Идентификаторы подписчиков приемников пакетов из канала связи MKW40
// Эти же идентификаторы являются индексами в массиве подписки
#define MKW40_SUBS_VT100    1  // Идентификатор пакета для драйвера терминала VT100
#define MKW40_SUBS_FILEMAN  2  // Идентификатор пакета для файлового менеджера
#define MKW40_SUBS_CMDMAN   3  // Идентификатор пакета для менеджера команд

#define MKW_SUBSCR_NAX_CNT  3 // Колическтво элементов в массиве подписок на прием пакетов из канала свзя с MKW40


void    Init_host_channel_task(void);
int32_t Host_channel_push(uint8_t chanid, uint8_t *data, uint32_t sz);

#endif // BLE_HOST_CHANNEL_H



