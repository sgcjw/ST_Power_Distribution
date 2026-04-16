#include "relay_device.h"

relay_device_t relay[MAX_RELAYS];
uint8_t relay_count = 0;
uint8_t Relay_Configure[MAX_RELAYS] = {1,2,18};

void scan_bus(I2C_HandleTypeDef *i2c)
{
    uint8_t temp_list[16];
    uint8_t found = INA228_Scan(i2c, temp_list, 16);

    for (uint8_t i = 0; i < found; i++)
    {
        for (uint8_t m = 0; m < sizeof(slot_map)/sizeof(slot_map[0]); m++)
        {
            if (slot_map[m].i2c == i2c &&
                slot_map[m].addr == temp_list[i])
            {
                uint8_t slot = slot_map[m].slot;

                relay[slot].i2c  = i2c;
                relay[slot].addr = temp_list[i];
                relay[slot].slot = slot;
                relay[slot].ready = 0;

                if (slot >= relay_count)
                    relay_count = slot + 1;
            }
        }
    }
}

uint8_t RELAY_ReadPG(uint8_t slot)
{
    return HAL_GPIO_ReadPin(pg_map[slot].port,
                             pg_map[slot].pin);
}