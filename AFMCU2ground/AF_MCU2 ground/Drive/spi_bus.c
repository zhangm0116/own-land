#include "spi_bus.h"

/**********************************************************************
 * this function send data and then recv data.
 * param[in] device: pointer to struct rt_spi_device
 * param[in] send_buf: pointer to send buffer address
 * param[in] send_length: send buffer size
 * param[in] recv_buf: pointer to recv buffer address
 * param[in] recv_length: recv buffer size
 * return status of the operation 0:success others:failed
 **********************************************************************/
int32_t rt_spi_send_then_recv(struct rt_spi_device *device, const void *send_buf, uint32_t send_length, void *recv_buf, uint32_t recv_length)
{
    int32_t result;
    struct rt_spi_message message;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);

    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        if (device->bus->owner != device)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == RT_EOK)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                result = -RT_EIO;
                goto __exit;
            }
        }

        /* send data */
        message.send_buf   = send_buf;
        message.recv_buf   = RT_NULL;
        message.length     = send_length;
        message.cs_take    = 1;
        message.cs_release = 0;
        message.next       = RT_NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result < 0)
        {
            result = -RT_EIO;
            goto __exit;
        }
        

        /* recv data */
        message.send_buf   = RT_NULL;
        message.recv_buf   = recv_buf;
        message.length     = recv_length;
        message.cs_take    = 0;
        message.cs_release = 1;
        message.next       = RT_NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result < 0)
        {
            result = -RT_EIO;
            goto __exit;
        }

        result = RT_EOK;
    }
    else
    {
        return -RT_EIO;
    }

__exit:
    rt_mutex_release(&(device->bus->lock));

    return result;
}

/**********************************************************************
 * this function send data and then send data.
 * param[in] device: pointer to struct rt_spi_device
 * param[in] send_buf1: pointer to send buffer address
 * param[in] send_length1: send buffer size
 * param[in] send_buf2: pointer to send buffer address
 * param[in] send_length2: send buffer size
 * return status of the operation 0:success others:failed
 **********************************************************************/
int32_t rt_spi_send_then_send(struct rt_spi_device *device, const void *send_buf1, uint32_t send_length1, const void *send_buf2, uint32_t send_length2)
{
    int32_t result;
    struct rt_spi_message message;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);

    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        if (device->bus->owner != device)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == RT_EOK)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                result = -RT_EIO;
                goto __exit;
            }
        }

        /* send data1 */
        message.send_buf   = send_buf1;
        message.recv_buf   = RT_NULL;
        message.length     = send_length1;
        message.cs_take    = 1;
        message.cs_release = 0;
        message.next       = RT_NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result < 0)
        {
            result = -RT_EIO;
            goto __exit;
        }

        /* send data2 */
        message.send_buf   = send_buf2;
        message.recv_buf   = RT_NULL;
        message.length     = send_length2;
        message.cs_take    = 0;
        message.cs_release = 1;
        message.next       = RT_NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result < 0)
        {
            result = -RT_EIO;
            goto __exit;
        }

        result = RT_EOK;
    }
    else
    {
        return -RT_EIO;
    }

__exit:
    rt_mutex_release(&(device->bus->lock));

    return result;
}
/**********************************************************************
 * this function send data and simultaneously recv data.
 * param[in] device: pointer to struct rt_spi_device
 * param[in] send_buf: pointer to send buffer address
 * param[in] recv_buf: pointer to send buffer address
 * param[in] length: buffer size
 * return the number of transfer data
 **********************************************************************/
