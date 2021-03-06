#ifndef RUN_STATE_H_
#define RUN_STATE_H_

#include "user_interface.h"

typedef struct {
    uint8_t wifi_cfg_update;
    uint8_t wifi_access_status;

    uint32_t temp;
    uint32_t humidity;
    uint32_t pressure;
} RunState;

extern RunState run_state;

#endif