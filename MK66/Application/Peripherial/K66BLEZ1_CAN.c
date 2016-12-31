#include "App.h"


volatile CAN_MemMapPtr           CAN_PTR;
LWEVENT_STRUCT                   can_tx_event;
LWEVENT_STRUCT                   can_rx_event;
uint32_t                         can_events_mask;
T_can_statistic                  can_stat;
_queue_id                        can_tx_queue_id;
_pool_id                         can_msg_pool_id;

#ifdef ENABLE_CAN_LOG
uint32_t                         canrx_log_head;
uint32_t                         canrx_log_tail;
T_can_msg                         canrx_log[CAN_RX_LOG_SZ];
LWSEM_STRUCT                     can_log_sem;
#endif

const T_can_rx_config         rx_mboxes[RX_MBOX_CNT] =
{
  { CAN_RX_MB1, 0xFFFFFFFF, 0x00000000 }, // ��������� ��� ������
  { CAN_RX_MB2, 0xFFFFFFFF, 0x00000000 }, // ��������� ��� ������
  { CAN_RX_MB3, 0xFFFFFFFF, 0x00000000 }, // ��������� ��� ������
  { CAN_RX_MB4, 0xFFFFFFFF, 0x00000000 }, // ��������� ��� ������
  { CAN_RX_MB5, 0xFFFFFFFF, 0x00000000 }, // ��������� ��� ������
  { CAN_RX_MB6, 0xFFFFFFFF, 0x00000000 }, // ��������� ��� ������
  { CAN_RX_MB7, 0xFFFFFFFF, 0x00000000 }, // ��������� ��� ������
  { CAN_RX_MB8, 0xFFFFFFFF, 0x00000000 }, // ��������� ��� ������
};



