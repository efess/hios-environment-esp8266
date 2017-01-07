#include "user_interface.h"

#define SECTOR_SIZE         0x1000

#define CONFIG_FLASH_HEADER 0x100000
#define CONFIG_FLASH_STARTA 0x101000
#define CONFIG_FLASH_STARTB 0x102000 // end is 0x101FFF 

typedef struct {
    uint8_t wifi_ssid[32];
    uint8_t wifi_pass[32];
    uint8_t wifi_security;

    uint8_t mqtt_host[32];
    uint16_t mqtt_port;
} Config;

typedef struct {
    uint8_t active_sector;
    uint8_t padding[3];
} ConfigHeader;

void config_save();
void config_load();