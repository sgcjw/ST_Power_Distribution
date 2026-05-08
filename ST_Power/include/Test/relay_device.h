#ifndef RELAY_DEVICE_H
#define RELAY_DEVICE_H

#include "main.h"
#include "ina228.h"

#define MAX_RELAYS 27

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

typedef struct {
    I2C_HandleTypeDef *i2c;
    INA228_t ina;

    uint8_t addr;
    uint8_t slot;

    uint8_t ready;
    uint8_t present;

    uint8_t pg;
    uint8_t fault;
    uint8_t oc_set;
} relay_device_t;

typedef struct {
    I2C_HandleTypeDef *i2c;
    uint8_t addr;
    uint8_t slot;
} SlotMap_t;

static const SlotMap_t slot_map[] = {
    {&hi2c2, 0x40, 1},
    {&hi2c3, 0x40, 2},
    {&hi2c2, 0x41, 3},
    {&hi2c3, 0x41, 4},
    {&hi2c2, 0x42, 5},
    {&hi2c3, 0x42, 6},
    {&hi2c2, 0x43, 7},
    {&hi2c3, 0x43, 8},
    {&hi2c2, 0x44, 9},
    {&hi2c3, 0x44, 10},
    {&hi2c2, 0x45, 11},
    {&hi2c3, 0x45, 12},
    {&hi2c2, 0x46, 13},
    {&hi2c3, 0x46, 14},
    {&hi2c2, 0x47, 15},
    {&hi2c3, 0x47, 16},
    {&hi2c2, 0x48, 17},
    {&hi2c3, 0x48, 18},
    {&hi2c2, 0x49, 19},
    {&hi2c3, 0x49, 20},
    {&hi2c2, 0x4A, 21},
    {&hi2c3, 0x4A, 22},
    {&hi2c2, 0x4B, 23},
    {&hi2c3, 0x4B, 24},
    {&hi2c2, 0x4C, 25},
    {&hi2c3, 0x4C, 26},
};

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} GPIO_Map_t;

static const GPIO_Map_t pg_map[] = {
    [1]  = {GPIOA, GPIO_PIN_15},
    [2]  = {GPIOC, GPIO_PIN_0},
    [3]  = {GPIOA, GPIO_PIN_10},
    [4]  = {GPIOF, GPIO_PIN_13},
    [5]  = {GPIOC, GPIO_PIN_11},
    [6]  = {GPIOF, GPIO_PIN_12},
    [7]  = {GPIOG, GPIO_PIN_5},
    [8]  = {GPIOG, GPIO_PIN_9},
    [9]  = {GPIOG, GPIO_PIN_7},
    [10] = {GPIOG, GPIO_PIN_3},
    [11] = {GPIOE, GPIO_PIN_14},
    [12] = {GPIOD, GPIO_PIN_5},
    [13] = {GPIOB, GPIO_PIN_10},
    [14] = {GPIOD, GPIO_PIN_7},
    [15] = {GPIOB, GPIO_PIN_8},
    [16] = {GPIOB, GPIO_PIN_4},
    [17] = {GPIOD, GPIO_PIN_15},
    [18] = {GPIOB, GPIO_PIN_6},
    [19] = {GPIOC, GPIO_PIN_7},
    [20] = {GPIOB, GPIO_PIN_9},
    [21] = {GPIOG, GPIO_PIN_1},
    [22] = {GPIOE, GPIO_PIN_0},
    [23] = {GPIOG, GPIO_PIN_3},
    [24] = {GPIOE, GPIO_PIN_3},
    [25] = {GPIOC, GPIO_PIN_8},
    [26] = {GPIOE, GPIO_PIN_5},
};

