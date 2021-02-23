#include <avr/interrupt.h>
#include "state.h"
#include "hardware.h"

ISR(SOUND_COMPARE_VECTOR)
{
    if(state.machine.sound.period == 0){
        return;
    }
    DIGITAL_5_PORT ^= DIGITAL_5_PIN;
    OCR0A += state.machine.sound.period;
    if(state.machine.sound.delay_counter-- == 0){
        state.machine.sound.period = 0;
        put_event(EV_BEEP_END);
    }
}

void init_sound(){
//  sound(0xA0, 2, 0);
}

void sound(uint8_t period, uint8_t duration, uint8_t beeps){
    SOUND_TIMER_COMPARE_ENABLE; 
    state.machine.sound.period = 0xFF - period;
    state.machine.sound.delay_counter = (uint16_t)duration * state.machine.sound.period;
}

void sound_off(){
    SOUND_TIMER_COMPARE_DISABLE; 
    state.machine.sound.period = 0;
}