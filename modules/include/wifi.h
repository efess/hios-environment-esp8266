/*
 * wifi.h
 *
 *  Created on: Dec 30, 2014
 *      Author: Minh
 */

#ifndef USER_WIFI_H_
#define USER_WIFI_H_
#include "os_type.h"

#define WIFI_FAILED_ATTEMPS_MAX 3

//defined in sdk
// enum {
//     STATION_IDLE = 0,
//     STATION_CONNECTING,
//     STATION_WRONG_PASSWORD,
//     STATION_NO_AP_FOUND,
//     STATION_CONNECT_FAIL,
//     STATION_GOT_IP
// };

typedef void (*WifiCallback)(uint8_t);
void WIFI_StationMode();
void WIFI_APMode();
void WIFI_SetStatusCallback(WifiCallback cb);

#endif /* USER_WIFI_H_ */
