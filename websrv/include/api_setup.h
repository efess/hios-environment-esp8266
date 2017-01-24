
#define API_WIFI_SCAN_BAD_REQUEST -1
#define API_WIFI_SCAN_REFRESH 0
#define API_WIFI_SCAN_LIST 1

typedef struct {
    int8_t request;
} SetupReq;

void api_setup_request(void *context, uint8_t *req_buffer, uint16_t length);
void api_setup_response(int (* json_putchar)(int c), uint8_t *params);