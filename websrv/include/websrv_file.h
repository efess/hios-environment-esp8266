#include "user_interface.h"

uint32_t websrv_file_request_data_size(uint8_t *resource);
void websrv_file_request_data_chunk(uint8_t *resource, uint8_t *data, uint16_t buffer_size, uint32_t offset, uint16_t *length);