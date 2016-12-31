#include   "App.h"




// �������� ������ ����
// ���������� � ��������� ������ - �� ���� ����� �� ������ ��������� ��� ������������ ������
extern const T_parmenu parmenu[];
extern const T_work_params dwvar[];

#define  INI_STR_SIZE     255
#define  FILE_BUF_SZ      512


static char     param_str[INI_STR_SIZE+1];


static const char *PARAM_EDITOR_HELP = "\033[5C Press digit key to select menu item.\r\n"
                                       "\033[5C <M> - Main menu, <R> - return on prev. level\r\n"
                                       "\033[5C 2 times Enter - Accept, Esc - Cancel, ^[H - erase\r\n";



static void     Goto_to_edit_param(void);
static uint8_t* Get_mn_caption(void);
static uint8_t  Get_mn_prevlev(void);

int             Read_cstring_from_buf(char **buf, char *str, uint32_t len);
char*           Trim_and_dequote_str(char *str);
/*-------------------------------------------------------------------------------------------
  �������������� ���������� �� ���������, ����� ����� ������� ��� ����� ������
---------------------------------------------------------------------------------------------*/
void Return_def_params(void)
{
  uint16_t  i;

  // ��������� ��������� ���������� �� ���������
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if ((dwvar[i].attr & VAL_NOINIT) == 0)
    {
      switch (dwvar[i].vartype)
      {
        // tint8u, tint16u, tint32u, tfloat, tarrofdouble, tarrofbyte
      case tint8u:
        *(uint8_t *)dwvar[i].val = (uint8_t)dwvar[i].defval;
        break;
      case tint16u:
        *(uint16_t *)dwvar[i].val = (uint16_t)dwvar[i].defval;
        break;
      case tint32u:
        *(uint32_t *)dwvar[i].val = (uint32_t)dwvar[i].defval;
        break;
      case tint32s:
        *(int32_t *)dwvar[i].val = (int32_t)dwvar[i].defval;
        break;
      case tfloat:
        *(float *)dwvar[i].val = (float)dwvar[i].defval;
        break;
      case tstring:
        {
          uint8_t *st;

          strncpy(dwvar[i].val, (const char *)dwvar[i].pdefval, dwvar[i].varlen - 1);
          st = dwvar[i].val;
          st[dwvar[i].varlen - 1] = 0;
        }
        break;
      case tarrofbyte:
        memcpy(dwvar[i].val, dwvar[i].pdefval, dwvar[i].varlen);
        break;
      }
    }
  }

// ���������� ����������������� ������� ����������
//  !!! ���������� ������� ��� ������������� ���������� �� ��������� ����� �������� � ��������� ������ � ID � ������������ !!!
/*
  for ( i=0;i<DWVAR_SIZE;i++ )
  {
    if ( (dwvar[i].attr & VAL_NOINIT)==0 )
    {
      if ( dwvar[i].func!=0 ) dwvar[i].func();
    }
  }
*/


}



/*-------------------------------------------------------------------------------------------
  �������� ���������� �� �����, ����� ������ �������
---------------------------------------------------------------------------------------------*/
int32_t Restore_parameters_and_settings(void)
{
  int status;

  // ������� ������� � ��������� �������� �� ���������
  Return_def_params();
  LOG("Default settings are restored.", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);

  if (K66BLEZ1_VBAT_RAM_validation() == MQX_OK)
  {
    LOG("VBAT RAM data is valid.", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
  }
  else
  {
    LOG("VBAT RAM data is incorrect.", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
  }

  status = Restore_from_INIfile();
  if (status != MQX_OK)
  {
    Return_def_params();
    Save_Params_to_INIfile();
  }

  snprintf((char *)wvar.ver, sizeof(wvar.ver), "%s %s", __DATE__, __TIME__);
  return (MQX_OK);
}



/*-------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------*/
int Restore_from_Flash(void)
{
  int result = MQX_OK;


//  if (SPIFlash_chek_chip() != MQX_OK)
//  {
//    LOG("Restore from FLASH error.", __FUNCTION__, __LINE__, SEVERITY_RED);
//    return MQX_ERROR;
//  }
//  result = SPIFlash_read_databuf(0, (uint8_t *)&wvar, sizeof(wvar));


  return result;
}

/*-------------------------------------------------------------------------------------------
   ��������� ���������� � ini-���� ����������
---------------------------------------------------------------------------------------------*/
int Save_Params_to_Flash(void)
{
  int result = MQX_OK;


//  if (SPIFlash_chek_chip() != MQX_OK)
//  {
//    LOG("Saving to FLASH error.", __FUNCTION__, __LINE__, SEVERITY_RED);
//    return MQX_ERROR;
//  }
//  result = SPIFlash_reprogr_databuf(0, (uint8_t *)&wvar, sizeof(wvar));
//
//
//  LOG("Saving to FLASH done.", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);


  return MQX_OK;
}

/*-------------------------------------------------------------------------------------------
  ����� �������� ���� �� ��� ��������������
---------------------------------------------------------------------------------------------*/
static uint8_t* Get_mn_caption(void)
{
  int i;
  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].currlev == pvt100_cb->current_param_category) return(uint8_t *)parmenu[i].name;
  }
  return(uint8_t *)parmenu[0].name;
}

