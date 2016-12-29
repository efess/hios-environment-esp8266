#include "user_interface.h"

typedef struct {
    struct espconn *con;
    int somethingElse;
} WebSrv;

void websrv_listen(WebSrv *srv, uint16_t port);
void websrv_receive(void *arg, char *pdata, unsigned short len);