/*-------------------------------------------------------------------------------------------------------------
  ������������ ���������� CAN ���� ��� �������� � ��������� ������
-------------------------------------------------------------------------------------------------------------*/
void CAN_isr(void *ptr)
{
  volatile CAN_MemMapPtr CAN;
  uint32_t           reg;

  CAN = (CAN_MemMapPtr)ptr;

  reg = CAN->IFLAG1; // �������� ����� ���������� (����� � ������ 16-�� ������� ������).
                     // ���� �� �������� FIFO �� ������ ���� �������� ���������� �� ���������������� ������.
                     // ���������� ����� ������� 1
                     // ������ ���� ����� ��������, ��� ���������� �� ������ ��� � �� ��������.
  if (reg & 0x0FF)
  {
    _lwevent_set(&can_rx_event, reg & 0x0FF); // ������� ��� ������� ����� �������
  }

  if (reg & BIT(CAN_TX_MB1))
  {
    _lwevent_set(&can_tx_event, BIT(CAN_TX_MB1)); // ������� ��� ������� ����� �������
  }
  CAN->IFLAG1 = reg;                              // ���������� �����, �� ������ �� � ������� �� ��������
}
/*-------------------------------------------------------------------------------------------------------------
  ������������ ���������� CAN ���� � ���������� ������ � ������ �������
-------------------------------------------------------------------------------------------------------------*/
void CAN_err_isr(void *ptr)
{
  volatile CAN_MemMapPtr CAN;
  uint32_t           reg;

  CAN = (CAN_MemMapPtr)ptr;

  // �������� ����� ������ � �������� ��
  reg = CAN->ESR1 & (
                     BIT(17)   // TWRNINT. 1 The Tx error counter transitioned from less than 96 to greater than or equal to 96.
                     + BIT(16) // RWRNINT. 1 The Rx error counter transitioned from less than 96 to greater than or equal to 96.
                     + BIT(2)  // BOFFINT. �Bus Off� Interrupt
                     + BIT(1)  // ERRINT . Error Interrupt
                     + BIT(0)  // WAKINT . Wake-Up Interrupt
                    );
  if (reg)
  {
    CAN->ESR1 = reg; // ���������� �����
  }
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
int Solve_can_timings(uint32_t bus_clock, uint32_t bitrate, T_can_ctrl *ctrl)
{
  uint32_t TQ_x_Prescaler = bus_clock / bitrate;
  uint32_t TQ;
  uint32_t lowest_diff;
  uint32_t diff;
  uint32_t best_TQ;
  uint32_t actual_freq;


  ctrl->pseg1 = 0;
  ctrl->pseg2 = 0;
  ctrl->propseg = 0;
  ctrl->rjw = 0;
  ctrl->presdiv = 0;


  // If BSP_CANPE_CLOCK is defined, then we will calculate the CAN bit timing parameters
  // using the method outlined in AN1798, section 4.1. A maximum time for PROP_SEG will be used,
  // the remaining TQ will be split equally between PSEG1 and PSEG2, provided PSEG2 >=2. RJW is
  // set to the minimum of 4 or PSEG1.


  if (TQ_x_Prescaler < (CAN_MIN_TQ - 1))
  {
    // We will be off by more than 12.5%
    return MQX_ERROR;
  }

  // First, find the best TQ and pre-scaler to use for the desired frequency. If any exact matches
  // is found, we use the match that gives us the lowest pre-scaler and highest TQ, otherwise we pick
  // the TQ and prescaler that generates the closest frequency to the desired frequency.

  lowest_diff = bitrate;
  best_TQ = 0;
  for (TQ = CAN_MAX_TQ; TQ >= CAN_MIN_TQ; TQ--)
  {
    ctrl->presdiv = TQ_x_Prescaler / TQ;
    if (ctrl->presdiv <= 256)
    {
      if (TQ_x_Prescaler == TQ * ctrl->presdiv)
      {
        best_TQ = TQ;
        break;
      }
      actual_freq = (bus_clock / ctrl->presdiv) / TQ;

      if (actual_freq > bitrate)
      {
        diff = actual_freq - bitrate;
      }
      else
      {
        diff = bitrate - actual_freq;
      }

      if (diff < lowest_diff)
      {
        lowest_diff = diff;
        best_TQ = TQ;
      }
    }
  }
  if ((best_TQ >= CAN_MIN_TQ) && (ctrl->presdiv <= 256))
  {
    ctrl->pseg2 = (best_TQ - CAN_MAX_PROPSEG) / 2;
    if (ctrl->pseg2 < CAN_MIN_PSEG2) ctrl->pseg2 = CAN_MIN_PSEG2;

    if (best_TQ == CAN_MIN_TQ)
    {
      ctrl->pseg1 = 1;
    }
    else
    {
      ctrl->pseg1 = ctrl->pseg2;
    }

    ctrl->propseg = best_TQ - ctrl->pseg1 - ctrl->pseg2 - 1;

    if (ctrl->pseg1 < CAN_MAX_RJW)
    {
      ctrl->rjw = ctrl->pseg1;
    }
    else
    {
      ctrl->rjw = CAN_MAX_RJW;
    }

    ctrl->propseg -= 1;
    ctrl->rjw -= 1;
    ctrl->pseg1 -= 1;
    ctrl->pseg2 -= 1;
    ctrl->presdiv -= 1;
  }
  else
  {
    return MQX_ERROR;
  }

  return MQX_OK;

}



/*-------------------------------------------------------------------------------------------------------------
  �������� �������� ������������� ������ ���������� ������
-------------------------------------------------------------------------------------------------------------*/
static _mqx_uint Create_CANTX_control_sync_obj()
{
  _mqx_uint res;

  res = _lwevent_create(&can_tx_event, LWEVENT_AUTO_CLEAR);
  if (res != MQX_OK)
  {
    return res;
  }


  // ������� ������� ��������� ��� �������� ������� �����������
  can_tx_queue_id = _msgq_open(CAN_TX_QUEUE, 0);
  if (can_tx_queue_id == MSGQ_NULL_QUEUE_ID)
  {
    res = _task_get_error();
    return res;
  }

  // �������� ��� ������ ��� ���������
  can_msg_pool_id = _msgpool_create(sizeof(T_can_tx_message), CAT_TX_QUEUE_SZ, 0, 0);
  if (can_msg_pool_id == 0)
  {
    res = _task_get_error();
  }

  return MQX_OK;
}



/*-------------------------------------------------------------------------------------------------------------
  ������������ ���������� CAN ����

  ptr - ��������� �� ������� ��������� ����������� CAN:  CAN0_BASE_PTR ��� CAN1_BASE_PTR
-------------------------------------------------------------------------------------------------------------*/
int CAN_init(CAN_MemMapPtr ptr, uint32_t bitrate)
{
  _mqx_uint              res;

  SIM_MemMapPtr          SIM  = SIM_BASE_PTR;
  T_can_ctrl             ctrl;
  uint32_t           i;

  CAN_PTR = ptr;

  res = _lwevent_create(&can_rx_event, 0);
  if (res != MQX_OK)
  {
    return res;
  }
#ifdef ENABLE_CAN_LOG
  res = _lwsem_create(&can_log_sem, 1);
  if (res != MQX_OK)
  {
    return res;
  }
#endif

  if (ptr == CAN0_BASE_PTR)
  {
    SIM->SCGC6 |= BIT(4);  // ��������� ������������ CAN0
    CAN_PTR->CTRL1 |= BIT(13); // ������������ �� ��������� ������������ ���������

    // ������������� ���� ����������
    _int_install_isr(INT_CAN0_ORed_Message_buffer, CAN_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_ORed_Message_buffer, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN0_Bus_Off, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_Bus_Off, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN0_Error, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_Error, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN0_Tx_Warning, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_Tx_Warning, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN0_Rx_Warning, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_Rx_Warning, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN0_Wake_Up, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN0_Wake_Up, CAN_ISR_PRIO, 0, TRUE);
  }
  else if (ptr == CAN1_BASE_PTR)
  {
    SIM->SCGC3 |= BIT(4);  // ��������� ������������ CAN1
    CAN_PTR->CTRL1 |= BIT(13); // ������������ �� ��������� ������������ ���������

    // ������������� ���� ����������
    _int_install_isr(INT_CAN1_ORed_Message_buffer, CAN_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_ORed_Message_buffer, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN1_Bus_Off, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_Bus_Off, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN1_Error, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_Error, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN1_Tx_Warning, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_Tx_Warning, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN1_Rx_Warning, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_Rx_Warning, CAN_ISR_PRIO, 0, TRUE);

    _int_install_isr(INT_CAN1_Wake_Up, CAN_err_isr, (void *)ptr);
    _bsp_int_init(INT_CAN1_Wake_Up, CAN_ISR_PRIO, 0, TRUE);
  }
  else return MQX_ERROR;

  // ��������� ����������� ����� � ��������� �� Freeze mode
  if (CAN_PTR->MCR & BIT(20)) // �������� �������� �� ������, � ������� ���� ����� ���� ����
  {
    CAN_PTR->MCR &= (~BIT(31));       // �������� ������
    while (CAN_PTR->MCR & BIT(20))    // ���� ���� ���� �� ���������
    {
    }
  }

  CAN_PTR->MCR = BIT(25);             // ��������� SOFTRESET
  while (CAN_PTR->MCR & BIT(25))      // ������� ���� SOFTRESET ����������
  {
  }

  CAN_PTR->MCR |= 0
              + BIT(30) // FRZ    . Freeze Enable. 1 Enabled to enter Freeze Mode
              + BIT(28) // HALT   . Halt FlexCAN. 1 Enters Freeze Mode if the FRZ bit is asserted.
  ;
  while ((CAN_PTR->MCR & BIT(24)) == 0)   // ������� ��������� FRZACK
  {
  }

  //Solve_can_timings(BSP_BUS_CLOCK, bitrate, &ctrl);

  // �� ����� DoorController �������� ������� ��������� CAN = 36 000 000 ��.
  // ������� ������� = 1800000 ������������ ������ = 0.16666... ���
  // ������� ����� = 333333 ��
  // � ���� 18 �������, ��� PROP_SEG + PHASE_SEG1 = 12, PHASE_SEG2 = 5

  // ����������� ������� �������� CAN ����� �������������� � ��������� DoorController. ������� ������������ CAN ��������� ����� ����� 60 ���
  // �������� ��������� ��������� �� ����� FrontEnd
  ctrl.presdiv = 10 - 1; // �������� ������� = 10. �������� ����� Tq = 10/60000000 = 0.16666... ���
  ctrl.rjw = 4 - 1;  // ����������� �������� = 3 (�� ����� ���� �� ������)
  ctrl.propseg = 4 - 1;  // ����� PROP_SEG.   3-� ������ ���� (���� ����� ������� �������)
  ctrl.pseg1 = 8 - 1;  // ����� PHASE_SEG1. 3-� ������ ���� (���� ����� ������� �������).
  ctrl.pseg2 = 5 - 1;  // ����� PHASE_SEG2. 3-� ������ ���� (���� ����� ������ �������).
                       // ����� ���� � ���������� ���������� = SYNC_SEG(������ ����� 1) + PROP_SEG + PHASE_SEG1 + PHASE_SEG2 = 18Tq = 18*0.55 = 9.9 ���

  CAN_PTR->CTRL1 = 0
               + LSHIFT(ctrl.presdiv, 24) // PRESDIV. Prescaler Division Factor
               + LSHIFT(ctrl.rjw, 22) // RJW    . Resync Jump Width
               + LSHIFT(ctrl.pseg1, 19) // PSEG1  . Phase Segment 1
               + LSHIFT(ctrl.pseg2, 16) // PSEG2  . Phase Segment 2
               + LSHIFT(1, 15) // BOFFMSK. Bus Off Mask. 1 Bus Off interrupt enabled
               + LSHIFT(1, 14) // ERRMSK . Error Mask.   1 Error interrupt enabled
               + LSHIFT(1, 13) // CLKSRC . CAN Engine Clock Source. 1 The CAN engine clock source is the peripheral clock
               + LSHIFT(0, 12) // LPB    . Loop Back Mode
               + LSHIFT(1, 11) // TWRNMSK. Tx Warning Interrupt Mask. 1 Tx Warning Interrupt enabled
               + LSHIFT(1, 10) // RWRNMSK. Rx Warning Interrupt Mask. 1 Rx Warning Interrupt enabled
               + LSHIFT(1, 7) // SMP    . CAN Bit Sampling. 0 Just one sample is used to determine the bit value
               + LSHIFT(0, 6) // BOFFREC. Bus Off Recovery. 1 Automatic recovering from Bus Off state disabled
               + LSHIFT(0, 5) // TSYN   . Timer Sync. 1 Timer Sync feature enabled
               + LSHIFT(0, 4) // LBUF   . Lowest Buffer Transmitted First. 1 Lowest number buffer is transmitted first.
               + LSHIFT(0, 3) // LOM    . Listen-Only Mode. 1 FlexCAN module operates in Listen-Only Mode.
               + LSHIFT(ctrl.propseg, 0) // PROPSEG. Propagation Segment.
  ;

  CAN_PTR->CTRL2 = 0
               + LSHIFT(1, 28) // WRMFRZ. Write-Access to Memory in Freeze mode. 1 Enable unrestricted write access to FlexCAN memory.
               + LSHIFT(0, 24) // RFFN  . Number of Rx FIFO Filters
               + LSHIFT(0x22, 19) // TASD  . Tx Arbitration Start Delay.
               + LSHIFT(0, 18) // MRP   . Mailboxes Reception Priority. 0 Matching starts from Rx FIFO and continues on Mailboxes.
               + LSHIFT(0, 17) // RRS   . Remote Request Storing. 0 Remote Response Frame is generated.
               + LSHIFT(0, 16) // EACEN . Entire Frame Arbitration Field Comparison Enable for Rx Mailboxes.0 Rx Mailbox filter�s IDE bit is always compared and RTR is never compared despite mask bits.
  ;

  for (i = 0; i < 16; i++)
  {
    CAN_PTR->RXIMR[i] = 0;    // ������� ��� �����
    CAN_PTR->MB[i].CS = 0;    // ��� ������� ��������� �� �������
    CAN_PTR->MB[i].ID = 0;    //
    CAN_PTR->MB[i].WORD0 = 0; //
    CAN_PTR->MB[i].WORD1 = 0; //
  }
  //CAN->RXIMR[CAN_RX_MB1] = 0xFF000000;

  // ������� �� ������ Freeze
  CAN_PTR->MCR = 0
             + LSHIFT(0, 31) // MDIS   . Module Disable. 1 Disable the FlexCAN module.
             + LSHIFT(0, 30) // FRZ    . Freeze Enable. 1 Enabled to enter Freeze Mode
             + LSHIFT(0, 29) // RFEN   . Rx FIFO Enable. 1 Rx FIFO enabled
             + LSHIFT(0, 28) // HALT   . Halt FlexCAN. 1 Enters Freeze Mode if the FRZ bit is asserted.
             + LSHIFT(0, 26) // WAKMSK . Wake Up Interrupt Mask. 1 Wake Up Interrupt is enabled.
             + LSHIFT(0, 25) // SOFTRST. Soft Reset. 1 Resets the registers affected by soft reset.
             + LSHIFT(0, 23) // SUPV   . Supervisor Mode. 1 FlexCAN is in Supervisor Mode.
             + LSHIFT(0, 22) // SLFWAK . Self Wake Up. 1 FlexCAN Self Wake Up feature is enabled.
             + LSHIFT(1, 21) // WRNEN  . Warning Interrupt Enable. 1 TWRNINT and RWRNINT bits are set when the respective error counter transitions from less than 96 to greater than or equal to 96.
             + LSHIFT(1, 17) // SRXDIS . Self Reception Disable. 1 Self reception disabled
             + LSHIFT(1, 16) // IRMQ   . Individual Rx Masking and Queue Enable. 1 Individual Rx masking and queue feature are enabled.
             + LSHIFT(0, 13) // LPRIOEN. Local Priority Enable. 1 Local Priority enabled
             + LSHIFT(0, 12) // AEN    . Abort Enable. 0 Abort disabled.  !!!�� ���������� 1, ��������� ��������� ������������� MB ��������� �� ����� 0
             + LSHIFT(0, 8) // IDAM   . ID Acceptance Mode. 00 Format A: One full ID (standard and extended) per ID Filter Table element.
             + LSHIFT(16, 0) // MAXMB  . Number of the Last Message Buffer.
  ;

  return MQX_OK;
}



/*-------------------------------------------------------------------------------------------------------------
  ��������� ������� �� ����� � ��������� ������
  n    - ����� ������
  id   - ������������� CAN ������
  mask - ������-����� ������������ ��������������
  ext  - ���� ����������� ������ �� 1, ����� 0
  rtr  - ���� 1 �� ����� ��������� ����� ���� Remote Request Frame
-------------------------------------------------------------------------------------------------------------*/
void CAN_set_rx_mbox(volatile CAN_MemMapPtr CAN, uint8_t n, uint32_t id, uint32_t mask, uint8_t ext, uint8_t rtr)
{
  // ��������� � Freeze ����� ��������� ����� ������� ����� �������� ������ � ���� ������
  CAN->MCR |= 0
              + BIT(30) // FRZ    . Freeze Enable. 1 Enabled to enter Freeze Mode
              + BIT(28) // HALT   . Halt FlexCAN. 1 Enters Freeze Mode if the FRZ bit is asserted.
  ;
  while ((CAN->MCR & BIT(24)) == 0)   // ������� ��������� FRZACK
  {
  }

  // ������� ���� ����������
  CAN->IFLAG1 |= (1 << n);

  // �������������� mailbox �� ��������
  CAN->MB[n].CS = 0
                  + LSHIFT(0x0, 24) // CODE. 0b0000:INACTIVE- MB is not active.
                  + LSHIFT(1, 22) // SRR.  Substitute Remote Request. (������ ���� 1 � ����������� �������)
                  + LSHIFT(ext, 21) // IDE.  ID Extended Bit.           1 = ����������� ������
                  + LSHIFT(rtr, 20) // RTR.  Remote Transmission Request. 1 - ��� mailbox � Remote Request Frame
                  + LSHIFT(0, 16) // DLC.  ����� ������
  ;
  CAN->MB[n].ID = id;
  CAN->MB[n].WORD0 = 0;
  CAN->MB[n].WORD1 = 0;
  CAN->RXIMR[n] = mask; // ��������� �����


  CAN->IMASK1 |= (1 << n); // ��������� ���������� ����� ������

  // ���������� ���������� � ������
  CAN->MB[n].CS |= BIT(26); // 0b0100: EMPTY - MB is active and empty.

  CAN->MCR &= ~(BIT(30) + BIT(28)); // ������� �� ������ Freeze
  while (CAN->MCR & BIT(24))        // ������� ������ FRZACK
  {
  }

}

/*-------------------------------------------------------------------------------------------------------------
  ��������� ������� �� �������� � ��������� ��������
  n    - ����� ������
  id   - ������������� CAN ������
  data - ������ � ������� ������� 8 ����
  len  - ����� ������
  ext  - ���� ����������� ������ �� 1, ����� 0
  rtr  - ���� 1 �� ����� ��������� ����� ���� Remote transmission request
-------------------------------------------------------------------------------------------------------------*/
static _mqx_uint CAN_set_tx_mbox(volatile CAN_MemMapPtr CAN, uint8_t n, uint32_t id, uint8_t *data, uint8_t len, uint8_t ext, uint8_t rtr)
{
  // ������� ���� ����������
  CAN->IFLAG1 |= (1 << n);
  // �������������� mailbox �� ��������
  CAN->MB[n].CS = 0
                  + LSHIFT(0x8, 24) // CODE. 0b1000:MB is not active
                  + LSHIFT(1, 22) // SRR.  Substitute Remote Request. (������ ���� 1 � ����������� �������)
                  + LSHIFT(ext, 21) // IDE.  ID Extended Bit.           1 = ����������� ������
                  + LSHIFT(rtr, 20) // RTR.  Remote Transmission Request. 1 - ��� mailbox � Remote Request Frame
                  + LSHIFT(len, 16) // DLC.  ����� ������
  ;
  CAN->MB[n].ID = id;
  CAN->MB[n].WORD0 = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
  CAN->MB[n].WORD1 = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];

  // ��������� ���������� ����� ��������
  CAN->IMASK1 |= (1 << n);
  // ��������� ��������
  CAN->MB[n].CS |= BIT(26); // 0b1100: DATA - MB is a Tx Data Frame (MB RTR must be 0)

  return MQX_OK;
}


