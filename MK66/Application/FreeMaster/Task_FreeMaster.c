#include "App.h"
#include "freemaster.h"
#include "freemaster_mqx.h"

/*

������ ��������� FreeMaster



������ ������ ������ ���������� � Scope

 [2b] - ������ ������
 [04] - ������� FMSTR_CMD_READMEM_EX        0x04U
 [05] - ���������� ���� � ������ �� �������� ���������� ��� ����� � ���� ����������� �����
 [04] - ����� ������
 [f8][18][00][20] - �����
 [c3] - ����������� �����


������ ������� ������ ������ ��� �������� Scope
 [2b]
 [0a] - ������� FMSTR_CMD_SETUPSCOPE_EX     0x0aU
 [10] - ���������� ���� � ������ �� �������� ���������� ��� ����� � ���� ����������� �����
 [03] - ���������� ���������� � Scope
 [04] - ������ ������ ����������
 [84 1a 00  20] - ����� ������ ����������
 [04] - ������ ������ ����������
 [f8 18  00 20] - ����� ������ ����������
 [04] - ������ ������� ����������
 [98  1a 00 20] - ����� ������� ����������
 [17] - ����������� �����

������ ������� ������� ���������� �������� ��� �������� Scope
 [2b] - ������ ������
 [c5] - FMSTR_CMD_READSCOPE         0xc5U
 [3b] - ����������� �����

������ ������ �� ������
 [2b] - ������ ������
 [00] - ���������
 [00 00 00 00] - �������� ����������
 [00] - ����������� �����. ����������� ���:  ~(����� ���� ���� ����� 2b) + 1

������ ������� ������ ������ ��� �������� Recorder

 [2b] - ������ ������
 [0b] - ������� FMSTR_CMD_SETUPREC_EX       0x0bU
 [21] - ���������� ���� � ������ �� �������� ���������� ��� ����� � ���� ����������� �����
 [01] - ��� ������������ ������� ��� ������ ������
 [18 15] - ���������� �������
 [03 15] - ���������� ������� ������������ ����� ��������
 [01 00] - ������������ ������� �������
 [ac 2f  00 20] - ����� ���������� ��������
 [04] - ������ ���������� ��������
 [00] - ������ ��������� �������� (��������, �����������)
 [01 00 00 00] - �������� �������� ���������� ��������
 [03] - ���������� ������������ ����������
 [04] - ������ ������ ����������
 [98 1a  00 20] - ����� ������ ����������
 [04] - ������ ������ ����������
 [a0  1a 00 20] - ����� ������ ����������
 [04] - ������ ������� ����������
 [ec 2f 00 20] - ����� ������� ����������
 [97] - ����������� �����

������ ������� ������� ���������� �������� �������� Recorder
 [2b] - ������ ������
 [c3] - FMSTR_CMD_GETRECSTS         0xc3U    get the recorder status
 [3d] - ����������� �����

����� ��� ���������� ������
 [2b] - ������ ������
 [01] - FMSTR_STS_RECRUN            0x01U
 [ff] - ����������� �����

����� ��� ������� ������
 [2b] - ������ ������
 [02] - FMSTR_STS_RECDONE           0x02U
 [fe] - ����������� �����

������ ������� ������� ������ ��� �������� Recorder
 [2b] - ������ ������
 [c9] - FMSTR_CMD_GETRECBUFF_EX     0xc9U
 [37] - ����������� �����

������ ������ �� ������ ������ ��� �������� Recorder
 [2b] - ������ ������
 [00] - FMSTR_STS_OK                0x00U
 [b8 16 ff 1f] - ����� ������ � �������
 [96  02] - ������ ���������� ������� � ������ + 1 (pcm_wRecBuffStartIx)
 [7c] - ����������� �����

������ ������� �� ������ ������ �� ������� ��� �������� Recorder
 [2b] - ������ ������
 [04] - FMSTR_CMD_READMEM_EX        0x04U    read a block of memory
 [05] - ���������� ���� � ������ �� �������� ���������� ��� ����� � ���� ����������� �����
 [3c] - ���������� �������� ����
 [b8 16 ff 1f] - ��������� �����
 [cf] - ����������� �����


 */



uint8_t myhandler(uint8_t /*nAppcmd*/, uint8_t* /*pData*/, uint16_t /*nDataLen*/);

#if !BSPCFG_ENABLE_IO_SUBSYSTEM
  #error This application requires BSPCFG_ENABLE_IO_SUBSYSTEM defined non-zero in user_config.h. Please recompile BSP with this option.
#endif


#ifndef BSP_DEFAULT_IO_CHANNEL_DEFINED
  #error This application requires BSP_DEFAULT_IO_CHANNEL to be not NULL. Please set corresponding BSPCFG_ENABLE_TTYx to non-zero in user_config.h and recompile BSP with this option.
#endif
FMSTR_TSA_TABLE_LIST_BEGIN()
FMSTR_TSA_TABLE(tbl_measres)
#ifdef SB200M
FMSTR_TSA_TABLE(tbl_sb200m_measres)
FMSTR_TSA_TABLE(tbl_motctrl)
#endif
FMSTR_TSA_TABLE(tbl_zerdet)
FMSTR_TSA_TABLE(tbl_adc)
FMSTR_TSA_TABLE_LIST_END()


/*-------------------------------------------------------------------------------------------------------------
  ������� ��������� ������ �������. �������������� �������� FMSTR_RegisterAppCmdCall
-------------------------------------------------------------------------------------------------------------*/
uint8_t myhandler(uint8_t code/*nAppcmd*/, uint8_t* pdata/*pData*/, uint16_t size/*nDataLen*/)
{
  return 0x10;  // ���������� ����� ����� ���������
}



/*-------------------------------------------------------------------------------------------------------------
  ���� ������ FreeMaster
-------------------------------------------------------------------------------------------------------------*/
void Task_FreeMaster(uint32_t initial_data)
{
  uint16_t pDataLen;
  uint8_t *dbuf;
  _mqx_uint res;

  unsigned short nAppCmdCode;

  if ( !FMSTR_Init() )
  {
    _time_delay(200L);
    return;
  }

  // registering the App.Command handler
  // FMSTR_RegisterAppCmdCall(10, myhandler); �� ������������. ����� ������� ������� �������������� ��������� ����� Wait for result � �������, ����� ��������� ���������� �������

  while (TRUE)
  {
    nAppCmdCode = FMSTR_GetAppCmd();

    if ( nAppCmdCode != FMSTR_APPCMDRESULT_NOCMD )
    {
      switch (nAppCmdCode)
      {
      case 99:
        // ������� ������ �� ������
        FMSTR_AppCmdAck(0);
        return;
      default: FMSTR_AppCmdAck(0);
        break;
      }
    }
    FMSTR_Poll();
  }
}


