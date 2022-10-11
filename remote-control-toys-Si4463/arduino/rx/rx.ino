/*
 * Project: Si4463 Radio Library for AVR and Arduino (Ping server example)
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2017 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/si4463-radio-library-avr-arduino/
 */

/*
 * Ping server
 *
 * Listen for packets and send them back
 */

#include <Si446x.h>

#define CHANNEL 20
#define MAX_PACKET_SIZE 10

#define PACKET_NONE		0
#define PACKET_OK		1
#define PACKET_INVALID	2

typedef struct{
	uint16_t ch[4];
	uint8_t flags1;
	uint8_t flags2;
} rxBufferStruct_t;

typedef struct{
	uint16_t ch[4];
	int16_t rssi;
} responseBufferStruct_t;

typedef struct{
	uint8_t ready;
	uint32_t timestamp;
	int16_t rssi;
	uint8_t length;
	rxBufferStruct_t rx_buffer;
	responseBufferStruct_t response_buffer;
} pingInfo_t;

static volatile pingInfo_t pingInfo;

void SI446X_CB_RXCOMPLETE(uint8_t length, int16_t rssi)
{
	if(length > MAX_PACKET_SIZE)
		length = MAX_PACKET_SIZE;

	pingInfo.ready = PACKET_OK;
	pingInfo.rssi = rssi;
	pingInfo.length = length;

  pingInfo.response_buffer.rssi = rssi;

	Si446x_read(&pingInfo.rx_buffer, length);
  Si446x_TX(&pingInfo.response_buffer, sizeof(pingInfo.response_buffer), CHANNEL, SI446X_STATE_RX);
}

void SI446X_CB_RXINVALID(int16_t rssi)
{
	pingInfo.ready = PACKET_INVALID;
	pingInfo.rssi = rssi;
  Si446x_RX(CHANNEL);
}

void setup()
{
	Serial.begin(500000);

	pinMode(A5, OUTPUT); // LED

	// Start up
	Si446x_init();
//	Si446x_setTxPower(SI446X_MAX_TX_POWER);
//	Si446x_setTxPower(0); // -32dBm (<1uW)
	Si446x_setTxPower(7); // 0dBm (1mW)

	Serial.println(F("Waiting for ping..."));
}

void loop()
{
	static uint32_t pings;
	static uint32_t invalids;

	// Put into receive mode
	Si446x_RX(CHANNEL);

	// Wait for data
	while(pingInfo.ready == PACKET_NONE);
		
	if(pingInfo.ready != PACKET_OK)
	{
		invalids++;
		pingInfo.ready = PACKET_NONE;
		Serial.print(F("Invalid rssi:["));
		Serial.print(pingInfo.rssi);
		Serial.println(F("]"));
	}
	else
	{
		pings++;
		pingInfo.ready = PACKET_NONE;

		// Serial.println(F("Got ping, sending reply..."));

		// Send back the data, once the transmission has completed go into receive mode

		// Serial.println(F("Reply sent"));

		// Toggle LED
		static uint8_t ledState;
		digitalWrite(A5, ledState ? HIGH : LOW);
		ledState = !ledState;

    Serial.print(F("Totals: "));
    Serial.print(pings);

		Serial.print(F(" rssi: "));
		Serial.print(pingInfo.rssi);
		Serial.print(F("dBm length:["));
		Serial.print(pingInfo.length);
		Serial.print(F("] ch0:["));
		Serial.print(pingInfo.rx_buffer.ch[0]);
		Serial.print(F("] ch1:["));
		Serial.print(pingInfo.rx_buffer.ch[1]);
		Serial.print(F("] ch2:["));
		Serial.print(pingInfo.rx_buffer.ch[2]);
		Serial.print(F("] ch3:["));
		Serial.print(pingInfo.rx_buffer.ch[3]);
		Serial.println(F("]"));
	}

	// Serial.print(F("Pings, "));
	// Serial.print(invalids);
	// Serial.println(F("Invalid"));
	// Serial.println(F("------"));
}
