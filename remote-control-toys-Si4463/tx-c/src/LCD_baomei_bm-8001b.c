#include <avr/pgmspace.h>
#include "LCD_baomei_bm-8001b.h"
#include "arduino-compatibility.h"

//Original: https://github.com/Deman75/BAOMEI_BM-8001B

#define INIT_LCD_CS_PIN PIN_MODE_9_OUTPUT
#define INIT_MOSI_PIN PIN_MODE_7_OUTPUT
#define INIT_CLK_PIN PIN_MODE_8_OUTPUT

#define LCD_CS_0 DIGITAL_WRITE_9_LOW
#define LCD_CS_1 DIGITAL_WRITE_9_HIGH

#define MOSI_1 DIGITAL_WRITE_7_HIGH
#define MOSI_0 DIGITAL_WRITE_7_LOW

#define CLK_1 DIGITAL_WRITE_8_HIGH
#define CLK_0 DIGITAL_WRITE_8_LOW

uint8_t data[16] = {
	//0			//1			//2	  //3	//4	  //5	//6	  //7
	0b00000000, 0b00000000, 0x09, 0x02, 0x80, 0x80, 0x00, 0x28,
	//8	  //9	//10  //11	//12  //13  //14		//15 
	0x00, 0x00, 0x90, 0x82, 0x00, 0x08, 0b10001000, 0x20
	};

//|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|

inline void SPI_Send (uint8_t value, uint8_t length) {
	for (register int8_t i = length - 1; i >= 0; i--) {
		CLK_0;

		if (value & (1 << i)) {
			MOSI_1;
		} else {
			MOSI_0;
		}

		CLK_1;
	}
}

const uint8_t init[8] PROGMEM = {0x52, 0x30, 0x00, 0x0A, 0x02, 0x06, 0xC0, 0x10};

void LCD_Init() {

	register int8_t i, j;
  
	INIT_LCD_CS_PIN;
	INIT_MOSI_PIN;
	INIT_CLK_PIN;

	for (i = 0; i < 8; i++) {
		LCD_CS_0;
		SPI_Send(0x08, 4);
		for (j = 0; j < 5; j++);
		SPI_Send(pgm_read_byte(&init[i]), 8);
		LCD_CS_1;
	}

	LCD_CS_0;

	SPI_Send(0x05, 3);
	SPI_Send(0x00, 6);

	for (i = 0; i < sizeof(data); i++) {
		SPI_Send(data[i], 8);
		for (j = 0; j < 10; j++);
	}

	LCD_CS_1;

	MOSI_1;

}

#define SETBIT(value, position) (value | (1 << position))

#define UNSETBIT(value, position) (value & ~(1 << position))

void LCD_Update() {
	LCD_CS_0;

	SPI_Send(0x05, 3);
	SPI_Send(0x00, 6);

	for (register uint8_t i = 0; i < sizeof(data); i++) {
		SPI_Send(data[i], 8);
	}

	LCD_CS_1;
}

// =========================================================== //

typedef struct tag_Bits{
	struct tag_TrimBits{
		uint8_t throttle[11];
		uint8_t rudder[11];
		uint8_t elev[11];
		uint8_t eler[11];
	} trim;
	struct tag_Trottle{
		uint8_t right[11];
		uint8_t left[11];
	} throtle;
}BITS; //256

const BITS bits PROGMEM = {
	.trim = {
		.throttle = {11,10,9,8,12,13,14,15,2,1,0},
		.rudder = {15,14,13,12,0,1,2,3,6,5,4},
		.elev = {7,6,5,4,0,1,2,3,14,13,12},
		.eler = {8,9,10,15,14,13,12,0,1,2,3}
	},
	.throtle = {
		.right = {15,14,13,12,0,1,2,3,6,5,4},
		.left = {3,2,1,0,12,13,14,15,10,9,8}
	}
};

// #define BSWAP_16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

int8_t limit_min_max(int8_t value, int8_t min, int8_t max){
	if(value < min){
		value = min;
	}
	if(value > max){
		value = max;
	}
	return value;	
}

void TrimRudder(int8_t pos){
	uint16_t* word = (uint16_t*)&data[11];
	*word &= 0b0000111110000000;
	*word |= 1 << pgm_read_byte(&bits.trim.rudder[limit_min_max(-5, 5, pos) + 5]);
}

void TrimThrottle(int8_t pos) {
	uint16_t* word = (uint16_t*)&data[14];
	*word &= 0b0000000011111000;
	*word |= 1 << pgm_read_byte(&bits.trim.throttle[limit_min_max(-5, 5, pos) + 5]);
}

void TrimElev(int8_t pos) {
	uint16_t* word = (uint16_t*)&data[3];
	*word &= 0b1000111100000000;
	*word |= 1 << pgm_read_byte(&bits.trim.elev[limit_min_max(-5, 5, pos) + 5]);
}

