#include <Adafruit_GFX.h> // https://github.com/adafruit/Adafruit-GFX-Library
#include "hardware.hpp"
#include "screens.hpp"

App app;

extern MainScreen mainScreen;
ViewElement* screens[] = { &mainScreen, NULL };

void setup() {
  hardware.init();
  app.init(screens);
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  app.loop();
}
