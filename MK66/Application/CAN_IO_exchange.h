#ifndef CAN_IO_EXCHANGE_H
  #define CAN_IO_EXCHANGE_H


// ����������� ���� ������� ��� ���� T_input_cbl.itype
#define   GEN_SW     0
#define   ESC_SW     1


// ��������� ��� ����������� ���������� ��������
typedef struct
{
  int8_t    val;
  int8_t    curr;
  int8_t    prev;
  uint32_t  cnt;
  uint8_t   init;

} T_bncf;


// ��������� ��������� ����������� ��������� ��������
typedef struct
{
  uint8_t            itype;     // ���  �������. 0 - ������� ������� ������������, 1 - ������� � ���� ������������ � 3-� �����������
  uint16_t           *p_smpl1;  // ��������� �� ��������� ������������� ���������� �� �������� � ����� ������� �����������
  uint16_t           *p_smpl2;  // ��������� �� ��������� ������������� ���������� �� �������� � ����� ������ �����������
  uint16_t           log_bound; // ������� ����� ���������� 0 � 1 �� �����
  uint32_t           l0_time;   // ����� ������������ ��������� ��� �������� ������� ������ �������
  uint32_t           l1_time;   // ����� ������������ ��������� ��� �������� �������� ������ �������
  uint32_t           lu_time;   // ����� ������������ ��������� ��� �������� ��������������� ������ �������
  int8_t             *val;      // ��������� �� ���������� ��� ���������� ������������ ��������� �����
  int8_t             *val_prev; // ��������� �� ���������� ��� ���������� ����������� ��������� �����
  int8_t             *flag;     // ��������� �� ���� ����������. ���� �� ������ ���� ��������� �� ������������ ��������� ��������� ����������
  T_bncf             pbncf;     // ��������� ��� ��������� ���������� ��������
} T_input_cbl;


// ����������� ��������� ��� ��������� �������� ��������� �������� � 8-� ������� ����� CAN
typedef struct
{
  uint8_t            itype;    // ���  �������. 0 - ������� ������� ������������, 1 - ������� � ���� ������������ � 3-� �����������
  uint8_t            nbyte;    // ����� �����
  uint8_t            nbit;     // ����� ���� � �����
  int8_t             *val;     // ��������� �� ���������� ��������� �������
  int8_t             *val_prev;// ��������� �� ���������� ��������� �������
  int8_t             *flag;    // ��������� �� ���� ����������. ���� �� ������ ���� ��������� �� ������������ ��������� ��������� ����������
  const uint8_t      *name;    // ��� ����������
} T_can_inp_pack;


// ��������� ��������� �������� ��������� ������� � 8-� ������� ����� CAN
typedef struct
{
  uint8_t            mask;     // ������� ����� ���������� ���������� �� ������ � �����
  uint8_t            nbyte;    // ����� �����
  uint8_t            nbit;     // ����� ���� � �����
  int8_t             *val;     // ��������� �� ���������� ��������� �������
  const uint8_t      *name;    // ��� ����������
} T_can_out_pack;




uint32_t  Do_input_processing(T_input_cbl *scbl);
uint32_t  CAN_pack_inputs(T_can_inp_pack *parr, uint32_t sz, uint8_t *canbuf, uint8_t *packlen);
int32_t   CAN_unpack_received_inputs(T_can_msg *rx, T_can_inp_pack *parr, uint32_t sz);
uint32_t  CAN_pack_outputs(T_can_out_pack *parr, uint32_t sz, uint8_t *canbuf, uint8_t *packlen);
uint32_t  CAN_unpack_received_outputs(T_can_msg *rx, T_can_out_pack *parr, uint32_t sz);

#endif // CAN_IO_EXCHANGE_H



