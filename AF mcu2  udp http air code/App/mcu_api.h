#ifndef _MCU_API_H_
#define _MCU_API_H_

#include "stdint.h"
#include "spi_bus.h"

int mcu_flash_write(struct rt_spi_device *spi, uint32_t offset, uint32_t len, const void *buf);
int mcu_flash_erase(struct rt_spi_device *spi, uint32_t offset, uint32_t len);

int mcu_set_vga(struct rt_spi_device *spi, uint32_t vga);
int mcu_set_freq(struct rt_spi_device *spi, uint32_t freq, uint8_t txrx);
int mcu_set_mode(struct rt_spi_device *spi, uint8_t mode, uint8_t txrx);
int mcu_get_status(struct rt_spi_device *spi, void *buf, uint32_t len);
#endif
