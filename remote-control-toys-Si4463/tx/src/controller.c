#include "state.h"
#include "hardware.h"
#include "lcd.h"
#include "serial.h"

void controller(uint8_t event){
  switch(event){
    case EV_STARTUP:
      serial_write_block((void*)&state.matrix, sizeof(state.matrix), 'A');
      break;

    case EV_CLOCK_500MS:
      blink(&state.machine.blink, 2, 2);
      break;

    case EV_CLOCK_100MS:
      update_lcd();
      break;

    case EV_CLOCK_20MS:
      blink_step(&state.machine.blink);
      break;

    case EV_BEEP_END:
      break;
  }
}
