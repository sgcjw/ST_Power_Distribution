#include "user_main.h"
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

extern ADC_HandleTypeDef hadc1;

// ADD YOUR INCLUDES HERE
uint32_t screen_timer = 0;
uint32_t hb_timer = 0;
uint32_t monitor_timer = 0;
uint32_t usb_timer = 0;

bool PG = true;
bool FAULT = false;
bool EN_TEST = false;
bool OC_TEST = false;

INA228_t ina228;
uint8_t ina228_address = 0x40;
float maxcurrent = 30.0;
float shunt = 0.001f;
uint8_t bvct = 3; // us bus voltage conversion time
uint8_t svct = 5; // us shunt voltage conversion time
uint8_t tct = 3;  // us temperature conversion time
uint16_t ppm = 200; // 200 ppm temperature coefficent
uint16_t id = 0;
float current = 0;
float fault_current = 0;
float voltage = 0;
float fault_voltage = 0;
// float shunt_voltage = 0;
float temp = 0;
int oc = 0;


void user_setup()
{
    MX_USB_DEVICE_Init();    // MUST be called after MX_USB_PCD_Init()
    // ADD SETUP CODE HERE
    ssd1306_Init();
    ssd1306_DisplayOnMsg();
    if (INA228_Init(&ina228, &hi2c1, ina228_address, maxcurrent, shunt, bvct, svct, tct, ppm) == 1) {
        ssd1306_DisplayReadyMsg();
    }
    Automated_Check();
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_7);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
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
        current = INA228_ReadCurrent(&ina228, maxcurrent);
        voltage = INA228_ReadBusVoltage(&ina228);
        // shunt_voltage = INA228_ReadShuntVoltage(&ina228)/0.004f;
        temp = INA228_getTemperature(&ina228);
    }
    if (HAL_GetTick() - screen_timer > SCREEN_TIMER) {
        id = INA228_getDieID(&ina228);
        oc = OC_check();
        screen_timer = HAL_GetTick();
        ssd1306_DisplayData();
    }
    if (HAL_GetTick() - usb_timer > USB_TIMER) {
        usb_timer = HAL_GetTick();
        char string[128];

        int v_i = (int)(voltage/1000);
        int v_f = (int)(voltage - v_i * 1000);

        int c_i = (int)(current/1000);
        int c_f = (int)(current - c_i * 1000);

        int t_i = (int)(temp/1000);
        int t_f = (int)(temp - t_i * 1000);

        snprintf(string, sizeof(string),
            "Voltage: %d.%03d V\r\n"
            "Current: %d.%03d A\r\n"
            // for Temp Recording
            // "%d.%03d \r\n", 
            "Temp:    %d.%03d C\r\n",
            v_i, v_f,
            c_i, c_f,
            t_i, t_f);

        CDC_Transmit_FS((uint8_t*)string, strlen(string));
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
    HAL_Delay(500); // Wait for system to stabilize
}

// Overcurrent setting check function
int OC_check() {
    uint16_t reading = 0;
    int oc_setting = 0;
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    reading = HAL_ADC_GetValue(&hadc1);
    if (700 <= reading && reading <= 900){
        oc_setting = 5;
    }
    else if (1400 <= reading && reading <= 1600){
        oc_setting = 10;
    }
    else if (1850 <= reading && reading <= 2050){
        oc_setting = 20;
    }
    else if (2150 <= reading && reading <= 2350){
        oc_setting = 30;
    }
    else {
        oc_setting = 0; // No OC Setting 
    }
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
    if (PG == GPIO_PIN_RESET) {
        if (FAULT == GPIO_PIN_SET){
            if (fault_current > (oc - 1)*1000){
                ssd1306_Fill(White);
                ssd1306_SetCursor(2,31);
                ssd1306_WriteString("Overcurrent: ", Font_6x8, Black);
                integer = (int) fault_current/1000;
	            fraction = (int)(fault_current - integer*1000);
                snprintf(buff, sizeof(buff), "%d.%03d A", integer, fraction);
                ssd1306_WriteString(buff, Font_6x8, Black);
                ssd1306_UpdateScreen();
            }
            else if(fault_voltage > 28000){
                ssd1306_Fill(White);
                ssd1306_SetCursor(2,31);
                ssd1306_WriteString("Overvoltage: ", Font_6x8, Black);
	            integer = (int) fault_voltage/1000;
	            fraction = (int)(fault_voltage - integer*1000);
                snprintf(buff, sizeof(buff), "%d.%03d V", integer, fraction);
                ssd1306_WriteString(buff, Font_6x8, Black);
                ssd1306_UpdateScreen();
            }
            else if(fault_voltage < 11000){
                ssd1306_Fill(White);
                ssd1306_SetCursor(2,31);
                ssd1306_WriteString("Undervoltage: ", Font_6x8, Black);
                integer = (int) fault_voltage/1000;
	            fraction = (int)(fault_voltage - integer*1000);
                snprintf(buff, sizeof(buff), "%d.%03d V", integer, fraction);
                ssd1306_WriteString(buff, Font_6x8, Black);
                ssd1306_UpdateScreen();
            }
            else{
                ssd1306_Fill(White);
                ssd1306_SetCursor(2,20);
                ssd1306_WriteString("Unknown Fault", Font_6x8, Black);
                ssd1306_SetCursor(2,30);
                integer = (int) fault_voltage/1000;
	            fraction = (int)(fault_voltage - integer*1000);
                snprintf(buff, sizeof(buff), "%d.%03d V", integer, fraction);
                ssd1306_WriteString(buff, Font_6x8, Black);
                ssd1306_SetCursor(2,40);
                integer = (int) fault_current/1000;
	            fraction = (int)(fault_current - integer*1000);
                snprintf(buff, sizeof(buff), "%d.%03d A", integer, fraction);
                ssd1306_WriteString(buff, Font_6x8, Black);
                ssd1306_UpdateScreen();
            }
        }
        else{
                ssd1306_Fill(White);
                ssd1306_SetCursor(2,31);
                ssd1306_WriteString("Channel Disabled", Font_6x8, Black);
                ssd1306_UpdateScreen();
        }
    }
    else{
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
        integer = oc;
        snprintf(buff, sizeof(buff), "%d A", integer);
        ssd1306_WriteString(buff, Font_6x8, Black);

        ssd1306_UpdateScreen();
    }
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
	ssd1306_WriteString("ERROR: Init Failure", Font_6x8, Black);
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

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
}

void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    fault_current = current;
    // //to bypass undervoltage when no voltage readings
    // fault_voltage = 20000;
    fault_voltage = voltage;
    PG = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
    FAULT = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
}

void user_error_handler()
{
    while (1)
    {
        ssd1306_DisplayErrorMsg();
    }
}


// ADD OTHER HELPER FUNCTIONS HERE