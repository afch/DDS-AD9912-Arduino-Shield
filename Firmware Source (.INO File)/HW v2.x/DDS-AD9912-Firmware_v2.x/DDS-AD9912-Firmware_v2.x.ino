/****
Stable Core freq: 1280MHz
Max Core freq: 1360MHz
***/

//////////////////// ONLY FOR HARDWARE Version 2.x //////////////////////////

#include <Wire.h>
#include <stdint.h>
#include <inttypes.h>
#include <SPI.h>
#include "AD9912.h"
#include <math.h>

#include <Adafruit_GFX.h>
#include "Custom_Adafruit_SSD1306.h"
#include "font.h"
#include "menusweep.h"

#include <GParser.h>

#define DBG 0

#define FIRMWAREVERSION 2.08
//v2.08 06.10.2025 
//Fixed: Improved FTW calculation accuracy
//v2.07 12.02.2025
//Отключены выходы S1 и S4, которые конфликторвали с RF Switch, что в свою очередь приводило к неккоректной инициализации пути для источника тактирования
//v2.06 11.02.2025
//Добавлено управление светодиодами
//Добавлен новый новый тип источника тактирования - Ext TCXO/OCXO
  //!!! Неправильно применяется при включении !!!!!!
//v2.05 15.01.2025 
//анализ напряжения питания
//v2.04 07.01.2025 начат переход на версию платы HW 2.х
  //+Нужно добавить управление коммутатором
    //V1 - D9
    //V2 - D10
  //+Нужно добавить включение и выключение выхода аппаратной кнопкой "OUTPUT"
  //+Нужно добавить управление всеми светодиодами:
    //REF_LED - D13
    //HSTL_LED - D12
    //CMOS_LED - D11
  //Частота TCXO по-умолчанию установлена на 50 МГц
//v1.04 22.06.2023
//Turn-on(E) and turn-off(S) commands have been added.
//v1.03 20.06.2023
//Accelerated processing of commands transmitted via a serial port.
//Encoder performance has been improved.
//v1.02 01.06.2023
//Serial port remote control
//v1.01 26.04.2023
//EXT OSC renamed to REF CLK IN
//RFsin renamed to RFout

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Custom_Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

#include "main.h"

Encoder myEnc(B_PIN, A_PIN);

ClickButton modeButton(MODE_PIN, LOW, CLICKBTN_PULLUP);
ClickButton RFOutButton(RFOUT_PIN, LOW, CLICKBTN_PULLUP);

bool isPWR_DWN = false;
#define VOLTAGE_DIVIDER_FACTOR 0.1754 // 1k/(1k+4.7k)=0.1754
#define ANALOG_REFERENCE_VOLTAGE 4950
#define INPUT_VOLTAGE_PIN A0 //VIN_MEAS 

#define I2C2SLA 20
#define I2C2SCL 21

#define LOW_FREQ_LIMIT  100000
#define HIGH_FREQ_LIMIT  500000000
// 0 - 20 used for clock settings
#define M_ADR 24
#define K_ADR 28
#define H_ADR 32
#define A_ADR 36

#define HSTL_STATE_EEPROM_ADR 40
#define CMOS_STATE_EEPROM_ADR 44
#define CMOS_DIV_KHZ_EEPROM_ADR 48
#define CMOS_DIV_HZ_EEPROM_ADR 52

#define MAIN_SETTINGS_FLAG_ADR 100 // defualt settings 
// ADR 101 reserved for clock settings

#define INIT_M 100
#define INIT_K 0
#define INIT_H 0
#define INIT_A 7
#define INIT_HSTL_STATE HSTL_OFF
#define INIT_CMOS_STATE 0
#define INIT_CMOS_DIVIDER_KHZ 0
#define INIT_CMOS_DIVIDER_HZ 1

