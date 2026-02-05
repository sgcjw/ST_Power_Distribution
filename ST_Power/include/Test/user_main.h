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
#include "Test/INA228.h"

void user_setup();
void user_loop();
void user_error_handler();

// ADD OTHER HELPER FUNCTIONS HERE
void Automated_Check();
int OC_check();
void ssd1306_DisplayData();
void ssd1306_DisplayOnMsg();
void ssd1306_DisplayReadyMsg();
void ssd1306_DisplayErrorMsg();
void ssd1306_DisplayENTestMsg();
void ssd1306_DisplayOCTestMsg();


#define SCREEN_TIMER 1000  // Screen update interval in milliseconds
#define HB_TIMER 1000  // Heartbeat interval in milliseconds
#define MONITOR_TIMER 0 // Monitor interval in milliseconds
#define USB_TIMER 1000 // Sending USB message in milliseconds

#ifdef __cplusplus
}
#endif

#endif /* USER_MAIN_H */