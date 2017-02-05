#include "user_interface.h"
#include "mqtt.h"

#define SENSOR_INTERVAL 5000

void sensors_publisher(MQTT_Client *client);
void sensors_init();
void sensors_capture(void *arg);