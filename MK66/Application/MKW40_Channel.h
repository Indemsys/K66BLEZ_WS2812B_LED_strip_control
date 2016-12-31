#ifndef MKW40_CHANNEL_H
  #define MKW40_CHANNEL_H

// �������������� ����������� ���������� ������� �� ������ ����� MKW40
// ��� �� �������������� �������� ��������� � ������� ��������
  #define MKW40_SUBS_VT100    1  // ������������� ������ ��� �������� ��������� VT100
  #define MKW40_SUBS_FILEMAN  2  // ������������� ������ ��� ��������� ���������
  #define MKW40_SUBS_CMDMAN   3  // ������������� ������ ��� ��������� ������

  #define MKW_SUBSCR_NAX_CNT  3 // ����������� ��������� � ������� �������� �� ����� ������� �� ������ ���� � MKW40

// ���� ��������� ���������� ��������
//
// ���� �������
  #define  REPLY_DEVICE_READY_FOR_CMD   0x0000AA00  // ���������� ������ � ������
  #define  REPLY_FILE_PREPARED          0x0000AA01  // ���� �����������
  #define  REPLY_FILE_ERROR             0x0000AA03  // ������ �����
  #define  REPLY_PLAYING_END            0x0000AA04  // ��������������� ��������
  #define  REPLY_CMD_ERROR              0x01010101  // ������ �������

// ���� ������
  #define  CMD_START                0x00000002
  #define  CMD_STOP                0x00000003


typedef void (*T_MKW40_receiver)(uint8_t *data, uint32_t sz, void *ptr);

typedef struct
{
  T_MKW40_receiver   receiv_func;
  void               *pcbl; // ��������� �� ��������������� ����������� ��������� ��� ������� ���������
} T_MKW40_recv_subsc;  // ������ ��������� �� ����� �� ������ MKW40


void        Task_MKW40(uint32_t parameter);
_mqx_uint   MKW40_subscibe(uint8_t llid, T_MKW40_receiver receiv_func, void *pcbl);
_mqx_uint   MKW40_send_buf(uint8_t llid, uint8_t *data, uint32_t sz);

#endif // MKW40_CHANNEL_H



