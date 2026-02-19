/*
 * INA228.h
 *
 *  Created on: Dec 30, 2020
 *  Updated on: Jan 29, 2022
 *      Author: Piotr Smolen <komuch@gmail.com>
 *     	Updated by: Brandon Thibeaux <github: thibeaux>
 */

#ifndef INC_INA228_H_
#define INC_INA228_H_

#include <stdbool.h>
#include <stdint.h>

//
//	Registers
//
//      REGISTERS                   ADDRESS    BITS  RW
#define INA228_CONFIG               0x00    //  16   RW
#define INA228_ADC_CONFIG           0x01    //  16   RW
#define INA228_SHUNT_CAL            0x02    //  16   RW
#define INA228_SHUNT_TEMP_CO        0x03    //  16   RW
#define INA228_SHUNT_VOLTAGE        0x04    //  24   R-
#define INA228_BUS_VOLTAGE          0x05    //  24   R-
#define INA228_TEMPERATURE          0x06    //  16   R-
#define INA228_CURRENT              0x07    //  24   R-
#define INA228_POWER                0x08    //  24   R-
#define INA228_ENERGY               0x09    //  40   R-
#define INA228_CHARGE               0x0A    //  40   R-
#define INA228_DIAG_ALERT           0x0B    //  16   RW
#define INA228_SOVL                 0x0C    //  16   RW
#define INA228_SUVL                 0x0D    //  16   RW
#define INA228_BOVL                 0x0E    //  16   RW
#define INA228_BUVL                 0x0F    //  16   RW
#define INA228_TEMP_LIMIT           0x10    //  16   RW
#define INA228_POWER_LIMIT          0x11    //  16   RW
#define INA228_MANUFACTURER         0x3E    //  16   R-
#define INA228_DEVICE_ID            0x3F    //  16   R-


//  CONFIG MASKS (register 0)
#define INA228_CFG_RST              0x8000
#define INA228_CFG_RSTACC           0x4000
#define INA228_CFG_CONVDLY          0x3FC0
#define INA228_CFG_TEMPCOMP         0x0020
#define INA228_CFG_ADCRANGE         0x0010
#define INA228_CFG_RESERVED         0x000F  //  all unused bits


//  ADC MASKS (register 1)
#define INA228_ADC_MODE             0xF000
#define INA228_ADC_VBUSCT           0x0E00
#define INA228_ADC_VSHCT            0x01C0
#define INA228_ADC_VTCT             0x0038
#define INA228_ADC_AVG              0x0007

//  for setMode() and getMode()
enum ina228_mode_enum {
  INA228_MODE_SHUTDOWN            = 0x00,
  INA228_MODE_TRIG_BUS            = 0x01,
  INA228_MODE_TRIG_SHUNT          = 0x02,
  INA228_MODE_TRIG_BUS_SHUNT      = 0x03,
  INA228_MODE_TRIG_TEMP           = 0x04,
  INA228_MODE_TRIG_TEMP_BUS       = 0x05,
  INA228_MODE_TRIG_TEMP_SHUNT     = 0x06,
  INA228_MODE_TRIG_TEMP_BUS_SHUNT = 0x07,

  INA228_MODE_SHUTDOWN2           = 0x08,
  INA228_MODE_CONT_BUS            = 0x09,
  INA228_MODE_CONT_SHUNT          = 0x0A,
  INA228_MODE_CONT_BUS_SHUNT      = 0x0B,
  INA228_MODE_CONT_TEMP           = 0x0C,
  INA228_MODE_CONT_TEMP_BUS       = 0x0D,
  INA228_MODE_CONT_TEMP_SHUNT     = 0x0E,
  INA228_MODE_CONT_TEMP_BUS_SHUNT = 0x0F
};


//  for setAverage() and getAverage()
enum ina228_average_enum {
    INA228_1_SAMPLE     = 0,
    INA228_4_SAMPLES    = 1,
    INA228_16_SAMPLES   = 2,
    INA228_64_SAMPLES   = 3,
    INA228_128_SAMPLES  = 4,
    INA228_256_SAMPLES  = 5,
    INA228_512_SAMPLES  = 6,
    INA228_1024_SAMPLES = 7
};


//  for Bus, shunt and temperature conversion timing.
enum ina228_timing_enum {
    INA228_50_us   = 0,
    INA228_84_us   = 1,
    INA228_150_us  = 2,
    INA228_280_us  = 3,
    INA228_540_us  = 4,
    INA228_1052_us = 5,
    INA228_2074_us = 6,
    INA228_4120_us = 7
};


//  for diagnose/alert() bit fields.
//  TODO bit masks?
enum ina228_diag_enum {
  INA228_DIAG_MEMORY_STATUS      = 0,
  INA228_DIAG_CONVERT_COMPLETE   = 1,
  INA228_DIAG_POWER_OVER_LIMIT   = 2,
  INA228_DIAG_BUS_UNDER_LIMIT    = 3,
  INA228_DIAG_BUS_OVER_LIMIT     = 4,
  INA228_DIAG_SHUNT_UNDER_LIMIT  = 5,
  INA228_DIAG_SHUNT_OVER_LIMIT   = 6,
  INA228_DIAG_TEMP_OVER_LIMIT    = 7,
  INA228_DIAG_RESERVED           = 8,
  INA228_DIAG_MATH_OVERFLOW      = 9,
  INA228_DIAG_CHARGE_OVERFLOW    = 10,
  INA228_DIAG_ENERGY_OVERFLOW    = 11,
  INA228_DIAG_ALERT_POLARITY     = 12,
  INA228_DIAG_SLOW_ALERT         = 13,
  INA228_DIAG_CONVERT_READY      = 14,
  INA228_DIAG_ALERT_LATCH        = 15
};

