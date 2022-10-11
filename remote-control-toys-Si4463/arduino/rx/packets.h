#ifndef __PACKETS_H__
#define __PACKETS_H__

typedef struct{
	int ch1:11;
	int ch2:11;
	int ch3:11;
	int ch4:11;
	int button1:1;
	int button2:1;
	int button3:1;
	int button4:1;
} stickPacketStruct_t;

// Use to be 6
#define STICK_PACK_SIZE sizeof(stickPacketStruct_t)

typedef struct{
	int button1:1;
	int button2:1;
	int button3:1;
	int button4:1;
	int button5:1;
	int button6:1;
	int button7:1;
	int button8:1;
} switchPacketStruct_t;

// Use to be 1
#define SWITCH_PACK_SIZE sizeof(switchPacketStruct_t)

#endif