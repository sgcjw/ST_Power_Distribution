#include "user_main.h"
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "relay_device.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern FDCAN_HandleTypeDef hfdcan1;

// ADD YOUR INCLUDES HERE
uint32_t screen_timer = 0;
uint32_t hb_timer = 0;
uint32_t monitor_timer = 0;
uint32_t usb_timer = 0;

bool PG = true;
bool FAULT = false;
bool FAULT_CHECK = false;
uint16_t FAULT_TYPE = 0; // 1 for overcurrent, 2 for overvoltage, 3 for undervoltage, 4 for overtemperature
bool EN_TEST = false;
bool OC_TEST = false;

uint8_t RELAY_READY[MAX_RELAYS] = {0};
uint8_t relay_addrs[MAX_RELAYS] = {0};

float maxcurrent = 30.0;
float shunt = 0.004f;
uint8_t bvct = 3; // 280us bus voltage conversion time
uint8_t svct = 5; // 1052us shunt voltage conversion time
uint8_t tct = 3;  // 280us temperature conversion time
uint16_t ppm = 200; // 200 ppm temperature coefficent
uint16_t id = 0;
float current = 0;
float fault_current = 0;
float voltage = 0;
float fault_voltage = 0;
// float shunt_voltage = 0;
float temp = 0;
int oc = 0;
float voltage_buf[MAX_RELAYS] = {0};
float current_buf[MAX_RELAYS] = {0};
float temp_buf[MAX_RELAYS] = {0};

//CAN Message Variables
FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];

