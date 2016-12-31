#define  ADC_GLOBAL
#include "App.h"


#define ADC_a_group  0
#define ADC_b_group  1

#define ADC_sw_trig  0 
#define ADC_hw_trig  1 

#define ADC0_config  0 
#define ADC1_config  1 

#define ADC_enable_int   1 
#define ADC_disable_int  0 

/*
  ������������� ADC ��� ������ ������ ��������� � ����������� PWM 
  ����������� ���������� ������� ������� � DMA
  ������������� �� 12-� ������ ��������������
 
  ������� �������������� ���������:
  - ���������������� ADC � ������ DMA � �������� �� ����������� �������
  - ���������������� DMA.
      DMA ���������� ��� ��������� ������. ������ ������ ���������� ��������� ADC � ������,
      ��������� ��������� ����� ��������� ������� ADC->SC1[0] � ��������� ��������� ��������������   
  - ���������������� FTM (��������� PWM) � ���������� �������� ������� ��������.
      FTM ��������������� � ���������� ������������ PWM. �� ��������� ��������� PWM ������������ ������ ������� ����������� ADC
  - ADC �� ������� FTM ��������� ������ �������������� � ���������� ����������.
  - � ADC ���������� ����������� ��������� ������� �������������� � ��������������� ������ ADC �� ������������ �������.
      � ��� �� ���������� ���������� ����������� ��������� �������������� ADC. 
  - ����� �������������� ADC ��������� DMA ������� ��������������� ��������� ��� ��� ���� ��������� ���������� �����������
      � ������ �������������� � ��������� ������� ADC �� �������� �������.
 
 
*/ 

// ������ �������� 
//   
//   UVDD        - ���������� ������� ������  | ADC0_SE11 (����� �������)
//   SNS_IA      - ��� ���� A                 | ADC0_SE16/ADC0_SE21
//   SNS_IB      - ��� ���� B                 | ADC1_SE16/ADC0_SE22
//   SNS_IC      - ��� ���� C                 | ADC1_SE4b  (����� �������)
//   U_A         - ���������� � ���� A        | ADC0_DM0/ADC1_DM3
//   U_B         - ���������� � ���� B        | ADC1_DP0/ADC0_DP3
//   U_C         - ���������� � ���� C        | ADC1_DM0/ADC0_DM3
//   TEMP        - ���������                  | ADC0_SE23  


// ������ ���� ��� ������������ �������� ADC->SC1[0]
#pragma data_alignment= 64 // ������������ �������������
// ������ ���������������� �������� ����������� � ������� ADC->SC1[0] ������� DMA ����� ������� ������� ���������� ��������������
// ������ ������� ������ ���� �������� ������ � ���������� � ���������� DEST_MODULO
const uint8_t  adc0_ch_cfg[ADC_SCAN_SZ+1] =
{
// --------------------------------------------------------------------------
//  ����� ADC0    ������                                     ���� ADC 
// --------------------------------------------------------------------------
  16,     //       SNS_IA      - ��� ���� A                 | ADC0_SE16
  19,     //       U_A         - ���������� � ���� A        | ADC0_DM0
  3 ,     //       U_B         - ���������� � ���� B        | ADC0_DP3
  11,     //       UVDD        - ���������� ������� ������  | ADC0_SE11
  23,     //       TEMP        - ���������                  | ADC0_SE23  
//  26,     //       Temperature Sensor (S.E)
//  29,     //       VREFH (S.E)
//  30,     //       VREFL
};
#pragma data_alignment= 64 // ������������ �������������
// ������ ���������������� �������� ����������� � ������� ADC->SC1[0] ������� DMA ����� ������� ������� ���������� ��������������
const uint8_t  adc1_ch_cfg[ADC_SCAN_SZ+1] =
{
// --------------------------------------------------------------------------
//  ����� ADC1    ������                                     ���� ADC 
// --------------------------------------------------------------------------
  16,     //       SNS_IB      - ��� ���� B                 | ADC1_SE16
  4 ,     //       SNS_IC      - ��� ���� C                 | ADC1_SE4b
  19,     //       U_C         - ���������� � ���� C        | ADC1_DM0
  26,     //       Temperature Sensor (S.E)
  26,     //       Temperature Sensor (S.E)
//  29,     //       VREFH (S.E)
//  30,     //       VREFL
//  27,     //       Bandgap (S.E)
};



