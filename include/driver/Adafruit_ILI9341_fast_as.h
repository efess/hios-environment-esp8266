#ifndef _ADAFRUIT_ILI9341H_
#define _ADAFRUIT_ILI9341H_

#include "driver/Adafruit_GFX_AS.h"

#include "osapi.h"
#include "gpio.h"
#include "hspi.h"


#define ILI9341_TFTWIDTH  240
#define ILI9341_TFTHEIGHT 320

#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0A
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF  0x20
#define ILI9341_INVON   0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON  0x29

#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_RAMRD   0x2E

#define ILI9341_PTLAR   0x30
#define ILI9341_MADCTL  0x36
#define ILI9341_PIXFMT  0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1
/*
#define ILI9341_PWCTR6  0xFC

*/

// Color definitions
#define	ILI9341_BLACK   0x0000
#define	ILI9341_BLUE    0x001F
#define	ILI9341_RED     0xF800
#define	ILI9341_GREEN   0x07E0
#define ILI9341_CYAN    0x07FF
#define ILI9341_MAGENTA 0xF81F
#define ILI9341_YELLOW  0xFFE0  
#define ILI9341_WHITE   0xFFFF

#define TFT_DC_DATA		GPIO_OUTPUT_SET(2, 1)
#define TFT_DC_COMMAND	GPIO_OUTPUT_SET(2, 0)
#define TFT_DC_INIT 	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); TFT_DC_DATA

#define TFT_RST_ACTIVE		GPIO_OUTPUT_SET(4, 0)
#define TFT_RST_DEACTIVE 	GPIO_OUTPUT_SET(4, 1)
#define TFT_RST_INIT		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); TFT_RST_DEACTIVE

#define MAKEWORD(b1, b2, b3, b4) ((b1) | ((b2) << 8) | ((b3) << 16) | ((b4) << 24))

//class Adafruit_ILI9341 : public Adafruit_GFX_AS {

//private:
uint8_t  tabcolor;
void adafruit_ili9341_transmitCmdData(uint8_t cmd, const uint8_t *data, uint8_t numDataByte);

//public:
//Adafruit_ILI9341();

void adafruit_ili9341_init(ili9341_lcd *lcd);
void adafruit_ili9341_fillScreen(ili9341_lcd *lcd, uint16_t color);
void adafruit_ili9341_drawPixel(ili9341_lcd *lcd, int16_t x, int16_t y, uint16_t color);
void adafruit_ili9341_drawFastVLine(ili9341_lcd *lcd, int16_t x, int16_t y, int16_t h, uint16_t color);
void adafruit_ili9341_drawFastHLine(ili9341_lcd *lcd, int16_t x, int16_t y, int16_t w, uint16_t color);
void adafruit_ili9341_fillRect(ili9341_lcd *lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void adafruit_ili9341_setRotation(ili9341_lcd *lcd, uint8_t r);
void adafruit_ili9341_invertDisplay(ili9341_lcd *lcd, bool i);

uint16_t adafruit_ili9341_color565(uint8_t r, uint8_t g, uint8_t b);

void ili9341_drawSingleBitBitmap(ili9341_lcd *lcd,  int16_t x, int16_t y,
    uint8_t *bitmap, int16_t w, int16_t h);

#endif
