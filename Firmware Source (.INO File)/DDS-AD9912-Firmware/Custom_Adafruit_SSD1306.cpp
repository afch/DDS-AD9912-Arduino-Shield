#include "Custom_Adafruit_SSD1306.h"
#include <Adafruit_GFX.h>
#include "glcdfont.c"
#include <avr/pgmspace.h>
#include <Wire.h>

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
 #define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

inline GFXglyph * pgm_read_glyph_ptr(const GFXfont *gfxFont, uint8_t c)
{
   return &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
}

inline uint8_t * pgm_read_bitmap_ptr(const GFXfont *gfxFont)
{
  return (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
}

Custom_Adafruit_SSD1306::Custom_Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire *twi=&Wire, int8_t rst_pin=-1, uint32_t clkDuring=400000UL, uint32_t clkAfter=100000UL): 
    Adafruit_SSD1306(w, h, twi, rst_pin, clkDuring, clkAfter)
    {
      
    };

size_t Custom_Adafruit_SSD1306::write(uint8_t c)
{
    if(!gfxFont) { // 'Classic' built-in font

        if(c == '\n') {                        // Newline?
            cursor_x  = 0;                     // Reset x to zero,
            cursor_y += textsize_y * 8;        // advance y one line
        } else if(c != '\r') {                 // Ignore carriage returns
            if(wrap && ((cursor_x + textsize_x * 6) > _width)) { // Off right?
                cursor_x  = 0;                 // Reset x to zero,
                cursor_y += textsize_y * 8;    // advance y one line
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
            cursor_x += textsize_x * 6;          // Advance x one char
        }

    } else { // Custom font

        if(c == '\n') {
            cursor_x  = 0;
            cursor_y += (int16_t)textsize_y *
                        (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        } else if(c != '\r') {
            uint8_t first = pgm_read_byte(&gfxFont->first);
            if((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
                GFXglyph *glyph  = pgm_read_glyph_ptr(gfxFont, c - first);
                uint8_t   w     = pgm_read_byte(&glyph->width),
                          h     = pgm_read_byte(&glyph->height);
                if((w > 0) && (h > 0)) { // Is there an associated bitmap?
                    int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
                    if(wrap && ((cursor_x + textsize_x * (xo + w)) > _width)) {
                        cursor_x  = 0;
                        cursor_y += (int16_t)textsize_y *
                          (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                    }
                    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
                }
                cursor_x += (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
            }
        }

    }
    return 1;
}

//void Custom_Adafruit_SSD1306::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {};

void Custom_Adafruit_SSD1306::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y)
  {
    if(!gfxFont)  // 'Classic' built-in font
    {

        if((x >= _width)            || // Clip right
           (y >= _height)           || // Clip bottom
           ((x + 6 * size_x - 1) < 0) || // Clip left
           ((y + 8 * size_y - 1) < 0))   // Clip top
            return;

        if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

        startWrite();
        for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
            uint8_t line = pgm_read_byte(&font[c * 5 + i]);
            for(int8_t j=0; j<8; j++, line >>= 1) {
                if(line & 1) {
                    if(size_x == 1 && size_y == 1)
                        writePixel(x+i, y+j, color);
                    else
                        writeFillRect(x+i*size_x, y+j*size_y, size_x, size_y, color);
                } else if(bg != color) {
                    if(size_x == 1 && size_y == 1)
                        writePixel(x+i, y+j, bg);
                    else
                        writeFillRect(x+i*size_x, y+j*size_y, size_x, size_y, bg);
                }
            }
        }
        if(bg != color) { // If opaque, draw vertical line for last column
            if(size_x == 1 && size_y == 1) writeFastVLine(x+5, y, 8, bg);
            else          writeFillRect(x+5*size_x, y, size_x, 8*size_y, bg);
        }
        endWrite();

    } else // Custom font
    { 
        // Character is assumed previously filtered by write() to eliminate
        // newlines, returns, non-printable characters, etc.  Calling
        // drawChar() directly with 'bad' characters of font may cause mayhem!

        c -= (uint8_t)pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph  = pgm_read_glyph_ptr(gfxFont, c);
        uint8_t  *bitmap = pgm_read_bitmap_ptr(gfxFont);

        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t  w  = pgm_read_byte(&glyph->width),
                 h  = pgm_read_byte(&glyph->height);
        int8_t   xo = pgm_read_byte(&glyph->xOffset),
                 yo = pgm_read_byte(&glyph->yOffset);
        uint8_t  xx, yy, bits = 0, bit = 0;
        int16_t  xo16 = 0, yo16 = 0;
        
        if(size_x > 1 || size_y > 1) {
            xo16 = xo;
            yo16 = yo;
        }

        startWrite();

        if(bg != color) {
            writeFillRect(x, y-CUSTOM_FONT_H+1, CUSTOM_FONT_W, CUSTOM_FONT_H-3, bg);
        }

        for(yy=0; yy<h; yy++) {
            for(xx=0; xx<w; xx++) {
                if(!(bit++ & 7)) {
                    bits = pgm_read_byte(&bitmap[bo++]);
                }
                if(bits & 0x80) {
                    if(size_x == 1 && size_y == 1) {
                        writePixel(x+xo+xx, y+yo+yy, color);
                    } else {
                        writeFillRect(x+(xo16+xx)*size_x, y+(yo16+yy)*size_y,
                          size_x, size_y, color);
                    }
                } else if(bg != color) {
                          //  writePixel(x+xo+xx, y+yo+yy, bg);
                        }
                bits <<= 1;
            }
        }

        endWrite();

    } // End classic vs custom font
}