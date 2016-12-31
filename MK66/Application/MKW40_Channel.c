// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.29
// 15:08:39
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define MKW_BUF_SZ   22 // ����������� ����� ������
#define MKW_DATA_SZ  20 // ���������� �������� ������ � ������
static uint8_t       mkw_rx_buf[MKW_BUF_SZ]; // ����� ������ ������ �� BLE ����
static uint8_t       mkw_tx_buf[MKW_BUF_SZ]; // ����� �������� ������ BLE ����
static MUTEX_STRUCT  mutex;

static T_MKW40_recv_subsc MKW40_subsribers[MKW_SUBSCR_NAX_CNT];  // ������ � ���������� �� ����� �� ����� ����� MKW40

/*------------------------------------------------------------------------------
  ������ ��������� ������ ����� � ����� ������������ ����� MKW40 �������������� �������� Bluetooth LE (BLE)

 \param parameter
 ------------------------------------------------------------------------------*/
void Task_MKW40(uint32_t parameter)
{
  if (Create_mutex_P_inhr_P_queue(&mutex) != MQX_OK) return;

  _mutex_lock(&mutex);

  Init_MKW40_channel();
  memset(mkw_tx_buf, 0, MKW_BUF_SZ); // ������� ����� ������������ ������

  for (;;)
  {
    // ��������� � ���������� ������ ���� MKW40 � ������ ������
    // ������ MKW40 �������� ������ �������� MKW_BUF_SZ ������ 2 ��
    // ��������� ������:
    // ����� �����   ����������
    // � ������
    // 0     [llid]  - ������������� ����������� ������
    // 1     [LEN]   - ����������� ������������ ������
    // 2     [data0] - ������ ���� ������
    // ...
    // LEN+2 [dataN] - ��������� ���� ������



    if (MKW40_SPI_slave_read_write_buf(mkw_tx_buf, mkw_rx_buf, MKW_BUF_SZ) == MQX_OK)
    {
      uint8_t  llid = mkw_rx_buf[0] - 1;
      // ���������� ���������� ������ � ������� � �������������� ���


      if (llid < MKW_SUBSCR_NAX_CNT)
      {
        if (MKW40_subsribers[llid].receiv_func!= 0)
        {
          MKW40_subsribers[llid].receiv_func(&mkw_rx_buf[2], mkw_rx_buf[1], MKW40_subsribers[llid].pcbl);
        }
      }

      // �������� ����� ������ ��� ��������
      memset(mkw_tx_buf, 0, MKW_BUF_SZ); // �������������� �������� ����� ����� �� ���� ������� ������ ������
      _mutex_unlock(&mutex);
      // ����� ������ ��������� �������� �������� ������ � ���������� ������ mkw_tx_buf
      _mutex_lock(&mutex);
    }

  }
}

/*------------------------------------------------------------------------------
 ��������� ��������� ������ � ������� �����������

 \param llid          - ������������� ����������� ������ ��������, �� �� ������ � ������� �����������
 \param receiv_func   - ��������� �� ������� ������
 \param pcbl          - ��������� �� ��������������� ��������� ������������ ��� ������ ������� ������

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
 ��������� �������� ������ � ������ ����� MKW40


 \param llid - ������������� ����������� ������ ��������
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
