#ifndef UPGRADE_H_
#define UPGRADE_H_

#define SECTOR_SIZE 							0x1000
#define BOOT_CONFIG_SECTOR 				        1
#define BOOT_CONFIG_MAGIC 				        0xF01A

#define CHKSUM_INIT 							0xEF
#define UPGRADE_BOOT_CFG_START                  0x20000

#define NEW_ROM_START                           0x200000

#include "user_interface.h"
#include "mqtt.h"

typedef struct {
	uint32 magic;
	uint32 app_rom_addr;
	uint32 new_rom_addr;
	uint32 backup_rom_addr;
	uint8 chksum;
} espboot_cfg;


void ICACHE_FLASH_ATTR upgrade_subscribe(MQTT_Client* client);

#endif /* UPGRADE_H_ */