//*********************************
#define MAIN_MENU_MHZ_INDEX 0
#define MAIN_MENU_KHZ_INDEX 1
#define MAIN_MENU_HZ_INDEX 2
#define MAIN_MENU_HSTL_INDEX 3
#define MAIN_MENU_CMOS_INDEX 4
#define MAIN_MENU_CMOS_DIVIDER_KHZ_INDEX 5
#define MAIN_MENU_CMOS_DIVIDER_HZ_INDEX 6
#define MAIN_MENU_DBM_INDEX 7
#define MAIN_MENU_SWEEP_INDEX 8
#define MAIN_MENU_SK_INDEX 9

#define MAIN_MENU_LAST_INDEX MAIN_MENU_SK_INDEX

int M, K, H, A, MenuPos;
uint16_t CMOS_Divider;
uint16_t CMOS_Divider_KHz, CMOS_Divider_Hz;
bool CMOS_State;
#define CMOS_DIVIDER_MIN_VALUE 1
#define CMOS_DIVIDER_MAX_VALUE 65535

uint8_t HSTL_State;
#define HSTL_OFF 0
#define HSTL_ON 1
#define HSTL_ONx2 2

//                         -7  -6  -5   -4   -3   -2   -1    0   +1   +2   +3   +4 dBm
uint16_t dBmToCurrent[12]={15, 65, 120, 182, 250, 325, 415, 511, 615, 735, 865, 1000};
#define DBM_MIN_INDEX 0
#define DBM_MAX_INDEX 11

void setup()
{
  CMOS_Divider_Hz=2;
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  Serial.println(F("DDS AD9912 Arduino Shield by GRA & AFCH. (gra-afch.com)"));
  Serial.print(F("Firmware v.:"));
  Serial.println(FIRMWAREVERSION);

  Serial.print(F("Input voltage = "));
  Serial.println(getInputVoltage());

  if ((getInputVoltage() < 4) ||
      (getInputVoltage() > 8))
      {
        pinMode(PWR_DOWN_PIN, OUTPUT);
        digitalWrite(PWR_DOWN_PIN, HIGH);
        displayPowerWarning();
        while(1);
      }

  DisplayHello();
  delay(3000);

  modeButton.debounceTime   = 25;   // Debounce timer in ms
  modeButton.multiclickTime = 1;  // Time limit for multi clicks
  modeButton.longClickTime  = 1000; // time until "held-down clicks" register

  RFOutButton.debounceTime   = 25;   // Debounce timer in ms
  RFOutButton.multiclickTime = 1;  // Time limit for multi clicks
  RFOutButton.longClickTime  = 1000; // time until "held-down clicks" register

  modeButton.Update();

  if (modeButton.depressed == true) //если при включении была зажата кнопка DOWN, то затираем управляющие флаги в EEPROM, которые восстановят заводские значения всех параметров
  {
    Serial.println(F("Default settings..."));
    EEPROM.write(CLOCK_SETTINGS_FLAG_ADR, 255); //flag that force save default clock settings to EEPROM
    EEPROM.write(MAIN_SETTINGS_FLAG_ADR, 255); //flag that force save default main settings to EEPROM
    EEPROM.write(SWEEP_SETTINGS_FLAG_ADR, 255); //flag that force save default sweep settings to EEPROM
  }

  pinMode(EXTERANL_SRC_PATH_PIN, OUTPUT);
  pinMode(TCXO_PATH_PIN, OUTPUT);
  pinMode(TCXO_POWER_PIN, OUTPUT);

  pinMode(HSTL_LED_PIN, OUTPUT);
  pinMode(CMOS_LED_PIN, OUTPUT);

  digitalWrite(HSTL_LED_PIN, LOW);
  digitalWrite(CMOS_LED_PIN, LOW);

  LoadClockSettings();
  LoadMainSettings();
  LoadSweepSettings();

  DDS_Init();

  DrawMainMenu();
  
  MakeOut();
}

int dBm=0;
int curPos=0;

