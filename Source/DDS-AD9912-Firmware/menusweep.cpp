#include "main.h"
#include "font.h"
#include "menusweep.h"
// http://javl.github.io/image2cpp/

// 'infinity9x11', 9x11px
const unsigned char bitmap_infinity [] PROGMEM = {
	0x00, 0x00, 0x1c, 0x00, 0x22, 0x00, 0x27, 0x00, 0x2a, 0x80, 0x22, 0x00, 0xaa, 0x00, 0x72, 0x00, 
	0x22, 0x00, 0x1c, 0x00, 0x00, 0x00
};
// 'up_down9x11', 9x11px
const unsigned char bitmap_up_down [] PROGMEM = {
	0x00, 0x00, 0x22, 0x00, 0x72, 0x00, 0xaa, 0x00, 0x22, 0x00, 0x22, 0x00, 0x22, 0x00, 0x2a, 0x80, 
	0x27, 0x00, 0x22, 0x00, 0x00, 0x00
};
// 'up9x11', 9x11px
const unsigned char bitmap_up [] PROGMEM = {
	0x00, 0x00, 0x08, 0x00, 0x1c, 0x00, 0x2a, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 
	0x08, 0x00, 0x08, 0x00, 0x00, 0x00
};

#define DEFUALT_SWEEP_START_FREQ 100000
#define DEFUALT_SWEEP_END_FREQ 200000

//#define MAX_SWEEP_FREQ 600000000UL
//#define MIN_SWEEP_FREQ 100000

#define MIN_SWEEP_TIME 1 
#define MAX_SWEEP_TIME 999

//************SWEEP INITs********************
#define INIT_SWEEP_START_FREQ_M 100
#define INIT_SWEEP_START_FREQ_K 0
#define INIT_SWEEP_START_FREQ_H 0
#define INIT_SWEEP_START_FREQ 100000000

#define INIT_SWEEP_END_FREQ_M 200
#define INIT_SWEEP_END_FREQ_K 0
#define INIT_SWEEP_END_FREQ_H 0
#define INIT_SWEEP_END_FREQ 200000000

#define INIT_SWEEP_TIME 1
#define INIT_SWEEP_TIME_FORMAT 0

#define INIT_SWEEP_MODE SWEEP_MODE_UP_DOWN
#define INIT_SWEEP_LOOP SWEEP_LOOPED

//****************************************
#define MENU_SWEEP_START_FREQ_M_INDEX 0
#define MENU_SWEEP_START_FREQ_K_INDEX 1
#define MENU_SWEEP_START_FREQ_H_INDEX 2

#define MENU_SWEEP_END_FREQ_M_INDEX 3
#define MENU_SWEEP_END_FREQ_K_INDEX 4
#define MENU_SWEEP_END_FREQ_H_INDEX 5

#define MENU_SWEEP_TIME_FORMAT_INDEX 6
#define MENU_SWEEP_TIME_INDEX 7

#define MENU_SWEEP_START_INDEX 8

#define MENU_SWEEP_MODE_INDEX 9
#define MENU_SWEEP_LOOP_INDEX 10

#define MENU_SWEEP_EXIT_INDEX 11
#define MENU_SWEEP_LAST_INDEX MENU_SWEEP_EXIT_INDEX

//*********************************

#define SWEEP_MODE_UP 0
#define SWEEP_MODE_UP_DOWN 1

#define SWEEP_ONCE 0
#define SWEEP_LOOPED 1

#define SWEEP_START_FREQ_M_ADR 66
#define SWEEP_START_FREQ_K_ADR 68
#define SWEEP_START_FREQ_H_ADR 70

#define SWEEP_END_FREQ_M_ADR 72
#define SWEEP_END_FREQ_K_ADR 74
#define SWEEP_END_FREQ_H_ADR 76

#define SWEEP_TIME_ADR 78
#define SWEEP_TIME_FORMAT_ADR 80

