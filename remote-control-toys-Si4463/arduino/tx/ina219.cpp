#include "ina219.h"
#include <Wire.h>

INA219::INA219()
    : _i2caddr(INA219_ADDR), _i2c(&Wire) {}

int16_t INA219::getShuntVoltage(){
  return read_int16(INA219_ShuntVoltage);
}

uint16_t INA219::getBusVoltagemV(){
  // At full-scale range = 16 V (decimal = 4000, hex = 0FA0), and LSB = 4 mV.
  // 0 0001 1111 0100 000
  return (((uint16_t)read_int16(INA219_BusVoltage)) >> 3) * 4;
}

uint16_t INA219::getPowermW(){
  return (uint16_t)read_int16(INA219_Power) * 20;
}

uint16_t INA219::getCurrentmA(){
  return (uint16_t)read_int16(INA219_Current);
}

// 18:44:42.494 -> ShuntVoltage:[30] busVoltage:[10000001110 010] current:[155]
// 18:44:42.559 -> ShuntVoltage:[31] busVoltage:[10000001111 010] current:[155]

void INA219::begin(uint16_t cfg, uint16_t calibration){
  _i2c->beginTransmission(_i2caddr);
  _i2c->write(INA219_Configuration);
  _i2c->write(cfg >> 8);
  _i2c->write(cfg & 0xFF);
  _i2c->endTransmission();
  _i2c->beginTransmission(_i2caddr);
  _i2c->write(INA219_Calibration);
  _i2c->write(calibration >> 8);
  _i2c->write(calibration & 0xFF);
  _i2c->endTransmission();
}

int16_t INA219::read_int16(uint8_t addr){
  _i2c->beginTransmission(_i2caddr);
  _i2c->write(addr);
  _i2c->endTransmission();
  _i2c->requestFrom(_i2caddr, (uint8_t)2);
  uint8_t msb = _i2c->read();
  uint8_t lsb = _i2c->read();
  _i2c->endTransmission();
  return (int16_t)((uint16_t)lsb | ((uint16_t)msb) << 8);
}