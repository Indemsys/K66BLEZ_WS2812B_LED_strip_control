#ifndef K66BLEZ1_MKW40_CHANNEL_H
  #define K66BLEZ1_MKW40_CHANNEL_H


void Init_MKW40_channel(void);
_mqx_uint MKW40_SPI_send_buf(const uint8_t *buff, uint32_t sz);
_mqx_uint MKW40_SPI_read_buf(const uint8_t *buff, uint32_t sz);
_mqx_uint MKW40_SPI_write_read_buf(const uint8_t *wbuff, uint32_t wsz, const uint8_t *rbuff, uint32_t rsz);


_mqx_uint MKW40_SPI_slave_read_buf(const uint8_t *buff, uint32_t sz);
_mqx_uint MKW40_SPI_slave_read_write_buf(const uint8_t *outbuff, uint8_t *inbuff, uint32_t sz);


#endif // K66BLEZ1_MKW40_CHANNEL_H



