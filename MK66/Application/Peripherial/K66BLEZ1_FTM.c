// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.08.31
// 10:47:58
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


static void        FTM1_isr(void *user_isr_ptr);
static T_qdec_isr  qdec_isr;

#define QDEC_CNT_MOD 4


/*-------------------------------------------------------------------------------------------------------------
  ���������� ���������� ������� FTM1
  ���������� ���������� ��� ������������ ��������
  ������� ����������� ����� ������� ������������
-------------------------------------------------------------------------------------------------------------*/
static void FTM1_isr(void *user_isr_ptr)
{
  FTM_MemMapPtr FTM1 = FTM1_BASE_PTR;
  uint32_t sc;

  sc =  FTM1->SC; // ������ ���������
  if (sc & BIT(7))
  {
    FTM1->SC = sc & ~(BIT(7)); // ���������� ��� TOF

    if (qdec_isr != 0) qdec_isr(FTM1->QDCTRL & BIT(1));
  }
}

/*-------------------------------------------------------------------------------------------------------------
  ���������� ���������� ������� FTM3� 
  ������� �������� ������������ ��� ������� ADC 
  ���������� ���������� ��� ������������ ��������
 
  � ������ ����� ����� � ���� ���������� ����� ���������� ������ �����
-------------------------------------------------------------------------------------------------------------*/
static void FTM3_isr(void *user_isr_ptr)
{
  FTM_MemMapPtr FTM3 = FTM3_BASE_PTR;
  uint32_t sc;

  sc =  FTM3->SC; // ������ ���������
  if (sc & BIT(7))
  {
    FTM3->SC = sc & ~(BIT(7)); // ���������� ��� TOF
  }
// ����� ������� ������������� ADC ��� ���������� �� �����������
//  sc =  FTM3->EXTTRIG; // ������ ���������
//  if (sc & BIT(7))
//  {
//    FTM3->EXTTRIG = sc & ~(BIT(7)); // ���������� ��� TRIGF
//  }

}

/*------------------------------------------------------------------------------
  ���������������� FTM1 ��� ������ � ������ ������������� �������� 


 \param isr
 ------------------------------------------------------------------------------*/
void FTM1_init_QDEC(T_qdec_isr isr)
{
  FTM_MemMapPtr FTM1 = FTM1_BASE_PTR;

  // ��������� ������������ FTM1
  SIM_SCGC6 |= BIT(25);

  // �������� ������ ���������������� ������ ������� � ��������� ������ �� ��� ���� ���������
  FTM1->MODE = 0
    + LSHIFT(0, 7) // FAULTIE | Fault Interrupt Enable   |
    + LSHIFT(0, 5) // FAULTM  | Fault Control Mode       | 00 Fault control is disabled for all channels.
    + LSHIFT(0, 4) // CAPTEST | Capture Test Mode Enable
    + LSHIFT(0, 3) // PWMSYNC | PWM Synchronization Mode
    + LSHIFT(1, 2) // WPDIS   | Write Protection Disable | 1 Write protection is disabled.
    + LSHIFT(0, 1) // INIT    | Initialize the Channels Output | When a 1 is written to INIT bit the channels output is initialized according to the state of their corresponding bit in the OUTINIT register
    + LSHIFT(1, 0) // FTMEN   | FTM Enable | 1 All registers including the FTM-specific registers (second set of registers) are available for use with no restrictions.
  ;

  FTM1->CONF  = 0
    + LSHIFT(0, 10) // GTBEOUT | Global time base output
    + LSHIFT(0, 9)  // GTBEEN  | Global time base enable
    + LSHIFT(3, 6)  // BDMMODE | Selects the FTM behavior in BDM mode | � ������ ������� ����������� ������ ������� FTM
    + LSHIFT(0, 0)  // NUMTOF  | TOF Frequency | NUMTOF = 0: The TOF bit is set for each counter overflow.
  ;

  // ������ ����������
  FTM1->SC = 0
    + LSHIFT(0, 7) // TOF   | 1 FTM counter has overflowed. | ����� � 0
    + LSHIFT(0, 6) // TOIE  | 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
    + LSHIFT(0, 5) // CPWMS | 1 FTM counter operates in up-down counting mode.
    + LSHIFT(0, 3) // CLKS  | 00 ������ ������� ����������
    + LSHIFT(0, 0) // PS    | Prescale Factor Selection. 000 Divide by 1. ������������ = 1
  ;



  FTM1->MOD      = QDEC_CNT_MOD;    // ��������� �������� ������������. ���������� ����� 4-� ��������. ����� ������� ���������� ������� � ������������ ��������� ���������
  FTM1->CNTIN    = 0;    // ��������� �������� ��������
  FTM1->CNT      = 0;    // ������ � ������� ������� ������ �������� �������� � ������ �������� �� CNTIN � ��������� ���������� ��������� �������

  FTM1->FILTER  = LSHIFT(0xF, 4) + LSHIFT(0xF, 0); // �������� ����������� �� 16 ��������  � ������ ������

  // ������������� ����� ������������ ��������
  FTM1->QDCTRL  = 0
    + LSHIFT(1, 7) // PHAFLTREN | Phase A Input Filter Enable
    + LSHIFT(1, 6) // PHBFLTREN | Phase B Input Filter Enable
    + LSHIFT(0, 5) // PHAPOL    | Phase A Input Polarity
    + LSHIFT(0, 4) // PHBPOL    | Phase B Input Polarity
    + LSHIFT(0, 3) // QUADMODE  | Quadrature Decoder Mode  | 0 Phase A and phase B encoding mode.
    + LSHIFT(0, 2) // QUADIR    | Read Only. FTM Counter Direction in Quadrature Decoder Mode    | 1 Counting direction is increasing (FTM counter increment).
    + LSHIFT(0, 1) // TOFDIR    | Read Only. Timer Overflow Direction in Quadrature Decoder Mode | 1 TOF bit was set on the top of counting.
    + LSHIFT(1, 0) // QUADEN    | Quadrature Decoder Mode Enable
  ;



  // ��������� ��������� ����������� ������ �� ������
  FTM1->SC = 0
    + LSHIFT(1, 6) // TOIE. 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
    + LSHIFT(0, 5) // CPWMS. 1 -FTM counter operates in up-down counting mode.
    + LSHIFT(3, 3) // CLKS. 11 External clock
    + LSHIFT(0, 0) // PS. Prescale Factor Selection. 000 Divide by 1
  ;
  // ������������ ��������� �������
  qdec_isr = isr;
  Install_and_enable_isr(INT_FTM1, FTM1_ISR_PRIO, FTM1_isr);
}



