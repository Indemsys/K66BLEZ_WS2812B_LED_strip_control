// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.09.21
// 17:42:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "K66BLEZ.h"

#define TEST_DATA_BUF_SZ 20
static uint64_t          g_rcv_time;
static uint64_t          g_start_rcv_time;
static T_perf_test_data  g_ptd;
static uint32_t          g_transf_data_cnt = 0;
static uint32_t          g_transf_data_sz;
static uint32_t          g_notification; // Флаг отправки данных теста чтения способом нотификации

static uint8_t           g_test_data_buf[TEST_DATA_BUF_SZ];
static uint8_t           g_read_test_active;
static deviceId_t        g_peer_device_id;
static uint16_t          g_handle;
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void Restart_perfomance_test()
{
  DEBUG_PERF_TEST_PRINT_ARG("Perfomance test restart\r\n", 0);

  g_ptd.test_data_crc =  0;
  g_transf_data_cnt = 0;
  //Init_CRC32();
}

/*------------------------------------------------------------------------------



 \param handle
 \param value
 \param len

 \return uint8_t
 ------------------------------------------------------------------------------*/
uint8_t PerfomanceTest_write_test(uint16_t handle, uint8_t *value, uint16_t len)
{
  g_rcv_time = BSP_Read_PIT();
  g_ptd.test_data_crc = Finish_CRC32(g_ptd.test_data_crc);
  g_ptd.test_data_crc = Update_CRC32(g_ptd.test_data_crc, value, len);
  g_ptd.test_data_crc = Finish_CRC32(g_ptd.test_data_crc);

  if (g_transf_data_cnt == 0)
  {
    g_start_rcv_time = g_rcv_time;
  }
  g_ptd.delta_t = (uint32_t)(g_rcv_time - g_start_rcv_time);
  GattDb_WriteAttribute(handle, sizeof(g_ptd), (uint8_t *)&g_ptd); // Записываем контрольную сумму обратно в прочитанный аттрибут чтобы ее мог прочитатть сервер

  g_transf_data_cnt += len;
  DEBUG_PERF_TEST_PRINT_ARG("Received:%06d Time stump=%08X%08X\r\n", g_transf_data_cnt, (uint32_t)(g_rcv_time >> 32), (uint32_t)(g_rcv_time & 0xFFFFFFFF));


  //Debug_dump_8bit_mem(len, value);
  return gAttErrCodeNoError_c;

}

/*------------------------------------------------------------------------------
 Инициализация теста чтения
 Вызывается после записи в тестовую характеристику


 \param handle
 \param value
 \param len

 \return uint8_t
 ------------------------------------------------------------------------------*/
uint8_t  PerfomanceTest_read_test_init(deviceId_t peer_device_id, uint16_t handle, uint8_t *value, uint16_t len)
{
  Restart_perfomance_test();
  if (len >= sizeof(g_transf_data_sz))
  {

    memcpy(&g_transf_data_sz, value, sizeof(g_transf_data_sz)); // Сохраняем количество читаемых байт данных
  }
  else return gAttErrCodeRequestNotSupported_c;

  // Дополнительные проверки и ограничения в случае если клиент пытается неправильно сконфигурировать тест
  if (len >= (sizeof(g_transf_data_sz) +  sizeof(g_transf_data_sz)))
  {
    memcpy(&g_notification, value + sizeof(g_transf_data_sz), sizeof(g_notification)); // Сохраняем флаг нотификации
    if (g_notification > 1) g_notification = 0;
  }
  else g_notification = 0;

  DEBUG_PERF_TEST_PRINT_ARG("Read perfomance test params: size=%d, notif=%d \r\n", g_transf_data_sz, g_notification);

  g_peer_device_id = peer_device_id;
  g_handle = handle;
  g_read_test_active = 1;
  return gAttErrCodeNoError_c;
}

/*------------------------------------------------------------------------------
 Подготовка данных теста производительности чтения
 Вызывается при чтении тестовой характеристики удаленным клиентом

 \param void
 ------------------------------------------------------------------------------*/