T_DMA_TCD  sw_trig_tcd; 


#define ADC_DMA_DEST_ADDR_MODULO 4    // ���������� ������������ ��� ������ ���������� ��� DMA ���������. 
                                      // 4 ��� ������������� ���������  8 ������� �� 2 ����� ������

#pragma data_alignment= 64 // ������������
volatile uint16_t adc0_results[ADC_SCAN_SZ+1];

#pragma data_alignment= 64 // ������� ����������� ����������� �� ������� � ����� �������� ������ ������� ����
volatile uint16_t adc1_results[ADC_SCAN_SZ+1];

// ������ �������� ��� ������������� ������� DMA ������������� 2-� ������ ADC.
const T_init_ADC_DMA_cbl init_ADC_DMA_cbl[2] =
{
  { ADC0_BASE_PTR, adc0_results, adc0_ch_cfg, DMA_ADC0_RES_CH, DMA_ADC0_CFG_CH, DMA_ADC0_DMUX_SRC },
  { ADC1_BASE_PTR, adc1_results, adc1_ch_cfg, DMA_ADC1_RES_CH, DMA_ADC1_CFG_CH, DMA_ADC1_DMUX_SRC },
};

static T_ADC_state adc_state;


static void ADC_start_DMA(uint8_t n, uint8_t enint);

/*-----------------------------------------------------------------------------------------------------
  ���������� ����� ������� �������������� ADC
  ���������� ������ ����!!!
-----------------------------------------------------------------------------------------------------*/
static void ADC_Isr(void)
{
  // �������� ���������
  adcs.smpl_SNS_IA = ADC0_RA;
  adcs.smpl_SNS_IB = ADC1_RA;

  // ������������� ����������� ������� � ����� ADC
  ADC0_SC2 &= ~BIT(6);
  ADC1_SC2 &= ~BIT(6);

  // ��������� ���������� � ��������� ��������� ��������������
  ADC0_SC1A = adc0_ch_cfg[1];
  ADC1_SC1A = adc1_ch_cfg[1];

  // ����� �������� ������ DMA � ��������� ������������� �������������� ���� ��������� ������� 
}

/*-------------------------------------------------------------------------------------------------------------
  ���������� ����� ��������� ��������� ����� ������  DMA
  ���������� ������ ����!!!
-------------------------------------------------------------------------------------------------------------*/
static void DMA_ADC_Isr(void)
{
  DMA_INT = BIT(DMA_ADC1_RES_CH); // ���������� ���� ����������  ������ DMA 

  // �������� ���������� DMA � ������� ���������� adcs
  adcs.smpl_U_A     = adc0_results[1];
  adcs.smpl_U_B     = adc0_results[2];
  adcs.smpl_UVDD    = adc0_results[3];
  adcs.smpl_TEMP    = adc0_results[4];
//  adcs.smpl_Temper1 = adc0_results[5];
//  adcs.smpl_VREFH1  = adc0_results[6];
//  adcs.smpl_VREFL1  = adc0_results[7];

  adcs.smpl_SNS_IC  = adc1_results[1];
  adcs.smpl_U_C     = adc1_results[2];
  adcs.smpl_Temper2 = adc1_results[3];
  adcs.smpl_Temper3 = adc1_results[4];
//  adcs.smpl_VREFH2  = adc1_results[5];
//  adcs.smpl_VREFL2  = adc1_results[6];
//  adcs.smpl_Bandgap = adc1_results[7];


  // ���������������� � ������� �������������� ADC � ������� ���������� 
  // ������������� ���������� ������� � ����� ADC
  ADC0_SC2 |= BIT(6);
  ADC1_SC2 |= BIT(6);

  ADC0_SC1A = adc0_ch_cfg[0];
  ADC1_SC1A = BIT(6) + adc1_ch_cfg[0]; // ��������� ���������� �� ADC1  

  ADC_start_DMA(ADC1_config, ADC_enable_int);
  ADC_start_DMA(ADC0_config, ADC_disable_int);  // ��������� ���������� �� ����� ������ DMA

  // Inputs_set_ADC_evt(); // � ��������� ������ ���� ������� RTOS ������������ �� �����
}

