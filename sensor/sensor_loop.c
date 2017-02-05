#include "sensor_loop.h"
#include "i2c_bme280.h"
#include "mqtt.h"
#include "info.h"
#include "jsonh.h"

#define MQTT_TOPIC_ENV "/device/%u/environment"

static ETSTimer _loopTimer;
MQTT_Client *mqttClient;

void print_bme280()
{
    BME280_readSensorData();

    // signed long int temp = BME280_GetTemperature();
    // unsigned long int pres = BME280_GetPressure();
    // unsigned long int humid = BME280_GetHumidity();
    
    // INFO("TEMP: %d\r\n", temp);
    // INFO("PRES: %u\r\n", pres);
    // INFO("HUMID: %u\r\n", humid);
    
    if(!mqttClient) {
        return;
    }

    uint8_t json_buffer[128] = {0};
    uint8_t topic[50] = {0};

    json_init_putchar_buffer(json_buffer, sizeof(json_buffer));

    JSON_PAIR_INT(temp, "temp", BME280_GetTemperature());
    JSON_PAIR_INT(pres, "pres", BME280_GetPressure());
    JSON_PAIR_INT(humid, "humid", BME280_GetHumidity());

    JSON_OBJECT(dataz, temp, pres, humid);

    struct jsontree_context json_context;
    jsontree_setup(&json_context, (struct jsontree_value *) &dataz, json_get_putchar());
    
    while(jsontree_print_next(&json_context)) {}

    os_sprintf(topic, MQTT_TOPIC_ENV, system_get_chip_id());
    MQTT_Publish(mqttClient, topic, json_buffer, json_get_buffer_length(), 0, 0);
}

void sensors_publisher(MQTT_Client *client)
{
    mqttClient = client;
}

void sensors_init(MQTT_Client *client)
{
    BME280_Init(BME280_MODE_FORCED);

    os_timer_setfn(&_loopTimer, (os_timer_func_t *)sensors_capture, NULL);
    os_timer_arm(&_loopTimer, SENSOR_INTERVAL, 0);
}

void sensors_capture(void *arg)
{
    os_timer_disarm(&_loopTimer);

    print_bme280();

    os_timer_setfn(&_loopTimer, (os_timer_func_t *)sensors_capture, NULL);
    os_timer_arm(&_loopTimer, SENSOR_INTERVAL, 0);
}