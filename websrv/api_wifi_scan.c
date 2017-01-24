#include "api_wifi_scan.h"
#include "wifi_scan.h"
#include "mem.h"
#include "jsonh.h"
#include "json/jsontree.h"
#include "json/jsonparse.h"
#include "info.h"

void ICACHE_FLASH_ATTR api_wifi_scan_params(void *context, uint8_t *req_buffer, uint16_t length)
{
    WifiScanParams *params = (WifiScanParams*)context;

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

void ICACHE_FLASH_ATTR api_print_wifi_scan(int (* json_putchar)(int c), uint8_t *params)
{
    WifiScanParams *wifi_params = (WifiScanParams*)params;
    
    struct jsontree_string status;
    uint8_t status_str[14] = {0};
    uint8_t c = 0;  

    status.type = JSON_TYPE_STRING;
    status.value = status_str;

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
    
    INFO("WebSrvAPI: creating json, apcount: %u\r\n", wifi_scan_state.ap_count);
    
    char *ssid = "ssid",
        *strength = "strength",
        *auth = "auth";

    struct jsontree_array ap_list = {
        JSON_TYPE_ARRAY,
        wifi_scan_state.ap_count,
        NULL
    };

    if(wifi_scan_state.ap_count > 0)
    {
        ap_list.values = (struct jsontree_value**)os_zalloc(sizeof(struct jsontree_value*) * wifi_scan_state.ap_count);
        struct jsontree_object *ap_obj = (struct jsontree_object *)json_dynamic_create_object(wifi_scan_state.ap_count);

        for (c = 0; c < wifi_scan_state.ap_count; c++) {

            ApInfo *ap_info = wifi_scan_state.ap_list[c];
            struct jsontree_object *obj = &ap_obj[c];

            obj->type = JSON_TYPE_OBJECT;
            obj->count = 3;
            obj->pairs = (struct jsontree_pair *)json_dynamic_create_pairs(3);
            
            obj->pairs[0].name = strength;
            obj->pairs[0].value = (struct jsontree_value*)json_dynamic_create_int(ap_info->signal);

            obj->pairs[1].name = ssid;
            obj->pairs[1].value = (struct jsontree_value*)json_dynamic_create_string(ap_info->ssid);

            obj->pairs[2].name = auth;
            obj->pairs[2].value = (struct jsontree_value*)json_dynamic_create_int(ap_info->auth);
            
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
    
    jsontree_setup(&context, (struct jsontree_value *) &root, json_putchar);
    
    while (jsontree_print_next(&context)) { }

    if(wifi_scan_state.ap_count > 0) 
    {
        json_dynamic_free_all();
        os_free(ap_list.values);
    }
}