#define SWEEP_MODE_ADR 82
#define SWEEP_LOOP_ADR 84

int SweepStartFreqM = 100;
int SweepStartFreqK = 0;
int SweepStartFreqH = 0;
int SweepStartFreq = INIT_SWEEP_START_FREQ;

int SweepEndFreqM = 200;
int SweepEndFreqK = 0;
int SweepEndFreqH = 0;
int SweepEndFreq = INIT_SWEEP_END_FREQ;

int SweepTime = INIT_SWEEP_TIME;
int SweepTimeFormat = INIT_SWEEP_TIME_FORMAT;
String TimeFormatsNames[] = {"Sec", "mSec"};

bool SweepMode = INIT_SWEEP_MODE;
bool SweepLoop = INIT_SWEEP_LOOP;

int8_t SweepMenuPos = 0;
int SweepEncoderPos = 0;

volatile bool SweepInProgress = false;

void SweepMenu()
{
  SweepMenuPos = 0;
  EditMode = false;

  while(1)
  {
    SweepEncoderPos=0;
    SweepEncoderPos=myEnc.read();
    modeButton.Update();
    
    if (modeButton.clicks > 0) 
    {
      switch (SweepMenuPos)
      {
        case MENU_SWEEP_START_FREQ_M_INDEX:
        case MENU_SWEEP_START_FREQ_K_INDEX:
        case MENU_SWEEP_START_FREQ_H_INDEX:
        case MENU_SWEEP_END_FREQ_M_INDEX:
        case MENU_SWEEP_END_FREQ_K_INDEX:
        case MENU_SWEEP_END_FREQ_H_INDEX:
        case MENU_SWEEP_TIME_INDEX:
          EditMode = !EditMode;
        break;

        case MENU_SWEEP_TIME_FORMAT_INDEX:
          SweepTimeFormat =! SweepTimeFormat;
        break;

        case MENU_SWEEP_MODE_INDEX:
          SweepMode = !SweepMode;
        break;

        case MENU_SWEEP_LOOP_INDEX:
          SweepLoop = !SweepLoop;
        break;

        case MENU_SWEEP_START_INDEX:
          SaveSweepSettings();
          SweepInProgress = !SweepInProgress;
          if (SweepInProgress == true) MakeSweep();
            else TimerStop();
        break;

        case MENU_SWEEP_EXIT_INDEX:
          return;
        break;
      }
    } 

    if (EditMode == false)
    {
      if (SweepEncoderPos > 0) 
      {
        SweepMenuPos++;
        if (SweepMenuPos > MENU_SWEEP_LAST_INDEX) SweepMenuPos = MENU_SWEEP_START_FREQ_M_INDEX;
      }
      if (SweepEncoderPos < 0) 
      {
        SweepMenuPos--;
        if (SweepMenuPos < MENU_SWEEP_START_FREQ_M_INDEX) SweepMenuPos = MENU_SWEEP_LAST_INDEX;
      }
    } else 
    {
     if (SweepEncoderPos > 0)
     {
      switch (SweepMenuPos)
      {
        case MENU_SWEEP_START_FREQ_M_INDEX:
          if (Check(SweepStartFreqM + SweepEncoderPos, SweepStartFreqK, SweepStartFreqH)) SweepStartFreqM = Inc(SweepStartFreqM, SweepEncoderPos);
        break;
        case MENU_SWEEP_START_FREQ_K_INDEX:
          if (Check(SweepStartFreqM, SweepStartFreqK + SweepEncoderPos, SweepStartFreqH)) SweepStartFreqK = Inc(SweepStartFreqK, SweepEncoderPos);
        break;
        case MENU_SWEEP_START_FREQ_H_INDEX:
          if (Check(SweepStartFreqM, SweepStartFreqK, SweepStartFreqH + SweepEncoderPos)) SweepStartFreqH = Inc(SweepStartFreqH, SweepEncoderPos);
        break;

        case MENU_SWEEP_END_FREQ_M_INDEX:
          if (Check(SweepEndFreqM + SweepEncoderPos, SweepEndFreqK, SweepEndFreqH)) SweepEndFreqM = Inc(SweepEndFreqM, SweepEncoderPos);
        break;
        case MENU_SWEEP_END_FREQ_K_INDEX:
          if (Check(SweepEndFreqM, SweepEndFreqK + SweepEncoderPos, SweepEndFreqH)) SweepEndFreqK = Inc(SweepEndFreqK, SweepEncoderPos);
        break;
        case MENU_SWEEP_END_FREQ_H_INDEX:
          if (Check(SweepEndFreqM, SweepEndFreqK, SweepEndFreqH + SweepEncoderPos)) SweepEndFreqH = Inc(SweepEndFreqH, SweepEncoderPos);
        break;

        case MENU_SWEEP_TIME_INDEX:
          if (SweepTime + SweepEncoderPos <= MAX_SWEEP_TIME) SweepTime = SweepTime + SweepEncoderPos; 
            else SweepTime = MIN_SWEEP_TIME;
        break;
      }
     }

     if (SweepEncoderPos < 0)
     {
      switch (SweepMenuPos)
      {
        case MENU_SWEEP_START_FREQ_M_INDEX:
          if (Check(SweepStartFreqM + SweepEncoderPos, SweepStartFreqK, SweepStartFreqH)) SweepStartFreqM = Dec(SweepStartFreqM, SweepEncoderPos);
        break;
        case MENU_SWEEP_START_FREQ_K_INDEX:
          if (Check(SweepStartFreqM, SweepStartFreqK + SweepEncoderPos, SweepStartFreqH)) SweepStartFreqK = Dec(SweepStartFreqK, SweepEncoderPos);
        break;
        case MENU_SWEEP_START_FREQ_H_INDEX:
          if (Check(SweepStartFreqM, SweepStartFreqK, SweepStartFreqH + SweepEncoderPos)) SweepStartFreqH = Dec(SweepStartFreqH, SweepEncoderPos);
        break;

        case MENU_SWEEP_END_FREQ_M_INDEX:
          if (Check(SweepEndFreqM + SweepEncoderPos, SweepEndFreqK, SweepEndFreqH)) SweepEndFreqM = Dec(SweepEndFreqM, SweepEncoderPos);
        break;
        case MENU_SWEEP_END_FREQ_K_INDEX:
          if (Check(SweepEndFreqM, SweepEndFreqK + SweepEncoderPos, SweepEndFreqH)) SweepEndFreqK = Dec(SweepEndFreqK, SweepEncoderPos);
        break;
        case MENU_SWEEP_END_FREQ_H_INDEX:
          if (Check(SweepEndFreqM, SweepEndFreqK, SweepEndFreqH + SweepEncoderPos)) SweepEndFreqH = Dec(SweepEndFreqH, SweepEncoderPos);
        break;

        case MENU_SWEEP_TIME_INDEX:
          if (SweepTime + SweepEncoderPos >= MIN_SWEEP_TIME) SweepTime = SweepTime + SweepEncoderPos; 
            else SweepTime = MAX_SWEEP_TIME;
        break;
      }
     }

    }

    DrawSweepMenu();
  }
}

