#ifndef _RESOURCE_HANDLER_H_
#define _RESOURCE_HANDLER_H_

#include "http.h"


typedef void (* handler_setup_context)(void **context, uint8_t *resource, HttpRequest *request);
typedef void (* handler_request_body)(void *context, uint8_t *resource, uint8_t *data, uint16_t length);
typedef uint16_t (* handler_response_body_chunk)(void *context, uint8_t *resource, uint8_t *data, uint16_t buffer_size);
typedef uint32_t (* handler_response_body_size)(void *context, uint8_t *resource);
typedef void (* handler_response_header)(void *context, uint8_t *resource, HttpResponse *response);
typedef void (* handler_free_context)(void *context);
typedef struct {
    handler_setup_context       setup_context;
    handler_request_body        request_body;
    handler_response_body_chunk response_body_chunk;
    handler_response_body_size  response_body_size ;
    handler_response_header     response_header;
    handler_free_context        free_context;
} ResourceHandler;

#endif