/*-------------------------------------------------------------------------------------------
  ����� �������� �������� ���� �� ��� ��������������
---------------------------------------------------------------------------------------------*/
uint8_t* Get_mn_shrtcapt(enum enm_parmnlev menu_lev)
{
  uint16_t i;
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].currlev == menu_lev) return(uint8_t *)parmenu[i].shrtname;
  }
  return(uint8_t *)parmenu[0].shrtname;
}

/*-------------------------------------------------------------------------------------------
  ����� �������� ���� �� ��� ��������������
---------------------------------------------------------------------------------------------*/
uint8_t* Get_mn_name(enum enm_parmnlev menu_lev)
{
  uint16_t i;
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].currlev == menu_lev) return(uint8_t *)parmenu[i].name;
  }
  return(uint8_t *)parmenu[0].name;
}


/*-------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------*/
static uint8_t Get_mn_prevlev(void)
{
  int i;
  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].currlev == pvt100_cb->current_param_category) return (parmenu[i].prevlev);
  }
  return (MAIN_PARAMS_ROOT);
}

/*-------------------------------------------------------------------------------------------
  ������� ���������� ���� ��� ��� ����� � ����� �������������� ����������
---------------------------------------------------------------------------------------------*/
void Do_Params_editor(uint8_t keycode)
{
  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  pvt100_cb->current_param_category = MAIN_PARAMS_ROOT;
  Display_params_menu();                 // �������� ���� ����������
  Set_monitor_func(Display_params_func); // �������������� ����������� ������� � ������� ����� ��������
}

/*-------------------------------------------------------------------------------------------
  ����� �������� ���� �� ��� ��������������
---------------------------------------------------------------------------------------------*/
void Display_params_menu(void)
{
  int             i, n;
  uint8_t           str[80];
  uint8_t          *st;
  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  pvt100_cb->_printf(VT100_CLEAR_AND_HOME);
  // ����� ��������� ���� ����������
  VT100_send_str_to_pos(DASH_LINE, 2, 0);
  st = Get_mn_caption();
  VT100_send_str_to_pos(st, 1, Find_str_center(st));
  VT100_send_str_to_pos((uint8_t *)PARAM_EDITOR_HELP, 3, 0);
  pvt100_cb->_printf("\r\n");
  pvt100_cb->_printf(DASH_LINE);

  // ������� ������ ���� ������� �� ������ ������
  n = 0;
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].prevlev != pvt100_cb->current_param_category) continue;
    sprintf((char *)str, "%1X - %s", n, parmenu[i].name);
    if ((strlen((char *)str) + SCR_ITEMS_HOR_OFFS) > COLCOUNT)
    {
      str[COLCOUNT - SCR_ITEMS_HOR_OFFS - 3] = '.';
      str[COLCOUNT - SCR_ITEMS_HOR_OFFS - 2] = '>';
      str[COLCOUNT - SCR_ITEMS_HOR_OFFS - 1] = 0;
    }
    VT100_send_str_to_pos(str, SCR_ITEMS_VERT_OFFS + n, SCR_ITEMS_HOR_OFFS);
    n++;
  }
  pvt100_cb->current_categories_count = n;

  // ������� ������ ���� ���������� ����������� � ����� ������
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    int len;
    if (dwvar[i].parmnlev != pvt100_cb->current_param_category) continue;
    VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS + n, SCR_ITEMS_HOR_OFFS);
    sprintf((char *)str, "%1X - ", n);
    pvt100_cb->_printf((char *)str);
    len = strlen((char *)str) + SCR_ITEMS_HOR_OFFS;
    pvt100_cb->_printf(VT100_REVERSE_ON);
    pvt100_cb->_printf((char *)dwvar[i].name);
    pvt100_cb->_printf("= ");
    len = len + strlen((char *)dwvar[i].name) + 2;
    pvt100_cb->_printf(VT100_REVERSE_OFF);
    // ������������� �������� � ������
    Param_to_str(str, 80, i);
    if ((strlen((char *)str) + len) > COLCOUNT)
    {
      str[COLCOUNT - len - 3] = '.';
      str[COLCOUNT - len - 2] = '>';
      str[COLCOUNT - len - 1] = 0;
    }
    pvt100_cb->_printf((char *)str);
    n++;
  }
  pvt100_cb->current_category_items_count = n;

}

