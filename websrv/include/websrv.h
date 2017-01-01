#include "user_interface.h"

#define WEB_SRV_BUF 1024

typedef struct {
    uint8_t buf[WEB_SRV_BUF];
    uint8_t *identity;

    void *next;
} Client;

typedef struct {
    struct espconn *con;
    uint8_t buf[WEB_SRV_BUF];
} WebSrv;

void websrv_listen(WebSrv *srv, uint16_t port);
void websrv_receive(void *arg, char *pdata, unsigned short len);