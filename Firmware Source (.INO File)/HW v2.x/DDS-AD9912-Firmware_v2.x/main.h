#ifndef __MAIN_H
#define __MAIN_H

//#include <Wire.h>
//#include <Adafruit_SSD1306.h>
#include "Custom_Adafruit_SSD1306.h"
#include <ClickButton.h>
#include <EEPROM.h>
#include "menuclk.h"

#include <Encoder.h>
#ifndef GRA_AND_AFCH_ENCODER_MOD3
  #error The "Encoder" library modified by GRA and AFCH must be used!
#endif

//extern Adafruit_SSD1306 display;
extern Custom_Adafruit_SSD1306 display;

#define MODE_PIN     2
//#define BACK_PIN     3
#define A_PIN       18
#define B_PIN       19

#define RFOUT_PIN   3

#define HSTL_LED_PIN    12
#define CMOS_LED_PIN    11

// #define CLOCK_SOURCE_XO_INDEX         0
// #define CLOCK_SOURCE_TCXO_INDEX       1
// #define CLOCK_SOURCE_EXT_TCXO_INDEX   2
// #define CLOCK_SOURCE_EXT_OSC_INDEX    3

#define EXTERANL_SRC_PATH_PIN   9 //switcher V1, выход на коммутатор источников тактирования
#define TCXO_PATH_PIN           10 //switcher V2, выход на коммутатор источников тактирования
#define TCXO_POWER_PIN          13   //REF_LED_PIN

extern ClickButton modeButton;
extern ClickButton RFOutButton;
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

extern void selectClockSrcPath(uint8_t path);

#endif
