#ifndef __I2C_BIT_OPS_H__
#define __I2C_BIT_OPS_H__

#include "stdint.h"
#include "rtthread.h"
#include "i2c_bus.h"

struct rt_i2c_bit_ops
{
    void		*data;            /* private data for lowlevel routines */
    void		(*set_sda)(void *data, int32_t state);
    void		(*set_scl)(void *data, int32_t state);
    int32_t		(*get_sda)(void *data);
    int32_t		(*get_scl)(void *data);

    void		(*udelay)(uint32_t us);

    uint32_t	delay_us;  /* scl and sda line delay */
    uint32_t	timeout;   /* in tick */
};

#endif
