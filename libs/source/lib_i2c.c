#include "lib_i2c.h"

void Lib_I2C_Init(void)
{
    LL_GPIO_InitTypeDef gpio_config = {0};
    LL_I2C_InitTypeDef i2c_config = {0};

    LIB_I2C_PORT_ENCLK();
    LIB_I2C_ENCLK();

    // 配置GPIO
    // I2C_SCL和I2C_SDA都是复用开漏输出
    gpio_config.Pin = LIB_I2C_SCL_PIN;
    gpio_config.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_config.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio_config.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(LIB_I2C_SCL_PORT, &gpio_config);
    gpio_config.Pin = LIB_I2C_SDA_PIN;
    LL_GPIO_Init(LIB_I2C_SDA_PORT, &gpio_config);

    // 配置I2C
    i2c_config.PeripheralMode = LL_I2C_MODE_I2C;
    i2c_config.DutyCycle = LL_I2C_DUTYCYCLE_2;
    i2c_config.ClockSpeed = LIB_I2C_SPEED;
    i2c_config.OwnAddress1 = LIB_I2C_ADDR;
    i2c_config.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    i2c_config.TypeAcknowledge = LL_I2C_ACK;
    LL_I2C_Init(LIB_I2C, &i2c_config);
    LL_I2C_Enable(LIB_I2C);
}