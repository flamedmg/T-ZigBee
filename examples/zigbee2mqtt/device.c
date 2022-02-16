
/******************************************************************************/
/***        include files                                                   ***/
/******************************************************************************/

#include "device.h"
#include "app_mqtt.h"

#include "cJSON.h"
#include "esp_log.h"

#include <stdio.h>
#include <string.h>

/******************************************************************************/
/***        macro definitions                                               ***/
/******************************************************************************/

/******************************************************************************/
/***        type definitions                                                ***/
/******************************************************************************/

/******************************************************************************/
/***        local function prototypes                                       ***/
/******************************************************************************/

static void sub_humidity(uint64_t u64IeeeAddr, cJSON *json);

static void sub_temperature(uint64_t u64IeeeAddr, cJSON *json);

static void sub_pressure(uint64_t u64IeeeAddr, cJSON *json);

static void sub_occupancy(uint64_t u64IeeeAddr, cJSON *json);

static void sub_illuminance_lux(uint64_t u64IeeeAddr, cJSON *json);

/******************************************************************************/
/***        exported variables                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        local variables                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        exported functions                                              ***/
/******************************************************************************/

void wsdcgq11lm_add(uint64_t u64IeeeAddr)
{
    char ieeeaddr_str[20] = { 0 };
    cJSON *json = cJSON_CreateObject();
    cJSON *device = cJSON_CreateObject();
    cJSON *identifiers = cJSON_CreateArray();

    if (!json) return ;
    if (!device) goto OUT;
    if (!identifiers) goto OUT1;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddItemToArray(identifiers, cJSON_CreateString(ieeeaddr_str));
    cJSON_AddItemToObject(device, "identifiers", identifiers);
    cJSON_AddStringToObject(device, "manufacturer", "Xiaomi");
    cJSON_AddStringToObject(device, "model", "Aqara temperature, humidity and pressure sensor (WSDCGQ11LM)");
    cJSON_AddStringToObject(device, "name", ieeeaddr_str);
    cJSON_AddStringToObject(device, "sw_version", "3000-0001");
    cJSON_AddItemToObject(json, "device", device);

    cJSON *json_humidity = cJSON_Duplicate(json, 1);
    if (json_humidity)
    {
        sub_humidity(u64IeeeAddr, json_humidity);
        cJSON_Delete(json_humidity);
    }
    cJSON *json_temperature = cJSON_Duplicate(json, 1);
    if (json_humidity)
    {
        sub_temperature(u64IeeeAddr, json_temperature);
        cJSON_Delete(json_temperature);
    }
    cJSON *json_pressure = cJSON_Duplicate(json, 1);
    if (json_humidity)
    {
        sub_pressure(u64IeeeAddr, json_pressure);
        cJSON_Delete(json_pressure);
    }
    ESP_LOGI("Zigbee2MQTT", "Successfully interviewed '%#016llx', device has successfully been paired", u64IeeeAddr);
OUT:
    cJSON_Delete(json);
    return;
OUT1:
    cJSON_Delete(json);
    cJSON_Delete(device);
}


void wsdcgq11lm_delete(uint64_t u64IeeeAddr)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/sensor/%s/humidity/config", ieeeaddr_str);
    app_mqtt_client_publish(topic ,"");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/sensor/%s/temperature/config", ieeeaddr_str);
    app_mqtt_client_publish(topic ,"");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/sensor/%s/pressure/config", ieeeaddr_str);
    app_mqtt_client_publish(topic ,"");
}


void wsdcgq11lm_report(uint64_t u64IeeeAddr,
                       int16_t  i16Temperature,
                       int16_t  i16Humidity,
                       int16_t  i16Pressure)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    cJSON *json = cJSON_CreateObject();
    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);

    cJSON_AddNumberToObject(json, "temperature", (double)i16Temperature/100);
    cJSON_AddNumberToObject(json, "humidity",    (double)i16Humidity/100);
    cJSON_AddNumberToObject(json, "pressure",    (double)i16Pressure);
    char *str = cJSON_Print(json);

    app_mqtt_client_publish(topic ,str);
    cJSON_Delete(json);
}


void rtcgq11lm_add(uint64_t u64IeeeAddr)
{
    char ieeeaddr_str[20] = { 0 };
    cJSON *json = cJSON_CreateObject();
    cJSON *device = cJSON_CreateObject();
    cJSON *identifiers = cJSON_CreateArray();

    if (!json) return ;
    if (!device) goto OUT;
    if (!identifiers) goto OUT1;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddItemToArray(identifiers, cJSON_CreateString(ieeeaddr_str));
    cJSON_AddItemToObject(device, "identifiers", identifiers);
    cJSON_AddStringToObject(device, "manufacturer", "Xiaomi");
    cJSON_AddStringToObject(device, "model", "Aqara human body movement and illuminance sensor (RTCGQ11LM)");
    cJSON_AddStringToObject(device, "name", ieeeaddr_str);
    cJSON_AddStringToObject(device, "sw_version", "3000-0001");
    cJSON_AddItemToObject(json, "device", device);

    cJSON *json_illuminance_lux = cJSON_Duplicate(json, 1);
    if (json_illuminance_lux)
    {
        sub_illuminance_lux(u64IeeeAddr, json_illuminance_lux);
        cJSON_Delete(json_illuminance_lux);
    }
    cJSON *json_occupancy = cJSON_Duplicate(json, 1);
    if (json_occupancy)
    {
        sub_occupancy(u64IeeeAddr, json_occupancy);
        cJSON_Delete(json_occupancy);
    }

    ESP_LOGI("Zigbee2MQTT", "Successfully interviewed '%#016llx', device has successfully been paired", u64IeeeAddr);
OUT:
    cJSON_Delete(json);
    return;
OUT1:
    cJSON_Delete(json);
    cJSON_Delete(device);
}


