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
uint32_t hb_timer = 0; // Timer for heartbeat messages
uint32_t monitor_timer = 0; // Timer for monitoring relay status
uint32_t usb_timer = 0; // Timer for USB status messages

bool PG = true; // Power Good status, true means relay is ON and power is good
bool FAULT = false; // Fault status, true means a fault has been detected
bool FAULT_CHECK = false; // Flag to indicate if fault checking is in progress
uint16_t FAULT_TYPE = 0; // 1 for overcurrent, 2 for overvoltage, 3 for undervoltage, 4 for overtemperature


float current = 0; // Latest current reading
float fault_current = 0; // Latest current reading at the time of fault detection, used for diagnostics
float voltage = 0; // Latest voltage reading
float fault_voltage = 0; // Latest voltage reading at the time of fault detection, used for diagnostics
float temp = 0; // Latest temperature reading
int oc = 0; // Overcurrent threshold setting
float voltage_buf[MAX_RELAYS] = {0}; // Buffer to store latest voltage readings for all relays
float current_buf[MAX_RELAYS] = {0}; // Buffer to store latest current readings for all relays
float temp_buf[MAX_RELAYS] = {0}; // Buffer to store latest temperature readings for all relays
bool PG_buf[MAX_RELAYS] = {0}; // Buffer to store latest Power Good status for all relays
float OC_thresholds_buf[MAX_RELAYS] = {0}; // Buffer to store latest Overcurrent threshold settings for all relays

//CAN Message Variables
FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];

void user_setup()
{
    // ADD SETUP CODE HERE
    relay_count = 0;
    Relay_BeginRefresh(); 
    scan_bus(&hi2c2); // Scan first I2C bus to detect any relays connected to it
    scan_bus(&hi2c3); // Scan second I2C bus to detect any relays connected to it
    Relay_EndRefresh();
    // Relay_CheckPresence();
    HAL_FDCAN_Start(&hfdcan1);
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
}

void user_loop()
{
    /**
     * @brief Every HB_TIMER milliseconds
     * Toggle the onboard LED to indicate the system is alive and optionally send a heartbeat message over CAN or USB. 
     */
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
        char msg[64];
        sprintf(msg, "Heartbeat: System is alive\r\n");
        CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
        HAL_Delay(5); // small delay to avoid USB buffer overflow
    }
    /**
     * @brief Every MONITOR_TIMER milliseconds, 
     * Read the status of all connected relays and store the latest readings in buffers. 
     * These buffers will be used to send status updates over USB in the next section. 
     */
    if (HAL_GetTick() - monitor_timer > MONITOR_TIMER)
    {
        monitor_timer = HAL_GetTick();
        Relay_BeginRefresh();
        scan_bus(&hi2c2);
        scan_bus(&hi2c3);
        Relay_EndRefresh();
        for (uint8_t i = 0; i < MAX_RELAYS; i++)
        {
            if (!relay[i].ready) continue;
            // store latest readings in buffers for all relays, these will be sent over USB in the next section
            voltage_buf[i] = INA228_ReadBusVoltage(&relay[i].ina);
            current_buf[i] = INA228_ReadCurrent(&relay[i].ina, 30);
            temp_buf[i]    = INA228_getTemperature(&relay[i].ina);
            PG_buf[i]      = RELAY_ReadPG(relay[i].slot);
            OC_thresholds_buf[i] = RELAY_ReadOC(relay[i].slot);
        }
    }
    /**
     * @brief Every USB_TIMER milliseconds, 
     * Send the latest status of all relays over USB in a human-readable format. 
     * This includes voltage, current, temperature, Power Good status, and Overcurrent threshold settings for each relay. 
     */
    if (HAL_GetTick() - usb_timer > USB_TIMER) {
        usb_timer = HAL_GetTick();
        for (uint8_t i = 0; i < MAX_RELAYS; i++)
        {
            if (!relay[i].ready) continue;
            float v = voltage_buf[i];
            float c = current_buf[i];
            float t = temp_buf[i];
            uint16_t oc = OC_thresholds_buf[i];

            char msg[128];

            // convert float readings into integer and fractional parts for USB transmission
            int v_i = (int)(v / 1000);
            int v_f = (int)(v - v_i * 1000);

            int c_i = (int)(c / 1000);
            int c_f = (int)(c - c_i * 1000);

            int t_i = (int)(t / 1000);
            int t_f = (int)(t - t_i * 1000);

            snprintf(msg, sizeof(msg),
                "Relay[%d] ADDR:0x%02X\r\n"
                "  PG: %s\r\n"
                "  V: %d.%03d V\r\n"
                "  C: %d.%03d A\r\n"
                "  T: %d.%03d C\r\n"
                "  OC: %d A\r\n\r\n",
                i,
                relay[i].addr,
                PG_buf[i] ? "ON" : "OFF",
                v_i, v_f,
                c_i, c_f,
                t_i, t_f,
                oc
            );
            CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
            HAL_Delay(5); // small delay to avoid USB buffer overflow
        }
    }
}

/** @brief Check the presence of relays on the I2C bus based on a fixed configuration list and send the results over USB.
 *  Future improvement: the list could be dynamically generated based on CAN messages from the PLC
 *  @param void
 *  @retval void
 */
void Relay_CheckPresence(void)
{
    char msg[64];

    sprintf(msg, "\r\n===== RELAY PRESENCE CHECK =====\r\n");
    CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
    HAL_Delay(5);

    for (uint8_t i = 0; i < MAX_RELAYS; i++)
    {
        uint8_t slot = Relay_Configure[i];

        // stop when list ends (0 terminator)
        if (slot == 0)
            break;

        // Was slot discovered during scan?
        if (relay[slot].i2c == NULL)
        {
            sprintf(msg, "relay[%d] is MISSING\r\n", slot);
            CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
            HAL_Delay(5);
            continue;
        }
        else{
            sprintf(msg, "relay[%d] found at I2C 0x%02X\r\n", slot, relay[slot].addr);
            CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
            HAL_Delay(5);
        }
    }

    sprintf(msg, "================================\r\n\r\n");
    CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
    HAL_Delay(5);
}

/** @brief EXTI9_5_IRQHandler - Interrupt handler for EXTI lines 9 to 5
 *  @param void
 *  @retval void
 */
void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
}

/** @brief EXTI0_IRQHandler - Interrupt handler for EXTI line 0
 *  @param void
 *  @retval void
 */

void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/** @brief HAL_GPIO_EXTI_Callback - Callback function for GPIO EXTI interrupts
 *  Future improvement: Implement a similar fault interrupt structure for all relay slots instead of hardcoding for one PG and one FAULT pin
 *  @param GPIO_Pin The GPIO pin that triggered the interrupt.
 *  @retval void
 */
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