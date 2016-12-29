#ifndef FLASHGET_H_
#define FLASHGET_H_

#define GET_SUCCESS     0
#define GET_FAIL        -1

#define HTTP_BUFF_SIZE 1024
#define HTTP_CHUNK_SIZE 1024

#include "user_interface.h"
#include "espconn.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

typedef void (* flash_get_finish_callback)(uint8 result);
typedef void (* flash_get_data_chunk)(uint8_t *data, uint32_t offset, uint16_t length, int *err);
typedef void (* flash_get_flash_size)(uint32_t size, int *err);

typedef enum {
    FLASHGET_METADATA,
    FLASHGET_TRANSFER_HEADER,
    FLASHGET_TRANSFER_BODY,
    FLASHGET_CANCEL
} flash_get_state;

typedef struct {
    flash_get_state state;
    
    struct espconn* con;
    // keeps track of multiple receive callbacks
    uint32 http_buffered_progress;
    uint32 http_chunk_length;
    uint32 flash_len;
    uint32 flash_pos;
    flash_get_finish_callback finish_cb;
    flash_get_data_chunk data_chunk_cb;
    flash_get_flash_size flash_size_cb;
    uint8_t* url;
    uint8_t buf[HTTP_BUFF_SIZE];
} flash_get_context;


uint8 ICACHE_FLASH_ATTR flashget_download(
    const char *hostname, 
    flash_get_flash_size flash_size_cb,
    flash_get_data_chunk data_chunk_cb, 
    flash_get_finish_callback finish_cb);

#endif /* FLASHGET_H_ */