void loop()
{
  //DDS_SPKILL_CH1();
 while (1)
  {
    LEDsControl();
    ReadSerialCommands();
    curPos=0;
    curPos=myEnc.read();
    modeButton.Update();
    RFOutButton.Update();

    if (RFOutButton.clicks > 0) // short click 
    {
      isPWR_DWN = !isPWR_DWN;
      if (isPWR_DWN) 
      {
        digitalWrite(PWR_DOWN_PIN, HIGH);
      }
        else 
        {
          digitalWrite(PWR_DOWN_PIN, LOW);
        }
    }

    if (modeButton.clicks > 0) 
    {
      switch (MenuPos)
      {
        case MAIN_MENU_MHZ_INDEX:
        case MAIN_MENU_KHZ_INDEX:
        case MAIN_MENU_HZ_INDEX:
        case MAIN_MENU_CMOS_DIVIDER_KHZ_INDEX:
        case MAIN_MENU_CMOS_DIVIDER_HZ_INDEX:
        case MAIN_MENU_DBM_INDEX:
          EditMode = !EditMode;
        break;

        case MAIN_MENU_HSTL_INDEX:
          HSTL_State++;
          if (HSTL_State > HSTL_ONx2) HSTL_State = HSTL_OFF;
          SaveMainSettings();
          MakeOut();
        break;

        case MAIN_MENU_CMOS_INDEX:
          CMOS_State = !CMOS_State;
          SaveMainSettings();
          MakeOut();
        break;

        case MAIN_MENU_SWEEP_INDEX:
          SweepMenu();
        break;
      }
    }

    if (EditMode == false)
    {
    if (curPos > 0) 
    {
      MenuPos++;
      if (MenuPos > MAIN_MENU_LAST_INDEX) MenuPos = MAIN_MENU_MHZ_INDEX;
      if ((MenuPos == MAIN_MENU_CMOS_DIVIDER_KHZ_INDEX) && (CMOS_State == 0)) MenuPos = MAIN_MENU_DBM_INDEX; // Пропускаем даблер, если CMOS выключен
    }
    if (curPos < 0) 
    {
      MenuPos--;
      if (MenuPos < MAIN_MENU_MHZ_INDEX) MenuPos = MAIN_MENU_LAST_INDEX;
      if ((MenuPos == MAIN_MENU_CMOS_DIVIDER_HZ_INDEX) && (CMOS_State == 0)) MenuPos = MAIN_MENU_CMOS_INDEX; // Пропускаем даблер, если CMOS выключен
    }
    } else 
    {
     if (curPos > 0)
     {
      switch (MenuPos)
      {
        case MAIN_MENU_MHZ_INDEX:
          if (Check (M + curPos, K, H)) M = Inc(M, curPos);
        break;
        case MAIN_MENU_KHZ_INDEX:
          if (Check (M, K + curPos, H)) K = Inc(K, curPos);
        break;
        case MAIN_MENU_HZ_INDEX:
          if (Check (M, K, H + curPos)) H = Inc(H, curPos);
        break;
        //Check_Div
        case MAIN_MENU_CMOS_DIVIDER_KHZ_INDEX:
          if (Check_Div (CMOS_Divider_KHz + curPos, CMOS_Divider_Hz)) CMOS_Divider_KHz = Inc(CMOS_Divider_KHz, curPos);
        break;
        case MAIN_MENU_CMOS_DIVIDER_HZ_INDEX:
          if (Check_Div (CMOS_Divider_KHz, CMOS_Divider_Hz + curPos)) CMOS_Divider_Hz = Inc(CMOS_Divider_Hz, curPos);
        break;
        //dBmIndex
        case MAIN_MENU_DBM_INDEX:
          if (A+1 <= DBM_MAX_INDEX) A++; 
        break;
      }
     }

     if (curPos < 0)
     {
      switch (MenuPos)
      {
        case MAIN_MENU_MHZ_INDEX:
          if (Check(M + curPos, K, H)) M = Dec(M, curPos);
        break;
        case MAIN_MENU_KHZ_INDEX:
          if (Check(M, K + curPos, H)) K = Dec(K, curPos);
        break;
        case MAIN_MENU_HZ_INDEX:
          if (Check(M, K, H + curPos)) H = Dec(H, curPos);
        break;
        //Check_Div
        case MAIN_MENU_CMOS_DIVIDER_KHZ_INDEX:
          if (Check_Div(CMOS_Divider_KHz + curPos, CMOS_Divider_Hz)) CMOS_Divider_KHz = Dec(CMOS_Divider_KHz, curPos);
        break;
        case MAIN_MENU_CMOS_DIVIDER_HZ_INDEX:
          if (Check_Div(CMOS_Divider_KHz, CMOS_Divider_Hz + curPos)) CMOS_Divider_Hz = Dec(CMOS_Divider_Hz, curPos);
        break;
        //dBmIndex
        case MAIN_MENU_DBM_INDEX:
          if (A-1 >= DBM_MIN_INDEX) A--; 
        break;
      }
     }

    if (curPos != 0) SaveMainSettings();
    }

    if (curPos != 0) MakeOut();

    if (modeButton.clicks < 0) DDS_Clock_Config_Menu();

    DrawMainMenu();
  }
}

