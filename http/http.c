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
#include "logic.h"

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

// puts data into the buffer, and increments the buffer pointer
// returns number of bytes written
uint8_t ICACHE_FLASH_ATTR put(const uint8_t* str, uint8_t** buf) 
{
    uint16_t len = strlen(str);

    strcpy(*buf, str);
    *buf += len;
    return len;
}

uint8_t ICACHE_FLASH_ATTR write_header(const uint8_t *name,const uint8_t *value, uint8_t **buf)
{
	return put(name, buf) + put(": ", buf) + put(value, buf) + put("\r\n", buf);
}

uint8_t ICACHE_FLASH_ATTR write_status(uint16_t status, uint8_t **buf)
{
	switch(status)
	{
		case 200: return put(" 200 OK\r\n", buf);
		case 400: return put(" 400 Bad request\r\n", buf);
		case 404: return put(" 404 Not Found\r\n", buf);
		default: return put(" 501 Not Implemented\r\n", buf);
	}
}

uint16_t ICACHE_FLASH_ATTR http_write_response_header(HttpResponse *res, uint8_t *buf, uint16_t maxlen)
{
	uint16_t byte_counter = 0;
	uint8_t temp[16];

	byte_counter += put("HTTP/1.1", &buf);
	byte_counter += write_status(res->status, &buf);
	if(res->status == 200 && res->length) {
		byte_counter += write_header("Content-Type", res->content_type, &buf);

		os_sprintf(temp, "%u", res->length);
		byte_counter += write_header("Content-Length", temp, &buf);
	}
	byte_counter += put("\r\n", &buf);

	return byte_counter;
}

uint8_t ICACHE_FLASH_ATTR http_get_host_from_url(const uint8_t* url, uint8_t* host)
{
	uint8_t *httpEnd = strstr(url, "://");
	uint8_t *hostEnd = 0;
	if (!httpEnd) {
		INFO("HTTP: Invalid URL.\r\n");
		return -1;
	}
	httpEnd += 3;

	hostEnd = strstr(httpEnd, ":");
	if(!hostEnd)
	{
		hostEnd = strstr(httpEnd, "/");
	}
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
	if (!bufPtr) 
    {
		INFO("HTTP: Invalid header, bad name\r\n");
		return -1;
	}

	strncpy(name, bufStart, min(length, HTTP_MAX_HEADER_NAME - 1));
	name[length] = '\0';

	bufPtr += 2; // skip colon and space
	bufStart = bufPtr;
	bufPtr = strstr(bufPtr, "\r\n");
	length = bufPtr - bufStart;
	if (!bufPtr) 
    {
		INFO("HTTP: Invalid header, bad value\r\n");
		return -1;
	}

	strncpy(value, bufStart, min(length, HTTP_MAX_HEADER_VALUE - 1));
	value[length] = '\0';
	bufPtr += 2; // skip cr ln

	return bufPtr - buf;
}

uint8_t ICACHE_FLASH_ATTR parse_response_start_line(HttpResponse *response, uint8_t *buf, int* err)
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

uint16_t ICACHE_FLASH_ATTR parse_request_start_line(HttpRequest *request, uint8_t *buf, int* err)
{
	uint8_t str[100] = { 0 };
	uint8_t len = 0;
	uint8_t *bufStart = buf;
	uint8_t *bufPtr = buf;
	uint8_t *tokenPtr = 0;

	tokenPtr = strchr(bufPtr, ' ');
	if (!tokenPtr)
	{
		INFO("HTTP: Invalid header start line\r\n");
		*err = -1;
		return 0;
	}
	strncpy(str, bufPtr, tokenPtr - bufPtr);
	str[tokenPtr - bufPtr] = '\0';
	bufPtr = tokenPtr + 1;// skip token

	if (strcmp(str, "GET") == 0)
	{
		request->verb = HTTP_GET;
	}
	else if (strcmp(str, "HEAD") == 0)
	{
		request->verb = HTTP_HEAD;
	}
	else if (strcmp(str, "POST") == 0)
	{
		request->verb = HTTP_POST;
	}
	else
	{
		INFO("HTTP: Unsupported verb '%s'\r\n", str);
		*err = -1;
		return 0;
	}

	tokenPtr = strchr(bufPtr, ' ');
	if (!tokenPtr)
	{
		INFO("HTTP: Invalid header start line\r\n");
		*err = -1;
		return 0;
	}
	len = tokenPtr - bufPtr;
	if (len >= 100)
	{
		INFO("HTTP: URL is at or above 100 characters\r\n");
		*err = -1;
		return 0;
	}

	request->url = (uint8_t*)os_zalloc(len + 1);
	strncpy(request->url, bufPtr, len);
	request->url[len] = 0;
	bufPtr = tokenPtr + 1; // skip space

	tokenPtr = strstr(bufPtr, "\r\n");
	if (!tokenPtr)
	{
		INFO("HTTP: Invalid header start line, end not found\r\n");
		*err = -1;
		return 0;
	}

	bufPtr = tokenPtr + 2;
	return bufPtr - bufStart;
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

uint16_t ICACHE_FLASH_ATTR http_parse_request_header(HttpRequest *request, uint8_t *buf, uint16_t length, int* err)
{
	uint8_t name[HTTP_MAX_HEADER_NAME];
	uint8_t value[HTTP_MAX_HEADER_VALUE];
	uint8_t *bufPtr = buf;
	int16_t byte_counter = 0;

	byte_counter = parse_request_start_line(request, buf, err);
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
			request->length = atoi(value);
		}
	}

	return bufPtr - buf;
}