#include "lcd.h"
#include "file.h"
#include "mem.h"
#include "info.h"
#include "run_state.h"
#include "driver/Adafruit_ILI9341_fast_as.h"
#include "weather.h"

ili9341_lcd tft;
static ETSTimer _lcdTimer;

void ICACHE_FLASH_ATTR lcd_get_weather_icon_name(uint8_t weather_icon, uint8_t* icon_file) 
{
    switch(weather_icon)
    {
        case WEATHER_CLEAR:
            os_strcpy(icon_file, "sunny.bmp");
            return;
        case WEATHER_PARTLY_CLOUDY:
            os_strcpy(icon_file, "partlycloudy.bmp");
            return;
        case WEATHER_CLOUDY:
            os_strcpy(icon_file, "cloudy.bmp");
            return;
        case WEATHER_RAIN:
            os_strcpy(icon_file, "rain.bmp");
            return;
        case WEATHER_SLEET:
            os_strcpy(icon_file, "sleet.bmp");
            return;
        case WEATHER_SNOW:
            os_strcpy(icon_file, "snow.bmp");
            return;
        case WEATHER_THUNDER:
            os_strcpy(icon_file, "thunder.bmp");
            return;
        case WEATHER_PARTLY_CLOUDY_NIGHT:
            os_strcpy(icon_file, "cloudynight.bmp");
            return;
        case WEATHER_CLEAR_NIGHT:
            os_strcpy(icon_file, "night.bmp");
            return;
    }
}

int16_t ICACHE_FLASH_ATTR cToF(long c) 
{
    int16_t f = (c * 1.8) + 32;

    return f;
}

void ICACHE_FLASH_ATTR drawOneBitBitmap(ili9341_lcd *lcd, int16_t x, int16_t y,
    uint8_t *bitmap, int16_t w, int16_t h) {

    uint8_t *bmpBuf = bitmap + 10;
    uint32_t offset = (bmpBuf[3] << 24) | (bmpBuf[2] << 16) | (bmpBuf[1] << 8) | bmpBuf[0];
    
    bmpBuf = bitmap + offset;
    
    uint8_t row_byte_length = w / 8;
    row_byte_length = row_byte_length + (4 - (row_byte_length % 4));

    uint8_t bmpIdx = 0;
    uint8_t bmpBit = 0;
    uint16_t rowStart = 0;
    uint16_t byteIdx = 0;
    uint16_t color = 0;
    
    // this is not working.
    for(uint16_t j=0; j <h ; j++) {
        rowStart = (h - j) * row_byte_length;
        for(uint16_t i=0; i<w; i++ ) {
            byteIdx = rowStart + (i / 8);
            bmpBit = 7 - (i % 8);

            color = (bmpBuf[byteIdx] >> bmpBit) & 1 == 1 ? 0xFFFF : 0x0000;
            adafruit_ili9341_drawPixel(lcd, x+i, y+j, color);
        }
    }
}


void ICACHE_FLASH_ATTR lcd_update(void *args)
{
    os_timer_disarm(&_lcdTimer);
    
    adafruit_ili9341_fillScreen(&tft, 0);
    drawString(&tft, "Temp: ", 20, 60, 4);
    drawNumber(&tft, cToF(run_state.temp), 160, 60, 4);

    drawString(&tft, "Humid: ", 20, 100, 4);
    drawNumber(&tft, run_state.humidity, 160, 100, 4);

    EspFileDescriptor desc;
    uint8_t fileName[22] = {0};
    uint8_t *buf = (uint8_t*)os_zalloc(2048);
    
    os_strcpy(fileName, "img/");
    lcd_get_weather_icon_name(weather_state->current_icon, fileName + 4);
    
    if(!file_find(&desc, fileName))
    {
        INFO("FILE NOT FOUND %s", fileName);
    } 
    else 
    {
        file_read(&desc, buf, 0, desc.length);
        
        //drawOneBitBitmap(&tft, 20, 140, buf, 114, 104);// 107, 92
        ili9341_drawSingleBitBitmap(&tft, 20, 140, buf, 107, 92);
    }

    INFO("before %d\r\n", weather_state->temp);
    int16_t asdf= cToF(weather_state->temp);
    INFO("asdf %d\r\n", asdf);
    drawString(&tft, "Outside: ", 20, 240, 4);
    drawNumber(&tft, asdf, 160, 240, 4);
    
    
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
