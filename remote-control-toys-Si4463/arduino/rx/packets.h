#ifndef __PACKETS_H__
#define __PACKETS_H__

typedef struct{
	int ch1:11;
	int ch2:11;
	int ch3:11;
	int ch4:11;
	uint8_t button1:1;
	uint8_t button2:1;
	uint8_t button3:1;
	uint8_t button4:1;
} stickPacketStruct_t;

// Use to be 6
#define STICK_PACK_SIZE sizeof(stickPacketStruct_t)

typedef struct{
	uint8_t sw1:1;
	uint8_t sw2:1;
	uint8_t sw3:1;
	uint8_t sw4:1;
	uint8_t sw5:1;
	uint8_t sw6:1;
	uint8_t sw7:1;
	uint8_t sw8:1;
} switchPacketStruct_t;

// Use to be 1
#define SWITCH_PACK_SIZE sizeof(switchPacketStruct_t)

#endif