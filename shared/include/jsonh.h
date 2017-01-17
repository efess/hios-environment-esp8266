#ifndef JSONH_H_
#define JSONH_H_

#include "user_interface.h"
#include "json/jsontree.h"
#include "json/jsonparse.h"

#define JSON_OK 0
#define JSON_NOT_FOUND -1
#define JSON_NOT_CORRECT_TYPE -2
#define JSON_MAX_DYNAMIC_ALLOCATIONS 128

void json_find_next_sibling_string(struct jsonparse_state *json_context, const uint8_t *name, uint8_t *value, uint8_t val_length, int8_t *err);

struct jsontree_object * json_dynamic_create_object(uint8_t count);
struct jsontree_pair * json_dynamic_create_pairs(uint8_t count);
struct jsontree_string * json_dynamic_create_string(const uint8_t *var);
struct jsontree_int * json_dynamic_create_int(const int integer);
void json_dynamic_free_all();

#endif