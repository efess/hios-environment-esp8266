#include "lcd.h"
#include "file.h"
#include "mem.h"
#include "driver/Adafruit_ILI9341_fast_as.h"

ili9341_lcd tft;
static ETSTimer _lcdTimer;


void ICACHE_FLASH_ATTR drawOneBitBitmap(ili9341_lcd *lcd, int16_t x, int16_t y,
        const uint8_t *bitmap, int16_t w, int16_t h) {
    uint8_t bmpIdx = 0;
    uint8_t bmpBit = 0;
    uint16_t idx = 0;
    uint16_t color = 0;
    // this is not working.
    for(uint16_t j=0; j<h; j++) {
        for(uint16_t i=0; i<w; i++ ) {
            idx = j * w + i;
            bmpIdx = idx / 8;
            bmpBit = idx % 8;
            color = (bitmap[bmpIdx] >> bmpBit) & 1 == 1 ? 0xFFFF : 0x0000;
            adafruit_ili9341_drawPixel(lcd, x+i, y+j, color);
        }
    }
}


void ICACHE_FLASH_ATTR lcd_update(void *args)
{
    os_timer_disarm(&_lcdTimer);
    
    EspFileDescriptor desc;
    uint8_t *buf = (uint8_t*)os_malloc(2048);

    file_find(&desc, "img/sunshine.bmp");
    file_read(&desc, buf, 0, desc.length);
    
    drawOneBitBitmap(&tft, 20, 60, buf, 114, 104);
    
    os_free(buf);

    os_timer_setfn(&_lcdTimer, (os_timer_func_t *)lcd_update, NULL);
    os_timer_arm(&_lcdTimer, 5000, 0);
}

void ICACHE_FLASH_ATTR lcd_start()
{
    tft.WIDTH     = ILI9341_TFTWIDTH;
    tft.HEIGHT    = ILI9341_TFTHEIGHT;
    tft._width    = ILI9341_TFTWIDTH;
    tft._height   = ILI9341_TFTHEIGHT;
    tft.rotation  = 0;
    tft.textcolor = tft.textbgcolor = 0xFFFF;
    tft.wrap      = true;
    
    adafruit_ili9341_init(&tft);

    adafruit_ili9341_fillScreen(&tft, 0);


    drawString(&tft, "hello world?", 20, 20, 4);

    os_timer_setfn(&_lcdTimer, (os_timer_func_t *)lcd_update, NULL);
    os_timer_arm(&_lcdTimer, 5000, 0);
}
