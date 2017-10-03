/* main.c -- MQTT client example
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "gpio.h"
#include "config.h"
#include "user_interface.h"
#include "mem.h"
#include "websrv.h"
#include "info.h"
#include "upgrade.h"
#include "wifi_scan.h"
#include "sensor_loop.h"
#include "lcd.h"
#include "weather.h"
#define TOPIC_OTA_UPGRADE  "/flash/available"

MQTT_Client *mqttClient;
WebSrv webServer;

static void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status)
{
  if (status == STATION_GOT_IP) {
    mqttClient->host = cfg.mqtt_host;
    mqttClient->port = cfg.mqtt_port;
    MQTT_Connect(mqttClient);
    weather_start();
  } else {
    MQTT_Disconnect(mqttClient);
    weather_stop();
  }
}

static void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args)
{
  MQTT_Client* client = (MQTT_Client*)args;
  INFO("MQTT: Connected\r\n");

  MQTT_Subscribe(client, TOPIC_OTA_UPGRADE, 0);
}

static void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args)
{
  MQTT_Client* client = (MQTT_Client*)args; 
  INFO("MQTT: Disconnected\r\n");
}

static void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args)
{
  MQTT_Client* client = (MQTT_Client*)args;
  INFO("MQTT: Published\r\n");
}

static void ICACHE_FLASH_ATTR mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
  char *topicBuf = (char*)os_zalloc(topic_len + 1),
        *dataBuf = (char*)os_zalloc(data_len + 1);

  MQTT_Client* client = (MQTT_Client*)args;
  os_memcpy(topicBuf, topic, topic_len);
  topicBuf[topic_len] = 0;
  os_memcpy(dataBuf, data, data_len);
  dataBuf[data_len] = 0;
  INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);

  if(strcmp(topicBuf, TOPIC_OTA_UPGRADE) == 0) 
  { 
    INFO("Running upgrade\r\n");
    upgrade_available(dataBuf);
  }

  os_free(topicBuf);
  os_free(dataBuf);
}

void ICACHE_FLASH_ATTR print_info()
{
  INFO("\r\n\r\n[INFO] BOOTUP...\r\n");
  INFO("[INFO] SDK: %s\r\n", system_get_sdk_version());
  INFO("[INFO] Chip ID: %08X\r\n", system_get_chip_id());
  INFO("[INFO] Memory info:\r\n");
  system_print_meminfo();

  INFO("[INFO] -------------------------------------------\n");
  INFO("[INFO] Build time: %s\n", BUID_TIME);
  INFO("[INFO] -------------------------------------------\n");

}

static void ICACHE_FLASH_ATTR app_init(void)
{
  mqttClient = (MQTT_Client*)os_zalloc(sizeof(MQTT_Client));
  config_load();

  uart_init(BIT_RATE_115200, BIT_RATE_115200);
  print_info();

  web_listen(&webServer, 80);

  MQTT_InitConnection(mqttClient, MQTT_HOST, MQTT_PORT, DEFAULT_SECURITY);

  uint8_t client_id[20] = {0};
  os_sprintf(client_id, "ESP_%u", system_get_chip_id());
  if ( !MQTT_InitClient(mqttClient, client_id, MQTT_USER, MQTT_PASS, MQTT_KEEPALIVE, MQTT_CLEAN_SESSION) )
  {
    INFO("Failed to initialize properly. Check MQTT version.\r\n");
    return;
  }
  
  MQTT_InitLWT(mqttClient, "/lwt", "offline", 0, 0);
  MQTT_OnConnected(mqttClient, mqttConnectedCb);
  MQTT_OnDisconnected(mqttClient, mqttDisconnectedCb);
  MQTT_OnPublished(mqttClient, mqttPublishedCb);
  MQTT_OnData(mqttClient, mqttDataCb);
  
  WIFI_SetStatusCallback(wifiConnectCb);
  WIFI_StationMode(wifiConnectCb);

   // wifi scan has to after system init done.
  system_init_done_cb(wifi_start_scan);
  weather_init();
  sensors_init();
  lcd_start();
  INFO("Free heap size: %u\r\n",  system_get_free_heap_size());

}

void user_init(void)
{
  system_init_done_cb(app_init);
}