void TrimEler(int8_t pos) {
	uint16_t* word = (uint16_t*)&data[6];
	*word &= 0b0000100011110000;
	*word |= 1 << pgm_read_byte(&bits.trim.eler[limit_min_max(-5, 5, pos) + 5]);
}

 //----------------------------------------------------------//
void SetMode(int8_t mode) { // 1 or 2
	data[14] &= 0b11001111;
	data[14] |= (mode == 1) ? 0b00100000 : 0b00010000;
}


//------------------------------------------------------------//
void ThrotleRight(uint8_t throttle){
	uint16_t* word = (uint16_t*)&data[5];
	*word &= 0b0000111110000000;

	for (register int8_t i = 0; i < limit_min_max(0, 11, throttle); i++) {
		*word |= 1 << pgm_read_byte(&bits.throtle.right[i]);
	}
}

void ThrotleLeft(uint8_t throttle){
	uint16_t* word = (uint16_t*)&data[12];
	*word &= 0b0000100011110000;

	for (register int8_t i = 0; i < limit_min_max(0, 11, throttle); i++) {
		*word |= 1 << pgm_read_byte(&bits.throtle.left[i]);
	}
}

//------------------------------------------------------------//
const uint8_t elevator_pos_e[11] PROGMEM = {3,2,1,0,6,4,5,4,5,6,7};

void Elevator(int8_t pos){

	data[10] = 0b10000000;
	data[9] &= 0b00001111;

	int8_t i;

	if (pos >= 0) {
		for (i = 5; i <= pos + 5; i++) {
			if (i > 6) {
				data[9] = SETBIT(data[9], pgm_read_byte(&elevator_pos_e[i]));
			} else {
				data[10] = SETBIT(data[10], pgm_read_byte(&elevator_pos_e[i]));
			}
		}
	} else {
		for (i = 5; i >= pos + 5; i--) {
			if (i > 6) {
				data[9] = SETBIT(data[9], pgm_read_byte(&elevator_pos_e[i]));
			} else {
				data[10] = SETBIT(data[10], pgm_read_byte(&elevator_pos_e[i]));
			}
		}
	}
}

//-----------------------------------------------------------------//
const uint8_t eleron_pos_e[11] PROGMEM = {3,7,6,5,4,4,0,1,2,3,3};

void Eleron(int8_t pos) {
	data[1] &= 0b00000111;
	data[9] &= 0b11110000;
	data[8] = UNSETBIT(data[8], 3);

	if (pos >= 0) {
		for (register int8_t i = 5; i <= pos + 5; i++) {//pgm_read_byte(&eleron_pos_e[i])
			if (i > 5 && i < 10) {
				data[9] = SETBIT(data[9], pgm_read_byte(&eleron_pos_e[i]));
			} else if (i == 10) {
				data[8] = SETBIT(data[8], pgm_read_byte(&eleron_pos_e[i]));
			}
		}
	} else {
		for (register int8_t i = 5; i >= pos + 5; i--) {
			data[1] = SETBIT(data[1], pgm_read_byte(&eleron_pos_e[i]));
		}
	}
}

//----------------------------------------------------------------//
const uint8_t rudder_pos_e[14] PROGMEM = {0,1,2,4,5,6,7,7,6,5,4,2,1,0};

void Rudder(int8_t pos){
	data[8] &= 0b00001000;
	data[1] &= ~0b111;
	data[2] &= 0b00001111;

	pos = limit_min_max(-7, 7, pos);

	if (pos >= 0) {
		for (register int8_t i = 7; i <= pos + 7; i++) {
			data[8] = SETBIT(data[8], pgm_read_byte(&rudder_pos_e[i]));
		}
	} else {
		for (register int8_t i = 6; i >= pos + 6; i--) {
			if (i > 2) {
				data[2] = SETBIT(data[2], pgm_read_byte(&rudder_pos_e[i]));
			} else {
				data[1] = SETBIT(data[1], pgm_read_byte(&rudder_pos_e[i]));
			}
		}
	}
}


//---------------------------------------------------------------//

void DoubleRate(int8_t dr) {
	data[2] &= ~0b110;
	data[2] |= dr ? 0b010 : 0b100;
}

//----------------------------------------------------------------//

void Bat(int8_t pos){
	data[4] &= ~0b1111;
	switch (pos)
	{
		case 1: data[4] |= 0b1000;	break;
		case 2: data[4] |= 0b1100;	break;
		case 3: data[4] |= 0b1110;	break;
		case 4: data[4] |= 0b1111;	break;
	}
}

//---------------------------------------------------------------//

void Ant(int8_t on) {
	data[14] &= ~0b1000000;
	data[14] |= on ? 0b1000000 : 0b0000000;
}

//-------------------------------------------------------------//

void Quadro(int8_t on){
	data[2] &= ~0b1000;
	data[2] |= on ? 0b1000 : 0b0000;
}
