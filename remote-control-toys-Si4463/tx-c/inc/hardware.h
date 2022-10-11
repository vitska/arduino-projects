#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "arduino-compatibility.h"

#ifdef __AVR_ATmega168__

#define LED_PORT DIGITAL_4_PORT
#define LED_PIN DIGITAL_4_PIN
#define LED_PIN_TEST DIGITAL_READ_4
#define LED_PIN_OUTPUT PIN_MODE_4_OUTPUT

    // 31.25kHz Overflow: 122.0703125Hz
    //  Enable orverflow vector;
#define INIT_TIMING_TIMER TCCR0B = (1 << CS01) | (1 << CS00); TIMSK0 = (1 << TOIE0);
#define TIMING_OVERFLOW_VECTOR TIMER0_OVF_vect

#define TIMING_PRESCALLER 64.0
#define TIMING_RELOAD 256.0
#define TIMING_US 1000000.0

#define S_TO_RELOAD_K ((TIMING_PRESCALLER * TIMING_RELOAD * TIMING_US)/F_CPU)
#define MS_TO_RELOAD_INT8_VALUE(ms) (uint8_t)((ms * 1000UL) / S_TO_RELOAD_K)

#define SOUND_COMPARE_VECTOR TIMER0_COMPA_vect
#define SOUND_PIN_OUTPUT PIN_MODE_5_OUTPUT
#define SOUND_TIMER_COMPARE_ENABLE TIMSK0 |= (1 << OCIE0A) 
#define SOUND_TIMER_COMPARE_DISABLE TIMSK0 &= ~(1 << OCIE0A) 

#define SERIAL_TX_VECTOR USART_TX_vect

#define UART_CTL_A_REG UCSR0A
#define UART_CTL_B_REG UCSR0B
#define UART_CTL_C_REG UCSR0C
#define UART_DATA_REG UDR0
#define UART_BAUD_REG UBRR0

#define UART_TX_INT_BIT TXCIE0
#define UART_RX_INT_BIT RXCIE0

#define UART_TX_INT_ENABLED (UART_CTL_B_REG & (1 << UART_TX_INT_BIT))
#define UART_TX_ENABLE_INT() (UART_CTL_B_REG |=	(1 << UART_TX_INT_BIT))
#define UART_RX_ENABLE_INT() (UART_CTL_B_REG |= (1 << UART_RX_INT_BIT))
#define UART_TX_DISABLE_INT() (UART_CTL_B_REG &= ~(1 << UART_TX_INT_BIT))
#define UART_RX_DISABLE_INT() (UART_CTL_B_REG &= ~(1 << UART_RX_INT_BIT))

#define U2X_0_DIV 16
#define U2X_1_DIV 8
#define U2X_DIV U2X_1_DIV
#define UBRR_BAUD(baud) F_CPU/U2X_DIV/baud-1

/* Double the USART Transmission Speed */
#define INIT_SERIAL_CTL_A() UART_CTL_A_REG |= (1<<U2X0);
/* Set frame format: 8data, 1stop bit */
#define INIT_SERIAL_CTL_C() UART_CTL_C_REG = (3<<UCSZ00);
/* Enable receiver and transmitter */
#define INIT_SERIAL_CTL_B() UART_CTL_B_REG = (1<<RXEN0)|(1<<TXEN0);

#define INIT_SERIAL_CTL() INIT_SERIAL_CTL_A();INIT_SERIAL_CTL_B();INIT_SERIAL_CTL_C();

#endif // __AVR_ATmega168__

#endif