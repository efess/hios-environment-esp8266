#include "websrv.h"
#include "user_interface.h"
#include "osapi.h"
#include "mem.h"
#include "info.h"
#include "espconn.h"
#include "http.h"
#include "websrv_file.h"

Client *client = 0;

uint8 ICACHE_FLASH_ATTR webserver_client_count()
{
    uint8_t count = 0;
    Client *itemPtr = client;
    
    while(itemPtr)
    {
        count++;
        itemPtr = itemPtr->next;
    }
    return count;
}

Client* ICACHE_FLASH_ATTR webserver_client_find(uint8_t *identity)
{
    Client *itemPtr = client;

    while(itemPtr)
    {
        if(strcmp(itemPtr->identity, identity) == 0) 
        {
            return itemPtr;
        }

        itemPtr = itemPtr->next;
    }
    return 0;
}

Client * ICACHE_FLASH_ATTR webserver_client_back()
{
    Client *listPtr = client;
    while(listPtr && listPtr->next) 
    {
        listPtr = listPtr->next;
    }

    return listPtr;
}

void ICACHE_FLASH_ATTR webserver_client_add(Client *newItem)
{
    Client *listPtr = webserver_client_back();
    if(listPtr) 
    {
        listPtr->next = newItem;
    } 
    else
    {
        client = newItem;
    }
}

void ICACHE_FLASH_ATTR webserver_client_remove(Client *removeClient)
{
    Client *itemPtr = client;
    if(!client){
        return;
    }

    if(client == removeClient) 
    {
        client = removeClient->next;
        return;
    }

    while(itemPtr && itemPtr->next != removeClient)
    {
        itemPtr = itemPtr->next;
    }

    if(itemPtr && itemPtr->next == removeClient) 
    {
        itemPtr->next = removeClient->next;
    }
}

Client* ICACHE_FLASH_ATTR webserver_create_client(uint8_t *identity)
{
    Client *clientPtr = webserver_client_find(identity);

    if(clientPtr) 
    {
        INFO("WebSrv: Create client called when client already exists: %s\r\n", identity);
        return clientPtr;
    }

    Client *newClient = (Client*)os_zalloc(sizeof(Client));
    newClient->identity = (uint8_t*)os_zalloc(strlen(identity) + 1);

    strcpy(newClient->identity, identity);
    
    return newClient;
}

void ICACHE_FLASH_ATTR webserver_destroy_client(Client *client)
{
    os_free(client->identity);
    os_free(client);
}

uint8_t * ICACHE_FLASH_ATTR webserver_create_client_identity(uint8_t *identity, uint8_t *ip, uint16_t port) 
{
    os_sprintf(identity, "%d.%d.%d.%d:%d", *(ip), *(ip + 1), *(ip + 2), *(ip + 3),  port);
}

uint8_t * ICACHE_FLASH_ATTR webserver_get_resource_name(uint8_t *url)
{
    uint8_t tmp[50];
    uint8_t *bufPtr = url;
    uint8_t counter = 0;
    for(counter = 0; counter < 3; counter++)
    {
        bufPtr = strchr(bufPtr, '/');
        if(!bufPtr) { return 0; }
    }
    bufPtr++;

    uint8_t *resource = (uint8_t*)os_zalloc(strlen(bufPtr) + 1);
    strcpy(resource, bufPtr);

    return resource;
}

void ICACHE_FLASH_ATTR webserver_handle_received(Client *client, struct espconn *con, uint8_t *data, uint16_t len)
{
    int err = 0;
    uint16_t byte_count = 0;
    uint8_t *bufPtr = data;

    switch(client->state)
    {
        case CLIENT_IDLE:
        {
            // assuming header
            HttpRequest *request = (HttpRequest*)os_zalloc(sizeof(HttpRequest));
            
            byte_count = http_parse_request_header(request, bufPtr, len, &err);
            if(err != HTTP_OK) 
            {
                return;
            }
            client->resource = webserver_get_resource_name(request->url);
            INFO("WebSrv: header received for resource %s\r\n", client->resource);
            client->current_client_request = request;
            client->current_byte_offset = 0;
            bufPtr += byte_count;

            if(request->length) {
                client->state = CLIENT_SENDING;
                client->current_size = request->length;
            } else {
                client->current_size = 0;
            }

            client->current_handler.request_chunk_fn = websrv_file_request_data_chunk;
            client->current_handler.request_size_fn = websrv_file_request_data_size;
            
            // parse URL, find handler, process request
            break;
        }
        case CLIENT_SENDING:
        {
            // receiving message body
            client->current_byte_offset += len;
            INFO("WebSrv: Received all %u bytes\r\n", client->current_byte_offset);
            break;
        }
    }

    if(client->current_size == client->current_byte_offset)
    {
        client->state = CLIENT_RECEIVING;
        client->current_byte_offset = 0;
        uint32_t resource_length = client->current_handler.request_size_fn(client->resource);
        INFO("WebSrv: Received all bytes\r\n");
        // Got the dataz. Send response.
        HttpResponse response;
        response.verb  = client->current_client_request->verb;
        
        if(resource_length == 0) 
        {
            INFO("WebSrv: Resource not found\r\n");
            response.status = 404;
        }
        else 
        {
            INFO("WebSrv: Resource found at %u bytes\r\n", resource_length);
            response.status = 200;
            response.length = resource_length;
            websrv_file_request_data_content_type(client->resource, response.content_type);
            client->current_size = resource_length;
        }
        
        INFO("WebSrv: Writing response http_parse_request_header\r\n");
        uint16_t bytes_written = http_write_response_header(&response, client->buf, WEB_SRV_BUF);
        INFO("WebSrv: Sending response\r\n");
        espconn_send(con, client->buf, bytes_written);
        INFO("WebSrv: response sent\r\n");
    }
}

