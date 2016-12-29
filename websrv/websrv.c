#include "websrv.h"
#include "espconn.h"

void ICACHE_FLASH_ATTR receive_cb(void *arg, char *pdata, unsigned short len)
{

}

void websrvlisten(WebSrv *srv, uint16_t port) 
{
    int8_t result;
    
    srv->con = (struct espconn*)os_zalloc(sizeof(struct espconn));  
    srv->con->type = ESPCONN_TCP;
    srv->con->state = ESPCONN_NONE;
    srv->con->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    srv->con->proto.tcp->local_port = port;
    srv->con->recv_callback = receive_cb;
    srv->con->reverse = srv;

    result = espconn_accept(srv->con);

    if(result != ESPCONN_OK) {
        print_error("espconn_accept", result);
        return;
    }

    result = espconn_regist_time(srv->con, 60, 0);
    if(result != ESPCONN_OK) {
        print_error("espconn_regist_time", result);
        return;
    }

    INFO("HTTP: Web server listening on port %d", port);
}

void websrv_receive(void *arg, char *pdata, unsigned short len)
{
    INFO("HTTP: Received data length %d", len);
    struct espconn* con = (struct espconn*)arg;
    uint8_t its[] = "hello world";

    espconn_send(con, its, 12);
    INFO("HTTP: Sending...");
}