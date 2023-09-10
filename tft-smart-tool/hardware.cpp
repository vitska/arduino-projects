#include "hardware.hpp"

Hardware hardware;

void Keyboard::init(void){
  Wire.begin();
  mcp.init();
  mcp.portMode(MCP23017Port::A, 0);          //Port A as output
  mcp.portMode(MCP23017Port::B, 0b11111111); //Port B as input
  mcp.writeRegister(MCP23017Register::GPPU_B, 0b11111111);  //Enable inputs pullups
  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A 
  mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B
  state = 0b11111111;
}

void Keyboard::on_event(event_t event, EventBuffer* eventBuffer){
  switch(event){
    case EVENT_TIMING_100MS:
      uint8_t current = read();
      if(current != state){
        checkState(current, 0b10000000, 0, eventBuffer);
        checkState(current, 0b01000000, 1, eventBuffer);
        checkState(current, 0b00100000, 2, eventBuffer);
        checkState(current, 0b00010000, 3, eventBuffer);
        checkState(current, 0b00001000, 4, eventBuffer);
        checkState(current, 0b00000100, 5, eventBuffer);
        checkState(current, 0b00000010, 6, eventBuffer);
        checkState(current, 0b00000001, 7, eventBuffer);
      }
      state = current;
      break;
  }
}

uint8_t Keyboard::read(){
  return mcp.readRegister(MCP23017Register::GPIO_B);
}

void Keyboard::checkState(uint8_t current, uint8_t mask, int button, EventBuffer* eventBuffer){
  if(((current & mask) == 0) && ((state & mask) == mask)){
    eventBuffer->push(EVENT_BUTTON + (button * 2));
  }

  if(((current & mask) == mask) && ((state & mask) == 0)){
    eventBuffer->push(EVENT_BUTTON + (button * 2) + 1);
  }
}