void DrawSweepMenu()
{
    display.clearDisplay();
    display.setFont(&font);
    display.setTextColor(WHITE);
    
    display.setCursor(0, 15);
    display.print(F("Sweep GRA&AFCH"));
    
    display.setCursor(0, 30);
    display.print(F("From:"));
    if (SweepMenuPos == MENU_SWEEP_START_FREQ_M_INDEX) GetColor();
    display.print(PreZero(SweepStartFreqM));
    display.setTextColor(WHITE);

    if (SweepMenuPos == MENU_SWEEP_START_FREQ_K_INDEX) GetColor();
    display.print(PreZero(SweepStartFreqK));
    display.setTextColor(WHITE);

    if (SweepMenuPos == MENU_SWEEP_START_FREQ_H_INDEX) GetColor();
    display.print(PreZero(SweepStartFreqH));
    display.setTextColor(WHITE);

    display.setCursor(0, 42);
    display.print(F("To:"));
    display.setCursor(45, 42);

    if (SweepMenuPos == MENU_SWEEP_END_FREQ_M_INDEX) GetColor();
    display.print(PreZero(SweepEndFreqM));
    display.setTextColor(WHITE);

    if (SweepMenuPos == MENU_SWEEP_END_FREQ_K_INDEX) GetColor();
    display.print(PreZero(SweepEndFreqK));
    display.setTextColor(WHITE);

    if (SweepMenuPos == MENU_SWEEP_END_FREQ_H_INDEX) GetColor();
    display.print(PreZero(SweepEndFreqH));
    display.setTextColor(WHITE);
  
    display.setCursor(0, 54);
    if (SweepMenuPos == MENU_SWEEP_TIME_FORMAT_INDEX) GetColor();
    display.print(TimeFormatsNames[SweepTimeFormat]);
    display.setTextColor(WHITE);
    display.print(F(":"));
    
    display.setCursor(45, 54);
    if (SweepMenuPos == MENU_SWEEP_TIME_INDEX) GetColor();
    display.print(PreZero(SweepTime));
    display.setTextColor(WHITE);

    display.setCursor(0, 66);
    if (SweepMenuPos == MENU_SWEEP_START_INDEX) GetColor();
    if (SweepInProgress == true) {display.print(F("Stop")); display.setTextColor(WHITE); display.print(" ");}
      else display.print(F("Start"));
    display.setTextColor(WHITE);

    if (SweepMenuPos == MENU_SWEEP_MODE_INDEX)
    {
      display.fillRect(59, 52, 9, 11, WHITE);
      if (SweepMode == SWEEP_MODE_UP) display.drawBitmap(59, 52, bitmap_up, 9, 11, BLACK);
        else display.drawBitmap(59, 52, bitmap_up_down, 9, 11, BLACK); 
    } else
    {
      display.fillRect(59, 52, 9, 11, BLACK);
      if (SweepMode == SWEEP_MODE_UP) display.drawBitmap(59, 52, bitmap_up, 9, 11, WHITE);
        else display.drawBitmap(59, 52, bitmap_up_down, 9, 11, WHITE); 
    }

    if (SweepMenuPos == MENU_SWEEP_LOOP_INDEX)
    {
      display.fillRect(70, 52, 9, 11, WHITE);
      if (SweepLoop == SWEEP_LOOPED) display.drawBitmap(70, 52, bitmap_infinity, 9, 11, BLACK);
        else {display.setCursor(70, 66); display.setTextColor(BLACK); display.print(1); display.setTextColor(WHITE);}
    } else
    {
      display.fillRect(70, 52, 9, 11, BLACK);
      if (SweepLoop == SWEEP_LOOPED) display.drawBitmap(70, 52, bitmap_infinity, 9, 11, WHITE);
        else {display.setCursor(70, 66); display.setTextColor(WHITE); display.print(1); }
    } 

    display.setCursor(92, 66);
    if (SweepMenuPos == MENU_SWEEP_EXIT_INDEX) GetColor();
    display.print(F("Exit"));
    display.setTextColor(WHITE);
    
    display.display();
}

