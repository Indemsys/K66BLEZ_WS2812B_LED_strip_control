// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-07
// 15:58:28
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

//#define RESET_BITS_CNT 50
#define   COLRS          3
#define   LEDS_NUM       122//78
#define   WS2812B_BITS_NUM (8*COLRS*LEDS_NUM)

#define   MAX_PTTRN_LEN 64 // ������������ ����� ������� ��� ������ ��������� ����������


uint32_t  enable_led_strip;

static   T_DMA_WS2812B_config  ws2812B_DMA_cfg;

// ��������� ������� �������� ������������� ������ ��� ��� ��������� � ������� DMA
typedef struct
{
  uint16_t  buf[LEDS_NUM][COLRS][8];
  uint16_t  bend;
} T_WS2812B_bits;

// ����������� ��������� ������ ���������  ��� ������� ���������� WS2812B
typedef struct
{
  uint32_t  cnt;         // ������� �������� ���������
  uint32_t  *chain_ptr;  // ��������� �� ������ ������� ����������� �����
  uint32_t  *curr_ptr;   // ������� ������� � ������� ����������� �����
  uint32_t  code;
  uint32_t  data;
  uint32_t  prev_hsv;
  uint32_t  hsv;
  uint32_t  duration;
  uint32_t  jmp_done;    // ���� ������������ �������� � ����������� �������
} T_WS2812B_sm_cbl;


// ��������� ������ � ����������� ������� ������ ������ ����������
#define  B_JMP   BIT(31)
#define  B_STOP  BIT(30)
#define  B_RAMP  BIT(29)

typedef struct
{
  uint32_t  code;
  // �������� ������� ���� code
  // ����� ����     ��������
  //  31            ���� ��������, ��� ������� ����� ����� ���� data �������� ����� ��������� �� ������� ������� ������� �������� ���������
  //  30            ���� ���������. ������� ����� ����� ��������� ���������� ���������� �������� ���������
  //  29            ���� �������� ��������. ��������� ����� ������� ���������� �� ������� � �������� � ������� ��������� ��������� � ���� data
  //  24..0         ��� ����� � �������   [hue] - 0..360 (9 bit), [saturation] - 0..255 (8 bit),  [value] - 0..255 (8-bit)

  uint32_t  data;  // �������� ������� ��������� ������� ��������� ��� ����� �������� � ���������� �� ����������� ������

} T_pattrn_item;

#pragma data_alignment= 64
static T_WS2812B_bits WS2812B_bits; // ������ ������������� ������ ��� ��� ��������� � ������� DMA

static T_WS2812B_sm_cbl lcbl[LEDS_NUM]; // ��������� ����� ��������� �����������
static uint32_t g_atten; // ���������� �������

static uint32_t ptrns_arr[LEDS_NUM][MAX_PTTRN_LEN];  // ������ �������� ��������

// ������� ��������������� ��� ��������������� HSV -> RGB
const uint8_t         dim_curve[256] = {
  0, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6,
  6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8,
  8, 8, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11,
  11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15,
  15, 15, 16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20,
  20, 20, 21, 21, 22, 22, 22, 23, 23, 24, 24, 25, 25, 25, 26, 26,
  27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 35, 35,
  36, 36, 37, 38, 38, 39, 40, 40, 41, 42, 43, 43, 44, 45, 46, 47,
  48, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
  63, 64, 65, 66, 68, 69, 70, 71, 73, 74, 75, 76, 78, 79, 81, 82,
  83, 85, 86, 88, 90, 91, 93, 94, 96, 98, 99, 101, 103, 105, 107, 109,
  110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
  146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
  193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,
};

static void  WS2812B_state_automat(void);
/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void WS2812B_init_bits(void)
{
  uint32_t i, j, k;

  for (i = 0; i < LEDS_NUM; i++)
  {
    for (j = 0; j < COLRS; j++)
    {
      for (k = 0; k < 8; k++)
      {
        WS2812B_bits.buf[i][j][k] = FTM_WS2812B_0;
      }
    }
  }
  WS2812B_bits.bend = 0;
}