/*-------------------------------------------------------------------------------------------
  ������� ������������ ���������� � �������� ����������� �� �������� ����� ��������
---------------------------------------------------------------------------------------------*/
void Display_params_func(uint8_t b)
{
  int           i;
  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  if (b < 0x30)
  {
    return;
  }

  if ((b == 'M') || (b == 'm'))
  {
    Goto_main_menu();
    return;
  }
  if ((b == 'R') || (b == 'r'))
  {
    if (pvt100_cb->current_param_category != MAIN_PARAMS_ROOT)
    {
      pvt100_cb->current_param_category = Get_mn_prevlev();
      Display_params_menu();
      return;
    }
    else
    {
      // ����� �� ����� ���� �� �������� ����� � �������� ����������
      Return_to_prev_menu();
      Display_menu();
      return;
    }

  }

  b = ascii_to_hex(b);
  if (b < pvt100_cb->current_category_items_count)
  {
    if (b < pvt100_cb->current_categories_count)
    {
      // ������ ����� �������
      uint8_t k = 0;
      for (i = 0; i < PARMNU_ITEM_NUM; i++)
      {
        if (parmenu[i].prevlev == pvt100_cb->current_param_category)
        {
          if (k == b)
          {
            pvt100_cb->current_param_category = parmenu[i].currlev;
            Display_params_menu();
            return;
          }
          k++;
        }
      }

    }
    else
    {
      // ������ �������� ��� ��������������
      // ����� ������ ��������� ���� ����� ���� � ������������� ����
      uint8_t  n = 0;
      for (i = 0; i < DWVAR_SIZE; i++)
      {
        if (dwvar[i].parmnlev == pvt100_cb->current_param_category)
        {
          if (n == (b -  pvt100_cb->current_categories_count))
          {
            pvt100_cb->par_indx = i;
            Goto_to_edit_param();
            return;
          }
          n++;
        }
      }
    }

    return;
  }

}



