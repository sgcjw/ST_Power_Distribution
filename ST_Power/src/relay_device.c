#include "relay_device.h"
#include "ina228.h"

relay_device_t relay[MAX_RELAYS];
uint8_t relay_count = 0;
uint8_t Relay_Configure[MAX_RELAYS] = {1,2,18};

/** @brief Scan the I2C bus for INA228 devices. Record any found devices in the global relay array.
 *  @param i2c Pointer to the I2C handle.
 *  @retval void
*/

void scan_bus(I2C_HandleTypeDef *i2c)
{
    uint8_t temp_list[16];
    uint8_t found = INA228_Scan(i2c, temp_list, 16);

    // Loop through found addresses and match to slot map to populate relay array
    for (uint8_t i = 0; i < found; i++)
    {
        for (uint8_t m = 0; m < sizeof(slot_map) / sizeof(slot_map[0]); m++)
        {
            if (slot_map[m].i2c == i2c &&
                slot_map[m].addr == temp_list[i])
            {
                uint8_t slot = slot_map[m].slot;

                // if (slot >= MAX_RELAYS)
                //     continue;
                relay[slot].i2c = i2c;
                relay[slot].addr = temp_list[i];
                relay[slot].slot = slot;
                relay[slot].present = 1;

                // Based on the ready flag, determine whether the relay is newly connected or was already known from a previous scan. 
                // If newly connected, attempt to initialize the INA228 device.
                if (!relay[slot].ready)
                {
                    if (INA228_Init(&relay[slot].ina,
                                    relay[slot].i2c,
                                    relay[slot].addr,
                                    30, 0.004f, 3, 5, 3, 200))
                    {
                        relay[slot].ready = 1;
                    }
                }
                break;
            }
        }
    }
}

/** @brief Mark the beginning of a relay rescan cycle. This will clear the 'present' flag for all relays.
 *  @param void
 *  @retval void
 */
void Relay_BeginRefresh(void)
{
    for (uint8_t i = 0; i < MAX_RELAYS; i++)
    {
        relay[i].present = 0;
    }
}

/** @brief Mark the end of a relay rescan cycle. Any relay that was not marked 'present' during the scan will be considered disconnected and have its I2C info cleared.
 *  @param void
 *  @retval void
 */
void Relay_EndRefresh(void)
{
    for (uint8_t i = 0; i < MAX_RELAYS; i++)
    {
        if (!relay[i].present)
        {
            relay[i].i2c = NULL;
            relay[i].addr = 0;
            relay[i].slot = 0xFF;
            relay[i].ready = 0;
        }
    }
}

/** @brief Read the Power Good (PG) signal for a given relay slot.
 *  @param slot The relay slot number to read from.
 *  @retval uint8_t The state of the PG signal (0 or 1).
*/
uint8_t RELAY_ReadPG(uint8_t slot)
{
    return HAL_GPIO_ReadPin(pg_map[slot].port,
                             pg_map[slot].pin);
}

/** @brief Read the Fault signal for a given relay slot.
 *  @param slot The relay slot number to read from.
 *  @retval uint8_t The state of the Fault signal (0 or 1).
*/
uint8_t RELAY_ReadFault(uint8_t slot)
{
    return HAL_GPIO_ReadPin(fault_map[slot].port,
                             fault_map[slot].pin);
}

/** @brief Read the Overcurrent (OC) signal for a given relay slot.
 *  @param slot The relay slot number to read from.
 *  @retval uint16_t The value of the OC signal.
 */
uint16_t RELAY_ReadOC(uint8_t slot)
{
    if (slot >= MAX_RELAYS)
        return 0;

    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = oc_map[slot].channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;

    HAL_ADC_ConfigChannel(oc_map[slot].hadc, &sConfig);

    HAL_ADC_Start(oc_map[slot].hadc);
    HAL_ADC_PollForConversion(oc_map[slot].hadc, 10);

    uint16_t value = HAL_ADC_GetValue(oc_map[slot].hadc);

    HAL_ADC_Stop(oc_map[slot].hadc);

    if (value >= 2300 && value <= 2450)
        return 5;
    else if (value >= 2700 && value <= 2850)
        return 10;
    else if (value >= 2950 && value <= 3100)
        return 20;
    else if (value >= 3150 && value <= 3300)
        return 30;
    else
        return 0;
}