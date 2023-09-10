#ifndef _SCREENS_HPP_
#define _SCREENS_HPP_

#include "app.hpp"

class MainScreen : public Screen {
  public:
    virtual void redraw();
    virtual void on_event(event_t event, EventBuffer* eventBuffer);
    virtual void button_down(int button);
    virtual void button_up(int button);
    virtual void blink(bool state);

  private:
    Button button1;
};

#endif