#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#include "spi_bus.h"
//#include "stm32f10x.h"
#include "sys/types.h"

//#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_SPANSION
//#define CONFIG_SPI_FLASH_W25QXX
#define CONFIG_SPI_FLASH_MICRON


struct spi_flash
{
	struct rt_spi_device	*spi;
	const char				*name;
	uint32_t				size;
	int (*read)(struct spi_flash *flash, uint32_t offset, size_t len, void *buf);
	int (*write)(struct spi_flash *flash, uint32_t offset, size_t len, const void *buf);
	int (*erase)(struct spi_flash *flash, uint32_t offset, size_t len);
};

struct spi_flash *spi_flash_probe(const char *name);
		
void spi_flash_free(struct spi_flash *flash);

static inline int spi_flash_read(struct spi_flash *flash, uint32_t offset, size_t len, void *buf)
{
	return flash->read(flash, offset, len, buf);
}

static inline int spi_flash_write(struct spi_flash *flash, uint32_t offset, size_t len, const void *buf)
{
	return flash->write(flash, offset, len, buf);
}

static inline int spi_flash_erase(struct spi_flash *flash, uint32_t offset, size_t len)
{
	return flash->erase(flash, offset, len);
}

static void delay_ms(uint16_t time)
{    
	uint16_t i=0;  
	while(time--)
	{
		i=12000;
		while(i--) ;    
	}
}
#endif
