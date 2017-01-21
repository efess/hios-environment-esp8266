#include "user_interface.h"
#define MAX_AP_LIST 20

typedef enum  {
    NOT_SCANNED,
    SCANNING,
    SCAN_FINISHED,
    SCAN_ERROR
}ApScanStatus;

typedef struct {
    uint8_t *ssid;
    sint8_t signal;
    uint8_t auth;
} ApInfo;

typedef struct {
    ApInfo *ap_list[MAX_AP_LIST];
    uint8_t ap_count;
    ApScanStatus ap_scan_status;
} ScanState;

extern ScanState wifi_scan_state;
void wifi_scan_finish(void *args, STATUS status);
void wifi_start_scan();