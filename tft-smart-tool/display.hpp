#ifndef _DISPLAY_HPP_
#define _DISPLAY_HPP_

#include <MCUFRIEND_kbv.h> // https://github.com/prenticedavid/MCUFRIEND_kbv

#define HARDWARE_DISPLAY_DRIVER MCUFRIEND_kbv

class DisplayDriver : public HARDWARE_DISPLAY_DRIVER{
  public:
    void fillRect(Rect* rect, uint16_t color){
      HARDWARE_DISPLAY_DRIVER::fillRect(rect->x(), rect->y(), rect->width(), rect->height(), color);
    }

    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
      HARDWARE_DISPLAY_DRIVER::fillRect(x, y, w, h, color);
    }

    void drawBitmap(int16_t x, int16_t y, const tImage* image, uint16_t color){
      if(image == NULL){
        return;
      }

      HARDWARE_DISPLAY_DRIVER::drawBitmap(x, y, image->data, image->width, image->height, color);
    }

    void drawImage(int16_t x, int16_t y, const tImage* image){
      if(image == NULL){
        return;
      }

      HARDWARE_DISPLAY_DRIVER::drawRGBBitmap(x, y, (const uint16_t*)image->data, image->width, image->height);
    }

    void init(void){
      begin(readID());
      fillScreen(TFT_NAVY);
    }

    void on_event(event_t event, EventBuffer* eventBuffer){
      switch(event){
        case EVENT_TIMING_10MS:
          {
            // setCursor(0, 30);
            // setTextColor(TFT_CYAN, TFT_BLACK);

            // printf("[%d]-[%d]-[%d]-[%d]",scrolling_direction, scrolloffset, scroll_top, scroll_lines);

            if(scrolling_direction == 0){
              break;
            }

            scrolloffset += scrolling_direction;
            if(scrolling_direction < 0){
              if(scrolloffset < 0){
                scrolloffset = scroll_lines;
              }
            } else if(scrolling_direction > 0){
              if(scrolloffset > scroll_lines){
                scrolloffset = 0;
              }
            }
            vertScroll(scroll_top, scroll_lines, scrolloffset);
          }
          break;
      }      
    }

    uint16_t readID(void) {
        uint16_t ID = HARDWARE_DISPLAY_DRIVER::readID();
        if (ID == 0xD3D3) ID = 0x9486;
        return ID;
    }

    void vscroll(int top, int bottom, int direction){
      scrolling_direction = direction;
      scrolloffset = 0;
      scroll_top = top;
      scroll_bottom = bottom;
      if(scroll_bottom < 0){
        scroll_bottom = height();
      }
      scroll_lines = scroll_bottom - scroll_top;
      scrolloffset += scrolling_direction;

      if(direction == 0){
        vertScroll(scroll_top, scroll_lines, scrolloffset);
      }
    }

    int vscroll_direction(void){ return scrolling_direction; }

  private:
    int scrolloffset;
    int scrolling_direction;
    int scroll_top;
    int scroll_bottom;
    int scroll_lines;
};

#endif