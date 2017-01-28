#include "websrv_api.h"
#include "osapi.h"
#include "http.h"
#include "info.h"
#include "mem.h"
#include "logic.h"
#include "jsonh.h"
#include "json/jsontree.h"
#include "api_wifi_scan.h"
#include "api_setup.h"

static jsontree_buffer json_putchar_buffer;
static int ICACHE_FLASH_ATTR _json_putchar(int c)
{
    if (json_putchar_buffer.pos < JSON_CONTEXT_BUFFER_SIZE) {
        json_putchar_buffer.data[json_putchar_buffer.pos++] = c;
        return c;
    }
    return 0;
}

void ICACHE_FLASH_ATTR websrv_api_print_unknown_action()
{
    JSON_PAIR_STRING(error, "error", "Unknown action");

    JSON_OBJECT(setup_obj, error);

    struct jsontree_context json_context;
    jsontree_setup(&json_context, (struct jsontree_value *) &setup_obj, _json_putchar);

    while(jsontree_print_next(&json_context)) {}
}

void ICACHE_FLASH_ATTR websrv_api_init_putchar_buffer(uint8_t *buffer) 
{
    json_putchar_buffer.data = buffer;
    json_putchar_buffer.pos = 0;
}

void ICACHE_FLASH_ATTR websrv_api_setup_context(void **context, uint8_t *resource, HttpRequest *request)
{
    *context = (void*)os_zalloc(sizeof(WebSrvApiContext));
    WebSrvApiContext *handler_context = (WebSrvApiContext*)*context;

    uint8_t *api_slash = strchr(resource, '/');
    if(request->verb != HTTP_POST) 
    {
        INFO("WebSrvAPI: API should be called with post. Handling anyway..\r\n");
    }       
    if(!api_slash) 
    {
        INFO("WebSrvAPI: No API action found.\r\n");
    }
    
    os_strncpy(handler_context->api_action, (api_slash + 1), MAX_LENGTH_API_ACTION);
}

void ICACHE_FLASH_ATTR websrv_api_request_body(void *context, uint8_t *resource, uint8_t *data, uint16_t length)
{
    WebSrvApiContext *handler_context = (WebSrvApiContext*)context;

    websrv_api_init_putchar_buffer(handler_context->response_data);
    
    if(strcmp(handler_context->api_action, "apscan") == 0)
    {
        api_wifi_scan_params(handler_context->request_params, data, length);
        api_print_wifi_scan(_json_putchar, handler_context->request_params);
    }
    else if(strcmp(handler_context->api_action, "setup") == 0)
    {
        api_setup_request(handler_context->request_params, data, length);
        api_setup_response(_json_putchar, handler_context->request_params);
    }
    else
    {
        websrv_api_print_unknown_action();
    }

    handler_context->length = json_putchar_buffer.pos;
}

uint32_t ICACHE_FLASH_ATTR websrv_api_response_body_size(void *context, uint8_t *resource)
{
    WebSrvApiContext *handler_context = (WebSrvApiContext*)context;
    return handler_context->length;
}

uint16_t ICACHE_FLASH_ATTR websrv_api_response_body_chunk(void *context, uint8_t *resource, uint8_t *data, uint16_t buffer_size)
{
    WebSrvApiContext *handler_context = (WebSrvApiContext*)context;
    uint16_t to_send = min(handler_context->length - handler_context->transfer_progress, buffer_size);

    os_memcpy(data, handler_context->response_data, to_send);
    
    handler_context->transfer_progress += to_send;
    return to_send;
}

void ICACHE_FLASH_ATTR websrv_api_response_header(void *context, uint8_t *resource, HttpResponse *response)
{
    WebSrvApiContext *handler_context = (WebSrvApiContext*)context;
    os_strcpy(response->content_type, "application/json");
    response->length = handler_context->length;
    response->status = 200;
}

void ICACHE_FLASH_ATTR websrv_api_free_context(void *context)
{
    os_free(context);
}