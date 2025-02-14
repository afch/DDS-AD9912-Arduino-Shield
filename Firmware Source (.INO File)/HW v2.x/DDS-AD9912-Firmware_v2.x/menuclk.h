
#ifndef __MENUCLK_H
#define __MENUCLK_H

#include "main.h"

  #define CLOCK_MENU_SOURCE_INDEX         0
  #define CLOCK_MENU_FREQ_INDEX           1
  #define CLOCK_MENU_DOUBLER_INDEX        2
  #define CLOCK_MENU_DDS_CORE_CLOCK_INDEX 3
  #define CLOCK_MENU_SAVE_INDEX           4
  #define CLOCK_MENU_EXIT_INDEX           5
  #define CLOCK_MENU_LAST_INDEX           CLOCK_MENU_EXIT_INDEX

#define CLOCK_SOURCE_INDEX_ADR 0
#define CLOCK_XO_FREQ_INDEX_ADR 2
#define CLOCK_TCXO_FREQ_INDEX_ADR 4
#define SRC_DOUBLER_ADR 6
#define EXT_OSC_FREQ_ADR 8 //and 9, 10, 11
#define DDS_CORE_CLOCK_ADR 12 //and 13, 14, 15
#define N_ADR 16
// Addresses from 24 to 60 used for main settings !!!!!!!!!!!!!!

#define CLOCK_SETTINGS_FLAG_ADR 101 // defualt settings flag address

//******* EEPROM INIT VALUES ******
#define INIT_CLOCK_SOURCE_INDEX CLOCK_SOURCE_TCXO_INDEX
#define INIT_CLOCK_XO_INDEX 0
#define INIT_CLOCK_TCXO_INDEX 4
#define INIT_SRC_DOUBLER_INDEX 0
#define INIT_EXT_OSC_FREQ 1000000000
#define INIT_DDS_CORE_FREQ 960000000
#define INIT_N 12 

void DDS_Clock_Config_Menu();
void DisplayClockSetupMenu();
void SaveClockSettings();
void LoadClockSettings();
void GetColor();
uint32_t GetRefClk();
uint32_t GetDDSCoreClock();
uint8_t FindBestNMultiplier();

  extern int SetupMenuPos;//=0;
  extern int ClockSourceIndex;//=0;
  extern uint32_t EXT_OSC_Freq;//=1000;
  extern int SRCDoublerIndex;
  extern uint8_t N_Multiplier;
  #define CLOCK_SOURCE_XO_INDEX         0
  #define CLOCK_SOURCE_TCXO_INDEX       1
  #define CLOCK_SOURCE_EXT_TCXO_INDEX   2
  #define CLOCK_SOURCE_EXT_OSC_INDEX    3
  
  extern uint32_t DDS_Core_Clock;

#endif
