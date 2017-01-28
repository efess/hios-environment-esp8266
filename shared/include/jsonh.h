#ifndef JSONH_H_
#define JSONH_H_

#include "user_interface.h"
#include "jsontree.h"
#include "jsonparse.h"

#define JSON_OK 0
#define JSON_NOT_FOUND -1
#define JSON_NOT_CORRECT_TYPE -2
#define JSON_MAX_DYNAMIC_ALLOCATIONS 128

typedef int (* json_putchar)(int c);

int8_t json_find_next_sibling(struct jsonparse_state *json_context, const char *name, struct jsonparse_state *found_json);
void json_find_next_sibling_string(struct jsonparse_state *json_context, const uint8_t *name, uint8_t *value, uint8_t val_length, int8_t *err);
void json_find_next_sibling_int(struct jsonparse_state *json_context, const uint8_t *name, int *value, int8_t *err);

struct jsontree_object * json_dynamic_create_object(uint8_t count);
struct jsontree_pair * json_dynamic_create_pairs(uint8_t count);
struct jsontree_string * json_dynamic_create_string(const uint8_t *var);
struct jsontree_int * json_dynamic_create_int(const int integer);
void json_dynamic_free_all();

#define JSON_PAIR_STRING(name, field, string)                                          \
    struct jsontree_string _json_tree_##name ={JSON_TYPE_STRING, (string)};            \
    struct jsontree_pair name = {(field), (struct jsontree_value *)(&_json_tree_##name)};

#define JSON_PAIR_INT(name, field, interger)                                           \
    struct jsontree_int _json_tree_##name ={JSON_TYPE_NUMBER, (interger)};             \
    struct jsontree_pair name = {(field), (struct jsontree_value *)(&_json_tree_##name)};

#define JSON_OBJECT(name, ...)                                     \
    struct jsontree_pair jsontree_pair_##name[] = {__VA_ARGS__};   \
    struct jsontree_object name = {                                \
        JSON_TYPE_OBJECT,                                          \
        sizeof(jsontree_pair_##name)/sizeof(struct jsontree_pair), \
        jsontree_pair_##name }
#endif