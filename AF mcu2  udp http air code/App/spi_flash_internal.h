#ifndef _SPI_FLASH_INTERNAL_H_
#define _SPI_FLASH_INTERNAL_H_

#define SPI_FLASH_PROG_TIMEOUT			(2000)
#define SPI_FLASH_PAGE_ERASE_TIMEOUT	(5000)
#define SPI_FLASH_SECTOR_ERASE_TIMEOUT	(10000)

#define ARRAY_SIZE(arr)					(sizeof(arr)/sizeof((arr)[0]))
	
#define CMD_READ_ID						0x9f

#define SPANSION_RESET					0xF0

#define CMD_EX4B						0xE9
#define CMD_EN4B						0xB7

/* Send a single-byte command to the device and read the response */
int spi_flash_cmd(struct rt_spi_device *spi, uint8_t cmd, void *response, size_t len);

/*
 * Send a multi-byte command to the device and read the response. Used
 * for flash array reads, etc.
 */
int spi_flash_cmd_read(struct rt_spi_device *spi, const uint8_t *cmd,
		size_t cmd_len, void *data, size_t data_len);

/*
 * Send a multi-byte command to the device followed by (optional)
 * data. Used for programming the flash array, etc.
 */
int spi_flash_cmd_write(struct rt_spi_device *spi, const uint8_t *cmd, size_t cmd_len,
		const void *data, size_t data_len);

/*
 * Same as spi_flash_cmd_read() except it also claims/releases the SPI
 * bus. Used as common part of the ->read() operation.
 */
int spi_flash_read_common(struct spi_flash *flash, const uint8_t *cmd,
		size_t cmd_len, void *data, size_t data_len);

/* Manufacturer-specific probe functions */
struct spi_flash *spi_flash_probe_spansion(struct rt_spi_device *spi, uint8_t *idcode);
struct spi_flash *spi_flash_probe_macronix(struct rt_spi_device *spi, uint8_t *idcode);
struct spi_flash *spi_flash_probe_w25qxx(struct rt_spi_device *spi, uint8_t *idcode);
#endif
