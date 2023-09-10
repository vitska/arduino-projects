#ifndef _HARDWARE_HPP_
#define _HARDWARE_HPP_

#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPping.h>
#include <MCP23017.h> // https://github.com/blemasle/arduino-mcp23017
#include <DS3231.h>
#include "rect.hpp"
#include "events.hpp"
#include "images.h"
#include "display.hpp"

class Display : public DisplayDriver {
  public:
    void clear(void){
      fillScreen(TFT_BLACK);
    }

    void print(const char* str){
      Print::print(str);
    }

    void print(int n, int base){
      Print::print((long) n, base);
    }

    void print(int16_t x, int16_t y, const char* text){
      HARDWARE_DISPLAY_DRIVER::setCursor(x, y);
      HARDWARE_DISPLAY_DRIVER::print(text);
    }

    void print(int16_t x, int16_t y, DateTime dt){
      setCursor(x, y);
      printf("%04d-%02d-%02d %02d:%02d:%02d", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
    }
};

#define KEY_MAX_COUNT 8
#define MCP23017_ADDR 0x20
class Keyboard {
  public:
    Keyboard() : mcp(MCP23017_ADDR, Wire) {}

    void init(void);
    void on_event(event_t event, EventBuffer* eventBuffer);

  private:
    MCP23017 mcp;
    uint8_t state;

    uint8_t read();
    void checkState(uint8_t current, uint8_t mask, int button, EventBuffer* eventBuffer);
};

class RTC {
  public:
    void init(void){
      Wire.begin();
    }

    DateTime now(void){
      return RTClib::now();      
    }

    void on_event(event_t event, EventBuffer* eventBuffer){}

  private:
    DS3231 _ds;
    uint8_t state;
};

class TimingController{
  public:
    void loop(EventBuffer* eventBuffer){
      uint64_t time = millis();

      createTimingEvents(eventBuffer, time / 60000, _last / 60000, EVENT_TIMING_1M);
      createTimingEvents(eventBuffer, time / 10000, _last / 10000, EVENT_TIMING_10S);
      createTimingEvents(eventBuffer, time / 1000, _last / 1000, EVENT_TIMING_1S);
      createTimingEvents(eventBuffer, time / 250, _last / 250, EVENT_BLINK);
      createTimingEvents(eventBuffer, time / 100, _last / 100, EVENT_TIMING_100MS);
      createTimingEvents(eventBuffer, time / 20, _last / 20, EVENT_TIMING_20MS);
      createTimingEvents(eventBuffer, time / 10, _last / 10, EVENT_TIMING_10MS);

      _last = time;
    }

  private:
    uint64_t _last;

    void createTimingEvents(EventBuffer* eventBuffer, uint64_t current, uint64_t last, event_t event){
      uint64_t dif = current - last;
      if(dif < 1){
        return;        
      }

      for(int i=0;i < dif;i++){
        eventBuffer->push(event);
      }
    }
};

class Network{
  public:
    void init(void){
      WiFi.begin("DALEKA-2", "senaw@78-3");
    }

    int getRSSI(){
      if(!wifi_connected()){
        return -1;
      }

      int dBm = WiFi.RSSI();

      if(dBm <= RSSI_MIN)
      {
        return 0;
      }
      else if(dBm >= RSSI_MAX)
      {  
        return 100;
      }

      return 2 * (dBm + 100);
    }

    bool wifi_connected(void){ return WiFi.status() == WL_CONNECTED; }

    float pingGateway(void){
      Ping.ping(WiFi.gatewayIP());
      return Ping.averageTime();
    }

    void on_event(event_t event, EventBuffer* eventBuffer){
      switch(event){
        case EVENT_TIMING_1S:
          {
            if(_rssi != getRSSI()){
              eventBuffer->push(EVENT_WIFI_RSSI);
            }
          }
          break;
      }
    }

  private:
    int _rssi;
    static const int RSSI_MAX =-50;// define maximum strength of signal in dBm
    static const int RSSI_MIN =-100;// define minimum strength of signal in dBm
};

class Hardware {
  public:
    Display display;
    Keyboard keyboard;
    TimingController timing;
    RTC rtc;
    Network network;

    void init(void){
      rtc.init();
      display.init();
      keyboard.init();
      network.init();
    }

    void on_event(event_t event, EventBuffer* eventBuffer){
      display.on_event(event, eventBuffer);
      keyboard.on_event(event, eventBuffer);
      network.on_event(event, eventBuffer);
    }
};

extern Hardware hardware;

#endif