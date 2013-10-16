/*  
 * Copyright (c) 2006 XXX  Co., Ltd.  
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program. 
 *  
 * 
 * History: 
 *     10-Feb-2009 create this file 
 *       
 */  
  
#include <linux/module.h>  
#include <linux/config.h>  
#include <linux/errno.h>  
#include <linux/miscdevice.h>  
#include <linux/fcntl.h>  
  
#include <linux/init.h>  
#include <linux/delay.h>  
#include <linux/proc_fs.h>  
#include <linux/workqueue.h>  
  
#include <asm/uaccess.h>  
#include <asm/system.h>  
#include <asm/io.h>  
  
#include "gpio_i2c.h"   
  
  
/* GPIO0_0 */  
#define SCL             (1 << 6)      /* 6 indicated 6th bit*/  
  
/* GPIO0_1 */  
#define SDA             (1 << 7)  
  
  
#define GPIO_I2C_BASE 0x101e6000  
  
#define GPIO_I2C_DIR IO_ADDRESS(GPIO_I2C_BASE + 0x400)  
  
#define GPIO_I2C_SDA_REG IO_ADDRESS(GPIO_I2C_BASE + (SDA<<2))    /* data reg  */  
#define GPIO_I2C_SCL_REG IO_ADDRESS(GPIO_I2C_BASE + (SCL<<2))  
  
#define GPIO_I2C_SCLSDA_REG IO_ADDRESS(GPIO_I2C_BASE + 0x300)    /* simultaneously change SDA&SCL data reg */  
  
#define HW_REG(reg) *((volatile unsigned int *)(reg))  
#define DELAY(us)       time_delay_us(us)  
  
  
/*  
 * I2C by GPIO simulated  clear 0 routine. 
 * 
 * @param whichline: GPIO control line 
 * 
 */  
static void i2c_clr(unsigned char whichline)  
{  
    unsigned char regvalue;  
      
    if(whichline == SCL)  
    {  
        regvalue = HW_REG(GPIO_I2C_DIR);  
        regvalue |= SCL;  
        HW_REG(GPIO_I2C_DIR) = regvalue;           /*   set scl dir output    */  
          
        HW_REG(GPIO_I2C_SCL_REG) = 0;              /*   set scl data 0        */  
        return;  
    }  
    else if(whichline == SDA)  
    {  
        regvalue = HW_REG(GPIO_I2C_DIR);  
        regvalue |= SDA;  
        HW_REG(GPIO_I2C_DIR) = regvalue;  
          
        HW_REG(GPIO_I2C_SDA_REG) = 0;  
        return;  
    }  
    else if(whichline == (SDA|SCL))  
    {  
        regvalue = HW_REG(GPIO_I2C_DIR);  
        regvalue |= (SDA|SCL);  
        HW_REG(GPIO_I2C_DIR) = regvalue;  
          
        HW_REG(GPIO_I2C_SCLSDA_REG) = 0;  
        return;  
    }  
    else  
    {  
        printk("Error input.\n");  
        return;  
    }  
      
}  
  
/*  
 * I2C by GPIO simulated  set 1 routine. 
 * 
 * @param whichline: GPIO control line 
 * 
 */  
static void  i2c_set(unsigned char whichline)  
{  
    unsigned char regvalue;  
      
    if(whichline == SCL)  
    {  
        regvalue = HW_REG(GPIO_I2C_DIR);  
        regvalue |= SCL;  
        HW_REG(GPIO_I2C_DIR) = regvalue;  
          
        HW_REG(GPIO_I2C_SCL_REG) = SCL;  
        return;  
    }  
    else if(whichline == SDA)  
    {  
        regvalue = HW_REG(GPIO_I2C_DIR);  
        regvalue |= SDA;  
        HW_REG(GPIO_I2C_DIR) = regvalue;  
          
        HW_REG(GPIO_I2C_SDA_REG) = SDA;  
        return;  
    }  
    else if(whichline == (SDA|SCL))  
    {  
        regvalue = HW_REG(GPIO_I2C_DIR);  
        regvalue |= (SDA|SCL);  
        HW_REG(GPIO_I2C_DIR) = regvalue;  
          
        HW_REG(GPIO_I2C_SCLSDA_REG) = (SDA|SCL);  
        return;  
    }  
    else  
    {  
        printk("Error input.\n");  
        return;  
    }  
}  
  
