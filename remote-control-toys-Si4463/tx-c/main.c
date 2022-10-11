#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "inc/tx.h"

void setup() {
  wdt_enable(WDTO_2S);
  wdt_reset();
//  Serial.begin(9600*2); // The baudrate of Serial monitor is set in 9600
//  initialize digital pin LED_BUILTIN as an output.
//  pinMode(4, OUTPUT);
  init_state();
  init_lcd();
  init_mcu_hardware(); // Init continuous MCU ADC scan
//  init_serial();
//  scan_i2c();
//  tone(5, 5000, 100);
}

// the loop function runs over and over again forever
void loop() 
{
//  digitalWrite(4, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(20);                       // wait for a second
//  digitalWrite(4, LOW);    // turn the LED off by making the voltage LOW
//  delay(400);                       // wait for a second
//  update_lcd();

//  Serial.print("ADC=");
//  Serial.print(state.matrix[0], DEC);
//  Serial.print(",");
//  Serial.print(state.matrix[1], DEC);
//  Serial.print(",");
//  Serial.print(state.matrix[2], DEC);
//  Serial.print(",");
//  Serial.print(state.matrix[3], DEC);
//  Serial.print(",");
//  Serial.print(state.matrix[4], DEC);
//  Serial.print(",");
//  Serial.print(state.matrix[5], DEC);
//  Serial.print(",");
//  Serial.print(state.matrix[6], DEC);
//  Serial.print(",");
//  Serial.println(state.matrix[7], DEC);

  controller(wait_event());
  wdt_reset();
}

int main(void){
    setup();
    while(1)
    {
        loop();
    }
}