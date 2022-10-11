#include "LCD_baomei_bm-8001b.h"
#include "stdint.h"
#include "state.h"

struct lcd_t{
  int8_t trim:4;
  uint8_t bat:3;
  uint8_t mode:1;
  uint8_t dr:1;
  uint8_t ant:1;
  int8_t quadro:1;
//  uint8_t throttle:4;
  int8_t elev:4;
  int8_t eler:4;
  int8_t rudder:4;
};

struct lcd_t lcd;

void init_lcd(){
  LCD_Init();
}

void lcd_demo(){
  Bat(lcd.bat);
  if(lcd.bat++ > 4){
    lcd.bat = 0;
  }
  TrimThrottle(lcd.trim);
  TrimRudder(lcd.trim);
  TrimEler(lcd.trim);
  TrimElev(lcd.trim);
  if(lcd.trim++ > 5){
    lcd.trim = -5;
  }
  SetMode(lcd.mode);
  if(lcd.mode++ > 2){
     lcd.mode = 1;
  }
  DoubleRate(lcd.dr++);
  Ant(lcd.ant++);
  Quadro(lcd.quadro++);
  // if(lcd.quadro > 1){
  //   lcd.quadro = 0;
  // }
  ThrotleLeft(((state.matrix[VM_AIN4] * 10) / 1024) + 5);
  ThrotleRight(((state.matrix[VM_AIN2] * 10) / 1024) + 5);
//  if(lcd.throttle++ > 11){
//     lcd.throttle = 0;
//  }
  // Elevator(lcd.elev++); // -5 -- 5
  // if(lcd.elev > 5){
  //   lcd.elev = -5;
  // }
  // Eleron(lcd.eler++); // -5 -- 5
  // if(lcd.eler > 5){
  //   lcd.eler = -5;
  // }
  Rudder(lcd.rudder);
  if(lcd.rudder++ > 7){
    lcd.rudder = -7;
  }
}

void update_lcd(){
  lcd_demo();
  LCD_Update();
}