/* 
 *  delays for a specified number of micro seconds rountine. 
 * 
 *  @param usec: number of micro seconds to pause for 
 * 
 */  
void time_delay_us(unsigned int usec)  
{  
    int i,j;  
      
    for(i=0;i<usec * 5;i++)  
    {  
        for(j=0;j<47;j++)  
        {;}  
    }  
}  
  
/*  
 * I2C by GPIO simulated  read data routine. 
 * 
 * @return value: a bit for read  
 * 
 */  
   
static unsigned char i2c_data_read(void)  
{  
    unsigned char regvalue;  
      
    regvalue = HW_REG(GPIO_I2C_DIR);  
    regvalue &= (~SDA);  
    HW_REG(GPIO_I2C_DIR) = regvalue;  
    DELAY(1);  
          
    regvalue = HW_REG(GPIO_I2C_SDA_REG);  
    if((regvalue & SDA) != 0)  
        return 1;  
    else  
        return 0;  
}  
  
  
  
/* 
 * sends a start bit via I2C rountine. 
 * 
 */  
static void i2c_start_bit(void)  
{  
        DELAY(1);  
        i2c_set(SDA | SCL);  
        DELAY(1);  
        i2c_clr(SDA);  
        DELAY(2);  
}  
  
/* 
 * sends a stop bit via I2C rountine. 
 * 
 */  
static void i2c_stop_bit(void)  
{  
        /* clock the ack */  
        DELAY(1);  
        i2c_set(SCL);  
        DELAY(1);   
        i2c_clr(SCL);    
  
        /* actual stop bit */  
        DELAY(1);  
        i2c_clr(SDA);  
        DELAY(1);  
        i2c_set(SCL);  
        DELAY(1);  
        i2c_set(SDA);  
        DELAY(1);  
}  
  
/* 
 * sends a character over I2C rountine. 
 * 
 * @param  c: character to send 
 * 
 */  
static void i2c_send_byte(unsigned char c)  
{  
    int i;  
    local_irq_disable();  
    for (i=0; i<8; i++)  
    {  
        DELAY(1);  
        i2c_clr(SCL);  
        DELAY(1);  
  
        if (c & (1<<(7-i)))  
            i2c_set(SDA);  
        else  
            i2c_clr(SDA);  
  
        DELAY(1);  
        i2c_set(SCL);  
        DELAY(1);  
        i2c_clr(SCL);  
    }  
    DELAY(1);  
    local_irq_enable();  
}  
  
/*  receives a character from I2C rountine. 
 * 
 *  @return value: character received 
 * 
 */  
static unsigned char i2c_receive_byte(void)  
{  
    int j=0;  
    int i;  
    unsigned char regvalue;  
  
    local_irq_disable();  
    for (i=0; i<8; i++)  
    {  
        DELAY(1);  
        i2c_clr(SCL);  
        DELAY(2);  
        i2c_set(SCL);  
          
        regvalue = HW_REG(GPIO_I2C_DIR);  
        regvalue &= (~SDA);  
        HW_REG(GPIO_I2C_DIR) = regvalue;  
        DELAY(1);  
          
        if (i2c_data_read())  
            j+=(1<<(7-i));  
  
        DELAY(1);  
        i2c_clr(SCL);  
    }  
    local_irq_enable();  
    DELAY(1);  
  
    return j;  
}  
  
/*  receives an acknowledge from I2C rountine. 
 * 
 *  @return value: 0--Ack received; 1--Nack received 
 *           
 */  
