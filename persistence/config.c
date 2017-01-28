#include "config.h"
#include "spi_flash.h"
#include "osapi.h"
#include "info.h"

Config cfg;

void ICACHE_FLASH_ATTR config_save() 
{
    ConfigHeader header;

    spi_flash_read(CONFIG_FLASH_HEADER, (void*)&header, sizeof(ConfigHeader));
    uint32_t save_location = header.active_sector == 1 ? 
        CONFIG_FLASH_STARTB : CONFIG_FLASH_STARTA;

    if(spi_flash_erase_sector(save_location / SECTOR_SIZE) != SPI_FLASH_RESULT_OK)
    {
        INFO("Config: Failure erasing sector for re-write\r\n");
        return;
    }

    spi_flash_write(save_location, (void *)&cfg, sizeof(cfg));

    // activate new config
    header.active_sector = header.active_sector == 1 ? 0 : 1;
    spi_flash_write(CONFIG_FLASH_HEADER, (void*)&header, sizeof(ConfigHeader));
}

void ICACHE_FLASH_ATTR config_load()
{
    ConfigHeader header;

    spi_flash_read(CONFIG_FLASH_HEADER, (void*)&header, sizeof(ConfigHeader));
    uint32_t save_location = header.active_sector == 1 ? 
        CONFIG_FLASH_STARTB : CONFIG_FLASH_STARTA;

    spi_flash_read(save_location, (void *)&cfg, sizeof(cfg));
    INFO("CONFIG Load SSID: %s\r\n", cfg.wifi_ssid);
}