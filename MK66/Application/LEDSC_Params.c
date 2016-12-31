#include "App.h"

const T_parmenu parmenu[2]=
{
  {MDC_v1_0,MDC_v1_main,"Parameters and settings","PARAMETERS", -1},//  Основная категория
  {MDC_v1_main,MDC_v1_General,"General settings","GENERAL_SETTINGS", -1},//  
};


const T_work_params dwvar[4]=
{
  {
    " Product  name ",
    "SYSNAM",
    (void*)&wvar.name,
    tstring,
    0,
    0,
    0,
    0,
    MDC_v1_General,
    "MDC 1.0",
    "%s",
    0,
    sizeof(wvar.name),
  },
  {
    " Firmware version ",
    "FRMVER",
    (void*)&wvar.ver,
    tstring,
    0,
    0,
    0,
    0,
    MDC_v1_General,
    "?",
    "%s",
    0,
    sizeof(wvar.ver),
  },
  {
    " Enable log to file ",
    "ENBLLOG",
    (void*)&wvar.en_log_file,
    tint8u,
    1,
    0,
    1,
    0,
    MDC_v1_General,
    "",
    "%d",
    0,
    sizeof(wvar.en_log_file),
  },
  {
    " Enable verbose log ",
    "VERBLOG",
    (void*)&wvar.en_verbose_log,
    tint8u,
    0,
    0,
    1,
    0,
    MDC_v1_General,
    "",
    "%d",
    0,
    sizeof(wvar.en_verbose_log),
  },
};