uint32_t GetFreqValue()
{
  return M * 1000000L + K * 1000L + H;
}

uint16_t GetCMOSDividerValue()
{
  CMOS_Divider = CMOS_Divider_KHz * 1000L + CMOS_Divider_Hz;
  if (CMOS_Divider != 0) return CMOS_Divider;
    else return 1;
}

void MakeOut()
{
  if (HSTL_State == HSTL_OFF)
  {
    Power_Down_Reset_Register_Value |= Disable_PD_HSTL_Driver;
  }

  if (HSTL_State == HSTL_ON)
  {
    Power_Down_Reset_Register_Value &= ~Disable_PD_HSTL_Driver;
    bitSet(HSTL_Driver_Register_Value, HSTL_Doubler_Disabled-1);
    bitClear(HSTL_Driver_Register_Value, HSTL_Doubler_Enabled-1);
    Power_Down_Reset_Register_Value &= ~Enable_Output_Doubler;
  }

  if (HSTL_State == HSTL_ONx2)
  {
    Power_Down_Reset_Register_Value &= ~Disable_PD_HSTL_Driver;
    bitSet(HSTL_Driver_Register_Value, HSTL_Doubler_Enabled-1);
    bitClear(HSTL_Driver_Register_Value, HSTL_Doubler_Disabled-1);
    Power_Down_Reset_Register_Value |= Enable_Output_Doubler;
  }

  if (CMOS_State == 0)
  {
    Power_Down_Reset_Register_Value &= ~Enable_CMOS_Driver;
  } else
  {
    Power_Down_Reset_Register_Value |= Enable_CMOS_Driver;
  }

  if (GetCMOSDividerValue() == 1)
  {
    CMOS_Driver_Register_Value &= ~CMOS_Mux;
  } else
  {
    DDS_Config(S_Divider_LSB_ADDR, lowByte(GetCMOSDividerValue()-1));
    DDS_Config(S_Divider_MSB_ADDR, highByte(GetCMOSDividerValue()-1));
    CMOS_Driver_Register_Value |= CMOS_Mux;
    DDS_Config(CMOS_out_divider_ADDR, 0); // disable s/2 divider
  }
  
  //S_Divider_MSB_ADDR

  DDS_Config(Power_Down_Reset_ADDR, Power_Down_Reset_Register_Value);
  DDS_Config(HSTL_Driver_ADDR, HSTL_Driver_Register_Value);
  DDS_Config(CMOS_Driver_ADDR, CMOS_Driver_Register_Value);

  DDS_Current(dBmToCurrent[A]);
  //DDS_Freq_Set(M * 1000000L + K * 1000L + H, DDS_Core_Clock); //GetDDSCoreClock()
  DDS_Freq_Set(GetFreqValue(), DDS_Core_Clock); //GetDDSCoreClock()

  //SingleProfileFreqOut(M * 1000000L + K * 1000L + H, A * -1);

  //Sweep(GetSweepStartFreq(), GetSweepEndFreq(), SweepTime, SweepTimeFormat);
}

