
//#include "main.h"
#include "menuclk.h"

#define DBG 0

 bool EditMode=false;
 uint8_t incrValue=0;
 uint8_t decrValue=0;

  int SetupMenuPos=0;
  
  int ClockSourceIndex=INIT_CLOCK_SOURCE_INDEX;
  String ClockSourceName[3]={"XO AT-cut", "TCXO/OCXO", "REF CLK IN"};
  #define CLOCK_SOURCE_XO_INDEX         0
  #define CLOCK_SOURCE_TCXO_INDEX       1
  #define CLOCK_SOURCE_EXT_OSC_INDEX    2
  uint32_t XO_Values[2]={20000000, 25000000}; // Гц
  #define XO_QTY 2
  #define XO_MIN_VALUE 20000000 // Гц
  #define XO_MAX_VALUE 25000000 // Гц
  //uint32_t XO_Freq=XO_MIN_VALUE;
  int XO_Freq_Index=0;
  uint32_t TCXO_Values[5]={10000000, 20000000, 25000000, 40000000, 50000000}; // Гц
  #define TCXO_QTY 5
  #define TCXO_MIN_VALUE 10000000 // Гц
  #define TCXO_MAX_VALUE 50000000 // Гц
  //uint32_t TCXO_Freq=TCXO_MIN_VALUE;
  int TCXO_Freq_Index=INIT_CLOCK_TCXO_INDEX;
  #define N_MULTIPLIER_DEFAULT_VALUE 12
  #define N_MULTIPLIER_MIN_VALUE 2
  #define N_MULTIPLIER_MAX_VALUE 33
  uint8_t N_Multiplier = N_MULTIPLIER_DEFAULT_VALUE;
  int Max_Freq_Index=0;
  #define EXT_OSC_MIN_VALUE 250000000 //Гц
  #define EXT_OSC_MAX_VALUE 1500000000 //Гц
  //int EXT_OSC_Value=EXT_OSC_MIN_VALUE;
  String SRCDoublerName[2]={"OFF", "ON"};
  int SRCDoublerIndex=0;
  #define BASE_DDS_CORE_CLOCK 1000000000.0 //это частота на которую будет сбрасываться значение DDS_Core_Clock в любых "непонятных" случаях
    
  #define MAX_DDS_CORE_CLOCK 1500000000.0 //максимально допустимое значение DDS_CORE_CLOCK
  #define MIN_DDS_CORE_CLOCK 250000000.0
  #define MAX_PLL_FREQ 1380000000
  #define MIN_PLL_FREQ 700000000
  uint32_t EXT_OSC_Freq=BASE_DDS_CORE_CLOCK;
  uint32_t DDS_Core_Clock=BASE_DDS_CORE_CLOCK; // частота ядра
  //uint32_t Ref_Clk=0; //реальная частота тактирующего генератора в Герцах, хранит реальное значение частоты для любого источника и для кварца и для припаянных генераторов и для внешней источника, подключенного ко входу INOSC  
  uint32_t Ref_Clk=TCXO_Values[INIT_CLOCK_TCXO_INDEX]; //реальная частота тактирующего генератора в Герцах, хранит реальное значение частоты для любого источника и для кварца и для припаянных генераторов и для внешней источника, подключенного ко входу INOSC  

void DisplayClockSetupMenu()
{
  display.setFont(NULL);
  display.clearDisplay();
  display.setTextSize(2);      
  display.setTextColor(WHITE); 
  //display.setCursor(40, 0);  
  display.setCursor(0, 0);     
  display.cp437(true);         
  display.println("SETUP");

    //*********
  display.setTextSize(1); 
  display.setCursor(62, 0);
  display.println("DDS CORE");
  display.setCursor(62, 8);
  display.print("Clock: ");
  if (SetupMenuPos==CLOCK_MENU_DDS_CORE_CLOCK_INDEX) GetColor(); //display.setTextColor(BLACK, WHITE); 
  display.print(DDS_Core_Clock/1000000); 
  display.setTextColor(WHITE);
   //***********
  
  display.setTextSize(1);
  display.setCursor(0, 17);
  display.print("Clock Src: ");
  if (SetupMenuPos==CLOCK_MENU_SOURCE_INDEX) GetColor(); //display.setTextColor(BLACK, WHITE);
  display.print(ClockSourceName[ClockSourceIndex]); 
  display.setTextColor(WHITE);

  display.setCursor(0, 26);
  display.print("Clock Freq: ");
  if (SetupMenuPos==CLOCK_MENU_FREQ_INDEX) GetColor(); //display.setTextColor(BLACK, WHITE);
  display.print(Ref_Clk/1000000); 
  display.print(" MHz");
  display.setTextColor(WHITE);

  if (ClockSourceIndex != CLOCK_SOURCE_EXT_OSC_INDEX)
  {
    display.setCursor(0, 35);
    display.print("Src Doubler: ");
    if (SetupMenuPos==CLOCK_MENU_DOUBLER_INDEX) GetColor(); //display.setTextColor(BLACK, WHITE);
    display.print(SRCDoublerName[SRCDoublerIndex]);
    display.setTextColor(WHITE);
  }

  display.setCursor(0, 55);
  if (SetupMenuPos==CLOCK_MENU_SAVE_INDEX) display.setTextColor(BLACK, WHITE);
  display.println("SAVE");

  display.setTextColor(WHITE);
  display.setCursor(103, 55);
  if (SetupMenuPos==CLOCK_MENU_EXIT_INDEX) display.setTextColor(BLACK, WHITE);
  display.println("EXIT");

  display.display();
}