/*-------------------------------------------------------------------------------------------
  ������� � �������������� ���������

  ���������:
             n    - ������ ��������� � ������� ����������
             item - ����� ��������� � ����
---------------------------------------------------------------------------------------------*/
static void Goto_to_edit_param(void)
{
  int n;
  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  pvt100_cb->rowcount = (MAX_PARAMETER_STRING_LEN + 1) / COLCOUNT;
  if (((MAX_PARAMETER_STRING_LEN + 1) % COLCOUNT) > 0) pvt100_cb->rowcount++;
  if (pvt100_cb->rowcount > ROWCOUNT - 2) return;  // ������� ������� ������� ��������������
  pvt100_cb->firstrow = ROWCOUNT - pvt100_cb->rowcount + 1;
  VT100_set_cursor_pos(pvt100_cb->firstrow - 2, 0);
  pvt100_cb->_printf("Edited parameter: '%s'\r\n", dwvar[pvt100_cb->par_indx].name);
  pvt100_cb->_printf(DASH_LINE);
  Param_to_str(pvt100_cb->param_str, MAX_PARAMETER_STRING_LEN, pvt100_cb->par_indx);
  VT100_set_cursor_pos(pvt100_cb->firstrow, 0);
  pvt100_cb->_printf(VT100_CLR_FM_CRSR);
  VT100_set_cursor_pos(pvt100_cb->firstrow, 0);
  pvt100_cb->_printf(">");
  pvt100_cb->_printf(VT100_CURSOR_ON);
  pvt100_cb->_printf((char *)pvt100_cb->param_str);
  // �������� ��� ������ ��������� ������
  n = strlen((char *)pvt100_cb->param_str);
  if (n != 0)
  {
    pvt100_cb->current_row = (n + 2) / COLCOUNT;
    pvt100_cb->current_col = (n + 2) % COLCOUNT;
    pvt100_cb->current_pos =  n;
  }
  else
  {
    pvt100_cb->current_row = 0;
    pvt100_cb->current_col = 2;
    pvt100_cb->current_pos = 0;
  }
  VT100_set_cursor_pos(pvt100_cb->firstrow + pvt100_cb->current_row, pvt100_cb->current_col);
  pvt100_cb->ed_enter_cnt = 0;
  Set_monitor_func(Edit_func);
}

/*-------------------------------------------------------------------------------------------
   ������� �������������� ��������� � ���� ���������
---------------------------------------------------------------------------------------------*/
void Edit_func(uint8_t b)
{
  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  switch (b)
  {
  case 0x08:  // Back Space
    if (pvt100_cb->current_pos > 0)
    {
      pvt100_cb->current_pos--;
      pvt100_cb->param_str[pvt100_cb->current_pos] = 0;

      if (pvt100_cb->current_col < 2)
      {
        pvt100_cb->current_col = COLCOUNT;
        pvt100_cb->current_row--;
      }
      else
      {
        pvt100_cb->current_col--;
      }
    }
    VT100_set_cursor_pos(pvt100_cb->firstrow + pvt100_cb->current_row, pvt100_cb->current_col);
    pvt100_cb->_printf(" ");
    VT100_set_cursor_pos(pvt100_cb->firstrow + pvt100_cb->current_row, pvt100_cb->current_col);
    pvt100_cb->ed_enter_cnt = 0;
    break;
  case 0x7E:  // DEL
  case 0x7F:  // DEL
    pvt100_cb->current_row = 0;
    pvt100_cb->current_col = 2;
    pvt100_cb->current_pos = 0;
    pvt100_cb->param_str[pvt100_cb->current_pos] = 0;
    VT100_set_cursor_pos(pvt100_cb->firstrow + pvt100_cb->current_row, pvt100_cb->current_col);
    pvt100_cb->_printf(VT100_CLR_FM_CRSR);
    pvt100_cb->ed_enter_cnt = 0;
    break;
  case 0x1B:  // ESC

    pvt100_cb->_printf(VT100_CURSOR_OFF);
    Display_params_menu();
    Set_monitor_func(Display_params_func);

    break;
  case 0x0D:  // Enter
    pvt100_cb->ed_enter_cnt++;
    if (pvt100_cb->ed_enter_cnt < 2)
    {
      break;
    }
    pvt100_cb->param_str[pvt100_cb->current_pos] = 0;

    Str_to_param(pvt100_cb->param_str, pvt100_cb->par_indx);
    Save_Params_to_INIfile();  // ��������� ��������� � ����
    Execute_param_func(pvt100_cb->par_indx);
    pvt100_cb->_printf(VT100_CURSOR_OFF);
    Display_params_menu();
    Set_monitor_func(Display_params_func);
    break;
  default:
    if (isspace(b) || isgraph(b))
    {
      pvt100_cb->param_str[pvt100_cb->current_pos] = b;
      pvt100_cb->_printf("%c", b);
      if (pvt100_cb->current_pos < (MAX_PARAMETER_STRING_LEN - 1))
      {
        pvt100_cb->current_pos++;
        if (pvt100_cb->current_col == (COLCOUNT))
        {
          pvt100_cb->current_col = 1;
          pvt100_cb->current_row++;
        }
        else pvt100_cb->current_col++;
      }
      VT100_set_cursor_pos(pvt100_cb->firstrow + pvt100_cb->current_row, pvt100_cb->current_col);
    }
    pvt100_cb->ed_enter_cnt = 0;
    break;

  }

}


