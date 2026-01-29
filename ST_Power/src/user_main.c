#include "user_main.h"
#include "main.h"

// ADD YOUR INCLUDES HERE
uint32_t screen_timer = 0;
uint32_t hb_timer = 0;

void user_setup()
{
    // ADD SETUP CODE HERE
    ssd1306_DisplayOnMsg();
    ssd1306_Init();
}

void user_loop()
{
    // ADD LOOP CODE HERE
    if (HAL_GetTick() - screen_timer > SCREEN_TIMER) {
        screen_timer = HAL_GetTick();
        //ssd1306_DisplayData();
    }
    if (HAL_GetTick() - hb_timer > HB_TIMER) {
        hb_timer = HAL_GetTick();
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    }
}

//	SSD1306 Data Display
//

/**
 * @brief Display normal operation screen
 * 
 */

// void ssd1306_DisplayData(BQ_data *BMS) {
// 	char buff[64];
// 	uint16_t integer, fraction;
// 	ssd1306_Fill(White);
// 	ssd1306_SetCursor(2,1);
// 	ssd1306_WriteString("Temp:   ", Font_6x8, Black);
//     integer = BMS->data.int_temperature / 10;
//     fraction = BMS->data.int_temperature % 10;  // two decimal places
    
//     snprintf(buff, sizeof(buff), "%d.%01d C", integer, fraction);
//     ssd1306_WriteString(buff, Font_6x8, Black);


//     ssd1306_SetCursor(96,1);
// 	ssd1306_WriteString("PMB", Font_6x8, Black);
//     snprintf(buff, sizeof(buff), "%d", PMB_ID);
//     ssd1306_WriteString(buff, Font_6x8, Black);

//     ssd1306_SetCursor(2,10);
// 	ssd1306_WriteString("Pres:    ", Font_6x8, Black);
// 	integer = (int) board_pressure;
// 	fraction = (int)((board_pressure - integer)*100);
//     snprintf(buff, sizeof(buff), "%d.%02d kPa", integer, fraction);
//     ssd1306_WriteString(buff, Font_6x8, Black);

//     ssd1306_SetCursor(2,21);

//     if (BMS->connection != CONNECTED){
//         ssd1306_WriteString("BMS Not Connected", Font_7x10, Black);
//     }
//     else {
//         ssd1306_WriteString("Current: ", Font_6x8, Black);

//         uint16_t current = BMS->data.current;
        
//         integer = current / 1000;
//         fraction = (current % 1000) / 10;  // two decimal places
        
//         snprintf(buff, sizeof(buff), "%d.%02d A", integer, fraction);
//         ssd1306_WriteString(buff, Font_6x8, Black);
    
//         ssd1306_SetCursor(2,31);
//         ssd1306_WriteString("Voltage: ", Font_6x8, Black);
//         integer = (int)(BMS->data.voltage/1000);
//         fraction = (int)((BMS->data.voltage/1000.0 - integer)*100);
//         snprintf(buff, sizeof(buff), "%d.%02d V", integer, fraction);
//         ssd1306_WriteString(buff, Font_6x8, Black);
    
//         ssd1306_SetCursor(2,41);
//         ssd1306_WriteString("Batt %:  ", Font_6x8, Black);
//         snprintf(buff, sizeof(buff), "%d", (int)BMS->data.percentage);
//         ssd1306_WriteString(buff, Font_6x8, Black);
    
//         ssd1306_SetCursor(2,51);
//         ssd1306_WriteString("State:   ", Font_6x8, Black);
//         setState(BMS->BQ_batteryStatus.all);
//         snprintf(buff, sizeof(buff), "%s", state);
//         ssd1306_WriteString(buff, Font_6x8, Black);
    
//     }

//     ssd1306_UpdateScreen();
// }

/**
 * @brief Display Turning On Process Screen
 * 
 */
void ssd1306_DisplayOnMsg() {
	ssd1306_Fill(White);
    ssd1306_SetCursor(2,31);
	ssd1306_WriteString("Waiting for Relay PCB Connection ...", Font_6x8, Black);
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
}

/**
 * @brief Display Turning Off Process Screen
 * 
 */
// void ssd1306_DisplayOffMsg() {
// 	ssd1306_Fill(White);
//     ssd1306_SetCursor(2,31);
// 	ssd1306_WriteString("TURNING OFF PMB ...", Font_6x8, Black);
// 	ssd1306_UpdateScreen();
// 	HAL_Delay(1000);
// }

void user_error_handler()
{
    // [OPTIONAL]: ADD ERROR HANDLER CODE HERE
}


// ADD OTHER HELPER FUNCTIONS HERE