static int i2c_receive_ack(void)  
{  
    int nack;  
    unsigned char regvalue;  
      
    DELAY(1);  
      
    regvalue = HW_REG(GPIO_I2C_DIR);  
    regvalue &= (~SDA);  
    HW_REG(GPIO_I2C_DIR) = regvalue;  
      
    DELAY(1);  
    i2c_clr(SCL);  
    DELAY(1);  
    i2c_set(SCL);  
    DELAY(1);  
      
  
    nack = i2c_data_read();  
  
    DELAY(1);  
    i2c_clr(SCL);  
    DELAY(1);  
  
    if (nack == 0)  
        return 1;   
  
    return 0;  
}  
  
  
  
/*  
 * sends an acknowledge over I2C rountine. 
 * 
 */  
static void i2c_send_ack(void)  
{  
    DELAY(1);  
    i2c_clr(SCL);  
    DELAY(1);  
    i2c_set(SDA);  
    DELAY(1);  
    i2c_set(SCL);  
    DELAY(1);  
    i2c_clr(SCL);  
    DELAY(1);  
    i2c_clr(SDA);  
    DELAY(1);  
}  
  
/*   
 *  read data from the I2C bus by GPIO simulated of a device rountine. 
 * 
 *  @param  devaddress:  address of the device 
 *  @param  address: address of register within device 
 *    
 *  @return value: data from the device readed 
 *  
 */  
  
unsigned char gpio_i2c_read(unsigned char devaddress, unsigned char address)  
{  
    int rxdata;  
      
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(devaddress));  
    i2c_receive_ack();  
    i2c_send_byte(address);  
    i2c_receive_ack();     
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(devaddress) | 1);  
    i2c_receive_ack();  
    rxdata = i2c_receive_byte();  
    i2c_send_ack();  
    i2c_stop_bit();  
  
    return rxdata;  
}  
  
/* 
 *  writes data to a device on the I2C bus rountine.  
 * 
 *  @param  devaddress:  address of the device 
 *  @param  address: address of register within device 
 *  @param  data:   data for write to device 
 * 
 */  
  
void gpio_i2c_write(unsigned char devaddress, unsigned char address, unsigned char data)  
{  
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(devaddress));  
    i2c_receive_ack();  
    i2c_send_byte(address);  
    i2c_receive_ack();  
    i2c_send_byte(data);   
    i2c_stop_bit();  
}  
  
  
/*   
 *  read data from the I2C bus by GPIO simulated of a digital camera device rountine. 
 * 
 *  @param  devaddress:  address of the device 
 *  @param  address: address of register within device 
 *   
 */  
   
unsigned char gpio_sccb_read(unsigned char devaddress, unsigned char address)  
{  
    int rxdata;  
  
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(devaddress));  
    i2c_receive_ack();  
    i2c_send_byte(address);  
    i2c_receive_ack();  
    i2c_stop_bit();  
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(devaddress) | 1);  
    i2c_receive_ack();  
    rxdata = i2c_receive_byte();  
    i2c_send_ack();  
    i2c_stop_bit();  
  
    return rxdata;  
}  
  
static unsigned int  gpioinitialized =0;  
/* 
 * initializes I2C interface routine. 
 * 
 * @return value:0--success; 1--error. 
 * 
 */  
static int __init gpio_i2c_init(void)  
{  
   if(gpioinitialized == 0)  
    {  
        printk(KERN_INFO "Hisilicon GPIO control for I2C Driver \n");                
        i2c_clr(SCL | SDA);  
        gpioinitialized =1;  
          
        return 0;  
    }  
    else  
    {  
        printk("GPIO control for I2C has been initialized.\n");  
        return 0;  
    }  
}  
  
static void __exit gpio_i2c_exit(void)  
{  
    gpioinitialized =0;  
}  
  
module_init(gpio_i2c_init);  
module_exit(gpio_i2c_exit);  
  
#ifdef MODULE  
#include <linux/compile.h>  
#endif  
MODULE_INFO(build, UTS_VERSION);  
MODULE_LICENSE("GPL");  
  
EXPORT_SYMBOL(gpio_i2c_read);  
EXPORT_SYMBOL(gpio_i2c_write); 