/*-------------------------------------------------------------------------------------------
   ������������� ������ � ��������
---------------------------------------------------------------------------------------------*/
void Str_to_param(uint8_t *buf, uint16_t indx)
{
  uint8_t  uch_tmp;
  uint16_t uin_tmp;
  uint32_t ulg_tmp;
  int32_t slg_tmp;
  char *end;
  float  d_tmp;
  // tint8u, tint16u, tint32u, tfloat, tstring, tarrofdouble, tarrofbyte
  switch (dwvar[indx].vartype)
  {
  case tint8u:
    uch_tmp = strtol((char *)buf, &end, 10);
    if (uch_tmp > ((uint8_t)dwvar[indx].maxval)) uch_tmp = (uint8_t)dwvar[indx].maxval;
    if (uch_tmp < ((uint8_t)dwvar[indx].minval)) uch_tmp = (uint8_t)dwvar[indx].minval;
    *(uint8_t *)dwvar[indx].val = uch_tmp;
    break;
  case tint16u:
    uin_tmp = strtol((char *)buf, &end, 10);
    if (uin_tmp > ((uint16_t)dwvar[indx].maxval)) uin_tmp = (uint16_t)dwvar[indx].maxval;
    if (uin_tmp < ((uint16_t)dwvar[indx].minval)) uin_tmp = (uint16_t)dwvar[indx].minval;
    *(uint16_t *)dwvar[indx].val = uin_tmp;
    break;
  case tint32u:
    ulg_tmp = strtol((char *)buf, &end, 10);
    if (ulg_tmp > ((uint32_t)dwvar[indx].maxval)) ulg_tmp = (uint32_t)dwvar[indx].maxval;
    if (ulg_tmp < ((uint32_t)dwvar[indx].minval)) ulg_tmp = (uint32_t)dwvar[indx].minval;
    *(uint32_t *)dwvar[indx].val = ulg_tmp;
    break;
  case tint32s:
    slg_tmp = strtol((char *)buf, &end, 10);
    if (slg_tmp > ((int32_t)dwvar[indx].maxval)) slg_tmp = (uint32_t)dwvar[indx].maxval;
    if (slg_tmp < ((int32_t)dwvar[indx].minval)) slg_tmp = (uint32_t)dwvar[indx].minval;
    *(uint32_t *)dwvar[indx].val = slg_tmp;
    break;
  case tfloat:
    d_tmp = strtod((char *)buf, &end);
    if (d_tmp > ((float)dwvar[indx].maxval)) d_tmp = (float)dwvar[indx].maxval;
    if (d_tmp < ((float)dwvar[indx].minval)) d_tmp = (float)dwvar[indx].minval;
    *(float *)dwvar[indx].val = d_tmp;
    break;
  case tstring:
    {
      uint8_t *st;
      strncpy(dwvar[indx].val, (char *)buf, dwvar[indx].varlen - 1);
      st = dwvar[indx].val;
      st[dwvar[indx].varlen - 1] = 0;

    }
    break;
  case tarrofbyte:
    break;
  }
}

