#include "user_interface.h"
#include "http.h"

#define MAX_LENGTH_API_ACTION 16
#define MAX_LENGTH_API_PARAMS 16
#define JSON_CONTEXT_BUFFER_SIZE 4096

#define API_WIFI_SCAN_BAD_REQUEST -1
#define API_WIFI_SCAN_REFRESH 0
#define API_WIFI_SCAN_LIST 1

typedef struct {
    int8_t request;
} WifiScanParams;

typedef struct {
    uint8_t *data;
    uint16_t pos;
} jsontree_buffer;

typedef struct {
    uint8_t response_data[JSON_CONTEXT_BUFFER_SIZE];
    uint8_t api_action[MAX_LENGTH_API_ACTION];
    uint8_t request_params[MAX_LENGTH_API_PARAMS];
    
    uint32_t length;
    uint32_t transfer_progress;
} WebSrvApiContext;

void websrv_api_setup_context(void **context, uint8_t *resource, HttpRequest *request);
void websrv_api_request_body(void *context, uint8_t *resource, uint8_t *data, uint16_t length);
uint32_t websrv_api_response_body_size(void *context, uint8_t *resource);
uint16_t websrv_api_response_body_chunk(void *context, uint8_t *resource, uint8_t *data, uint16_t buffer_size);
void websrv_api_response_header(void *context, uint8_t *resource, HttpResponse *response);
void websrv_api_free_context(void *context);