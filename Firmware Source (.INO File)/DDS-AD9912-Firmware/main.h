#ifndef __MAIN_H
#define __MAIN_H

//#include <Wire.h>
//#include <Adafruit_SSD1306.h>
#include "Custom_Adafruit_SSD1306.h"
#include <ClickButton.h>
#include <EEPROM.h>
#include "menuclk.h"

#include <Encoder.h>
#ifndef GRA_AND_AFCH_ENCODER_MOD2
  #error The "Encoder" library modified by GRA and AFCH must be used!
#endif

//extern Adafruit_SSD1306 display;
extern Custom_Adafruit_SSD1306 display;

#define MODE_PIN     2
#define BACK_PIN     3
#define A_PIN       18
#define B_PIN       19

extern ClickButton modeButton;
//extern ClickButton backButton;
extern Encoder myEnc;


extern bool EditMode;
//extern uint8_t incrValue;
//extern uint8_t decrValue;
extern int curPos;

extern void DisplaySaved();
extern void DrawMainMenu();
extern void MakeOut();
extern void DDS_Init ();
extern void DDS_Freq_Set(uint64_t Freq, uint64_t Fs); //думаю это лучше убрать отсюда
extern bool Check(uint16_t _M, uint16_t _K, uint16_t _H);
extern int32_t Inc(int32_t val, int incr);
extern int32_t Dec(int32_t val, int decr);

extern String PreZero(int Digit);

#endif
