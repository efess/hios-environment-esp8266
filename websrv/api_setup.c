#include "api_setup.h"
#include "mem.h"
#include "jsonh.h"
#include "info.h"
#include "config.h"

void ICACHE_FLASH_ATTR print_api_bad_response(json_putchar putchar)
{
    JSON_PAIR_STRING(error, "error", "bad request");

    JSON_OBJECT(setup_obj, error);

    struct jsontree_context json_context;
    jsontree_setup(&json_context, (struct jsontree_value *) &setup_obj, putchar);

    while(jsontree_print_next(&json_context)) {}
}

void ICACHE_FLASH_ATTR print_api_set_response(json_putchar putchar)
{
    JSON_PAIR_STRING(set, "set", "success");

    JSON_OBJECT(setup_obj, set);

    struct jsontree_context json_context;
    jsontree_setup(&json_context, (struct jsontree_value *) &setup_obj, putchar);

    while(jsontree_print_next(&json_context)) {}
}

void ICACHE_FLASH_ATTR print_api_get_response(json_putchar putchar)
{
    JSON_PAIR_STRING(ssid, "ssid", cfg.wifi_ssid);
    JSON_PAIR_STRING(mqtt_host, "mqttHost", cfg.mqtt_host);
    JSON_PAIR_INT(mqtt_port, "mqttPort", cfg.mqtt_port);

    JSON_OBJECT(setup_obj, ssid, mqtt_host, mqtt_port);

    struct jsontree_context json_context;
    jsontree_setup(&json_context, (struct jsontree_value *) &setup_obj, putchar);

    while(jsontree_print_next(&json_context)) {}
}

void ICACHE_FLASH_ATTR parse_api_setup_data(struct jsonparse_state *json_state, int8_t *err)
{
    uint8_t value[33] = {0};
    *err = 0;

    INFO("API_SETUP 0\r\n");
    if(json_find_next_sibling(json_state, "setup", json_state) != JSON_OK)
    {
        *err = API_SETUP_BAD_REQUEST;
        return;
    }

    jsonparse_next(json_state);
    if(*err)
    {
        INFO("err: %d\r\n", err);

        *err = API_SETUP_BAD_REQUEST;
        return;
    }

    INFO("API_SETUP 2\r\n");
    os_strcpy(cfg.wifi_ssid, value);

    json_find_next_sibling_string(json_state, (uint8_t*)"password", value, sizeof(value), err);
    if(*err)
    {
        *err = API_SETUP_BAD_REQUEST;
        return;
    }

    os_strcpy(cfg.wifi_pass, value);
    
    json_find_next_sibling_string(json_state, (uint8_t*)"mqttHost", value, sizeof(value), err);
    if(*err)
    {
        *err = API_SETUP_BAD_REQUEST;
        return;
    }

    INFO("API_SETUP 3\r\n");
    os_strcpy(cfg.mqtt_host, value);
    
    json_find_next_sibling_int(json_state, (uint8_t*)"mqttPort", (int*)&cfg.mqtt_port, err);
    if(*err)
    {
        *err = API_SETUP_BAD_REQUEST;
        return;
    }
    
    INFO("API_SETUP 4\r\n");
    os_strcpy(cfg.mqtt_port, value);
}

void ICACHE_FLASH_ATTR api_setup_request(void *context, uint8_t *req_buffer, uint16_t length)
{
    SetupReq *params = (SetupReq*)context;
   
    struct jsonparse_state state;
    int8_t err = 0;
    uint8_t type[20] = {0};
    
    jsonparse_setup(&state, req_buffer, length);;
    jsonparse_next(&state);

    json_find_next_sibling_string(&state, (uint8_t*)"type", type, sizeof(type), &err);

    if(err != JSON_OK)
    {
        params->request = API_SETUP_BAD_REQUEST;
    }
    else if(os_strcmp(type, "get") == 0)
    {
        params->request = API_SETUP_GET_DATA;
    }
    else if(os_strcmp(type, "set") == 0)
    {
        params->request = API_SETUP_SET_DATA;
        parse_api_setup_data(&state, &err);
        if(err) 
        {
            // reload values if they were mutated
            config_load();
            params->request = err;
        }
    }
    else
    {
        params->request = API_SETUP_INVALID_TYPE;
    }
}

void ICACHE_FLASH_ATTR api_setup_response(json_putchar putchar, uint8_t *context)
{
    SetupReq *params = (SetupReq*)context;
    switch(params->request)
    {
        case API_SETUP_GET_DATA:
            print_api_get_response(putchar);
            return;
        case API_SETUP_SET_DATA:
            print_api_set_response(putchar);
            return;
        case API_SETUP_BAD_REQUEST:
        case API_SETUP_INVALID_TYPE:
        case API_SETUP_INVALID_DATA:
            print_api_bad_response(putchar);
            return;
    }
}