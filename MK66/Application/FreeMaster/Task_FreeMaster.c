#include "App.h"
#include "freemaster.h"
#include "freemaster_mqx.h"

/*

Анализ протокола FreeMaster



Пример пакета чтения переменной в Scope

 [2b] - начало пакета
 [04] - команда FMSTR_CMD_READMEM_EX        0x04U
 [05] - количество байт в пакете не учитывая предыдущие два байта и байт контрольной суммы
 [04] - длина адреса
 [f8][18][00][20] - адрес
 [c3] - контрольная сумма


Пример команды начала чтения для элемента Scope
 [2b]
 [0a] - команда FMSTR_CMD_SETUPSCOPE_EX     0x0aU
 [10] - количество байт в пакете не учитывая предыдущие два байта и байт контрольной суммы
 [03] - количество переменных в Scope
 [04] - Размер первой переменной
 [84 1a 00  20] - Адрес первой переменной
 [04] - Размер второй переменной
 [f8 18  00 20] - Адрес второй переменной
 [04] - Размер третьей переменной
 [98  1a 00 20] - Адрес третьей переменной
 [17] - контрольная сумма

Пример команды запроса следующего значения для элемента Scope
 [2b] - начало пакета
 [c5] - FMSTR_CMD_READSCOPE         0xc5U
 [3b] - контрольная сумма

Пример ответа на запрос
 [2b] - начало пакета
 [00] - константа
 [00 00 00 00] - Значение переменной
 [00] - контрольная сумма. Вычисляется как:  ~(сумма всех байт кроме 2b) + 1

Пример команды начала чтения для элемента Recorder

 [2b] - начало пакета
 [0b] - команда FMSTR_CMD_SETUPREC_EX       0x0bU
 [21] - количество байт в пакете не учитывая предыдущие два байта и байт контрольной суммы
 [01] - тип применяемого тригера для начала записи
 [18 15] - количество сэмплов
 [03 15] - количество сэмплов записываемых после триггера
 [01 00] - предделитель частоты выборки
 [ac 2f  00 20] - адрес переменной триггера
 [04] - размер переменной триггера
 [00] - способ сравнения триггера (знаковый, беззнаковый)
 [01 00 00 00] - значение величины переменной триггера
 [03] - количество записываемых переменных
 [04] - размер первой переменной
 [98 1a  00 20] - Адрес первой переменной
 [04] - Размер второй переменной
 [a0  1a 00 20] - Адрес второй переменной
 [04] - Размер третьей переменной
 [ec 2f 00 20] - Адрес третьей переменной
 [97] - контрольная сумма

Пример команды запроса состоянпия триггера элемента Recorder
 [2b] - начало пакета
 [c3] - FMSTR_CMD_GETRECSTS         0xc3U    get the recorder status
 [3d] - контрольная сумма

Ответ при отсутствии данных
 [2b] - начало пакета
 [01] - FMSTR_STS_RECRUN            0x01U
 [ff] - контрольная сумма

Ответ при наличии данных
 [2b] - начало пакета
 [02] - FMSTR_STS_RECDONE           0x02U
 [fe] - контрольная сумма

Пример команды запроса данных для элемента Recorder
 [2b] - начало пакета
 [c9] - FMSTR_CMD_GETRECBUFF_EX     0xc9U
 [37] - контрольная сумма

Пример ответа на запрос данных для элемента Recorder
 [2b] - начало пакета
 [00] - FMSTR_STS_OK                0x00U
 [b8 16 ff 1f] - адрес буфера с данными
 [96  02] - индекс последнего отсчета в буфере + 1 (pcm_wRecBuffStartIx)
 [7c] - контрольная сумма

Пример запроса на чтение данных из буффера для элемента Recorder
 [2b] - начало пакета
 [04] - FMSTR_CMD_READMEM_EX        0x04U    read a block of memory
 [05] - количество байт в пакете не учитывая предыдущие два байта и байт контрольной суммы
 [3c] - количество читаемых байт
 [b8 16 ff 1f] - начальный адрес
 [cf] - контрольная сумма


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
  Функция обратного вызова команды. Регистрируется функцией FMSTR_RegisterAppCmdCall
-------------------------------------------------------------------------------------------------------------*/
uint8_t myhandler(uint8_t code/*nAppcmd*/, uint8_t* pdata/*pData*/, uint16_t size/*nDataLen*/)
{
  return 0x10;  // Возвращать можем любой результат
}



/*-------------------------------------------------------------------------------------------------------------
  Цикл движка FreeMaster
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
  // FMSTR_RegisterAppCmdCall(10, myhandler); Не регистрируем. Такие команды требуют дополнительной установки флага Wait for result в диалоге, иначе блокируют дальнейшие команды

  while (TRUE)
  {
    nAppCmdCode = FMSTR_GetAppCmd();

    if ( nAppCmdCode != FMSTR_APPCMDRESULT_NOCMD )
    {
      switch (nAppCmdCode)
      {
      case 99:
        // Команды выхода из режима
        FMSTR_AppCmdAck(0);
        return;
      default: FMSTR_AppCmdAck(0);
        break;
      }
    }
    FMSTR_Poll();
  }
}