/*-----------------------------------------------------------------------------------------------------
 
 \param cfg 
-----------------------------------------------------------------------------------------------------*/
static void WS2812B_init_DMA_TCD(T_DMA_WS2812B_config *cfg)
{
  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;

  // �������������� ����� DMA �� ����������� ����������� �������� �� RAM � ������ ����������� ������������������ 0 � 1 � ���� ���������� PWM

  DMA->TCD[cfg->dma_ch].SADDR = cfg->saddr;                   // �������� - ����� � �������
  DMA->TCD[cfg->dma_ch].SOFF = 2;                             // ����� ��������� ������� �� 2 ����� ����� ������ ��������
                                                              // DMA->TCD[cfg->dma_ch].SLAST = (uint32_t)(-cfg->arrsz * 2);    // ������������ �� ��������� ����� ��������� ����� ���������� ����� ����� DMA (��������� ��������� �����)
  DMA->TCD[cfg->dma_ch].SLAST = 0;
  DMA->TCD[cfg->dma_ch].DADDR = cfg->daddr;                   // ����� ��������� - ������� PUSHR SPI
  DMA->TCD[cfg->dma_ch].DOFF = 0;                             // �����  ������ ��������� ��������� �� �������
  DMA->TCD[cfg->dma_ch].DLAST_SGA = 0;                        // ������� ������������ �� ���������
  DMA->TCD[cfg->dma_ch].NBYTES_MLNO = 2;                      // ���������� ���� ������������ �� ���� ������ DMA (� �������� �����)

  DMA->TCD[cfg->dma_ch].BITER_ELINKNO = 0 //TCD Beginning Minor Loop Link
    + LSHIFT(0, 15)                  // ELINK  | �������� �� ���������
    + LSHIFT(cfg->arrsz, 0)          // BITER  | Starting Major Iteration Count (15 bit) ���������� � CITER �� ��������� �������� ��������� �����
  ;
  DMA->TCD[cfg->dma_ch].CITER_ELINKNO = 0 //TCD Current Minor Loop Link
    + LSHIFT(0, 15)                  // ELINK  | �������� �� ���������
    + LSHIFT(cfg->arrsz, 0)          // CITER  | Current Major Iteration Count (15 bit)
  ;
  DMA->TCD[cfg->dma_ch].ATTR = 0
    + LSHIFT(0, 11) // SMOD  | ������ ������ ��������� �� ����������
    + LSHIFT(1, 8)  // SSIZE | 16-� ������ ��������� �� ���������
    + LSHIFT(0, 3)  // DMOD  | ������ ������ ���������
    + LSHIFT(1, 0)  // DSIZE | 16-� ������ ��������� � ��������
  ;
  DMA->TCD[cfg->dma_ch].CSR = 0
    + LSHIFT(0, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
    + LSHIFT(0, 8)  // MAJORLINKCH | ����� ��������������� ������. ������� ������������ ����� ��� � ���� �� ��������
    + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
    + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
    + LSHIFT(0, 5)  // MAJORELINK  | �������� �� ���������
    + LSHIFT(0, 4)  // ESG         | ������� ������������ �� ���������
    + LSHIFT(1, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
    + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
    + LSHIFT(0, 1)  // INTMAJOR    | ������������ ���������� �� �������� ��������� DMA
    + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
  ;
  DMA->SERQ = cfg->dma_ch; // ��������� ������ ������ DMA
  DMA->SSRT = cfg->dma_ch; // �������� ����� �� DMA, ��������� ��� ������ �� �������� ����� ������� ���� ��� ���������� �������� 0
}

/*-----------------------------------------------------------------------------------------------------
  ������������� ������ ������ � ������� DMA � WS2812B
 
----------------------------------------------------------------------------------------------------*/
static void WS2812B_init_DMA_stream(T_DMA_WS2812B_config *cfg)
{
  if (cfg->ftm_ch > 7) return;

  cfg->DMAMUX->CHCFG[cfg->dma_ch] = cfg->dmux_src + BIT(7); // ����� ������������� ��������� ������ �� ������� ��������� (����� �� ������ SPI) � ������ ���������� ������ DMA

  WS2812B_init_DMA_TCD(cfg);

  cfg->FTM->OUTMASK &= ~LSHIFT(1, cfg->ftm_ch);             // ����������� ������ ������
  cfg->FTM->CONTROLS[cfg->ftm_ch].CnSC |= 0
    + LSHIFT(1, 6) // CHIE.
    + LSHIFT(1, 0) // DMA.  1 Enable DMA transfers.
  ;
}


/*-----------------------------------------------------------------------------------------------------
 
 \param tid 
 \param data_ptr 
 \param secs 
 \param msecs 
-----------------------------------------------------------------------------------------------------*/
static void  WS2812B_refresh(_timer_id tid, void *data_ptr, uint32_t secs, uint32_t msecs)
{
  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;
  DMA->INT = BIT(DMA_WS2812B_CH); // ���������� ���� ����������  ������

  WS2812B_init_DMA_TCD(&ws2812B_DMA_cfg);
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void WS2812B_periodic_refresh(void)
{
  int32_t           n;
  static uint32_t   color1;
  static uint32_t   color2;
  static uint32_t   color3;


  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;
  DMA->INT = BIT(DMA_WS2812B_CH); // ���������� ���� ����������  ������

  if (enable_led_strip==1)
  {

    WS2812B_init_DMA_TCD(&ws2812B_DMA_cfg);

    WS2812B_state_automat();

    // ������ ����� ������� ����� ���� ����
    for (n = 0; n < LEDS_NUM; n++)
    {
      if (lcbl[n].jmp_done != 0)
      {
        if (n == 0)
        {
          // ���� ������� �� ���������� 0, �� ������ �����
          // ��� ����� � �������� � �������   [hue] - 0..360 (9 bit), [saturation] - 0..255 (8 bit),  [value] - 0..255 (8-bit)
          color1 = (rand() & 0x1FF0000) + 0xFFFF;
          color2 = (rand() & 0x1FF0000) + 0xFFFF;
          color3 = (rand() & 0x1FF0000) + 0xFFFF;
        }

        // ����� ���������� ����������� ��������� �������
        ptrns_arr[n][2] = color1 + B_RAMP;
        ptrns_arr[n][3] = 400;
        ptrns_arr[n][4] = HSV_NONE + B_RAMP;
        ptrns_arr[n][5] = 400;
        ptrns_arr[n][6] = color2 + B_RAMP;
        ptrns_arr[n][7] = 400;
        ptrns_arr[n][8] = HSV_NONE + B_RAMP;
        ptrns_arr[n][9] = 400;
        ptrns_arr[n][10] = color3 + B_RAMP;
        ptrns_arr[n][11] = 400;
        ptrns_arr[n][12] = HSV_NONE + B_RAMP;
        ptrns_arr[n][13] = 400;

        lcbl[n].jmp_done = 0;
      }
    }
  }

}

/*------------------------------------------------------------------------------
  ���������� �� HSV � RGB � ������������� ����������
 
  ���� - HSV, ��� ����� � �������   [hue] - 0..360 (9 bit), [saturation] - 0..255 (8 bit),  [value] - 0..255 (8-bit)
 ------------------------------------------------------------------------------*/
uint32_t Convert_H_S_V_to_RGB(uint32_t hue, uint32_t sat, uint32_t val)
{
  uint32_t   r;
  uint32_t   g;
  uint32_t   b;
  uint32_t   base;
  uint32_t   rgb;

  val = dim_curve[val];
  sat = 255 - dim_curve[255 - sat];

  if (sat == 0) // Acromatic color (gray). Hue doesn't mind.
  {
    rgb = val | (val << 8) | (val << 16);
  }
  else
  {
    base = ((255 - sat) * val) >> 8;
    switch (hue / 60)
    {
    case 0:
      r = val;
      g = (((val - base) * hue) / 60) + base;
      b = base;
      break;
    case 1:
      r = (((val - base) * (60 - (hue % 60))) / 60) + base;
      g = val;
      b = base;
      break;
    case 2:
      r = base;
      g = val;
      b = (((val - base) * (hue % 60)) / 60) + base;
      break;
    case 3:
      r = base;
      g = (((val - base) * (60 - (hue % 60))) / 60) + base;
      b = val;
      break;
    case 4:
      r = (((val - base) * (hue % 60)) / 60) + base;
      g = base;
      b = val;
      break;
    case 5:
      r = val;
      g = base;
      b = (((val - base) * (60 - (hue % 60))) / 60) + base;
      break;
    }
    rgb = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
  }
  return rgb;

}


/*------------------------------------------------------------------------------
  ���������� �� HSV � RGB � ������������� ����������
 
  ���� - HSV, ��� ����� � �������   [hue] - 0..360 (9 bit), [saturation] - 0..255 (8 bit),  [value] - 0..255 (8-bit)
 ------------------------------------------------------------------------------*/
uint32_t Convert_HSV_to_RGB(uint32_t hsv)
{
  uint32_t   hue;
  uint32_t   sat;
  uint32_t   val;

  hue = (hsv >> 16) & 0x1FF;
  sat = (hsv >> 8) & 0xFF;
  val = (hsv >> 0) & 0xFF;
  return Convert_H_S_V_to_RGB(hue, sat, val);
}

/*------------------------------------------------------------------------------
   ������������� ���� ����������
   color - ���� � ������� HSV
 ------------------------------------------------------------------------------*/
void WS2812B_set_led_state(uint32_t ledn, uint32_t hue, uint32_t sat, uint32_t val, uint32_t attn)
{
  uint32_t i;
  uint8_t  c_red;
  uint8_t  c_green;
  uint8_t  c_blue;
  uint32_t color;

  color = Convert_H_S_V_to_RGB(hue, sat, val);
  c_red   = ((color >> 16) & 0xFF) >> attn;
  c_green = ((color >> 8) & 0xFF) >> attn;
  c_blue  = ((color >> 0) & 0xFF) >> attn;

  for (i = 0; i < 8; i++)
  {

    // �������
    if ((c_green >> (7 - i)) & 1) WS2812B_bits.buf[ledn][0][i] = FTM_WS2812B_1;
    else WS2812B_bits.buf[ledn][0][i] = FTM_WS2812B_0;
    // �������
    if ((c_red >> (7 - i)) & 1) WS2812B_bits.buf[ledn][1][i] = FTM_WS2812B_1;
    else WS2812B_bits.buf[ledn][1][i] = FTM_WS2812B_0;
    // �����
    if ((c_blue >> (7 - i)) & 1) WS2812B_bits.buf[ledn][2][i] = FTM_WS2812B_1;
    else WS2812B_bits.buf[ledn][2][i] = FTM_WS2812B_0;
  }
}


/*-------------------------------------------------------------------------------------------------------------
  ������������� ������� ��� ������ ��������� ������� �� ���������

  ������ ������� �� ������� ���� ����.
  ������ ����� � ������ - ��� ����� 
  ������ ����� � ������ - ������������ ��������� ������� � �������� ��
    �������� ������ 0x00000000 - �������� ������� � ������ �������
    �������� ������ 0xFFFFFFFF - �������� ���������� ���������

  n - ������ ���������� 0..(LEDS_CNT - 1)
-------------------------------------------------------------------------------------------------------------*/
void WS2812B_Set_pattern(uint32_t *pattern, uint32_t n)
{

  if (n >= LEDS_NUM) return;

  _int_disable();
  if ((pattern != 0) && (lcbl[n].chain_ptr != pattern))
  {
    lcbl[n].chain_ptr = pattern;
    lcbl[n].curr_ptr = pattern;
    lcbl[n].prev_hsv = HSV_NONE;
    lcbl[n].hsv      = HSV_NONE;
    lcbl[n].cnt      = 0;
  }
  _int_enable();
}


/*------------------------------------------------------------------------------
   ������� ��������� �����������
   ���������� ������ ��� 
 ------------------------------------------------------------------------------*/
static void  WS2812B_state_automat(void)
{
  uint32_t       n;
  int32_t        res = -1;

  for (n = 0; n < LEDS_NUM; n++)
  {
    if (lcbl[n].chain_ptr != 0) // ��������� ������� ��������� �������
    {
      if (lcbl[n].cnt == 0)  // ��������� � ������ ������������ �������� �������
      {
        lcbl[n].code = *lcbl[n].curr_ptr;   // ������� �������� �����
        lcbl[n].curr_ptr++;
        lcbl[n].data = *lcbl[n].curr_ptr;   // ������� ������
        lcbl[n].curr_ptr++;                 // �������� ��������� �� ��������� ����������� �������


        if (lcbl[n].code & B_STOP)
        {
          // ���������� ������ �������� ���������
          lcbl[n].cnt = 0;
          lcbl[n].chain_ptr = 0;
        }
        else if (lcbl[n].code & B_JMP)
        {
          // ������� �� ������� �� ���������
          lcbl[n].curr_ptr = (uint32_t *)lcbl[n].data;
          lcbl[n].jmp_done = 1;
        }
        else
        {
          lcbl[n].duration = Conv_ms_to_ticks(lcbl[n].data);
          lcbl[n].cnt = lcbl[n].duration;
          lcbl[n].prev_hsv = lcbl[n].hsv;
          lcbl[n].hsv = lcbl[n].code;

          if ((lcbl[n].code & B_RAMP) == 0)
          {
            uint32_t   hue;
            uint32_t   sat;
            uint32_t   val;

            hue = (lcbl[n].hsv >> 16) & 0x1FF;
            sat = (lcbl[n].hsv >> 8) & 0xFF;
            val = (lcbl[n].hsv >> 0) & 0xFF;

            // ���� ��� �����, �� ����� ������������� �������� ����
            WS2812B_set_led_state(n, hue, sat, val, g_atten);
          }
        }
      }
      else
      {
        lcbl[n].cnt--;
        if (lcbl[n].code & B_RAMP)
        {
          // ������ ������ �������� ����� � ������ �����
          uint32_t   prev_hue;
          uint32_t   prev_sat;
          uint32_t   prev_val;

          uint32_t   hue;
          uint32_t   sat;
          uint32_t   val;

          uint32_t   delta;

          hue = (lcbl[n].hsv >> 16) & 0x1FF;
          sat = (lcbl[n].hsv >> 8) & 0xFF;
          val = (lcbl[n].hsv >> 0) & 0xFF;

          prev_hue = (lcbl[n].prev_hsv >> 16) & 0x1FF;
          prev_sat = (lcbl[n].prev_hsv >> 8) & 0xFF;
          prev_val = (lcbl[n].prev_hsv >> 0) & 0xFF;

          if (hue > prev_hue)
          {
            delta = ((hue - prev_hue) * lcbl[n].cnt) / lcbl[n].duration;
            hue = hue - delta;
          }
          else
          {
            delta = ((prev_hue - hue) * lcbl[n].cnt) / lcbl[n].duration;
            hue = hue + delta;
          }
          if (sat > prev_sat)
          {
            delta = ((sat - prev_sat) * lcbl[n].cnt) / lcbl[n].duration;
            sat = sat - delta;
          }
          else
          {
            delta = ((prev_sat - sat) * lcbl[n].cnt) / lcbl[n].duration;
            sat = sat + delta;
          }
          if (val > prev_val)
          {
            delta = ((val - prev_val) * lcbl[n].cnt) / lcbl[n].duration;
            val = val - delta;
          }
          else
          {
            delta = ((prev_val - val) * lcbl[n].cnt) / lcbl[n].duration;
            val = val + delta;
          }
          WS2812B_set_led_state(n, hue, sat, val, g_atten);
        }
      }
    }
    else
    {
      // ���� ��� �������, �� ��������� ���������
      WS2812B_set_led_state(n, 0, 0, 0, g_atten);
    }
  }
}



/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void WS2812B_Demo_DMA(void)
{
  uint32_t i;

  // ������ �� �������� ���� �������. ������� �� ����
  //refr_tmr_id = _timer_start_periodic_every(WS2812B_refresh, 0, TIMER_KERNEL_TIME_MODE, 10);

  WS2812B_init_bits();

  ws2812B_DMA_cfg.FTM      = FTM0_BASE_PTR;
  ws2812B_DMA_cfg.ftm_ch   = FTM_CH_2;
  ws2812B_DMA_cfg.dma_ch   = DMA_WS2812B_CH;
  ws2812B_DMA_cfg.saddr    = (uint32_t)&WS2812B_bits.buf;
  ws2812B_DMA_cfg.arrsz    = WS2812B_BITS_NUM + 1;
  ws2812B_DMA_cfg.daddr    = (uint32_t)&ws2812B_DMA_cfg.FTM->CONTROLS[FTM_CH_2].CnV;
  ws2812B_DMA_cfg.DMAMUX   = DMA_WS2812B_DMUX_PTR;
  ws2812B_DMA_cfg.dmux_src = DMA_WS2812B_DMUX_SRC;

  WS2812B_init_DMA_stream(&ws2812B_DMA_cfg);

  // ��������� ������� ������� ������������ ����
  for (i = 0; i < LEDS_NUM; i++)
  {
    ptrns_arr[i][0] = HSV_NONE + B_RAMP;
    ptrns_arr[i][1] = 40 * i;

    ptrns_arr[i][2] = HSV_GREEN + B_RAMP;
    ptrns_arr[i][3] = 400;
    ptrns_arr[i][4] = HSV_NONE + B_RAMP;
    ptrns_arr[i][5] = 400;
    ptrns_arr[i][6] = HSV_BLUE + B_RAMP;
    ptrns_arr[i][7] = 400;
    ptrns_arr[i][8] = HSV_NONE + B_RAMP;
    ptrns_arr[i][9] = 400;
    ptrns_arr[i][10] = HSV_RED + B_RAMP;
    ptrns_arr[i][11] = 400;
    ptrns_arr[i][12] = HSV_NONE + B_RAMP;
    ptrns_arr[i][13] = 400;
    ptrns_arr[i][14] = B_JMP;
    ptrns_arr[i][15] = (uint32_t)&ptrns_arr[i][2];
  }
  for (i = 0; i < LEDS_NUM; i++)
  {
    WS2812B_Set_pattern(&ptrns_arr[i][0], i);
  }
}

