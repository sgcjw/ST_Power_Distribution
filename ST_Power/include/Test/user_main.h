/**
 * @file user_main.h
 * @author [Add name]
 * @brief user main header file
 * @version 0.1
 * @date yyyy-mm-dd
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef USER_MAIN_H
#define USER_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

// ADD YOUR INCLUDES HERE
#include "Test/SSD1306/ssd1306.h"

void user_setup();
void user_loop();
void user_error_handler();

// ADD OTHER HELPER FUNCTIONS HERE
void ssd1306_DisplayData();
void ssd1306_DisplayOnMsg();
void ssd1306_DisplayOffMsg();

#define SCREEN_TIMER 500  // Screen update interval in milliseconds
#define HB_TIMER 1000  // Heartbeat interval in milliseconds

#ifdef __cplusplus
}
#endif

#endif /* USER_MAIN_H */