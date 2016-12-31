#include   "ctype.h"
#include   "math.h"
#include   "limits.h"
#include   "arm_itm.h"
#include   "mqx.h"
#include   "bsp.h"
#include   "message.h"
#include   "timer.h"

#include   "K66BLEZ1_PERIPHERIAL.h"
#include   "bsp_prv.h"
#include   "message.h"
#include   "mutex.h"
#include   "sem.h"
#include   "lwmsgq.h"
#include   "lwevent_prv.h"
#include   "mfs.h"
#include   "part_mgr.h"
#include   "SEGGER_RTT.h"
#include   "Debug_io.h"
#include   "Inputs_processing.h"
#include   "Supervisor_task.h"
#include   "WatchDogCtrl.h"
#include   "RTOS_utils.h"
#include   "MonitorVT100.h"
#include   "Monitor_serial_drv.h"
#include   "Monitor_usb_drv.h"
#include   "App_logger.h"
#include   "MFS_man.h"
#include   "MFS_Shell.h"
#include   "LED_control.h"
#include   "USB_Virtual_com.h"
#include   "Task_FreeMaster.h"
#include   "CRC_utils.h"
#include   "CAN_IO_exchange.h"
#include   "QuadDecoder.h"
#include   "MKW40_Channel.h"

#ifdef LEDSC_APP
#include   "LEDSC_Params.h"
#include   "LEDSC_app/LEDSC.h"
#endif


#include   "Parameters.h"


#define PARAMS_FILE_NAME DISK_NAME"Params.ini"


//#define USB_VT100  // ���������� ���� �������� �������� ����� USB

//#define ENABLE_CAN_LOG  ���������� ���� ����� LOG CAN
//#define ENABLE_ACCESS_CONTROL  ���������� ���� ����� �������� �������



//#define DEBUG_USB_APP

#ifdef DEBUG_USB_APP
  #define DEBUG_PRINT_USP_APP RTT_terminal_printf
#else
  #define DEBUG_PRINT_USP_APP
#endif



#define ADC_PERIOD          500  // ������ ������� �����-���������� ��������������� (ADC) � ���



#define BIT(n) (1u << n)
#define LSHIFT(v,n) (((unsigned int)(v) << n))

#define MQX_MFS   // ��������� ���� ������������ MQX MFS
#define MQX_SHELL // ���������� ���� ������������ MQX SHELL � ��������� VT100
#define MFS_TEST  // ���������� ���� ������������� ��������� ������������ �������� ������� MFS


#define MAIN_TASK_IDX           1
#define CAN_TX_IDX              2
#define CAN_RX_IDX              3
#define MKW40_IDX               4
#define VT100_IDX               5
#define USB_IDX                 6
#define SUPERVISOR_IDX          7
#define SHELL_IDX               8
#define FILELOG_IDX             9
#define BACKGR_IDX              10


// ��������� ����������� �����
// ��� ������ ����� ��� ������ ���������.
#define MAIN_TASK_PRIO          8
#define CAN_RX_ID_PRIO          9
#define CAN_TX_ID_PRIO          9
#define USB_TASK_PRIO           9
#define MKW_ID_PRIO             9
#define SUPRVIS_ID_PRIO         10
#define VT100_ID_PRIO           11
#define SHELL_ID_PRIO           12
#define FILELOG_ID_PRIO         13 // ��������� ������ ������ ���� � ����
#define TIMERS_ID_PRIO          7
#define BACKGR_ID_PRIO          100


#define MAX_MQX_PRIO        BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX

// ���������� ��� ISR
// ��� ������ ����� ��� ���� ��������� ISR
// ISR � ����� ������� ����������� ��������� ISR � ����� ������ �����������
// ��������� ISR ������������ ������� RTOS �� ����� ���� ������ �������� BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX!!!
#define WDT_ISR_PRIO        MAX_MQX_PRIO      //
#define FTM3_ISR_PRIO       MAX_MQX_PRIO      // ��������� ���������� ���������� PWM ������ 
#define FTM1_ISR_PRIO       MAX_MQX_PRIO + 1  // ��������� ���������� ������������� ��������
#define CAN_ISR_PRIO        MAX_MQX_PRIO + 1  // ��������� ��������� ���������� �� ����������� CAN ����

#define SPI0_PRIO           MAX_MQX_PRIO + 1
#define SPI1_PRIO           MAX_MQX_PRIO + 1
#define SPI2_PRIO           MAX_MQX_PRIO + 1

// ��������� ���������� ������ ����
// ��� ������ ����� ��� ���� ��������� ISR
#define DMA_ADC_PRIO        MAX_MQX_PRIO-1     // �������� ���������� DMA ����� ��������� ���� ������� ADC
#define ADC_PRIO            MAX_MQX_PRIO-1     // �������� ���������� ADC




#define REF_TIME_INTERVAL   5  // �������� ������� � ������������ �� ������� ������������� ���������� � ��������� ������������� ����������

#define CAN_TX_QUEUE        8  // ���������� ����� ������� �� ������ ����������. ������ ������������� ������ 8-�. ����������� � �������� � ����������



//#define dbg_printf RTT_printf
#define dbg_printf printf

// ����������� ��� �������� �������
#define    PARTMAN_NAME   "pm:"
#define    DISK_NAME      "a:"
#define    PARTITION_NAME "pm:1"

// ����������� ������� ������� ��� ������� ����������
#define   SEVERITY_RED             0
#define   SEVERITY_GREEN           1
#define   SEVERITY_YELLOW          2
#define   SEVERITY_DEFAULT         4



#ifdef _MAIN_GLOBALS_

WVAR_TYPE                  wvar;

uint32_t                   ref_time;             // ������������� ��������� ��������������� ��� ��������� �������� ���������������
volatile uint32_t          cpu_usage;            // ������� �������� ����������

#else

extern WVAR_TYPE           wvar;

extern uint32_t            ref_time;
extern volatile uint32_t   cpu_usage;


#endif

#define  DELAY_1us       Delay_m7(25)           // 1.011     ��� ��� ������� 180 ���
#define  DELAY_4us       Delay_m7(102)          // 4.005     ��� ��� ������� 180 ���
#define  DELAY_8us       Delay_m7(205)          // 8.011     ��� ��� ������� 180 ���
#define  DELAY_32us      Delay_m7(822)          // 32.005    ��� ��� ������� 180 ���
#define  DELAY_ms(x)     Delay_m7(25714*x-1)    // 999.95*N  ��� ��� ������� 180 ���

extern void Delay_m7(int cnt); // �������� �� (cnt+1)*7 ������ . �������� ���� �������������


_mqx_int  TimeManInit(void);
void      Get_time_counters(HWTIMER_TIME_STRUCT *t);
uint32_t  Eval_meas_time(HWTIMER_TIME_STRUCT t1, HWTIMER_TIME_STRUCT t2);
uint32_t  Get_usage_time(void);
