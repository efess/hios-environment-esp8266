
typedef void (* push_data_chunk)(uint8_t *resource, uint8_t *data, uint32_t offset, uint16_t length);
typedef void (* request_data_chunk)(uint8_t *resource, uint8_t *data, uint16_t buffer_size, uint32_t offset, uint16_t *length);
typedef uint32_t (* request_data_size)(uint8_t *resource);
typedef void (* request_data_content_type)(uint8_t *resource, uint8_t *content_type);