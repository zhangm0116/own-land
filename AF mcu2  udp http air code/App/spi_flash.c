	/*
	 * File      : spi_flash.c
	 * COPYRIGHT (C) 2017, TaiSync Development Team
	 *
	 * Change Logs:
	 * Date           Author       Notes
	 * 2017-12-21     hgz      		 the first version
	 * …
	*/
#include "spi_bus.h"
#include "spi_flash.h"
#include "spi_flash_internal.h"
#include "stdlib.h"
#include "string.h"
#include "rtthread.h" 



int spi_flash_cmd(struct rt_spi_device *spi, uint8_t cmd, void *response, size_t len)
{
	int ret;
	 rt_uint32_t level;
	
//	rt_sem_take(&sem233,RT_WAITING_FOREVER);
	
  ret = rt_spi_send_then_recv(spi, &cmd, 1, response, len);
	
 //rt_sem_release(&sem233);
	
	
  return ret;
}

int spi_flash_cmd_read(struct rt_spi_device *spi, const uint8_t *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	int ret;
	
	ret = rt_spi_send_then_recv(spi, cmd, cmd_len, data, data_len);

	return ret;
}

int spi_flash_cmd_write(struct rt_spi_device *spi, const uint8_t *cmd, size_t cmd_len,
		const void *data, size_t data_len)
{
	int ret;
	
	ret = rt_spi_send_then_send(spi, cmd, cmd_len, data, data_len);

	return ret;
}


int spi_flash_read_common(struct spi_flash *flash, const uint8_t *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	struct rt_spi_device *spi = flash->spi;
	int ret;
	
	ret = rt_spi_send_then_recv(spi, cmd, cmd_len, data, data_len);

	return ret;
}

struct spi_flash *spi_flash_probe(const char *name)
{
	struct rt_spi_device *spi;
	rt_device_t	dev;
	struct spi_flash *flash;
	int ret;
	uint8_t idcode[5];
	
	dev = rt_device_find(name);
	if(dev == RT_NULL)
	{
		return NULL;
	}
	
	spi = (struct rt_spi_device *)dev;
	
	if(strcmp(name, "flash") == 0)
	{
		spi_flash_cmd(spi, SPANSION_RESET, NULL, 0);
		delay_ms(10);
	}

	/* Read the ID codes */
	ret = spi_flash_cmd(spi, CMD_READ_ID, &idcode, sizeof(idcode));
	if (ret)
		goto err_read_id;

	rt_kprintf("SF: Got idcode %02x %02x %02x %02x %02x\n", idcode[0], idcode[1], idcode[2], idcode[3], idcode[4]);

	switch (idcode[0]) {
#ifdef CONFIG_SPI_FLASH_SPANSION
	case 0x01:
		flash = spi_flash_probe_spansion(spi, idcode);
		break;
#endif
#ifdef CONFIG_SPI_FLASH_MACRONIX
	case 0xc2:
		flash = spi_flash_probe_macronix(spi, idcode);
		break;
#endif
#ifdef CONFIG_SPI_FLASH_W25QXX
	case 0xef:
		flash = spi_flash_probe_w25qxx(spi, idcode);
		break;
#endif	
#ifdef CONFIG_SPI_FLASH_MICRON
	case 0x20:
		flash = spi_flash_probe_spansion(spi, idcode);
		break;
#endif		
	default:
		rt_kprintf("SF: Unsupported manufacturer %02X\n", idcode[0]);
		flash = NULL;
		break;
	}

	if (!flash)
		goto err_manufacturer_probe;

	return flash;

err_manufacturer_probe:
err_read_id:
	return NULL;
}

void spi_flash_free(struct spi_flash *flash)
{
	free(flash);
}
