#include "driver/Adafruit_ILI9341_fast_as.h"
#include "driver/Adafruit_GFX_AS.h"

#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "driver/hspi.h"
//#include "espmissingincludes.h"

void ICACHE_FLASH_ATTR adafruit_ili9341_transmitData_nr(uint16_t data) 
{
    hspi_wait_ready(); 
    hspi_send_uint16(data);
}

void ICACHE_FLASH_ATTR adafruit_ili9341_transmitCmdData_nr(uint8_t cmd, uint32_t data) 
{
    hspi_wait_ready(); 
    TFT_DC_COMMAND; 
    hspi_send_uint8(cmd); 
    hspi_wait_ready(); 
    TFT_DC_DATA; 
    hspi_send_uint32(data);
}


void ICACHE_FLASH_ATTR adafruit_ili9341_transmitData(uint16_t data, int32_t repeats)
{
    hspi_wait_ready();
    hspi_send_uint16_r(data, repeats);
}

void ICACHE_FLASH_ATTR adafruit_ili9341_transmitCmd(uint8_t cmd)
{
    hspi_wait_ready();
    TFT_DC_COMMAND; 
    hspi_send_uint8(cmd);
    hspi_wait_ready(); 
    TFT_DC_DATA;
}

#define SWAPBYTES(i) ((i>>8) | (i<<8))

void ICACHE_FLASH_ATTR adafruit_ili9341_transmitCmdData(uint8_t cmd, const uint8_t *data, uint8_t numDataByte)
{
    hspi_wait_ready();
    TFT_DC_COMMAND;
    hspi_send_uint8(cmd);
    hspi_wait_ready();
    TFT_DC_DATA;
    hspi_send_data(data, numDataByte);
}