void SaveSweepSettings()
{
  //*******SWEEP VARIABLES
  EEPROM.put(SWEEP_START_FREQ_M_ADR, SweepStartFreqM);
  EEPROM.put(SWEEP_START_FREQ_K_ADR, SweepStartFreqK);
  EEPROM.put(SWEEP_START_FREQ_H_ADR, SweepStartFreqH);

  EEPROM.put(SWEEP_END_FREQ_M_ADR, SweepEndFreqM);
  EEPROM.put(SWEEP_END_FREQ_K_ADR, SweepEndFreqK);
  EEPROM.put(SWEEP_END_FREQ_H_ADR, SweepEndFreqH);

  EEPROM.put(SWEEP_TIME_ADR, SweepTime);
  EEPROM.put(SWEEP_TIME_FORMAT_ADR, SweepTimeFormat);

  EEPROM.put(SWEEP_MODE_ADR, SweepMode); 
  EEPROM.put(SWEEP_LOOP_ADR, SweepLoop); 

  EEPROM.write(SWEEP_SETTINGS_FLAG_ADR, 56);
}

void LoadSweepSettings()
{
  if (EEPROM.read(SWEEP_SETTINGS_FLAG_ADR) != 56)
  {
    Serial.println(F("Default sweep settings..."));
    //*********SWEEP**********
    SweepStartFreqM = INIT_SWEEP_START_FREQ_M;
    SweepStartFreqK = INIT_SWEEP_START_FREQ_K;
    SweepStartFreqH = INIT_SWEEP_START_FREQ_H;

    SweepEndFreqM = INIT_SWEEP_END_FREQ_M;
    SweepEndFreqK = INIT_SWEEP_END_FREQ_K;
    SweepEndFreqH = INIT_SWEEP_END_FREQ_H;

    SweepTime = INIT_SWEEP_TIME;
    SweepTimeFormat = INIT_SWEEP_TIME_FORMAT;

    SweepMode = INIT_SWEEP_MODE;
    SweepLoop = INIT_SWEEP_LOOP;

    SaveSweepSettings();
  } else
  {
    Serial.println(F("Loading Sweep Settings"));
    //**********SWEEP*****************
    EEPROM.get(SWEEP_START_FREQ_M_ADR, SweepStartFreqM);
    EEPROM.get(SWEEP_START_FREQ_K_ADR, SweepStartFreqK);
    EEPROM.get(SWEEP_START_FREQ_H_ADR, SweepStartFreqH);

    EEPROM.get(SWEEP_END_FREQ_M_ADR, SweepEndFreqM);
    EEPROM.get(SWEEP_END_FREQ_K_ADR, SweepEndFreqK);
    EEPROM.get(SWEEP_END_FREQ_H_ADR, SweepEndFreqH);

    EEPROM.get(SWEEP_TIME_ADR, SweepTime);
    EEPROM.get(SWEEP_TIME_FORMAT_ADR, SweepTimeFormat);

    EEPROM.get(SWEEP_MODE_ADR, SweepMode);
    EEPROM.get(SWEEP_LOOP_ADR, SweepLoop);
  }
}

