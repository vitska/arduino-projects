#ifndef __BM8001B_H_
#define __BM8001B_H_

#include <Arduino.h>

class BM8001B {
public:
  BM8001B();

  void begin(void);
  void update(void);

  // =============================================== //

  // ---------------- pos = -5 -- 5 --------------//
  void trimThrottle(int8_t pos);
  void trimRudder(int8_t pos);
  void trimEler(int8_t pos);
  void trimElev(int8_t pos);
  // ---------------- pos = -5 -- 5 --------------//

  void setMode(int8_t mode); // 1 -> mode1, 2 -> mode2
  void doubleRate(int8_t dr); // 0 -> L, 1 -> H
  void bat(int8_t pos); // 0 - 4, 0 -> low, 4 -> full
  void ant(int8_t on); // 0 -> off, 1 -> on
  void quadro(int8_t on); // 0 -> off, 1 -> on
  void throtleRight(uint8_t throttle); // 0 -- 11
  void throtleLeft(uint8_t throttle); // 0 -- 11
  void elevator(int8_t pos); // -5 -- 5
  void eleron(int8_t pos); // -5 -- 5
  void rudder(int8_t pos); // -7 -- 7

private:
  void spi_send (uint8_t value, uint8_t length);
  //uint8_t setbit(const uint8_t value, const uint8_t position);
  int unsetbit(const int value, const int position);
  uint8_t checkbit(const uint8_t value, const uint8_t position);
};

#endif /* __SYMA_X5_TRANSMITTER_LCD */