void GetColor()
{
  static uint32_t lastTimeColorChangedTime=millis();
  static uint8_t color=1; //1 - white (normal color), 0 - black;
  if (EditMode == false)
  {
    display.setTextColor(BLACK, WHITE);
    return;
  }
  if ((millis()-lastTimeColorChangedTime)>333)
      {
        lastTimeColorChangedTime=millis();
        color=!color;
      }
  if (color == 1)
  {
    display.setTextColor(WHITE);
  } else 
  {
    display.setTextColor(BLACK, WHITE);
  }
}

void DDS_Clock_Config_Menu()
{
  EditMode=false; 
  GetDDSCoreClock();

  while(1)
  {
  
  curPos=0;
  curPos=myEnc.read();
  modeButton.Update();

  if ((curPos !=0) || (modeButton.clicks !=0)) GetDDSCoreClock();

  if (modeButton.clicks !=0) 
  {
    switch (SetupMenuPos)
    {
      case CLOCK_MENU_SOURCE_INDEX:
      case CLOCK_MENU_FREQ_INDEX:
      case CLOCK_MENU_DDS_CORE_CLOCK_INDEX:
        EditMode = !EditMode;
      break;
      case CLOCK_MENU_DOUBLER_INDEX:
        SRCDoublerIndex =! SRCDoublerIndex;  
          if (SRCDoublerIndex == 1) 
          {
            N_Multiplier = N_Multiplier / 2;
          } else
          {
            FindBestNMultiplier();
          }
      break;
      case CLOCK_MENU_SAVE_INDEX: 
        SaveClockSettings(); 
        MakeOut(); // заглушка - еще не реализовано!!!!!!!!!
        DisplaySaved(); 
        delay(1000); 
        SetupMenuPos=CLOCK_MENU_EXIT_INDEX; 
      break;
      case CLOCK_MENU_EXIT_INDEX: LoadClockSettings(); DrawMainMenu(); MakeOut(); return; 
      break;
    }
  }
   
  if (EditMode == false)
  {
    if (curPos > 0) 
    {
      SetupMenuPos++;
      if (SetupMenuPos>CLOCK_MENU_LAST_INDEX) SetupMenuPos=CLOCK_MENU_SOURCE_INDEX;
      if ((ClockSourceIndex==CLOCK_SOURCE_EXT_OSC_INDEX) && (SetupMenuPos==CLOCK_MENU_DOUBLER_INDEX)) SetupMenuPos=CLOCK_MENU_SAVE_INDEX; //пропускаем настройку даблера если выбрано внешнее тактирование
    }
    if (curPos < 0) 
    {
      SetupMenuPos--;
      if (SetupMenuPos < CLOCK_MENU_SOURCE_INDEX) SetupMenuPos = CLOCK_MENU_LAST_INDEX;
      if ((ClockSourceIndex==CLOCK_SOURCE_EXT_OSC_INDEX) && (SetupMenuPos==CLOCK_MENU_DDS_CORE_CLOCK_INDEX)) SetupMenuPos=CLOCK_MENU_FREQ_INDEX; //пропускаем настройку даблера если выбрано внешнее тактирование
    }
  } else 
    {
      if (curPos > 0) incrValue = 1;
      if (curPos < 0) decrValue = 1;
    }

    //if (modeButton.clicks !=0) SetupMenuPos++;
    
    if (ClockSourceIndex==CLOCK_SOURCE_EXT_OSC_INDEX) DDS_Core_Clock=EXT_OSC_Freq; // /(SRCDoublerIndex+1);
      
   /* if (incrValue != 0)  // ускоренная перемотка //curPos
    {
      if (ClockSourceIndex==CLOCK_SOURCE_EXT_OSC_INDEX) 
      {
        uint32_t increment=1000000;
        if ((millis()-UpButtonHoldReleased)>2000) increment=10000000;
          else increment=1000000;
        EXT_OSC_Freq=EXT_OSC_Freq+increment;
        if (EXT_OSC_Freq>EXT_OSC_MAX_VALUE) EXT_OSC_Freq=EXT_OSC_MIN_VALUE;
      } */
      
    
    if (incrValue != 0)
    {
      switch (SetupMenuPos)
        {
        case CLOCK_MENU_SOURCE_INDEX: 
          ClockSourceIndex++;
          if (ClockSourceIndex>CLOCK_SOURCE_EXT_OSC_INDEX) ClockSourceIndex=0;
          //DDS_Core_Clock=BASE_DDS_CORE_CLOCK; 
          FindBestNMultiplier();
          break;
        case CLOCK_MENU_FREQ_INDEX: 
          if (ClockSourceIndex == CLOCK_SOURCE_XO_INDEX) {XO_Freq_Index++; FindBestNMultiplier();}
          if (ClockSourceIndex == CLOCK_SOURCE_TCXO_INDEX) {TCXO_Freq_Index++; FindBestNMultiplier();}
          if (ClockSourceIndex == CLOCK_SOURCE_EXT_OSC_INDEX) EXT_OSC_Freq=EXT_OSC_Freq+1000000;
          if (EXT_OSC_Freq>EXT_OSC_MAX_VALUE) EXT_OSC_Freq=EXT_OSC_MIN_VALUE;
          if (ClockSourceIndex != CLOCK_SOURCE_EXT_OSC_INDEX) DDS_Core_Clock=BASE_DDS_CORE_CLOCK; 
          break;
        case CLOCK_MENU_DDS_CORE_CLOCK_INDEX: 
          N_Multiplier++; //DDS_Core_Clock=DDS_Core_Clock + Ref_Clk;
          GetDDSCoreClock(); 
          //if (DDS_Core_Clock > MAX_DDS_CORE_CLOCK) DDS_Core_Clock=BASE_DDS_CORE_CLOCK;
          if ((DDS_Core_Clock > MAX_PLL_FREQ) || 
              (N_Multiplier > N_MULTIPLIER_MAX_VALUE)) {N_Multiplier--; GetDDSCoreClock();}
          break;
        /*case CLOCK_MENU_SAVE_INDEX: 
          SaveClockSettings(); 
          MakeOut(); // заглушка - еще не реализовано!!!!!!!!!
          DisplaySaved(); 
          delay(1000); 
          SetupMenuPos=CLOCK_MENU_EXIT_INDEX; 
          break;
        case CLOCK_MENU_EXIT_INDEX: LoadClockSettings(); DrawMainMenu(); MakeOut(); return; 
          break;*/
        }
      GetDDSCoreClock();
    }

    /*if (DownButtonFunction < 0 && downButton.depressed == true)  // ускоренная перемотка //curPos
    {
      if (ClockSourceIndex==CLOCK_SOURCE_EXT_OSC_INDEX) 
      {
        uint32_t increment=1000000;
        if ((millis()-DownButtonHoldReleased)>2000) increment=10000000;
          else increment=1000000;
        EXT_OSC_Freq=EXT_OSC_Freq-increment;
        if (EXT_OSC_Freq<EXT_OSC_MIN_VALUE) EXT_OSC_Freq=EXT_OSC_MAX_VALUE;
      }*/
      

    if (decrValue  != 0)
    {
      switch (SetupMenuPos)
      {
        case CLOCK_MENU_SOURCE_INDEX: 
          ClockSourceIndex--;
          if (ClockSourceIndex<CLOCK_SOURCE_XO_INDEX) ClockSourceIndex=CLOCK_SOURCE_EXT_OSC_INDEX;
          //DDS_Core_Clock=BASE_DDS_CORE_CLOCK; 
          FindBestNMultiplier();
          break;
        case CLOCK_MENU_FREQ_INDEX: 
          if (ClockSourceIndex == CLOCK_SOURCE_XO_INDEX) {XO_Freq_Index--; FindBestNMultiplier();}
          if (ClockSourceIndex == CLOCK_SOURCE_TCXO_INDEX) {TCXO_Freq_Index--; FindBestNMultiplier();}
          if (ClockSourceIndex == CLOCK_SOURCE_EXT_OSC_INDEX) EXT_OSC_Freq=EXT_OSC_Freq-1000000;
          if (EXT_OSC_Freq<EXT_OSC_MIN_VALUE) EXT_OSC_Freq=EXT_OSC_MAX_VALUE;
          if (ClockSourceIndex != CLOCK_SOURCE_EXT_OSC_INDEX) DDS_Core_Clock=BASE_DDS_CORE_CLOCK;
          break;
        case CLOCK_MENU_DDS_CORE_CLOCK_INDEX:
          N_Multiplier--;
          GetDDSCoreClock(); 
          if ((DDS_Core_Clock < MIN_PLL_FREQ) || 
              (N_Multiplier < N_MULTIPLIER_MIN_VALUE)) {N_Multiplier++; GetDDSCoreClock();}
          break;
        /*case CLOCK_MENU_SAVE_INDEX:
          SaveClockSettings(); 
          MakeOut();
          DisplaySaved(); 
          delay(1000); 
          SetupMenuPos=CLOCK_MENU_EXIT_INDEX; 
          break;  
        case CLOCK_MENU_EXIT_INDEX: LoadClockSettings(); DrawMainMenu(); MakeOut(); return; 
          break;*/
      }
      GetDDSCoreClock();
    }
    
    if (XO_Freq_Index > (XO_QTY-1)) XO_Freq_Index = 0;
    if (TCXO_Freq_Index > (TCXO_QTY-1)) TCXO_Freq_Index = 0;

    if (XO_Freq_Index < 0) XO_Freq_Index = XO_QTY-1;
    if (TCXO_Freq_Index < 0) TCXO_Freq_Index = TCXO_QTY-1;
      
    if (ClockSourceIndex == CLOCK_SOURCE_XO_INDEX) Ref_Clk=XO_Values[XO_Freq_Index]; // Это стоит заменить на вызов функции
    if (ClockSourceIndex == CLOCK_SOURCE_TCXO_INDEX) Ref_Clk=Ref_Clk=TCXO_Values[TCXO_Freq_Index]; // и это
    if (ClockSourceIndex == CLOCK_SOURCE_EXT_OSC_INDEX) Ref_Clk=EXT_OSC_Freq; // и это тоже

    DisplayClockSetupMenu();
    incrValue=0;
    decrValue=0;
  }
}

