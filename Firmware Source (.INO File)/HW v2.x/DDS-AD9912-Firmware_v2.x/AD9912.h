/* AD9912.h -  SPI interface library for Analog Devices AD9912 DDS Chip
   Written by Chiranth Siddappa
*/

#ifndef AD9912_h
#define AD9912_h

#include <Arduino.h>
#include <stdint.h>
#include <inttypes.h>
#include <SPI.h>

#define PWR_DOWN_PIN 8 

#define ad9912_PartID 0x1902
#define ad9912_read 0x1
#define ad9912_write 0x0

//DDS Register Addresses
//LSB
#define FTW0_1 0x01A6
#define FTW0_2 0x01A7
#define FTW0_3 0x01A8
#define FTW0_4 0x01A9
//MSB
#define FTW0_start_1 0x01AA
#define FTW0_start_2 0x01AB
#define DAC_fsc_1 0x040B
#define DAC_fsc_2 0x040C

//DDS Default values and maxes
#define DAC_fcs_default 0x01FF
#define DAC_fcs_max 0x03FF

/*DDS AD9912 I/O REGISTER MAP and Mask*/
#define Serial_Config_ADDR      0x0000 // HAD INSTRUCTION WORD
#define SDO_Active              0x99   // Value 0b10011001
#define LSB_First               0x5A   
#define Soft_Reset              0x3C 

#define Read_DATA               0x8000
#define Write_DATA              0x0000
#define DATA_1_Byte             0x0000 // Bytes to Transfer (Excluding the 1-Byte Instruction)
#define DATA_2_Byte             0x2000 // Bytes to Transfer (Excluding the 2-Byte Instruction)
#define DATA_3_Byte             0x4000 // Bytes to Transfer (Excluding the 3-Byte Instruction)
#define DATA_Streaming_Mode     0x6000 // Bytes to Transfer (Excluding the Byte Instruction)

#define Part_ID_L_ADDR           0x0002 // Part ID read-only
#define Part_ID_H_ADDR           0x0003 // Part ID read-only

#define Read_Buffer_Reg_ADDR     0x0004
#define Read_Buffer_Reg         0x01

/*Soft Reg Update, or can Hardware Reg Update via 60 pin IO_UPDATE AD9912*/
#define Serial_Options_ADDR      0x0005
#define Register_Update_Soft    0x01 
                                      
#define Power_Down_Reset_ADDR    0x0010
#define Disable_PD_HSTL_Driver  0x80 // 1 = HSTL driver powered down.
#define Enable_CMOS_Driver      0x40 // 1 = CMOS output driver on.
#define Enable_Output_Doubler   0x20 // 1 = Powers up output clock generator doubler
#define Disable_PD_SYSCLK_PLL   0x10 // 1 = system clock multiplier powered down.
#define Full_PD                 0x02
#define Digital_PD              0x01

#define DDS_Reset_L_ADDR         0x0012
#define DDS_Reset               0x01

#define DDS_Reset_H_ADDR         0x0013
#define PD_Fund_DDS             0x80
#define S_Div_2_Reset           0x08
#define S_Divider_Reset         0x02

#define N_Divider_ADDR          0x0020
#define N_Divider               14   // Exp1 _2xReference - OFF F = REF_IN_CLK * 2 * (N_Devider+2) Calc F=25000000*2*(18+2)= 1000MHz
                                    // Exp2 _2xReference - ON  F = REF_IN_CLK * 2 *2*(N_Devider+2) Calc F=40000000*2*2*(4+2)= 960MHz
                                    // _2xReference - OFF, N = (Fs/(REF_IN_CLK * 2))-2
//Devider PLL range of 0 to 31, logic automatically adds a bias of 2 to the value entered, extending the range to 33. C 

#define PLL_Parameters_ADDR     0x0022
#define VCO_Auto_Range          0x80 // Automatic VCO range selection
#define _2xReference            0x08 // Enables a frequency doubler prior to 
                                     // the SYSCLK PLL and can be useful in 
                                     // reducing jitter induced by the SYSCLK PLL. 
#define VCO_High_Range          0x04 // 1 = high range (900 MHz to 1000 MHz).
                                     // else 0 = low range (700 MHz to 810 MHz).
#define CP_Current_250uA        0x00 // PLL Charge pump current 00 = 250 uA.
#define CP_Current_375uA        0x01 // PLL Charge pump current 01 = 375 uA.
#define CP_Current_Off          0x02 // PLL Charge pump current 10 = off
#define CP_Current_125uA        0x03 // PLL Charge pump current 11 = 125 uA.

