#ifndef _GPIO_I2C_H
#define _GPIO_I2C_H

extern unsigned char gpio_i2c_read(unsigned char, unsigned char);
extern void gpio_i2c_write(unsigned char, unsigned char, unsigned char);
extern unsigned char gpio_sccb_read(unsigned char, unsigned char);
#endif
