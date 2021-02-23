#include <string.h>
#include "state.h"
#include "model_config.h"

volatile DEVICE_STATE state;
STORED_CONFIG config;

void init_state(){
  memset((void*)&state, 0, sizeof(state));
  put_event(EV_STARTUP);
}

void put_event(char event)
{
  state.events.buf[state.events.in++] = event;
  
  if(state.events.in >= sizeof(state.events.buf))
  {
    state.events.in = 0;
  }
  
  if(state.events.in == state.events.out)
  {
    state.flag_buffer_overflow = 1;
  }
}

char wait_event()
{
  while(state.events.in == state.events.out);
  
  register char event = state.events.buf[state.events.out++];
  if(state.events.out >= sizeof(state.events.buf))
  {
    state.events.out = 0;
  }
  
  return event;
}
