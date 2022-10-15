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
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "packets.h"

#define CHANNEL 20
#define MAX_PACKET_SIZE 10
#define TIMEOUT 50

#define PACKET_NONE		0
#define PACKET_OK		1
#define PACKET_INVALID	2

typedef struct{
	int16_t rssi;
} responseBufferStruct_t;

typedef struct{
	uint8_t ready;
	uint32_t timestamp;
	int16_t rssi;
	uint8_t length;
	stickPacketStruct_t stick_packet;
	switchPacketStruct_t switch_packet;
	responseBufferStruct_t response_buffer;
} pingInfo_t;

static volatile pingInfo_t radio_state;

void SI446X_CB_RXCOMPLETE(uint8_t length, int16_t rssi)
{
	if(length > MAX_PACKET_SIZE)
		length = MAX_PACKET_SIZE;

	radio_state.ready = PACKET_OK;
	radio_state.rssi = rssi;
	radio_state.length = length;

  radio_state.response_buffer.rssi = rssi;

  switch(length){
    case STICK_PACK_SIZE:
    	Si446x_read(&radio_state.stick_packet, length);
      break;
    case SWITCH_PACK_SIZE:
    	Si446x_read(&radio_state.switch_packet, length);
      break;
  }
  Si446x_TX(&radio_state.response_buffer, sizeof(radio_state.response_buffer), CHANNEL, SI446X_STATE_RX);
}

void SI446X_CB_RXINVALID(int16_t rssi)
{
	radio_state.ready = PACKET_INVALID;
	radio_state.rssi = rssi;
  Si446x_RX(CHANNEL);
}

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup()
{
  reset_state();

	Serial.begin(500000);

	pinMode(A5, OUTPUT); // LED

  pwm.begin();
  pwm.setPWMFreq(50);  // Analog servos run at ~60 Hz updates
  pwm.setPWM(0, 0, 0 );
  pwm.setPWM(1, 0, 0 );
  pwm.setPWM(2, 0, 0 );
  pwm.setPWM(3, 0, 0 );

	// Start up
	Si446x_init();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
//	Si446x_setTxPower(SI446X_MAX_TX_POWER);
//	Si446x_setTxPower(0); // -32dBm (<1uW)
	Si446x_setTxPower(7); // 0dBm (1mW)

	Serial.println(F("Waiting for ping..."));
}

void reset_state(){
	memset(&radio_state, 0, sizeof(radio_state));
}

void motor1(int value){
  if(value > 1023) value = 1023;
  if(value < -1023) value = -1023;
  if(value < 0){
    pwm.setPWM(0, 0, 0 );
    pwm.setPWM(1, 0, value * -4 );
  } else {
    pwm.setPWM(0, 0, value * 4 );
    pwm.setPWM(1, 0, 0 );
  }
}

void motor2(int value){
  if(value > 1023) value = 1023;
  if(value < -1023) value = -1023;
  if(value < 0){
    pwm.setPWM(2, 0, 0 );
    pwm.setPWM(3, 0, value * -4 );
  } else {
    pwm.setPWM(2, 0, value * 4 );
    pwm.setPWM(3, 0, 0 );
  }
}

void map_packet(){
    motor1(radio_state.stick_packet.ch1 - radio_state.stick_packet.ch4);
    motor2(radio_state.stick_packet.ch1 + radio_state.stick_packet.ch4);
    pwm.setPWM(4, 0, radio_state.stick_packet.button1 ? 4095 : 0 );
    pwm.setPWM(5, 0, radio_state.stick_packet.button2 ? 4095 : 0 );
    pwm.setPWM(6, 0, radio_state.stick_packet.button2 ? 4095 : 0 );
}

void print_packet(){
    // Serial.print(pings);
		// Serial.print(F(" rssi:["));
		// Serial.print(radio_state.rssi);
		// // Serial.print(F("] length:["));
		// // Serial.print(radio_state.length);
		// Serial.print(F("] ch0:["));
		// Serial.print(radio_state.stick_packet.ch1);
		// Serial.print(F("] ch1:["));
		// Serial.print(radio_state.stick_packet.ch2);
		// Serial.print(F("] ch2:["));
		// Serial.print(radio_state.stick_packet.ch3);
		// Serial.print(F("] ch3:["));
		// Serial.print(radio_state.stick_packet.ch4);
		// Serial.print(F("] b1:["));
		// Serial.print(radio_state.stick_packet.button1);
		// Serial.print(F("] b2:["));
		// Serial.print(radio_state.stick_packet.button2);
		// Serial.println(F("]"));
}

void loop()
{
	static uint32_t pings;
	static uint32_t invalids;

	// Put into receive mode
	Si446x_RX(CHANNEL);

	// Wait for data
	uint8_t success;

	// Wait for reply with timeout
	uint32_t sendStartTime = millis();
	while(1)
	{
		success = radio_state.ready;
		if(success != PACKET_NONE){
      break;
    }
		else if(millis() - sendStartTime > TIMEOUT){
      reset_state();
      map_packet();
      Serial.println(F("Timeout"));
			break;
    } // Timeout // TODO typecast to uint16_t
	}

  switch(radio_state.ready){
    case PACKET_NONE:
      break;

    case PACKET_INVALID:
      invalids++;
      radio_state.ready = PACKET_NONE;
      Serial.print(F("Invalid rssi:["));
      Serial.print(radio_state.rssi);
      Serial.println(F("]"));
      break;

    case PACKET_OK:
      pings++;
      radio_state.ready = PACKET_NONE;
      // Serial.println(F("Got ping, sending reply..."));

      print_packet();
      map_packet();
      break;
  }
}