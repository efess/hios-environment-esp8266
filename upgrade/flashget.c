#include "flashget.h"
#include "string.h"
#include "osapi.h"
#include "espconn.h"
#include "spi_flash.h"
#include "mem.h"
#include "info.h"
#include "http.h"
#include "logic.h"

void free_context(flash_get_context *context) 
{
    espconn_disconnect(context->con);
    os_free(context->url);
    os_free(context->con->proto.tcp);
    os_free(context->con);
    os_free(context);
}

void ICACHE_FLASH_ATTR request_chunk(void *arg)
{
    
}

void ICACHE_FLASH_ATTR parse_head_response(void *arg)
{

} 


void ICACHE_FLASH_ATTR request_head(void *arg, char *pdata, unsigned short len)
{

}

void ICACHE_FLASH_ATTR sent_cb(void *arg) 
{
    
}

void ICACHE_FLASH_ATTR send_chunk_request(flash_get_context *context) 
{
    int err = 0;
    uint16_t byte_count = 0;
    HttpRequest req;

    req.url = context->url;
    req.verb = HTTP_GET;
    req.rangeStart = context->flash_pos;
    req.rangeEnd = min(
        (context->flash_pos + HTTP_CHUNK_SIZE - 1), (context->flash_len - 1));

    os_memset(context->buf, 0, HTTP_BUFF_SIZE);

    context->state = FLASHGET_TRANSFER_HEADER;
    byte_count = http_write_request_header(&req, context->buf, HTTP_BUFF_SIZE);
    
    // INFO("Request:\r\n\r\n");
    // os_printf_plus(context->buf);
    // INFO("End Request\r\n\r\n");

    if(err != HTTP_OK) 
    {
        INFO("FlashGet: Fail creating transfer request\r\n");
        free_context(context);
        return;
    }

    err = espconn_send(context->con, context->buf, byte_count);
    if(err != ESPCONN_OK)
    {
        INFO("FlashGet: Failed sending chunk at %u bytes\r\n", context->flash_pos);
        http_print_error("espconn_send", err);
        free_context(context);
        return;
    }
}

void ICACHE_FLASH_ATTR receive_cb(void *arg, char *pdata, unsigned short len)
{
    struct espconn *con = (struct espconn *)arg;
    flash_get_context* context = (flash_get_context *)con->reverse;

    // INFO("Response length %u\r\n\r\n", len);
    // os_printf_plus(pdata);
    // INFO("End Response\r\n\r\n");

    uint16_t byte_count;
    int err = 0;

    HttpResponse resp;
    switch(context->state) 
    {
        case FLASHGET_METADATA:
        {
            byte_count = http_parse_response_header(&resp, pdata, len, &err);

            INFO("FlashGet: Receive Metadata\r\n");
            if(err != HTTP_OK)
            {
                INFO("FlashGet: Http parse error, cancelling flash update\r\n");
                return;
            }
            INFO("FlashGet: New flash size %d bytes\r\n", resp.length);

            context->flash_size_cb(resp.length, &err);
            if(err != 0) 
            {
                return;
            }
            context->flash_len = resp.length;
            context->flash_pos = 0;

            context->state = FLASHGET_TRANSFER_HEADER;
            
            send_chunk_request(context);
            break;
        }
        case FLASHGET_TRANSFER_HEADER:
        {
            byte_count = http_parse_response_header(&resp, pdata, len, &err);
            context->http_buffered_progress = 0;
            context->http_chunk_length = resp.length;
            context->state = FLASHGET_TRANSFER_BODY;

            break;
        }
        case FLASHGET_TRANSFER_BODY:
        {
            INFO("FlashGet: Receive %u bytes for position %u\r\n", len, context->flash_pos);

            context->data_chunk_cb(pdata, context->flash_pos, len, &err);
            if(err != 0) 
            {
                context->state = FLASHGET_CANCEL;
                return;
            }

            context->flash_pos += len;
            context->http_buffered_progress += len;

            if(context->flash_pos == context->flash_len)
            {
                INFO("FlashGet: Finished firmware download\r\n", context->flash_pos);
                // DONE
                // Call SPI Finish
                context->finish_cb(0);
                free_context(context);
            } 
            else if(context->http_buffered_progress == context->http_chunk_length)
            {
                send_chunk_request(context);
            }

            break;
        }
    }
}

void ICACHE_FLASH_ATTR connected_cb(void *arg)
{
    struct espconn *con = (struct espconn *)arg;
    flash_get_context* context = (flash_get_context *)con->reverse;

    INFO("FlashGet: Connected\r\n");
    
    uint16_t offset;
    uint16_t byte_count;

    switch(context->state) 
    {
        case FLASHGET_METADATA:
        {
            INFO("FlashGet: Sending Metadata request\r\n");
            HttpRequest req;
            req.rangeEnd = 0;
            req.rangeStart = 0;
            req.url = context->url;
            req.verb = HTTP_HEAD;

            byte_count = http_write_request_header(&req, context->buf, HTTP_BUFF_SIZE);
            espconn_send(con, context->buf, byte_count);

            break;
        }
        case FLASHGET_TRANSFER_HEADER:
        {
            INFO("FlashGet: Sending transfer request\r\n");
        }
    }
}

void ICACHE_FLASH_ATTR dns_cb(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *con = (struct espconn *)arg;
    flash_get_context* context = (flash_get_context *)con->reverse;

    if(ipaddr->addr != 0) 
    {
        INFO("FlashGet: found ip %d.%d.%d.%d\r\n",
                *((uint8 *) &ipaddr->addr),
                *((uint8 *) &ipaddr->addr + 1),
                *((uint8 *) &ipaddr->addr + 2),
                *((uint8 *) &ipaddr->addr + 3));
        
        os_memcpy(con->proto.tcp->remote_ip, &ipaddr->addr, 4);
        
        espconn_connect(con);
    } 
    else 
    {
        INFO("FlashGet: Cannot resolve host DNS\r\n");
        free_context(context);
    }
}


uint8 ICACHE_FLASH_ATTR flashget_download(
    const char *url, 
    flash_get_flash_size flash_size_cb,
    flash_get_data_chunk data_chunk_cb, 
    flash_get_finish_callback finish_cb)
{
    flash_get_context *get_context = (flash_get_context*)os_zalloc(sizeof(flash_get_context));
    get_context->finish_cb = finish_cb;
    get_context->data_chunk_cb = data_chunk_cb;
    get_context->flash_size_cb = flash_size_cb;
    get_context->url = (char*)os_zalloc(strlen(url) + 1);
    get_context->state = FLASHGET_METADATA;
    strcpy(get_context->url, url);

    get_context->con = (struct espconn *)os_zalloc(sizeof(struct espconn));
    get_context->con->type = ESPCONN_TCP;
    get_context->con->state = ESPCONN_NONE;
    get_context->con->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    get_context->con->proto.tcp->remote_port = 80;
    get_context->con->proto.tcp->local_port = espconn_port();
    get_context->con->reverse = get_context;

    espconn_regist_recvcb(get_context->con, receive_cb);
    espconn_regist_connectcb(get_context->con, connected_cb);
    espconn_regist_sentcb(get_context->con, sent_cb);
    
    uint8 result = 0;
    uint8 host[100];
    ip_addr_t ip;

    INFO("FlashGet: Looking up host\r\n");
	if (http_get_host_from_url(url, host) < 0) {
        INFO("FlashGet: Invalid URL, couldn't parse host\r\n");
		return -1;
    }
    
    INFO("FlashGet: host: %s\r\n", host);
    espconn_gethostbyname(get_context->con, host, &ip, dns_cb);
}

