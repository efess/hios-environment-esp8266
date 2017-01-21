#include "wifi_scan.h"
#include "mem.h"
#include "osapi.h"
#include "info.h"

ScanState wifi_scan_state;

void ICACHE_FLASH_ATTR wifi_clear_list() 
{
    uint8_t c = 0;
    for(c = 0; c < wifi_scan_state.ap_count; c++)
    {
        os_free(wifi_scan_state.ap_list[c]->ssid);
        os_free(wifi_scan_state.ap_list[c]);
        wifi_scan_state.ap_list[c] = 0;
    }
    wifi_scan_state.ap_count = 0;
}

void ICACHE_FLASH_ATTR wifi_start_scan()
{
    wifi_clear_list();
    wifi_scan_state.ap_scan_status = SCANNING;
    wifi_station_scan(NULL, wifi_scan_finish);
}

void ICACHE_FLASH_ATTR wifi_scan_finish(void *args, STATUS status)
{
    if(status != OK)
    {
        wifi_scan_state.ap_scan_status = SCAN_ERROR;
        INFO("WifiScan: Failed %u\r\n", status);
    }
    wifi_scan_state.ap_scan_status = SCAN_FINISHED;
    struct bss_info *bss_link = (struct bss_info*)args;
    bss_link = bss_link->next.stqe_next;//ignore the first one , it's invalid.

    uint8_t c = 0;
    for(c = 0; c < MAX_AP_LIST && bss_link != NULL; c++)
    {
        ApInfo *info = (ApInfo*)os_zalloc(sizeof(ApInfo));
        info->ssid = (uint8_t*)os_zalloc(os_strlen(bss_link->ssid) + 1);

        os_strncpy(info->ssid, bss_link->ssid, os_strlen(bss_link->ssid));

        info->signal = bss_link->rssi;
        info->auth = bss_link->authmode;
        wifi_scan_state.ap_list[c] = info;

        bss_link = bss_link->next.stqe_next;
    }

    wifi_scan_state.ap_count = c;
}