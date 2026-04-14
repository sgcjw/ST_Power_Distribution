/*
 * INA228.c
 *
 *  Created on: Dec 30, 2020
 *  Updated on: Jan 29, 2022
 *       Author: Piotr Smolen <komuch@gmail.com>
 *       Updated by: Brandon Thibeaux <github: thibeaux>
 */

#include "main.h"
#include "INA228.h"


/*
 * @brief:		Read a register from the IN228 sensor.
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina228)
 * @param:		register address in hexadecimal
 * @retval:		24 bit/16 bit unsigned integer that represents the register's contents.
 */

uint16_t Read16(INA228_t *ina228, uint8_t Register)
{
	uint8_t Value[2];

	HAL_I2C_Mem_Read(ina228->ina228_i2c,
					 (ina228->Address << 1),
					 Register,
					 I2C_MEMADD_SIZE_8BIT,
					 Value,
					 2,
					 1000);

	return ((uint16_t)Value[0] << 8) |
		   (uint16_t)Value[1];
}

uint32_t Read24(INA228_t *ina228, uint8_t Register)
{
    uint8_t Value[3];

    HAL_I2C_Mem_Read(ina228->ina228_i2c,
                     (ina228->Address << 1),
                     Register,
                     I2C_MEMADD_SIZE_8BIT,
                     Value,
                     3,
                     1000);

    return ((uint32_t)Value[0] << 16) |
           ((uint32_t)Value[1] << 8)  |
           (uint32_t)Value[2];
}

/*
 * @brief:		Write to a register on the IN228 sensor.
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina228)
 * @param:		Register address in hexadecimal
 * @param:		16 bit integer in hexadecimal that is the value you want to write to the register.
 * @retval:		HAL_StatusTypeDef, this will include an enum value representing
 * 				if the I2C transmission was successful or not.
 * 				typedef enum
				{
				  HAL_OK       = 0x00U,
				  HAL_ERROR    = 0x01U,
				  HAL_BUSY     = 0x02U,
				  HAL_TIMEOUT  = 0x03U
				} HAL_StatusTypeDef;
 */
HAL_StatusTypeDef Write16(INA228_t *ina228, uint8_t Register, uint16_t Value)
{
	uint8_t addr[2];
	addr[0] = (Value >> 8) & 0xff;  // upper byte
	addr[1] = (Value >> 0) & 0xff; // lower byte
	return HAL_I2C_Mem_Write(ina228->ina228_i2c, (ina228->Address<<1), Register, 1, (uint8_t*)addr, 2, 1000);
}

