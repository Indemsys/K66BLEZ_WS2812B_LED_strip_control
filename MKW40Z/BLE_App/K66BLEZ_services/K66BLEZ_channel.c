// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.09.19
// 15:58:24
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "K66BLEZ.h"


#define MKW_BUF_SZ   22 // ����������� ����� ������
#define MKW_DATA_SZ  20 // ���������� �������� ������ � ������

#define MKW_QUEUE_SZ 4  // ���������� �������� �� �������� � �������

// ��������� � ���������� ������ ��������������� � ������ �������
// �������������� ������ �������� MKW_BUF_SZ ������ 2 ��
// ��������� ������:
// ����� �����   ����������
// � ������
// 0     [llid]  - ������������� ����������� ������
// 1     [LEN]   - ����������� ������������ ������
// 2     [data0] - ������ ���� ������
// ...
// LEN+2 [dataN] - ��������� ���� ������


// ������� ������� �� ��������
volatile uint32_t  mkw40_queue_head;
volatile uint32_t  mkw40_queue_tail;
uint8_t   mkw_buf_queue[MKW_QUEUE_SZ][MKW_BUF_SZ];


#define HOST_CHAN_TASK_STACK_SZ  1000
#define HOST_CHAN_TASK_PRIO      6
OSA_TASK_DEFINE(HOSTCHAN, HOST_CHAN_TASK_STACK_SZ);

static void Host_channel_receiver(void);
static void Host_channel_pop(void);
/*------------------------------------------------------------------------------



 \param argument
 ------------------------------------------------------------------------------*/
static void Host_Chan_Task(task_param_t argument)
{
  static uint8_t div = 0;

  while (1)
  {
    Host_channel_pop();       // ��������� ����� �������� ��  SPI
    BSP_SPI_DMA_read_write(); // ���������� � ������ ������ �� SPI

    // ����������� �������� ������ � ������������ �� �� ����������
    Host_channel_receiver();

    if (div == 1) // �������� ��� ��������� � ������������ �������� �������
    {
      PerfomanceTest_read_notification(); // ��������� �������� ������� �� ����� ������������ ������������������� ������
                                          // �� ����� ���������� ����� ������ ���� � 4 ��


      div = 0;
    }
    else
    {
      div++;
    }
    vTaskDelay(1);
  };
}


/*------------------------------------------------------------------------------



 \param void
 ------------------------------------------------------------------------------*/
static void Host_channel_receiver(void)
{
  volatile uint8_t dummy;
  uint8_t *data;
  data = BSP_SPI_DMA_Get_rx_buf();


  switch (data[0])
  {
  case MKW40_SUBS_CMDMAN:
    CommandService_receive_from_host(&data[2], data[1]);
    break;
  default:
    {
      uint32_t i;
      for (i = 0; i < 22; i++)
      {
        if (data[i]!=0)
        {
          dummy = data[i];
          break;
        }
      }
    }
    break;
  }

}
/*------------------------------------------------------------------------------
  ������������� ������ ����������������� ������ � ���� �����������������

 \param void
 ------------------------------------------------------------------------------*/
void Init_host_channel_task(void)
{
  task_handler_t  host_chan_task_id;

  OSA_TaskCreate(Host_Chan_Task, "Host_Chan", HOST_CHAN_TASK_STACK_SZ, HOSTCHAN_stack, HOST_CHAN_TASK_PRIO, (task_param_t)NULL, FALSE, &host_chan_task_id);

}

/*------------------------------------------------------------------------------
 ��������� ������ �� ������� � �������� �������� SPI


 \param void
 ------------------------------------------------------------------------------*/
static void Host_channel_pop(void)
{
  uint32_t ptail;
  uint32_t tail;
  uint8_t *data;
  uint8_t  sz;

  data = BSP_SPI_DMA_Get_tx_buf(); // ������� ��������� �� ����� �������� ������ SPI
  memset(data, 0, MKW_BUF_SZ);     // �������������� ����� �������� ��������� ������

  ptail = mkw40_queue_tail;
  if (ptail == mkw40_queue_head) return; // ������� ������ ���
  tail = ptail + 1;
  if (tail >= MKW_QUEUE_SZ) tail = 0;

  sz = mkw_buf_queue[ptail][1] + 2;       // ������� ������ ������ �� ������ ���������� � ��������� ������
  memcpy(data, mkw_buf_queue[ptail], sz); // ��������� ����� �������� �� ������ �������

  mkw40_queue_tail = tail; // ������� ��������� ������
}

/*------------------------------------------------------------------------------



 \param data
 \param sz
 ------------------------------------------------------------------------------*/
int32_t Host_channel_push(uint8_t chanid, uint8_t *data, uint32_t sz)
{
  uint32_t phead;
  uint32_t head;

  // �������� ������ � ��������� ������
  if (sz > MKW_DATA_SZ) return -1; // �������, ����� ������ ������� �������

  phead = mkw40_queue_head;
  head = phead + 1;
  if (head >= MKW_QUEUE_SZ) head = 0;
  if (head == mkw40_queue_tail) return -1; //�������, � ������� ���� �����

  // ��������� ������ � ����� �������
  mkw_buf_queue[phead][0] = chanid;
  mkw_buf_queue[phead][1] = sz;
  memcpy(&mkw_buf_queue[phead][2], data, sz);

  // ������� ��������� ������ �������
  mkw40_queue_head = head;
  return 0;
}

