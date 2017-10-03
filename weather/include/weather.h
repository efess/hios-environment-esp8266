#include "user_interface.h"

#define HTTP_BUFF_SIZE 2048

typedef enum {
    WEATHER_CLEAR,
    WEATHER_PARTLY_CLOUDY,
    WEATHER_CLOUDY,
    WEATHER_RAIN,
    WEATHER_SLEET,
    WEATHER_SNOW,
    WEATHER_THUNDER,
    WEATHER_PARTLY_CLOUDY_NIGHT,
    WEATHER_CLEAR_NIGHT
} weather_icon;

typedef struct {
    uint8_t current_icon;
    int8_t temp;
} WeatherState;

typedef struct {
    uint8_t buf[HTTP_BUFF_SIZE];
    uint16_t buf_pos;
    uint16_t length;
    uint8_t receivedHeader;
    struct espconn* con;
} weather_get_context;

extern WeatherState *weather_state;
void weather_start();
void weather_stop();
void weather_init();
void weather_download(void *arg);