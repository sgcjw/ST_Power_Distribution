#ifndef RELAY_DEVICE_H
#define RELAY_DEVICE_H

#include "main.h"
#include "ina228.h"

#define MAX_RELAYS 27

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;

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

// Shared global array
extern relay_device_t relay[MAX_RELAYS];
extern uint8_t relay_count;
extern uint8_t Relay_Configure[MAX_RELAYS];

void scan_bus(I2C_HandleTypeDef *i2c);
void Relay_BeginRefresh(void);
void Relay_EndRefresh(void);
uint8_t RELAY_ReadPG(uint8_t slot);

#endif