/*-------------------------------------------------------------------------------------------------------------
  �������������� DMA ��� �������������� ������� �� �������� ADC �������
  �� ������� Document Number: AN4590 "Using DMA to Emulate ADC Flexible Scan Mode on Kinetis K Series"

  ����� ������������ �������� ���������� ������ ��������� ����� ������ ������ DMA �� ��������� ��������� ����� ������� ������ DMA


  n     -  ����� ���������������� ������ � ������� init_ADC_DMA_cbl
  enint - ���� ���������� ���������� �� ������ ������������
-------------------------------------------------------------------------------------------------------------*/
static void ADC_start_DMA(uint8_t n, uint8_t enint)
{
  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;


  ADC_MemMapPtr      ADC    = init_ADC_DMA_cbl[n].ADC;
  uint8_t            res_ch = init_ADC_DMA_cbl[n].results_channel; // ����� res_ch DMA (����� ������������) ��������� ������ �� ADC
  uint8_t            cfg_ch = init_ADC_DMA_cbl[n].config_channel;  // ����� cfg_ch DMA (����� ������������) ��������� ����� ��������� ������� ADC->SC1[0]


  // ������������� ����� ������������ ����� ��������� ������ �� ADC
  DMA->TCD[res_ch].SADDR = (uint32_t)&ADC->R[0];      // �������� - ������� ������ ADC
  DMA->TCD[res_ch].SOFF = 0;                        // ����� ��������� �� ���������� ����� ������
  DMA->TCD[res_ch].SLAST = 0;                       // �� ������������ ����� ��������� ����� ���������� ����� ����� DMA (��������� ��������� �����)
  DMA->TCD[res_ch].DADDR = (uint32_t)&init_ADC_DMA_cbl[n].results_vals[1]; // ����� ��������� ������ �� ADC - ������ ��������
  DMA->TCD[res_ch].DOFF = 2;                        // ����� ������ ������ ������� ��������� ��������� �� ��� �����
  DMA->TCD[res_ch].DLAST_SGA = 0;                   // ����� �� ������������ ������ ��������� ����� ��������� �����. ��� ��������� ���������� ���� DMOD � �������� ATTR
                                                    // DMA->TCD[res_ch].DLAST_SGA = (uint32_t)(-ADC_SCAN_SZ*2);  // ����� ������������ ������ ��������� ����� ��������� �����.
  DMA->TCD[res_ch].NBYTES_MLNO = 2;                 // ���������� ���� ������������ �� ���� ������ DMA (� �������� �����)
  DMA->TCD[res_ch].BITER_ELINKNO = 0
    + LSHIFT(1, 15)           // ELINK  | �������� �������� � ��������� ����� ������� ������
    + LSHIFT(cfg_ch, 9)       // LINKCH | ��������� � ������ cfg_ch
    + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | ���������� �������� � �������� �����
  ;
  DMA->TCD[res_ch].CITER_ELINKNO = 0
    + LSHIFT(1, 15)           // ELINK  | �������� �������� � ��������� ����� ������� ������
    + LSHIFT(cfg_ch, 9)       // LINKCH | ��������� � ������ cfg_ch
    + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | ���������� �������� � �������� �����
  ;
  DMA->TCD[res_ch].ATTR = 0
    + LSHIFT(0, 11) // SMOD  | ������ ������ ���������. �� ����������
    + LSHIFT(1, 8)  // SSIZE | 16-� ������ ��������� �� ���������
    + LSHIFT(0, 3)  // DMOD  | ������ ������ ���������. �� ����������
    + LSHIFT(1, 0)  // DSIZE | 16-� ������ ��������� � ��������
  ;
  DMA->TCD[res_ch].CSR = 0
    + LSHIFT(3, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
    + LSHIFT(0, 8)  // MAJORLINKCH | ���� ��������� �����. �������� ���� �� ����������� �� ��������� �������� ���������. ���� �����, �� ���� ���� ��������� � ��� ��������� �����
    + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
    + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
    + LSHIFT(0, 5)  // MAJORELINK  | ����� ���������� ��������� ����� �� �������� �� � ����
    + LSHIFT(0, 4)  // ESG         | Enable Scatter/Gather Processing
    + LSHIFT(1, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
    + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
    + LSHIFT(enint, 1)  // INTMAJOR    | ��������� ���������� ����� ��������� �����
    + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
  ;

  // ������������� ����� ������������ ������ ��������� ��������� � ADC
  DMA->TCD[cfg_ch].SADDR = (uint32_t)&init_ADC_DMA_cbl[n].config_vals[2];     // �������� - ������ �������� ��� ADC
  DMA->TCD[cfg_ch].SOFF = 1;                        // ����� ��������� ����� ��������� ��������� �� 1 ���� ������
  DMA->TCD[cfg_ch].SLAST = 0;                       // ������������ ����� ��������� ����� ���������� ����� ����� DMA (��������� ��������� �����)
  DMA->TCD[cfg_ch].DADDR = (uint32_t)&ADC->SC1[0];  // ����� ��������� ������ - ������� ���������� ADC
  DMA->TCD[cfg_ch].DOFF = 0;                        // ��������� ��������� �������� ����� ������
  DMA->TCD[cfg_ch].DLAST_SGA = 0;                   // ��������� ������ ��������� �� ���������� ����� ��������� ���� ������� ������������ (��������� ��������� �����)
  DMA->TCD[cfg_ch].NBYTES_MLNO = 1;                 // ���������� ���� ������������ �� ���� ������ DMA (� �������� �����)
  DMA->TCD[cfg_ch].BITER_ELINKNO = 0
    + LSHIFT(0, 15)           // ELINK  | �������� ���������
    + LSHIFT(0, 9)            // LINKCH |
    + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | ���������� �������� � �������� �����
  ;
  DMA->TCD[cfg_ch].CITER_ELINKNO = 0
    + LSHIFT(0, 15)           // ELINK  | �������� ���������
    + LSHIFT(0, 9)            // LINKCH |
    + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | ���������� �������� � �������� �����
  ;
  DMA->TCD[cfg_ch].ATTR = 0
    + LSHIFT(0, 11) // SMOD  | ������ ������ ��������� �� ����������
    + LSHIFT(0, 8)  // SSIZE | 8-� ������ ��������� �� ���������
    + LSHIFT(0, 3)  // DMOD  | ������ ������ ��������� �� ����������
    + LSHIFT(0, 0)  // DSIZE | 8-� ������ ��������� � ��������
  ;
  DMA->TCD[cfg_ch].CSR = 0
    + LSHIFT(3, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
    + LSHIFT(0, 8)  // MAJORLINKCH | Link Channel Number
    + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
    + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
    + LSHIFT(0, 5)  // MAJORELINK  | ��������� � ������ 0  ����� ���������� ��������� �����
    + LSHIFT(0, 4)  // ESG         | Enable Scatter/Gather Processing
    + LSHIFT(0, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
    + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
    + LSHIFT(0, 1)  // INTMAJOR    | ���������� �� ����������
    + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
  ;

  DMA->SERQ = res_ch;                                           // ��������� ������� �� ������� ��������� ��� ������ DMA � ������� res_ch
}

/*-----------------------------------------------------------------------------------------------------
 
 \param n  - ����� ���������������� ������ � ������� init_ADC_DMA_cbl
-----------------------------------------------------------------------------------------------------*/
void ADC_config_DMA_MUX(uint8_t n)
{
  DMAMUX_MemMapPtr DMAMUX  = DMA_ADC_DMUX_PTR;
  uint8_t            res_ch = init_ADC_DMA_cbl[n].results_channel; // ����� res_ch DMA (����� ������������) ��������� ������ �� ADC
  DMAMUX->CHCFG[res_ch] = init_ADC_DMA_cbl[n].req_src + BIT(7);    // ����� ������������� ��������� ������ �� ������� ��������� (����� �� ������ ADC) � ������ ���������� ������ DMA
                                                                   // BIT(7) �������� DMA Channel Enable
}

/*-------------------------------------------------------------------------------------------------------------
  �������������� DMA ��� �������������� ������� �� �������� ADC �������
-------------------------------------------------------------------------------------------------------------*/
void ADC_config_start_DMA(void)
{

  Install_and_enable_kernel_isr(DMA_ADC_INT_NUM, DMA_ADC_PRIO, DMA_ADC_Isr);

  ADC_config_DMA_MUX(ADC1_config);
  ADC_config_DMA_MUX(ADC0_config);


  ADC_start_DMA(ADC0_config, ADC_disable_int);  
  ADC_start_DMA(ADC1_config, ADC_enable_int); // ��������� ���������� �� ����� ������ DMA
}


/*-------------------------------------------------------------------------------------------------------------
 �������� ������������ ���� ADC
-------------------------------------------------------------------------------------------------------------*/
void ADC_switch_on_all(void)
{
  SIM_SCGC3 |= BIT(27); // ������������ ADC1
  SIM_SCGC6 |= BIT(27); // ������������ ADC0
}



/*-------------------------------------------------------------------------------------------------------------

 


  ab - ����� ����� �������� � �������� a ��� b. 0 - a, 1 - b
  hw_trig - ���� ��������� ����������� ������� 
-------------------------------------------------------------------------------------------------------------*/
static void ADC_config(ADC_MemMapPtr ADC, uint32_t ab, uint32_t hw_trig)
{
  // ����� ������� ������������, ����������� ����������
  // ������� ������������ ��� 16-� ������� ���������� �� ������ ���� ������ 12 ���, ��� 13-� ������� 18 ���
  // ������������� 15 ���
  ADC->CFG1 = 0
              + LSHIFT(0, 7) // ADLPC.  Low power configuration. The power is reduced at the expense of maximum clock speed. 0 Normal power configuration
              + LSHIFT(2, 5) // ADIV.   Clock divide select. 00 The divide ratio is 1 and the clock rate is input clock.
                             // 10 - The divide ratio is 4 and the clock rate is (input clock)/4. = 15 Mhz
              + LSHIFT(0, 4) // ADLSMP. Sample time configuration. 0 Short sample time.
              + LSHIFT(1, 2) // MODE.   Conversion mode selection. 01 When DIFF=0: It is single-ended 12-bit conversion; when DIFF=1, it is differential 13-bit conversion with 2's complement output.
              + LSHIFT(0, 0) // ADICLK. Input clock select. 00 Bus clock. 01 Bus clock divided by 2(BUSCLK/2)
  ;

  ADC->CFG2 = 0
              + LSHIFT(ab & 1, 4) // MUXSEL.  0 ADxxa channels are selected. ����� ����� �������� � �������� a ��� b
              + LSHIFT(0, 3) // ADACKEN. Asynchronous clock output enable
              + LSHIFT(0, 2) // ADHSC.   High speed configuration. 0 Normal conversion sequence selected.
              + LSHIFT(0, 0) // ADLSTS.  Default longest sample time (20 extra ADCK cycles; 24 ADCK cycles total). If ADLSMP = 1
  ;

  // ������� ������� � ����������. ����� ���� �������, ���������� �������� ���������, ���������� DMA, ����� ���� �����
  // ����� ��������� �������  ������� ��� ADC ������������ � �������� SIM_SOPT7.
  // ����������� ������� �������� ����� ���� PDB 0-3, High speed comparator 0-3, PIT 0-3, FTM 0-3, RTC, LP Timer,

  ADC->SC2 = 0
             + LSHIFT(0, 7) // ADACT.   Read only. 1 Conversion in progress.
             + LSHIFT(hw_trig, 6) // ADTRG.   Conversion trigger select. 0 Software trigger selected. 1 Hardware trigger selected.
             + LSHIFT(0, 5) // ACFE.    Compare function enable. 0 Compare function disabled.
             + LSHIFT(0, 4) // ACFGT.   Compare function greater than enable
             + LSHIFT(0, 3) // ACREN.   Compare function range enable
             + LSHIFT(1, 2) // DMAEN.   DMA enable
             + LSHIFT(0, 0) // REFSEL.  Voltage reference selection. 00 Default voltage reference pin pair (external pins VREFH and VREFL)
  ;
  // ������� ������� � ����������. ���������� � ������ ����������, ���������� �����������, ���������� ����������� ���������������
  ADC->SC3 = 0
             + LSHIFT(0, 7) // CAL.     CAL begins the calibration sequence when set.
             + LSHIFT(0, 3) // ADCO.    Continuous conversion enable
             + LSHIFT(0, 2) // AVGE.
             + LSHIFT(0, 0) // AVGS.
  ;


  // ����������� ������� B
  // ��������� ����������� ���� ��������� ����� ���������� � �������� ADC->RB
  // ������������� ������ ��� ���������� ���������� ��������
  ADC->SC1[1] = 0
                + LSHIFT(0, 7) // COCO. Read only. 1 Conversion completed.
                + LSHIFT(0, 6) // AIEN. 1 Conversion complete interrupt enabled.
                + LSHIFT(0, 5) // DIFF. 1 Differential conversions and input channels are selected.
                + LSHIFT(0, 0) // ADCH. Input channel select. 11111 Module disabled.
                               //                             00000 When DIFF=0, DADP0 is selected as input; when DIFF=1, DAD0 is selected as input.
  ;
  // ����������� ������� A
  // ��������� ����������� ���� ��������� ����� ���������� � �������� ADC0->RA
  // ������ � ������ ������ ���� ��������� ������� �������������� ���� ���������� ����� (���� ���������� ���� ������������ ������� - ADTRG = 0)
  ADC->SC1[0] = 0
                + LSHIFT(0, 7) // COCO. Read only. 1 Conversion completed.
                + LSHIFT(0, 6) // AIEN. 1 Conversion complete interrupt enabled.
                + LSHIFT(0, 5) // DIFF. 1 Differential conversions and input channels are selected.
                + LSHIFT(0, 0) // ADCH. Input channel select. 11111 Module disabled.
                               //                             00000 When DIFF=0, DADP0 is selected as input; when DIFF=1, DAD0 is selected as input.
  ;

}

/*-------------------------------------------------------------------------------------------------------------


   ������������� ADICLK = 1, �.�. ������������ ��  Bus clock �� �� Peripheral Clock ���������� �� 2 = 30 Mhz

   �� ����� ���������� ������������� ���������� ������ ������������ ������� ��� ������ 4 ���
    � ���������� ������������ ������� ����������
-------------------------------------------------------------------------------------------------------------*/
int32_t ADC_calibrating(ADC_MemMapPtr ADC)
{

  unsigned short tmp;

  ADC->SC1[0] = 0
                + LSHIFT(0, 7) // COCO. Read only. 1 Conversion completed.
                + LSHIFT(0, 6) // AIEN. 1 Conversion complete interrupt enabled.
                + LSHIFT(1, 5) // DIFF. 1 Differential conversions and input channels are selected.
                + LSHIFT(0, 0) // ADCH. Input channel select. 11111 Module disabled.
                               //                             00000 When DIFF=0, DADP0 is selected as input; when DIFF=1, DAD0 is selected as input.
  ;
  // ������������� ������� ADC 4.6875 MHz
  ADC->CFG1 = 0
              + LSHIFT(0, 7) // ADLPC.  Low power configuration. The power is reduced at the expense of maximum clock speed. 0 Normal power configuration
              + LSHIFT(3, 5) // ADIV.   Clock divide select. 11 - The divide ratio is 8 and the clock rate is (input clock)/8. = 4.6875 MHz
              + LSHIFT(0, 4) // ADLSMP. Sample time configuration. 0 Short sample time.
              + LSHIFT(1, 2) // MODE.   Conversion mode selection. 01 When DIFF=0: It is single-ended 12-bit conversion; when DIFF=1, it is differential 13-bit conversion with 2's complement output.
              + LSHIFT(1, 0) // ADICLK. Input clock select. 01 Bus clock divided by 2 = 30 MHz
  ;
  ADC->CFG2 = 0
              + LSHIFT(0, 4) // MUXSEL.  0 ADxxa channels are selected.
              + LSHIFT(0, 3) // ADACKEN. Asynchronous clock output enable
              + LSHIFT(0, 2) // ADHSC.   High speed configuration. 0 Normal conversion sequence selected.
              + LSHIFT(0, 0) // ADLSTS.  Long sample time select
  ;
  // ������� ������� � ����������. ����� ���� �������, ���������� �������� ���������, ���������� DMA, ����� ���� �����
  ADC->SC2 = 0
             + LSHIFT(0, 7) // ADACT.   Read only. 1 Conversion in progress.
             + LSHIFT(0, 6) // ADTRG.   Conversion trigger select. 0 Software trigger selected.
             + LSHIFT(0, 5) // ACFE.    Compare function enable. 0 Compare function disabled.
             + LSHIFT(0, 4) // ACFGT.   Compare function greater than enable
             + LSHIFT(0, 3) // ACREN.   Compare function range enable
             + LSHIFT(0, 2) // DMAEN.   DMA enable
             + LSHIFT(0, 0) // REFSEL.  Voltage reference selection. 00 Default voltage reference pin pair (external pins VREFH and VREFL)
  ;
  ADC->SC3 = 0
             + LSHIFT(1, 7) // CAL.     CAL begins the calibration sequence when set.
             + LSHIFT(1, 6) // CALF.    Read Only. 1 Calibration failed.
             + LSHIFT(0, 3) // ADCO.    Continuous conversion enable
             + LSHIFT(1, 2) // AVGE.    1 Hardware average function enabled.
             + LSHIFT(3, 0) // AVGS.    11 32 samples averaged.
  ;

  // ������� ���������� ����������
  while (ADC->SC3 & BIT(7));

  tmp = ADC->CLP0;
  tmp += ADC->CLP1;
  tmp += ADC->CLP2;
  tmp += ADC->CLP3;
  tmp += ADC->CLP4;
  tmp += ADC->CLPS;
  tmp /= 2;
  ADC->PG = tmp | 0x8000;

  tmp =  ADC->CLM0;
  tmp += ADC->CLM1;
  tmp += ADC->CLM2;
  tmp += ADC->CLM3;
  tmp += ADC->CLM4;
  tmp += ADC->CLMS;
  tmp /= 2;
  ADC->MG = tmp | 0x8000;

  if (ADC->SC3 & BIT(6))
  {
    return MQX_ERROR;
  }
  else
  {
    return MQX_OK;
  }
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_ADC_state* ADC_get_state(void)
{
  return &adc_state;
}


/*-------------------------------------------------------------------------------------------------------------
  ���������� ����������  ADC � ������.
  ���������� � ����� ������ ���� ����������� ADC
-------------------------------------------------------------------------------------------------------------*/
void ADC_calibr_config_start(void)
{
  SIM_MemMapPtr  SIM   = SIM_BASE_PTR;

  ADC_switch_on_all();
  adc_state.adc0_cal_res = ADC_calibrating(ADC0_BASE_PTR);  // �������� ��������� ���������� ������� ADC
  adc_state.adc1_cal_res = ADC_calibrating(ADC1_BASE_PTR);  // �������� ��������� ���������� ������� ADC

  ADC_config(ADC0_BASE_PTR, ADC_b_group, ADC_hw_trig);  // 
  ADC_config(ADC1_BASE_PTR, ADC_b_group, ADC_hw_trig);  // 

  // ������������� ������ ������� ���������� ������ ��������������� ����� ����������� ��������
  ADC0_SC1A = adc0_ch_cfg[0];
  ADC1_SC1A = BIT(6) + adc1_ch_cfg[0]; // ��������� ���������� �� ADC1  
  Install_and_enable_kernel_isr(INT_ADC1, ADC_PRIO, ADC_Isr); // ������������ ���������� ADC1
 
  ADC_config_start_DMA();

  // �������� ��������� �������� ���������� ��������� ��� ADC. �������� FTM3 ������
  // ������ �� FTM3 ����� ���� �������� ������ ������ �� ���� ����������� ��������� ADC
  // (������� �� ������ PDB ����� ��������������� ������������ ��� ����������� �������� ADC)
  SIM->SOPT7 = 0
               + LSHIFT(1, 15) // ADC1ALTTRGEN  | ADC alternate trigger enable | 1 Alternate trigger selected for ADC1.
               + LSHIFT(0, 12) // ADC1PRETRGSEL | ADC pre-trigger select       | 0 Pre-trigger A selected for ADC1. ����� ������������ �������� ADC1 (A ��� B), ������� ����� �������������� ������
               + LSHIFT(11, 8) // ADC1TRGSEL    | ADC trigger select           | 0100 PIT trigger 0, 1011 FTM3 trigger
               + LSHIFT(1,  7) // ADC0ALTTRGEN  | ADC alternate trigger enable | 1 Alternate trigger selected for ADC0.
               + LSHIFT(0,  4) // ADC0PRETRGSEL | ADC pre-trigger select       | 0 Pre-trigger A selected for ADC0. ����� ������������ �������� ADC0 (A ��� B), ������� ����� �������������� ������
               + LSHIFT(11, 0) // ADC0TRGSEL    | ADC trigger select           | 0100 PIT trigger 0, 1011 FTM3 trigger
  ;


}


/*-------------------------------------------------------------------------------------------------------------
  �������� ������ �������� �� ������ �����
-------------------------------------------------------------------------------------------------------------*/
void Get_ADC_samples(T_ADC_res **pp_adc_res)
{
  *pp_adc_res = &adcs;
}