/*-------------------------------------------------------------------------------------------
   ������������� �������� � ������
---------------------------------------------------------------------------------------------*/
void Param_to_str(uint8_t *buf, uint16_t maxlen, uint16_t indx)
{
  void *val;

  // tint8u, tint16u, tint32u, tdouble, tstring, tarrofdouble, tarrofbyte
  val = dwvar[indx].val;
  switch (dwvar[indx].vartype)
  {
  case tint8u:
    snprintf((char *)buf, maxlen, dwvar[indx].format, *(uint8_t *)val);
    break;
  case tint16u:
    snprintf((char *)buf, maxlen, dwvar[indx].format, *(uint16_t *)val);
    break;
  case tint32u:
    snprintf((char *)buf, maxlen, dwvar[indx].format, *(uint32_t *)val);
    break;
  case tint32s:
    snprintf((char *)buf, maxlen, dwvar[indx].format, *(int32_t *)val);
    break;
  case tfloat:
    {
      float f;
      f = *((float *)val);
      snprintf((char *)buf, maxlen, dwvar[indx].format, f);
      break;
    }
  case tstring:
    {
      int len;
      if (dwvar[indx].varlen > maxlen)
      {
        len = maxlen - 1;
      }
      else
      {
        len = dwvar[indx].varlen - 1;
      }
      strncpy((char *)buf, (char *)val, len);
      buf[len] = 0;
    }
    break;
  default:
    break;
  }
}

/*-------------------------------------------------------------------------------------------
  ����� ������ ��������� �� ������������
---------------------------------------------------------------------------------------------*/
int32_t Find_param_by_abbrev(char *abbrev)
{
  int i;
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if (strcmp((char *)dwvar[i].abbreviation, abbrev) == 0) return (i);
  }
  return (-1);
}

/*-------------------------------------------------------------------------------------------
  �������� ��� ���������
---------------------------------------------------------------------------------------------*/
enum  vartypes Get_param_type(int n)
{
  return (dwvar[n].vartype);
}

/*-------------------------------------------------------------------------------------------
   ����� ������� ���������� ���� �� ������ ������ ����
---------------------------------------------------------------------------------------------*/
int16_t Get_menu_items_count(enum enm_parmnlev menu_item)
{
  int16_t i;
  int16_t n;
  n = 0;
  // ������� �������
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].prevlev != menu_item) continue;
    n++;
  }
  // ������� ���� ��������� �������� � ����
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if (dwvar[i].parmnlev != menu_item) continue;
    n++;
  }
  return (n);
}

/*-------------------------------------------------------------------------------------------
   ���������� ��������� �� �������� ������ ������� ��� ��������� ��� ��������� ������ ����
   curr_menu - ������������� �������� ����
   item      - ����� �������� ��������� � ����
   ����������:
   submenu   - ���������� �������, �� ���� ���� ����� �� �������, �� ���������� 0
---------------------------------------------------------------------------------------------*/
uint8_t* Get_item_capt(enum enm_parmnlev curr_menu, uint8_t item, enum enm_parmnlev *submenu, uint16_t *parnum)
{
  int16_t i;
  int16_t n;
  n = 0;
  // ������� �������
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].prevlev != curr_menu) continue;
    if (n == item)
    {
      *submenu = parmenu[i].currlev;
      return(uint8_t *)parmenu[i].shrtname;
    }
    n++;
  }
  // ������� ���� ��������� �������� � ����
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if (dwvar[i].parmnlev != curr_menu) continue;
    if ((dwvar[i].parmnlev & VAL_UNVISIBLE) != 0) continue;
    if (n == item)
    {
      *submenu = PARAMS_ROOT;
      *parnum  = i;
      return(uint8_t *)dwvar[i].abbreviation;
    }
    n++;
  }
  return (0);
}

/*-------------------------------------------------------------------------------------------
  �������� ������������� ������������� ���� �� �������������� �������
---------------------------------------------------------------------------------------------*/
enum enm_parmnlev Get_parent_menu(enum enm_parmnlev curr_menu)
{
  int16_t i;
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].currlev == curr_menu) return (parmenu[i].prevlev);
  }
  return (MAIN_PARAMS_ROOT);
}





/*-------------------------------------------------------------------------------------------
  ������� ���������� ��������� �� ������ ������ ����.
  � ���������� *sz ������������ ���������� ������� � ������� ����������� ������
---------------------------------------------------------------------------------------------*/
T_parmenu* Get_parmenu(int16_t *sz)
{
  *sz = PARMNU_ITEM_NUM;
  return(T_parmenu *)parmenu;

}


/*-------------------------------------------------------------------------------------------
  ������� ���������� ��������� �� ������ ����������� ������� ����������.
  � ���������� *sz ������������ ���������� ������� � �������
---------------------------------------------------------------------------------------------*/
T_work_params* Get_params(int16_t *sz)
{
  *sz = DWVAR_SIZE;
  return(T_work_params *)dwvar;
}

