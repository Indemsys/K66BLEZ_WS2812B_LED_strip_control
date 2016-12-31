#ifndef __K66BLEZ_ADC
  #define __K66BLEZ_ADC



#define ADC_AVER_4   1
#define ADC_AVER_8   2
#define ADC_AVER_16  3
#define ADC_AVER_32  4



typedef struct
{
  ADC_MemMapPtr ADC;                           //  ��������� �� ������ ADC
  unsigned short volatile     *results_vals;   //  ��������� �� ������ � ������������ ADC
  unsigned char const         *config_vals;    //  ��������� �� ������ �������� ���������� ������������ ADC
  uint8_t                       results_channel; //  ����� ������ DMA ������������ �������� ADC.     ����� ������������. �� ��������� ����� � ������� ������� ����� ������� ���������.
  uint8_t                       config_channel;  //  ����� ������ DMA ������������ ������������ ADC. ����� ������������
  uint8_t                       req_src;         //  ����� ��������� �������� ��� ���������������� �������������� DMAMUX
} T_init_ADC_DMA_cbl;


typedef struct
{
  // ���������� ���������� ������� ADC
  int32_t adc0_cal_res;
  int32_t adc1_cal_res;
} T_ADC_state;


typedef struct
{
  uint16_t smpl_SNS_IA   ; 
  uint16_t smpl_U_A      ; 
  uint16_t smpl_U_B      ; 
  uint16_t smpl_UVDD     ; 
  uint16_t smpl_TEMP     ; 
  uint16_t smpl_Temper1  ; 
  uint16_t smpl_VREFH1   ; 
  uint16_t smpl_VREFL1   ; 
                         
  uint16_t smpl_SNS_IB   ; 
  uint16_t smpl_SNS_IC   ; 
  uint16_t smpl_U_C      ; 
  uint16_t smpl_Temper2  ; 
  uint16_t smpl_Temper3  ; 
  uint16_t smpl_VREFH2   ; 
  uint16_t smpl_VREFL2   ;
  uint16_t smpl_Bandgap  ;
}
T_ADC_res;



typedef struct
{
  const char   *name;
  float        (*int_converter)(int);
  float        (*flt_converter)(float);

} T_vals_scaling;

void ADC_config_start_DMA(void);

void ADC_calibr_config_start(void);
void ADC_switch_on_all(void);
int  ADC_calibrating(ADC_MemMapPtr ADC);


T_ADC_state *ADC_get_state(void);

void Get_ADC_samples(T_ADC_res **pp_adc_res);
void Copy_to_ADC_res(uint8_t n, uint16_t *buf);

#endif
