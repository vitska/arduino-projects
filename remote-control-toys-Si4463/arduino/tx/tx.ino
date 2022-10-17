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
#include <Wire.h>
#include "mcp23017.h"
#include "ina219.h"
#include "state.h"
#include "model_config.h"
#include "radio_state.h"

#define CHANNEL 20
#define MAX_PACKET_SIZE 10
#define TIMEOUT 20

#define PACKET_NONE		0
#define PACKET_OK		1
#define PACKET_INVALID	2

static volatile radio_state_t radio_state;
static volatile DEVICE_STATE state;
static STORED_CONFIG config;

MCP23017 ext_port;
INA219 power;

void send_packet(){
  radio_state.stick_packet.ch1 = state.matrix[3];
  radio_state.stick_packet.ch2 = state.matrix[2];
  radio_state.stick_packet.ch3 = state.matrix[1];
  radio_state.stick_packet.ch4 = state.matrix[0];

  radio_state.stick_packet.button1 = (state.matrix[6] >= 80) && (state.matrix[6] <= 100) ? 1 : 0; // 94 BUTTON1
  radio_state.stick_packet.button2 = (state.matrix[6] >= -1024) && (state.matrix[6] <= -950) ? 1 : 0; // -1024 BUTTON2

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
  reset_state();

  Wire.begin();
	Serial.begin(500000);

//  scan_i2c();
  power.begin(0x019F, 0x5000);
  ext_port.setPullup(MCP23017_PORTA, 0xFF);
//  init_i2c_ports();

	pinMode(LED_PIN, OUTPUT); // LED
  digitalWrite(LED_PIN, HIGH);

	init_adc();

	// Start up
	Si446x_init();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
//	Si446x_setTxPower(0); // -32dBm (<1uW)
//	Si446x_setTxPower(7); // 0dBm (1mW)
  Si446x_setTxPower(22); // 10dBm (10mW)
}

#define I2C_SWITCH1 0x8
#define I2C_SWITCH2 0x4
#define I2C_SWITCH3 0x2
#define I2C_SWITCH4 0x1
#define I2C_SWITCH5 0x10
#define I2C_SWITCH6 0x20
#define I2C_SWITCH7 0x40
#define I2C_SWITCH8 0x80

void read_switches(){
  uint8_t val = ext_port.readPort(MCP23017_PORTA);  
  radio_state.switch_packet.sw1 = (val & I2C_SWITCH1) ? 1 : 0;
  radio_state.switch_packet.sw2 = (val & I2C_SWITCH2) ? 1 : 0;
  radio_state.switch_packet.sw3 = (val & I2C_SWITCH3) ? 1 : 0;
  radio_state.switch_packet.sw4 = (val & I2C_SWITCH4) ? 1 : 0;
  radio_state.switch_packet.sw5 = (val & I2C_SWITCH5) ? 1 : 0;
  radio_state.switch_packet.sw6 = (val & I2C_SWITCH6) ? 1 : 0;
  radio_state.switch_packet.sw7 = (val & I2C_SWITCH7) ? 1 : 0;
  radio_state.switch_packet.sw8 = (val & I2C_SWITCH8) ? 1 : 0;
}

// void scan_i2c(){
//   byte error, address; //variable for error and I2C address
//   int nDevices;

//   Serial.println(F("Scanning..."));

//   nDevices = 0;
//   for (address = 1; address < 127; address++ )
//   {
//     // The i2c_scanner uses the return value of
//     // the Write.endTransmisstion to see if
//     // a device did acknowledge to the address.
//     Wire.beginTransmission(address);
//     error = Wire.endTransmission();

//     if (error == 0)
//     {
//       Serial.print(F("I2C device [0x"));
//       Serial.print(address, HEX);
//       Serial.println(F("]"));
//       nDevices++;
//     }
//     else if (error == 4)
//     {
//       Serial.print(F("Unknown error at address [0x"));
//       Serial.print(address, HEX);
//       Serial.println(F("]"));
//     }
//   }
//   if (nDevices == 0)
//     Serial.println(F("No I2C devices found\n"));
//   else
//     Serial.println(F("done\n"));

//   delay(5000); // wait 5 seconds for the next I2C scan
// }

void reset_state(){
	memset(&radio_state, 0, sizeof(radio_state));
  memset(&config, 0, sizeof(config));
  memset(&state, 0, sizeof(state));
}

