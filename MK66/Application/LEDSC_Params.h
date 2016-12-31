#ifndef __PARAMS
  #define __PARAMS

#define  DWVAR_SIZE        4
#define  PARMNU_ITEM_NUM   2



  #define VAL_LOCAL_EDITED 0x01  //
  #define VAL_READONLY     0x02  // Можно только читать
  #define VAL_PROTECT      0x04  // Защишено паролем
  #define VAL_UNVISIBLE    0x08  // Не выводится на дисплей
  #define VAL_NOINIT       0x10  // Не инициализируется


enum vartypes
{
    tint8u  = 1,
    tint16u  = 2,
    tint32u  = 3,
    tfloat  = 4,
    tarrofdouble  = 5,
    tstring  = 6,
    tarrofbyte  = 7,
    tint32s  = 8,
};


enum enm_parmnlev
{
    MDC_v1_0,
    MDC_v1_main,
    MDC_v1_General,
};


typedef struct 
{
  enum enm_parmnlev prevlev;
  enum enm_parmnlev currlev;
  const char* name;
  const char* shrtname;
  const char  visible;
}
T_parmenu;


typedef struct
{
  const uint8_t*     name;         // Строковое описание
  const uint8_t*     abbreviation; // Короткая аббревиатура
  void*              val;          // Указатель на значение переменной в RAM
  enum  vartypes     vartype;      // Идентификатор типа переменной
  float              defval;       // Значение по умолчанию
  float              minval;       // Минимальное возможное значение
  float              maxval;       // Максимальное возможное значение  
  uint8_t            attr;         // Аттрибуты переменной
  unsigned int       parmnlev;     // Подгруппа к которой принадлежит параметр
  const  void*       pdefval;      // Указатель на данные для инициализации
  const  char*       format;       // Строка форматирования при выводе на дисплей
  void               (*func)(void);// Указатель на функцию выполняемую после редактирования
  uint16_t           varlen;       // Длинна переменной
} T_work_params;


typedef struct
{
  uint8_t        en_log_file;                   // Enable log to file | def.val.= 1
  uint8_t        en_verbose_log;                // Enable verbose log | def.val.= 0
  uint8_t        name[64];                      // Product  name | def.val.= MDC 1.0
  uint8_t        ver[64];                       // Firmware version | def.val.= ?
} WVAR_TYPE;


#endif
