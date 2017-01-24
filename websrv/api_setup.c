#include "api_setup.h"
#include "mem.h"
#include "jsonh.h"
#include "json/jsontree.h"
#include "json/jsonparse.h"
#include "info.h"

void api_setup_request(void *context, uint8_t *req_buffer, uint16_t length)
{
    struct jsonparse_state state;
    
    jsonparse_setup(&state, req_buffer, length);
    jsonparse_next(&state);
    
    if(!state.error) 
    {
        // todo
    }
}

void api_setup_response(int (* json_putchar)(int c), uint8_t *params)
{

}