#ifndef _ADAFRUIT_GFX_AS_H
#define _ADAFRUIT_GFX_AS_H

#include "Load_fonts.h"
#include <c_types.h>

typedef struct  {
  int16_t WIDTH;
  int16_t HEIGHT;   // This is the 'raw' display w/h - never changes
  int16_t _width;
  int16_t _height; // Display w/h as modified by current rotation
  uint16_t textcolor;
  uint16_t textbgcolor;
  uint8_t rotation;
  bool wrap; // If set, 'wrap' text at right edge of display
} ili9341_lcd;
// class Adafruit_GFX_AS {
//  public:
  //Adafruit_GFX_AS(int16_t w, int16_t h); // Constructor

  // This MUST be defined by the subclass:
  //virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

  // These MAY be overridden by the subclass to provide device-specific
  // optimized code.  Otherwise 'generic' versions are used.
  
  void adafruit_ili9341_drawLine(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  void adafruit_ili9341_drawRect(ili9341_lcd *lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);


  // These exist only with Adafruit_GFX_AS (no subclass overrides)
  void drawCircle(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t r, uint16_t color);
  void drawCircleHelper(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
      uint16_t color);
  void fillCircle(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t r, uint16_t color);
  void fillCircleHelper(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
      int16_t delta, uint16_t color);
  void drawTriangle(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
      int16_t x2, int16_t y2, uint16_t color);
  void fillTriangle(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
      int16_t x2, int16_t y2, uint16_t color);
  void drawRoundRect(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t w, int16_t h,
      int16_t radius, uint16_t color);
  void fillRoundRect(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t w, int16_t h,
      int16_t radius, uint16_t color);
  void drawBitmap(ili9341_lcd *lcd, int16_t x, int16_t y, const uint16_t *bitmap,
      int16_t w, int16_t h);
  //void setTextColor(ili9341_lcd *lcd, uint16_t c);
  void setTextColor(ili9341_lcd *lcd, uint16_t c, uint16_t bg);
  void setTextWrap(ili9341_lcd *lcd, bool w);
  void setRotation(ili9341_lcd *lcd, uint8_t r);

  int drawUnicode(ili9341_lcd *lcd, uint16_t uniCode, uint16_t x, uint16_t y, uint8_t size);
  int drawNumber(ili9341_lcd *lcd, long long_num,uint16_t poX, uint16_t poY, uint8_t size);
  int drawChar(ili9341_lcd *lcd, char c, uint16_t x, uint16_t y, uint8_t size);
  int drawString(ili9341_lcd *lcd, const char *string, uint16_t poX, uint16_t poY, uint8_t size);
  int drawCentreString(ili9341_lcd *lcd, const char *string, uint16_t dX, uint16_t poY, uint8_t size);
  int drawRightString(ili9341_lcd *lcd, const char *string, uint16_t dX, uint16_t poY, uint8_t size);
  int drawFloat(ili9341_lcd *lcd, float floatNumber,uint8_t decimal,uint16_t poX, uint16_t poY, uint8_t size);

  int16_t height(void);
  int16_t width(void);

  uint8_t getRotation(void);


#endif // _ADAFRUIT_GFX_AS_H