void SaveClockSettings()
{
  Serial.println(F("SaveClockSettings"));

  GetDDSCoreClock();

  EEPROM.put(CLOCK_SOURCE_INDEX_ADR, ClockSourceIndex);
  EEPROM.put(CLOCK_XO_FREQ_INDEX_ADR, XO_Freq_Index);
  EEPROM.put(CLOCK_TCXO_FREQ_INDEX_ADR, TCXO_Freq_Index);
  EEPROM.put(SRC_DOUBLER_ADR, SRCDoublerIndex);
  EEPROM.put(EXT_OSC_FREQ_ADR, EXT_OSC_Freq);
  EEPROM.put(DDS_CORE_CLOCK_ADR, DDS_Core_Clock);
  EEPROM.put(N_ADR, N_Multiplier);

  EEPROM.put(CLOCK_SETTINGS_FLAG_ADR, 56);

  DDS_Init();
  DDS_Freq_Set(100000000, DDS_Core_Clock);
}

void LoadClockSettings()
{
  Serial.println(F("Loading Clock Settings"));
  if (EEPROM.read(CLOCK_SETTINGS_FLAG_ADR)!=56)
  {
    ClockSourceIndex=INIT_CLOCK_SOURCE_INDEX;
    XO_Freq_Index=INIT_CLOCK_XO_INDEX; 
    TCXO_Freq_Index=INIT_CLOCK_TCXO_INDEX; 
    SRCDoublerIndex=INIT_SRC_DOUBLER_INDEX; 
    EXT_OSC_Freq=INIT_EXT_OSC_FREQ;
    DDS_Core_Clock=INIT_DDS_CORE_FREQ;
    N_Multiplier=INIT_N; 
     
    #if DBG==1
    Serial.println(F("Set default clock settings"));
    Serial.print(F("ClockSourceIndex="));
    Serial.println(ClockSourceIndex);
    Serial.print(F("XO_Freq_Index="));
    Serial.println(XO_Freq_Index);
    Serial.print(F("TCXO_Freq_Index="));
    Serial.println(TCXO_Freq_Index);
    Serial.print(F("SRCDoublerIndex="));
    Serial.println(SRCDoublerIndex);
    Serial.print(F("EXT_OSC_Freq="));
    Serial.println(EXT_OSC_Freq);
    Serial.print(F("DDS_Core_Clock="));
    Serial.println(DDS_Core_Clock);
    Serial.print(F("N_Multiplier="));
    Serial.println(N_Multiplier);
    #endif
    
    SaveClockSettings();
  } else
  {
    EEPROM.get(CLOCK_SOURCE_INDEX_ADR, ClockSourceIndex);
    EEPROM.get(CLOCK_XO_FREQ_INDEX_ADR, XO_Freq_Index);
    EEPROM.get(CLOCK_TCXO_FREQ_INDEX_ADR, TCXO_Freq_Index);
    EEPROM.get(SRC_DOUBLER_ADR, SRCDoublerIndex);
    EEPROM.get(EXT_OSC_FREQ_ADR, EXT_OSC_Freq);
    EEPROM.get(DDS_CORE_CLOCK_ADR, DDS_Core_Clock);
    EEPROM.get(N_ADR, N_Multiplier);
    
    #if DBG==1
    Serial.println(F("Values from EEPROM"));
    Serial.print(F("ClockSourceIndex="));
    Serial.println(ClockSourceIndex);
    Serial.print(F("XO_Freq_Index="));
    Serial.println(XO_Freq_Index);
    Serial.print(F("TCXO_Freq_Index="));
    Serial.println(TCXO_Freq_Index);
    Serial.print(F("SRCDoublerIndex="));
    Serial.println(SRCDoublerIndex);
    Serial.print(F("EXT_OSC_Freq="));
    Serial.println(EXT_OSC_Freq);
    Serial.print(F("DDS_Core_Clock="));
    Serial.println(DDS_Core_Clock);
    Serial.print(F("N_Multiplier="));
    Serial.println(N_Multiplier);
    #endif
  }
}

