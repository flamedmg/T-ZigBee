/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <Arduino.h>
#include <zbhci.h>
#include <hci_display.h>
#include <OneButton.h>
#include "esp_task_wdt.h"

#include <Thermistor.h>
#include <NTC_Thermistor.h>
#include <AverageThermistor.h>

#define CONFIG_ZIGBEE_MODULE_PIN 0
#define CONFIG_USR_BUTTON_PIN 2
#define CONFIG_BLUE_LIGHT_PIN 3
#define CONFIG_FLOOR_SENSOR_LEFT_PIN 5
#define CONFIG_FLOOR_SENSOR_RIGHT_PIN 6
#define CONFIG_REFERENCE_RESISTANCE 10000
#define CONFIG_NOMINAL_RESISTANCE 10000 // needs to be measusured
#define CONFIG_NOMINAL_TEMPERATURE 25   // needs to be measusured
#define CONFIG_NTC_READINGS_NUMBER 10
#define CONFIG_NTC_READINGS_DELAY 10
#define CONFIG_ANALOG_RESOLUTION 4095
#define B_VALUE 3590

#define REPORTING_PERIOD 10

Thermistor *leftThermistor = NULL;
Thermistor *rightThermistor = NULL;

const uint8_t au8ManufacturerName[] = {13, 'D', 'm', 'G', '.', 'F', 'l', 'o', 'o', 'r', 'S', 'e', 'n', 's', 'o', 'r'};

QueueHandle_t msg_queue;

/**
 * Initialize a new OneButton instance for a button
 * connected to digital pin 4 and GND, which is active low
 * and uses the internal pull-up resistor.
 */
OneButton btn = OneButton(CONFIG_USR_BUTTON_PIN, // Input pin for the button
                          true,                  // Button is active LOW
                          true);                 // Enable internal pull-up resistor

uint8_t ledState = 0;
uint8_t netState = 0;
uint8_t autoReport = 0;

