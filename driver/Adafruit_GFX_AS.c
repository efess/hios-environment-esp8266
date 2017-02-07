/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */

#include "driver/Adafruit_GFX_AS.h"

#include "driver/mini-printf.h"

#ifdef LOAD_FONT2
#include "driver/Font16.h"
#endif

#ifdef LOAD_FONT4
#include "driver/Font32.h"
#endif

#ifdef LOAD_FONT6
#include "driver/Font64.h"
#endif
//
#ifdef LOAD_FONT7
#include "driver/Font7s.h"
#endif

#define abs(x) ((x)<0 ? -(x) : (x))
#define swap(x, y) do { int16_t temp##x##y = x; x = y; y = temp##x##y; } while (0)

// void ICACHE_FLASH_ATTR Adafruit_GFX_AS::Adafruit_GFX_AS(int16_t w, int16_t h):
//           WIDTH(w), HEIGHT(h)
// {
//     _width    = WIDTH;
//     _height   = HEIGHT;
//     rotation  = 0;
//     textcolor = textbgcolor = 0xFFFF;
//     wrap      = true;
// }

// Draw a circle outline
void ICACHE_FLASH_ATTR drawCircle(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t r,
        uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    adafruit_ili9341_drawPixel(lcd, x0  , y0+r, color);
    adafruit_ili9341_drawPixel(lcd, x0  , y0-r, color);
    adafruit_ili9341_drawPixel(lcd, x0+r, y0  , color);
    adafruit_ili9341_drawPixel(lcd, x0-r, y0  , color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        adafruit_ili9341_drawPixel(lcd, x0 + x, y0 + y, color);
        adafruit_ili9341_drawPixel(lcd, x0 - x, y0 + y, color);
        adafruit_ili9341_drawPixel(lcd, x0 + x, y0 - y, color);
        adafruit_ili9341_drawPixel(lcd, x0 - x, y0 - y, color);
        adafruit_ili9341_drawPixel(lcd, x0 + y, y0 + x, color);
        adafruit_ili9341_drawPixel(lcd, x0 - y, y0 + x, color);
        adafruit_ili9341_drawPixel(lcd, x0 + y, y0 - x, color);
        adafruit_ili9341_drawPixel(lcd, x0 - y, y0 - x, color);
    }
}