uint32_t GetSweepTime() // возвращает время в миллисекунадх указанное пользователем в меню
{
  if (SweepTimeFormat == 0) return SweepTime * 1E3;
  else if (SweepTimeFormat == 1) return SweepTime;
}

//**********
// Return SweepStartFreq in HZ
//**********
uint32_t GetSweepStartFreq()
{
#if DBG==1
  Serial.print("SweepStartFreqM=");
  Serial.println(SweepStartFreqM);
  Serial.print("SweepStartFreqK=");
  Serial.println(SweepStartFreqK);
  Serial.print("SweepStartFreqH=");
  Serial.println(SweepStartFreqH);
#endif
  return SweepStartFreqM * 1000000UL + SweepStartFreqK * 1000UL + SweepStartFreqH;
}

//**********
// Return SweepEndFreq in HZ
//**********
uint32_t GetSweepEndFreq()
{
  return SweepEndFreqM * 1000000UL + SweepEndFreqK * 1000UL + SweepEndFreqH;
}

/*void DisplayMessage(String Title, String Message)
{
  display.clearDisplay();
  display.cp437(true);
  display.setTextSize(2);
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);
  display.print(Title);
  display.setTextSize(1);
  display.setCursor(5, 28);
  display.print(Message);

  display.display();
}*/

