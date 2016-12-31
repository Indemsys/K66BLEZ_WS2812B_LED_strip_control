#ifndef BLE_HOST_CHANNEL_H
  #define BLE_HOST_CHANNEL_H

// �������������� ����������� ���������� ������� �� ������ ����� MKW40
// ��� �� �������������� �������� ��������� � ������� ��������
#define MKW40_SUBS_VT100    1  // ������������� ������ ��� �������� ��������� VT100
#define MKW40_SUBS_FILEMAN  2  // ������������� ������ ��� ��������� ���������
#define MKW40_SUBS_CMDMAN   3  // ������������� ������ ��� ��������� ������

#define MKW_SUBSCR_NAX_CNT  3 // ����������� ��������� � ������� �������� �� ����� ������� �� ������ ���� � MKW40


void    Init_host_channel_task(void);
int32_t Host_channel_push(uint8_t chanid, uint8_t *data, uint32_t sz);

#endif // BLE_HOST_CHANNEL_H



