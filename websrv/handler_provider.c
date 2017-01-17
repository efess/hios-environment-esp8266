#include "websrv_file.h"
#include "websrv_api.h"
#include "resource_handler.h"
#include "osapi.h"
#include "info.h"

void ICACHE_FLASH_ATTR handler_provider_assign_websrv_file(ResourceHandler *handler)
{
    handler->setup_context = websrv_file_setup_context;
    //handler->request_body ;
    handler->response_body_chunk = websrv_file_response_body_chunk;
    handler->response_body_size = websrv_file_response_body_size;
    handler->response_header = websrv_file_response_header;
    handler->free_context = websrv_file_free_context;
}

void ICACHE_FLASH_ATTR handler_provider_assign_websrv_api(ResourceHandler *handler)
{
    handler->setup_context = websrv_api_setup_context;
    handler->request_body = websrv_api_request_body;
    handler->response_body_chunk = websrv_api_response_body_chunk;
    handler->response_body_size = websrv_api_response_body_size;
    handler->response_header = websrv_api_response_header;
    handler->free_context = websrv_api_free_context;
}

void ICACHE_FLASH_ATTR handler_provider_assign(uint8_t *resource, ResourceHandler *handler)
{
    uint8_t *slash = strchr(resource, '/');

    if(slash)
    {
        if(os_strncmp(resource, "api", slash-resource) == 0) 
        {
            handler_provider_assign_websrv_api(handler);
            return;
        }
    }

    handler_provider_assign_websrv_file(handler);
}