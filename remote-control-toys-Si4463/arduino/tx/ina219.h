#ifndef _INA219_H
#define _INA219_H

#include <Arduino.h>
#include <Wire.h>

#define INA219_ADDR 0x40

// REGISTER ADDRESSES
#define INA219_Configuration 0x00 // Configuration voltage range resolution/averaging., PGA Gain, ADC 00111001 10011111 399F R/W
#define INA219_ShuntVoltage 0x01 // Shunt voltage Shunt voltage measurement data. Shunt voltage — R
#define INA219_BusVoltage 0x02 // Bus voltage Bus voltage measurement data. Bus voltage — R
#define INA219_Power 0x03 // Power(2) Power measurement data. 00000000 00000000 0000 R
#define INA219_Current 0x04 // Current(2) // Contains the value of the current flowing through the shunt resistor. 00000000 00000000 0000 R
#define INA219_Calibration 0x05 // Calibration Sets full-scale range and LSB of current and power measurements. Overall system calibration. 00000000 00000000 0000 R/W

#define CFG_RST 15
#define CFG_BRNG 13
#define CFG_PG1 12
#define CFG_PG0 11
#define CFG_BADC4 10
#define CFG_BADC3 9
#define CFG_BADC2 8
#define CFG_BADC1 7
#define CFG_SADC4 6
#define CFG_SADC3 5
#define CFG_SADC2 4
#define CFG_SADC1 3
#define CFG_MODE3 2
#define CFG_MODE2 1
#define CFG_MODE1 0

#define CFG_PG_G1_40MV 0x0000
#define CFG_PG_G1_80MV 0x0800
#define CFG_PG_G1_160MV 0x1000
#define CFG_PG_G1_320MV 0x1800

#define CFG_BUS_VOLTAGE_RANGE_16V 0x0000
#define CFG_BUS_VOLTAGE_RANGE_32V 0x2000

#define CFG_BUS_BADC_9BIT 0x0000
#define CFG_BUS_BADC_10BIT 0x0080
#define CFG_BUS_BADC_11BIT 0x0100
#define CFG_BUS_BADC_12BIT 0x0180
#define CFG_BUS_BADC_2SAMPLES 0x0480
#define CFG_BUS_BADC_4SAMPLES 0x0500
#define CFG_BUS_BADC_8SAMPLES 0x0580
#define CFG_BUS_BADC_16SAMPLES 0x0600
#define CFG_BUS_BADC_32SAMPLES 0x0680
#define CFG_BUS_BADC_64SAMPLES 0x0700
#define CFG_BUS_BADC_128SAMPLES 0x0780

#define CFG_SHUNT_SADC_9BIT 0x0000
#define CFG_SHUNT_SADC_10BIT 0x0008
#define CFG_SHUNT_SADC_11BIT 0x0010
#define CFG_SHUNT_SADC_12BIT 0x0018
#define CFG_SHUNT_SADC_2SAMPLES 0x0048
#define CFG_SHUNT_SADC_4SAMPLES 0x0050
#define CFG_SHUNT_SADC_8SAMPLES 0x0058
#define CFG_SHUNT_SADC_16SAMPLES 0x0060
#define CFG_SHUNT_SADC_32SAMPLES 0x0068
#define CFG_SHUNT_SADC_64SAMPLES 0x0070
#define CFG_SHUNT_SADC_128SAMPLES 0x0078

#define CFG_MODE 0x0000
#define CFG_SHUNT_SADC_10BIT 0x0008
#define CFG_SHUNT_SADC_11BIT 0x0010
#define CFG_SHUNT_SADC_12BIT 0x0018
#define CFG_SHUNT_SADC_2SAMPLES 0x0048
#define CFG_SHUNT_SADC_4SAMPLES 0x0050
#define CFG_SHUNT_SADC_8SAMPLES 0x0058
#define CFG_SHUNT_SADC_16SAMPLES 0x0060
#define CFG_SHUNT_SADC_32SAMPLES 0x0068
#define CFG_SHUNT_SADC_64SAMPLES 0x0070
#define CFG_SHUNT_SADC_128SAMPLES 0x0078
//                   P GBAD C 
// Default 399F:  0011 1001 1001 1111 = BRNG:1 PG:11 BADC:0011 SADC: 0011 Mode:111
// App note 019F: 0000 0001 1001 1111 = BRNG:0 PG:00 BADC:0011 SADC: 0011 Mode:111
// 0000 0111 1111 1111

class INA219 {
public:
  INA219();

  void INA219::begin(uint16_t cfg, uint16_t calibration);
  int16_t getShuntVoltage();
  uint16_t getBusVoltagemV();
  uint16_t getPowermW();
  uint16_t getCurrentmA();
private:
  uint8_t _i2caddr;
  TwoWire *_i2c;

  int16_t read_int16(uint8_t addr);
};

#endif