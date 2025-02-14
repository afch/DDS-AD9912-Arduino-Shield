/* AD9912.h -  SPI interface library for Analog Devices AD9912 DDS Chip
   Written by Chiranth Siddappa
*/


#include "AD9912.h"
#include "Arduino.h"
#include <stdint.h>
#include <inttypes.h>
#include <SPI.h>
#include "menuclk.h"

#define SPISCK 52
#define SPIMISO 0
#define SPIMOSI 51

#define SPI_CS 5
#define IO_update 6
#define RESET_PIN 7

// #define S1_PIN 4
// #define S4_PIN 9 

uint8_t strBuffer[8];
uint32_t DAC_Current;
uint32_t DAC_Current_L;
uint32_t DAC_Current_H;
uint64_t ADDR_FTW;

uint8_t Power_Down_Reset_Register_Value = 0xC0;
uint8_t PLL_Parameters_Register_Value = 0x04;
uint8_t HSTL_Driver_Register_Value = 0x05;
uint8_t CMOS_Driver_Register_Value = 0;


/*****************************************************************************************
   DDS Init AD9912
*****************************************************************************************/ 
void DDS_Init (void)
{
  pinMode(RESET_PIN, OUTPUT);
  pinMode(PWR_DOWN_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
  digitalWrite(PWR_DOWN_PIN, LOW);

  // pinMode(S1_PIN, OUTPUT);
  // digitalWrite(S1_PIN, LOW);
  // pinMode(S4_PIN, OUTPUT);
  // digitalWrite(S4_PIN, LOW);

  DDS_SPI_Init(); // SPI Init

  DDS_Config(Serial_Config_ADDR, SDO_Active);  // Main Config, MSB
  DDS_Config(DDS_Reset_L_ADDR, DDS_Reset);     // Reset DDS
  DDS_Config(Serial_Options_ADDR, Register_Update_Soft); // IO_UPDETE via Soft   
  DDS_Config(Serial_Config_ADDR, SDO_Active);  // Main ReConfig after Reset DSS

  // Включаем PLL
  if (ClockSourceIndex != CLOCK_SOURCE_EXT_OSC_INDEX) //Если нужно включить PLL, тоесть тактируется не от внешнего источника
  {
    Power_Down_Reset_Register_Value &= ~Disable_PD_SYSCLK_PLL; // включить PLL
  } else 
  {
    Power_Down_Reset_Register_Value |= Disable_PD_SYSCLK_PLL; // выключить PLL
  }
  DDS_Config(Power_Down_Reset_ADDR, Power_Down_Reset_Register_Value);

  // настраиваем PLL
  if (SRCDoublerIndex == 1) // включен даблер для источника PLL
  {
    PLL_Parameters_Register_Value |= _2xReference;
  } else
  {
    PLL_Parameters_Register_Value &= ~_2xReference;
  }
  PLL_Parameters_Register_Value |= VCO_Auto_Range; // Включаем авто range
  PLL_Parameters_Register_Value |= CP_Current_375uA; // масимальный ток

  DDS_Config(PLL_Parameters_ADDR, PLL_Parameters_Register_Value);
  DDS_Config(N_Divider_ADDR, N_Multiplier-2);

  //*Prepare 16-Bit Instruction Word to send FWT, to save time when sending data*
  ADDR_FTW = Freq_Tuning_Word5_ADDR | DATA_Streaming_Mode; // Write+Command(W1W0)+Addr

  //DDS_Current(250);

 /* // 13.06.2022  
  DDS_Config(Power_Down_Reset_ADDR, Disable_PD_SYSCLK_PLL | Disable_PD_HSTL_Driver); //external clock 0x10 - external clock, PLL OFF (By-PASS)
// DDS_Config(Power_Down_Reset_ADDR, Disable_PD_HSTL_Driver); //Disable_PD_HSTL_Driver | Enable_CMOS_Driver); // PLL ON, 
// DDS_Config(Power_Down_Reset_ADDR, Disable_PD_HSTL_Driver | Enable_CMOS_Driver); // PLL ON + CMOS Out
// DDS_Config(Power_Down_Reset_ADDR, 0); // LL ON + + HSTL

 //*** HSTL - ON, Doubler - OFF!!!! *******************************
//  DDS_Config(Power_Down_Reset_ADDR, 0); // PLL + clock + HSTL
//  DDS_Config(HSTL_Driver_ADDR, 0x05); // HSTL Doubler Worcking only 1.8V (pin 37)!!!!

// *** HSTL - ON, Doubler - ON!!!! HSTL_Doubler_Enable ?????*******************************
  DDS_Config(Power_Down_Reset_ADDR, Enable_Output_Doubler); // PLL + clock + HSTL
  DDS_Config(HSTL_Driver_ADDR, B00000110); // HSTL Doubler Worcking only 1.8V (pin 37)!!!!

  DDS_Config(N_Divider_ADDR, N_Divider); // РАБОЧИЙ ВАРИАНТ 12,05,2022
  //DDS_Config(PLL_Parameters_ADDR, B10001101);//VCO_High_Range | CP_Current_250uA); //РАБОЧИЙ ВАРИАНТ 12,05,2022  B10001001 с этим значением лучше всего
  DDS_Config(PLL_Parameters_ADDR, (VCO_High_Range | CP_Current_375uA)); // с выключенным Даблером меньше суб-гармоник!!!! но больше шум!!!

 //*** Set output current *******************************
  DDS_Current(250);
   
  /*Prepare 16-Bit Instruction Word to send FWT, to save time when sending data*
  ADDR_FTW = Freq_Tuning_Word5_ADDR | DATA_Streaming_Mode; // Write+Command(W1W0)+Addr

  /**Работа с CMOS*
  DDS_Config(CMOS_Driver_ADDR, CMOS_Mux); // CMOS ENABLE
  DDS_Config(CMOS_out_divider_ADDR, ~S_divider_2); // Disable S_divider/2 0&(~BIT_DEFINE)
  DDS_Config(S_Divider_ADDR, 10-1); // CMOS Divider N-1
  
 //***Работа с HSTL*******
 // DDS_Config(Power_Down_Reset_ADDR, 0); //Enable PD_HSTL_Driver //Enable_Output_Doubler

 // DDS_Config(HSTL_Driver_ADDR, HSTL_Doubler_Enable); // HSTL Doubler Worcking only 1.8V (pon 37)!!!!
 */ //13.06.2022
}

/******************************************************************************
 * Init SPI, 8bit, Master
 * MODE 3, MSB, 
******************************************************************************/
void DDS_SPI_Init(void)
{
    //Initialize other DDS pins
  pinMode(IO_update, OUTPUT);
  pinMode(SPISCK, OUTPUT);
  //  pinMode(SPIMISO, INPUT);
  pinMode(SPIMOSI, OUTPUT);
  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);
  digitalWrite(SPISCK, LOW);
  digitalWrite(SPIMOSI, LOW);
  
  SPI.begin(); //
  SPI.setDataMode (SPI_MODE0); 
  //SPI.setClockDivider(SPI_CLOCK_DIV8); //16MHZ/8=2MHZ
  SPI.setClockDivider(SPI_CLOCK_DIV2); //16MHZ/2=8MHZ
  SPI.setBitOrder(MSBFIRST);
}

