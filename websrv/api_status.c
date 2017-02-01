#include "api_status.h"
#include "run_state.h"
#include "jsonh.h"

void ICACHE_FLASH_ATTR api_status_print_response(json_putchar putchar)
{
    JSON_PAIR_INT(wifiStatus, "wifiStatus", 1);
    JSON_OBJECT(status_obj, wifiStatus);

    struct jsontree_context json_context;
    jsontree_setup(&json_context, (struct jsontree_value *) &status_obj, putchar);

    while(jsontree_print_next(&json_context)) {}
}

void ICACHE_FLASH_ATTR api_status_request(void *context, uint8_t *req_buffer, uint16_t length)
{
    // meh?
}

void ICACHE_FLASH_ATTR api_status_response(int (* putchar)(int c) , uint8_t *params)
{
    api_status_print_response(putchar);
}