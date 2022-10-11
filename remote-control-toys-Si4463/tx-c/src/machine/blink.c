#include "state.h"
#include "hardware.h"

void blink_step(volatile BLINK_MACHINE* machine){
    if(machine->counter == 0){
        //disabled
        return;
    }
    if(machine->delay_counter++ >= machine->delay){
        machine->delay_counter = 0;
        machine->counter--;
        LED_PORT ^= LED_PIN;
    }
    if(LED_PIN_TEST){ // On is twise shorter
        machine->delay_counter+=2;
    }
}

void init_blink(){
    blink(&state.machine.blink, 4, 1);
}

void blink(volatile BLINK_MACHINE* machine, uint8_t delay, uint8_t count){
    LED_PORT |= LED_PIN;
    machine->delay = delay;
    machine->counter = (count * 2) - 1;
    machine->delay_counter = 0;
}