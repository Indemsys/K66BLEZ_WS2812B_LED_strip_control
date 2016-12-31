#ifndef PERFOMANCETEST_SERVICE_H
  #define PERFOMANCETEST_SERVICE_H

typedef struct
{
  uint32_t test_data_crc;
  uint32_t delta_t;
}
T_perf_test_data;

void     Restart_perfomance_test();
uint8_t  PerfomanceTest_write_test(uint16_t handle, uint8_t *value, uint16_t len);
uint8_t  PerfomanceTest_read_test_init(deviceId_t peer_device_id, uint16_t handle, uint8_t *value, uint16_t len);
void     PerfomanceTest_read_test(void);
void     PerfomanceTest_read_notification(void);
void     Debug_dump_8bit_mem(uint16_t len, uint8_t *mem);
void     Debug_dump_32bit_mem(uint16_t len, uint32_t *mem);
#endif // PERFOMANCETEST_SERVICE_H



