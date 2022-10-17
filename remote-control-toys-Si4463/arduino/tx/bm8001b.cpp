#include "bm8001b.h"
#include <avr/pgmspace.h>
//#include "arduino-compatibility.h"

//Original: https://github.com/Deman75/BAOMEI_BM-8001B
#define LCD_CS_PIN 9
#define LCD_MOSI_PIN 7
#define LCD_CLK_PIN 8

#define INIT_LCD_CS_PIN PIN_MODE_9_OUTPUT
#define INIT_MOSI_PIN PIN_MODE_7_OUTPUT
#define INIT_CLK_PIN PIN_MODE_8_OUTPUT

#define LCD_CS_1 digitalWrite(LCD_CS_PIN, HIGH);
#define LCD_CS_0 digitalWrite(LCD_CS_PIN, LOW);

#define MOSI_1 digitalWrite(LCD_MOSI_PIN, HIGH);
#define MOSI_0 digitalWrite(LCD_MOSI_PIN, LOW);

#define CLK_1 digitalWrite(LCD_CLK_PIN, HIGH);
#define CLK_0 digitalWrite(LCD_CLK_PIN, LOW);

uint8_t lcd_data[16] = {
                                            //0x80                                          //0b00001000
	//0			    //1			    //2	  //3	  //4	  //5	  //6	  //7	  //8	  //9	  //10  //11	      //12        //13        //14		    //15
	0b00000000, 0b00000000, 0x09, 0x02, 0x80, 0x00, 0x00, 0x28, 0x00, 0x00, 0x90, 0x00000010, 0b00000000, 0b00000000, 0b10001000, 0x20
	};

//|----0---|----1---|----2---|----3---|----4---|----5---|----6---|----7---|----8---|----9---|---10---|---11---|---12---|---13---|---14---|---15---|
//|00000000|00000000|00000000|00000000|00000000|10000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000|
//                                              │└┴┴┴┴┴┴─┼┴┴┘                                         │└┴┴┼┴┴┴─┼┴┴┘└┴┴┴─┼┴┴┴┼┴┴┘
//                                              │        └── Throtle Right                            │   │    │        │   └── Frame
//                                              └─────────── Frame                                    │   │    │        └────── ThrotleLeft
//                                                                                                    │   │    └── Rudder trim
//                                                                                                    │   └─────── Center point
//                                                                                                    └─────────── Frame
//                                                                                                    0b10000000 00001111
BM8001B::BM8001B() {}

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

const uint8_t init_data[8] PROGMEM = {0x52, 0x30, 0x00, 0x0A, 0x02, 0x06, 0xC0, 0x10};

void BM8001B::begin() {

	register int8_t i, j;
  
	pinMode(LCD_CS_PIN, OUTPUT);
	pinMode(LCD_MOSI_PIN, OUTPUT);
	pinMode(LCD_CLK_PIN, OUTPUT);

	for (i = 0; i < 8; i++) {
		LCD_CS_0;
		SPI_Send(0x08, 4);
		for (j = 0; j < 5; j++);
		SPI_Send(pgm_read_byte(&init_data[i]), 8);
		LCD_CS_1;
	}

	LCD_CS_0;

	SPI_Send(0x05, 3);
	SPI_Send(0x00, 6);

	for (i = 0; i < sizeof(lcd_data); i++) {
		SPI_Send(lcd_data[i], 8);
		for (j = 0; j < 10; j++);
	}

	LCD_CS_1;

	MOSI_1;

}

#define SETBIT(value, position) (value | (1 << position))

#define UNSETBIT(value, position) (value & ~(1 << position))

void BM8001B::update() {
	LCD_CS_0;

	SPI_Send(0x05, 3);
	SPI_Send(0x00, 6);

	for (register uint8_t i = 0; i < sizeof(lcd_data); i++) {
		SPI_Send(lcd_data[i], 8);
	}

	LCD_CS_1;
}

// =========================================================== //