void handleClick(void)
{
    ts_DstAddr sDstAddr;

    sDstAddr.u16DstAddr = 0x0000;
    if (netState == 1)
    {
        int16_t lt = leftThermistor->readCelsius();
        int16_t rt = rightThermistor->readCelsius();

        Serial.printf("temp-left=%f, temp-right=%f\n", (float)(lt / 100.0), (float)(rt / 100.0));

        zbhci_ZclSendReportCmd(0x02, sDstAddr, 1, 1, 0, 1, 0x0402, 0x0000, ZCL_DATA_TYPE_DATA16, 2, (uint8_t *)&rt);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        zbhci_ZclSendReportCmd(0x02, sDstAddr, 1, 1, 0, 1, 0x0402, 0x0000, ZCL_DATA_TYPE_DATA16, 2, (uint8_t *)&rt);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    else
    {
        Serial.println("Not joined the zigbee network");
    }

    digitalWrite(CONFIG_BLUE_LIGHT_PIN, true);
    delay(2000);
    digitalWrite(CONFIG_BLUE_LIGHT_PIN, false);
}

void reportTask(void *pvParameters)
{
    ts_DstAddr sDstAddr;
    int16_t lt, rt;

    sDstAddr.u16DstAddr = 0x0000;
    while (autoReport)
    {
        digitalWrite(CONFIG_BLUE_LIGHT_PIN, true);

        lt = leftThermistor->readCelsius();
        rt = rightThermistor->readCelsius();

        Serial.printf("temp-left=%f, temp-right=%f\n", (float)(lt / 100.0), (float)(rt / 100.0));
        zbhci_ZclSendReportCmd(0x02, sDstAddr, 1, 1, 0, 1, 0x0402, 0x0000, ZCL_DATA_TYPE_DATA16, 2, (uint8_t *)&rt);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        zbhci_ZclSendReportCmd(0x02, sDstAddr, 1, 1, 0, 1, 0x0402, 0x0000, ZCL_DATA_TYPE_DATA16, 2, (uint8_t *)&rt);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        digitalWrite(CONFIG_BLUE_LIGHT_PIN, false);
        vTaskDelay(REPORTING_PERIOD * 1000 / portTICK_PERIOD_MS);
    }
}

void handleDoubleClick(void)
{
    if (autoReport == 0)
    {
        autoReport = 1;
        xTaskCreatePinnedToCore(
            reportTask,
            "report", // A name just for humans
            4096,     // This stack size can be checked & adjusted by reading the Stack Highwater
            NULL,
            6, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
            NULL,
            ARDUINO_RUNNING_CORE);
    }
    else
    {
        autoReport = 0;
        Serial.println("Stop report task");
        delay(1000);
    }
}

void handleLongPress()
{
    if (netState == 0)
    {
        Serial.println("Joining the zigbee network");
        zbhci_BdbCommissionSteer();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    else if (netState == 1)
    {
        Serial.println("leave the zigbee network");
        zbhci_BdbFactoryReset();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        netState = 0;
    }
}

void zbhciTask(void *pvParameters)
{
    ts_HciMsg sHciMsg;
    ts_DstAddr sDstAddr;

    while (1)
    {
        bzero(&sHciMsg, sizeof(sHciMsg));
        if (xQueueReceive(msg_queue, &sHciMsg, portMAX_DELAY))
        {
            switch (sHciMsg.u16MsgType)
            {
            case ZBHCI_CMD_ACKNOWLEDGE:
                // displayAcknowledg(&sHciMsg.uPayload.sAckPayload);
                break;

            case ZBHCI_CMD_NETWORK_STATE_RSP:
                if (sHciMsg.uPayload.sNetworkStateRspPayloasd.u16NwkAddr == 0x0000)
                {
                    zbhci_BdbFactoryReset();
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    zbhci_NetworkStateReq();
                }
                else if (sHciMsg.uPayload.sNetworkStateRspPayloasd.u16NwkAddr != 0xFFFF)
                {
                    netState = 1;
                }
                break;

            case ZBHCI_CMD_NETWORK_STATE_REPORT:
                netState = 1;
                sDstAddr.u16DstAddr = 0x0000;
                zbhci_ZclSendReportCmd(0x02, sDstAddr, 1, 1, 0, 1, 0x0000, 0x0005, ZCL_DATA_TYPE_CHAR_STR, sizeof(au8ManufacturerName), (uint8_t *)&au8ManufacturerName);
                break;

            case ZBHCI_CMD_ZCL_ONOFF_CMD_RCV:
                /*
                if (sHciMsg.uPayload.sZclOnOffCmdRcvPayload.u8CmdId == 0)
                {
                    digitalWrite(CONFIG_BLUE_LIGHT_PIN, LOW);
                    ledState = 0;
                }
                else if (sHciMsg.uPayload.sZclOnOffCmdRcvPayload.u8CmdId == 1)
                {
                    digitalWrite(CONFIG_BLUE_LIGHT_PIN, HIGH);
                    ledState = 1;
                }
                else if (sHciMsg.uPayload.sZclOnOffCmdRcvPayload.u8CmdId == 2)
                {
                    ledState = !ledState;
                    digitalWrite(CONFIG_BLUE_LIGHT_PIN, ledState);
                }
                zbhci_ZclSendReportCmd(0x02, sDstAddr, 1, 1, 0, 1, 0x0006, 0x0000, ZCL_DATA_TYPE_BOOLEAN, 1, &ledState);
                */
                break;

            default:
                Serial.printf("u16MsgType %d\n", sHciMsg.u16MsgType);
                break;
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(10);
    Serial.printf("Init\n");

    pinMode(CONFIG_ZIGBEE_MODULE_PIN, OUTPUT);
    digitalWrite(CONFIG_ZIGBEE_MODULE_PIN, HIGH);
    delay(500);

    pinMode(CONFIG_BLUE_LIGHT_PIN, OUTPUT);
    digitalWrite(CONFIG_BLUE_LIGHT_PIN, LOW);

    Thermistor *originLeftThermistor = new NTC_Thermistor(
        CONFIG_FLOOR_SENSOR_LEFT_PIN,
        CONFIG_REFERENCE_RESISTANCE,
        CONFIG_NOMINAL_RESISTANCE,
        CONFIG_NOMINAL_TEMPERATURE,
        B_VALUE, CONFIG_ANALOG_RESOLUTION);
    leftThermistor = new AverageThermistor(
        originLeftThermistor,
        CONFIG_NTC_READINGS_NUMBER,
        CONFIG_NTC_READINGS_DELAY);

    Thermistor *originrightThermistor = new NTC_Thermistor(
        CONFIG_FLOOR_SENSOR_RIGHT_PIN,
        CONFIG_REFERENCE_RESISTANCE,
        CONFIG_NOMINAL_RESISTANCE,
        CONFIG_NOMINAL_TEMPERATURE,
        B_VALUE, CONFIG_ANALOG_RESOLUTION);
    rightThermistor = new AverageThermistor(
        originLeftThermistor,
        CONFIG_NTC_READINGS_NUMBER,
        CONFIG_NTC_READINGS_DELAY);

    btn.attachClick(handleClick);
    btn.attachDoubleClick(handleDoubleClick);
    btn.setPressTicks(3000);
    btn.attachLongPressStart(handleLongPress);

    msg_queue = xQueueCreate(10, sizeof(ts_HciMsg));
    zbhci_Init(msg_queue);

    xTaskCreatePinnedToCore(
        zbhciTask,
        "zbhci", // A name just for humans
        4096,    // This stack size can be checked & adjusted by reading the Stack Highwater
        NULL,
        5, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        NULL,
        ARDUINO_RUNNING_CORE);

    // zbhci_BdbFactoryReset();
    delay(100);
    zbhci_NetworkStateReq();
}

void loop()
{
    btn.tick();
}