void print_state(){
  // Serial.print(F("] a0:["));
  // Serial.print(state.matrix[0]);
  // Serial.print(F("] a1:["));
  // Serial.print(state.matrix[1]);
  // Serial.print(F("] a2:["));
  // Serial.print(state.matrix[2]);
  // Serial.print(F("] a3:["));
  // Serial.print(state.matrix[3]);
  // Serial.print(F("] a4:["));
  // Serial.print(state.matrix[4]);
  // Serial.print(F("] a5:["));
  // Serial.print(state.matrix[5]);
  // Serial.print(F("] a6:["));
  // Serial.print(state.matrix[6]);
  // Serial.print(F("] a7:["));
  // Serial.print(state.matrix[7]);
  // Serial.print(F("] b1:["));
  // Serial.print(radio_state.stick_packet.button1);
  // Serial.print(F("] b2:["));
  // Serial.print(radio_state.stick_packet.button2);
  // Serial.print(F("] sw1:["));
  // Serial.print(radio_state.switch_packet.sw1);
  // Serial.print(F("] sw2:["));
  // Serial.print(radio_state.switch_packet.sw2);
  // Serial.print(F("] sw3:["));
  // Serial.print(radio_state.switch_packet.sw3);
  // Serial.print(F("] sw4:["));
  // Serial.print(radio_state.switch_packet.sw4);
  // Serial.println(F("]"));

  // Serial.print(F("ShuntVoltage:["));
  // Serial.print(power.getShuntVoltage());
  // Serial.print(F("] busVoltage:["));
  // Serial.print(power.getBusVoltagemV(), DEC);
  // // Serial.print(F("] power:["));
  // // Serial.print(power.getPower());
  // Serial.print(F("] current:["));
  // Serial.print(power.getCurrentmA());
  // Serial.println(F("]"));
}

void print_packet(){

		// Serial.print(F("Ping:["));
		// Serial.print(radio_state.totalTime);
		// Serial.print(F("] rssi:["));
		// Serial.print(radio_state.rssi);
		// Serial.print(F(":"));
		// Serial.print(radio_state.response_buffer.rssi);
		// // Serial.print(F("] len:["));
		// // Serial.print(radio_state.length);
		// // Serial.print(F("] ch0:["));
		// // Serial.print(radio_state.stick_packet.ch1);
		// // Serial.print(F("] ch1:["));
		// // Serial.print(radio_state.stick_packet.ch2);
		// // Serial.print(F("] ch2:["));
		// // Serial.print(radio_state.stick_packet.ch3);
		// // Serial.print(F("] ch3:["));
		// // Serial.print(radio_state.stick_packet.ch4);
		// // Serial.print(F("] a0:["));
		// // Serial.print(state.matrix[0]);
		// // Serial.print(F("] a1:["));
		// // Serial.print(state.matrix[1]);
		// // Serial.print(F("] a2:["));
		// // Serial.print(state.matrix[2]);
		// // Serial.print(F("] a3:["));
		// // Serial.print(state.matrix[3]);
		// Serial.println(F("]"));
}

void loop()
{
  delay(10); // overvise do not manages to receive
  read_switches(); // all heavy delay stuff goes here
  power.getBusVoltagemV();
  power.getCurrentmA();
	uint32_t startTime = millis();

  send_packet(); //tx starts
	
  // all heavy delay stuff goes here

	uint8_t success;

	// Wait for reply with timeout
	while(1)
	{
		success = radio_state.ready;
		if(success != PACKET_NONE)
			break;
		else if(millis() - startTime > TIMEOUT) // Timeout // TODO typecast to uint16_t
			break;
	}
		
	radio_state.ready = PACKET_NONE;

  print_state();

  switch(success){
    case PACKET_NONE:
      Serial.println(F("Timeout"));
      digitalWrite(LED_PIN, LOW);
      break;

    case PACKET_INVALID:
      Serial.print(F("Invalid packet! Signal: "));
      Serial.print(radio_state.rssi);
      Serial.println(F("dBm"));
      digitalWrite(LED_PIN, LOW);
      break;

    case PACKET_OK:
      radio_state.totalTime = radio_state.timestamp - startTime;
      digitalWrite(LED_PIN, HIGH);
      print_packet();
      break;
  }
}
