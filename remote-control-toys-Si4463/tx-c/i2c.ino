#include <Wire.h>

void scan_i2c(){
  Wire.begin(); // Wire communication begin

  Serial.print("I2C=");
  bool comma = false;
  for (uint8_t address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();

    if (error == 0)
    {
      if (comma)
      {
        Serial.print(",");
      }
      Serial.print(address, HEX);
      comma = true;
    }
  }
  Serial.println();
}