uint16_t INA228_Init(INA228_t *ina228, I2C_HandleTypeDef *i2c, uint8_t Address, float maxcurrent, float shunt, uint8_t bvct, uint8_t svct, uint8_t tct, uint16_t ppm)
{
	ina228->ina228_i2c = i2c;
	ina228->Address = Address;

	uint8_t ina228_isReady = HAL_I2C_IsDeviceReady(i2c, (Address<<1), 3, 2);
	if(ina228_isReady == HAL_OK)
	{
		// just to initialize our state machine.
		//Feel free to change this if you want. This function should be called in your main function to be polled.
		INA228_Reset(ina228);
		INA228_setCalibration(ina228, maxcurrent, shunt); // default calibration 30A, 1mOhm shunt
		INA228_setBusVoltageConversionTime(ina228, bvct); // set bus voltage conversion time to bvct
		INA228_setShuntVoltageConversionTime(ina228, svct); // set shunt voltage conversion time to svct
		INA228_setTemperatureConversionTime(ina228, tct); // set temperature conversion time to tct
		INA228_setTemperatureCompensation(ina228, 1); // enable temperature compensation
		INA228_setShuntTemperatureCoefficent(ina228, ppm); // set shunt temperature coefficient to ppm
		INA228_setLatch(ina228, true); // set alert latch to false (latch until cleared)
		INA228_setBusOvervoltageTH(ina228, 28); // set bus overvoltage detection threshold to 28V
		INA228_setBusUndervoltageTH(ina228, 10); // set bus undervoltage threshold to 10V
		INA228_setShuntOvervoltageTH(ina228, 4, shunt); // set overcurrent threshold to 5A
		INA228_setTemperatureOverLimitTH(ina228, 80); // set temperature over limit threshold to 80 degrees Celsius
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 * @brief: 		This function will read the bus voltage level.
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina228)
 * @retval:		Returns voltage level in mili-volts
 */
float INA228_ReadBusVoltage(INA228_t *ina228)
{
	//  always positive, remove reserved bits.
  	uint32_t value = Read24(ina228, INA228_BUS_VOLTAGE) >> 4;
  	float bus_LSB = 195.3125e-3;  //  195.3125 uV
  	float voltage = value * bus_LSB;
  	return voltage;
}

/*
 *  @brief:	  	Gets the raw current value (24-bit signed integer, so +-8388607)
 *  @param:		Pointer to the device object that was made from the struct. EX:  (&ina228)
 *  @retval:	The raw current reading
 */
uint32_t INA228_ReadCurrent_raw(INA228_t *ina228)
{
	uint32_t result = Read24(ina228, INA228_CURRENT) >> 4; // shift right by 4 bits to remove reserved bits

	return (result);
}

/*
 * @brief:  	Gets the current value in mA, taking into account the
 *          	config settings and current LSB
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina228)
 * @return: 	The current reading convereted to milliamps
 */
float INA228_ReadCurrent(INA228_t *ina228, float maxCurrent)
{
	uint32_t result = INA228_ReadCurrent_raw(ina228);

	// Calculate current_LSB in mA based on maxCurrent
    float current_LSB = maxCurrent * 1.9073486328125e-3;  //  pow(2, -19) *1000 to convert to mA;

	return (result * current_LSB); // current is the raw current times the current_LSB
}

/*
 * @brief: 		This function will read the shunt voltage level.
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina228)
 * @retval:		Returns voltage level in mili-volts. This value represents the difference
 * 				between the voltage of the power supply and the bus voltage after the shunt
 * 				resistor.
 */
float INA228_ReadShuntVoltage(INA228_t *ina228)
{
	uint32_t result = Read24(ina228, INA228_SHUNT_VOLTAGE) >> 4; // shift right by 4 bits to remove reserved bits

	return (result * 312.5e-6); // shunt voltage in mV is the register value times the shunt_LSB (312.5nV)
}
/*
 * @brief: 	This reads the power register then multiplies it by the power multiplier.
 * 			Power multiplier is initialize in the calibration function.
 * @param:	Pointer to the device object that was made from the struct. EX:  (&ina228)
 * @retval:	Returns power level in mili-watts
 */
uint32_t INA228_ReadPower(INA228_t *ina228)
{
	uint32_t result = Read24(ina228, INA228_POWER );
	result = result * ina228_powerMultiplier_mW; // power is the power register times the power_LSB (power multiplier)
	return (result);
}

float INA228_getTemperature(INA228_t *ina228)
{
  uint16_t value = Read16(ina228, INA228_TEMPERATURE);
  float LSB = 7.8125;  //  milli degree Celsius
  return value * LSB;
}

uint16_t INA228_getDieID(INA228_t *ina228){
	return (Read24(ina228, INA228_DEVICE_ID)>> 4) & 0x0FFF;
}

/*
 * @brief: get Delta time in mili-seconds which is the difference between the last time you called this function and now
 */
int lastTime,deltaTime,now;
int INA228_GetDeltaTime_ms()
{
	  int now = HAL_GetTick();
	  deltaTime = now -lastTime;
	  lastTime = now;

	  return deltaTime;
}

void INA228_Reset(INA228_t *ina228)
{
	Write16(ina228, INA228_CONFIG, INA228_CFG_RST);
	HAL_Delay(1);
}

//set configuration
void INA228_setCalibration(INA228_t *ina228, float maxCurrent, float shunt)
{
	// set calibration register
    float current_LSB = maxCurrent * 1.9073486328125e-6;  //  pow(2, -19);

    //  PAGE 31 (8.1.2)
    float shunt_cal = 13107.2e6 * current_LSB * shunt;
    //  shunt_cal must be written to its REGISTER.
    Write16(ina228, INA228_SHUNT_CAL, (uint16_t)shunt_cal);
}

void INA228_setBusVoltageConversionTime(INA228_t *ina228, uint8_t bvct)
{
  uint16_t value = Read16(ina228, INA228_ADC_CONFIG);
  value &= ~INA228_ADC_VBUSCT;
  value |= (bvct << 9);
  Write16(ina228, INA228_ADC_CONFIG, value);
}

void INA228_setShuntVoltageConversionTime(INA228_t *ina228, uint8_t svct)
{
  uint16_t value = Read16(ina228, INA228_ADC_CONFIG);
  value &= ~INA228_ADC_VSHCT;
  value |= (svct << 6);
  Write16(ina228, INA228_ADC_CONFIG, value);
}

void INA228_setTemperatureConversionTime(INA228_t *ina228, uint8_t tct)
{
  uint16_t value = Read16(ina228, INA228_ADC_CONFIG);
  value &= ~INA228_ADC_VTCT;
  value |= (tct << 3);
  Write16(ina228, INA228_ADC_CONFIG, value);
}

void INA228_setTemperatureCompensation(INA228_t *ina228, bool on)
{
  uint16_t value = Read16(ina228, INA228_CONFIG);
  if (on) value |= INA228_CFG_TEMPCOMP;
  else    value &= ~INA228_CFG_TEMPCOMP;
  Write16(ina228, INA228_CONFIG, value);
}

void INA228_setShuntTemperatureCoefficent(INA228_t *ina228, uint16_t ppm)
{
  Write16(ina228, INA228_SHUNT_TEMP_CO, ppm);
}

////////////////////////////////////////////////////////
//
//  DIAGNOSE ALERT REGISTER 11
//
////////////////////////////////////////////////////////

void INA228_setDiagnoseAlert(INA228_t *ina228, uint16_t flags)
{
  Write16(ina228, INA228_DIAG_ALERT, flags);
}

//  INA228.h has an enum for the bit fields.
void INA228_setDiagnoseAlertBit(INA228_t *ina228, uint8_t bit)
{
  uint16_t value = Read16(ina228, INA228_DIAG_ALERT);
  uint16_t mask = (1 << bit);
  //  only write new value if bit not set
  if ((value & mask) == 0)
  {
    value |= mask;
    Write16(ina228, INA228_DIAG_ALERT, value);
  }
}

uint16_t INA228_getDiagnoseAlert(INA228_t *ina228)
{
	uint16_t value = Read16(ina228, INA228_DIAG_ALERT);
	return value;
}

void INA228_setLatch(INA228_t *ina228, bool latch)
{
	if (latch){
		INA228_setDiagnoseAlertBit(ina228, INA228_DIAG_ALERT_LATCH); // set alert latch bit to 1 to latch alerts
	}
}

uint16_t INA228_checkFault(INA228_t *ina228)
{
	uint16_t fault = INA228_getDiagnoseAlert(ina228);
	uint16_t number = 0;
	if (fault & (1 << INA228_DIAG_SHUNT_OVER_LIMIT)){
		number = 1;
	}
	if (fault & (1 << INA228_DIAG_BUS_OVER_LIMIT)){
		number = 2;
	}
	if (fault & (1 << INA228_DIAG_BUS_UNDER_LIMIT)){
		number = 3;
	}
	if (fault & (1 << INA228_DIAG_TEMP_OVER_LIMIT)){
		number = 4;
	}
	return number;
}

////////////////////////////////////////////////////////
//
//  THRESHOLD AND LIMIT REGISTERS 12-17
//
////////////////////////////////////////////////////////

void INA228_setShuntOvervoltageTH(INA228_t *ina228, float threshold, float shunt)
{
  //  Conversion Factor: 5 μV/LSB when ADCRANGE = 0
  //  1.25 μV/LSB when ADCRANGE = 1.
  float LSB = 5.0e-6;
  uint16_t TH = threshold * shunt / LSB;
  Write16(ina228, INA228_SOVL, TH);
}

void INA228_setBusOvervoltageTH(INA228_t *ina228, float threshold)
{
  float LSB = 3.125e-3;  //  3.125 mV/LSB.
  uint16_t TH = threshold / LSB;
  Write16(ina228, INA228_BOVL, TH);
}

void INA228_setBusUndervoltageTH(INA228_t *ina228, float threshold)
{
  float LSB = 3.125e-3;  //  3.125 mV/LSB.
  uint16_t TH = threshold / LSB;
  Write16(ina228, INA228_BUVL, TH);
}


void INA228_setTemperatureOverLimitTH(INA228_t *ina228, float threshold)
{
  float LSB = 7.8125e-3;  //  milli degrees Celsius
  uint16_t TH = threshold / LSB;
  Write16(ina228, INA228_TEMP_LIMIT, TH);
}
