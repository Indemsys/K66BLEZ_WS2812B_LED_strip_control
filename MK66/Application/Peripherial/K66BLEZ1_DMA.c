#include "App.h"








static volatile uint8_t   dummy_tx = 0xFF;
static volatile uint8_t   dummy_rx;
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Init_DMA(void)
{
  unsigned int  i;
  DMA_MemMapPtr DMA = DMA_BASE_PTR;
  SIM_MemMapPtr SIM = SIM_BASE_PTR;


  SIM->SCGC6 |= 0
    + LSHIFT(1, 1) // DMAMUX. DMAMUX clock gate control. 1 Clock is enabled.
  ;

  SIM->SCGC7 |= 0
    + LSHIFT(1, 1) // DMA. DMA controller clock gate control. 1 Clock is enabled.
  ;

  // ��������� ��� �������� DMA ���� ��� ������������� � ������ ������ � ������������� ��� ����������� ��������
  DMA->CR = 0
    + LSHIFT(1, 17) // CX      | Cancel Transfer. 1 Cancel the remaining data transfer
    + LSHIFT(0, 16) // ECX     | Error Cancel Transfer. 1 Cancel the remaining data transfer in the same fashion as the CX bit.
    + LSHIFT(1, 10) // GRP1PRI | Channel Group 1 Priority. Group 1 priority level when fixed priority group arbitration is enabled
    + LSHIFT(0, 8)  // GRP0PRI | Channel Group 0 Priority. Group 0 priority level when fixed priority group arbitration is enabled.
    + LSHIFT(0, 7)  // EMLM    | Enable Minor Loop Mapping. 1 Enabled. TCDn.word2 is redefined to include individual enable fields, an offset field, and the NBYTES field.
    + LSHIFT(0, 6)  // CLM     | Continuous Link Mode. 1 A minor loop channel link made to itself does not go through channel arbitration before being activated again.
    + LSHIFT(1, 5)  // HALT    | Halt DMA Operations. 1 Stall the start of any new channels
    + LSHIFT(0, 4)  // HOE     | Halt On Error. 1 Any error causes the HALT bit to set.
    + LSHIFT(0, 3)  // ERGA    | Enable Round Robin Group Arbitration. 1 Round robin arbitration is used for selection among the groups.
    + LSHIFT(0, 2)  // ERCA    | Enable Round Robin Channel Arbitration. 1 Round robin arbitration is used for channel selection within each group.
    + LSHIFT(0, 1)  // EDBG    | Enable Debug. 1 When in debug mode, the DMA stalls the start of a new channel.
  ;
  while (DMA->CR & BIT(17));  // ������� ������ ����� CX
  DMA->CR = BIT(17) + BIT(10) + BIT(5); // ��������� ���������� � ��������� �������� �� ������ ������� ��������� ��������
  while (DMA->CR & BIT(17));  // ������� ������ ����� CX

  DMA->EEI = 0; // ��������� ���������� �� ���� ������� � ������ ������
  DMA->CINT = BIT(6); // CAIR. Clear All Interrupt Requests. 1 Clear all bits in INT
  DMA->CERR = BIT(6); // CAEI. Clear All Error Indicators. Clear all bits in ERR
  DMA->CERQ = BIT(6); // CAER. Clear All Enable Requests. 1 Clear all bits in ERQ

/*
  ���������� ���� 32-� ������� ��������� �� ���������
  � ������ ������ �� 16 ������� ����� 0 ����� ������ ���������, ����� 15 - ������
  DMA->DCHPRI0 = 0
                 + LSHIFT(1, 7) // ECP    | Enable Channel Preemption.
                 + LSHIFT(0, 6) // DPA    | Disable Preempt Ability.
                 + LSHIFT(0, 4) // GRPPRI | Channel n Current Group Priority (read only)
                 + LSHIFT(0, 4) // CHPRI  | Channel n Arbitration Priority
  ;
*/

  // ������������ Round Robin Group Arbitration � Fixed priority arbitration is used for channel selection
  // � ����� Enable Minor Loop Mapping
  DMA->CR = 0
    + LSHIFT(0, 17) // CX      | Cancel Transfer. 1 Cancel the remaining data transfer
    + LSHIFT(0, 16) // ECX     | Error Cancel Transfer. 1 Cancel the remaining data transfer in the same fashion as the CX bit.
    + LSHIFT(1, 10) // GRP1PRI | Channel Group 1 Priority. Group 1 priority level when fixed priority group arbitration is enabled
    + LSHIFT(0, 8)  // GRP0PRI | Channel Group 0 Priority. Group 0 priority level when fixed priority group arbitration is enabled.
    + LSHIFT(0, 7)  // EMLM    | Enable Minor Loop Mapping. 1 Enabled. TCDn.word2 is redefined to include individual enable fields, an offset field, and the NBYTES field.
    + LSHIFT(0, 6)  // CLM     | Continuous Link Mode. 1 A minor loop channel link made to itself does not go through channel arbitration before being activated again.
    + LSHIFT(0, 5)  // HALT    | Halt DMA Operations. 1 Stall the start of any new channels
    + LSHIFT(0, 4)  // HOE     | Halt On Error. 1 Any error causes the HALT bit to set.
    + LSHIFT(0, 3)  // ERGA    | Enable Round Robin Group Arbitration. 1 Round robin arbitration is used for selection among the groups.
    + LSHIFT(0, 2)  // ERCA    | Enable Round Robin Channel Arbitration. 1 Round robin arbitration is used for channel selection within each group.
    + LSHIFT(0, 1)  // EDBG    | Enable Debug. 1 When in debug mode, the DMA stalls the start of a new channel.
  ;

  for (i = 0; i < DMA_CHANNELS_COUNT; i++)
  {
    DMA->TCD[i].SADDR = 0;          // Memory address pointing to the source data
    DMA->TCD[i].SOFF = 0;           // Sign-extended offset applied to the current source address to form the next-state value as each source read is completed.
    DMA->TCD[i].ATTR = 0
      + LSHIFT(0, 11) // SMOD  | Source Address Modulo
      + LSHIFT(0, 8)  // SSIZE | Source data transfer size. 000 8-bit,001 16-bit,010 32-bit,011 Reserved,100 16-byte burst,101 32-byte burst
      + LSHIFT(0, 3)  // DMOD  | Destination Address Modulo
      + LSHIFT(0, 0)  // DSIZE | Destination data transfer size
    ;
    DMA->TCD[i].NBYTES_MLNO = 0    // Minor Byte Transfer Count
      + LSHIFT(0, 31) // SMLOE  | Source Minor Loop Offset Enable
      + LSHIFT(0, 30) // DMLOE  | Destination Minor Loop Offset enable
      + LSHIFT(0, 10) // MLOFF  | If SMLOE or DMLOE is set, this field represents a sign-extended offset applied to the source or destination address to form the next-state value after the minor loop completes.
      + LSHIFT(0, 0)  // NBYTES | Minor Byte Transfer Count
    ;

    DMA->TCD[i].SLAST = 0;          // Last Source Address Adjustment. Adjustment value added to the source address at the completion of the major iteration count
    DMA->TCD[i].DADDR = 0;          // Memory address pointing to the destination data.
    DMA->TCD[i].DOFF = 0;           // Sign-extended offset applied to the current destination address to form the next-state value as each destination write is completed.
    DMA->TCD[i].CITER_ELINKNO = 0
      + LSHIFT(0, 15) // ELINK  | Enable channel-to-channel linking on minor-loop complete
      + LSHIFT(0, 9)  // LINKCH | Link Channel Number
      + LSHIFT(0, 0)  // CITER  | Current Major Iteration Count
    ;
    DMA->TCD[i].DLAST_SGA = 0;      // Destination last address adjustment or the memory address for the next transfer control descriptor to be loaded into this channel (scatter/gather).
    DMA->TCD[i].CSR = 0
      + LSHIFT(0, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
      + LSHIFT(0, 8)  // MAJORLINKCH | Link Channel Number
      + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
      + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
      + LSHIFT(0, 5)  // MAJORELINK  | Enable channel-to-channel linking on major loop complete
      + LSHIFT(0, 4)  // ESG         | Enable Scatter/Gather Processing
      + LSHIFT(0, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
      + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
      + LSHIFT(0, 1)  // INTMAJOR    | Enable an interrupt when major iteration count completes
      + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
    ;

    DMA->TCD[i].BITER_ELINKNO = 0
      + LSHIFT(0, 15) // ELINK  | Enable channel-to-channel linking on minor-loop complete
      + LSHIFT(0, 9)  // LINKCH | Link Channel Number
      + LSHIFT(0, 0)  // BITER  | Starting major iteration count
    ;
  }
}




/*-------------------------------------------------------------------------------------------------------------
  �������������� DMA ��� �������� 8-� ������ ������ �� SPI
-------------------------------------------------------------------------------------------------------------*/
void Config_DMA_for_SPI_TX(T_DMA_SPI_TX_config *cfg, T_DMA_SPI_cbl *pDS_cbl)
{
  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;


  DMA->TCD[cfg->ch].SADDR = 0;                       // �������� - ����� � ������� (�� ���� ������������ �� �������������!!!)
  DMA->TCD[cfg->ch].SOFF = 1;                        // ����� ��������� ������� �� 1 ���� ����� ������ ��������
  DMA->TCD[cfg->ch].SLAST = 0;                       // �� ������������ ����� ��������� ����� ���������� ����� ����� DMA (��������� ��������� �����)
  DMA->TCD[cfg->ch].DADDR = (uint32_t)cfg->spi_pushr;  // ����� ��������� - ������� PUSHR SPI
  DMA->TCD[cfg->ch].DOFF = 0;                        // �����  ������ ��������� ��������� �� �������
  DMA->TCD[cfg->ch].DLAST_SGA = 0;                   // ������� ������������ �� ���������
  DMA->TCD[cfg->ch].NBYTES_MLNO = 1;                 // ���������� ���� ������������ �� ���� ������ DMA (� �������� �����)
  DMA->TCD[cfg->ch].BITER_ELINKNO = 0
    + LSHIFT(0, 15)                                 // ELINK  | �������� �� ���������
    + LSHIFT(0, 9)                                  // LINKCH |
    + LSHIFT(0, 0)                                  // BITER  | (�� ���� ������������ �� �������������!!!)
  ;
  DMA->TCD[cfg->ch].CITER_ELINKNO = 0
    + LSHIFT(0, 15)                                 // ELINK  | �������� �� ���������
    + LSHIFT(0, 9)                                  // LINKCH |
    + LSHIFT(0, 0)                                  // BITER  | (�� ���� ������������ �� �������������!!!)
  ;
  DMA->TCD[cfg->ch].ATTR = 0
    + LSHIFT(0, 11) // SMOD  | ������ ������ ��������� �� ����������
    + LSHIFT(0, 8)  // SSIZE | 8-� ������ ��������� �� ���������
    + LSHIFT(0, 3)  // DMOD  | ������ ������ ���������
    + LSHIFT(0, 0)  // DSIZE | 8-� ������ ��������� � ��������
  ;
  DMA->TCD[cfg->ch].CSR = 0
    + LSHIFT(0, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
    + LSHIFT(0, 8)  // MAJORLINKCH | �������� �� ���������
    + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
    + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
    + LSHIFT(0, 5)  // MAJORELINK  | �������� �� ���������
    + LSHIFT(0, 4)  // ESG         | ������� ������������ �� ���������
    + LSHIFT(1, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
    + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
    + LSHIFT(0, 1)  // INTMAJOR    | �� ���������� ���������� �� �������� ��������� DMA
    + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
  ;

  pDS_cbl->tx_ch  = cfg->ch;
  {
    uint8_t indx = cfg->ch;
    cfg->DMAMUX->CHCFG[indx] = cfg->dmux_src + BIT(7); // ����� ������������� ��������� ������ �� ������� ��������� (����� �� ������ SPI) � ������ ���������� ������ DMA
  }

}


/*-------------------------------------------------------------------------------------------------------------
  �������������� DMA ��� ������ ������ �� SPI
-------------------------------------------------------------------------------------------------------------*/
void Config_DMA_for_SPI_RX(T_DMA_SPI_RX_config *cfg, T_DMA_SPI_cbl *pDS_cbl)
{
  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;


  DMA->TCD[cfg->ch].SADDR = (uint32_t)cfg->spi_popr;   // �������� - FIFO ��������� SPI
  DMA->TCD[cfg->ch].SOFF = 0;                        // ����� ��������� �� ��������
  DMA->TCD[cfg->ch].SLAST = 0;                       // �� ������������ ����� ��������� ����� ���������� ����� ����� DMA (��������� ��������� �����)
  DMA->TCD[cfg->ch].DADDR = 0;                       // ����� ��������� - ����� ������. �� ���� ������������ �� �������������
  DMA->TCD[cfg->ch].DOFF = 1;                        // �����  ������ ��������� ��������� ������� �� 1
  DMA->TCD[cfg->ch].DLAST_SGA = 0;                   // ������� ������������ �� ���������
  DMA->TCD[cfg->ch].NBYTES_MLNO = 1;                 // ���������� ���� ������������ �� ���� ������ DMA (� �������� �����)
  DMA->TCD[cfg->ch].BITER_ELINKNO = 0
    + LSHIFT(0, 15)                                 // ELINK  | �������� �� ���������
    + LSHIFT(0, 9)                                  // LINKCH |
    + LSHIFT(0, 0)                                  // BITER  |
  ;
  DMA->TCD[cfg->ch].CITER_ELINKNO = 0
    + LSHIFT(0, 15)                                 // ELINK  | �������� �� ���������
    + LSHIFT(0, 9)                                  // LINKCH |
    + LSHIFT(0, 0)                                  // BITER  |
  ;
  DMA->TCD[cfg->ch].ATTR = 0
    + LSHIFT(0, 11) // SMOD  | ������ ������ ��������� �� ����������
    + LSHIFT(0, 8)  // SSIZE | 8-� ������ ��������� �� ���������
    + LSHIFT(0, 3)  // DMOD  | ������ ������ ���������
    + LSHIFT(0, 0)  // DSIZE | 8-� ������ ��������� � ��������
  ;
  DMA->TCD[cfg->ch].CSR = 0
    + LSHIFT(0, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
    + LSHIFT(0, 8)  // MAJORLINKCH | �������� �� ���������
    + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
    + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
    + LSHIFT(0, 5)  // MAJORELINK  | �������� �� ���������
    + LSHIFT(0, 4)  // ESG         | ������� ������������ �� ���������
    + LSHIFT(1, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
    + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
    + LSHIFT(1, 1)  // INTMAJOR    | ���������� ���������� �� �������� ��������� DMA
    + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
  ;

  pDS_cbl->rx_ch  = cfg->ch;
  {
    uint8_t indx = cfg->ch;
    cfg->DMAMUX->CHCFG[indx] = cfg->dmux_src + BIT(7); // ����� ������������� ��������� ������ �� ������� ��������� (����� �� ������ SPI) � ������ ���������� ������ DMA
  }

}


/*-------------------------------------------------------------------------------------------------------------
  �������������� DMA ��� �������� ������ �� SPI

  ������ ������������� ������: �� ����� 2048 ���� (MAX_DMA_SPI_BUFF) !!!
-------------------------------------------------------------------------------------------------------------*/
void Start_DMA_for_SPI_TX(T_DMA_SPI_cbl *pDS_cbl, const uint8_t *buf, uint32_t sz)
{
  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;

  T_DMA_TCD  volatile *pTDCmf;
  T_DMA_TCD  volatile *pTDCfm;

  // ���������� ����������� DMA ���������������� ��� ������� ������ �� SPI

  // ���������� ��� DONE
  DMA->CDNE = BIT(pDS_cbl->rx_ch);

  // ������������� ���������� ������� ������
  pTDCfm = (T_DMA_TCD *)&DMA->TCD[pDS_cbl->rx_ch];
  pTDCfm->DADDR         = (uint32_t)&dummy_rx;
  pTDCfm->DOFF          = 0;
  pTDCfm->BITER_ELINKNO = (pTDCfm->BITER_ELINKNO & ~0x7FF) |  sz;
  pTDCfm->CITER_ELINKNO = pTDCfm->BITER_ELINKNO;

  // ���������� ����������� DMA ��������������� ��� ������ ������ � SPI

  // ���������� ��� DONE
  DMA->CDNE = BIT(pDS_cbl->tx_ch);

  pTDCmf = (T_DMA_TCD *)&DMA->TCD[pDS_cbl->tx_ch];
  pTDCmf->SADDR         = (uint32_t)buf;
  pTDCmf->SOFF          = 1;
  pTDCmf->BITER_ELINKNO = (pTDCmf->BITER_ELINKNO & ~0x7FF) |  sz;
  pTDCmf->CITER_ELINKNO = pTDCmf->BITER_ELINKNO;

  DMA->SERQ = pDS_cbl->rx_ch;
  DMA->SERQ = pDS_cbl->tx_ch;
}

/*-------------------------------------------------------------------------------------------------------------
  �������������� DMA ��� ������ ������ �� SPI

  ������ ������������ ������: �� ����� 2048 ���� (MAX_DMA_SPI_BUFF) !!!
-------------------------------------------------------------------------------------------------------------*/
void Start_DMA_for_SPI_RX(T_DMA_SPI_cbl *pDS_cbl, const uint8_t *buf, uint32_t sz)
{
  DMA_MemMapPtr        DMA     = DMA_BASE_PTR;
  T_DMA_TCD  volatile *pTDCmf;
  T_DMA_TCD  volatile *pTDCfm;

  // ���������� ����������� DMA ���������������� ��� ������ �� SPI

  // ���������� ��� DONE
  DMA->CDNE = BIT(pDS_cbl->rx_ch);

  // ������������� ����������� � ����������� ������ ������� � �������
  pTDCfm = (T_DMA_TCD *)&DMA->TCD[pDS_cbl->rx_ch];
  pTDCfm->DADDR         = (uint32_t)buf;
  pTDCfm->DOFF          = 1;
  pTDCfm->BITER_ELINKNO = (pTDCfm->BITER_ELINKNO & ~0x7FF) |  sz;
  pTDCfm->CITER_ELINKNO = pTDCfm->BITER_ELINKNO;


  // ���������� ����������� DMA ��������������� ��� ������ ������ � SPI

  // ���������� ��� DONE
  DMA->CDNE = BIT(pDS_cbl->tx_ch);

  pTDCmf = (T_DMA_TCD *)&DMA->TCD[pDS_cbl->tx_ch];
  pTDCmf->SADDR         = (uint32_t)&dummy_tx;
  pTDCmf->SOFF          = 0;
  pTDCmf->BITER_ELINKNO = (pTDCmf->BITER_ELINKNO & ~0x7FF) |  sz;
  pTDCmf->CITER_ELINKNO = pTDCmf->BITER_ELINKNO;


  DMA->SERQ = pDS_cbl->rx_ch;
  DMA->SERQ = pDS_cbl->tx_ch;
}


/*-------------------------------------------------------------------------------------------------------------
  �������������� DMA ��� �������� � ������ ������ �� SPI

  ������ ������������� ������: �� ����� 2048 ���� (MAX_DMA_SPI_BUFF) !!!
-------------------------------------------------------------------------------------------------------------*/
void Start_DMA_for_SPI_RXTX(T_DMA_SPI_cbl *pDS_cbl, const uint8_t *outbuf, uint8_t *inbuf, uint32_t sz)
{
  DMA_MemMapPtr        DMA     = DMA_BASE_PTR;
  T_DMA_TCD  volatile *pTDCmf;
  T_DMA_TCD  volatile *pTDCfm;

  // ���������� ����������� DMA ���������������� ��� ������ �� SPI

  // ���������� ��� DONE
  DMA->CDNE = BIT(pDS_cbl->rx_ch);

  // ������������� ����������  ������
  pTDCfm = (T_DMA_TCD *)&DMA->TCD[pDS_cbl->rx_ch];
  pTDCfm->DADDR         = (uint32_t)inbuf;
  pTDCfm->DOFF          = 1;
  pTDCfm->BITER_ELINKNO = (pTDCfm->BITER_ELINKNO & ~0x7FF) |  sz;
  pTDCfm->CITER_ELINKNO = pTDCfm->BITER_ELINKNO;

  // ���������� ����������� DMA ��������������� ��� ������ ������ � SPI

  // ���������� ��� DONE
  DMA->CDNE = BIT(pDS_cbl->tx_ch);

  pTDCmf = (T_DMA_TCD *)&DMA->TCD[pDS_cbl->tx_ch];
  pTDCmf->SADDR         = (uint32_t)outbuf;
  pTDCmf->SOFF          = 1;
  pTDCmf->BITER_ELINKNO = (pTDCmf->BITER_ELINKNO & ~0x7FF) |  sz;
  pTDCmf->CITER_ELINKNO = pTDCmf->BITER_ELINKNO;

  DMA->SERQ = pDS_cbl->rx_ch;
  DMA->SERQ = pDS_cbl->tx_ch;
}