/*-----------------------------------------------------------------------------------------------------
 ������������� ��������� FTM ��� ��������� PWM �������� ���������� ������ WS2812B
 ������ ��� �������� 0 � 3 !!!
 FTM ����������� ��� ���� � ����� TPM � ���������� ��������������, ��������� ������ � ���� ������ �������� DMA    
 
 ������������ �������� ������������ �� ������� ���� = 60 ���
 ������ ������� �� ��������� � ���������� 
 ������ PWM = 1.25 ��� 
 
 
 
 \param FTM   - ��������� �� ����������� ��������� �������
 \param presc - ������������ ������� ������� �������
 
-----------------------------------------------------------------------------------------------------*/
void FTM_init_PWM_DMA(FTM_MemMapPtr FTM)
{
  uint32_t sc_reg;
  uint8_t presc = FTM_PRESC_1;

  if (FTM == FTM0_BASE_PTR)
  {
    // ��������� ������������ FTM0
    SIM_SCGC6 |= BIT(24);
  }
  else if (FTM == FTM3_BASE_PTR)
  {
    // ��������� ������������ FTM3
    SIM_SCGC3 |= BIT(25);
  }
  else return;

  FTM->OUTINIT = 0    // ��������� �������� ��� ������� �������
    + LSHIFT(0, 7)  // CH7OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 6)  // CH6OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 5)  // CH5OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 4)  // CH4OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 3)  // CH3OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 2)  // CH2OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 1)  // CH1OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 0)  // CH0OI | 0 The initialization value is 0. 1 The initialization value is 1.
  ;

  // �������� ������ ���������������� ������ ������� � ��������� ������ �� ��� ���� ���������
  FTM->MODE = 0
    + LSHIFT(0, 7) // FAULTIE | Fault Interrupt Enable   |
    + LSHIFT(0, 5) // FAULTM  | Fault Control Mode       | 00 Fault control is disabled for all channels.
    + LSHIFT(0, 4) // CAPTEST | Capture Test Mode Enable
    + LSHIFT(0, 3) // PWMSYNC | PWM Synchronization Mode | 0 No restrictions. Software and hardware triggers can be used by MOD, CnV, OUTMASK, and FTM counter synchronization.
    + LSHIFT(1, 2) // WPDIS   | Write Protection Disable | 1 Write protection is disabled.
    + LSHIFT(1, 1) // INIT    | Initialize the Channels Output | When a 1 is written to INIT bit the channels output is initialized according to the state of their corresponding bit in the OUTINIT register
                   // ����� �������� ����� ������������� � TPM � ������� �������� DMA
                   // � ������ FTM ���������� ��������� DMA ��������� ��������� ������ ������� ��� �������� ��������� CnV �� �� �������
    + LSHIFT(0, 0) // FTMEN   | FTM Enable | 1 All registers including the FTM-specific registers (second set of registers) are available for use with no restrictions.
  ;

  FTM->CONF = 0
    + LSHIFT(0, 10) // GTBEOUT | Global time base output
    + LSHIFT(0, 9)  // GTBEEN  | Global time base enable
    + LSHIFT(3, 6)  // BDMMODE | Selects the FTM behavior in BDM mode | � ������ ������� ����������� ������ ������� FTM
    + LSHIFT(0, 0)  // NUMTOF  | TOF Frequency | NUMTOF = 0: The TOF bit is set for each counter overflow.
  ;

  // ������ ����������
  sc_reg = 0
    + LSHIFT(0, 7) // TOF   | 1 FTM counter has overflowed. | ����� � 0
    + LSHIFT(0, 6) // TOIE  | 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
    + LSHIFT(0, 5) // CPWMS | 1 FTM counter operates in up-down counting mode.0 FTM counter operates in Up Counting mode.
    + LSHIFT(0, 3) // CLKS  | 00 ������ ������� ����������
    + LSHIFT(presc, 0) // PS    | Prescale Factor Selection. 000 Divide by 1. ������������ = 1, 001 Divide by 2
  ;
  FTM->SC = sc_reg;

  // � ������ ������������� � TPM ��������� ������������� � ��������  FTM->SYNCONF �� ��������

  FTM->SYNCONF = 0
    + LSHIFT(0, 20) // HWSOC.      1 A hardware trigger activates the SWOCTRL register synchronization.
    + LSHIFT(0, 19) // HWINVC.     1 A hardware trigger activates the INVCTRL register synchronization.
    + LSHIFT(0, 18) // HWOM.       1 A hardware trigger activates the OUTMASK register synchronization.
    + LSHIFT(0, 17) // HWWRBUF.    1 A hardware trigger activates MOD, CNTIN, and CV registers synchronization.
    + LSHIFT(0, 16) // HWRSTCNT.   1 A hardware trigger activates the FTM counter synchronization.
    + LSHIFT(0, 12) // SWSOC.      1 The software trigger activates the SWOCTRL register synchronization.
    + LSHIFT(0, 11) // SWINVC.     1 The software trigger activates the INVCTRL register synchronization.
    + LSHIFT(0, 10) // SWOM.       1 The software trigger activates the OUTMASK register synchronization.
    + LSHIFT(0, 9) // SWWRBUF.    1 The software trigger activates MOD, CNTIN, and CV registers synchronization
    + LSHIFT(0, 8) // SWRSTCNT.   1 The software trigger activates the FTM counter synchronization. ���� 1 �� ���������� ������ ������ ��� ����� ����� 1 � SWSYNC
    + LSHIFT(1, 7) // SYNCMODE.   1 Enhanced PWM synchronization is selected.
    + LSHIFT(0, 5) // SWOC.       1 SWOCTRL ������� ����� �������� � ����� �������� ��� ������� ����� �������������, ���� 0 �� ����� �������� �����
    + LSHIFT(0, 4) // INVC.       1 INVCTRL ������� ����� �������� � ����� �������� ��� ������� ����� �������������, ���� 0 �� ����� �������� �����
    + LSHIFT(0, 2) // CNTINC.     1 CNTIN ������� ����� �������� � ����� �������� ��� ������� ����� �������������, ���� 0 �� ����� �������� �����
    + LSHIFT(0, 1) // HWTRIGMODE. 1 FTM does not clear the TRIGj bit when the hardware trigger j is detected.
  ;



  FTM->MOD     = FTM_WS2812B_MOD;   // ��������� �������� ������������.
  FTM->CNTIN   = 0;    // ��������� �������� ��������
  FTM->CNT     = 0;    // ������ � ������� ������� ������ �������� �������� � ������ �������� �� CNTIN � ��������� ���������� ��������� �������
  FTM->POL     = 0;    // ���������� � ��������

  FTM->OUTMASK = 0    // ���������� ������� ������� �� ������� ����������, �.�. ����� ���������� PWM �� �������� �������
    + LSHIFT(1, 7)  // CH7OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 6)  // CH6OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 5)  // CH5OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 4)  // CH4OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 3)  // CH3OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 2)  // CH2OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 1)  // CH1OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 0)  // CH0OM | 1 Channel output is masked. It is forced to its inactive state.
  ;
  FTM->COMBINE = 0     // ��������� ������ ������� �� ����������
                       // ��������� ���  ������� 6 � 7
    + LSHIFT(0, 30) // FAULTEN3. 1 The fault control in this pair of channels is enabled.
    + LSHIFT(1, 29) // SYNCEN3.  1 The PWM synchronization in this pair of channels is enabled. ������ ���� ����������� ��������� ����� ����� ���� ������ ������ � �������� CnV
    + LSHIFT(0, 28) // DTEN3.    0 The deadtime insertion in this pair of channels is disabled.
    + LSHIFT(0, 27) // DECAP3.   0 The dual edge captures are inactive.
    + LSHIFT(0, 26) // DECAPEN3  0 The dual edge capture mode in this pair of channels is disabled.
    + LSHIFT(0, 25) // COMP3     1 The channel (7) output is the complement of the channel (6) output.
    + LSHIFT(0, 24) // COMBINE3  1 Channels (6) and (7) are combined.
                    // ��������� ���  ������� 4 � 5
    + LSHIFT(0, 22) // FAULTEN2  1 The fault control in this pair of channels is enabled.
    + LSHIFT(1, 21) // SYNCEN2   1 The PWM synchronization in this pair of channels is enabled.
    + LSHIFT(0, 20) // DTEN2     0 The deadtime insertion in this pair of channels is disabled.
    + LSHIFT(0, 19) // DECAP2    0 The dual edge captures are inactive.
    + LSHIFT(0, 18) // DECAPEN2  0 The dual edge capture mode in this pair of channels is disabled.
    + LSHIFT(0, 17) // COMP2     1 The channel (5) output is the complement of the channel (4) output.
    + LSHIFT(0, 16) // COMBINE2  1 Channels (4) and (5) are combined.
                    // ��������� ���  ������� 2 � 3
    + LSHIFT(0, 14) // FAULTEN1  1 The fault control in this pair of channels is enabled.
    + LSHIFT(1, 13) // SYNCEN1   1 The PWM synchronization in this pair of channels is enabled.
    + LSHIFT(0, 12) // DTEN1     0 The deadtime insertion in this pair of channels is disabled.
    + LSHIFT(0, 11) // DECAP1    0 The dual edge captures are inactive.
    + LSHIFT(0, 10) // DECAPEN1  0 The dual edge capture mode in this pair of channels is disabled.
    + LSHIFT(0,  9) // COMP1     1 The channel (3) output is the complement of the channel (2) output.
    + LSHIFT(0,  8) // COMBINE1  1 Channels (2) and (3) are combined.
                    // ��������� ���  ������� 0 � 1
    + LSHIFT(0,  6) // FAULTEN0  1 The fault control in this pair of channels is enabled.
    + LSHIFT(1,  5) // SYNCEN0   1 The PWM synchronization in this pair of channels is enabled.
    + LSHIFT(0,  4) // DTEN0     0 The deadtime insertion in this pair of channels is disabled.
    + LSHIFT(0,  3) // DECAP0    0 The dual edge captures are inactive.
    + LSHIFT(0,  2) // DECAPEN0  0 The dual edge capture mode in this pair of channels is disabled.
    + LSHIFT(0,  1) // COMP0     1 The channel (1) output is the complement of the channel (0) output.
    + LSHIFT(0,  0) // COMBINE0  1 Channels (0) and (1) are combined.
  ;
  FTM->DEADTIME = 0; // ������� ����� �� ����������
  FTM->INVCTRL = 0;  // ���������� ���������������� ��� ������� �� ����������

  FTM->FILTER = 0;     // ������� ��� ������� ������� �� ����������


  // ��������� ������ ������� � ������ PWM ������������ �� �����
  // ��������� � 1 � ������ �������, ����� � 0 ����� �������� �����������

  FTM->CONTROLS[0].CnSC = 0
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[1].CnSC = 0
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[2].CnSC = 0
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[3].CnSC = 0
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[4].CnSC = 0
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[5].CnSC = 0
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[6].CnSC = 0
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[7].CnSC = 0
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;

  // �������� �� �������� ������������ �� ����������
  FTM->PWMLOAD = 0
    + LSHIFT(0, 9)  // LDOK   | �������������� �������� � �������� MOD, CNTIN, � CnV  ������ ���������� �������. ���� �����������. ��������� �� ���������� ������� �������������.
    + LSHIFT(0, 7)  // CH7SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 7
    + LSHIFT(0, 6)  // CH6SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 6
    + LSHIFT(0, 5)  // CH5SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 5
    + LSHIFT(0, 4)  // CH4SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 4
    + LSHIFT(0, 3)  // CH3SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 3
    + LSHIFT(0, 2)  // CH2SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 2
    + LSHIFT(0, 1)  // CH1SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 1
    + LSHIFT(0, 0)  // CH0SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 0
  ;
  // �������� ��������� �� ���������
  FTM->SWOCTRL = 0
    + LSHIFT(0, 15) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 14) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 13) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 12) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 11) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 10) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 9)  // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 8)  // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 7)  // CH7OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 6)  // CH6OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 5)  // CH5OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 4)  // CH4OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 3)  // CH3OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 2)  // CH2OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 1)  // CH1OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 0)  // CH0OC  | 1 The channel output is affected by software output control.
  ;

  // � ������ ������������� � TPM ��������� ������������� � ��������  FTM->SYNC �� ��������

  // ���������� ������������ ��������� �������� ��������� �� ��������
  FTM->SYNC = 0
    + LSHIFT(0, 7)  // SWSYNC  | PWM Synchronization Software Trigger. The software trigger happens when a 1 is written to this bit. ���� �����������. ��������� �� ���������� ������� �������������.
    + LSHIFT(0, 6)  // TRIG2   | PWM Synchronization Hardware Trigger 2
    + LSHIFT(0, 5)  // TRIG1   | PWM Synchronization Hardware Trigger 1
    + LSHIFT(0, 4)  // TRIG0   | PWM Synchronization Hardware Trigger 0
    + LSHIFT(0, 3)  // SYNCHOM | 0 OUTMASK register is updated with the value of its buffer in all rising edges of the system clock. 1 OUTMASK register is updated with the value of its buffer only by the PWM synchronization.
    + LSHIFT(0, 2)  // REINIT  | FTM Counter Reinitialization by Synchronization | 0 FTM counter continues to count normally. (�� ����� �������� � ������ ������ �������)
    + LSHIFT(0, 1)  // CNTMAX  | 1 The maximum loading point is enabled. ���� ��  ���� ������ (CNTMAX, CNTMIN) ������ ���� ����������� ���������� ��� ������ ���������� ������ � ��������
    + LSHIFT(0, 0)  // CNTMIN  | 1 The minimum loading point is enabled.
  ;

  FTM->CONTROLS[0].CnV = 0; // ������� �������� ������������ ��� ���� �������
  FTM->CONTROLS[1].CnV = 0;
  FTM->CONTROLS[2].CnV = 0;
  FTM->CONTROLS[3].CnV = 0;
  FTM->CONTROLS[4].CnV = 0;
  FTM->CONTROLS[5].CnV = 0;
  FTM->CONTROLS[6].CnV = 0;
  FTM->CONTROLS[7].CnV = 0;

  // ��������� ������
  FTM->SC = sc_reg | LSHIFT(1, 3); // CLKS  | 01 System clock;

}