String PreZero2(int Digit)
{
  if (Digit < 10) return "0" + String(Digit);
  return String(Digit);
}

int32_t Inc(int32_t val, int incr)
{
  //val = val + increment;
  //val = val + incrValue;
  val = val + incr;
  if (val > 999) val = 999;
  return val;
}

int32_t Dec(int32_t val, int decr)
{
  //val = val - decrement;
  //val = val - decrValue;
  val = val + decr;
  if (val < 0) val = 0;
  return val;
}


/*****************************************************************

 ***************************************************************/
bool Check(uint16_t _M, uint16_t _K, uint16_t _H)
{
  long F_Val;
  F_Val = _M * 1000000L + _K * 1000L + _H;
  if ((F_Val >= LOW_FREQ_LIMIT) && (F_Val <= HIGH_FREQ_LIMIT)) {GetFreqValue(); return 1;}
  else return 0;
}

bool Check_Div(uint16_t _K, uint16_t _H)
{
  long Div_Val;
  Div_Val =_K * 1000L + _H;
  if ((Div_Val >= CMOS_DIVIDER_MIN_VALUE) && (Div_Val <= CMOS_DIVIDER_MAX_VALUE)) {GetCMOSDividerValue(); return 1;}
  else return 0;
}

void SaveMainSettings()
{
  EEPROM.put(M_ADR, M);
  EEPROM.put(K_ADR, K);
  EEPROM.put(H_ADR, H);
  EEPROM.put(A_ADR, A);

  EEPROM.put(HSTL_STATE_EEPROM_ADR, HSTL_State);
  EEPROM.put(CMOS_STATE_EEPROM_ADR, CMOS_State);
  EEPROM.put(CMOS_DIV_KHZ_EEPROM_ADR, CMOS_Divider_KHz);
  EEPROM.put(CMOS_DIV_HZ_EEPROM_ADR, CMOS_Divider_Hz);

  EEPROM.write(MAIN_SETTINGS_FLAG_ADR, 55);
}

/**************************************************************************
#define INIT_HSTL_STATE HSTL_OFF
#define INIT_CMOS_STATE 0
#define INIT_CMOS_DIVIDER_KHZ 0
#define INIT_CMOS_DIVIDER_HZ 1
 *************************************************************************/
void LoadMainSettings()
{
  Serial.println(F("Loading Main Settings"));
  if (EEPROM.read(MAIN_SETTINGS_FLAG_ADR) != 55)
  {
    M = INIT_M;
    K = INIT_K;
    H = INIT_H;
    A = INIT_A;
    HSTL_State = INIT_HSTL_STATE;
    CMOS_State = INIT_CMOS_STATE;
    CMOS_Divider_KHz = INIT_CMOS_DIVIDER_KHZ;
    CMOS_Divider_Hz = INIT_CMOS_DIVIDER_HZ;
    SaveMainSettings();
#if DBG==1
    Serial.println(F("Loading init values"));
    Serial.print("M=");
    Serial.println(M);
    Serial.print("K=");
    Serial.println(K);
    Serial.print("H=");
    Serial.println(H);
    Serial.print("A=");
    Serial.println(A);
    Serial.print("HSTL_State=");
    Serial.println(HSTL_State);
    Serial.print("CMOS_State=");
    Serial.println(CMOS_State);
    Serial.print("CMOS_Divider_KHz=");
    Serial.println(CMOS_Divider_KHz);
    Serial.print("CMOS_Divider_Hz=");
    Serial.println(CMOS_Divider_Hz);
#endif
  }
  else
  {
    EEPROM.get(M_ADR, M);
    EEPROM.get(K_ADR, K);
    EEPROM.get(H_ADR, H);
    EEPROM.get(A_ADR, A);

    EEPROM.get(HSTL_STATE_EEPROM_ADR, HSTL_State);
    EEPROM.get(CMOS_STATE_EEPROM_ADR, CMOS_State);
    EEPROM.get(CMOS_DIV_KHZ_EEPROM_ADR, CMOS_Divider_KHz);
    EEPROM.get(CMOS_DIV_HZ_EEPROM_ADR, CMOS_Divider_Hz);
#if DBG==1
    Serial.println(F("Values from EEPROM"));
    Serial.print("M=");
    Serial.println(M);
    Serial.print("K=");
    Serial.println(K);
    Serial.print("H=");
    Serial.println(H);
    Serial.print("A=");
    Serial.println(A);
    Serial.print("HSTL_State=");
    Serial.println(HSTL_State);
    Serial.print("CMOS_State=");
    Serial.println(CMOS_State);
    Serial.print("CMOS_Divider_KHz=");
    Serial.println(CMOS_Divider_KHz);
    Serial.print("CMOS_Divider_Hz=");
    Serial.println(CMOS_Divider_Hz);
#endif
  }
}

