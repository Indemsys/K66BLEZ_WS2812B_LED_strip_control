// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.29
// 13:50:42
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

extern const T_SPI_modules spi_mods[3];
extern T_SPI_cbls  spi_cbl[3];

#define  MKW40_RXEND   BIT(1)  // ���� ������������ �� ISR �� ��������� ������ �� SPI

static T_DMA_SPI_cbl DS_cbl;
/*------------------------------------------------------------------------------
  ���������� ���������� �� ������ DMA �� ���������� ������ ������ �� SPI ���������� � MKW40

 \param user_isr_ptr
 ------------------------------------------------------------------------------*/
static void DMA_SPI_MKW40_rx_isr(void *user_isr_ptr)
{
  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;

  DMA->INT = BIT(DMA_MKW40_FM_CH); // ���������� ���� ����������  ������

  // ������� FIFO ��������� � �����������
  spi_mods[MKW40_SPI].spi->MCR  |= BIT(CLR_RXF) + BIT(CLR_TXF);
  // ������� ��� ����� � SPI
  spi_mods[MKW40_SPI].spi->SR =  spi_mods[MKW40_SPI].spi->SR;

  _lwevent_set(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND); // ���������� �� ��������� ��������� �� DMA �� SPI � ������
}

/*------------------------------------------------------------------------------
  ���������������� 2-� ������� DMA �� ����� � �� �������� ��� ������ � ������� SPI


 ------------------------------------------------------------------------------*/
void Init_MKW40_SPI_DMA(void)
{
  T_DMA_SPI_RX_config   rx_cfg;
  T_DMA_SPI_TX_config   tx_cfg;

  tx_cfg.ch       = DMA_MKW40_MF_CH;                           // ����� ������ DMA
  tx_cfg.spi_pushr = (uint32_t)&(spi_mods[MKW40_SPI].spi->PUSHR); // ����� �������� PUSHR SPI
  tx_cfg.DMAMUX   = DMA_MKW40_DMUX_PTR;                        // ��������� �� ������������� ������� �������� ��� DMA
  tx_cfg.dmux_src = DMA_MKW40_DMUX_TX_SRC;                     // ����� ����� ��������� ��� ���������� �������������� DMAMUX ��� �������� �� DMA.

  Config_DMA_for_SPI_TX(&tx_cfg, &DS_cbl);


  rx_cfg.ch        = DMA_MKW40_FM_CH;                          // ����� ������ DMA
  rx_cfg.spi_popr  = (uint32_t)&(spi_mods[MKW40_SPI].spi->POPR); // ����� �������� POPR SPI
  rx_cfg.DMAMUX    = DMA_MKW40_DMUX_PTR;                       // ����� ����� ��������� ��� ���������� �������������� DMAMUX ��� �������� �� DMA.
  rx_cfg.dmux_src  = DMA_MKW40_DMUX_RX_SRC;

  Config_DMA_for_SPI_RX(&rx_cfg, &DS_cbl);
  Install_and_enable_isr(DMA_MKW40_RX_INT_NUM, spi_mods[MKW40_SPI].prio, DMA_SPI_MKW40_rx_isr); // ���������� �� ���������� ������ �� DMA
}



/*-------------------------------------------------------------------------------------------------------------
  �������� ������ �� SPI � �������������� DMA
-------------------------------------------------------------------------------------------------------------*/
_mqx_uint MKW40_SPI_send_buf(const uint8_t *buff, uint32_t sz)
{
  _mqx_uint    res = MQX_OK;
  uint32_t       s;
  int          i;

  Set_MKW40_CS_state(0);
  while (sz > 0)
  {
    if (sz >= MAX_DMA_SPI_BUFF) s = MAX_DMA_SPI_BUFF - 1;
    else s = sz;

    Start_DMA_for_SPI_TX(&DS_cbl, buff, s);
    // ������� ����� ��������� �������� ������ �� DMA
    if (_lwevent_wait_ticks(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND, FALSE, 10) != MQX_OK)
    {
      spi_cbl[MKW40_SPI].tx_err_cnt++;
      res = MQX_ERROR;
    }
    buff = buff + s;
    sz -= s;
  }
  Set_MKW40_CS_state(1);
  return res;
}

