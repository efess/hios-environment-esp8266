#include "websrv_api.h"
#include "osapi.h"
#include "http.h"
#include "info.h"
#include "mem.h"
#include "wifi_scan.h"
#include "logic.h"
#include "json/jsontree.h"
#include "json/jsonparse.h"

static jsontree_buffer json_putchar_buffer;
static int ICACHE_FLASH_ATTR _json_putchar(int c)
{
    if (json_putchar_buffer.pos < JSON_CONTEXT_BUFFER_SIZE) {
        json_putchar_buffer.data[json_putchar_buffer.pos++] = c;
        return c;
    }
    return 0;
}

void ICACHE_FLASH_ATTR websrv_api_init_putchar_buffer(uint8_t *buffer) 
{
    json_putchar_buffer.data = buffer;
    json_putchar_buffer.pos = 0;
}

uint16_t ICACHE_FLASH_ATTR websrv_api_print_wifi_scan(uint8_t *buffer, uint8_t *params)
{
    INFO("WebSrvAPI: printing\r\n");
    WifiScanParams *wifi_params = (WifiScanParams*)params;

    websrv_api_init_putchar_buffer(buffer);
    
    struct jsontree_string status;
    uint8_t status_str[14] = {0};
    uint8_t c = 0;

    status.type = JSON_TYPE_STRING;
    status.value = status_str;

    struct jsontree_array ap_list = {
        JSON_TYPE_ARRAY,
        wifi_scan_state.ap_count,
        NULL
    };

    if(wifi_params->request == API_WIFI_SCAN_BAD_REQUEST)
    {
        os_strcpy(status_str, "BAD REQUEST");
    }
    else if(wifi_params->request == API_WIFI_SCAN_REFRESH)
    {
        if(wifi_scan_state.ap_scan_status != SCANNING) 
        {
            wifi_start_scan();
        }
        os_strcpy(status_str, "SCANNING");
    }
    else if(wifi_scan_state.ap_scan_status == SCAN_FINISHED) 
    {
        os_strcpy(status_str, "FINISHED");
    }
    else if(wifi_scan_state.ap_scan_status == SCANNING) 
    {
        os_strcpy(status_str, "SCANNING");
    }
    else if(wifi_scan_state.ap_scan_status == NOT_SCANNED) 
    {
        os_strcpy(status_str, "NOT SCANNED");
    } 
    else if(wifi_scan_state.ap_scan_status == SCAN_ERROR)
    {
        os_strcpy(status_str, "SCAN ERROR");
    }

    if(wifi_scan_state.ap_count > 0)
    {
        ap_list.values = (struct jsontree_value**)os_zalloc(sizeof(struct jsontree_value*) * wifi_scan_state.ap_count);
        struct jsontree_object *ap_obj = (struct jsontree_object *)json_dynamic_create_object(wifi_scan_state.ap_count);

        for (c = 0; c < wifi_scan_state.ap_count; c++) {
            ApInfo *ap_info = wifi_scan_state.ap_list[c];
            struct jsontree_object *obj = &ap_obj[c];

            obj->count = 2;
            obj->pairs = (struct jsontree_pair *)json_dynamic_create_pairs(2);
            
            obj->type = JSON_TYPE_OBJECT;
            obj->count = 2;
            obj->pairs[0].name = "strength";
            obj->pairs[0].value = (struct jsontree_value*)json_dynamic_create_int(ap_info->signal);

            obj->pairs[1].name = "ssid";
            obj->pairs[1].value = (struct jsontree_value*)json_dynamic_create_string(ap_info->ssid);

            ap_list.values[c] = (struct jsontree_value*)obj;
        }
    }

    struct jsontree_pair p_status = JSONTREE_PAIR("status", &status);
    struct jsontree_pair p_list = JSONTREE_PAIR("apList", &ap_list);
    
    struct jsontree_pair jsontree_pair_root[] = {p_status, p_list};
    struct jsontree_object root = {
        JSON_TYPE_OBJECT,
        2,
        jsontree_pair_root
    };

    struct jsontree_context context;
    
    jsontree_setup(&context, (struct jsontree_value *) &root, _json_putchar);
    
    uint8_t counter = 0;
    
    while (jsontree_print_next(&context)) { }

    if(wifi_scan_state.ap_count > 0) 
    {
        os_free(ap_list.values);
        json_dynamic_free_all();
    }

    return json_putchar_buffer.pos;
}

void ICACHE_FLASH_ATTR websrv_api_wifi_scan_params(WebSrvApiContext *context, uint8_t *req_buffer, uint16_t length)
{
    WifiScanParams *params = (WifiScanParams*)context->request_params;

    uint8_t value[20] = {0};
    int8_t err = 0;

    struct jsonparse_state state;
    
    jsonparse_setup(&state, req_buffer, length);
    jsonparse_next(&state);
    
    if(!state.error) 
    {
        json_find_next_sibling_string(&state, (uint8_t*)"aplist", value, 20, &err);
        if (err == 0)
        {
            if(strcmp("refresh", value) == 0)
            {
                params->request = API_WIFI_SCAN_REFRESH;
                return;
            }
            else if (strcmp("list", value) == 0)
            {
                params->request = API_WIFI_SCAN_LIST;
                return;
            }
        } 
    }

    params->request = API_WIFI_SCAN_BAD_REQUEST;
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
 
    if(strcmp(handler_context->api_action, "apscan") == 0)
    {
        websrv_api_wifi_scan_params(handler_context, data, length);
        handler_context->length = websrv_api_print_wifi_scan(handler_context->response_data, handler_context->request_params);
    }
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