#include "stm32f4_i2c.h"

#ifdef RT_USING_I2C
static struct i2c_gpio_platform_data	p_data;
static struct rt_i2c_bus_device			i2c_bus;
#endif

/* I2C1 */
#define I2C_GPIO_SCL		    GPIO_Pin_8
#define I2C_GPIO_SDA		    GPIO_Pin_9
#define I2C_GPIO			    GPIOB

#ifdef RT_USING_I2C
static void udelay(uint32_t us)
{
	uint32_t count = us;
	
	TIM_Cmd(TIM2, ENABLE);
	
	TIM_SetCounter(TIM2, count);
	while(count >= 1)
	{
		count = TIM_GetCounter(TIM2);
	}
	TIM_Cmd(TIM2, DISABLE);
}

static void i2c_gpio_set_scl(void *data, int32_t value)
{
	struct i2c_gpio_platform_data	*p_data = data;
	
	if(value == 0)
	{
		GPIO_ResetBits(p_data->GPIO, p_data->scl_pin);
	}
	else if(value == 1)
	{
		GPIO_SetBits(p_data->GPIO, p_data->scl_pin);
	}
}

static void i2c_gpio_set_sda(void *data, int32_t value)
{
	struct i2c_gpio_platform_data	*p_data = data;
	
	if(value == 0)
	{
		GPIO_ResetBits(p_data->GPIO, p_data->sda_pin);
	}
	else if(value == 1)
	{
		GPIO_SetBits(p_data->GPIO, p_data->sda_pin);
	}
}

static int32_t i2c_gpio_get_scl(void *data)
{
	struct i2c_gpio_platform_data	*p_data = data;
	
	return GPIO_ReadInputDataBit(p_data->GPIO, p_data->scl_pin);
}

static int32_t i2c_gpio_get_sda(void *data)
{
	struct i2c_gpio_platform_data	*p_data = data;

	return GPIO_ReadInputDataBit(p_data->GPIO, p_data->sda_pin);
}

/* i2c bit operation instance */
static const struct rt_i2c_bit_ops bit_ops = {
	&p_data,
	i2c_gpio_set_sda,
	i2c_gpio_set_scl,
	i2c_gpio_get_sda,
	i2c_gpio_get_scl,
	
	udelay,

	5,
	100
};
#endif
/*******************************************i2c bus init/register************************/
static void RCC_Configuration(void)
{
#ifdef RT_USING_I2C
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
#endif
}

static void GPIO_Configuration(void)
{
#ifdef RT_USING_I2C
    GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	/* Configure I2C1 SCL/SDA PIN */
	GPIO_InitStructure.GPIO_Pin = I2C_GPIO_SCL | I2C_GPIO_SDA;
	GPIO_SetBits(I2C_GPIO, I2C_GPIO_SCL);
	GPIO_SetBits(I2C_GPIO, I2C_GPIO_SDA);
	GPIO_Init(I2C_GPIO, &GPIO_InitStructure);
#endif
}

static void TIM_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseStructure.TIM_Period = 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 72;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	TIM_Cmd(TIM2, DISABLE);
}

/* this function init struct rt_i2c_bus and register i2c bus */
void rt_hw_i2c_init()
{
	RCC_Configuration();
	GPIO_Configuration();
	TIM_Configuration();
	
#ifdef RT_USING_I2C
	p_data.GPIO = I2C_GPIO;
	p_data.scl_pin = I2C_GPIO_SCL;
	p_data.sda_pin = I2C_GPIO_SDA;
	
	i2c_bus.priv = (void *)&bit_ops;
	i2c_bus.retries = 5;
	i2c_bus.timeout = 5;
	
	rt_i2c_bit_add_bus(&i2c_bus, "i2c");
#endif	
}
