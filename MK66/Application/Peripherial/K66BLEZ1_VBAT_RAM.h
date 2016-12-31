#ifndef K66BLEZ1_VBAT_RAM_H
  #define K66BLEZ1_VBAT_RAM_H


#define VBAT_RAM_WRD_SZ 32 //������  VBAT_RAM � 4-� ������� ������


typedef struct
{
  uint32_t reg[VBAT_RAM_WRD_SZ];
}
T_VBAT_RAM;

#define VBAT_RAM_ptr    ((T_VBAT_RAM*)0x4003E000u)

uint32_t K66BLEZ1_VBAT_RAM_validation(void);
void     K66BLEZ1_VBAT_RAM_sign_data(void);

#endif // K66BLEZ1_VBAT_RAM_H



