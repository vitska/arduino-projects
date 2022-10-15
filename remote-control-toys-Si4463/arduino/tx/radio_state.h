#ifndef __RADIO_STATE_H__
#define __RADIO_STATE_H__

#include "packets.h"

typedef struct{
	int16_t rssi;
} responseBufferStruct_t;

typedef struct{
	uint8_t ready;
	uint32_t timestamp;
  uint16_t totalTime;
	int16_t rssi;
	uint8_t length;
	uint8_t packet_counter;
	stickPacketStruct_t stick_packet;
	switchPacketStruct_t switch_packet;
	responseBufferStruct_t response_buffer;
} radio_state_t;

#endif