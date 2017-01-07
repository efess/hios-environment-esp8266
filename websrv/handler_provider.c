#include "websrv_file.h"
#include "resource_handler.h"

void handler_provider_assign_websrv_file(ResourceHandler *handler)
{
    handler->setup_context = websrv_file_setup_context;
    //handler->request_body ;
    handler->response_body_chunk = websrv_file_response_body_chunk;
    handler->response_body_size = websrv_file_response_body_size;
    handler->response_header = websrv_file_response_header;
}

void handler_provider_assign(uint8_t *resource, ResourceHandler *handler)
{
    handler_provider_assign_websrv_file(handler);
}