	/*
	 * File      : spansion.c
	 * COPYRIGHT (C) 2017, TaiSync Development Team
	 *
	 * Change Logs:
	 * Date           Author       Notes
	 * 2017-12-21     hgz      		 the first version
	 * …
	*/
#include "spi_flash.h"
#include "spi_flash_internal.h"
#include "rtthread.h"
#include "stdlib.h"

/* S25FLxx-specific commands */
#define CMD_S25FLXX_READ			0x03	/* Read Data Bytes */
#define CMD_S25FLXX_FAST_READ		0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_S25FLXX_READID			0x90	/* Read Manufacture ID and Device ID */
#define CMD_S25FLXX_WREN			0x06	/* Write Enable */
#define CMD_S25FLXX_WRDI			0x04	/* Write Disable */
#define CMD_S25FLXX_RDSR			0x05	/* Read Status Register */
#define CMD_S25FLXX_WRSR			0x01	/* Write Status Register */
#define CMD_S25FLXX_PP				0x02	/* Page Program */
#define CMD_S25FLXX_SE				0xd8	/* Sector Erase */
#define CMD_S25FLXX_BE				0xc7	/* Bulk Erase */
#define CMD_S25FLXX_DP				0xb9	/* Deep Power-down */
#define CMD_S25FLXX_RES				0xab	/* Release from DP, and Read Signature */

#define SPSN_ID_S25FL128P			0x2018
#define SPSN_EXT_ID_S25FL128P_256KB	0x4D00
#define SPSN_EXT_ID_S25FL128P_64KB	0x4D01

#define SPANSION_SR_WIP				(1 << 0)	/* Write-in-Progress */

#define min(x, y)					(((x) < (y)) ? (x) : (y))

struct spansion_spi_flash_params
{
	uint16_t idcode1;
	uint16_t idcode2;
	uint16_t page_size;
	uint16_t pages_per_sector;
	uint16_t nr_sectors;
	const char *name;
};

struct spansion_spi_flash
{
	struct spi_flash flash;
	const struct spansion_spi_flash_params *params;
};

static inline struct spansion_spi_flash *to_spansion_spi_flash(struct spi_flash *flash)
{
	return (struct spansion_spi_flash *)flash;
}

static const struct spansion_spi_flash_params spansion_spi_flash_table[] =
{
	{
		.idcode1 = SPSN_ID_S25FL128P,
		.idcode2 = SPSN_EXT_ID_S25FL128P_64KB,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "S25FL128P_64K",
	},
	{
		.idcode1 = 0xBA18,
		.idcode2 = 0x1000,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "N25Q128A",
	},	
	{
		.idcode1 = SPSN_ID_S25FL128P,
		.idcode2 = SPSN_EXT_ID_S25FL128P_256KB,
		.page_size = 256,
		.pages_per_sector = 1024,
		.nr_sectors = 64,
		.name = "S25FL128P_256K",
	}
};

static int spansion_wait_ready(struct spi_flash *flash, unsigned int timeout)
{
	struct rt_spi_device *spi = flash->spi;
	int ret;
	uint8_t status;

	do {
		ret = spi_flash_cmd(spi, CMD_S25FLXX_RDSR, &status, sizeof(status));
		if (ret < 0)
			return -1;

		if ((status & SPANSION_SR_WIP) == 0)
			break;
		
		rt_thread_delay(2);

	} while (timeout--);


	if ((status & SPANSION_SR_WIP) == 0)
		return 0;

	/* Timed out */
	return -1;
}

static int spansion_read_fast(struct spi_flash *flash,
			     uint32_t offset, size_t len, void *buf)
{
	struct spansion_spi_flash *spsn = to_spansion_spi_flash(flash);
	unsigned long page_addr;
	unsigned long page_size;
	uint8_t cmd[5];

	page_size = spsn->params->page_size;
	page_addr = offset / page_size;

	cmd[0] = CMD_S25FLXX_FAST_READ;
	cmd[1] = page_addr >> 8;
	cmd[2] = page_addr;
	cmd[3] = offset % page_size;
	cmd[4] = 0x00;

//	rt_kprintf
//		("READ: 0x%x => cmd = { 0x%02x 0x%02x%02x%02x%02x } len = 0x%x\n",
//		 offset, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], len);

	return spi_flash_read_common(flash, cmd, sizeof(cmd), buf, len);
}

