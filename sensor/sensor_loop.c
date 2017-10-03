#include "sensor_loop.h"
#include "i2c_bme280.h"
#include "mqtt.h"
#include "info.h"
#include "jsonh.h"
#include "run_state.h"
#include "mqttclient.h"

#define MQTT_TOPIC_ENV "/device/%u/environment"

static ETSTimer _loopTimer;

void ICACHE_FLASH_ATTR print_bme280()
{
    BME280_readSensorData();

    // signed long int temp = BME280_GetTemperature();
    // unsigned long int pres = BME280_GetPressure();
    // unsigned long int humid = BME280_GetHumidity();
    
    // INFO("TEMP: %d\r\n", temp);
    // INFO("PRES: %u\r\n", pres);
    // INFO("HUMID: %u\r\n", humid);
    
    run_state.temp = BME280_GetTemperature() / 100;
    run_state.pressure = BME280_GetPressure();
    run_state.humidity = BME280_GetHumidity() / 1000    ;

    if(!mqttClient) {
        return;
    }

    uint8_t json_buffer[128] = {0};
    uint8_t topic[50] = {0};

    json_init_putchar_buffer(json_buffer, sizeof(json_buffer));

    JSON_PAIR_INT(temp, "temp", run_state.temp);
    JSON_PAIR_INT(pres, "pres", run_state.pressure);
    JSON_PAIR_INT(humid, "humid", run_state.humidity);

    JSON_OBJECT(dataz, temp, pres, humid);

    struct jsontree_context json_context;
    jsontree_setup(&json_context, (struct jsontree_value *) &dataz, json_get_putchar());
    
    while(jsontree_print_next(&json_context)) {}

    os_sprintf(topic, MQTT_TOPIC_ENV, system_get_chip_id());
    
    MQTT_Publish(mqttClient, topic, json_buffer, json_get_buffer_length() + 1, 0, 0);
}

void ICACHE_FLASH_ATTR sensors_init()
{
    BME280_Init(BME280_MODE_FORCED);

    os_timer_setfn(&_loopTimer, (os_timer_func_t *)sensors_capture, NULL);
    os_timer_arm(&_loopTimer, SENSOR_INTERVAL, 0);
}

void ICACHE_FLASH_ATTR sensors_capture(void *arg)
{
    os_timer_disarm(&_loopTimer);

    print_bme280();

    os_timer_setfn(&_loopTimer, (os_timer_func_t *)sensors_capture, NULL);
    os_timer_arm(&_loopTimer, SENSOR_INTERVAL, 0);
}