int32_t rt_spi_transfer(struct rt_spi_device *device, const void *send_buf, void *recv_buf, uint32_t length)
{
    int32_t result;
    struct rt_spi_message message;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);

    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        if (device->bus->owner != device)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == RT_EOK)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                rt_set_errno(-RT_EIO);
                result = 0;
                goto __exit;
            }
        }

        /* initial message */
        message.send_buf   = send_buf;
        message.recv_buf   = recv_buf;
        message.length     = length;
        message.cs_take    = 1;
        message.cs_release = 1;
        message.next       = RT_NULL;

        /* transfer message */
        result = device->bus->ops->xfer(device, &message);
        if (result < 0)
        {
            rt_set_errno(-RT_EIO);
            goto __exit;
        }
    }
    else
    {
        rt_set_errno(-RT_EIO);

        return 0;
    }

__exit:
    rt_mutex_release(&(device->bus->lock));

    return result;
}

int32_t rt_spi_configure(struct rt_spi_device *device, struct rt_spi_configuration *cfg)
{
    int32_t result;

    RT_ASSERT(device != RT_NULL);

    /* set configuration */
    device->config.data_width = cfg->data_width;
    device->config.mode       = cfg->mode & RT_SPI_MODE_MASK ;
    device->config.max_hz     = cfg->max_hz ;

    if (device->bus != RT_NULL)
    {
        result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            if (device->bus->owner == device)
            {
                result = device->bus->ops->configure(device, &device->config);
            }

            /* release lock */
            rt_mutex_release(&(device->bus->lock));
        }
    }

    return result;
}

static rt_err_t spi_device_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t spi_device_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    struct rt_spi_device *device;
	int32_t result;

    device = (struct rt_spi_device *)dev;
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
	
	result = rt_spi_transfer(device, RT_NULL, buffer, size);

    return result >= 0 ? result : 0;
}

static rt_size_t spi_device_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct rt_spi_device *device;
	int32_t result;

    device = (struct rt_spi_device *)dev;
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
	
	result = rt_spi_transfer(device, buffer, RT_NULL, size);

    return result >= 0 ? result : 0;
}

static rt_err_t spi_device_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    struct rt_spi_device *device;

    device = (struct rt_spi_device *)dev;
    RT_ASSERT(device != RT_NULL);
	int32_t result = 0;
	
    switch (cmd)
    {
    case 0: /* set device */
        result =  rt_spi_configure(device, (struct rt_spi_configuration *)args);
        break;
    case 1: 
        break;
    }

    return result;
}

int32_t rt_spi_device_register(struct rt_spi_device *dev, const char *name)
{
    struct rt_device *device;
    RT_ASSERT(dev != RT_NULL);

    device = &(dev->parent);

    /* set device type */
    device->type    = RT_Device_Class_SPIDevice;
    device->init    = spi_device_init;
    device->open    = RT_NULL;
    device->close   = RT_NULL;
    device->read    = spi_device_read;
    device->write   = spi_device_write;
    device->control = spi_device_control;
    
    /* register to device manager */
    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR);
}

int32_t rt_spi_bus_attach_device(struct rt_spi_device *device, const char *name, const char *bus_name, void *user_data)
{
    int32_t result;
    rt_device_t bus;

    /* get physical spi bus */
    bus = rt_device_find(bus_name);
    if (bus != RT_NULL && bus->type == RT_Device_Class_SPIBUS)
    {
        device->bus = (struct rt_spi_bus *)bus;

        /* initialize spidev device */
        result = rt_spi_device_register(device, name);
        if (result != RT_EOK)
            return result;

        rt_memset(&device->config, 0, sizeof(device->config));
        device->parent.user_data = user_data;

        return RT_EOK;
    }

    /* not found the host bus */
    return -RT_ERROR;
}

int32_t rt_spi_bus_register(struct rt_spi_bus *bus, const char *name, const struct rt_spi_ops *ops)
{
    struct rt_device *device;
    RT_ASSERT(bus != RT_NULL);

    device = &bus->parent;

    /* set device type */
    device->type    = RT_Device_Class_SPIBUS;
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
    /* set ops */
    bus->ops = ops;
    /* initialize owner */
    bus->owner = RT_NULL;

    return RT_EOK;
}
