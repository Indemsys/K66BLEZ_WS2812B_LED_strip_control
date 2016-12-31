#ifndef K66BLEZ1_FTM_H
  #define K66BLEZ1_FTM_H


#define FTM_PRESC_1    0
#define FTM_PRESC_2    1
#define FTM_PRESC_4    2
#define FTM_PRESC_8    3
#define FTM_PRESC_16   4
#define FTM_PRESC_32   5
#define FTM_PRESC_64   6
#define FTM_PRESC_128  7

#define FTM_CH_0    0
#define FTM_CH_1    1
#define FTM_CH_2    2
#define FTM_CH_3    3
#define FTM_CH_4    4
#define FTM_CH_5    5
#define FTM_CH_6    6
#define FTM_CH_7    7


// Устновка скважности ШИМ импульсов. 
// Значение меньше 3 не таймером отрабатыватся не успевают на частоте 60 МГц с предделителем установленным в 0 
#define FTM_WS2812B_MOD 75 // 1.25 мкс
#define FTM_WS2812B_1   48 // 0.8 мкс
#define FTM_WS2812B_0   24 // 0.4 мкс 



#define MOT_PWM_FREQ  (16000) // Частота ШИМ мотора
#define FTM_MOTOR_MOD (60000000ul/(MOT_PWM_FREQ*2))   // 


typedef  void (*T_qdec_isr)(uint32_t dir);

void    FTM1_init_QDEC(T_qdec_isr isr);
void    FTM_init_PWM_DMA(FTM_MemMapPtr FTM);
void    FTM3_init_motor_PWM(void);
void    FTM3_set_CnV(uint32_t val);
#endif // K66BLEZ1_FTM_H



