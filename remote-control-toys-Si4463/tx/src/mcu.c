#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "model_config.h"
#include "state.h"
#include "hardware.h"

void count_for_event(volatile uint8_t* counter, uint8_t reload, uint8_t event){
  if((*counter)++ >= reload)
  {
    (*counter) = 0;
    put_event(event);
  }
}

const uint8_t t0_ovf_reload_20ms = MS_TO_RELOAD_INT8_VALUE(20);
const uint8_t t0_ovf_reload_100ms = MS_TO_RELOAD_INT8_VALUE(100);
const uint8_t t0_ovf_reload_500ms = MS_TO_RELOAD_INT8_VALUE(500);

ISR(TIMING_OVERFLOW_VECTOR)
{
  count_for_event(&state.counters.t0_ovf_20ms, t0_ovf_reload_20ms, EV_CLOCK_20MS);
  count_for_event(&state.counters.t0_ovf_100ms, t0_ovf_reload_100ms, EV_CLOCK_100MS);
  count_for_event(&state.counters.t0_ovf_500ms, t0_ovf_reload_500ms, EV_CLOCK_500MS);
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


void init_mcu_hardware()
{
  LED_PIN_OUTPUT;
  SOUND_PIN_OUTPUT;
  init_adc();
  INIT_TIMING_TIMER;
  sei();
  init_sound();
  init_blink();
}