void ICACHE_FLASH_ATTR drawCircleHelper(ili9341_lcd *lcd, int16_t x0, int16_t y0,
        int16_t r, uint8_t cornername, uint16_t color) {
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
            adafruit_ili9341_drawPixel(lcd, x0 + x, y0 + y, color);
            adafruit_ili9341_drawPixel(lcd, x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            adafruit_ili9341_drawPixel(lcd, x0 + x, y0 - y, color);
            adafruit_ili9341_drawPixel(lcd, x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            adafruit_ili9341_drawPixel(lcd, x0 - y, y0 + x, color);
            adafruit_ili9341_drawPixel(lcd, x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            adafruit_ili9341_drawPixel(lcd, x0 - y, y0 - x, color);
            adafruit_ili9341_drawPixel(lcd, x0 - x, y0 - y, color);
        }
    }
}

void ICACHE_FLASH_ATTR fillCircle(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t r,
        uint16_t color) {
    adafruit_ili9341_drawFastVLine(x0, y0-r, 2*r+1, color);
    fillCircleHelper(lcd, x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void ICACHE_FLASH_ATTR fillCircleHelper(ili9341_lcd *lcd, int16_t x0, int16_t y0, int16_t r,
        uint8_t cornername, int16_t delta, uint16_t color) {

    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            adafruit_ili9341_drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
            adafruit_ili9341_drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
        }
        if (cornername & 0x2) {
            adafruit_ili9341_drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
            adafruit_ili9341_drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
        }
    }
}

// Bresenham's algorithm - thx wikpedia
void ICACHE_FLASH_ATTR adafruit_ili9341_drawLine(ili9341_lcd *lcd, int16_t x0, int16_t y0,
        int16_t x1, int16_t y1,
        uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            adafruit_ili9341_drawPixel(lcd, y0, x0, color);
        } else {
            adafruit_ili9341_drawPixel(lcd, x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

// Draw a rectangle
void ICACHE_FLASH_ATTR drawRect(ili9341_lcd *lcd, int16_t x, int16_t y,
        int16_t w, int16_t h,
        uint16_t color) {
    adafruit_ili9341_drawFastHLine(lcd, x, y, w, color);
    adafruit_ili9341_drawFastHLine(lcd, x, y+h-1, w, color);
    adafruit_ili9341_drawFastVLine(lcd, x, y, h, color);
    adafruit_ili9341_drawFastVLine(lcd, x+w-1, y, h, color);
}

// void ICACHE_FLASH_ATTR fillRect(ili9341_lcd *lcd, int16_t x, int16_t y, int16_t w, int16_t h,
//         uint16_t color) {
//     // Update in subclasses if desired!
//     for (int16_t i=x; i<x+w; i++) {
//         adafruit_ili9341_drawFastVLine(lcd, i, y, h, color);
//     }
// }

// void ICACHE_FLASH_ATTR fillScreen(ili9341_lcd *lcd, uint16_t color) {
//     adafruit_ili9341_fillRect(lcd, 0, 0, lcd->_width, lcd->_height, color);
// }

// Draw a rounded rectangle
void ICACHE_FLASH_ATTR drawRoundRect(ili9341_lcd *lcd, int16_t x, int16_t y, int16_t w,
        int16_t h, int16_t r, uint16_t color) {
    // smarter version
    adafruit_ili9341_drawFastHLine(lcd, x+r  , y    , w-2*r, color); // Top
    adafruit_ili9341_drawFastHLine(lcd, x+r  , y+h-1, w-2*r, color); // Bottom
    adafruit_ili9341_drawFastVLine(lcd, x    , y+r  , h-2*r, color); // Left
    adafruit_ili9341_drawFastVLine(lcd, x+w-1, y+r  , h-2*r, color); // Right
    // draw four corners
    drawCircleHelper(lcd, x+r    , y+r    , r, 1, color);
    drawCircleHelper(lcd, x+w-r-1, y+r    , r, 2, color);
    drawCircleHelper(lcd, x+w-r-1, y+h-r-1, r, 4, color);
    drawCircleHelper(lcd, x+r    , y+h-r-1, r, 8, color);
}

// Fill a rounded rectangle
void ICACHE_FLASH_ATTR fillRoundRect(ili9341_lcd *lcd, int16_t x, int16_t y, int16_t w,
        int16_t h, int16_t r, uint16_t color) {
    // smarter version
    adafruit_ili9341_fillRect(lcd, x+r, y, w-2*r, h, color);

    // draw four corners
    fillCircleHelper(lcd, x+w-r-1, y+r, r, 1, h-2*r-1, color);
    fillCircleHelper(lcd, x+r    , y+r, r, 2, h-2*r-1, color);
}

// Draw a triangle
void ICACHE_FLASH_ATTR drawTriangle(ili9341_lcd *lcd, int16_t x0, int16_t y0,
        int16_t x1, int16_t y1,
        int16_t x2, int16_t y2, uint16_t color) {
    adafruit_ili9341_drawLine(lcd, x0, y0, x1, y1, color);
    adafruit_ili9341_drawLine(lcd, x1, y1, x2, y2, color);
    adafruit_ili9341_drawLine(lcd, x2, y2, x0, y0, color);
}

// Fill a triangle
void ICACHE_FLASH_ATTR fillTriangle (ili9341_lcd *lcd, int16_t x0, int16_t y0,
        int16_t x1, int16_t y1,
        int16_t x2, int16_t y2, uint16_t color) {

    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        swap(y0, y1); swap(x0, x1);
    }
    if (y1 > y2) {
        swap(y2, y1); swap(x2, x1);
    }
    if (y0 > y1) {
        swap(y0, y1); swap(x0, x1);
    }

    if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if(x1 < a)      a = x1;
        else if(x1 > b) b = x1;
        if(x2 < a)      a = x2;
        else if(x2 > b) b = x2;
        adafruit_ili9341_drawFastHLine(lcd, a, y0, b-a+1, color);
        return;
    }

    int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1,
    sa   = 0,
    sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2) last = y1;   // Include y1 scanline
    else         last = y1-1; // Skip it

    for(y=y0; y<=last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
         */
        if(a > b) swap(a,b);
        adafruit_ili9341_drawFastHLine(lcd, a, y, b-a+1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for(; y<=y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
         */
        if(a > b) swap(a,b);
        adafruit_ili9341_drawFastHLine(lcd, a, y, b-a+1, color);
    }
}

void ICACHE_FLASH_ATTR drawBitmap(ili9341_lcd *lcd, int16_t x, int16_t y,
        const uint16_t *bitmap, int16_t w, int16_t h) {


    for(uint16_t j=0; j<h; j++) {
        for(uint16_t i=0; i<w; i++ ) {
            adafruit_ili9341_drawPixel(lcd, x+i, y+j, bitmap[j * w + i]);
        }
    }
}

// void ICACHE_FLASH_ATTR setTextColor(ili9341_lcd *lcd, uint16_t c) {
//     // For 'transparent' background, we'll set the bg
//     // to the same as fg instead of using a flag
//     textcolor = textbgcolor = c;
// }

void ICACHE_FLASH_ATTR setTextColor(ili9341_lcd *lcd, uint16_t c, uint16_t b) {
    lcd->textcolor   = c;
    lcd->textbgcolor = b;
}

void ICACHE_FLASH_ATTR setRotation(ili9341_lcd *lcd, uint8_t x) {
    lcd->rotation = (x & 3);
    switch(lcd->rotation) {
    case 0:
    case 2:
        lcd->_width  = lcd->WIDTH;
        lcd->_height = lcd->HEIGHT;
        break;
    case 1:
    case 3:
        lcd->_width  = lcd->HEIGHT;
        lcd->_height = lcd->WIDTH;
        break;
    }
}
/***************************************************************************************
 ** Function name:           drawUnicode
 ** Descriptions:            draw a unicode
 ***************************************************************************************/
int ICACHE_FLASH_ATTR drawUnicode(ili9341_lcd *lcd, uint16_t uniCode, uint16_t x, uint16_t y, uint8_t size)
{

    if (size) uniCode -= 32;

    uint16_t width = 0;
    uint16_t height = 0;
    const uint8_t *flash_address = 0;
    int8_t gap = 0;

#ifdef LOAD_FONT2
    if (size == 2) {
        flash_address = chrtbl_f16[uniCode];
        width = *(widtbl_f16+uniCode);
        height = chr_hgt_f16;
        gap = 1;
    }
#endif
//   if (size == 3) {
    //     flash_address = pgm_read_word(&chrtbl_f24[uniCode]);
    //     width = pgm_read_byte(widtbl_f24+uniCode);
    //     height = chr_hgt_f24;
    //     gap = 0;
    //   }
#ifdef LOAD_FONT4
    if (size == 4) {
        flash_address = chrtbl_f32[uniCode];
        width = *(widtbl_f32+uniCode);
        height = chr_hgt_f32;
        gap = -3;
    }
#endif
//   if (size == 5) {
    //     flash_address = pgm_read_word(&chrtbl_f48[uniCode]);
    //     width = pgm_read_byte(widtbl_f48+uniCode);
    //     height = chr_hgt_f48;
    //     gap = -3;
    //   }
#ifdef LOAD_FONT6
    if (size == 6) {
        flash_address = chrtbl_f64[uniCode];
        width = *(widtbl_f64+uniCode);
        height = chr_hgt_f64;
        gap = -3;
    }
#endif
#ifdef LOAD_FONT7
    if (size == 7) {
        flash_address = chrtbl_f7s[uniCode];
        width = *(widtbl_f7s+uniCode);
        height = chr_hgt_f7s;
        gap = 2;
    }
#endif

    uint16_t w = (width+ 7) / 8;
    uint16_t pX      = 0;
    uint16_t pY      = y;
    uint16_t color   = 0;
    uint8_t line = 0;

        for(int i=0; i<height; i++)
        {
            if (lcd->textcolor != lcd->textbgcolor) adafruit_ili9341_drawFastHLine(lcd, x, pY, width+gap, lcd->textbgcolor);
            for (int k = 0;k < w; k++)
            {
                line = *(flash_address + w*i + k);
                if(line) {
                    pX = x + k*8;
                    if(line & 0x80) adafruit_ili9341_drawPixel(lcd, pX, pY, lcd->textcolor);
                    if(line & 0x40) adafruit_ili9341_drawPixel(lcd, pX+1, pY, lcd->textcolor);
                    if(line & 0x20) adafruit_ili9341_drawPixel(lcd, pX+2, pY, lcd->textcolor);
                    if(line & 0x10) adafruit_ili9341_drawPixel(lcd, pX+3, pY, lcd->textcolor);
                    if(line & 0x8) adafruit_ili9341_drawPixel(lcd, pX+4, pY, lcd->textcolor);
                    if(line & 0x4) adafruit_ili9341_drawPixel(lcd, pX+5, pY, lcd->textcolor);
                    if(line & 0x2) adafruit_ili9341_drawPixel(lcd, pX+6, pY, lcd->textcolor);
                    if(line & 0x1) adafruit_ili9341_drawPixel(lcd, pX+7, pY, lcd->textcolor);
                }
            }
            pY++;
        }
    return width+gap;        // x +
}

/***************************************************************************************
 ** Function name:           drawNumber unsigned with size
 ** Descriptions:            drawNumber
 ***************************************************************************************/
int ICACHE_FLASH_ATTR drawNumber(ili9341_lcd *lcd, long long_num,uint16_t poX, uint16_t poY, uint8_t size)
{
    char tmp[10];
    if (long_num < 0) snprintf(tmp, sizeof(tmp), "%d", long_num);
    else snprintf(tmp, sizeof(tmp), "%u", long_num);
    return drawString(lcd, tmp, poX, poY, size);
}

/***************************************************************************************
 ** Function name:           drawChar
 ** Descriptions:            draw char
 ***************************************************************************************/
int ICACHE_FLASH_ATTR drawChar(ili9341_lcd *lcd, char c, uint16_t x, uint16_t y, uint8_t size)
{
    return drawUnicode(lcd, c, x, y, size);
}

/***************************************************************************************
 ** Function name:           drawString
 ** Descriptions:            draw string
 ***************************************************************************************/
int ICACHE_FLASH_ATTR drawString(ili9341_lcd *lcd, const char *string, uint16_t poX, uint16_t poY, uint8_t size)
{
    uint16_t sumX = 0;

    while(*string)
    {

        uint16_t xPlus = drawChar(lcd, *string, poX, poY, size);
        sumX += xPlus;
        *string++;

        //        if(poX < 264)
            //        {
        poX += xPlus;                                     /* Move cursor right            */
        //        }
    }

    return sumX;
}

/***************************************************************************************
 ** Function name:           drawCentreString
 ** Descriptions:            draw string across centre
 ***************************************************************************************/
int ICACHE_FLASH_ATTR drawCentreString(ili9341_lcd *lcd, const char *string, uint16_t dX, uint16_t poY, uint8_t size)
{
    uint16_t sumX = 0;
    uint16_t len = 0;
    const char *pointer = string;
    char ascii;

    while(*pointer)
    {
        ascii = *pointer;
        //if (size==0)len += 1+pgm_read_byte(widtbl_log+ascii);
        //if (size==1)len += 1+pgm_read_byte(widtbl_f8+ascii-32);
#ifdef LOAD_FONT2
        if (size==2)len += 1+*(widtbl_f16+ascii-32);
#endif
        //if (size==3)len += 1+pgm_read_byte(widtbl_f48+ascii-32)/2;
#ifdef LOAD_FONT4
        if (size==4)len += *(widtbl_f32+ascii-32)-3;
#endif
        //if (size==5) len += pgm_read_byte(widtbl_f48+ascii-32)-3;
#ifdef LOAD_FONT6
        if (size==6) len += *(widtbl_f64+ascii-32)-3;
#endif
#ifdef LOAD_FONT7
        if (size==7) len += *(widtbl_f7s+ascii-32)+2;
        *pointer++;
#endif
    }

    int poX = dX - len/2;

    if (poX < 0) poX = 0;

    while(*string)
    {

        uint16_t xPlus = drawChar(lcd, *string, poX, poY, size);
        sumX += xPlus;
        *string++;

        if(poX < 264)
        {
            poX += xPlus;                                     /* Move cursor right            */
        }
    }

    return sumX;
}

/***************************************************************************************
 ** Function name:           drawRightString
 ** Descriptions:            draw string right justified
 ***************************************************************************************/
int ICACHE_FLASH_ATTR drawRightString(ili9341_lcd *lcd, const char *string, uint16_t dX, uint16_t poY, uint8_t size)
{
    int sumX = 0;
    int len = 0;
    const char *pointer = string;
    char ascii;

    while(*pointer)
    {
        ascii = *pointer;
        //if (size==0)len += 1+pgm_read_byte(widtbl_log+ascii);
        //if (size==1)len += 1+pgm_read_byte(widtbl_f8+ascii-32);
#ifdef LOAD_FONT2
        if (size==2)len += 1+*(widtbl_f16+ascii-32);
#endif
        //if (size==3)len += 1+pgm_read_byte(widtbl_f48+ascii-32)/2;
#ifdef LOAD_FONT4
        if (size==4)len += *(widtbl_f32+ascii-32)-3;
#endif
        //if (size==5) len += pgm_read_byte(widtbl_f48+ascii-32)-3;
#ifdef LOAD_FONT6
        if (size==6) len += *(widtbl_f64+ascii-32)-3;
#endif
#ifdef LOAD_FONT7
        if (size==7) len += *(widtbl_f7s+ascii-32)+2;
#endif
        *pointer++;
    }

    int poX = dX - len;

    if (poX < 0) poX = 0;

    while(*string)
    {

        uint16_t xPlus = drawChar(lcd, *string, poX, poY, size);
        sumX += xPlus;
        *string++;

        if(poX < 264)
        {
            poX += xPlus;                                     /* Move cursor right            */
        }
    }

    return sumX;
}

/***************************************************************************************
 ** Function name:           drawFloat
 ** Descriptions:            drawFloat
 ***************************************************************************************/
int ICACHE_FLASH_ATTR drawFloat(ili9341_lcd *lcd, float floatNumber, uint8_t decimal, uint16_t poX, uint16_t poY, uint8_t size)
{
    unsigned long temp=0;
    float decy=0.0;
    float rounding = 0.5;

    float eep = 0.000001;

    int sumX    = 0;
    uint16_t xPlus   = 0;

    if(floatNumber-0.0 < eep)       // floatNumber < 0
            {
        xPlus = drawChar(lcd, '-',poX, poY, size);
        floatNumber = -floatNumber;

        poX  += xPlus;
        sumX += xPlus;
            }

    for (unsigned char i=0; i<decimal; ++i)
    {
        rounding /= 10.0;
    }

    floatNumber += rounding;

    temp = (long)floatNumber;


    xPlus = drawNumber(lcd, temp,poX, poY, size);

    poX  += xPlus;
    sumX += xPlus;

    if(decimal>0)
    {
        xPlus = drawChar(lcd, '.',poX, poY, size);
        poX += xPlus;                                       /* Move cursor right            */
        sumX += xPlus;
    }
    else
    {
        return sumX;
    }

    decy = floatNumber - temp;
    for(unsigned char i=0; i<decimal; i++)
    {
        decy *= 10;                                                      /* for the next decimal         */
        temp = decy;                                                    /* get the decimal              */
        xPlus = drawNumber(lcd, temp,poX, poY, size);

        poX += xPlus;                                       /* Move cursor right            */
        sumX += xPlus;
        decy -= temp;
    }
    return sumX;
}