/*-------------------------------------------------------------------------------------------------------------
  ������� ������ �� mailbox-� � �������� �������
  n    - ����� ������
-------------------------------------------------------------------------------------------------------------*/
void CAN_read_rx_mbox(volatile CAN_MemMapPtr CAN, uint8_t n, T_can_msg *rx)
{

  uint32_t          cs;
  uint32_t          w;
  volatile uint32_t tmp;

  cs = CAN->MB[n].CS;
  rx->len = (cs >> 16) & 0x0F;
  rx->ext = (cs >> 21) & 1;
  rx->rtr = (cs >> 20) & 1;
  rx->code = (cs >> 24) & 0x0F;
  rx->canid = CAN->MB[n].ID;
  w = CAN->MB[n].WORD0;
  rx->data[0] = (w >> 24) & 0xFF;
  rx->data[1] = (w >> 16) & 0xFF;
  rx->data[2] = (w >> 8) & 0xFF;
  rx->data[3] = (w >> 0) & 0xFF;
  w = CAN->MB[n].WORD1;
  rx->data[4] = (w >> 24) & 0xFF;
  rx->data[5] = (w >> 16) & 0xFF;
  rx->data[6] = (w >> 8) & 0xFF;
  rx->data[7] = (w >> 0) & 0xFF;
  tmp = CAN->TIMER; // ������������� Mialbox

}


