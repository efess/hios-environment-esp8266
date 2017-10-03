#include "weather.h"
#include "espconn.h"
#include "info.h"
#include "mem.h"
#include "http.h"
#include "jsonh.h"
#include "user_config.h"

WeatherState *weather_state;
static ETSTimer _weather_update_timer;

void ICACHE_FLASH_ATTR weather_free_context(weather_get_context* context) 
{
    //espconn_disconnect(context->con);
    os_free(context->con->proto.tcp);
    os_free(context->con);
    os_free(context);
}


void ICACHE_FLASH_ATTR weather_disconcb(void *arg)
{
    struct espconn *con = (struct espconn *)arg;
    
    weather_get_context* context = (weather_get_context *)con->reverse;
    weather_free_context(context);

    INFO("Weather End - free heap size: %u\r\n",  system_get_free_heap_size());
    os_timer_setfn(&_weather_update_timer, (os_timer_func_t *)weather_download, NULL);
    os_timer_arm(&_weather_update_timer, 5000, 0);
}

void ICACHE_FLASH_ATTR weather_reconcb_cb(void *arg, sint8 err)
{
    struct espconn *con = (struct espconn *)arg;
    weather_get_context* context = (weather_get_context *)con->reverse;
    INFO("Weather: Connection was aborted, error: %d \r\n", err);
    
    weather_free_context(context);

    os_timer_setfn(&_weather_update_timer, (os_timer_func_t *)weather_download, NULL);
    os_timer_arm(&_weather_update_timer, 5000, 0);
}

uint8_t ICACHE_FLASH_ATTR weather_get_icon_from_string(uint8_t *name)
{
    if(os_strcmp(name, "cloudy") == 0)
    {
        return WEATHER_CLOUDY;
    }
    else if(os_strcmp(name, "partlycloudy") == 0 ||
        os_strcmp(name, "partlysunny") == 0 ||
        os_strcmp(name, "mostlycloudy") == 0 ||
        os_strcmp(name, "mostlysunny") == 0)
    {
        return WEATHER_PARTLY_CLOUDY;
    }
    else if(os_strcmp(name, "clear") == 0 || os_strcmp(name, "sunny") == 0)
    {
        return WEATHER_CLEAR;
    }
    else if(os_strcmp(name, "rain") == 0)
    {
        return WEATHER_RAIN;
    }
    else if(os_strcmp(name, "tstorms") == 0)
    {
        return WEATHER_THUNDER;
    }
    else if(os_strcmp(name, "sleet") == 0 || os_strcmp(name, "flurries") == 0)
    {
        return WEATHER_SLEET;
    }
    else if(os_strcmp(name, "snow") == 0)
    {
        return WEATHER_SNOW;
    }
    // ???
    return WEATHER_CLEAR_NIGHT;
}

void ICACHE_FLASH_ATTR weather_parse_json(uint8_t *buf)
{
    uint8_t type = 0;
    int8_t err;
    uint8_t temp[50];
    int tempInt;

    struct jsonparse_state root;
    struct jsonparse_state state;
    struct jsonparse_state array;
    jsonparse_setup(&root, buf, os_strlen(buf));
    jsonparse_next(&root);

    json_find_next_sibling(&root, "current", &state);
    jsonparse_next(&state);
    json_find_next_sibling_string(&state, (uint8_t*)"icon", temp, 50, &err);
    INFO("cond %d\r\n", temp);
    weather_state->current_icon = weather_get_icon_from_string(temp);

    json_find_next_sibling_int(&state, (uint8_t*)"temp", &tempInt, &err);
    INFO("temp %d\r\n", tempInt);
    weather_state->temp = (int8_t)tempInt;
    
    json_find_next_sibling(&root, "forecast", &state);
    
    // TODO
    // type = jsonparse_next(&state);
    // array = state;


    // for (int i = 0; i < 7 && type; i++)
    // {
    //     state = array;

    //     jsonparse_next(&state);
    //     if (!type) {
    //         break;
    //     }
    //     json_find_next_sibling_string(&state, (uint8_t*)"tempHigh", temp, 50, &err);

    //     type = json_find_next_element(&array, &array);
    // }
}

