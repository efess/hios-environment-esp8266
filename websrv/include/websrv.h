#include "user_interface.h"
#include "resource_handler.h"
#include "http.h"

#define WEB_SRV_MAX_CON 10
#define WEB_SRV_BUF 1024

typedef enum {
    CLIENT_IDLE,
    // Client is receiving data from the server
    CLIENT_RECEIVING,
    // CLient is sending data to the server
    CLIENT_SENDING
} ClientState;


typedef struct {
    uint8_t buf[WEB_SRV_BUF];
    uint8_t *identity;
    ClientState state;

    ResourceHandler handler;
    uint8_t *handler_context;
        
    HttpRequest *current_client_request;
    
    // both receive and send counters
    uint32_t current_size;
    uint32_t current_byte_offset;
    
    uint8_t *resource;
    void *next;
} Client;

typedef struct {
    struct espconn *con;
} WebSrv;

void websrv_listen(WebSrv *srv, uint16_t port);
void websrv_destroy(WebSrv *srv);