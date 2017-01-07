#include "user_interface.h"
#include "http.h"

typedef struct {
    uint32_t found;
    uint32_t file_offset;
    uint32_t file_length;
    uint32_t transfer_progress;
} WebSrvContext;

void websrv_file_setup_context(void *context, uint8_t *resource);
uint32_t websrv_file_response_body_size(void *context, uint8_t *resource);
uint16_t websrv_file_response_body_chunk(void *context, uint8_t *resource, uint8_t *data, uint16_t buffer_size);
void websrv_file_response_header(void *context, uint8_t *resource, HttpResponse *response);

// typedef void (* request_body)(void *context, uint8_t *resource, uint8_t *body, uint16_t length);

// // returns number of bytes in buffer
// typedef uint16_t (* response_body_chunk)(void *context, uint8_t *resource, uint8_t *data, uint16_t buffer_size);

// typedef uint32_t (* response_data_size)(void *context, uint8_t *resource);
// typedef void (* response_header)(void *context, uint8_t *resource, HttpResponse *response);