void user_setup()
{
    // ADD SETUP CODE HERE
    // ssd1306_DisplayOnMsg();
    // ssd1306_Init();
    // Automated_Check();
    relay_count = INA228_Scan(&hi2c2, relay_addrs, MAX_RELAYS);
    HAL_FDCAN_Start(&hfdcan1);
    for (uint8_t i = 0; i < relay_count; i++) {
        if (INA228_Init(&relay[i].ina, &hi2c2, relay_addrs[i], maxcurrent, shunt, bvct, svct, tct, ppm) == 1) {
            relay[i].ready = 1;
        }
    }
    // HAL_ADC_Start(&hadc1);
    // HAL_ADC_Start(&hadc2);
    HAL_ADC_Start(&hadc3);
    // TxHeader.Identifier = 0x101;
    // TxHeader.IdType = FDCAN_STANDARD_ID;
    // TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    // TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    // TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    // TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    // TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    // TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    // TxHeader.MessageMarker = 0;
    // __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_7);
    // __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
    // HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    // HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void user_loop()
{
    // ADD LOOP CODE HERE
    if (HAL_GetTick() - hb_timer > HB_TIMER) {
        hb_timer = HAL_GetTick();
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        // // Prepare CAN heartbeat message
        // TxData[0] = 1; // Example data, can be modified to include actual status information
        // TxData[1] = 0;
        // TxData[2] = 0;
        // TxData[3] = 0;
        // TxData[4] = 0;
        // TxData[5] = 0;
        // TxData[6] = 0;
        // TxData[7] = 0;
        // if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) > 0) {
        //     HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData);
        // }
    }
    if (HAL_GetTick() - monitor_timer > MONITOR_TIMER)
    {
        monitor_timer = HAL_GetTick();
        for (uint8_t i = 0; i < relay_count; i++)
        {
            if (!relay[i].ready) continue;
            // store latest values ONLY
            voltage_buf[i] = INA228_ReadBusVoltage(&relay[i].ina);
            current_buf[i] = INA228_ReadCurrent(&relay[i].ina, maxcurrent);
            temp_buf[i]    = INA228_getTemperature(&relay[i].ina);
        }
    }
    if (HAL_GetTick() - usb_timer > USB_TIMER) {
        usb_timer = HAL_GetTick();
        for (uint8_t i = 0; i < relay_count; i++)
        {
            if (!relay[i].ready) continue;

            float v = voltage_buf[i];
            float c = current_buf[i];
            float t = temp_buf[i];

            char msg[128];

            int v_i = (int)(v / 1000);
            int v_f = (int)(v - v_i * 1000);

            int c_i = (int)(c / 1000);
            int c_f = (int)(c - c_i * 1000);

            int t_i = (int)(t / 1000);
            int t_f = (int)(t - t_i * 1000);

            snprintf(msg, sizeof(msg),
                "Relay[%d] ADDR:0x%02X\r\n"
                "  V: %d.%03d V\r\n"
                "  C: %d.%03d A\r\n"
                "  T: %d.%03d C\r\n\r\n",
                i,
                relay_addrs[i],
                v_i, v_f,
                c_i, c_f,
                t_i, t_f
            );

            CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
            HAL_Delay(5); // small delay to avoid USB buffer overflow
        }
    }
    // if (HAL_GetTick() - screen_timer > SCREEN_TIMER) {
    //     id = INA228_getDieID(&ina228);
    //     oc = OC_check();
    //     screen_timer = HAL_GetTick();
    //     ssd1306_DisplayData();
    // }
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

    HAL_ADC_Start(&hadc3);
    HAL_ADC_PollForConversion(&hadc3, 10);
    reading = HAL_ADC_GetValue(&hadc3);
    HAL_ADC_Stop(&hadc3);

    if (reading >= 700 && reading <= 900)
        oc_setting = 5;
    else if (reading >= 1400 && reading <= 1600)
        oc_setting = 10;
    else if (reading >= 1850 && reading <= 2050)
        oc_setting = 20;
    else if (reading >= 2150 && reading <= 2350)
        oc_setting = 30;
    else
        oc_setting = 0;

    return oc_setting;
}

//	SSD1306 Data Display
//
/**
 * @brief Display normal operation screen
 * 
 */

// void ssd1306_DisplayData() {
// 	char buff[64];
// 	uint16_t integer, fraction;
//     if (FAULT_CHECK){
//         FAULT_TYPE = INA228_checkFault(&relay[0].ina);
//         FAULT_CHECK = 0;
//     }
// 	ssd1306_Fill(White);
//     if (PG == GPIO_PIN_RESET) {
//         if (FAULT == GPIO_PIN_SET){
//             if (FAULT_TYPE == 1){
//                 ssd1306_Fill(White);
//                 ssd1306_SetCursor(2,31);
//                 ssd1306_WriteString("Overcurrent", Font_6x8, Black);
//                 ssd1306_UpdateScreen();
//             }
//             else if(FAULT_TYPE == 2){
//                 ssd1306_Fill(White);
//                 ssd1306_SetCursor(2,31);
//                 ssd1306_WriteString("Overvoltage", Font_6x8, Black);
//                 ssd1306_UpdateScreen();
//             }
//             else if(FAULT_TYPE == 3){
//                 ssd1306_Fill(White);
//                 ssd1306_SetCursor(2,31);
//                 ssd1306_WriteString("Undervoltage", Font_6x8, Black);
//                 ssd1306_UpdateScreen();
//             }
//             else{
//                 ssd1306_Fill(White);
//                 ssd1306_SetCursor(2,31);
//                 ssd1306_WriteString("Unknown Fault: ", Font_6x8, Black);
//                 integer = (int) FAULT_TYPE;
//                 snprintf(buff, sizeof(buff), "%d", FAULT_TYPE);
//                 ssd1306_WriteString(buff, Font_6x8, Black);
//                 ssd1306_UpdateScreen();
//             }
//         }
//         else{
//                 ssd1306_Fill(White);
//                 ssd1306_SetCursor(2,31);
//                 ssd1306_WriteString("Channel Disabled", Font_6x8, Black);
//                 ssd1306_UpdateScreen();
//         }
//     }
//     else{
//         // ssd1306_SetCursor(2,5);
// 	    // ssd1306_WriteString("PG:  ", Font_6x8, Black);
//         // integer = PG ? 1 : 0;
//         // snprintf(buff, sizeof(buff), "%d", integer);
//         // ssd1306_WriteString(buff, Font_6x8, Black);


//         // ssd1306_SetCursor(70,5);
// 	    // ssd1306_WriteString("Fault:  ", Font_6x8, Black);
//         // integer = FAULT ? 1 : 0;
//         // snprintf(buff, sizeof(buff), "%d", integer);
//         // ssd1306_WriteString(buff, Font_6x8, Black);

//         ssd1306_SetCursor(2,5);
// 	    ssd1306_WriteString("Board ID:  ", Font_6x8, Black);
//         integer = id;
//         snprintf(buff, sizeof(buff), "%d", integer);
//         ssd1306_WriteString(buff, Font_6x8, Black);

//         ssd1306_SetCursor(2,20);
// 	    ssd1306_WriteString("Voltage:", Font_6x8, Black);
// 	    integer = (int) voltage/1000;
// 	    fraction = (int)(voltage - integer*1000);
//         snprintf(buff, sizeof(buff), "%d.%03d V", integer, fraction);
//         ssd1306_WriteString(buff, Font_6x8, Black);

//         // ssd1306_SetCursor(2,20);
// 	    // ssd1306_WriteString("Shunt:", Font_6x8, Black);
// 	    // integer = (int) shunt_voltage/1000;
//         // fraction = (int)(shunt_voltage - integer*1000);
//         // snprintf(buff, sizeof(buff), "%d.%02d V", integer, fraction);
//         // ssd1306_WriteString(buff, Font_6x8, Black);

//         ssd1306_SetCursor(2,30);
// 	    ssd1306_WriteString("Current:", Font_6x8, Black);
// 	    integer = (int) current/1000;
// 	    fraction = (int)(current - integer*1000);
//         snprintf(buff, sizeof(buff), "%d.%03d A", integer, fraction);
//         ssd1306_WriteString(buff, Font_6x8, Black);

//         ssd1306_SetCursor(2,40);
// 	    ssd1306_WriteString("Temperature:", Font_6x8, Black);
// 	    integer = (int) temp/1000;
// 	    fraction = (int)((temp - integer*1000));
//         snprintf(buff, sizeof(buff), "%d.%03d C", integer, fraction);
//         ssd1306_WriteString(buff, Font_6x8, Black);

//         ssd1306_SetCursor(2,50);
// 	    ssd1306_WriteString("OC SETTING:", Font_6x8, Black);
//         integer = oc;
//         snprintf(buff, sizeof(buff), "%d A", integer);
//         ssd1306_WriteString(buff, Font_6x8, Black);

//         ssd1306_UpdateScreen();
//     }
// }

/**
 * @brief Display Turning On Process Screen
 * 
 */
// void ssd1306_DisplayOnMsg() {
// 	ssd1306_Fill(White);
//     ssd1306_SetCursor(2,31);
// 	ssd1306_WriteString("Waiting for Relay PCB Connection ...", Font_6x8, Black);
// 	ssd1306_UpdateScreen();
// 	HAL_Delay(1000);
// }

/**
 * @brief Display Ready Screen
 * 
 */
// void ssd1306_DisplayReadyMsg() {
// 	ssd1306_Fill(White);
//     ssd1306_SetCursor(2,31);
// 	ssd1306_WriteString("Ready to Start Monitoring", Font_6x8, Black);
// 	ssd1306_UpdateScreen();
// 	HAL_Delay(1000);
// }

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
// void ssd1306_DisplayErrorMsg() {
// 	ssd1306_Fill(White);
//     ssd1306_SetCursor(2,31);
// 	ssd1306_WriteString("ERROR: Init Failure", Font_6x8, Black);
// 	ssd1306_UpdateScreen();
// 	HAL_Delay(1000);
// }

/**
 * @brief Display EN Test Screen
 * 
 */
// void ssd1306_DisplayENTestMsg() {
//     ssd1306_Fill(White);
//     ssd1306_SetCursor(2,31);
//     if (EN_TEST){
//         ssd1306_WriteString("TESTING EN: PASS", Font_6x8, Black);
//     }
//     else{
//         ssd1306_WriteString("TESTING EN", Font_6x8, Black);
//     }
// 	ssd1306_UpdateScreen();
// 	HAL_Delay(1000);
// }

/**
 * @brief Display OC Test Screen
 * 
 */
// void ssd1306_DisplayOCTestMsg() {
// 	ssd1306_Fill(White);
//     ssd1306_SetCursor(2,31);
//     if (OC_TEST){
//         ssd1306_WriteString("TESTING OC: PASS", Font_6x8, Black);
//     }
//     else{
//         ssd1306_WriteString("TESTING OC PROTECTION", Font_6x8, Black);
//     }
// 	ssd1306_UpdateScreen();
// 	HAL_Delay(1000);
// }

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
    //FAULT_TYPE = INA228_checkFault(&ina228);
    FAULT_CHECK = 1;
    PG = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
    FAULT = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
}

void user_error_handler()
{
    while (1)
    {
    }
}


// ADD OTHER HELPER FUNCTIONS HERE