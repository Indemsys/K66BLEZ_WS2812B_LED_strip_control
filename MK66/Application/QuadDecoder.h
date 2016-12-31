#ifndef __QUADDECODER
  #define __QUADDECODER






typedef struct
{
  uint32_t           enabled;
  uint32_t           smpl_cnt;
  uint32_t           min_t;
  uint32_t           max_t;
  MQX_TICK_STRUCT    ticktime_prev; // ����� ������� ��� ���������� �� ����������� ����������
  MQX_TICK_STRUCT    ticktime;      // ����� ������� ��� ���������� �� ����������� ����������

} T_qdec_diagnostic;


typedef struct
{
  volatile int32_t    tacho_counter;      // ������� ������������� ��������. �������� ������������ ������� ��������� � �������������
  volatile int32_t    dir_sign;           // ����������� ���������
  volatile uint32_t   idle_cnt;           // ������� �������� ����� ��������������
  volatile int32_t    puls_len;           // �������� ������� � ������������� ����� ��������������
  volatile int32_t    rpm;                // �������� �������� � �������� � ������
} T_qdec_cbl;

typedef void (*T_app_qdec_isr)(int32_t pulse_cnt);


void                Init_QuadratureDecoder(void);
void                QDEC_increment_idle_cnt(void);
void                QDEC_set_positioning_calback(T_app_qdec_isr isr);

T_qdec_cbl*         QDEC_get_cbl(void);
T_qdec_diagnostic*  QDEC_get_diagnostic(void);

void                QDEC_get_ticks(MQX_TICK_STRUCT *tickt_prev, MQX_TICK_STRUCT *tickt);
void                QDEC_diagnostic_restart(void);


int32_t             Get_speed(void);
int32_t             QDEC_get_counter(void);
void                QDEC_set_couter(int32_t pos);
int32_t             QDEC_get_dir_sign(void);
void                QDEC_reset_counter(void);
int32_t             QDEC_get_tacho_interval(void);

#endif
