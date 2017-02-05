#include "jsonh.h"
#include "string.h"
#include "mem.h"
#include "info.h"

static jsontree_buffer json_putchar_buffer;
static int ICACHE_FLASH_ATTR _json_putchar(int c)
{
    if (json_putchar_buffer.pos < json_putchar_buffer.max_size) {
        json_putchar_buffer.data[json_putchar_buffer.pos++] = c;
        return c;
    }
    return 0;
}

json_putchar ICACHE_FLASH_ATTR json_get_putchar()
{
    return _json_putchar;
}

int ICACHE_FLASH_ATTR json_get_buffer_length()
{
    return json_putchar_buffer.pos;
}

void ICACHE_FLASH_ATTR json_init_putchar_buffer(uint8_t *buffer, uint16_t size) 
{
    json_putchar_buffer.data = buffer;
    json_putchar_buffer.pos = 0;
    json_putchar_buffer.max_size = size;
}

int8_t ICACHE_FLASH_ATTR json_find_next_sibling(struct jsonparse_state *json_context, const char *name, struct jsonparse_state *found_json)
{
    struct jsonparse_state local_context = *json_context;
    uint8_t depth = json_context->depth;
    uint8_t type = 0;

    while (jsonparse_has_next(&local_context))
    {
        type = jsonparse_next(&local_context);

        if (type == JSON_TYPE_PAIR_NAME &&
            local_context.depth == depth &&
            jsonparse_strcmp_value(&local_context, name) == 0)
        {
            *found_json = local_context;
            return JSON_OK;
        }
    }

    return JSON_NOT_FOUND;
}

/* returns nest available atomic value, does not affect state */
void ICACHE_FLASH_ATTR json_find_next_sibling_string(struct jsonparse_state *json_context, const uint8_t *name, uint8_t *value, uint8_t val_length, int8_t *err)
{
    struct jsonparse_state local_context;
    uint8_t json_type;

    if (json_find_next_sibling(json_context, name, &local_context) == JSON_NOT_FOUND) {
        *err = JSON_NOT_FOUND;
        // not found
        return;
    }

    json_type = jsonparse_next(&local_context);
    if (json_type != JSON_TYPE_STRING)
    {
        *err = JSON_NOT_CORRECT_TYPE;
        return;
    }
    
    jsonparse_copy_value(&local_context, value, val_length);
}

/* returns nest available atomic value, does not affect state */
void ICACHE_FLASH_ATTR json_find_next_sibling_int(struct jsonparse_state *json_context, const uint8_t *name, int *value, int8_t *err)
{
    struct jsonparse_state local_context;
    uint8_t json_type;

    if (json_find_next_sibling(json_context, name, &local_context) == JSON_NOT_FOUND) {
        *err = JSON_NOT_FOUND;
        // not found
        return;
    }

    json_type = jsonparse_next(&local_context);
    if (json_type != JSON_TYPE_NUMBER)
    {
        *err = JSON_NOT_CORRECT_TYPE;
        return;
    }
    *value = jsonparse_get_value_as_int(&local_context);
}

void* dynamic_allocs[JSON_MAX_DYNAMIC_ALLOCATIONS] = { 0 };
uint8_t dynamic_allocs_count = 0;

struct ICACHE_FLASH_ATTR jsontree_object * json_dynamic_create_object(uint8_t count)
{
    struct jsontree_object * mem;
    if (count > 0) {
        mem = (struct jsontree_object*)os_zalloc(sizeof(struct jsontree_object) * count);
        dynamic_allocs[dynamic_allocs_count++] = (void*)mem;
        return mem;
    }
    return 0;
}

struct ICACHE_FLASH_ATTR jsontree_pair * json_dynamic_create_pairs(uint8_t count)
{
    uint8_t i = 0;
    struct jsontree_pair * mem;
    if (count > 0) {
        mem = (struct jsontree_pair*)os_zalloc(sizeof(struct jsontree_pair) * count);
        dynamic_allocs[dynamic_allocs_count++] = (void*)mem;
        return mem;
    }
    return 0;
}
struct ICACHE_FLASH_ATTR jsontree_string * json_dynamic_create_string(const uint8_t *var)
{
    struct jsontree_string* mem = (struct jsontree_string*)os_zalloc(sizeof(struct jsontree_string));
    mem->type = JSON_TYPE_STRING;
    mem->value = var;

    dynamic_allocs[dynamic_allocs_count++] = (void*)mem;
    
    return mem;
}
struct ICACHE_FLASH_ATTR jsontree_int * json_dynamic_create_int(const int integer)
{
    struct jsontree_int* mem = (struct jsontree_int*)os_zalloc(sizeof(struct jsontree_int));
    mem->type = JSON_TYPE_INT;
    mem->value = integer;

    dynamic_allocs[dynamic_allocs_count++] = (void*)mem;

    return mem;
}

void ICACHE_FLASH_ATTR json_dynamic_free_all()
{
    uint8_t i = 0;
    for (i = 0; i < dynamic_allocs_count; i++)
    {
        os_free(dynamic_allocs[i]);
    }
    dynamic_allocs_count = 0;
}