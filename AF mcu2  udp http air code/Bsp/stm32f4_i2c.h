#ifndef _STM32F4_I2C_H_
#define _STM32F4_I2C_H_

#include "stm32f4xx.h"
#include "i2c_bit_ops.h"
#include "i2c_bus.h"

struct i2c_gpio_platform_data
{
	GPIO_TypeDef	*GPIO;
	uint16_t		scl_pin;
	uint16_t		sda_pin;
};

void rt_hw_i2c_init(void);
#endif
