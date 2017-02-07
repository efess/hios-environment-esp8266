#ifndef INCLUDE_HSPI_H_
#define INCLUDE_HSPI_H_

#include "driver/spi_register.h" // from 0.9.4 IoT_Demo
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "gpio.h"

#define SPI         0
#define HSPI        1
#define SPIFIFOSIZE 16 //16 words length

extern uint32_t *spi_fifo;

void hspi_init(void);
void hspi_send_data(const uint8_t * data, uint8_t datasize);
void hspi_send_uint16_r(const uint16_t data, int32_t repeats);
void hspi_wait_ready(void);


#endif /* INCLUDE_HSPI_H_ */
