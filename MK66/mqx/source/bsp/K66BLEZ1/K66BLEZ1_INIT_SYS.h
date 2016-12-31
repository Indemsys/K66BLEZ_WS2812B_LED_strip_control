#ifndef K66BLEZ1_INIT_SYS
  #define K66BLEZ1_INIT_SYS

void  Init_cpu(void);
void _bsp_get_unique_identificator(unsigned int *uid);
void NVIC_EnableIRQ(int32_t irq_index);
void NVIC_SetPriority(int32_t irq_index, uint32_t priority);

#endif
