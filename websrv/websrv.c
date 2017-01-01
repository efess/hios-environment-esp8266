#include "websrv.h"
#include "user_interface.h"
#include "osapi.h"
#include "mem.h"
#include "info.h"
#include "espconn.h"
#include "http.h"

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