/*-------------------------------------------------------------------------------------------------------------
  ����� ������ �� SPI � �������������� DMA
-------------------------------------------------------------------------------------------------------------*/
_mqx_uint MKW40_SPI_read_buf(const uint8_t *buff, uint32_t sz)
{
  _mqx_uint   res = MQX_OK;
  uint32_t      s;
  int         i;

  Set_MKW40_CS_state(0);
  while (sz > 0)
  {
    if (sz >= MAX_DMA_SPI_BUFF) s = MAX_DMA_SPI_BUFF - 1;
    else s = sz;

    Start_DMA_for_SPI_RX(&DS_cbl, buff, s);
    // ������� ����� ��������� �������� �������
    if (_lwevent_wait_ticks(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND, FALSE, 10) != MQX_OK)
    {
      spi_cbl[MKW40_SPI].rx_err_cnt++;
      res = MQX_ERROR;
    }
    buff = buff + s;
    sz -= s;
  }
  Set_MKW40_CS_state(1);
  return res;
}

/*-------------------------------------------------------------------------------------------------------------
  ����� ������ �� SPI � �������������� DMA
-------------------------------------------------------------------------------------------------------------*/
_mqx_uint MKW40_SPI_write_read_buf(const uint8_t *wbuff, uint32_t wsz, const uint8_t *rbuff, uint32_t rsz)
{
  _mqx_uint   res = MQX_OK;
  uint32_t      s;
  int         i;

  Set_MKW40_CS_state(0);
  while (wsz > 0)
  {
    if (wsz > MAX_DMA_SPI_BUFF) s = MAX_DMA_SPI_BUFF;
    else s = wsz;

    Start_DMA_for_SPI_TX(&DS_cbl, wbuff, s);
    // ������� ����� ��������� �������� ������ �� DMA
    if (_lwevent_wait_ticks(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND, FALSE, 10) != MQX_OK)
    {
      spi_cbl[MKW40_SPI].tx_err_cnt++;
      res = MQX_ERROR;
    }
    wbuff = wbuff + s;
    wsz -= s;
  }

  while (rsz > 0)
  {
    if (rsz > MAX_DMA_SPI_BUFF) s = MAX_DMA_SPI_BUFF;
    else s = rsz;

    Start_DMA_for_SPI_RX(&DS_cbl, rbuff, s);
    // ������� ����� ��������� �������� �������
    if (_lwevent_wait_ticks(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND, FALSE, 10) != MQX_OK)
    {
      spi_cbl[MKW40_SPI].rx_err_cnt++;
      res = MQX_ERROR;
    }
    rbuff = rbuff + s;
    rsz -= s;
  }
  Set_MKW40_CS_state(1);
  return res;
}


/*-------------------------------------------------------------------------------------------------------------
  ����� ������ �� SPI � �������������� DMA
  ������� ���������� ������� �����
  ������ ������ ����� ���� �� ����� MAX_DMA_SPI_BUFF
-------------------------------------------------------------------------------------------------------------*/
_mqx_uint MKW40_SPI_slave_read_buf(const uint8_t *buff, uint32_t sz)
{
  _mqx_uint   res = MQX_OK;
  if (sz >= MAX_DMA_SPI_BUFF) return MQX_ERROR;

  Start_DMA_for_SPI_RX(&DS_cbl, buff, sz);

  // ������� ����� ��������� �������� �������
  if (_lwevent_wait_ticks(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND, FALSE, 10) != MQX_OK)
  {
    spi_cbl[MKW40_SPI].rx_err_cnt++;
    res = MQX_ERROR;
  }
  return res;
}


/*-------------------------------------------------------------------------------------------------------------
  ����� � �������� ������ �� SPI � �������������� DMA
  ������ ������ ����� ���� �� ����� MAX_DMA_SPI_BUFF
-------------------------------------------------------------------------------------------------------------*/
_mqx_uint MKW40_SPI_slave_read_write_buf(const uint8_t *outbuff, uint8_t *inbuff, uint32_t sz)
{
  _mqx_uint   res = MQX_OK;
  if (sz >= MAX_DMA_SPI_BUFF) return MQX_ERROR;

  SPI_clear_FIFO(MKW40_SPI);
  Start_DMA_for_SPI_RXTX(&DS_cbl, outbuff, inbuff, sz);

  // ������� ����� ��������� �������� �������
  if (_lwevent_wait_ticks(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND, FALSE, 10) != MQX_OK)
  {
    spi_cbl[MKW40_SPI].rx_err_cnt++;
    res = MQX_ERROR;
  }
  return res;
}


/*------------------------------------------------------------------------------
  ������������� SPI ������ � 2-� ������� DMA ��� ����

 ------------------------------------------------------------------------------*/
void Init_MKW40_channel(void)
{
  //SPI_master_init(MKW40_SPI, SPI_8_BITS, 0, 0, SPI_BAUD_20MHZ, 0);
  SPI_slave_init(MKW40_SPI, SPI_8_BITS, 0, 0, 0);
  Init_MKW40_SPI_DMA(); // ���������������� 2-� ������� DMA �� ����� � �� �������� ��� ������ � ������� SPI

}

