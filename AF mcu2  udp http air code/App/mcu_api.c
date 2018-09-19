/*
 * File      : mcu_api.c
 * COPYRIGHT (C) 2017, TaiSync Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-12-21     hgz      		 the first version
 * …
*/
#include "rtthread.h"
#include "spi_flash.h"
#include "spi_flash_internal.h"

#define MCU_FLASH_SE					0xD8		/* page erase command */
#define MCU_FLASH_PP					0x02		/* page program command */
#define MCU_FLASH_SR					0x05		/* prog/erase status */

#define MCU_SET_VGA						0x90		/* set RF tx power */
#define	MCU_SET_FREQ					0x91		/* set RF lo frequency */
#define MCU_SET_MODE					0x92		/* set baseband qam/ldpc mode */
#define VIDEO_SET_RATE					0x94		/* set video code rate */
#define VIDEO_SET_MODE					0x95		/* set video compression format */
#define MCU_GET_STATUS					0x96		/* get baseband/RF status */
#define MCU_GET_STATUS_ENABLE			0x97		/* enable to get status */
#define ANTENNA_SW							 0x98

#define MCU_FLASH_SR_WIP				(1 << 0)	/* Write-in-Progress */
#define MCU_PORG_TIMEOUT				(5000)
#define min(x, y)						(((x) < (y)) ? (x) : (y))

 extern struct rt_spi_device 		*device;
 

int mcu_flash_wait_ready(struct rt_spi_device *spi, unsigned int timeout)
{
	int ret;
	uint8_t status;

	do {
		ret = spi_flash_cmd(spi, MCU_FLASH_SR, &status, sizeof(status));
		if (ret < 0)
			return -1;

		if ((status & MCU_FLASH_SR_WIP) == 0)
			break;
		
		delay_ms(1);

	} while (timeout--);


	if ((status & MCU_FLASH_SR_WIP) == 0)
		return 0;

	/* Timed out */
	return -1;
}

int mcu_flash_write(struct rt_spi_device *spi, uint32_t offset, size_t len, const void *buf)
{
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	size_t chunk_len;
	size_t actual;
	int ret;
	uint8_t cmd[4];

	page_size = 256;
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	ret = 0;
	for (actual = 0; actual < len; actual += chunk_len)
	{
		chunk_len = min(len - actual, page_size - byte_addr);

		cmd[0] = MCU_FLASH_PP;
		cmd[1] = page_addr >> 8;
		cmd[2] = page_addr;
		cmd[3] = byte_addr;

		ret = spi_flash_cmd_write(spi, cmd, 4, (unsigned char *)buf + actual, chunk_len);
		if (ret < 0) {
			rt_kprintf("mcu flash Page Program failed\n");
			break;
		}
		
		delay_ms(1);

		ret = mcu_flash_wait_ready(spi, MCU_PORG_TIMEOUT);
		if (ret < 0) {
			rt_kprintf("mcu flash page programming timed out\n");
			break;
		}

		page_addr++;
		byte_addr = 0;
	}

	//rt_kprintf("mcu flash Successfully programmed %u bytes @ 0x%x\n", len, offset);

	return ret;
}
			 
int mcu_flash_erase(struct rt_spi_device *spi, uint32_t offset, size_t len)
{
	unsigned long sector_size;
	size_t actual;
	int ret;
	uint32_t temp;
	uint8_t cmd[4];

	/*
	 * This function currently uses sector erase only.
	 * probably speed things up by using bulk erase
	 * when possible.
	 */

	sector_size = 1024;

	if (offset % sector_size || len % sector_size)
	{
		rt_kprintf("Erase offset/length not multiple of sector size\n");
		return -1;
	}

	len /= sector_size;
	cmd[0] = MCU_FLASH_SE;

	ret = 0;
	for (actual = 0; actual < len; actual++)
	{
		temp = ((offset / sector_size) + actual) * 1024;
		
		cmd[1] = temp >> 16;
		cmd[2] = temp >> 8;
		cmd[3] = temp;

		rt_kprintf("erase addr 0x%02x%02x%02x\n",cmd[1],cmd[2],cmd[3]);
		ret = spi_flash_cmd_write(spi, cmd, 4, NULL, 0);
		if (ret < 0) {
			rt_kprintf("mcu flash page erase failed\n");
			break;
		}
		
		delay_ms(1);

		/* Up to 2 seconds */
		ret = mcu_flash_wait_ready(spi, MCU_PORG_TIMEOUT);
		if (ret < 0) {
			rt_kprintf("mcu flash page erase timed out\n");
			break;
		}
	}

	//rt_kprintf("mcu flash: Successfully erased %u bytes @ 0x%x\n", len * sector_size, offset);

	return ret;
}

int mcu_set_vga(struct rt_spi_device *spi, uint32_t vga)
{
	int ret;
	uint8_t cmd[5];
	
	cmd[0] = MCU_SET_VGA;
	cmd[1] = (vga >> 24) & 0xff;
	cmd[2] = (vga >> 16) & 0xff;
	cmd[3] = (vga >> 8) & 0xff;
	cmd[4] = vga & 0xff;
	
	ret = spi_flash_cmd_write(spi, cmd, 5, NULL, 0);
	if (ret < 0)
	{
		rt_kprintf("set vga failed\n");
	}
	return ret;
}

int mcu_set_freq(struct rt_spi_device *spi, uint32_t freq, uint8_t txrx)
{
	int ret;
	uint8_t cmd[6];
	
	cmd[0] = MCU_SET_FREQ;
	cmd[1] = txrx;
	cmd[2] = (freq >> 24) & 0xff;
	cmd[3] = (freq >> 16) & 0xff;;
	cmd[4] = (freq >> 8) & 0xff;
	cmd[5] = freq & 0xff;
	
	ret = spi_flash_cmd_write(spi, cmd, 6, NULL, 0);
	if (ret < 0)
	{
		rt_kprintf("set freq failed\n");
	}
	return ret;
}

int mcu_set_mode(struct rt_spi_device *spi, uint8_t mode, uint8_t txrx)
{
	int ret;
	uint8_t cmd[3];
	
	cmd[0] = MCU_SET_MODE;
	cmd[1] = txrx;
	cmd[2] = mode;

	
	ret = spi_flash_cmd_write(spi, cmd, 3, NULL, 0);
	if (ret < 0)
	{
		rt_kprintf("set mode failed\n");
	}
	return ret;
}

int mcu_antenna_sw(struct rt_spi_device *spi, uint8_t antenna_sw)
{
	int ret;
	uint8_t cmd[3];
	
	cmd[0] = ANTENNA_SW;
	cmd[1] = antenna_sw;

	rt_kprintf("antenna sw %d\n",antenna_sw);
	
	ret = spi_flash_cmd_write(spi, cmd, 2, NULL, 0);
	if (ret < 0)
	{
		rt_kprintf("set antenna sw failed\n");
	}
	return ret;
}

int mcu_get_status(struct rt_spi_device *spi, void *buf, uint32_t len)
{
	int ret;
	uint8_t cmd;
	
	cmd = MCU_GET_STATUS;
	
	ret = spi_flash_cmd(spi, MCU_GET_STATUS_ENABLE, NULL, 0);
	if (ret < 0)
	{
		rt_kprintf("set vga failed\n");
		
	}
	
	delay_ms(10);
	
	rt_spi_transfer(spi, &cmd, buf, len);
	return ret;   
	
}