/*-------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------*/
void Execute_param_func(uint16_t indx)
{
  if (dwvar[indx].func != 0) dwvar[indx].func();
}



uint8_t ascii_to_hex(uint8_t c)
{
  if (c >= '0' && c <= '9')      return (c - '0') & 0x0f;
  else if (c >= 'a' && c <= 'f') return (c - 'a' + 10) & 0x0f;
  else                           return (c - 'A' + 10) & 0x0f;
}

uint8_t hex_to_ascii(uint8_t c)
{
  c = c & 0xf;
  if (c <= 9) return (c + 0x30);
  return (c + 'A' - 10);
}

/*-------------------------------------------------------------------------------------------------------------
  ������ ������ str ��������������� ����� ��� ����������� "\r\n" ��� "\n\r" �� ������ len �������� �� ������ buf.
  ���������� �������� ��������� �� ����� buf
 ������� \r � \n �� ������ ����������

  ���������� ERROR ���� � ������ ����� ��������� ����  0
-------------------------------------------------------------------------------------------------------------*/
int Read_cstring_from_buf(char **buf, char *str, uint32_t len)
{
  int32_t  pos;
  char    c;
  char    *ptr;

  ptr = *buf;
  pos = 0;
  do
  {
    c = *ptr;
    if (c == 0)
    {
      if (pos == 0)
      {
        *buf = ptr;
        return (-1);
      }
      if ((str[pos - 1] == '\r') || (str[pos - 1] == '\n'))
      {
        str[pos - 1] = 0;
      }
      *buf = ptr;
      return (len);
    }
    str[pos++] = c;
    ptr++;
    str[pos] = 0;
    if (pos > 1)
    {
      if (strcmp(&str[pos - 2], "\r\n") == 0)
      {
        str[pos - 2] = 0;
        *buf = ptr;
        return (pos - 2);
      }
      if (strcmp(&str[pos - 2], "\n\r") == 0)
      {
        str[pos - 2] = 0;
        ptr--;
        *buf = ptr;
        return (pos - 2);
      }
    }
    if (pos >= len)
    {
      *buf = ptr;
      return (len);
    }
  }
  while (1);
}

/*-------------------------------------------------------------------------------------------------------------
   ��������� �������� ������� � ����� � ������ ������� ���� ����
-------------------------------------------------------------------------------------------------------------*/
char* Trim_and_dequote_str(char *str)
{
  int i;

  while (*str == ' ')
  {
    str++;
    if (*str == 0) return str;
  }

  for (i = (strlen(str) - 1); i > 0; i--)
  {
    if (str[i] != ' ') break;
    str[i] = 0;
  }

  if ((str[0] == '"') && (str[strlen(str) - 1] == '"'))
  {
    str[strlen(str) - 1] = 0;
    str++;
  }
  return str;
}


/*-------------------------------------------------------------------------------------------
  ������������ �������� �����, MAC ����� � ������ ���������� �������� �� ����� ID.txt
---------------------------------------------------------------------------------------------*/
_mqx_int Restore_from_INIfile(void)
{
  MQX_FILE_PTR       f;
  _mqx_int           res;
  char              *str;
  char              *abbr;
  char              *val;
  int32_t            n;
  uint32_t           pcnt;



  str = _mem_alloc_zero(INI_STR_SIZE + 1);
  if (str == 0)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, "Memory allocation error.");
    return MQX_ERROR;
  }

  // ��������� ����
  f = _io_fopen(PARAMS_FILE_NAME, "r");
  if (f == NULL)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, "File %s opening error. Task stopped.", PARAMS_FILE_NAME);
    _mem_free(str);
    return MQX_ERROR;
  }

  pcnt = 0;
  do
  {
    // ������ ������ �� �����
    res = _io_fscanf(f, "%255s\n", str);
    if (res == IO_EOF)
    {
      if (_io_feof(f) != 0) break;
    }
    if (res == 1)
    {
      if (str[0] == 0)  continue;
      if (str[0] == ';') continue;

      // ���� ��������� �� ������ =
      val = strchr(str, '=');
      if (val == NULL) continue; // ���� ������� �� ������� �� ��� �� ������ ���������
      *val = 0; // ��������� 0 �� ������ ������ = ����� �������� ������� ����� � ��������
      val++; // ��������� �� ������ ������ ������ ��������
      abbr = str;
      // ����� �������� �� ������������
      abbr = Trim_and_dequote_str(abbr);
      n = Find_param_by_abbrev(abbr);
      if (n >= 0)
      {
        val = Trim_and_dequote_str(val);
        Str_to_param((uint8_t *)val, n);
        pcnt++;
      }
    }
  }
  while (1);


  _io_fclose(f);
  _mem_free(str);

  LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, "Restored %d paramters from file %s.", pcnt, PARAMS_FILE_NAME);
  return MQX_OK;
}