typedef struct tag_Bits{
	struct tag_TrimBits{
		uint8_t throttle[11];
		uint8_t rudder[12];
		uint8_t elev[11];
		uint8_t eler[11];
	} trim;
	struct tag_Trottle{
		uint8_t right[12];
		uint8_t left[12];
	} throtle;
}BITS; //256

const BITS bits PROGMEM = {
	.trim = {
		.throttle = {11,10,9,8,12,13,14,15,2,1,0},
		.rudder = {7,15,14,13,12,0,1,2,3,6,5,4},
		.elev = {7,6,5,4,0,1,2,3,14,13,12},
		.eler = {8,9,10,15,14,13,12,0,1,2,3}
	},
	.throtle = {
		.right = {7,15,14,13,12,0,1,2,3,6,5,4},
		.left = {11,3,2,1,0,12,13,14,15,10,9,8}
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

void BM8001B::trimRudderGauge(uint8_t value){
	uint16_t* word = (uint16_t*)&lcd_data[11];
	*word &= 0b0000111100000000;

	for (register int8_t i = 0; i < limit_min_max(0, sizeof(bits.trim.rudder), value); i++) {
		*word |= 1 << pgm_read_byte(&bits.trim.rudder[i]);
	}
}

//------------- -5..5 -------------------------
void BM8001B::trimRudder(int8_t pos){
	uint16_t* word = (uint16_t*)&lcd_data[11];
	*word &= 0b0000111110000000;
	*word |= 0b0000000010000000; // frame
	*word |= 1 << pgm_read_byte(&bits.trim.rudder[limit_min_max(-5, 5, pos) + 6]);
}

void BM8001B::trimThrottle(int8_t pos) {
	uint16_t* word = (uint16_t*)&lcd_data[14];
	*word &= 0b0000000011111000;
	*word |= 1 << pgm_read_byte(&bits.trim.throttle[limit_min_max(-5, 5, pos) + 5]);
}

void BM8001B::trimElev(int8_t pos) {
	uint16_t* word = (uint16_t*)&lcd_data[3];
	*word &= 0b1000111100000000;
	*word |= 1 << pgm_read_byte(&bits.trim.elev[limit_min_max(-5, 5, pos) + 5]);
}

void BM8001B::trimEler(int8_t pos) {
	uint16_t* word = (uint16_t*)&lcd_data[6];
	*word &= 0b0000100011110000;
	*word |= 1 << pgm_read_byte(&bits.trim.eler[limit_min_max(-5, 5, pos) + 5]);
}

 //----------------------------------------------------------//
void BM8001B::setMode(int8_t mode) { // 1 or 2
	lcd_data[14] &= 0b11001111;
	lcd_data[14] |= (mode == 1) ? 0b00100000 : 0b00010000;
}


//-------------------- 0..12 ----------------------------------------//
void BM8001B::throtleRight(uint8_t throttle){
	uint16_t* word = (uint16_t*)&lcd_data[5];
	*word &= 0b0000111100000000;

	for (register int8_t i = 0; i < limit_min_max(0, sizeof(bits.throtle.right), throttle); i++) {
		*word |= 1 << pgm_read_byte(&bits.throtle.right[i]);
	}
}

//-------------------- 0..12 ----------------------------------------//
void BM8001B::throtleLeft(uint8_t throttle){
	uint16_t* word = (uint16_t*)&lcd_data[12];
	*word &= 0b0000000011110000;

	for (register int8_t i = 0; i < limit_min_max(0, sizeof(bits.throtle.left), throttle); i++) {
		*word |= 1 << pgm_read_byte(&bits.throtle.left[i]);
	}
}

//------------------------------------------------------------//
const uint8_t elevator_pos_e[11] PROGMEM = {3,2,1,0,6,4,5,4,5,6,7};

void BM8001B::elevator(int8_t pos){

	lcd_data[10] = 0b10000000;
	lcd_data[9] &= 0b00001111;

	int8_t i;

	if (pos >= 0) {
		for (i = 5; i <= pos + 5; i++) {
			if (i > 6) {
				lcd_data[9] = SETBIT(lcd_data[9], pgm_read_byte(&elevator_pos_e[i]));
			} else {
				lcd_data[10] = SETBIT(lcd_data[10], pgm_read_byte(&elevator_pos_e[i]));
			}
		}
	} else {
		for (i = 5; i >= pos + 5; i--) {
			if (i > 6) {
				lcd_data[9] = SETBIT(lcd_data[9], pgm_read_byte(&elevator_pos_e[i]));
			} else {
				lcd_data[10] = SETBIT(lcd_data[10], pgm_read_byte(&elevator_pos_e[i]));
			}
		}
	}
}

//-----------------------------------------------------------------//
const uint8_t eleron_pos_e[11] PROGMEM = {3,7,6,5,4,4,0,1,2,3,3};

void BM8001B::eleron(int8_t pos) {
	lcd_data[1] &= 0b00000111;
	lcd_data[9] &= 0b11110000;
	lcd_data[8] = UNSETBIT(lcd_data[8], 3);

	if (pos >= 0) {
		for (register int8_t i = 5; i <= pos + 5; i++) {//pgm_read_byte(&eleron_pos_e[i])
			if (i > 5 && i < 10) {
				lcd_data[9] = SETBIT(lcd_data[9], pgm_read_byte(&eleron_pos_e[i]));
			} else if (i == 10) {
				lcd_data[8] = SETBIT(lcd_data[8], pgm_read_byte(&eleron_pos_e[i]));
			}
		}
	} else {
		for (register int8_t i = 5; i >= pos + 5; i--) {
			lcd_data[1] = SETBIT(lcd_data[1], pgm_read_byte(&eleron_pos_e[i]));
		}
	}
}

//----------------------------------------------------------------//
const uint8_t rudder_pos_e[14] PROGMEM = {0,1,2,4,5,6,7,7,6,5,4,2,1,0};

void BM8001B::rudder(int8_t pos){
	lcd_data[8] &= 0b00001000;
	lcd_data[1] &= ~0b111;
	lcd_data[2] &= 0b00001111;

	pos = limit_min_max(-7, 7, pos);

	if (pos >= 0) {
		for (register int8_t i = 7; i <= pos + 7; i++) {
			lcd_data[8] = SETBIT(lcd_data[8], pgm_read_byte(&rudder_pos_e[i]));
		}
	} else {
		for (register int8_t i = 6; i >= pos + 6; i--) {
			if (i > 2) {
				lcd_data[2] = SETBIT(lcd_data[2], pgm_read_byte(&rudder_pos_e[i]));
			} else {
				lcd_data[1] = SETBIT(lcd_data[1], pgm_read_byte(&rudder_pos_e[i]));
			}
		}
	}
}


//---------------------------------------------------------------//

void BM8001B::doubleRate(int8_t dr) {
	lcd_data[2] &= ~0b110;
	lcd_data[2] |= dr ? 0b010 : 0b100;
}

//----------------------------------------------------------------//

void BM8001B::bat(int8_t pos){
	lcd_data[4] &= ~0b1111;
	switch (pos)
	{
		case 1: lcd_data[4] |= 0b1000;	break;
		case 2: lcd_data[4] |= 0b1100;	break;
		case 3: lcd_data[4] |= 0b1110;	break;
		case 4: lcd_data[4] |= 0b1111;	break;
	}
}

//---------------------------------------------------------------//

void BM8001B::ant(int8_t on) {
	lcd_data[14] &= ~0b1000000;
	lcd_data[14] |= on ? 0b1000000 : 0b0000000;
}

//-------------------------------------------------------------//

void BM8001B::quadro(int8_t on){
	lcd_data[2] &= ~0b1000;
	lcd_data[2] |= on ? 0b1000 : 0b0000;
}
