#include "user_interface.h"
#include "osapi.h"
#include "file.h"
#include "info.h"
#include "spi_flash.h"

uint8_t file_find(EspFileDescriptor *descriptor, uint8_t *name)
{
    EspFileDescriptor desc_stg;
    
    uint16_t desc_size = sizeof(EspFileDescriptor);
    uint32_t fileCount = 0;
    uint16_t c = 0;

    spi_flash_read(FILE_FLASH_START, (void*)&fileCount, 4);
    
    for(c = 0; c < fileCount; c++)
    {
        spi_flash_read(FILE_FLASH_INDEX_START + (desc_size * c), (void*)&desc_stg, desc_size);
        if(strcmp(desc_stg.name, name) == 0)
        {
            *descriptor = desc_stg;
            return 1;
        }
    }

    return 0;
}

void file_read(EspFileDescriptor *descriptor, uint8_t *buffer, uint32_t offset, uint32_t length)
{
    uint32_t addr_start = FILE_FLASH_START + descriptor->offset + offset;
    
    spi_flash_read(addr_start, (void*)buffer, length);
}