void ICACHE_FLASH_ATTR webserver_handle_sent(Client *client, struct espconn *con)
{
    if(client->current_size == client->current_byte_offset)
    {
        client->state = CLIENT_IDLE;
        // cleanup.
        if(client->current_client_request)
        {
            os_free(client->current_client_request->url);
            os_free(client->current_client_request);
        }
        if(client->resource)
        {
            os_free(client->resource);
        }
    } 
    else 
    {
        uint16_t send_length = 0;
        client->current_handler.request_chunk_fn(
            client->resource,
            client->buf,
            WEB_SRV_BUF,
            client->current_byte_offset,
            &send_length
        );
        client->current_byte_offset += send_length;
        espconn_send(con, client->buf, send_length);
    }
}

void ICACHE_FLASH_ATTR webserver_receive_cb(void *arg, char *pdata, unsigned short len)
{
    struct espconn *con = (struct espconn *)arg;
    uint8_t identity[23];
    webserver_create_client_identity(identity, 
            con->proto.tcp->remote_ip,
            con->proto.tcp->remote_port);

    INFO("WebSrv: Receive CB. %s (%u connected clients)\r\n", identity, webserver_client_count());
    Client* client = webserver_client_find(identity);
    if(!client) {
        INFO("WebSrv: Receive called but client not foundfor %s\r\n", identity);
        return;
    }
    webserver_handle_received(client, con, pdata, len);
}

void ICACHE_FLASH_ATTR webserver_connect_cb(void *arg)
{
    struct espconn *con = (struct espconn *)arg;
    uint8_t identity[23];
    webserver_create_client_identity(identity, 
            con->proto.tcp->remote_ip,
            con->proto.tcp->remote_port);

    INFO("WebSrv: Connect CB. %s (%u connected clients)\r\n", identity, webserver_client_count());

    Client* client = webserver_create_client(identity);
    INFO("WebSrv: Created client %s\r\n", client->identity);
    webserver_client_add(client);
    INFO("WebSrv: Added client %s\r\n", client->identity);
}


void ICACHE_FLASH_ATTR webserver_disconnect_cb(void *arg)
{
    struct espconn *con = (struct espconn *)arg;
    uint8_t identity[23];
    webserver_create_client_identity(identity, 
            con->proto.tcp->remote_ip,
            con->proto.tcp->remote_port);

    INFO("WebSrv: Disconnect CB. %s (%u connected clients)\r\n", identity, webserver_client_count());
    Client* client = webserver_client_find(identity);
    if(!client) {
        INFO("WebSrv: Disconnect called but client not found for %s\r\n", identity);
        return;
    }

    webserver_client_remove(client);
    webserver_destroy_client(client);
}

void ICACHE_FLASH_ATTR webserver_sent_cb(void *arg)
{
    struct espconn *con = (struct espconn *)arg;
    uint8_t identity[23];
    webserver_create_client_identity(identity, 
            con->proto.tcp->remote_ip,
            con->proto.tcp->remote_port);

    INFO("WebSrv: Sent CB. %s (%u connected clients)\r\n", identity, webserver_client_count());
    Client* client = webserver_client_find(identity);
    if(!client) {
        INFO("WebSrv: Sent called but client not foundfor %s\r\n", identity);
        return;
    }

    webserver_handle_sent(client, con);
}

void ICACHE_FLASH_ATTR webserver_recon_cb(void *arg, sint8 err)
{
    struct espconn *con = (struct espconn *)arg;
    uint8_t identity[23];
    webserver_create_client_identity(identity, 
            con->proto.tcp->remote_ip,
            con->proto.tcp->remote_port);

    INFO("WebSrv: Recon CB. %s (%u connected clients)\r\n", identity, webserver_client_count());
    Client* client = webserver_client_find(identity);
    if(!client) {
        INFO("WebSrv: Recon called but client not foundfor %s\r\n", identity);
        return;
    }
    
    webserver_client_remove(client);
    webserver_destroy_client(client);
}



void ICACHE_FLASH_ATTR web_listen(WebSrv *srv, uint16_t port) 
{
    int8_t result;
    
    srv->con = (struct espconn*)os_zalloc(sizeof(struct espconn));  
    srv->con->type = ESPCONN_TCP;
    srv->con->state = ESPCONN_NONE;
    srv->con->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    srv->con->proto.tcp->local_port = port;
    //srv->con->reverse = srv;
    srv->con->reverse = "testing.";

    espconn_regist_recvcb(srv->con, webserver_receive_cb);
    espconn_regist_connectcb(srv->con, webserver_connect_cb);
    espconn_regist_sentcb(srv->con, webserver_sent_cb);
    espconn_regist_disconcb(srv->con, webserver_disconnect_cb);
    espconn_regist_reconcb(srv->con, webserver_recon_cb);

    result = espconn_accept(srv->con);

    if(result != ESPCONN_OK) {
        http_print_error("espconn_accept", result);
        return;
    }

    result = espconn_regist_time(srv->con, 60, 0);
    if(result != ESPCONN_OK) {
        http_print_error("espconn_regist_time", result);
        return;
    }

    INFO("WebSrv: Web server listening on port %d\r\n", port);
}

// void websrv_receive(void *arg, char *pdata, unsigned short len)
// {
//     INFO("WebSrv: Received data length %d", len);
//     struct espconn* con = (struct espconn*)arg;
//     uint8_t its[] = "hello world";

//     espconn_send(con, its, 12);
//     INFO("WebSrv: Sending...");
// }