#ifdef ENABLE_CAN_LOG
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_wait_log_rec(_mqx_uint ticks)
{
  if (_lwevent_wait_ticks(&can_rx_event, 0x80000000, FALSE, ticks) == MQX_OK)
  {
    _lwevent_clear(&can_rx_event, 0x80000000);
    return MQX_OK;
  }
  return MQX_ERROR;
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_pop_log_rec(T_can_msg *rx)
{
  _lwsem_wait(&can_log_sem);
  if (canrx_log_head != canrx_log_tail)
  {
    *rx = canrx_log[canrx_log_tail];
    canrx_log_tail++;
    if (canrx_log_tail >= CAN_RX_LOG_SZ)
    {
      canrx_log_tail = 0;
    }
    _lwsem_post(&can_log_sem);
    return MQX_OK;
  }
  _lwsem_post(&can_log_sem);
  return MQX_ERROR;
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_push_log_rec(T_can_msg *rx)
{
  _lwsem_wait(&can_log_sem);
  canrx_log[canrx_log_head] = *rx;
  canrx_log_head++;
  if (canrx_log_head >= CAN_RX_LOG_SZ)
  {
    canrx_log_head = 0;
  }
  if (canrx_log_head == canrx_log_tail)
  {
    canrx_log_tail++;
    if (canrx_log_tail >= CAN_RX_LOG_SZ)
    {
      canrx_log_tail = 0;
    }
  }
  _lwsem_post(&can_log_sem);

  _lwevent_set(&can_rx_event, 0x80000000); // ��������� � ������ � ���
  return 0;
}
#endif

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_post_packet_to_send(uint32_t canid, uint8_t *data, uint8_t len)
{
  T_can_tx_message *cmpt;

  // ������� ������ ��� ���������
  cmpt = (T_can_tx_message *)_msg_alloc(can_msg_pool_id);
  if (cmpt != NULL)
  {
    cmpt->header.TARGET_QID = _msgq_get_id(0, CAN_TX_QUEUE);
    cmpt->header.SIZE = sizeof(T_can_tx_message);
    cmpt->canid = canid;
    cmpt->rtr = CAN_NO_RTR;
    if (len > 0)
    {
      memcpy(cmpt->data, data, len);
    }
    cmpt->len = len;
    if (_msgq_send(cmpt) == FALSE)
    {
      return _task_get_error();
    }
    return MQX_OK;
  }
  else return MQX_ERROR;
}
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_post_rtr_packet_to_send(uint32_t canid, uint8_t *data, uint8_t len)
{
  T_can_tx_message *cmpt;

  // ������� ������ ��� ���������
  cmpt = (T_can_tx_message *)_msg_alloc(can_msg_pool_id);
  if (cmpt != NULL)
  {
    cmpt->header.TARGET_QID = _msgq_get_id(0, CAN_TX_QUEUE);
    cmpt->header.SIZE = sizeof(T_can_tx_message);
    cmpt->canid = canid;
    cmpt->rtr = CAN_RTR;
    if (len > 0)
    {
      memcpy(cmpt->data, data, len);
    }
    cmpt->len = len;
    if (_msgq_send(cmpt) == FALSE)
    {
      return _task_get_error();
    }
    return MQX_OK;
  }
  else return MQX_ERROR;
}


/*-------------------------------------------------------------------------------------------------------------
  ������� ������ �� ����� CAN
  Extended ������, ��� ����� RTR
-------------------------------------------------------------------------------------------------------------*/
_mqx_uint CAN_send_packet(uint32_t canid, uint8_t *data, uint8_t len, uint8_t rtr)
{
  return CAN_set_tx_mbox(CAN_PTR, CAN_TX_MB1, canid, data, len, CAN_EXTENDED_FORMAT, rtr);
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Task_CAN_Tx(uint32_t parameter)
{
  if (Create_CANTX_control_sync_obj() != MQX_OK)
  {
    return;
  }


  if (parameter != 0 )
  {
    ((T_can_processing)parameter)();
  }
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Task_CAN_Rx(uint32_t parameter)
{
  uint32_t   n;

  // ������������� ���� ���������� ��������������� ��� ������ ���������
  for (n = 0; n < RX_MBOX_CNT; n++)
  {
    CAN_set_rx_mbox(CAN_PTR, rx_mboxes[n].mb_num, rx_mboxes[n].canid, rx_mboxes[n].canid_mask, 1, 0);
    can_events_mask |= (1 << rx_mboxes[n].mb_num);
  }

  if (parameter != 0 )
  {
    ((T_can_processing)parameter)();
  }
}