static const GPIO_Map_t fault_map[] = {
    [1]  = {GPIOC, GPIO_PIN_10},
    [2]  = {GPIOB, GPIO_PIN_1},
    [3]  = {GPIOB, GPIO_PIN_1},
    [4]  = {GPIOF, GPIO_PIN_11},
    [5]  = {GPIOC, GPIO_PIN_12},
    [6]  = {GPIOB, GPIO_PIN_2},
    [7]  = {GPIOB, GPIO_PIN_3},
    [8]  = {GPIOA, GPIO_PIN_4},
    [9]  = {GPIOB, GPIO_PIN_4},
    [10] = {GPIOA, GPIO_PIN_3},
    [11] = {GPIOE, GPIO_PIN_15},
    [12] = {GPIOA, GPIO_PIN_2},
    [13] = {GPIOB, GPIO_PIN_6},
    [14] = {GPIOB, GPIO_PIN_3 },
    [15] = {GPIOB, GPIO_PIN_7},
    [16] = {GPIOA, GPIO_PIN_0},
    [17] = {GPIOC, GPIO_PIN_6},
    [18] = {GPIOC, GPIO_PIN_6},
    [19] = {GPIOB, GPIO_PIN_9},
    [20] = {GPIOE, GPIO_PIN_1},
    [21] = {GPIOB, GPIO_PIN_10},
    [22] = {GPIOE, GPIO_PIN_2},
    [23] = {GPIOD, GPIO_PIN_10},
    [24] = {GPIOE, GPIO_PIN_4},
    [25] = {GPIOC, GPIO_PIN_9},
    [26] = {GPIOE, GPIO_PIN_6},
};

typedef struct {
    ADC_HandleTypeDef *hadc;
    uint32_t channel;
} Analog_Map_t;

static const Analog_Map_t oc_map[] = {
    [1]  = {&hadc3, ADC_CHANNEL_10}, //PD13
    [2]  = {&hadc1, ADC_CHANNEL_1}, //PA1
    [3]  = {&hadc3, ADC_CHANNEL_11}, //PD14
    [4]  = {&hadc1, ADC_CHANNEL_0}, //PA0
    [5]  = {&hadc3, ADC_CHANNEL_8}, //PD11
    [6]  = {&hadc1, ADC_CHANNEL_3}, //PA3
    [7]  = {&hadc3, ADC_CHANNEL_2}, //PE9
    [8]  = {&hadc1, ADC_CHANNEL_2}, //PA2
    [9]  = {&hadc3, ADC_CHANNEL_16}, //PE12
    [10] = {&hadc2, ADC_CHANNEL_13}, //PA5
    [11] = {&hadc3, ADC_CHANNEL_15}, //PE11
    [12] = {&hadc2, ADC_CHANNEL_17}, //PA4
    [13] = {&hadc3, ADC_CHANNEL_3}, //PE13
    [14] = {&hadc2, ADC_CHANNEL_4}, //PA7
    [15] = {&hadc1, ADC_CHANNEL_11}, //PB12
    [16] = {&hadc2, ADC_CHANNEL_3}, //PA6
    [17] = {&hadc1, ADC_CHANNEL_14}, //PB11
    [18] = {&hadc2, ADC_CHANNEL_11}, //PC5
    [19] = {&hadc1, ADC_CHANNEL_5}, //PB14
    [20] = {&hadc2, ADC_CHANNEL_5}, //PC4
    [21] = {&hadc3, ADC_CHANNEL_5}, //PB13
    [22] = {&hadc3, ADC_CHANNEL_6}, //PE8
    [23] = {&hadc3, ADC_CHANNEL_7}, //PD10
    [24] = {&hadc3, ADC_CHANNEL_4}, //PE7
    [25] = {&hadc3, ADC_CHANNEL_9}, //PD12
    [26] = {&hadc3, ADC_CHANNEL_14}, //PE10
};

// Shared global array
extern relay_device_t relay[MAX_RELAYS];
extern uint8_t relay_count;
extern uint8_t Relay_Configure[MAX_RELAYS];

void scan_bus(I2C_HandleTypeDef *i2c);
void Relay_BeginRefresh(void);
void Relay_EndRefresh(void);
uint8_t RELAY_ReadPG(uint8_t slot);
uint8_t RELAY_ReadFault(uint8_t slot);
uint16_t RELAY_ReadOC(uint8_t slot);

#endif