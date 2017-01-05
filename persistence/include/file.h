#include "user_interface.h"

#define FILE_FLASH_START 0x102000
#define FILE_FLASH_INDEX_START  FILE_FLASH_START + 0x4
#define FILE_PATH_LENGTH 56

typedef struct {
    uint8_t name[FILE_PATH_LENGTH];
    uint32_t offset;
    uint32_t length;
} EspFileDescriptor;

uint8_t file_find(EspFileDescriptor *descriptor, uint8_t *name);
void file_read(EspFileDescriptor *descriptor, uint8_t *buffer, uint32_t offset, uint32_t length);