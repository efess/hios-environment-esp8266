#include "user_interface.h"
#include "http.h"

typedef struct {
    uint32_t found;
    uint32_t file_offset;
    uint32_t file_length;
    uint32_t transfer_progress;
} WebSrvContext;

void websrv_file_setup_context(void **context, uint8_t *resource, HttpRequest *request);
uint32_t websrv_file_response_body_size(void *context, uint8_t *resource);
uint16_t websrv_file_response_body_chunk(void *context, uint8_t *resource, uint8_t *data, uint16_t buffer_size);
void websrv_file_response_header(void *context, uint8_t *resource, HttpResponse *response);
void websrv_file_free_context(void *context);