void DisplayHello()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);
  display.print(F("Tips:"));
  display.setTextSize(1);
  display.setCursor(62, 0);
  display.print(F("Firmware"));
  display.setCursor(62, 8);
  display.print(F("ver.: "));
  display.print(FIRMWAREVERSION);
  display.setCursor(0, 20);

  display.print(F("1. Hold MODE to enterSetup."));

  display.setCursor(0, 40);
  display.print(F("2. Hold MODE, while  Powering On, to Resetall settings."));

  display.display();
}

void DrawMainMenu()
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(65, 0);
  if (MenuPos==MAIN_MENU_SWEEP_INDEX) GetColor();
  display.print(F("Sweep"));
  display.setTextColor(WHITE);

  display.setCursor(65, 8);
  if (MenuPos==MAIN_MENU_SK_INDEX) GetColor();
  display.print(F("SK"));
  display.setTextColor(WHITE);

  display.setCursor(75, 8);
  display.print(F(":"));
  display.setCursor(80, 8);
  display.print(F("A/B"));

  display.setFont(&font);
  display.setCursor(0, 15); //
  //display.setTextColor(BLACK, WHITE); 
  display.setTextColor(WHITE); 
  display.print(F("DDS9912")); 
  display.setCursor(100, 15);
  display.print(F("G&A"));

  display.setCursor(0, 30);
  display.print(F("RF"));

  display.setFont(NULL);
  display.setCursor(18, 20);
  if (!isPWR_DWN) display.print(F("Out"));
    else display.print(F("OFF"));
  display.setFont(&font);
  
  display.setCursor(33, 30);
  display.print(F(":"));

  display.setCursor(47, 30);
  if (MenuPos==MAIN_MENU_MHZ_INDEX) GetColor();
  display.print(PreZero(M));
  display.setTextColor(WHITE);

  display.setCursor(74, 30);
  if (MenuPos==MAIN_MENU_KHZ_INDEX) GetColor();
  display.print(PreZero(K));
  display.setTextColor(WHITE);

  display.setCursor(101, 30);
  if (MenuPos==MAIN_MENU_HZ_INDEX) GetColor();
  display.print(PreZero(H));
  display.setTextColor(WHITE);
  
  display.setCursor(0, 42);
  if (MenuPos==MAIN_MENU_HSTL_INDEX) GetColor();
  display.print(F("HSTL"));
  display.setTextColor(WHITE);

  display.setCursor(33, 42);
  display.print(F(":"));

  display.setCursor(38, 42);
  if (HSTL_State != HSTL_OFF)
  {
    char chrHSTLtmpBuff[10]; //Перенести в глобальные переменные
    if (HSTL_State == HSTL_ON) sprintf(chrHSTLtmpBuff, "%10lu", GetFreqValue());
      else sprintf(chrHSTLtmpBuff, "%10lu", GetFreqValue()*2);
    display.print(chrHSTLtmpBuff); //
  } else display.print(F(" ---OFF---"));
    
  display.setCursor(0, 54);
  if (MenuPos==MAIN_MENU_CMOS_INDEX) GetColor();
  display.print(F("CMOS"));
 
  display.setTextColor(WHITE);

  display.setCursor(33, 54);
  display.print(F(":"));

  display.setCursor(47, 54);
  if (CMOS_State == true)
  {
    char chrCMOStmpBuff[9]; //Перенести в глобальные переменные
    sprintf(chrCMOStmpBuff, "%9lu", GetFreqValue()/GetCMOSDividerValue());
    display.print(chrCMOStmpBuff);
  } else display.print(F("---OFF---"));
  
  if (CMOS_State == true)
  {
    display.setCursor(0, 66);
    display.print(F("Div"));
    display.setCursor(25, 66);
    display.print(F(":"));

    display.setCursor(32, 66);
    if (MenuPos==MAIN_MENU_CMOS_DIVIDER_KHZ_INDEX) GetColor();
    display.print(PreZero2(CMOS_Divider_KHz));
    display.setTextColor(WHITE);

    display.setCursor(50, 66);
    if (MenuPos==MAIN_MENU_CMOS_DIVIDER_HZ_INDEX) GetColor();
    display.print(PreZero(CMOS_Divider_Hz));
    display.setTextColor(WHITE);
  }
  display.setCursor(92, 66);
  if (MenuPos==MAIN_MENU_DBM_INDEX) GetColor();
  if (A-7 >= 0) display.print(" ");
  display.print(A - 7); //7 это сдвижка в массиве со значениями тока
  display.setTextColor(WHITE);

  display.setFont(NULL);
  display.setCursor(110, 56);
  display.print(F("dBm"));
  
  display.display();
}

