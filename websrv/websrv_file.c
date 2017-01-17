#include "user_interface.h"
#include "osapi.h"
#include "websrv_file.h"
#include "string.h"
#include "file.h"
#include "logic.h"
#include "mem.h"

void ICACHE_FLASH_ATTR websrv_get_content_type(uint8_t *ext, uint8_t *content_type)
{    
    if(strcmp(ext, ".htm") == 0 || strcmp(ext, ".html") == 0 )
    {
        strcpy(content_type, "text/html");
        return;
    }
    else if(strcmp(ext, ".js") == 0)
    {
        strcpy(content_type, "application/javascript");
        return;
    }
    else if(strcmp(ext, ".css") == 0)
    {
        strcpy(content_type, "text/css");
        return;
    }
    else if(strcmp(ext, ".jpg") == 0)
    {
        strcpy(content_type, "image/jpeg");
        return;
    }
    else if(strcmp(ext, ".gif") == 0)
    {
        strcpy(content_type, "image/gif");
        return;
    }
    else if(strcmp(ext, ".png") == 0)
    {
        strcpy(content_type, "image/png");
        return;
    }
    strcpy(content_type, "text/plain");
}

// typedef void (* push_data_chunk)(uint8_t *data, uint16_t buffer_size, uint32_t offset, uint16_t length, bool isFinished);
// typedef void (* request_data_chunk)(uint8_t *data, uint16_t buffer_size, uint32_t offset, uint16_t *length, bool isFinished);
void ICACHE_FLASH_ATTR websrv_file_setup_context(void **context, uint8_t *resource, HttpRequest *request)
{
    *context = (void*)os_zalloc(sizeof(WebSrvContext));
    WebSrvContext *websrv_context = (WebSrvContext*)*context;

    EspFileDescriptor desc;
    if(file_find(&desc, resource)) 
    {
        websrv_context->found = true;

        websrv_context->file_offset = desc.offset;
        websrv_context->file_length = desc.length;
        websrv_context->transfer_progress = 0;
    }
    else
    {
        // 404
    }
}

uint32_t ICACHE_FLASH_ATTR websrv_file_response_body_size(void *context, uint8_t *resource)
{
    WebSrvContext *websrv_context = (WebSrvContext*)context;
    return websrv_context->file_length;
}

uint16_t ICACHE_FLASH_ATTR websrv_file_response_body_chunk(void *context, uint8_t *resource, uint8_t *data, uint16_t buffer_size)
{
    WebSrvContext *websrv_context = (WebSrvContext*)context;

    uint16_t to_get = min(websrv_context->file_length - websrv_context->transfer_progress, buffer_size);
    
    EspFileDescriptor desc;
    desc.offset = websrv_context->file_offset;
    desc.length = websrv_context->file_length;

    file_read(&desc, (void*)data, websrv_context->transfer_progress, to_get);

    websrv_context->transfer_progress += to_get;

    return to_get;
}

void ICACHE_FLASH_ATTR websrv_file_response_header(void *context, uint8_t *resource, HttpResponse *response)
{
    WebSrvContext *websrv_context = (WebSrvContext*)context;
    
    if(!websrv_context->found) 
    {
        response->status = 404;
    }
    else
    {
        response->status = 200;
        response->length = websrv_context->file_length;
        
        uint8_t *ext = strrchr(resource, '.');

        websrv_get_content_type(ext, response->content_type);
    }
}

void ICACHE_FLASH_ATTR websrv_file_free_context(void *context)
{
    os_free(context);
}