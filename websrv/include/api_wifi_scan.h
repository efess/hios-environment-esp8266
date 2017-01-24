#include "user_interface.h"

#define API_WIFI_SCAN_BAD_REQUEST -1
#define API_WIFI_SCAN_REFRESH 0
#define API_WIFI_SCAN_LIST 1

typedef struct {
    int8_t request;
} WifiScanParams;

void api_wifi_scan_params(void *context, uint8_t *req_buffer, uint16_t length);
void api_print_wifi_scan(int (* json_putchar)(int c), uint8_t *params);