static int spansion_write(struct spi_flash *flash,
			 uint32_t offset, size_t len, const void *buf)
{
	struct spansion_spi_flash *spsn = to_spansion_spi_flash(flash);
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	size_t chunk_len;
	size_t actual;
	int ret;
	uint8_t cmd[4];

	page_size = spsn->params->page_size;
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	ret = 0;
	for (actual = 0; actual < len; actual += chunk_len)
	{
		chunk_len = min(len - actual, page_size - byte_addr);

		cmd[0] = CMD_S25FLXX_PP;
		cmd[1] = page_addr >> 8;
		cmd[2] = page_addr;
		cmd[3] = byte_addr;

		ret = spi_flash_cmd(flash->spi, CMD_S25FLXX_WREN, NULL, 0);
		if (ret < 0) {
			rt_kprintf("SF: Enabling Write failed\n");
			break;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, 4, (unsigned char *)buf + actual, chunk_len);
		if (ret < 0) {
			rt_kprintf("SF: SPANSION Page Program failed\n");
			break;
		}

		ret = spansion_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret < 0) {
			rt_kprintf("SF: SPANSION page programming timed out\n");
			break;
		}

		page_addr++;
		byte_addr = 0;
	}

//	rt_kprintf("SF: SPANSION: Successfully programmed %u bytes @ 0x%x\n",
//	      len, offset);

	return ret;
}

int spansion_erase(struct spi_flash *flash, uint32_t offset, size_t len)
{
	struct spansion_spi_flash *spsn = to_spansion_spi_flash(flash);
	unsigned long sector_size;
	size_t actual;
	int ret;
	uint8_t cmd[4];

	/*
	 * This function currently uses sector erase only.
	 * probably speed things up by using bulk erase
	 * when possible.
	 */

	sector_size = spsn->params->page_size * spsn->params->pages_per_sector;

	if (offset % sector_size || len % sector_size)
	{
		rt_kprintf("SF: Erase offset/length not multiple of sector size\n");
		return -1;
	}

	len /= sector_size;
	cmd[0] = CMD_S25FLXX_SE;
	cmd[2] = 0x00;
	cmd[3] = 0x00;

	ret = 0;
	for (actual = 0; actual < len; actual++)
	{
		cmd[1] = (offset / sector_size) + actual;

		ret = spi_flash_cmd(flash->spi, CMD_S25FLXX_WREN, NULL, 0);
		if (ret < 0) {
			rt_kprintf("SF: Enabling Write failed\n");
			break;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, 4, NULL, 0);
		if (ret < 0) {
			rt_kprintf("SF: SPANSION page erase failed\n");
			break;
		}

		/* Up to 2 seconds */
		ret = spansion_wait_ready(flash, SPI_FLASH_PAGE_ERASE_TIMEOUT);
		if (ret < 0) {
			rt_kprintf("SF: SPANSION page erase timed out\n");
			break;
		}
	}

	rt_kprintf("SF: SPANSION: Successfully erased %u bytes @ 0x%x\n",
	      len * sector_size, offset);

	return ret;
}

struct spi_flash *spi_flash_probe_spansion(struct rt_spi_device *spi, uint8_t *idcode)
{
	const struct spansion_spi_flash_params *params;
	struct spansion_spi_flash *spsn;
	unsigned int i;
	unsigned short jedec, ext_jedec;

	jedec = idcode[1] << 8 | idcode[2];
	ext_jedec = idcode[3] << 8 | idcode[4];

	for (i = 0; i < ARRAY_SIZE(spansion_spi_flash_table); i++)
	{
		params = &spansion_spi_flash_table[i];
		if (params->idcode1 == jedec)
		{
			if (params->idcode2 == ext_jedec)
				break;
		}
	}

	if (i == ARRAY_SIZE(spansion_spi_flash_table))
	{
		rt_kprintf("SF: Unsupported SPANSION ID %04x %04x\n", jedec, ext_jedec);
		return NULL;
	}

	spsn = malloc(sizeof(struct spansion_spi_flash));
	if (!spsn)
	{
		rt_kprintf("SF: Failed to allocate memory\n");
		return NULL;
	}

	spsn->params = params;
	spsn->flash.spi = spi;
	spsn->flash.name = params->name;

	spsn->flash.write = spansion_write;
	spsn->flash.erase = spansion_erase;
	spsn->flash.read = spansion_read_fast;
	spsn->flash.size = params->page_size * params->pages_per_sector
	    * params->nr_sectors;

	rt_kprintf("SF: Detected %s with page size %u, total %u bytes\n",
	      params->name, params->page_size, spsn->flash.size);

	return &spsn->flash;
}
