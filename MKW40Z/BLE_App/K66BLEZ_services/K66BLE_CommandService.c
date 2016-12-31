// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.09.25
// 20:49:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "K66BLEZ.h"

// ���� �������
#define  REPLY_DEVICE_READY_FOR_CMD   0x0000AA00  // ���������� ������ � ������
#define  REPLY_FILE_PREPARED          0x0000AA01  // ���� �����������
#define  REPLY_FILE_ERROR             0x0000AA03  // ������ �����

// ���� ������
#define  CMD_PLAY_FILE                0x00000002
#define  CMD_STOP_PLAY                0x00000003

static deviceId_t        g_peer_device_id;
/*------------------------------------------------------------------------------
  ����������� ������ � �������������� ������� ������


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

  // ���������� ���������� �� �������� ����������� ������ �� ������
  g_peer_device_id = peer_device_id;
  // ��������� �������� �� �� ������� ��� ��� �����
  if (len == 4)
  {
    memcpy(&cmd, value, 4);
  }
  else cmd = 0;

  if (cmd == CMD_PLAY_FILE)
  {
    // ������� ������ ��������������� �����
    DEBUG_CMDSERV_PRINT_ARG("Command to play file\r\n", 0);
    // ���������� ������� �����
    Host_channel_push(MKW40_SUBS_CMDMAN, (uint8_t *)&cmd, sizeof(cmd));
  }
  else if (cmd == CMD_STOP_PLAY)
  {
    // ������� ��������� ��������������� �����
    DEBUG_CMDSERV_PRINT_ARG("Command to stop playing\r\n", 0);
    // ���������� ������� �����
    Host_channel_push(MKW40_SUBS_CMDMAN, (uint8_t *)&cmd, sizeof(cmd));
  }
  else
  {
    // ������� ��� ��� ��� �����
    DEBUG_CMDSERV_PRINT_ARG("File name received\r\n", 0);
    DEBUG_CMDSERV_PRINT_ARG("%s\r\n", value);

    // ���������� ������� �����
    Host_channel_push(MKW40_SUBS_CMDMAN, value, len);


    // ����� ��������� �����.
    // ������ � ����� ������� ��� ������ ���� �������� �� �������������� ������ ���������� � ���������� �� ���������
    //{
    //  uint32_t val = REPLY_FILE_PREPARED;
    //  GattDb_WriteAttribute(value_k66_cmd_read, sizeof(val), (uint8_t *)&val);
    //  GattServer_SendNotification(g_peer_device_id, value_k66_cmd_read);
    //}

  }


  return gAttErrCodeNoError_c;

}

/*------------------------------------------------------------------------------
   ����� ������ �� ������ �����
   ����������� � ��������� ������ Host_Chan_Task


 \param data
 \param sz
 ------------------------------------------------------------------------------*/
void CommandService_receive_from_host(uint8_t *data, uint32_t sz)
{
  // ���������� ����� �� ���� ����������� � ��������������
  GattDb_WriteAttribute(value_k66_cmd_read, sz, data);
  // ����� �� �������� �����������
  GattServer_SendNotification(g_peer_device_id, value_k66_cmd_read);

}
/*------------------------------------------------------------------------------
  ����������� ������ �������������� ������ ������
  ���� �������� � ���� �������� ������� ������ ���������� ������� � ������


 \param void
 ------------------------------------------------------------------------------*/
void CommandService_read(void)
{
  uint32_t val = REPLY_DEVICE_READY_FOR_CMD;

  DEBUG_CMDSERV_PRINT_ARG("Intercepted red cmd\r\n", 0);

  GattDb_WriteAttribute(value_k66_cmd_read, sizeof(val), (uint8_t *)&val); // ���������� ������ � ��������������

}
