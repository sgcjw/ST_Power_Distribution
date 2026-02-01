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
	uint32_t result = Read24(ina228, INA228_SHUNT_VOLTAGE);

	return (result * 0.01 );
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