/*****************************************************************************************
   Update - data updates from memory
*****************************************************************************************/ 
void DDS_UPDATE(void)
{
  // Required - data updates from memory
  digitalWrite(IO_update, HIGH);
  //delay(10);
  //delayMicroseconds(50);
  digitalWrite(IO_update, LOW);
  //delay(10);
  //
}


/*****************************************************************************************
   SPI TRANSMIT strBuffer array uint_8t, nums in array
*****************************************************************************************/ 
void SPI_Transmit(uint8_t *strBuffer, int nums)
{
  digitalWrite(SPI_CS, LOW); // CS = 0
  for (int i=0;i<nums; i++)
  {
    SPI.transfer(*(strBuffer+i));
  }
  digitalWrite(SPI_CS, HIGH); // CS = 1
}


/******************************************************************************
  SPI 24 bit
  * 16bit Instruction Word
  * 8bit Data
******************************************************************************/
void DDS_Config(uint16_t Instr_Word, uint8_t Data)
 {
  Instr_Word = Write_DATA | DATA_1_Byte | Instr_Word; 
  
  digitalWrite(SPI_CS, LOW); // CS = 0     
  SPI.transfer((Instr_Word>>8) & 0x00FF); // send most significant byte  
  SPI.transfer(Instr_Word & 0x00FF); // send most last byte 
  SPI.transfer(Data); // send data    
  digitalWrite(SPI_CS, HIGH); // CS = 1
 }

/*****222222222222222222222222222222222222222222222222222222222*************
  SPI 32 bit
  * 16bit Instruction Word
  * 10bit Data 0...1023 Current 0...31,25mA
******2222222222222222222222222222222222222222222222222**********************/
void DDS_Current(uint16_t DAC_Current)
{  
  digitalWrite(SPI_CS, LOW); // CS = 0  Start transmission
  SPI.transfer16(DAC_Current_H_ADDR);  // send instruction
  SPI.transfer(DAC_Current>>8); // Set current DAC
  digitalWrite(SPI_CS, HIGH); // CS = 1

  digitalWrite(SPI_CS, LOW); // CS = 0  Start transmission
  SPI.transfer16(DAC_Current_L_ADDR);  // send instruction
  SPI.transfer(DAC_Current); // Set current DAC
  digitalWrite(SPI_CS, HIGH); // CS = 1
  
  /*DDS_Config(DAC_Current_H_ADDR, uint8_t((DAC_Current>>8) & 0x00FF));
  DDS_Config(DAC_Current_L_ADDR, uint8_t(DAC_Current & 0x00FF));*/
  
  //DDS_Config(DAC_Current_H_ADDR, 0);
  //DDS_Config(DAC_Current_L_ADDR, 0);
  DDS_UPDATE();
}


