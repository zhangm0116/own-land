#ifndef _SPI_BUS_H_
#define _SPI_BUS_H_

#include "rtthread.h"
#include "stdint.h"

#define RT_SPI_CPHA     (1<<0)                             /* bit[0]:CPHA, clock phase */
#define RT_SPI_CPOL     (1<<1)                             /* bit[1]:CPOL, clock polarity */

#define RT_SPI_LSB      (0<<2)                             /* bit[2]: 0-LSB */
#define RT_SPI_MSB      (1<<2)                             /* bit[2]: 1-MSB */

#define RT_SPI_MASTER   (0<<3)                             /* SPI master device */
#define RT_SPI_SLAVE    (1<<3)                             /* SPI slave device */

#define RT_SPI_MODE_0       (0 | 0)                        /* CPOL = 0, CPHA = 0 */
#define RT_SPI_MODE_1       (0 | RT_SPI_CPHA)              /* CPOL = 0, CPHA = 1 */
#define RT_SPI_MODE_2       (RT_SPI_CPOL | 0)              /* CPOL = 1, CPHA = 0 */
#define RT_SPI_MODE_3       (RT_SPI_CPOL | RT_SPI_CPHA)    /* CPOL = 1, CPHA = 1 */

#define RT_SPI_MODE_MASK    (RT_SPI_CPHA | RT_SPI_CPOL | RT_SPI_MSB)

struct rt_spi_message
{
    const void 				*send_buf;
    void 					*recv_buf;
    uint32_t 				length;
    struct rt_spi_message 	*next;
    unsigned 				cs_take    : 1;
    unsigned 				cs_release : 1;
};

struct rt_spi_configuration
{
    uint8_t	 	mode;
    uint8_t 	data_width;
    uint16_t 	reserved;
    uint32_t 	max_hz;
};

struct rt_spi_ops;

struct rt_spi_bus
{
    struct rt_device 			parent;
    const struct rt_spi_ops 	*ops;
    struct rt_mutex 			lock;
    struct rt_spi_device 		*owner;
};

struct rt_spi_ops
{   
    int32_t (*configure)(struct rt_spi_device *device, struct rt_spi_configuration *configuration);
    int32_t (*xfer)(struct rt_spi_device *device, struct rt_spi_message *message);
};

struct rt_spi_device
{
    struct rt_device 				parent;
    struct rt_spi_bus 				*bus;
    struct rt_spi_configuration 	config;
};

int32_t rt_spi_bus_register(struct rt_spi_bus *bus, const char *name, const struct rt_spi_ops *ops);
int32_t rt_spi_bus_attach_device(struct rt_spi_device *device, const char *name, const char *bus_name, void *user_data);
int32_t rt_spi_transfer(struct rt_spi_device *device, const void *send_buf, void *recv_buf, uint32_t length);
int32_t rt_spi_send_then_recv(struct rt_spi_device *device, const void *send_buf, uint32_t send_length, void *recv_buf, uint32_t recv_length);
int32_t rt_spi_send_then_send(struct rt_spi_device *device, const void *send_buf1, uint32_t send_length1, const void *send_buf2, uint32_t send_length2);
#endif
