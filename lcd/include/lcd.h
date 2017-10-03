#include "user_interface.h"

void lcd_start(void);

typedef struct __attribute__((packed)) {
	uint16_t start;
	uint32_t size;
	uint32_t reserved;
	uint32_t data_offset;
} BmpHeader;