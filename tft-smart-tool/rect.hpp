#ifndef _RECT_HPP_
#define _RECT_HPP_

class Point{
  public:
    int16_t x() { return _x; }
    void setX(int16_t x) { _x = x; }
    int16_t y() { return _y; }
    void setY(int16_t y) { _y = y; }
    void set(int16_t x, int16_t y){ setX(x); setY(y); }
    void set(Point* point){ set(point->_x, point->_y); }

  private:
    int16_t _x, _y;
};

class Size{
  public:
    int16_t width() { return _w; }
    void setWidth(int16_t w) { _w = w; }
    int16_t height() { return _h; }
    void setHeight(int16_t h) { _h = h; }

  private:
    int16_t _w, _h;
};

class Rect : public Point, public Size {
  public:
    Rect(int16_t x = 0, int16_t y = 0, int16_t w = 0, int16_t h = 0){
      set(x, y, w, h);
    }

    void set(int16_t x, int16_t y, int16_t w, int16_t h){
      setX(x);
      setY(y);
      setWidth(w);
      setHeight(h);
    }

    void set(Rect* rect){
      set(rect->x(), rect->y(), rect->width(), rect->height());
    }

    void limit(Point* point){
      if(point->x() < x()){
        point->setX(x());
      }

      if(point->y() < y()){
        point->setY(y());
      }
    }
};

#endif