/* In MSB first mode, the serial control port internal address generator 
decrements for each data byte of the multibyte transfer cycle.
 When LSB first, the serial control port internal byte address generator 
increments for each byte of the multibyte transfer cycle*/
#define Freq_Tuning_Word0_ADDR   0x01A6 // LSB    [7:0]bits
#define Freq_Tuning_Word1_ADDR   0x01A7 //       [15:8]bits
#define Freq_Tuning_Word2_ADDR   0x01A8 //      [23:16]bits
#define Freq_Tuning_Word3_ADDR   0x01A9 //      [31:24]bits
#define Freq_Tuning_Word4_ADDR   0x01AA //      [39:32]bits
#define Freq_Tuning_Word5_ADDR   0x01AB // MSB  [47:40]bits

#define HSTL_Driver_ADDR        0x0200
#define OPOL_Polarity           0x10
#define HSTL_Doubler_Disabled   0x01
#define HSTL_Doubler_Enabled    0x02

#define CMOS_Driver_ADDR        0x0201
#define CMOS_Mux                0x01

//#define S_Divider_ADDR          0x0104 // CMOS output divider. Divide ratio = (1 − 65,536)-1.
#define S_Divider_LSB_ADDR      0x0104
#define S_Divider_MSB_ADDR      0x0105
#define CMOS_out_divider_ADDR   0x0106
#define S_divider_2             0x01   // Enable - S_divider_2 

#define DAC_Current_L_ADDR      0x040B // Rage 0...1023          
#define DAC_Current_H_ADDR      0x040C 

/* Spur A Harmonic 1 to Spur A Harmonic 15. Allows user to choose which 
   harmonic to eliminate. */
#define Spur_A_Harmonic_ADDR     0x0500 
#define HSR_A_Enable            0x80 // Harmonic Spur Reduction A enable.
#define _2xAmplitude_A_Gain     0x40 // Setting this bit doubles the gain of 
                                     // the cancelling circuit and also doubles 
                                     // the minimum step size.

#define Spur_A_Magnitude_ADDR    0x0501 // Linear multiplier Spur A mag 0..255
#define Spur_A_Phase0_ADDR       0x0503 // Linear offset Spur A phase. [0-7]bits
#define Spur_A_Phase1_ADDR       0x0504 // [8]bit, range 0...511

#define Spur_B_Harmonic_ADDR     0x0505 
#define HSR_B_Enable            0x80 // Harmonic Spur Reduction A enable.
#define _2xAmplitude_B_Gain     0x40 // Setting this bit doubles the gain of 
                                     // the cancelling circuit and also doubles 
                                     // the minimum step size.

#define Spur_B_Magnitude_ADDR    0x0506 // Linear multiplier Spur A mag 0..255
#define Spur_B_Phase0_ADDR       0x0508 // Linear offset Spur A phase. [0-7]bits
#define Spur_B_Phase1_ADDR       0x0509 // [8th]bit, range 0...511



void DDS_SPI_Init(void);
void DDS_UPDATE(void);
void SPI_Transmit(uint8_t *strBuffer, int nums);
void DDS_Init (void);
void DDS_Config(uint16_t Instr_Word, uint8_t Data);
void DDS_Freq_Set(uint64_t Freq, uint64_t Fs);
void DDS_FTW_Send(uint64_t *BUFF_DDS);
uint64_t DDS_Freq_To_FTW(uint64_t Freq, uint64_t Fs);
void DDS_Current(uint16_t DAC_Current);
void DDS_SPKILL_CH1();

extern uint8_t Power_Down_Reset_Register_Value;
extern uint8_t PLL_Parameters_Register_Value;
extern uint8_t HSTL_Driver_Register_Value;
extern uint8_t CMOS_Driver_Register_Value;

struct uint128_t {
  uint64_t hi;
  uint64_t lo;
};

uint128_t mul64x64(uint64_t a, uint64_t b);
uint64_t div128by32(uint128_t x, uint32_t d);


/*
class AD9912 {
 public:
  void init(uint8_t SPICS, uint8_t SPISCK, uint8_t SPIMOSI, uint8_t SPIMISO, uint8_t IO_update, uint32_t clkFreq, uint64_t RDAC_REF);
  uint16_t read_PartID();
  uint64_t instruction(short command, uint16_t address, char bytes, uint64_t data);
  uint16_t DAC_read();
  void DAC_write(uint16_t DAC_val);
  void PLL_Config(uint8_t Enabled=1);
  uint64_t FTW_read();
  void FTW_write(uint64_t FTW);
  void setFrequency(uint32_t frequency);
  uint32_t getFrequency();
  uint32_t fDDS();
  void updateClkFreq(uint64_t clkFreq);
  float IDAC_REF();
  void setCurrent(float current);
  float getCurrent();
  float IDAC_FS();
 private:
  // global type variables
  uint8_t _SPISCK;
  uint8_t _SPIMISO;
  uint8_t _SPIMOSI;
  uint8_t _SPICS;
  uint8_t _IO_update;
  uint32_t _fs;
  uint64_t _RDAC_REF;
};
*/

#endif
