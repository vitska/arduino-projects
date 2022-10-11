#ifndef __STATE_H__
#define __STATE_H__

#include <stdint.h>

#define VALUE_MATRIX_NAME_LEN 5
#define VALUE_MATRIX_CNT 43
#define MAX_PPM_IN_CH_NUM 16  // Max limit PPM IN channels

#define VM_AIN1 0
#define VM_AIN2 1
#define VM_AIN3 2
#define VM_AIN4 3
#define VM_AIN5 4
#define VM_AIN6 5
#define VM_AIN7 6
#define VM_AIN8 7
#define VM_PSA1 8
#define VM_PSA2 9
#define VM_SW1  10
#define VM_SW2  11
#define VM_SW3  12
#define VM_SW4  13
#define VM_SW5  14
#define VM_SW6  15
#define VM_SW7  16
#define VM_SW8  17
#define VM_MIX1 18
#define VM_MIX2 19
#define VM_MIX3 20
#define VM_MIX4 21
#define VM_MIX5 22
#define VM_MIX6 23
#define VM_PPI1 24
#define VM_PPI2 25
#define VM_PPI3 26
#define VM_PPI4 27
#define VM_PPI5 28
#define VM_PPI6 29
#define VM_PPI7 30
#define VM_PPI8 31
#define VM_PPI9 32
#define VM_PPI10 33
#define VM_PPI11 34
#define VM_PPI12 35
#define VM_PPI13 36
#define VM_PPI14 37
#define VM_PPI15 38
#define VM_PPI16 39
#define VM_TEST1 40
#define VM_TEST2 41
#define VM_TEST3 42

#define MIX_OFF 0
#define MIX_ON  1
#define MIX_SW1 2
#define MIX_SW2 3
#define MIX_SW3 4
#define MIX_SW4 5
#define MIX_SW5 6
#define MIX_SW6 7
#define MIX_SW7 8
#define MIX_SW8 9


#define SW_DN_VAL -1024
#define SW_MD_VAL 0
#define SW_UP_VAL 1024

#define MIX_HARD_LIMIT_MIN  -2000
#define MIX_HARD_LIMIT_MAX  2000

#define MIX_OFF 0
#define MIX_ON  1

#define EV_STARTUP 0
#define EV_CLOCK_20MS 1
#define EV_CLOCK_100MS 2
#define EV_CLOCK_500MS 3
#define EV_BEEP_END 4
#define EV_SERIAL_LINE_TX_END 5

struct tag_Time
{
  char sec;
  char min;
  char hour;
};

typedef struct tag_Time t_time;
typedef volatile t_time* p_time;

typedef struct tagPPM_Flags {
  unsigned throtle_hold:1;
  unsigned test_adc_mode:1;
  unsigned test_rev_1:1;
  unsigned test_rev_2:1;
  unsigned test_rev_3:1;
  unsigned test_rev_4:1;
  unsigned reserved:2;
} PPM_FLAGS;

typedef struct tagPPM {
  uint16_t Out_Val[8];  //16 // OUT PPM Channel Values
  uint16_t CurOutCh;    //2
  PPM_FLAGS flags;    //1
  uint16_t in_ch_num;   //2
  uint16_t in_cnt;      //2
  int in_rise_val;      //2
  int32_t CurCalcPPMVal;//4
} PPM;

typedef struct tag_Event_Buffer{
  volatile uint8_t in;
  volatile uint8_t out;
  volatile char buf[16];
}EVENT_BUFFER; //256

typedef struct tag_Counters{
  volatile uint8_t t0_ovf_20ms; // 20MS counter for T0
  volatile uint8_t t0_ovf_100ms; // 100MS counter for T0
  volatile uint8_t t0_ovf_500ms; // 500MS counter for T0
}COUNTERS;

typedef struct tag_BlinkMachine{
  uint8_t counter:6;
  uint8_t delay_counter:5;
  uint8_t delay:5;
}BLINK_MACHINE;

void init_blink();
void blink_step(volatile BLINK_MACHINE* machine);
void blink(volatile BLINK_MACHINE* machine, uint8_t delay, uint8_t count);

typedef struct tag_SoundMachine{
  uint8_t period;
  uint16_t delay_counter;
  uint8_t beep_counter;
}SOUND_MACHINE;

void init_sound();
void sound(uint8_t period, uint8_t duration, uint8_t beeps);
void sound_off();

typedef struct tag_Machine{
  BLINK_MACHINE blink;
  SOUND_MACHINE sound;
}MACHINE;

typedef struct tag_USART_IN_Buffer{
	//char flag;
	uint16_t start;
	uint16_t end;
	char buf[256];
}USART_IN_Buffer_t; //256

typedef struct tag_USART_OUT_Struct{
	uint8_t state;
	uint8_t crc;
//	uint8_t size;
	uint8_t ptr;
	uint8_t* start_ptr;
	uint8_t start_size;
	uint8_t start_byte;
}USART_OUT_Struct_t; //256

typedef struct tag_USART{
	USART_IN_Buffer_t in;
	USART_OUT_Struct_t out;
}USART; //256

typedef struct tag_DeviceState{
  EVENT_BUFFER events;
  int matrix[VALUE_MATRIX_CNT];
  PPM ppm;
  int8_t sw_val[8];
  uint8_t active_mixes;
  t_time power_on_time;
  uint8_t flag_buffer_overflow:1;
  COUNTERS counters;
  MACHINE machine;
  USART serial;
}DEVICE_STATE;

extern volatile DEVICE_STATE state;

void init_state();
void put_event(char event);
char wait_event();
void controller(uint8_t event);


#endif
