#ifndef _VIRTUAL_COM_H
  #define _VIRTUAL_COM_H  1



typedef int32_t (*T_usb_packet_recever)(uint8_t *buf, uint32_t len);

void     Init_USB(void);
void     Virtual_com_change_usb_speed(uint32_t handle);
uint32_t Virtual_com_send_data(uint8_t *buff, uint32_t size, uint32_t ticks);
uint32_t Virtual_com_get_data(uint8_t *b, uint32_t ticks);
#endif


/* EOF */