void selectClockSrcPath(uint8_t path)
{
  switch (path)
  {
    case CLOCK_SOURCE_XO_INDEX:
      digitalWrite(TCXO_POWER_PIN, HIGH); //оставляем питание на встроенного TCXO чтобы не светился светодиод REF_LED!
      digitalWrite(TCXO_PATH_PIN, HIGH); //нужно сделть сразу две изоляции между j1, j2 и j3
      digitalWrite(EXTERANL_SRC_PATH_PIN, HIGH); //нужно сделть сразу две изоляции между j1, j2 и j3
    break;
    case CLOCK_SOURCE_TCXO_INDEX:
      digitalWrite(TCXO_POWER_PIN, HIGH);
      digitalWrite(TCXO_PATH_PIN, HIGH);
      digitalWrite(EXTERANL_SRC_PATH_PIN, LOW);
    break;
    case CLOCK_SOURCE_EXT_TCXO_INDEX:  //////////// ЭТО ЭКСТЕРНАЛ TCXO!!!!!!!!!!  //
      digitalWrite(TCXO_POWER_PIN, LOW);
      digitalWrite(TCXO_PATH_PIN, LOW);
      digitalWrite(EXTERANL_SRC_PATH_PIN, HIGH);
    break;
    case CLOCK_SOURCE_EXT_OSC_INDEX:
      digitalWrite(TCXO_POWER_PIN, LOW);
      digitalWrite(TCXO_PATH_PIN, LOW);
      digitalWrite(EXTERANL_SRC_PATH_PIN, HIGH);
    break;    
  }
}

float getInputVoltage()
{
  uint16_t voltage=0; //mV
  analogReference(DEFAULT); 
  delay(10);
  voltage = map(analogRead(INPUT_VOLTAGE_PIN), 0, 1023, 0, ANALOG_REFERENCE_VOLTAGE); //4.95
  return (float(voltage) / 1000.0 / VOLTAGE_DIVIDER_FACTOR) + 0.7; // из-за падения на диоде установленном на плате Arduino
}

