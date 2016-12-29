#include "user_interface.h"
#include "http.h"
#include "espconn.h"
#include "c_types.h"
#include "os_type.h"
#include "debug.h"
#include "osapi.h"
#include "mem.h"
#include "info.h"
#include "string.h"

void ICACHE_FLASH_ATTR http_print_error(const uint8_t *name, int8_t errNum) 
{
    switch(errNum) {
        case ESPCONN_MEM:
            INFO("HTTP: (%s) Out of Memory\r\n", name);
            return;
        case ESPCONN_TIMEOUT:
            INFO("HTTP: (%s) Timeout\r\n", name);
            return;
        case ESPCONN_RTE:
            INFO("HTTP: (%s) Routing problem\r\n", name);
            return;
        case ESPCONN_INPROGRESS:
            INFO("HTTP: (%s) Operation in progress\r\n", name);
            return;
        case ESPCONN_MAXNUM:
            INFO("HTTP: (%s) Total number exceeds the set maximum\r\n", name);
            return;
        case ESPCONN_ABRT:
            INFO("HTTP: (%s) Connection aborted\r\n", name);
            return;
        case ESPCONN_RST:
            INFO("HTTP: (%s) Connection reset\r\n", name);
            return;
        case ESPCONN_CLSD:
            INFO("HTTP: (%s) Connection closed\r\n", name);
            return;
        case ESPCONN_CONN:
            INFO("HTTP: (%s) Not connected\r\n", name);
            return;
        case ESPCONN_ARG:
            INFO("HTTP: (%s) Illegal argument\r\n", name);
            return;
        case ESPCONN_ISCONN:
            INFO("HTTP: (%s) Already connected\r\n", name);
            return;
        default:
            INFO("HTTP: (%s) Unkown error, code: (%d)\r\n", name, errNum);
            return;
    }
}

uint8_t ICACHE_FLASH_ATTR put(uint8_t* str, uint8_t** buf) 
{
    uint16_t len = strlen(str);

    strcpy(*buf, str);
    *buf += len;
    return len;
}

uint8_t ICACHE_FLASH_ATTR http_get_host_from_url(const uint8_t* url, uint8_t* host)
{
	uint8_t *httpEnd = strstr(url, "://");
	if (!httpEnd) {
		INFO("HTTP: Invalid URL.\r\n");
		return -1;
	}
	httpEnd += 3;

	uint8_t *hostEnd = strstr(httpEnd, "/");
	if (!hostEnd) {
		INFO("HTTP: Invalid URL.\r\n");
		return -1;
	}

	strncpy(host, httpEnd, hostEnd - httpEnd);
    host[hostEnd - httpEnd] = '\0';
	return 0;
}

uint16_t ICACHE_FLASH_ATTR http_write_request_header(HttpRequest *req, uint8_t *buf, uint16_t maxlen)
{
	if (strlen(req->url) > 100) {
		INFO("HTTP: URL is too long. Max is 100.\r\n");
		return -1;
	}

	char host[100] = { 0 };
	if (http_get_host_from_url(req->url, host) < 0) {
		INFO("HTTP: Invalid URL, couldn't parse host\r\n");
		return -1;
	}

	uint8_t *buffer = buf;
	uint8_t count = 0;
	uint16_t bytesWritten = 0;

	switch (req->verb)
	{
	case HTTP_GET:       bytesWritten += put("GET ", &buffer); break;
	case HTTP_POST:      bytesWritten += put("POST ", &buffer); break;
	case HTTP_HEAD:      bytesWritten += put("HEAD ", &buffer); break;
	case HTTP_DELETE:    bytesWritten += put("DELETE ", &buffer); break;
	case HTTP_PUT:       bytesWritten += put("PUT ", &buffer); break;
	}

	bytesWritten += put(req->url, &buffer);
	bytesWritten += put(" HTTP/1.1\r\n", &buffer);
	bytesWritten += put("Host: ", &buffer);
	bytesWritten += put(host, &buffer);
	bytesWritten += put("\r\n", &buffer);
	bytesWritten += put("Connection: keep-alive\r\n", &buffer);
	bytesWritten += put("User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64)\r\n", &buffer);

	if (req->rangeEnd > 0) {
		count = os_sprintf(buffer, "Range: bytes=%u-%u\r\n", req->rangeStart, req->rangeEnd);
		bytesWritten += count;
		buffer += count;
	}
	bytesWritten += put("\r\n", &buffer);

	return bytesWritten;
}

int8_t ICACHE_FLASH_ATTR parse_next_header(uint8_t *buf, char* name, char* value)
{
	uint8_t len = 0;
	if (buf[0] && buf[1] && 
		buf[0] == '\r' && buf[1] == '\n') 
    {
		return 2;
	}

	uint8_t *bufStart = buf;
	uint8_t *bufPtr;
	uint8_t length = 0;

	bufPtr = strchr(bufStart, ':');
	length = bufPtr - bufStart;
	if (!bufPtr || length >= HTTP_MAX_HEADER_NAME) 
    {
		INFO("HTTP: Invalid header, bad name\r\n");
		return -1;
	}

	strncpy(name, bufStart, length);
	name[length] = '\0';

	bufPtr += 2; // skip colon and space
	bufStart = bufPtr;
	bufPtr = strstr(bufPtr, "\r\n");
	length = bufPtr - bufStart;
	if (!bufPtr || length >= HTTP_MAX_HEADER_VALUE) 
    {
		INFO("HTTP: Invalid header, bad value\r\n");
		return -1;
	}

	strncpy(value, bufStart, length);
	value[length] = '\0';
	bufPtr += 2; // skip cr ln

	return bufPtr - buf;
}

int8_t ICACHE_FLASH_ATTR parse_response_start_line(HttpResponse *response, uint8_t *buf, int* err)
{
	uint8_t *bufPtr = strstr(buf, "\r\n");
	if (!bufPtr || bufPtr - buf >= 200) 
    {
		INFO("HTTP: Invalid header start line\r\n");
		return -1;
	}
	
	// TODO parse status...

	bufPtr += 2;
	return bufPtr - buf;
}

uint16_t ICACHE_FLASH_ATTR http_parse_response_header(HttpResponse *response, uint8_t *buf, uint16_t length, int* err)
{
	uint8_t name[HTTP_MAX_HEADER_NAME];
	uint8_t value[HTTP_MAX_HEADER_VALUE];
	uint8_t *bufPtr = buf;
	int16_t byte_counter = 0;
	
	byte_counter = parse_response_start_line(response, buf, err);
	if (*err != HTTP_OK) 
    {
		
	}
	bufPtr += byte_counter;

	while (1)
	{
		byte_counter = parse_next_header(bufPtr, name, value);
		bufPtr += byte_counter;
		if (byte_counter < 0) 
        {
			return 1;
		}
		if (byte_counter <= 2) 
        {
			break;
		}

		if (strcmp(name, HTTP_HEADER_CONTENT_LENGTH) == 0) 
        {
			response->length = atoi(value);
		}
	}

	return bufPtr - buf;
}