/******************************************************************************
  Frequency Set to DDS
   * Input Freq in Hz
   * Input Fs - Core Clock in Hz
******************************************************************************/
void DDS_Freq_Set(uint64_t Freq, uint64_t Fs)
{
  uint64_t FTW;
  FTW=DDS_Freq_To_FTW(Freq, Fs);
  DDS_FTW_Send(&FTW);
}


/*************************************************************************
   (SPI) Frequncy Word Write on DDS AD9912
   * ADDR_FTW Addr - 16bit comes from a global variable, in order not to waste                     
   * time each time you send
   * BUFF_DDS Write freq DDS - 48bit 0x000000000000 to 0x666666666666 equ 0...400MHz @ 1GHz Fs
   * Total transmission 64 bits                 
***************************************************************************/
void DDS_FTW_Send (uint64_t *BUFF_DDS)
  {
  digitalWrite(SPI_CS, LOW); // CS = 0  Start transmission
   
  SPI.transfer16((uint16_t)ADDR_FTW); // send Instruction Word 16bit  
  SPI.transfer16(*(((uint16_t*)BUFF_DDS)+ 2)); // send Frequncy Word H
  SPI.transfer16( *(((uint16_t*)BUFF_DDS)+ 1)); // send Frequncy Word  
  SPI.transfer16( *((uint16_t*)BUFF_DDS)); // send Frequncy Word L

  digitalWrite(SPI_CS, HIGH); // CS = 1
  DDS_UPDATE();
}

/******************************************************************************
  Frequency to FTW
   * Input Freq in Hz
   * Input Fs - Core Clock in Hz
******************************************************************************/
uint64_t DDS_Freq_To_FTW(uint64_t Freq, uint64_t Fs)
  {
    uint64_t FTW;
    FTW = (uint64_t) (281474976710656 * (Freq / (float) Fs)); //DDS_FTW_Send(&DDS_Freq_To_FTW());
    return FTW;
  }



void DDS_SPKILL_CH1()
{
  /* 1. Determine which offending harmonic spur to reduce and
      its amplitude. Enter that harmonic number into Bit 0 to
      Bit 3 of Register 0x0500/Register 0x0505.
    2. Turn off the fundamental by setting Bit 7 of Register 0x0013
      and enable the SpurKiller channel by setting Bit 7 of
      Register 0x0500/Register 0x0505.
    3. Adjust the amplitude of the SpurKiller channel so that it
      matches the amplitude of the offending spur.
    4. Turn the fundamental on by clearing Bit 7 of Register 0x0013.
    5. Adjust the phase of the SpurKiller channel so that
      maximum interference is achieved. */
 digitalWrite(SPI_CS, LOW); // CS = 0  Start transmission
 SPI.transfer16(0x0500); // Spurkiller ch1 addr
 SPI.transfer(0x83); //second harmonic

 SPI.transfer16(0x0013); // main ch1 addr
 SPI.transfer(0x80); //bit#7 enabled

 /*
  * set SPK CH1 amplitude 0x0501 addr (8bit max)
  */

 SPI.transfer16(0x0501); // amplitude Spurkiller ch1
 SPI.transfer(0xff); //amplitude 8 bit

 /*SPI.transfer16(0x0013); // main ch1 addr
 SPI.transfer(0x0); //bit#7 disabled*/
 digitalWrite(SPI_CS, HIGH); // CS = 1
 DDS_UPDATE();
}

