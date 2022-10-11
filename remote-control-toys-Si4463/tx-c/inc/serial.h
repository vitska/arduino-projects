#ifndef __SERIAL_H__
#define __SERIAL_H__

void init_serial();
void serial_write(uint8_t b);
void serial_write_block(void* ptr, uint8_t sz, uint8_t startb);

#endif
