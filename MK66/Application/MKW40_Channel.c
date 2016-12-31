// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.29
// 15:08:39
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define MKW_BUF_SZ   22 // Вместимость всего пакета
#define MKW_DATA_SZ  20 // Количество полезных данных в пакете
static uint8_t       mkw_rx_buf[MKW_BUF_SZ]; // Буфер приема данных от BLE чипа
static uint8_t       mkw_tx_buf[MKW_BUF_SZ]; // Буфер передачи данных BLE чипу
static MUTEX_STRUCT  mutex;

static T_MKW40_recv_subsc MKW40_subsribers[MKW_SUBSCR_NAX_CNT];  // Массив с подписками на прием из каала связи MKW40

/*------------------------------------------------------------------------------
  Задача поддержки канала связи с чипом беспроводной связи MKW40 поддерживающим протокол Bluetooth LE (BLE)

 \param parameter
 ------------------------------------------------------------------------------*/
void Task_MKW40(uint32_t parameter)
{
  if (Create_mutex_P_inhr_P_queue(&mutex) != MQX_OK) return;

  _mutex_lock(&mutex);

  Init_MKW40_channel();
  memset(mkw_tx_buf, 0, MKW_BUF_SZ); // Очищаеи буфер отправляемых данных

  for (;;)
  {
    // Принимаем и отправляем данные чипу MKW40 в режиме слэйва
    // Мастер MKW40 посылает пакеты размером MKW_BUF_SZ каждые 2 мс
    // Структура пакета:
    // Номер байта   Нахначение
    // в пакете
    // 0     [llid]  - идентификатор логического канала
    // 1     [LEN]   - колическтво передаваемых данных
    // 2     [data0] - первый байт данных
    // ...
    // LEN+2 [dataN] - последний байт данных



    if (MKW40_SPI_slave_read_write_buf(mkw_tx_buf, mkw_rx_buf, MKW_BUF_SZ) == MQX_OK)
    {
      uint8_t  llid = mkw_rx_buf[0] - 1;
      // Определяем назначение пакета с данными и маршрутизируем его


      if (llid < MKW_SUBSCR_NAX_CNT)
      {
        if (MKW40_subsribers[llid].receiv_func!= 0)
        {
          MKW40_subsribers[llid].receiv_func(&mkw_rx_buf[2], mkw_rx_buf[1], MKW40_subsribers[llid].pcbl);
        }
      }

      // Загрузка новых данных для отправки
      memset(mkw_tx_buf, 0, MKW_BUF_SZ); // Предварительно обнуляем буфер чтобы не были высланы старые данные
      _mutex_unlock(&mutex);
      // Здесь задачи ожидающие мьютекса получают доступ к заполнению буфера mkw_tx_buf
      _mutex_lock(&mutex);
    }

  }
}

/*------------------------------------------------------------------------------
 Установка процедуры приема в массиве подписчиков

 \param llid          - идентификатор логического канала передачи, он же индекс в массиве подписчиков
 \param receiv_func   - указатель на функцию приема
 \param pcbl          - указатель на вспомогательную структуру передаваемую при вызове функции приема

 \return _mqx_uint
 ------------------------------------------------------------------------------*/
_mqx_uint MKW40_subscibe(uint8_t llid, T_MKW40_receiver receiv_func, void* pcbl)
{
  llid -= 1;
  if (llid < MKW_SUBSCR_NAX_CNT)
  {
    MKW40_subsribers[llid].receiv_func = receiv_func;
    MKW40_subsribers[llid].pcbl = pcbl;
    return MQX_OK;
  }
  return MQX_ERROR;

}
/*------------------------------------------------------------------------------
 Процедура отправки данных в канала связи MKW40


 \param llid - идентификатор логического канала передачи
 \param data
 \param sz
 ------------------------------------------------------------------------------*/
_mqx_uint MKW40_send_buf(uint8_t llid, uint8_t *data, uint32_t sz)
{
  _mqx_uint res;
  uint32_t  len = 0;

  while (sz > 0)
  {
    res = _mutex_lock(&mutex);
    if (res != MQX_OK) return res;

    mkw_tx_buf[0] = llid;
    if (sz > MKW_DATA_SZ) len = MKW_DATA_SZ;
    else len = sz;
    mkw_tx_buf[1] = (uint8_t)len;
    memcpy(&mkw_tx_buf[2], data, len);
    sz -= len;
    data += len;
    _mutex_unlock(&mutex);
  }

  return MQX_OK;
}
