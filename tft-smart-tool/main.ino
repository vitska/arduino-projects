#include "screens.hpp"
#include "images.h"

void printIcon(int x, int y, const tImage* icon, const char* label){
  hardware.display.drawImage(x, y, icon);
  hardware.display.print(x + 10, y + 64 + 3, label);
}

void MainScreen::redraw(){
  clear();
  hardware.display.drawImage(0, 0, Images::wallpaper());
  hardware.display.drawImage(20, 20, Images::test_rgb_image());
  hardware.display.fillRect(0, 0, hardware.display.width(), 11, TFT_BLACK);
  
  int x = 15, y = 32;
  printIcon(x, y, Images::getIcon(0), "Label 1");
  x += 74;
  printIcon(x, y, Images::getIcon(1), "Label 2");
  x += 74;
  printIcon(x, y, Images::getIcon(0), "Label 3");

  x = 15; y += 77;
  printIcon(x, y, Images::getIcon(1), "Label 4");
  x += 74;
  printIcon(x, y, Images::getIcon(0), "Label 5");
  x += 74;
  printIcon(x, y, Images::getIcon(1), "Label 6");

  x = 15; y += 77;
  printIcon(x, y, Images::getIcon(0), "Label 7");
  x += 74;
  printIcon(x, y, Images::getIcon(1), "Label 8");
  x += 74;
  printIcon(x, y, Images::getIcon(0), "Label 9");

  println("Blah blah blah");
  println("AAAA AAAA AAAA");
  hardware.display.printf("ID:[%X] width:[%d] height:[%d]", hardware.display.readID(), hardware.display.width(), hardware.display.height());

  hardware.display.vscroll(11, -1, -2);
}

void MainScreen::blink(bool state){
  hardware.display.setTextColor(TFT_CYAN, TFT_BLACK);
  if(state){
    hardware.display.print(hardware.display.width() - 117, 2, hardware.rtc.now());
  } else {
    hardware.display.print(hardware.display.width() - 117, 2, "           ");
  }
}

void MainScreen::on_event(event_t event, EventBuffer* eventBuffer){
  Screen::on_event(event, eventBuffer);
  switch(event){
    // case EVENT_TIMING_1S:
    //   {
    //     hardware.display.drawRect(0, 0, hardware.display.width(), 11, TFT_DARKGREY);
    //     hardware.display.drawFastVLine(hardware.display.width() - 119, 1, 9, TFT_DARKGREY);
    //   }
    //   break;

    case EVENT_WIFI_RSSI:
      {
        int rssi = hardware.network.getRSSI();

        hardware.display.drawBitmap(2, 2, Images::getSignalImage(100), TFT_DARKGREY);
        if(rssi < 0){
          hardware.display.drawBitmap(2, 2, Images::no_signal(), TFT_RED);
        } else {
          hardware.display.drawBitmap(2, 2, Images::getSignalImage(rssi), TFT_WHITE);
        }
        hardware.display.setTextColor(TFT_WHITE, TFT_BLACK);
        hardware.display.setCursor(17, 2);
        hardware.display.printf("%d%%", rssi);
      }
      break;

    case EVENT_TIMING_1M:
      {
        hardware.display.setCursor(48, 2);
        if(hardware.network.wifi_connected()){
          hardware.display.printf("%.1fms", hardware.network.pingGateway());
        } else {
          hardware.display.print("-----");
        }
      }
      break;
  }  
}

typedef struct {
  int x;
  int y;
} button_position;
const uint16_t button_colors[8] = {TFT_RED, TFT_BLUE, TFT_CYAN, TFT_GREEN, TFT_LIGHTGREY, TFT_MAGENTA, TFT_MAROON, TFT_ORANGE};
const button_position button_positions[8] = {{0, 350}, {0, 285}, {0, 220}, {0, 155}, {215, 50}, {215, 0}, {0, 0}, {0, 0}};

int rotation = 0;

void MainScreen::button_down(int button){
  hardware.display.fillRoundRect(button_positions[button].x, button_positions[button].y, 15, 50, 3, button_colors[button]);
  hardware.display.setCursor(button_positions[button].x, button_positions[button].y + 20);
  hardware.display.setTextColor(TFT_BLACK, 0xFFFF);
  hardware.display.print(button, DEC);

  switch(button){
    case 1:
      hardware.display.vertScroll(0, hardware.display.height(), 30);
      break;
    case 2:
      hardware.display.setRotation(++rotation);
      if(rotation > 3){
        rotation = 0;
      }
      break;
    case 4:
      switch(hardware.display.vscroll_direction()){
        case 0: hardware.display.vscroll(11, -1, 1); break;
        case 1: hardware.display.vscroll(11, -1, -1); break;
        case -1: hardware.display.vscroll(11, -1, 0); break;
      }
      break;
  }
}

void MainScreen::button_up(int button){
  hardware.display.fillRoundRect(button_positions[button].x, button_positions[button].y, 15, 50, 3, TFT_BLACK);
}

MainScreen mainScreen;
