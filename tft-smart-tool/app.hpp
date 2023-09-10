#ifndef _APP_HPP_
#define _APP_HPP_

#include "rect.hpp"
#include "events.hpp"

class ViewElement{
  public:
    void setView(Rect* rect){
      _rect.set(rect);
      _cursor.set(rect);
    }

    void clear(){
      hardware.display.fillRect(&_rect, TFT_BLACK);
    }

    void println(const char* text){
      hardware.display.setCursor(_cursor.x(), _cursor.y());
      hardware.display.println(text);
      _cursor.set(hardware.display.getCursorX(), hardware.display.getCursorY());
      _rect.limit(&_cursor);
    }

    virtual void redraw() = 0;
    virtual void button_down(int button){}
    virtual void button_up(int button){}
    virtual void blink(bool state){}
    virtual void on_event(event_t event, EventBuffer* eventBuffer){
      if((event >= EVENT_BUTTON) && (event <= EVENT_BUTTON_LAST)){
        if((event & 0x1) == 0x1){
          button_up((event - EVENT_BUTTON) >> 1);
        } else {
          button_down((event - EVENT_BUTTON) >> 1);
        }
      }

      if(event == EVENT_BLINK){
        _blink_state = !_blink_state;
        blink(_blink_state);
      }
    }

  private:
    bool _blink_state;
    Rect _rect;
    Point _cursor;


};

class ScreenManager{
  public:

};

class App {
  public:
    void init(ViewElement** screens){
      _active = screens[0];
      Rect rect(10, 10, hardware.display.width()-20, hardware.display.height()-20);
      _active->setView(&rect);
      _active->redraw();
      keyboardState = 0;
    }

    uint8_t keyboardState;
    uint32_t counter;

    void loop(){
      hardware.timing.loop(&_eventBuffer);
//--------- DEBUG --------------
      // uint8_t current = hardware.keyboard.read();
      // if(current != keyboardState){
      //   // Rect textRect(50, 50, 100, 20);
      //   // hardware.display.fillRect(&textRect, TFT_BLACK);
      //   // hardware.display.setCursor(50,50);
      //   // hardware.display.println(current, BIN);
      //   keyboardState = current;
      // }

      event_t event;
      while((event = _eventBuffer.pop()) != EVENT_NONE){
        publish(event);
      }
    }

  private:
    ViewElement* _active;
    EventBuffer _eventBuffer;

    void publish(event_t event){
      // hardware.display.setCursor(50, 150 + event*10);
      // hardware.display.setTextColor(TFT_YELLOW, TFT_BLACK);
      // hardware.display.println(counter++);

      hardware.on_event(event, &_eventBuffer);
      _active->on_event(event, &_eventBuffer);
    }
};

class Button : public ViewElement{
  public:
    virtual void redraw(){

    }
    virtual void on_event(event_t event, EventBuffer* eventBuffer){

    }
};

class Screen : public ViewElement{
  public:
};

#endif