/*
 * Project: Si4463 Radio Library for AVR and Arduino (Ping client example)
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2017 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/si4463-radio-library-avr-arduino/
 */

/*
 * Ping client
 *
 * Time how long it takes to send some data and get a reply
 * Should be around 5-6ms with default settings
 */

#include <Si446x.h>
#include <SPI.h>
#include "state.h"
#include "model_config.h"
#include "radio_state.h"

#define CHANNEL 20
#define MAX_PACKET_SIZE 10
#define TIMEOUT 10

#define PACKET_NONE		0
#define PACKET_OK		1
#define PACKET_INVALID	2

static volatile radio_state_t radio_state;
static volatile DEVICE_STATE state;
static STORED_CONFIG config;

void send_packet(){
  radio_state.stick_packet.ch1 = state.matrix[3];
  radio_state.stick_packet.ch2 = state.matrix[2];
  radio_state.stick_packet.ch3 = state.matrix[1];
  radio_state.stick_packet.ch4 = state.matrix[0];

  radio_state.packet_counter++;
  if(radio_state.packet_counter>7){
    radio_state.packet_counter=0;
  }

  switch(radio_state.packet_counter){
    case 0:
    	Si446x_TX(&radio_state.switch_packet, sizeof(radio_state.switch_packet), CHANNEL, SI446X_STATE_RX);
    default:
      Si446x_TX(&radio_state.stick_packet, sizeof(radio_state.stick_packet), CHANNEL, SI446X_STATE_RX);
  }
}

void SI446X_CB_RXCOMPLETE(uint8_t length, int16_t rssi)
{
	if(length > MAX_PACKET_SIZE)
		length = MAX_PACKET_SIZE;

	radio_state.ready = PACKET_OK;
	radio_state.timestamp = millis();
	radio_state.rssi = rssi;
	radio_state.length = length;

	Si446x_read(&radio_state.response_buffer, length);

	// Radio will now be in idle mode
}

void SI446X_CB_RXINVALID(int16_t rssi)
{
	radio_state.ready = PACKET_INVALID;
	radio_state.rssi = rssi;
}

ISR(ADC_vect)
{
  register uint16_t ch = ADMUX & ((1 << MUX2)|(1 << MUX1)|(1 << MUX0));
  state.matrix[ch] = (ADC << 1) - 0x400;
  // Apply stick trims
  if(ch < 4)state.matrix[ch] = state.matrix[ch] + (int)config.model.stick_trim[ch] * 4;
  ch++;
  ch = ch & 0x0007;
  ADMUX = (ADMUX & ~((1 << MUX2)|(1 << MUX1)|(1 << MUX0)))|ch;
  ADCSRA |= 1 << ADSC;
}

void init_adc()
{
  //AVCC with external capacitor at AREF pin
  ADMUX = (1 << REFS0);
  //ADEN: ADC Enable
  //ADSC: ADC Start Conversion
  //ADIE: ADC Interrupt Enable
  //Division Factor: 128 (Single conversion rate ~200uS = 5kHz)
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}

#define LED_PIN 3

void setup()
{
	memset(&radio_state, 0, sizeof(radio_state));
  memset(&config, 0, sizeof(config));
  memset(&state, 0, sizeof(state));

	Serial.begin(500000);

	pinMode(LED_PIN, OUTPUT); // LED
  digitalWrite(LED_PIN, HIGH);

	init_adc();

	// Start up
	Si446x_init();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
//	Si446x_setTxPower(0); // -32dBm (<1uW)
	Si446x_setTxPower(7); // 0dBm (1mW)
}

void loop()
{
	static uint8_t counter;
	static uint32_t sent;
	static uint32_t replies;
	static uint32_t timeouts;
	static uint32_t invalids;

	// Make data
	counter++;
	
	uint32_t startTime = millis();

  send_packet();
	// Send the data
	sent++;
	
//	Serial.println(F("Data sent, waiting for reply..."));
	
	uint8_t success;

	// Wait for reply with timeout
	uint32_t sendStartTime = millis();
	while(1)
	{
		success = radio_state.ready;
		if(success != PACKET_NONE)
			break;
		else if(millis() - sendStartTime > TIMEOUT) // Timeout // TODO typecast to uint16_t
			break;
	}
		
	radio_state.ready = PACKET_NONE;

	if(success == PACKET_NONE)
	{
		Serial.println(F("Ping timed out"));
		timeouts++;
    digitalWrite(LED_PIN, LOW);
	}
	else if(success == PACKET_INVALID)
	{
		Serial.print(F("Invalid packet! Signal: "));
		Serial.print(radio_state.rssi);
		Serial.println(F("dBm"));
		invalids++;
    digitalWrite(LED_PIN, LOW);
	}
	else
	{
		// If success toggle LED and send ping time over UART
		uint16_t totalTime = radio_state.timestamp - startTime;

		digitalWrite(LED_PIN, HIGH);

		replies++;

		Serial.print(F("Ping:["));
		Serial.print(totalTime);
		Serial.print(F("] rssi:["));
		Serial.print(radio_state.rssi);
		Serial.print(F(":"));
		Serial.print(radio_state.response_buffer.rssi);
		// Serial.print(F("] len:["));
		// Serial.print(radio_state.length);
		// Serial.print(F("] ch0:["));
		// Serial.print(radio_state.stick_packet.ch1);
		// Serial.print(F("] ch1:["));
		// Serial.print(radio_state.stick_packet.ch2);
		// Serial.print(F("] ch2:["));
		// Serial.print(radio_state.stick_packet.ch3);
		// Serial.print(F("] ch3:["));
		// Serial.print(radio_state.stick_packet.ch4);
		// Serial.print(F("] a0:["));
		// Serial.print(state.matrix[0]);
		// Serial.print(F("] a1:["));
		// Serial.print(state.matrix[1]);
		// Serial.print(F("] a2:["));
		// Serial.print(state.matrix[2]);
		// Serial.print(F("] a3:["));
		// Serial.print(state.matrix[3]);
		Serial.println(F("]"));

    
	}

	// Serial.print(F("Totals: "));
	// Serial.print(sent);
	// Serial.print(F(" Sent, "));
	// Serial.print(replies);
	// Serial.print(F(" Replies, "));
	// Serial.print(timeouts);
	// Serial.print(F(" Timeouts, "));
	// Serial.print(invalids);
	// Serial.println(F(" Invalid"));
	// Serial.println(F("------"));

//	delay(1000);	
}