/*
void AD9912::init(uint8_t SPICS, uint8_t SPISCK, uint8_t SPIMOSI, uint8_t SPIMISO, uint8_t IO_update, uint32_t fs, uint64_t RDAC_REF) {
  _SPICS = SPICS;
  _SPISCK = SPISCK;
  _SPIMOSI = SPIMOSI;
  _SPIMISO = SPIMISO;
  _IO_update = IO_update;
  _fs = fs;
  _RDAC_REF = RDAC_REF;
}

uint16_t AD9912::read_PartID() {
  uint16_t id;
  id = AD9912::instruction(0x1, 0x3, 2, 0x0);
  return id;
}

uint64_t AD9912::instruction(short command, uint16_t address, char bytes, uint64_t data) {
  uint16_t instruction = 0x0;
  uint64_t return_data = 0x0;
  short nthByte;
  uint32_t shiftAmount;
  if(bytes > 3)
    instruction |= 3 << 13;
  else if(bytes > 0)
    instruction |= (bytes - 1) << 13;
  else
    return 0;
  instruction |= command << 15;
  instruction |= address & 0x7FF;
  pinMode(_SPIMOSI, OUTPUT);
  digitalWrite(_SPICS, HIGH);
  digitalWrite(_SPICS, LOW);
  shiftOut(_SPIMOSI, _SPISCK, MSBFIRST, instruction >> 8);
  shiftOut(_SPIMOSI, _SPISCK, MSBFIRST, instruction);
  if(command & 0x1) {
    digitalWrite(_SPIMOSI, LOW);
    pinMode(_SPIMOSI, INPUT);
    for(nthByte = 0, shiftAmount = (bytes - 1)*8; nthByte < bytes; nthByte++, shiftAmount -= 8) 
      return_data |= (uint64_t) shiftIn(_SPIMOSI, _SPISCK, MSBFIRST) << shiftAmount;
  }
  else {
    for(nthByte = 0, shiftAmount = (bytes - 1)*8; nthByte < bytes; nthByte++, shiftAmount -= 8)
      shiftOut(_SPIMOSI, _SPISCK, MSBFIRST, data >> shiftAmount);
    return_data = 0x0;
  }
  digitalWrite(_SPICS, HIGH);
  pinMode(_SPIMOSI, OUTPUT);

 // digitalWrite(_IO_update, HIGH);
 // delay(100);
 // digitalWrite(_IO_update, LOW);
  return return_data;
}

uint16_t AD9912::DAC_read() {
  uint16_t data;
  data = AD9912::instruction(0x1, 0x40C, 2, 0x0);
  return data;
}

void AD9912::DAC_write(uint16_t DAC_val) {
  AD9912::instruction(0x0, 0x40C, 2, DAC_val);
}



/***PLL **/
/*
void AD9912::PLL_Config(uint8_t Enabled=1) {
  

  
  AD9912::instruction(0x0, Serial_Config_ADDR, 1, SDO_Active);  // Main Config, MSB
  AD9912::instruction(0x0, DDS_Reset_L_ADDR, 1, DDS_Reset);     // Reset DDS
  AD9912::instruction(0x0, Serial_Options_ADDR, 1, Register_Update_Soft); // IO_UPDETE via Soft   
  AD9912::instruction(0x0, Serial_Config_ADDR, 1, SDO_Active); // Main ReConfig after Reset DSS





  AD9912::instruction(0x0, Power_Down_Reset_ADDR, 1, 0x00);//Disable_PD_HSTL_Driver | Enable_CMOS_Driver);
  AD9912::instruction(0x0, N_Devider_ADDR, 1, 12); // N-divider, Bits[4:0]
  AD9912::instruction(0x0, PLL_Parameters_ADDR, 1, 0x80);///VCO_High_Range | CP_Current_375uA);


 
  AD9912::FTW_write((uint64_t) (281474976710656 * (10000000 / (float) _fs)));



  

  while (1);
  
}

*/
/*
uint64_t AD9912::FTW_read() {
  uint64_t FTW;
  FTW = AD9912::instruction(0x1, 0x1AB, 6, 0x0);
  return FTW;
}

void AD9912::FTW_write(uint64_t FTW) {
  AD9912::instruction(0x0, 0x1AB, 6, FTW);
}

void AD9912::updateClkFreq(uint64_t fs) {
  _fs = fs;
}

void AD9912::setFrequency(uint32_t fDDS) {
  uint64_t FTW;
  FTW = (uint64_t) (281474976710656 * (fDDS / (float) _fs));
  AD9912::FTW_write(FTW);
  digitalWrite(_IO_update, HIGH);
  delay(10);
  digitalWrite(_IO_update, LOW);
}

uint32_t AD9912::getFrequency() {
  uint64_t FTW = AD9912::FTW_read();
  uint32_t fDDS;
  fDDS = (uint32_t) ((FTW / (float) 281474976710656) * _fs);
  return fDDS;
}

uint32_t AD9912::fDDS() {
  return AD9912::getFrequency();
}

float AD9912::IDAC_REF() {
  return 12 / (10 * (float) _RDAC_REF);
}

float AD9912::getCurrent() {
  uint16_t FSC = AD9912::DAC_read();
  return (AD9912::IDAC_REF() * (73728 + 192 * FSC)) / 1024;
}

void AD9912::setCurrent(float current) {
  uint16_t FSC;
  FSC = (uint16_t) ((((1024 * current) / AD9912::IDAC_REF()) - 73728) / 192);
  AD9912::DAC_write(FSC);
}

*/