typedef struct
{
	I2C_HandleTypeDef 	*ina228_i2c;
	uint8_t				Address;
} INA228_t;

bool isFirst;
uint16_t ina228_calibrationValue;
int16_t ina228_currentDivider_mA;
int16_t ina228_powerMultiplier_mW;

uint16_t INA228_Init(INA228_t *ina228, I2C_HandleTypeDef *i2c, uint8_t Address, float maxcurrent, float shunt, uint8_t bvct, uint8_t svct, uint8_t tct, uint16_t ppm);

//
//  CORE FUNCTIONS + scale wrappers.
//
//       BUS VOLTAGE
float INA228_ReadBusVoltage(INA228_t *ina228);

//       SHUNT VOLTAGE
float INA228_ReadShuntVoltage(INA228_t *ina228);

//       SHUNT CURRENT
uint32_t INA228_ReadCurrent_raw(INA228_t *ina228);
float INA228_ReadCurrent(INA228_t *ina228, float maxCurrent);
//       POWER
uint32_t INA228_ReadPower(INA228_t *ina228);

//       TEMPERATURE
float   INA228_getTemperature(INA228_t *ina228);    //  Celsius

// //  the Energy and Charge functions are returning double as they have higher accuracy.
// //       ENERGY
// double   getEnergy();         //  Joule or watt second
// double   getJoule()           { return getEnergy(); };


// //       CHARGE
// double   getCharge();         //  Coulombs
// double   getCoulomb()         { return getCharge(); };

//       Timer
int      INA228_GetDeltaTime_ms();

//
//  CONFIG REGISTER 0
//  read datasheet for details, section 7.6.1.1, page 22
//
void     INA228_Reset(INA228_t *ina228);
//  value: 0 == normal operation,  1 = clear registers
// //   bool     resetEnergyCharge(uint8_t value);  better name for setAccumulation()??
// //  [[deprecated("Use ")]]  ??
// bool     setAccumulation(uint8_t value);  //  idem
// bool     getAccumulation();
// //  Conversion delay in 0..255 steps of 2 ms
// void     setConversionDelay(uint8_t steps);
// uint8_t  getConversionDelay();
// void     setTemperatureCompensation(bool on);
// bool     getTemperatureCompensation();
// //  flag = false => 164 mV, true => 41 mV
// bool     setADCRange(bool flag);
// bool     getADCRange();

// //
// //  CONFIG ADC REGISTER 1
// //  read datasheet for details, section 7.6.1.2, page 22++
void INA228_setBusVoltageConversionTime(INA228_t *ina228, uint8_t bvct);
void INA228_setShuntVoltageConversionTime(INA228_t *ina228, uint8_t svct);
void INA228_setTemperatureConversionTime(INA228_t *ina228, uint8_t tct);

// //
// //  SHUNT CALIBRATION REGISTER 2
// //  read datasheet for details. use with care.
// //  maxCurrent <= 204, (in fact no limit)
// //  shunt >= 0.0001.
// //  returns error code => 0 == OK;
// int      setMaxCurrentShunt(float maxCurrent, float shunt);
// float    getMaxCurrent();
// float    getShunt();
// float    getCurrentLSB();  //  <= 0.0 means not calibrated.

// //
// //  SHUNT TEMPERATURE COEFFICIENT REGISTER 3
// //  read datasheet for details, page 16.
// //  ppm = 0..16383.
// //bool     setShuntTemperatureCoefficent(uint16_t ppm = 0);
// bool     setShuntTemperatureCoefficent(uint16_t ppm);
// uint16_t getShuntTemperatureCoefficent();


// //
// //  DIAGNOSE ALERT REGISTER 11  (0x0B)
void INA228_setDiagnoseAlertBit(INA228_t *ina228, uint8_t bit);
uint16_t INA228_getDiagnoseAlert(INA228_t *ina228);
void INA228_setLatch(INA228_t *ina228, bool latch);
int INA228_checkFault(INA228_t *ina228);

// //
// //  THRESHOLD AND LIMIT REGISTERS 12-17
void INA228_setShuntOvervoltageTH(INA228_t *ina228, uint16_t threshold, float shunt);
void INA228_setBusOvervoltageTH(INA228_t *ina228, uint16_t threshold);
void INA228_setBusUndervoltageTH(INA228_t *ina228, uint16_t threshold);
void INA228_setTemperatureOverLimitTH(INA228_t *ina228, uint16_t threshold);


// //
// //  MANUFACTURER and ID REGISTER 3E and 3F
// //
// //                               typical value
// uint16_t getManufacturer();  //  0x5449 ("TI" in ASCII)
uint16_t INA228_getDieID();         //  0x0228
// uint16_t getRevision();      //  0x0001

// //
// //  ERROR HANDLING
// //
// int      getLastError();

void INA228_setCalibration(INA228_t *ina228, float maxCurrent, float shunt);
void INA228_setTemperatureCompensation(INA228_t *ina228, bool on);
void INA228_setShuntTemperatureCoefficent(INA228_t *ina228, uint16_t ppm);
uint16_t Read16(INA228_t *ina228, uint8_t Register);
HAL_StatusTypeDef Write16(INA228_t *ina228, uint8_t Register, uint16_t Value);

#endif /* INC_INA228_H_ */