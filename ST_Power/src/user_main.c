#include "user_main.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1;

// ADD YOUR INCLUDES HERE
uint32_t screen_timer = 0;
uint32_t hb_timer = 0;
uint32_t monitor_timer = 0;

bool PG = false;
bool FAULT = false;
bool EN_TEST = false;
bool OC_TEST = false;

INA228_t *ina228;
uint8_t ina228_address = 0x40;
float maxcurrent = 30.0;
float shunt = 0.004f;
uint16_t id = 0;
float current = 0;
float voltage = 0;
// float shunt_voltage = 0;
uint16_t temp = 0;
uint16_t oc = 0;


void user_setup()
{
    // ADD SETUP CODE HERE
    ssd1306_DisplayOnMsg();
    ssd1306_Init();
    if (INA228_Init(&ina228, &hi2c1, ina228_address, maxcurrent, shunt) == 1) {
        ssd1306_DisplayReadyMsg();
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // Remove the OC fault condition
    Automated_Check();
}

void user_loop()
{
    // ADD LOOP CODE HERE
    if (HAL_GetTick() - hb_timer > HB_TIMER) {
        hb_timer = HAL_GetTick();
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    }
    if (HAL_GetTick() - monitor_timer > MONITOR_TIMER) {
        monitor_timer = HAL_GetTick();
        // PG = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
        // FAULT = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
        id = INA228_getDieID(&ina228);
        current = INA228_ReadCurrent(&ina228, maxcurrent); // Subtract 450mA offset for more accurate readings
        voltage = INA228_ReadBusVoltage(&ina228);
        // shunt_voltage = INA228_ReadShuntVoltage(&ina228)/0.004f;
        temp = INA228_getTemperature(&ina228);
        oc = OC_check();
    }
    if (HAL_GetTick() - screen_timer > SCREEN_TIMER) {
        screen_timer = HAL_GetTick();
        ssd1306_DisplayData();
    }
}

// Automated Check Function at startup
void Automated_Check() {
    // Implement automated checks needed at startup
    ssd1306_DisplayENTestMsg();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // Set PA5 high to enable relay
    HAL_Delay(500); // Wait for relay to turn on
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // Set PA5 low to test latching functionality
    HAL_Delay(500); // Wait for relay to latch
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_RESET) {
        user_error_handler(); // Relay did not enable successfully, handle error
    }
    else{
        EN_TEST = true;
        ssd1306_DisplayENTestMsg();
    }
    ssd1306_DisplayOCTestMsg();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // Set PA6 high to trigger OC Test
    HAL_Delay(500); // Wait for OC test to register
    if ((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) || (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET)) {
        user_error_handler(); // Proetction did not trigger, handle error
    }
    else{
        OC_TEST = true;
        ssd1306_DisplayOCTestMsg();
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // Reset OC Test
}

// Overcurrent setting check function
uint16_t OC_check() {
    uint16_t oc_setting = 0;
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    oc_setting = HAL_ADC_GetValue(&hadc1);
    return oc_setting; // Return in Amperes
}

//	SSD1306 Data Display
//
/**
 * @brief Display normal operation screen
 * 
 */

void ssd1306_DisplayData() {
	char buff[64];
	uint16_t integer, fraction;
	ssd1306_Fill(White);

	// ssd1306_SetCursor(2,5);
	// ssd1306_WriteString("PG:  ", Font_6x8, Black);
    // integer = PG ? 1 : 0;
    // snprintf(buff, sizeof(buff), "%d", integer);
    // ssd1306_WriteString(buff, Font_6x8, Black);


    // ssd1306_SetCursor(70,5);
	// ssd1306_WriteString("Fault:  ", Font_6x8, Black);
    // integer = FAULT ? 1 : 0;
    // snprintf(buff, sizeof(buff), "%d", integer);
    // ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(2,5);
	ssd1306_WriteString("Board ID:  ", Font_6x8, Black);
    integer = id;
    snprintf(buff, sizeof(buff), "%d", integer);
    ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(2,20);
	ssd1306_WriteString("Voltage:", Font_6x8, Black);
	integer = (int) voltage/1000;
	fraction = (int)(voltage - integer*1000);
    snprintf(buff, sizeof(buff), "%d.%03d V", integer, fraction);
    ssd1306_WriteString(buff, Font_6x8, Black);

    // ssd1306_SetCursor(2,20);
	// ssd1306_WriteString("Shunt:", Font_6x8, Black);
	// integer = (int) shunt_voltage/1000;
    // fraction = (int)(shunt_voltage - integer*1000);
    // snprintf(buff, sizeof(buff), "%d.%02d V", integer, fraction);
    // ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(2,30);
	ssd1306_WriteString("Current:", Font_6x8, Black);
	integer = (int) current/1000;
	fraction = (int)(current - integer*1000);
    snprintf(buff, sizeof(buff), "%d.%03d A", integer, fraction);
    ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(2,40);
	ssd1306_WriteString("Temperature:", Font_6x8, Black);
	integer = (int) temp/1000;
	fraction = (int)((temp - integer*1000));
    snprintf(buff, sizeof(buff), "%d.%03d C", integer, fraction);
    ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(2,50);
	ssd1306_WriteString("OC SETTING:", Font_6x8, Black);
	if (750 <= oc && oc <= 850){
        integer = 5;
    }
    else if (1450 <= oc && oc <= 1550){
        integer = 10;
    }
    else if (1900 <= oc && oc <= 2000){
        integer = 20;
    }
    else if (2200 <= oc && oc <= 2300){
        integer = 30;
    }
    else {
        integer = 0; // No OC Setting 
    }
    snprintf(buff, sizeof(buff), "%d A", integer);
    ssd1306_WriteString(buff, Font_6x8, Black);

    // ssd1306_SetCursor(2,21);
    // if (BMS->connection != CONNECTED){
    //     ssd1306_WriteString("BMS Not Connected", Font_7x10, Black);
    // }
    // else {
    //     ssd1306_WriteString("Current: ", Font_6x8, Black);

    //     uint16_t current = BMS->data.current;
        
    //     integer = current / 1000;
    //     fraction = (current % 1000) / 10;  // two decimal places
        
    //     snprintf(buff, sizeof(buff), "%d.%02d A", integer, fraction);
    //     ssd1306_WriteString(buff, Font_6x8, Black);
    
    //     ssd1306_SetCursor(2,31);
    //     ssd1306_WriteString("Voltage: ", Font_6x8, Black);
    //     integer = (int)(BMS->data.voltage/1000);
    //     fraction = (int)((BMS->data.voltage/1000.0 - integer)*100);
    //     snprintf(buff, sizeof(buff), "%d.%02d V", integer, fraction);
    //     ssd1306_WriteString(buff, Font_6x8, Black);
    
    //     ssd1306_SetCursor(2,41);
    //     ssd1306_WriteString("Batt %:  ", Font_6x8, Black);
    //     snprintf(buff, sizeof(buff), "%d", (int)BMS->data.percentage);
    //     ssd1306_WriteString(buff, Font_6x8, Black);
    
    //     ssd1306_SetCursor(2,51);
    //     ssd1306_WriteString("State:   ", Font_6x8, Black);
    //     setState(BMS->BQ_batteryStatus.all);
    //     snprintf(buff, sizeof(buff), "%s", state);
    //     ssd1306_WriteString(buff, Font_6x8, Black);
    
    // }

    ssd1306_UpdateScreen();
}

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
 * @brief Display Ready Screen
 * 
 */
void ssd1306_DisplayReadyMsg() {
	ssd1306_Fill(White);
    ssd1306_SetCursor(2,31);
	ssd1306_WriteString("Ready to Start Monitoring", Font_6x8, Black);
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

/**
 * @brief Display ERROR Screen
 * 
 */
void ssd1306_DisplayErrorMsg() {
	ssd1306_Fill(White);
    ssd1306_SetCursor(2,31);
	ssd1306_WriteString("ERROR: System Failure", Font_6x8, Black);
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
}

/**
 * @brief Display EN Test Screen
 * 
 */
void ssd1306_DisplayENTestMsg() {
    ssd1306_Fill(White);
    ssd1306_SetCursor(2,31);
    if (EN_TEST){
        ssd1306_WriteString("TESTING EN: PASS", Font_6x8, Black);
    }
    else{
        ssd1306_WriteString("TESTING EN", Font_6x8, Black);
    }
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
}

/**
 * @brief Display OC Test Screen
 * 
 */
void ssd1306_DisplayOCTestMsg() {
	ssd1306_Fill(White);
    ssd1306_SetCursor(2,31);
    if (OC_TEST){
        ssd1306_WriteString("TESTING OC: PASS", Font_6x8, Black);
    }
    else{
        ssd1306_WriteString("TESTING OC PROTECTION", Font_6x8, Black);
    }
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
}

void user_error_handler()
{
    while (1)
    {
        ssd1306_DisplayErrorMsg();
    }
}


// ADD OTHER HELPER FUNCTIONS HERE