void displayPowerWarning()
{
  static const unsigned char PROGMEM image_alert_bicubic_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,0x00,0x00,0x07,0xe0,0x00,0x00,0x00,0x00,0x07,0xe0,0x00,0x00,0x00,0x00,0x0f,0xf0,0x00,0x00,0x00,0x00,0x0e,0x70,0x00,0x00,0x00,0x00,0x1c,0x38,0x00,0x00,0x00,0x00,0x3c,0x3c,0x00,0x00,0x00,0x00,0x38,0x1c,0x00,0x00,0x00,0x00,0x78,0x1e,0x00,0x00,0x00,0x00,0x70,0x0e,0x00,0x00,0x00,0x00,0xf0,0x0f,0x00,0x00,0x00,0x00,0xe3,0xc7,0x00,0x00,0x00,0x01,0xc3,0xc3,0x80,0x00,0x00,0x03,0xc3,0xc3,0xc0,0x00,0x00,0x03,0x83,0xc1,0xc0,0x00,0x00,0x07,0x83,0xc1,0xe0,0x00,0x00,0x07,0x03,0xc0,0xe0,0x00,0x00,0x0e,0x03,0xc0,0xf0,0x00,0x00,0x0e,0x03,0xc0,0x70,0x00,0x00,0x1c,0x03,0xc0,0x38,0x00,0x00,0x3c,0x03,0xc0,0x3c,0x00,0x00,0x38,0x03,0xc0,0x1c,0x00,0x00,0x78,0x03,0xc0,0x1e,0x00,0x00,0x70,0x03,0xc0,0x0e,0x00,0x00,0xe0,0x03,0xc0,0x07,0x00,0x01,0xe0,0x03,0xc0,0x07,0x80,0x01,0xc0,0x03,0xc0,0x03,0x80,0x03,0xc0,0x01,0x80,0x03,0xc0,0x03,0x80,0x00,0x00,0x01,0xc0,0x07,0x80,0x00,0x00,0x01,0xe0,0x07,0x00,0x01,0x80,0x00,0xe0,0x0e,0x00,0x03,0xc0,0x00,0x70,0x1e,0x00,0x03,0xc0,0x00,0x78,0x1c,0x00,0x01,0x80,0x00,0x38,0x3c,0x00,0x00,0x00,0x00,0x3c,0x38,0x00,0x00,0x00,0x00,0x1c,0x38,0x00,0x00,0x00,0x00,0x1c,0x3f,0xff,0xff,0xff,0xff,0xfc,0x1f,0xff,0xff,0xff,0xff,0xf8,0x01,0xff,0xff,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  display.clearDisplay();
  display.setTextColor(1);
  display.setTextSize(2);
  display.setCursor(23, 1);
  display.print("WARNING");
  display.setTextWrap(false);
  display.setCursor(43, 17);
  display.print("7.5 VDC");
  display.setTextSize(1);
  display.setCursor(49, 35);
  display.print("POWER SUPPLY");
  display.setTextSize(2);
  display.setCursor(72, 45);
  display.print("USB");
  display.drawBitmap(1, 11, image_alert_bicubic_bits, 48, 48, 1);
  display.setTextSize(1);
  display.setCursor(55, 48);
  display.print("or");
  display.display();
}

void LEDsControl()
{
  static uint32_t lastTimeLEDsControl = 0;

  if ((millis() - lastTimeLEDsControl) < 500) return;
  lastTimeLEDsControl = millis();

  if (HSTL_State == HSTL_OFF) digitalWrite(HSTL_LED_PIN, LOW);
    else digitalWrite(HSTL_LED_PIN, HIGH);

  if (CMOS_State == 0) digitalWrite(CMOS_LED_PIN, LOW);
    else digitalWrite(CMOS_LED_PIN, HIGH);

  if (isPWR_DWN)
  {
    digitalWrite(HSTL_LED_PIN, LOW); 
    digitalWrite(CMOS_LED_PIN, LOW);
  }
}
