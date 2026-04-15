#ifndef RELAY_DEVICE_H
#define RELAY_DEVICE_H

#include "main.h"
#include "ina228.h"

#define MAX_RELAYS 13

typedef struct {
    INA228_t ina;

    float voltage;
    float current;
    float temp;

    uint8_t pg;
    uint8_t fault;

    uint16_t oc_adc;
    int oc_setting;

    uint8_t ready;
} relay_device_t;

// Shared global array
extern relay_device_t relay[MAX_RELAYS];
extern uint8_t relay_count;

#endif