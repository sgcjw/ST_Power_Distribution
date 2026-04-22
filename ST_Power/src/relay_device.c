#include "relay_device.h"
#include "ina228.h"

relay_device_t relay[MAX_RELAYS];
uint8_t relay_count = 0;
uint8_t Relay_Configure[MAX_RELAYS] = {1,2,18};

void scan_bus(I2C_HandleTypeDef *i2c)
{
    uint8_t temp_list[16];
    uint8_t found = INA228_Scan(i2c, temp_list, 16);

    for (uint8_t i = 0; i < found; i++)
    {
        for (uint8_t m = 0; m < sizeof(slot_map) / sizeof(slot_map[0]); m++)
        {
            if (slot_map[m].i2c == i2c &&
                slot_map[m].addr == temp_list[i])
            {
                uint8_t slot = slot_map[m].slot;

                if (slot >= MAX_RELAYS)
                    continue;

                relay[slot].i2c = i2c;
                relay[slot].addr = temp_list[i];
                relay[slot].slot = slot;
                relay[slot].present = 1;

                // first time seen or reconnected
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

void Relay_BeginRefresh(void)
{
    for (uint8_t i = 0; i < MAX_RELAYS; i++)
    {
        relay[i].present = 0;
    }
}

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

uint8_t RELAY_ReadPG(uint8_t slot)
{
    return HAL_GPIO_ReadPin(pg_map[slot].port,
                             pg_map[slot].pin);
}