void rtcgq11lm_delete(uint64_t u64IeeeAddr)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/sensor/%s/illuminance_lux/config", ieeeaddr_str);
    app_mqtt_client_publish(topic ,"");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/binary_sensor/%s/occupancy/config", ieeeaddr_str);
    app_mqtt_client_publish(topic ,"");
}


void rtcgq11lm_report(uint64_t u64IeeeAddr,
                      int8_t   u8Occupancy,
                      uint16_t u16Illuminance)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    cJSON *json = cJSON_CreateObject();
    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);

    cJSON_AddBoolToObject(json, "occupancy", u8Occupancy);
    cJSON_AddNumberToObject(json, "illuminance", u16Illuminance);
    char *str = cJSON_Print(json);

    app_mqtt_client_publish(topic ,str);
    cJSON_Delete(json);
}

/******************************************************************************/
/***        local functions                                                 ***/
/******************************************************************************/

static void sub_humidity(uint64_t u64IeeeAddr, cJSON *json)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    if (!json) return ;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddStringToObject(json, "device_class", "humidity");
    cJSON_AddBoolToObject(json, "enabled_by_default", 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s %s", ieeeaddr_str, "humidity");
    cJSON_AddStringToObject(json, "name", topic);

    cJSON_AddStringToObject(json, "state_class", "measurement");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_%s_zigbee2mqtt", ieeeaddr_str, "humidity");
    cJSON_AddStringToObject(json, "unique_id", topic);

    cJSON_AddStringToObject(json, "unit_of_measurement", "\%");
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.humidity }}");

    char *str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/sensor/%s/humidity/config", ieeeaddr_str);

    // push message
    app_mqtt_client_publish(topic ,str);
}


static void sub_temperature(uint64_t u64IeeeAddr, cJSON *json)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    if (!json) return ;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddStringToObject(json, "device_class", "temperature");
    cJSON_AddBoolToObject(json, "enabled_by_default", 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s %s", ieeeaddr_str, "temperature");
    cJSON_AddStringToObject(json, "name", topic);

    cJSON_AddStringToObject(json, "state_class", "measurement");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_%s_zigbee2mqtt", ieeeaddr_str, "temperature");
    cJSON_AddStringToObject(json, "unique_id", topic);

    cJSON_AddStringToObject(json, "unit_of_measurement", "°C");
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.temperature }}");

    char *str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/sensor/%s/temperature/config", ieeeaddr_str);

    // push message
    app_mqtt_client_publish(topic ,str);
}


static void sub_pressure(uint64_t u64IeeeAddr, cJSON *json)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    if (!json) return ;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddStringToObject(json, "device_class", "pressure");
    cJSON_AddBoolToObject(json, "enabled_by_default", 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s %s", ieeeaddr_str, "pressure");
    cJSON_AddStringToObject(json, "name", topic);

    cJSON_AddStringToObject(json, "state_class", "measurement");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_%s_zigbee2mqtt", ieeeaddr_str, "pressure");
    cJSON_AddStringToObject(json, "unique_id", topic);

    cJSON_AddStringToObject(json, "unit_of_measurement", "hPa");
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.pressure }}");

    char *str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/sensor/%s/pressure/config", ieeeaddr_str);

    // push message
    app_mqtt_client_publish(topic ,str);
}


static void sub_occupancy(uint64_t u64IeeeAddr, cJSON *json)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    if (!json) return ;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddStringToObject(json, "device_class", "motion");
    cJSON_AddBoolToObject(json, "enabled_by_default", 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s %s", ieeeaddr_str, "occupancy");
    cJSON_AddStringToObject(json, "name", topic);

    cJSON_AddBoolToObject(json, "payload_off", false);
    cJSON_AddBoolToObject(json, "payload_on", true);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_%s_zigbee2mqtt", ieeeaddr_str, "pressure");
    cJSON_AddStringToObject(json, "unique_id", topic);

    cJSON_AddStringToObject(json, "unit_of_measurement", "hPa");
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.occupancy }}");

    char *str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/binary_sensor/%s/pressure/config", ieeeaddr_str);

    // push message
    app_mqtt_client_publish(topic ,str);
}


static void sub_illuminance_lux(uint64_t u64IeeeAddr, cJSON *json)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    if (!json) return ;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddStringToObject(json, "device_class", "illuminance");
    cJSON_AddBoolToObject(json, "enabled_by_default", 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s illuminance lux", ieeeaddr_str);
    cJSON_AddStringToObject(json, "name", topic);

    cJSON_AddStringToObject(json, "state_class", "measurement");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_%s_zigbee2mqtt", ieeeaddr_str, "illuminance_lux");
    cJSON_AddStringToObject(json, "unique_id", topic);

    cJSON_AddStringToObject(json, "unit_of_measurement", "lx");
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.illuminance }}");

    char *str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/sensor/%s/illuminance_lux/config", ieeeaddr_str);

    // push message
    app_mqtt_client_publish(topic ,str);
}


/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/