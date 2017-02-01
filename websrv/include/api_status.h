#ifndef API_STATUS_H_
#define API_STATUS_H_

#include "user_interface.h"

void api_status_request(void *context, uint8_t *req_buffer, uint16_t length);
void api_status_response(int (* json_putchar)(int c), uint8_t *params);

#endif