void ICACHE_FLASH_ATTR setAddrWindow(ili9341_lcd *lcd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{  
    adafruit_ili9341_transmitCmdData_nr(ILI9341_CASET, MAKEWORD(x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF));
    adafruit_ili9341_transmitCmdData_nr(ILI9341_PASET, MAKEWORD(y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF));
    adafruit_ili9341_transmitCmd(ILI9341_RAMWR); // write to RAM
}
void ICACHE_FLASH_ATTR adafruit_ili9341_init(ili9341_lcd *lcd) {
    //Set communication using HW SPI Port
    hspi_init();
    TFT_DC_INIT;
    TFT_RST_INIT;

    TFT_RST_ACTIVE;
    os_delay_us(10000);
    TFT_RST_DEACTIVE;
    os_delay_us(1000);

    uint8_t data[15] = {0};

    data[0] = 0x39;
    data[1] = 0x2C;
    data[2] = 0x00;
    data[3] = 0x34;
    data[4] = 0x02;
    adafruit_ili9341_transmitCmdData(0xCB, data, 5);

    data[0] = 0x00;
    data[1] = 0XC1;
    data[2] = 0X30;
    adafruit_ili9341_transmitCmdData(0xCF, data, 3);

    data[0] = 0x85;
    data[1] = 0x00;
    data[2] = 0x78;
    adafruit_ili9341_transmitCmdData(0xE8, data, 3);

    data[0] = 0x00;
    data[1] = 0x00;
    adafruit_ili9341_transmitCmdData(0xEA, data, 2);

    data[0] = 0x64;
    data[1] = 0x03;
    data[2] = 0X12;
    data[3] = 0X81;
    adafruit_ili9341_transmitCmdData(0xED, data, 4);

    data[0] = 0x20;
    adafruit_ili9341_transmitCmdData(0xF7, data, 1);

    data[0] = 0x23;       //VRH[5:0]
    adafruit_ili9341_transmitCmdData(0xC0, data, 1);        //Power control

    data[0] = 0x10;       //SAP[2:0];BT[3:0]
    adafruit_ili9341_transmitCmdData(0xC1, data, 1);        //Power control

    data[0] = 0x3e;       //Contrast
    data[1] = 0x28;
    adafruit_ili9341_transmitCmdData(0xC5, data, 2);        //VCM control

    data[0] = 0x86;       //--
    adafruit_ili9341_transmitCmdData(0xC7, data, 1);        //VCM control2

    data[0] = 0x48;      //C8
    adafruit_ili9341_transmitCmdData(0x36, data, 1);        // Memory Access Control

    data[0] = 0x55;
    adafruit_ili9341_transmitCmdData(0x3A, data, 1);

    data[0] = 0x00;
    data[1] = 0x18;
    adafruit_ili9341_transmitCmdData(0xB1, data, 2);

    data[0] = 0x08;
    data[1] = 0x82;
    data[2] = 0x27;
    adafruit_ili9341_transmitCmdData(0xB6, data, 3);        // Display Function Control

    data[0] = 0x00;
    adafruit_ili9341_transmitCmdData(0xF2, data, 1);        // 3Gamma Function Disable

    data[0] = 0x01;
    adafruit_ili9341_transmitCmdData(0x26, data, 1);        //Gamma curve selected

    data[0] = 0x0F;
    data[1] = 0x31;
    data[2] = 0x2B;
    data[3] = 0x0C;
    data[4] = 0x0E;
    data[5] = 0x08;
    data[6] = 0x4E;
    data[7] = 0xF1;
    data[8] = 0x37;
    data[9] = 0x07;
    data[10] = 0x10;
    data[11] = 0x03;
    data[12] = 0x0E;
    data[13] = 0x09;
    data[14] = 0x00;
    adafruit_ili9341_transmitCmdData(0xE0, data, 15);        //Set Gamma

    data[0] = 0x00;
    data[1] = 0x0E;
    data[2] = 0x14;
    data[3] = 0x03;
    data[4] = 0x11;
    data[5] = 0x07;
    data[6] = 0x31;
    data[7] = 0xC1;
    data[8] = 0x48;
    data[9] = 0x08;
    data[10] = 0x0F;
    data[11] = 0x0C;
    data[12] = 0x31;
    data[13] = 0x36;
    data[14] = 0x0F;
    adafruit_ili9341_transmitCmdData(0xE1, data, 15);        //Set Gamma

    adafruit_ili9341_transmitCmd(0x11);        //Exit Sleep
    os_delay_us(120000);

    adafruit_ili9341_transmitCmd(0x29);    //Display on
    adafruit_ili9341_transmitCmd(0x2c);
}

void ICACHE_FLASH_ATTR adafruit_ili9341_drawPixel(ili9341_lcd *lcd, int16_t x, int16_t y, uint16_t color) {

    if((x < 0) ||(x >= lcd->_width) || (y < 0) || (y >= lcd->_height)) return;
    setAddrWindow(lcd, x,y,x+1,y+1);
    adafruit_ili9341_transmitData_nr(SWAPBYTES(color));
}


void ICACHE_FLASH_ATTR adafruit_ili9341_drawFastVLine(ili9341_lcd *lcd, int16_t x, int16_t y, int16_t h,
        uint16_t color) {

    // Rudimentary clipping
    if((x >= lcd->_width) || (y >= lcd->_height)) return;

    if((y+h-1) >= lcd->_height)
        h = lcd->_height-y;

    setAddrWindow(lcd, x, y, x, y+h-1);
    adafruit_ili9341_transmitData(SWAPBYTES(color), h);
}

void ICACHE_FLASH_ATTR adafruit_ili9341_drawFastHLine(ili9341_lcd *lcd, int16_t x, int16_t y, int16_t w,
        uint16_t color) {

    // Rudimentary clipping
    if((x >= lcd->_width) || (y >= lcd->_height)) return;
    if((x+w-1) >= lcd->_width)  w = lcd->_width-x;
    setAddrWindow(lcd, x, y, x+w-1, y);
    adafruit_ili9341_transmitData(SWAPBYTES(color), w);
}

void ICACHE_FLASH_ATTR adafruit_ili9341_fillScreen(ili9341_lcd *lcd, uint16_t color) {
    adafruit_ili9341_fillRect(lcd, 0, 0,  lcd->_width, lcd->_height, color);
}

// fill a rectangle
void ICACHE_FLASH_ATTR adafruit_ili9341_fillRect(ili9341_lcd *lcd, int16_t x, int16_t y, int16_t w, int16_t h,
        uint16_t color) {

    // rudimentary clipping (drawChar w/big text requires this)
    if((x >= lcd->_width) || (y >= lcd->_height)) return;
    if((x + w - 1) >= lcd->_width)  w = lcd->_width  - x;
    if((y + h - 1) >= lcd->_height) h = lcd->_height - y;

    setAddrWindow(lcd, x, y, x+w-1, y+h-1);
    adafruit_ili9341_transmitData(SWAPBYTES(color), h*w);
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t ICACHE_FLASH_ATTR adafruit_ili9341_color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void ICACHE_FLASH_ATTR adafruit_ili9341_setRotation(ili9341_lcd *lcd, uint8_t m) {

    uint8_t data;
    lcd->rotation = m % 4; // can't be higher than 3
    switch (lcd->rotation) {
    case 0:
        data = MADCTL_MX | MADCTL_BGR;
        lcd->_width  = ILI9341_TFTWIDTH;
        lcd->_height = ILI9341_TFTHEIGHT;
        break;
    case 1:
        data = MADCTL_MV | MADCTL_BGR;
        lcd->_width  = ILI9341_TFTHEIGHT;
        lcd->_height = ILI9341_TFTWIDTH;
        break;
    case 2:
        data = MADCTL_MY | MADCTL_BGR;
        lcd->_width  = ILI9341_TFTWIDTH;
        lcd->_height = ILI9341_TFTHEIGHT;
        break;
    case 3:
        data = MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR;
        lcd->_width  = ILI9341_TFTHEIGHT;
        lcd->_height = ILI9341_TFTWIDTH;
        break;
    }
    adafruit_ili9341_transmitCmdData(ILI9341_MADCTL, &data, 1);
}


void ICACHE_FLASH_ATTR adafruit_ili9341_invertDisplay(ili9341_lcd *lcd, bool i) {
    adafruit_ili9341_transmitCmd(i ? ILI9341_INVON : ILI9341_INVOFF);
}
