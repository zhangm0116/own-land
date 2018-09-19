#include "i2c_bus.h"
#include "string.h"

int32_t rt_i2c_bus_register(struct rt_i2c_bus_device *bus, const char *name)
{
	struct rt_device *device;
    RT_ASSERT(bus != RT_NULL);

    device = &bus->parent;
	
	device->user_data = bus;
    /* set device type */
    device->type    = RT_Device_Class_I2CBUS;
    /* initialize device interface */
    device->init    = RT_NULL;
    device->open    = RT_NULL;
    device->close   = RT_NULL;
    device->read    = RT_NULL;
    device->write   = RT_NULL;
    device->control = RT_NULL;

    /* register to device manager */
    rt_device_register(device, name, RT_DEVICE_FLAG_RDWR);
	
	/* initialize mutex lock */
    rt_mutex_init(&(bus->lock), name, RT_IPC_FLAG_FIFO);

	return 0;
}

int32_t i2c_transfer(struct rt_i2c_bus_device *bus, struct i2c_msg *msgs, uint32_t num)
{
	int32_t ret;
	
    RT_ASSERT(bus != RT_NULL);
    RT_ASSERT(msgs != RT_NULL);

    if (bus->ops->xfer)
    {
        rt_mutex_take(&bus->lock, RT_WAITING_FOREVER);
        ret = bus->ops->xfer(bus, msgs, num);
        rt_mutex_release(&bus->lock);

        return ret;
    }
    else
    {
        return 0;
    }
}

int32_t rt_i2c_master_send(struct rt_i2c_bus_device *bus, uint16_t addr, const uint8_t *buf, uint32_t count)
{
    int32_t ret;
    struct i2c_msg msg;
	RT_ASSERT(bus != RT_NULL);
	RT_ASSERT(buf != RT_NULL);

    msg.addr  = addr;
    msg.flags = RT_I2C_WR;
    msg.len   = count;
    msg.buf   = (uint8_t *)buf;

    ret = i2c_transfer(bus, &msg, 1);

    return (ret > 0) ? count : ret;
}

int32_t rt_i2c_master_recv(struct rt_i2c_bus_device *bus, uint16_t addr, uint8_t *buf, uint32_t count)
{
    int32_t ret;
    struct i2c_msg msg;
    RT_ASSERT(bus != RT_NULL);
	RT_ASSERT(buf != RT_NULL);
	
    msg.addr   = addr;
    msg.flags  = RT_I2C_RD;
    msg.len    = count;
    msg.buf    = buf;

    ret = i2c_transfer(bus, &msg, 1);

    return (ret > 0) ? count : ret;
}

int32_t rt_i2c_master_eeprom_read(struct rt_i2c_bus_device *bus, uint16_t addr, uint16_t offset, uint8_t off_len, uint8_t *buf, uint32_t count)
{
	int32_t ret;
    struct i2c_msg msg[2];
	uint8_t offset_address[2];
    RT_ASSERT(bus != RT_NULL);
	RT_ASSERT(buf != RT_NULL);
	
	if((off_len != 2) && (off_len != 1))
	{
		return (-1);
	}
	
	if(off_len == 2)
	{
		offset_address[0] = offset >> 8;
		offset_address[1] = offset;
	}
	else
	{
		offset_address[0] = offset;
	}
	
	msg[0].addr		= addr;
	msg[0].flags	= RT_I2C_WR;
	msg[0].buf		= offset_address;
	msg[0].len		= off_len;
	
	msg[1].addr		= addr;
	msg[1].flags	= RT_I2C_RD;
	msg[1].buf		= buf;
	msg[1].len		= count;
	
	ret = i2c_transfer(bus, &msg[0], 2);

    return (ret > 0) ? count : ret;
}

int32_t rt_i2c_master_eeprom_write(struct rt_i2c_bus_device *bus, uint16_t addr, uint16_t offset, uint8_t off_len, uint8_t *buf, uint32_t count)
{
	int32_t ret;
    struct i2c_msg msg;
	uint8_t tx_buf[18];
	
	RT_ASSERT(bus != RT_NULL);
	RT_ASSERT(buf != RT_NULL);
	
	if((off_len != 2) && (off_len != 1))
	{
		return (-1);
	}
	
	if(count > 16)
	{
		return (-1);
	}
	
	if(off_len == 2)
	{
		tx_buf[0] = offset >> 8;
		tx_buf[1] = offset;
		memcpy(&tx_buf[2], buf, count);
	}
	else
	{
		tx_buf[0] = offset;
		memcpy(&tx_buf[1], buf, count);
	}
	
	
    msg.addr  = addr;
    msg.flags = RT_I2C_WR;
    msg.len   = count + off_len;
    msg.buf   = (uint8_t *)tx_buf;

    ret = i2c_transfer(bus, &msg, 1);
	
	rt_thread_delay(20);

    return (ret > 0) ? count : ret;
}