uint32_t GetRefClk()
{
  if (XO_Freq_Index > (XO_QTY-1)) XO_Freq_Index = 0;
  if (TCXO_Freq_Index > (TCXO_QTY-1)) TCXO_Freq_Index = 0;

  if (XO_Freq_Index < 0) XO_Freq_Index = XO_QTY-1;
  if (TCXO_Freq_Index < 0) TCXO_Freq_Index = TCXO_QTY-1;
  //bool PLL=false;
  if (ClockSourceIndex!=CLOCK_SOURCE_EXT_OSC_INDEX) 
  {
    //PLL=true;
    if (ClockSourceIndex==CLOCK_SOURCE_XO_INDEX) Ref_Clk=XO_Values[XO_Freq_Index];
    if (ClockSourceIndex==CLOCK_SOURCE_TCXO_INDEX) Ref_Clk=TCXO_Values[TCXO_Freq_Index];
  } else
  {
    Ref_Clk=EXT_OSC_Freq;
  }

  //DDS_Init(PLL, DividerIndex, Ref_Clk);
  return Ref_Clk;
}

uint32_t GetDDSCoreClock()
{
  GetRefClk();
  if ((ClockSourceIndex == CLOCK_SOURCE_XO_INDEX)  || 
       (ClockSourceIndex == CLOCK_SOURCE_TCXO_INDEX)) 
       {
         DDS_Core_Clock = Ref_Clk * N_Multiplier * 2; //Всегда умножаем на 2 из-за зафиксированого удвоителя
         if (SRCDoublerIndex == 1) DDS_Core_Clock = DDS_Core_Clock * 2; //еще умножаем на 2, если включен doubler
       } else
       {
         DDS_Core_Clock = Ref_Clk;
       }
       /*Serial.print("Ref_Clk=");
       Serial.println(Ref_Clk);
       Serial.print("N_Multiplier=");
       Serial.println(N_Multiplier);
       Serial.print("DDS_Core_Clock=");
       Serial.println(DDS_Core_Clock);*/

  return DDS_Core_Clock;
}

uint8_t FindBestNMultiplier()
{
  uint8_t tmpN=0;
  GetRefClk();
  /*tmpN = BASE_DDS_CORE_CLOCK / Ref_Clk / 2;
  if (SRCDoublerIndex == 1) tmpN = tmpN /2;*/
  if (SRCDoublerIndex == 0) 
  {
    tmpN = BASE_DDS_CORE_CLOCK / Ref_Clk / 2;
  }
    else
    {
      tmpN = BASE_DDS_CORE_CLOCK / Ref_Clk / 4;
    }
  if (tmpN > N_MULTIPLIER_MAX_VALUE)
  {
    SRCDoublerIndex = 1; //Включаем даблер
    N_Multiplier = FindBestNMultiplier(); //да, это рекурсия
  } else
  {
    N_Multiplier = tmpN;
  }
  /*Serial.print("Best N_Multiplier = ");
  Serial.print(N_Multiplier);*/
  return N_Multiplier;
}