/*-------------------------------------------------------------------------------------------
   ��������� ���������� � ini-���� ����������
---------------------------------------------------------------------------------------------*/
_mqx_int Save_Params_to_INIfile(void)
{
  MQX_FILE_PTR       f;
  _mqx_int           res;
  uint32_t           i,n;
  char               *str;
  char               *inbuf;
  uint32_t           offs;
  char               *tmp_str;
  char               *name;
  char               *prev_name;


  str = _mem_alloc_zero(INI_STR_SIZE + 1);
  if (str == 0)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, "Memory allocation error.");
    return MQX_ERROR;
  }
  inbuf = _mem_alloc_zero(FILE_BUF_SZ + 1);
  if (inbuf == 0)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, "Memory allocation error.");
    return MQX_ERROR;
  }
  // ��������� ����
  f = _io_fopen(PARAMS_FILE_NAME, "w+");
  if (f == NULL)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, "File %s opening error. Task stopped.", PARAMS_FILE_NAME);
    _mem_free(str);
    return MQX_ERROR;
  }

  n =0;
  name      = 0;
  prev_name = 0;
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if ((dwvar[i].attr & 1) == 0) // ��������� ������ ���� �������� ��� ������
    {
      offs = 0;

      // �������� ������ ���� ��������
      name = (char *)Get_mn_name((enum enm_parmnlev )dwvar[i].parmnlev);
      if (name != prev_name)
      {
        sprintf(inbuf + offs, ";--------------------------------------------------------\r\n");
        offs = offs + strlen(inbuf + offs);
        sprintf(inbuf + offs, "; %s\r\n", name);
        offs = offs + strlen(inbuf + offs);
        sprintf(inbuf + offs, ";--------------------------------------------------------\r\n\r\n");
        offs = offs + strlen(inbuf + offs);
        prev_name = name;
      }
      sprintf(inbuf + offs, "; N=%03d %s\r\n%s=", n++, dwvar[i].name, dwvar[i].abbreviation);
      offs = offs + strlen(inbuf + offs);
      if (dwvar[i].vartype == tstring)
      {
        Param_to_str((uint8_t *)(str + 1), MAX_PARAMETER_STRING_LEN, i);
        *str = '"';
        tmp_str = str + strlen(str);
        *tmp_str = '"';
        tmp_str++;
      }
      else
      {
        Param_to_str((uint8_t *)str, MAX_PARAMETER_STRING_LEN, i);
        tmp_str = str + strlen(str);
      }
      *tmp_str = '\r';
      tmp_str++;
      *tmp_str = '\n';
      tmp_str++;
      *tmp_str = '\r';
      tmp_str++;
      *tmp_str = '\n';
      tmp_str++;
      *tmp_str = '\0';
      sprintf(inbuf + offs, "%s", str);
      offs = offs + strlen(inbuf + offs);

      res = _io_write(f, inbuf, offs);
      if (res != offs)
      {
        LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, "File %s write error %d.", PARAMS_FILE_NAME, res);
        _io_fclose(f);
        _mem_free(str);
        _mem_free(inbuf);
        return MQX_ERROR;
      }
    }
  }
  _io_fclose(f);
  _mem_free(str);
  _mem_free(inbuf);
  LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, "Parameters to file %s saved Ok.", PARAMS_FILE_NAME);
  return MQX_OK;
}

