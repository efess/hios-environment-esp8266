#ifndef _HTTP_H_
#define _HTTP_H_

#include "user_interface.h"

#define HTTP_OK 0
#define HTTP_ERROR_PARSE -5
#define HTTP_MAX_HEADER_NAME 30
#define HTTP_MAX_HEADER_VALUE 50
#define HTTP_HEADER_CONTENT_LENGTH "Content-Length"


typedef enum {
	HTTP_GET,
	HTTP_POST,
	HTTP_HEAD,
	HTTP_DELETE,
	HTTP_PUT
} HttpType;

typedef struct {
    HttpType verb;
    uint16_t status;
    uint32_t length;
    uint8_t content_type[50];
} HttpResponse;

typedef struct {
    char *url;
    HttpType verb;
    
    uint32_t rangeStart;
    uint32_t rangeEnd;

    uint32_t length;
} HttpRequest;

// typedef void (* http_receive_chunk_callback)(Http httpContext, uint8 *data, uint16 len);
// typedef void (* http_send_chunk_callback)(Http httpContext, uint8 **data, uint16 *len);

// Need callbacks for finish?
void http_print_error(const uint8_t *name, int8_t errNum);
uint8_t http_get_host_from_url(const uint8_t* url, uint8_t* host);
uint16_t http_parse_request_header(HttpRequest *request, uint8_t *buf, uint16_t length, int* err);
uint16_t http_write_request_header(HttpRequest *req, uint8_t *buf, uint16_t maxlen);
uint16_t http_parse_response_header(HttpResponse *response, uint8_t *buf, uint16_t length, int* err);
uint16_t http_write_response_header(HttpResponse *res, uint8_t *buf, uint16_t maxlen);
uint8_t ICACHE_FLASH_ATTR http_get_port_from_url(const uint8_t* url, uint16_t* host);

#endif