#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "state.h"
#include "hardware.h"
#include "serial.h"

#define LOW_HALF(x) (x & 0x0F)
#define HIGH_HALF(x) (x >> 4)

uint8_t b2c(uint8_t v){
    return (v < 0xA) ? v + '0' : v + 'A' - 0xA; 
}

#define SERIAL_STATE_SIZE_HI 0
#define SERIAL_STATE_SIZE_LO 1
#define SERIAL_STATE_SIZE_SEP 2
#define SERIAL_STATE_SEND_BUF_HI 3
#define SERIAL_STATE_SEND_BUF_LO 4
#define SERIAL_STATE_CRC_SEP 5
#define SERIAL_STATE_CRC_HI 6
#define SERIAL_STATE_CRC_LO 7
#define SERIAL_STATE_NEXT_LINE 8
#define SERIAL_STATE_END 0xFF

#define SERIAL_STATE(cs, next) case cs: state.serial.out.state = next;

/* USART, Tx Complete */
ISR(SERIAL_TX_VECTOR)
{
	switch(state.serial.out.state)
	{
        SERIAL_STATE(SERIAL_STATE_SIZE_HI, SERIAL_STATE_SIZE_LO)
            UART_DATA_REG = b2c(HIGH_HALF(state.serial.out.start_size));
            break;

        SERIAL_STATE(SERIAL_STATE_SIZE_LO, SERIAL_STATE_SIZE_SEP)
            UART_DATA_REG = b2c(LOW_HALF(state.serial.out.start_size));
            state.serial.out.crc += state.serial.out.start_size;
            break;
            
        SERIAL_STATE(SERIAL_STATE_SIZE_SEP, SERIAL_STATE_SEND_BUF_HI)
            UART_DATA_REG = '-';
            break;
            
        SERIAL_STATE(SERIAL_STATE_SEND_BUF_HI, SERIAL_STATE_SEND_BUF_LO)
            UART_DATA_REG = b2c(HIGH_HALF(*(state.serial.out.start_ptr + state.serial.out.ptr)));
            break;
            
        SERIAL_STATE(SERIAL_STATE_SEND_BUF_LO, SERIAL_STATE_SEND_BUF_HI)
            {
                register uint8_t b = *(state.serial.out.start_ptr + state.serial.out.ptr);
                state.serial.out.crc += b;
                UART_DATA_REG = b2c(LOW_HALF(b));
                state.serial.out.ptr++;
                state.serial.out.state = (state.serial.out.ptr % 4 == 0) ? SERIAL_STATE_SIZE_SEP : SERIAL_STATE_SEND_BUF_HI;
                if(state.serial.out.ptr >= state.serial.out.start_size){
                    state.serial.out.state = SERIAL_STATE_CRC_SEP;
                }
            }
            break;

        SERIAL_STATE(SERIAL_STATE_CRC_SEP, SERIAL_STATE_CRC_HI)
            UART_DATA_REG = '-';
            break;

        SERIAL_STATE(SERIAL_STATE_CRC_HI, SERIAL_STATE_CRC_LO)
            UART_DATA_REG = b2c(HIGH_HALF(state.serial.out.crc));
            break;

        SERIAL_STATE(SERIAL_STATE_CRC_LO, SERIAL_STATE_END)
            UART_DATA_REG = b2c(LOW_HALF(state.serial.out.crc));
            break;

        SERIAL_STATE(SERIAL_STATE_END, SERIAL_STATE_SIZE_HI)
//            UART_TX_DISABLE_INT(); //End of sequence
            UART_DATA_REG = 0xA;
            put_event(EV_SERIAL_LINE_TX_END);
            state.serial.out.crc = state.serial.out.start_byte;
            state.serial.out.ptr = 0;
            break;
            
	}
}

/* USART, Rx Complete */
// ISR(USART0_RX_vect)
// {
// 	state.serial.in.buf[state.serial.in.start] = UART_DATA_REG;
// 	state.serial.in.start++;
// 	if(state.serial.in.start >= sizeof(state.serial.in.buf))state.serial.in.start = 0;
// 	if(state.serial.in.start == state.serial.in.end) UART_RX_DISABLE_INT();//Buffer overflow do not receive
// }

uint8_t usart_pop_in_byte()
{
	UART_RX_ENABLE_INT();
	uint8_t ch = state.serial.in.buf[state.serial.in.end];
	state.serial.in.end++;
	if(state.serial.in.end >= sizeof(state.serial.in.buf))state.serial.in.end = 0;
	//uart.in.flag &= ~0x2;
	return ch;
}

void serial_write_block(void* ptr, uint8_t sz, uint8_t startb)
{
	//-------- Wait for transmission end ---------
	while(UART_TX_INT_ENABLED);// && !(UCSRA & (1 << TXC0)));
	state.serial.out.state = SERIAL_STATE_SIZE_HI;
    state.serial.out.start_byte = startb;
	state.serial.out.crc = startb;
	state.serial.out.ptr = 0;
    state.serial.out.start_ptr = ptr;
	//state.serial.out.size = sz;
    state.serial.out.start_size = sz;
	UART_TX_ENABLE_INT();
	UART_DATA_REG = startb;
}

void serial_write(uint8_t b){
    UART_DATA_REG = b;
}

const uint16_t serial_baud = UBRR_BAUD(500000);

void init_serial(){
	//UART_TX_DISABLE_INT();
	/* Set baud rate */
	UART_BAUD_REG = serial_baud;
    INIT_SERIAL_CTL();
	//UART_RX_ENABLE_INT();
//	while(UCSRA & (1<<RXC))
//	{
//		uint8_t tmp = UDR;
//	}
}
