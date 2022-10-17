#include "mcp23017.h"
#include <Wire.h>

MCP23017::MCP23017()
    : _i2caddr(MCP23017_ADDR), _i2c(&Wire) {}

void MCP23017::setPullup(uint8_t port, uint8_t mask){
  _i2c->beginTransmission(_i2caddr);
  _i2c->write(MCP23017_GPPUA + port);
  _i2c->write(mask);// Enable pullups on port A  
  _i2c->endTransmission();
}

uint8_t MCP23017::readPort(uint8_t port){
  _i2c->beginTransmission(_i2caddr); // select device with "beginTransmission()"
  _i2c->write(MCP23017_GPIOA + port); // select starting register with "write()"
  //Wire.endTransmission(); // end write operation, as we just wanted to select the starting register
  _i2c->requestFrom(_i2caddr, (uint8_t)1); // select number of bytes to get from the device (2 bytes in this case)
  uint8_t val = _i2c->read(); // read from the starting register
  _i2c->endTransmission(); // end write operation, as we just wanted to select the starting register
  return val;
}
