#ifndef __PARAMS
  #define __PARAMS

#define  DWVAR_SIZE        4
#define  PARMNU_ITEM_NUM   2



  #define VAL_LOCAL_EDITED 0x01  //
  #define VAL_READONLY     0x02  // ����� ������ ������
  #define VAL_PROTECT      0x04  // �������� �������
  #define VAL_UNVISIBLE    0x08  // �� ��������� �� �������
  #define VAL_NOINIT       0x10  // �� ����������������


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
  const uint8_t*     name;         // ��������� ��������
  const uint8_t*     abbreviation; // �������� ������������
  void*              val;          // ��������� �� �������� ���������� � RAM
  enum  vartypes     vartype;      // ������������� ���� ����������
  float              defval;       // �������� �� ���������
  float              minval;       // ����������� ��������� ��������
  float              maxval;       // ������������ ��������� ��������  
  uint8_t            attr;         // ��������� ����������
  unsigned int       parmnlev;     // ��������� � ������� ����������� ��������
  const  void*       pdefval;      // ��������� �� ������ ��� �������������
  const  char*       format;       // ������ �������������� ��� ������ �� �������
  void               (*func)(void);// ��������� �� ������� ����������� ����� ��������������
  uint16_t           varlen;       // ������ ����������
} T_work_params;


typedef struct
{
  uint8_t        en_log_file;                   // Enable log to file | def.val.= 1
  uint8_t        en_verbose_log;                // Enable verbose log | def.val.= 0
  uint8_t        name[64];                      // Product  name | def.val.= MDC 1.0
  uint8_t        ver[64];                       // Firmware version | def.val.= ?
} WVAR_TYPE;


#endif
