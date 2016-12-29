#include "upgrade.h"
#include "osapi.h"
#include "user_interface.h"
#include "flashget.h"
#include "info.h"
#include "spi_flash.h"

// some code taken from esp-bootloader - thanks tuanpmt

// calculate checksum for block of data
// from start up to (but excluding) end
static uint8 calc_chksum(uint8 *start, uint8 *end)
{
	uint8 chksum = CHKSUM_INIT;
	while(start < end) {
		chksum ^= *start;
		start++;
	}
	return chksum;
}

uint8 ICACHE_FLASH_ATTR load_boot_cfg(espboot_cfg *cfg)
{
	if (spi_flash_read(BOOT_CONFIG_SECTOR * SECTOR_SIZE, (void*)cfg, sizeof(espboot_cfg)) != 0)
	{
        INFO("Can not read boot configurations\r\n");
        return -1;
	}

	INFO("ESPBOOT: Load boot settings\r\n");
    
    return 0;
}

uint8 ICACHE_FLASH_ATTR save_boot_cfg(espboot_cfg *cfg)
{
	cfg->chksum = calc_chksum((uint8*)cfg, (uint8*)&cfg->chksum);
	if (spi_flash_erase_sector(BOOT_CONFIG_SECTOR) != 0)
	{
		INFO("Can not erase boot configuration sector\r\n");
        return -1;
	}
    if (spi_flash_write(BOOT_CONFIG_SECTOR * SECTOR_SIZE, (void*)cfg, sizeof(espboot_cfg)) != 0)
	{
        INFO("Can not save boot configurations\r\n");
        return -1;
	}
	INFO("ESPBOOT: Save boot settings\r\n");
    return 0;
}

uint8 ICACHE_FLASH_ATTR flag_bootloader_new_rom() 
{
    espboot_cfg cfg;

    if(load_boot_cfg(&cfg) != 0) 
    {
        return -1;
    }

    cfg.new_rom_addr = NEW_ROM_START;

    if(save_boot_cfg(&cfg) != 0) 
    {
        return -1;
    }
}

void ICACHE_FLASH_ATTR flash_download_new_rom_size(uint32_t size, int *err)
{
    uint16_t from_sector = NEW_ROM_START / SECTOR_SIZE;
    uint16_t to_sector = (NEW_ROM_START + size) / SECTOR_SIZE;
    uint16_t counter = from_sector;
    SpiFlashOpResult result;
    
    for(; counter <= to_sector; counter++) 
    {
        result = spi_flash_erase_sector(counter);
        if(result != SPI_FLASH_RESULT_OK) 
        {
            if(result == SPI_FLASH_RESULT_TIMEOUT) 
            {
                INFO("Upgrade: Flash erase timeout\r\n");
            } 
            else 
            {
                INFO("Upgrade: Flash erase error\r\n");
            }
            *err = -1;
            return;
        }
    }
    INFO("Upgrade: Erasing complete\r\n");
    *err = 0;
}

void ICACHE_FLASH_ATTR flash_download_data_chunk(uint8_t *data, uint32_t offset, uint16_t length, int *err)
{
    uint32_t start = NEW_ROM_START + offset;
    SpiFlashOpResult result = spi_flash_write(start, (void*)data, length);
    if(result != SPI_FLASH_RESULT_OK) 
    {
        if(result == SPI_FLASH_RESULT_TIMEOUT) 
        {
            INFO("Upgrade: Flash write timeout\r\n");
        } 
        else 
        {
            INFO("Upgrade: Flash write error at address %u\r\n", start);
        }
        *err = -1;
    }
    *err = 0;
}

void ICACHE_FLASH_ATTR flash_download_complete(uint8 result)
{
    if(result == GET_SUCCESS) 
    {
        if(flag_bootloader_new_rom() == 0)
        {
	        INFO("Upgrade: Upgrade complete. Rebooting.\r\n");
            // Reboot.
            system_restart();
        }
    }
    if(result == GET_FAIL) 
    {
        INFO("Upgrade: Download fail\r\n");
    }
}

void ICACHE_FLASH_ATTR upgrade_available(char* url)
{

    flashget_download(url, 
        flash_download_new_rom_size, 
        flash_download_data_chunk, 
        flash_download_complete);
}