/*-----------------------------------------------------------------------------------------------------
 ������������� FTM3 ��� ��������� 3-� ������� PWM ������� ���������� �������
 
 
 
 ������������ �������� ������������ �� ������� ���� = 60 ���
-----------------------------------------------------------------------------------------------------*/
void FTM3_init_motor_PWM(void)
{
  FTM_MemMapPtr FTM = FTM3_BASE_PTR;
  uint32_t sc_reg;
  uint8_t presc = FTM_PRESC_1;

  // ��������� ������������ FTM3
  SIM_SCGC3 |= BIT(25);

  FTM->OUTINIT = 0    // ��������� �������� ��� ������� �������
    + LSHIFT(0, 7)  // CH7OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 6)  // CH6OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 5)  // CH5OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 4)  // CH4OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 3)  // CH3OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 2)  // CH2OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 1)  // CH1OI | 0 The initialization value is 0. 1 The initialization value is 1.
    + LSHIFT(0, 0)  // CH0OI | 0 The initialization value is 0. 1 The initialization value is 1.
  ;

  // �������� ������ ���������������� ������ ������� � ��������� ������ �� ��� ���� ���������
  FTM->MODE = 0
    + LSHIFT(0, 7) // FAULTIE | Fault Interrupt Enable   |
    + LSHIFT(0, 5) // FAULTM  | Fault Control Mode       | 00 Fault control is disabled for all channels.
    + LSHIFT(0, 4) // CAPTEST | Capture Test Mode Enable
    + LSHIFT(0, 3) // PWMSYNC | PWM Synchronization Mode | 0 No restrictions. Software and hardware triggers can be used by MOD, CnV, OUTMASK, and FTM counter synchronization.
    + LSHIFT(1, 2) // WPDIS   | Write Protection Disable | 1 Write protection is disabled.
    + LSHIFT(1, 1) // INIT    | Initialize the Channels Output | When a 1 is written to INIT bit the channels output is initialized according to the state of their corresponding bit in the OUTINIT register
    + LSHIFT(1, 0) // FTMEN   | FTM Enable | 1 All registers including the FTM-specific registers (second set of registers) are available for use with no restrictions.
  ;

  FTM->CONF = 0
    + LSHIFT(0, 10) // GTBEOUT | Global time base output
    + LSHIFT(0, 9)  // GTBEEN  | Global time base enable
    + LSHIFT(3, 6)  // BDMMODE | Selects the FTM behavior in BDM mode | � ������ ������� ����������� ������ ������� FTM
    + LSHIFT(0, 0)  // NUMTOF  | TOF Frequency | NUMTOF = 0: The TOF bit is set for each counter overflow.
  ;

  // ������ ����������
  sc_reg = 0
    + LSHIFT(0, 7) // TOF   | 1 FTM counter has overflowed. | ����� � 0
    + LSHIFT(0, 6) // TOIE  | 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
    + LSHIFT(1, 5) // CPWMS | 1 FTM counter operates in up-down counting mode.0 FTM counter operates in Up Counting mode.
    + LSHIFT(0, 3) // CLKS  | 00 ������ ������� ����������
    + LSHIFT(presc, 0) // PS    | Prescale Factor Selection. 000 Divide by 1. ������������ = 1, 001 Divide by 2
  ;
  FTM->SC = sc_reg;

  // � ������ ������������� � TPM ��������� ������������� � ��������  FTM->SYNCONF �� ��������

  FTM->SYNCONF = 0
    + LSHIFT(0, 20) // HWSOC.      1 A hardware trigger activates the SWOCTRL register synchronization.
    + LSHIFT(0, 19) // HWINVC.     1 A hardware trigger activates the INVCTRL register synchronization.
    + LSHIFT(0, 18) // HWOM.       1 A hardware trigger activates the OUTMASK register synchronization.
    + LSHIFT(0, 17) // HWWRBUF.    1 A hardware trigger activates MOD, CNTIN, and CV registers synchronization.
    + LSHIFT(0, 16) // HWRSTCNT.   1 A hardware trigger activates the FTM counter synchronization.
    + LSHIFT(1, 12) // SWSOC.      1 The software trigger activates the SWOCTRL register synchronization.
    + LSHIFT(1, 11) // SWINVC.     1 The software trigger activates the INVCTRL register synchronization.
    + LSHIFT(1, 10) // SWOM.       1 The software trigger activates the OUTMASK register synchronization.
    + LSHIFT(1, 9) // SWWRBUF.    1 The software trigger activates MOD, CNTIN, and CV registers synchronization
    + LSHIFT(0, 8) // SWRSTCNT.   1 The software trigger activates the FTM counter synchronization. ���� 1 �� ���������� ������ ������ ��� ����� ����� 1 � SWSYNC
    + LSHIFT(1, 7) // SYNCMODE.   1 Enhanced PWM synchronization is selected.
    + LSHIFT(1, 5) // SWOC.       1 SWOCTRL ������� ����� �������� � ����� �������� ��� ������� ����� �������������, ���� 0 �� ����� �������� �����
    + LSHIFT(1, 4) // INVC.       1 INVCTRL ������� ����� �������� � ����� �������� ��� ������� ����� �������������, ���� 0 �� ����� �������� �����
    + LSHIFT(1, 2) // CNTINC.     1 CNTIN ������� ����� �������� � ����� �������� ��� ������� ����� �������������, ���� 0 �� ����� �������� �����
    + LSHIFT(0, 1) // HWTRIGMODE. 1 FTM does not clear the TRIGj bit when the hardware trigger j is detected.
  ;



  FTM->MOD     = FTM_MOTOR_MOD;   // ��������� �������� ������������.
  FTM->CNTIN   = 0;    // ��������� �������� ��������
  FTM->CNT     = 0;    // ������ � ������� ������� ������ �������� �������� � ������ �������� �� CNTIN � ��������� ���������� ��������� �������
  FTM->POL     = 0;    // ���������� � ��������

  FTM->OUTMASK = 0    // ���������� ������� ������� �� ������� ����������, �.�. ����� ���������� PWM �� �������� �������
    + LSHIFT(1, 7)  // CH7OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 6)  // CH6OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 5)  // CH5OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 4)  // CH4OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 3)  // CH3OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 2)  // CH2OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 1)  // CH1OM | 1 Channel output is masked. It is forced to its inactive state.
    + LSHIFT(1, 0)  // CH0OM | 1 Channel output is masked. It is forced to its inactive state.
  ;
  FTM->COMBINE = 0     // ��������� ������ ������� �� ����������
                       // ��������� ���  ������� 6 � 7
    + LSHIFT(0, 30) // FAULTEN3. 1 The fault control in this pair of channels is enabled.
    + LSHIFT(1, 29) // SYNCEN3.  1 The PWM synchronization in this pair of channels is enabled. ������ ���� ����������� ��������� ����� ����� ���� ������ ������ � �������� CnV
    + LSHIFT(1, 28) // DTEN3.    0 The deadtime insertion in this pair of channels is disabled.
    + LSHIFT(0, 27) // DECAP3.   0 The dual edge captures are inactive.
    + LSHIFT(0, 26) // DECAPEN3  0 The dual edge capture mode in this pair of channels is disabled.
    + LSHIFT(0, 25) // COMP3     1 The channel (7) output is the complement of the channel (6) output. �������� ������ ��� COMBINE3=1
    + LSHIFT(0, 24) // COMBINE3  1 Channels (6) and (7) are combined.
                    // ��������� ���  ������� 4 � 5
    + LSHIFT(0, 22) // FAULTEN2  1 The fault control in this pair of channels is enabled.
    + LSHIFT(1, 21) // SYNCEN2   1 The PWM synchronization in this pair of channels is enabled.
    + LSHIFT(1, 20) // DTEN2     0 The deadtime insertion in this pair of channels is disabled.
    + LSHIFT(0, 19) // DECAP2    0 The dual edge captures are inactive.
    + LSHIFT(0, 18) // DECAPEN2  0 The dual edge capture mode in this pair of channels is disabled.
    + LSHIFT(0, 17) // COMP2     1 The channel (5) output is the complement of the channel (4) output. �������� ������ ��� COMBINE2=1
    + LSHIFT(0, 16) // COMBINE2  1 Channels (4) and (5) are combined.
                    // ��������� ���  ������� 2 � 3
    + LSHIFT(0, 14) // FAULTEN1  1 The fault control in this pair of channels is enabled.
    + LSHIFT(1, 13) // SYNCEN1   1 The PWM synchronization in this pair of channels is enabled.
    + LSHIFT(1, 12) // DTEN1     0 The deadtime insertion in this pair of channels is disabled.
    + LSHIFT(0, 11) // DECAP1    0 The dual edge captures are inactive.
    + LSHIFT(0, 10) // DECAPEN1  0 The dual edge capture mode in this pair of channels is disabled.
    + LSHIFT(0,  9) // COMP1     1 The channel (3) output is the complement of the channel (2) output. �������� ������ ��� COMBINE1=1
    + LSHIFT(0,  8) // COMBINE1  1 Channels (2) and (3) are combined.
                    // ��������� ���  ������� 0 � 1
    + LSHIFT(0,  6) // FAULTEN0  1 The fault control in this pair of channels is enabled.
    + LSHIFT(1,  5) // SYNCEN0   1 The PWM synchronization in this pair of channels is enabled.
    + LSHIFT(1,  4) // DTEN0     0 The deadtime insertion in this pair of channels is disabled.
    + LSHIFT(0,  3) // DECAP0    0 The dual edge captures are inactive.
    + LSHIFT(0,  2) // DECAPEN0  0 The dual edge capture mode in this pair of channels is disabled.
    + LSHIFT(0,  1) // COMP0     1 The channel (1) output is the complement of the channel (0) output. �������� ������ ��� COMBINE0=1
    + LSHIFT(0,  0) // COMBINE0  1 Channels (0) and (1) are combined.
  ;
  FTM->DEADTIME = 0 // ������� �����
    + LSHIFT(0,  6)  // DTPS  | Deadtime Prescaler Value | 0x Divide the system clock by 1.
    + LSHIFT(15,  0) // DTVAL | Deadtime Value           | 0.25 ���
  ;
  FTM->INVCTRL = 0x07;  // ���������� �������� ������� ������� ��� �������
                        // �������������� ������������� �������� ������� ��������� ���� ���������� ���������� � ��������

  FTM->FILTER = 0;   // ������� ��� ������� ������� �� ����������


  // ��������� ������ ������� � ������ PWM ������������ �� �����
  // ��������� � 1 � ������ �������, ����� � 0 ����� �������� �����������

  FTM->CONTROLS[0].CnSC = 0 // ��������� ������ � 1 ��� ����������
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[1].CnSC = 0 // ��������� ������ � 0 ��� ����������
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[2].CnSC = 0 // ��������� ������ � 1 ��� ����������
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[3].CnSC = 0 // ��������� ������ � 0 ��� ����������
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[4].CnSC = 0 // ��������� ������ � 1 ��� ����������
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[5].CnSC = 0 // ��������� ������ � 0 ��� ����������
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[6].CnSC = 0 // ��������� ������ � 1 ��� ����������
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[7].CnSC = 0 // ��������� ������ � 0 ��� ����������
    + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
    + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
    + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
    + LSHIFT(0, 4) // MSA.  Channel Mode Select.
    + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - ��������� ������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - ��������� �������� ������  ��� ���������� ��� ������� ����
    + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;

  // �������� �� �������� ������������ �� ����������
  FTM->PWMLOAD = 0
    + LSHIFT(0, 9)  // LDOK   | �������������� �������� � �������� MOD, CNTIN, � CnV  ������ ���������� �������. ���� �����������. ��������� �� ���������� ������� �������������.
    + LSHIFT(0, 7)  // CH7SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 7
    + LSHIFT(0, 6)  // CH6SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 6
    + LSHIFT(0, 5)  // CH5SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 5
    + LSHIFT(0, 4)  // CH4SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 4
    + LSHIFT(0, 3)  // CH3SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 3
    + LSHIFT(0, 2)  // CH2SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 2
    + LSHIFT(0, 1)  // CH1SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 1
    + LSHIFT(0, 0)  // CH0SEL | �������������� �������� � �������� MOD, CNTIN, � CnV ��� �������� ����������� ������ 0
  ;
  // �������� ��������� �� ���������
  FTM->SWOCTRL = 0
    + LSHIFT(0, 15) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 14) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 13) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 12) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 11) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 10) // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 9)  // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 8)  // CH7OCV | 1 The software output control forces 1 to the channel output.
    + LSHIFT(0, 7)  // CH7OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 6)  // CH6OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 5)  // CH5OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 4)  // CH4OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 3)  // CH3OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 2)  // CH2OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 1)  // CH1OC  | 1 The channel output is affected by software output control.
    + LSHIFT(0, 0)  // CH0OC  | 1 The channel output is affected by software output control.
  ;

  // ���������� ������������ ��������� �������� ��������� �� ��������
  FTM->SYNC = 0
    + LSHIFT(1, 7)  // SWSYNC  | PWM Synchronization Software Trigger. The software trigger happens when a 1 is written to this bit. ���� �����������. ��������� �� ���������� ������� �������������.
    + LSHIFT(0, 6)  // TRIG2   | PWM Synchronization Hardware Trigger 2
    + LSHIFT(0, 5)  // TRIG1   | PWM Synchronization Hardware Trigger 1
    + LSHIFT(0, 4)  // TRIG0   | PWM Synchronization Hardware Trigger 0
    + LSHIFT(1, 3)  // SYNCHOM | 0 OUTMASK register is updated with the value of its buffer in all rising edges of the system clock. 1 OUTMASK register is updated with the value of its buffer only by the PWM synchronization.
    + LSHIFT(0, 2)  // REINIT  | FTM Counter Reinitialization by Synchronization | 0 FTM counter continues to count normally. (�� ����� �������� � ������ ������ �������)
    + LSHIFT(1, 1)  // CNTMAX  | 1 The maximum loading point is enabled. ���� ��  ���� ������ (CNTMAX, CNTMIN) ������ ���� ����������� ���������� ��� ������ ���������� ������ � ��������
    + LSHIFT(1, 0)  // CNTMIN  | 1 The minimum loading point is enabled.
  ;

  FTM->EXTTRIG = 0
    + LSHIFT(0, 7)  // TRIGF      | Channel Trigger Flag
    + LSHIFT(1, 6)  // INITTRIGEN | Initialization Trigger Enable
    + LSHIFT(0, 5)  // CH1TRIG    | Channel 1 Trigger Enable
    + LSHIFT(0, 4)  // CH0TRIG    | Channel 0 Trigger Enable
    + LSHIFT(0, 3)  // CH5TRIG    | Channel 5 Trigger Enable
    + LSHIFT(0, 2)  // CH4TRIG    | Channel 4 Trigger Enable
    + LSHIFT(0, 1)  // CH3TRIG    | Channel 3 Trigger Enable
    + LSHIFT(0, 0)  // CH2TRIG    | Channel 2 Trigger Enable
  ;


  FTM->CONTROLS[0].CnV = 0; // ������� �������� ������������ ��� ���� �������
  FTM->CONTROLS[1].CnV = 0;
  FTM->CONTROLS[2].CnV = 0;
  FTM->CONTROLS[3].CnV = 0;
  FTM->CONTROLS[4].CnV = 0;
  FTM->CONTROLS[5].CnV = 0;
  FTM->CONTROLS[6].CnV = 0;
  FTM->CONTROLS[7].CnV = 0;

  //  Install_and_enable_isr(INT_FTM3, FTM3_ISR_PRIO, FTM3_isr);
  //  ��������� ������
  FTM->SC = sc_reg | LSHIFT(1, 3); // CLKS  | 01 System clock;

}


/*-----------------------------------------------------------------------------------------------------
 
 \param n 
 \param val 
-----------------------------------------------------------------------------------------------------*/
void FTM3_set_CnV(uint32_t val)
{
  
  FTM3_C0V = val;
  FTM3_C1V = val;
  FTM3_C2V = val;
  FTM3_C3V = val;
  FTM3_C4V = val;
  FTM3_C5V = val;
  FTM3_SYNC |= BIT(7);
}
