#ifndef _EVENTS_HPP_
#define _EVENTS_HPP_

typedef uint16_t event_t;

#define EVENT_BUFFER_SIZE 128

#define EVENT_NONE 0
#define EVENT_TIMING_1S 0x1
#define EVENT_TIMING_10S 0x2
#define EVENT_TIMING_1M 0x3
#define EVENT_BLINK 0x4
#define EVENT_TIMING_100MS 0x5
#define EVENT_TIMING_20MS 0x6
#define EVENT_TIMING_10MS 0x7

#define EVENT_SCREEN_INIT 0x10
#define EVENT_WIFI_RSSI 0x11
#define EVENT_TOUCH 0x20

#define EVENT_BUTTON 0x30
#define EVENT_BUTTON1_DOWN EVENT_BUTTON + 0
#define EVENT_BUTTON1_UP EVENT_BUTTON + 1
#define EVENT_BUTTON2_DOWN EVENT_BUTTON + 2
#define EVENT_BUTTON2_UP EVENT_BUTTON + 3
#define EVENT_BUTTON3_DOWN EVENT_BUTTON + 4
#define EVENT_BUTTON3_UP EVENT_BUTTON + 5
#define EVENT_BUTTON4_DOWN EVENT_BUTTON + 6
#define EVENT_BUTTON4_UP EVENT_BUTTON + 7
#define EVENT_BUTTON5_DOWN EVENT_BUTTON + 8
#define EVENT_BUTTON5_UP EVENT_BUTTON + 9
#define EVENT_BUTTON6_DOWN EVENT_BUTTON + 10
#define EVENT_BUTTON6_UP EVENT_BUTTON + 11
#define EVENT_BUTTON7_DOWN EVENT_BUTTON + 12
#define EVENT_BUTTON7_UP EVENT_BUTTON + 13
#define EVENT_BUTTON8_DOWN EVENT_BUTTON + 14
#define EVENT_BUTTON8_UP EVENT_BUTTON + 15
#define EVENT_BUTTON_LAST EVENT_BUTTON8_UP

class EventBuffer{
  public:
    EventBuffer(){
      _start = 0;
      _end = 0;
    }

    void push(event_t event){
      _buffer[_start] = event;
      _start++;
      if(_start >= EVENT_BUFFER_SIZE){
        _start = 0;
      }
    }
    
    event_t pop(){
      if(_start == _end){
        return EVENT_NONE;
      }

      event_t result = _buffer[_end];
      _end++;
      if(_end >= EVENT_BUFFER_SIZE){
        _end = 0;
      }

      return result;
    }

  private:
    event_t _buffer[128];
    uint8_t _start;
    uint8_t _end;
};

#endif