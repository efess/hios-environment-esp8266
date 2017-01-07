/*
 * wifi.c
 *
 *  Created on: Dec 30, 2014
 *      Author: Minh
 */
#include "wifi.h"
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "mqtt_msg.h"
#include "info.h"
#include "user_config.h"

static uint8_t connectionFailedAttemps = 0;
static ETSTimer WiFiLinker;
WifiCallback wifiCb = NULL;
static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;
static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
  struct ip_info ipConfig;
  os_timer_disarm(&WiFiLinker);
  wifi_get_ip_info(STATION_IF, &ipConfig);
  wifiStatus = wifi_station_get_connect_status();
  if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)
  {
    os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
    os_timer_arm(&WiFiLinker, 2000, 0);
  }
  else
  {
    if(connectionFailedAttemps >= WIFI_FAILED_ATTEMPS_MAX)
    {
      connectionFailedAttemps = 0;
      INFO("Station exceeded connection attemps, starting AP Config mode\r\n");
      WIFI_APMode();
      return;
    }
    else if (wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
    {
      INFO("STATION_WRONG_PASSWORD\r\n");
      wifi_station_connect();
      connectionFailedAttemps++;
    }
    else if (wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
    {
      INFO("STATION_NO_AP_FOUND\r\n");
      wifi_station_connect();
      connectionFailedAttemps++;
    }
    else if (wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
    {
      INFO("STATION_CONNECT_FAIL\r\n");
      wifi_station_connect();
      connectionFailedAttemps++;
    }
    else
    {
      INFO("STATION_IDLE\r\n");
    }

    os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
    os_timer_arm(&WiFiLinker, 500, 0);
  }
  if (wifiStatus != lastWifiStatus) {
    lastWifiStatus = wifiStatus;
    if (wifiCb)
      wifiCb(wifiStatus);
  }
}

void ICACHE_FLASH_ATTR WIFI_Connect(uint8_t* ssid, uint8_t* pass, WifiCallback cb)
{
  struct station_config stationConf;

  INFO("WIFI_INIT Station\r\n");
  wifi_set_opmode_current(STATION_MODE);
  wifiCb = cb;
  os_memset(&stationConf, 0, sizeof(struct station_config));
  os_sprintf(stationConf.ssid, "%s", ssid);
  os_sprintf(stationConf.password, "%s", pass);
  wifi_station_set_config_current(&stationConf);
  os_timer_disarm(&WiFiLinker);
  os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
  os_timer_arm(&WiFiLinker, 1000, 0);
  wifi_station_connect();
}

void ICACHE_FLASH_ATTR WIFI_APMode()
{
  INFO("WIFI_INIT Access Point\r\n");
  wifi_set_opmode(SOFTAP_MODE);
  struct softap_config ap_config;
  os_memset(ap_config, 0, sizeof(struct softap_config));
  os_sprintf(ap_config.ssid, "esp8266_%u", system_get_chip_id());
  ap_config.ssid_len = os_strlen(ap_config.ssid);
  ap_config.channel = 8;
  ap_config.authmode = AUTH_OPEN;
  ap_config.max_connection = 4;
  ap_config.beacon_interval = 100;
  
  wifi_softap_set_config(&ap_config);
}