void ICACHE_FLASH_ATTR  weather_connected_cb(void *arg)
{
    struct espconn *con = (struct espconn *)arg;
    uint16_t byte_count;
    
    weather_get_context* context = (weather_get_context *)con->reverse;

    INFO("Weather: Connected\r\n");

    HttpRequest req;
    req.url = WEATHER_URL;
    req.verb = HTTP_POST;

    byte_count = http_write_request_header(&req, context->buf, HTTP_BUFF_SIZE);
    espconn_send(con, context->buf, byte_count);
}

void ICACHE_FLASH_ATTR weather_receive_cb(void *arg, char *pdata, unsigned short len)
{
    struct espconn *con = (struct espconn *)arg;
    uint16_t byte_count = len;
    uint16_t pdata_offset = 0;
    int err = 0;

    weather_get_context* context = (weather_get_context *)con->reverse;

    if(!context->receivedHeader)
    {
        context->receivedHeader = true;
        os_memset(context->buf, 0, sizeof(context->buf));

        HttpResponse resp;
        pdata_offset = http_parse_response_header(&resp, pdata, len, &err);
        if(err != HTTP_OK)
        {
            INFO("Weather: Http response parse error\r\n");
            return;
        }

        if(!resp.length)
        {
            INFO("Weather: No data returned, or http failure\r\n");
            return;
        }

        context->length = resp.length;
        byte_count = len - pdata_offset;
    }
    
    os_memcpy(context->buf + context->buf_pos, pdata + pdata_offset, byte_count);
    context->buf_pos += byte_count;
    if(context->buf_pos == context->length) 
    {
        weather_parse_json(context->buf);
        espconn_disconnect(con);
    }
}

void ICACHE_FLASH_ATTR weather_dns_cb(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *con = (struct espconn *)arg;
    weather_get_context* context = (weather_get_context *)con->reverse;

    if(ipaddr && ipaddr->addr != 0) 
    {
        INFO("Weather: found ip %d.%d.%d.%d\r\n",
                *((uint8 *) &ipaddr->addr),
                *((uint8 *) &ipaddr->addr + 1),
                *((uint8 *) &ipaddr->addr + 2),
                *((uint8 *) &ipaddr->addr + 3));
                
        os_memcpy(con->proto.tcp->remote_ip, &ipaddr->addr, 4);
        
        espconn_connect(con);
    } 
    else 
    {
        INFO("Weather: Cannot resolve host DNS\r\n");
        weather_free_context(context);
    }
}

void ICACHE_FLASH_ATTR weather_download(void *arg)
{
    INFO("Weather Start - free heap size: %u\r\n",  system_get_free_heap_size());
    os_timer_disarm(&_weather_update_timer);
    uint16_t port = 0;
    if(http_get_port_from_url(WEATHER_URL, &port) < 0) {
        return;
    }

    weather_get_context *get_context = (weather_get_context*)os_zalloc(sizeof(weather_get_context));

    get_context->con = (struct espconn *)os_zalloc(sizeof(struct espconn));
    get_context->con->type = ESPCONN_TCP;
    get_context->con->state = ESPCONN_NONE;
    get_context->con->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    get_context->con->proto.tcp->remote_port = port;
    get_context->con->proto.tcp->local_port = espconn_port();
    get_context->con->reverse = get_context;

    espconn_regist_recvcb(get_context->con, weather_receive_cb);
    espconn_regist_connectcb(get_context->con, weather_connected_cb);
    espconn_regist_reconcb(get_context->con, weather_reconcb_cb);
    espconn_regist_disconcb(get_context->con, weather_disconcb);
    
    uint8 result = 0;
    uint8 host[100];
    ip_addr_t ip;

    INFO("Weather: Looking up host\r\n");
    if (http_get_host_from_url(WEATHER_URL, host) < 0) {
        INFO("Weather: Invalid URL, couldn't parse host\r\n");
        return;
    }
    
    INFO("Weather: host: %s\r\n", host);
    espconn_gethostbyname(get_context->con, host, &ip, weather_dns_cb);
}

void ICACHE_FLASH_ATTR weather_init() 
{
    weather_state = (WeatherState*)os_zalloc(sizeof(WeatherState));
    os_timer_setfn(&_weather_update_timer, (os_timer_func_t *)weather_download, NULL);
}

void ICACHE_FLASH_ATTR weather_start() 
{
    os_timer_arm(&_weather_update_timer, 5000, 0);
}

void ICACHE_FLASH_ATTR weather_stop() 
{    
    os_timer_disarm(&_weather_update_timer);
}