void PerfomanceTest_read_test(void)
{
  uint32_t len;
  uint32_t i;
  // Фиксируем момент времени чтения данных
  g_rcv_time = BSP_Read_PIT();
  if (g_transf_data_cnt == 0)
  {
    g_start_rcv_time = g_rcv_time;
  }
  g_ptd.delta_t = (uint32_t)(g_rcv_time - g_start_rcv_time);

  // Генерируем случайные числа и записываем их в характеристику теста
  len = g_transf_data_sz - g_transf_data_cnt;
  if (len != 0)
  {
    if (len > TEST_DATA_BUF_SZ) len = TEST_DATA_BUF_SZ;

    for (i = 0; i < len; i++)
    {
      g_test_data_buf[i] = rand();
    }
    g_transf_data_cnt += len;

    // Подсчитываем контрольную сумму
    g_ptd.test_data_crc = Finish_CRC32(g_ptd.test_data_crc);
    g_ptd.test_data_crc = Update_CRC32(g_ptd.test_data_crc, g_test_data_buf, len);
    g_ptd.test_data_crc = Finish_CRC32(g_ptd.test_data_crc);

    DEBUG_PERF_TEST_PRINT_ARG("Read test block. CNT=%06d Time stump=%08X%08X\r\n", g_transf_data_cnt, (uint32_t)(g_rcv_time >> 32), (uint32_t)(g_rcv_time & 0xFFFFFFFF));
    GattDb_WriteAttribute(g_handle, len, g_test_data_buf); // Записываем данные в характеристику, чтобы их мог прочитать клиент
  }
  else
  {
    // len = 0, Значит переданы все данные. В характеристику сохраняем crc и время
    DEBUG_PERF_TEST_PRINT_ARG("Read test   END. CRC=%08X Delta T.=%d\r\n", g_ptd.test_data_crc, g_ptd.delta_t);
    GattDb_WriteAttribute(g_handle, sizeof(g_ptd), (uint8_t *)&g_ptd); // Записываем контрольную сумму обратно в прочитанный аттрибут чтобы ее мог прочитатть сервер
    g_read_test_active = 0;
    Restart_perfomance_test(); // Подготавливаемся сразу же к другому тесту
  }
}


/*------------------------------------------------------------------------------
  Периодически вызывается для отправки нотификаций о новом значении в  тестовой характеристике


 \param void
 ------------------------------------------------------------------------------*/
void PerfomanceTest_read_notification(void)
{
  if ((g_notification != 0) && (g_read_test_active != 0))
  {
    bool_t is_notif_active;
    if (Gap_CheckNotificationStatus(g_peer_device_id, cccd_k66rddata, &is_notif_active) == gBleSuccess_c)
    {
      if (is_notif_active == TRUE)
      {
        PerfomanceTest_read_test();
        DEBUG_PERF_TEST_PRINT_ARG("Send notification.\r\n", 0);
        GattServer_SendNotification(g_peer_device_id, g_handle);
      }
    }
  }
}
/*------------------------------------------------------------------------------



 \param cValueLength
 \param aValue
 ------------------------------------------------------------------------------*/
void Debug_dump_8bit_mem(uint16_t len, uint8_t *mem)
{
  uint32_t i;

  DEBUG_PERF_TEST_PRINT_ARG("\r\n", 0);
  while (len > 0)
  {
    for (i = 0; i < 16; i++)
    {
      DEBUG_PERF_TEST_PRINT_ARG(" %02X", *mem);
      mem++;
      len--;
      if (len == 0) break;
    }
    DEBUG_PERF_TEST_PRINT_ARG("\r\n", 0);
  }
}

/*------------------------------------------------------------------------------



 \param cValueLength
 \param aValue
 ------------------------------------------------------------------------------*/
void Debug_dump_32bit_mem(uint16_t len, uint32_t *mem)
{
  uint32_t i;

  DEBUG_PERF_TEST_PRINT_ARG("\r\n", 0);
  while (len > 0)
  {
    for (i = 0; i < 8; i++)
    {
      DEBUG_PERF_TEST_PRINT_ARG(" %08X", *mem);
      mem++;
      len--;
      if (len == 0) break;
    }
    DEBUG_PERF_TEST_PRINT_ARG("\r\n", 0);
  }
}
