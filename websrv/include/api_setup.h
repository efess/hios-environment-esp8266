#include "user_interface.h"

#define API_SETUP_INVALID_TYPE -3
#define API_SETUP_INVALID_DATA -2
#define API_SETUP_BAD_REQUEST -1
#define API_SETUP_GET_DATA 1
#define API_SETUP_SET_DATA 0

typedef struct {
    int8_t request;
} SetupReq;

void api_setup_request(void *context, uint8_t *req_buffer, uint16_t length);
void api_setup_response(int (* json_putchar)(int c), uint8_t *params);