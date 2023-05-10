#ifndef __CUSTOM_SSD1306
#define __CUSTOM_SSD1306

//#include <Adafruit_SSD1306.h>
#include "Custom_Adafruit_SSD1306.h"
#include <Wire.h>

#define CUSTOM_FONT_W 9
#define CUSTOM_FONT_H 14

class Custom_Adafruit_SSD1306 :public Adafruit_SSD1306
{
    public: 
    Custom_Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire *twi=&Wire, int8_t rst_pin=-1, uint32_t clkDuring=400000UL, uint32_t clkAfter=100000UL);//:
      // Adafruit_SSD1306(w, h, twi, rst_pin, clkDuring, clkAfter);

    //void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) override;
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);// override;
    //void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);

    virtual size_t write(uint8_t) override;
};
/*
Custom_Adafruit_SSD1306::Custom_Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire *twi=&Wire, int8_t rst_pin=-1, uint32_t clkDuring=400000UL, uint32_t clkAfter=100000UL):

{

}*/



#endif