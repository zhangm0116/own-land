#ifndef _I2C_BUS_H_
#define _I2C_BUS_H_

#include "rtthread.h"
#include "stdint.h"

#define RT_I2C_WR                0x0000
#define RT_I2C_RD               (1u << 0)
#define RT_I2C_ADDR_10BIT       (1u << 2)  /* this is a ten bit chip address */
#define RT_I2C_NO_START         (1u << 4)
#define RT_I2C_IGNORE_NACK      (1u << 5)
#define RT_I2C_NO_READ_ACK      (1u << 6)  /* when I2C reading, we do not ACK */

struct i2c_msg {
	uint16_t 	addr;		/* slave address			*/
	uint16_t 	flags;
	uint16_t 	len;		/* msg length				*/
	uint8_t		*buf;		/* pointer to msg data		*/
};

struct rt_i2c_bus_device
{
	struct rt_device 			parent;
	const struct rt_i2c_ops 	*ops;
    struct rt_mutex 			lock;
	uint32_t					timeout;
    uint32_t					retries;
    void						*priv;
};

struct rt_i2c_ops
{
    int32_t (*xfer)(struct rt_i2c_bus_device *dev, struct i2c_msg *msgs, int32_t num);
};

int32_t rt_i2c_bit_add_bus(struct rt_i2c_bus_device *bus, const char *bus_name);
int32_t rt_i2c_bus_register(struct rt_i2c_bus_device *bus, const char *name);
int32_t i2c_transfer(struct rt_i2c_bus_device *dev, struct i2c_msg *msgs, uint32_t num);
int32_t rt_i2c_master_send(struct rt_i2c_bus_device *bus, uint16_t addr, const uint8_t *buf, uint32_t count);
int32_t rt_i2c_master_recv(struct rt_i2c_bus_device *bus,uint16_t addr, uint8_t *buf, uint32_t count);
int32_t rt_i2c_master_eeprom_read(struct rt_i2c_bus_device *bus, uint16_t addr, uint16_t offset, uint8_t off_len, uint8_t *buf, uint32_t count);
int32_t rt_i2c_master_eeprom_write(struct rt_i2c_bus_device *bus, uint16_t addr, uint16_t offset, uint8_t off_len, uint8_t *buf, uint32_t count);
#endif
