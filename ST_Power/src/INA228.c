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
 * @retval:		16 bit unsigned integer that represents the register's contents.
 */
uint16_t Read16(INA228_t *ina228, uint8_t Register)
{
	uint8_t Value[2];

	HAL_I2C_Mem_Read(ina228->ina228_i2c, (ina228->Address<<1), Register, 1, Value, 2, 1000);
	return ((Value[0] << 8) | Value[1]);
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

uint16_t INA228_Init(INA228_t *ina228, I2C_HandleTypeDef *i2c, uint8_t Address, float maxcurrent, float shunt)
{
	ina228->ina228_i2c = i2c;
	ina228->Address = Address;

	uint8_t ina228_isReady = HAL_I2C_IsDeviceReady(i2c, (Address<<1), 3, 2);
	if(ina228_isReady == HAL_OK)
	{
		// just to initialize our state machine.
		//Feel free to change this if you want. This function should be called in your main function to be polled.
		INA228_Reset(ina228);
		INA228_setCalibration(ina228, maxcurrent, shunt); // default calibration 30A, 4mOhm shunt
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
uint16_t INA228_ReadBusVoltage(INA228_t *ina228)
{
	uint16_t result = Read16(ina228, INA228_BUS_VOLTAGE);

	return (result * 195.3125e-3); // each bit is 195.3125uV

}

/*
 *  @brief:	  	Gets the raw current value (16-bit signed integer, so +-32767)
 *  @param:		Pointer to the device object that was made from the struct. EX:  (&ina228)
 *  @retval:	The raw current reading
 */
int16_t INA228_ReadCurrent_raw(INA228_t *ina228)
{
	int16_t result = Read16(ina228, INA228_CURRENT);

	return (result);
}

/*
 * @brief:  	Gets the current value in mA, taking into account the
 *          	config settings and current LSB
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina228)
 * @return: 	The current reading convereted to milliamps
 */
int16_t INA228_ReadCurrent(INA228_t *ina228, float maxCurrent)
{
	int16_t result = INA228_ReadCurrent_raw(ina228);

	// Calculate current_LSB based on maxCurrent
    float current_LSB = maxCurrent * 1.9073486328125e-6;  //  pow(2, -19);

	return (result * current_LSB); // current is the raw current times the current_LSB
}

/*
 * @brief: 		This function will read the shunt voltage level.
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina228)
 * @retval:		Returns voltage level in mili-volts. This value represents the difference
 * 				between the voltage of the power supply and the bus voltage after the shunt
 * 				resistor.
 */
uint16_t INA228_ReadShuntVoltage(INA228_t *ina228)
{
	uint16_t result = Read16(ina228, INA228_SHUNT_VOLTAGE);

	return (result * 0.01 );
}
/*
 * @brief: 	This reads the power register then multiplies it by the power multiplier.
 * 			Power multiplier is initialize in the calibration function.
 * @param:	Pointer to the device object that was made from the struct. EX:  (&ina228)
 * @retval:	Returns power level in mili-watts
 */
uint16_t INA228_ReadPower(INA228_t *ina228)
{
	uint16_t result = Read16(ina228, INA228_POWER );
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
	return (Read16(ina228, INA228_DEVICE_ID)>> 4) & 0x0FFF;
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