void SetSweepStartFreq(uint32_t freq)
{
  SweepStartFreqH = freq % 1000;
  freq = freq / 1000;
  SweepStartFreqK = freq % 1000;
  freq = freq / 1000;
  SweepStartFreqM = freq;
}

void SetSweepEndFreq(uint32_t freq)
{
  SweepEndFreqH = freq % 1000;
  freq = freq / 1000;
  SweepEndFreqK = freq % 1000;
  freq = freq / 1000;
  SweepEndFreqM = freq;
}

volatile uint32_t currFreq=0;
uint32_t stepSize = 0;
uint32_t startFreq = 0;
uint32_t stopFreq = 0;
volatile bool direction = 0;
volatile uint8_t sweepsQty = 0;

void MakeSweep()
{
  startFreq =  GetSweepStartFreq();
  stopFreq = GetSweepEndFreq();
  uint32_t timeMs = GetSweepTime();

  uint8_t intrrptPeriodMs = 1;
  uint32_t deltaFreq = abs(int32_t(stopFreq - startFreq));
  uint16_t stepsQty = timeMs / intrrptPeriodMs;
  stepSize = deltaFreq / stepsQty;
  currFreq = startFreq;
  if (startFreq < stopFreq) direction = 1;
    else 
    {
      uint32_t tmp = startFreq;
      startFreq = stopFreq;
      stopFreq = tmp;
      direction = 0;
    }

  if (SweepMode == SWEEP_MODE_UP) sweepsQty = 1;
    else sweepsQty = 2;
  if (SweepLoop == SWEEP_LOOPED) sweepsQty = 3; // любое значение отличное от 0, так как 0 признак остановки

  TimerSetup();
  TimerStart();
}

void TimerStart()
{
  ////disable encoder
  cli();
  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  digitalWrite(A_PIN, LOW);
  digitalWrite(B_PIN, LOW);

  TIMSK4 = (1 << OCIE4A);//TIMER4_COMPA_vect interrupt enable
  sei();
}

void TimerStop()
{
    TIMSK4 &= ~(1 << OCIE1A);//TIMER4_COMPA_vect interrupt disable
    cli();
    pinMode(A_PIN, INPUT_PULLUP); //enable encoder
    pinMode(B_PIN, INPUT_PULLUP); //enable encoder
    sei();
}

void TimerSetup()
{
  // а посчитаем-ка мы 
  TCCR4A = 0;             //control registers reset (WGM21, WGM20)
  TCCR4B = 0;             //control registers reset 
  TCCR4B = (1 << CS12)|(1 << CS10)|(1 << WGM12); //prescaler 1024 and CTC mode
  //OCR5A = 31; //2 mS
  TCNT4=0; //reset counter to 0
  //OCR4A = 46; //3mS
  //OCR4A = 2000; //
  OCR4A = 16; // 1ms
}

ISR(TIMER4_COMPA_vect)
{   
  if (direction == 1) 
  {
    currFreq = currFreq + stepSize;
    if (currFreq > stopFreq) 
    {
      if (SweepLoop == SWEEP_ONCE) sweepsQty--;
      if (sweepsQty == 0) {TimerStop(); SweepInProgress = false; return;}
        else 
        {
          if (SweepMode == SWEEP_MODE_UP_DOWN) direction = !direction;
            else currFreq = startFreq;
        }
    }
  } else 
  {
    currFreq = currFreq - stepSize;
    if (currFreq < startFreq) 
    {
      if (SweepLoop == SWEEP_ONCE) sweepsQty--;
      if (sweepsQty == 0) {TimerStop(); SweepInProgress = false; return;}
        else 
        {
          if (SweepMode == SWEEP_MODE_UP_DOWN) direction = !direction;
            else currFreq = stopFreq;
        }
    }
  }

  //DDS_Freq_Set(currFreq, 960000000); //оптимизировать или хотябы учитывать частоту ядра !!!!!!!!!
  DDS_Freq_Set(currFreq, DDS_Core_Clock);
}