/******************************************************************************
 * mt_gpio_base.c - MTKLinux GPIO Device Driver
 * 
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *     This file provid the other drivers GPIO relative functions
 *
 ******************************************************************************/

#include <mach/sync_write.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_core.h>
#include <mach/mt_gpio_base.h>
#include <cust_gpio_usage.h>

/******************************************************************************/
static GPIO_REGS *gpio_reg = (GPIO_REGS*)(GPIO_BASE);
static GPIO_REGS *gpio1_reg = (GPIO_REGS*)(GPIO1_BASE);
/*---------------------------------------------------------------------------*/
int mt_set_gpio_dir_base(unsigned long pin, unsigned long dir)
{
    unsigned long pos;
    unsigned long bit;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;
    
    if (dir == GPIO_DIR_IN)
        GPIO_SET_BITS((1L << bit), &reg->dir[pos].rst);
    else
        GPIO_SET_BITS((1L << bit), &reg->dir[pos].set);
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_dir_base(unsigned long pin)
{    
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;
    
    data = GPIO_RD32(&reg->dir[pos].val);
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_pull_enable_base(unsigned long pin, unsigned long enable)
{
	u32 pos;
    u32 bit;
    u32 val;
    u32 mask;
    GPIO_REGS *reg = gpio_reg;

	unsigned long flags = 0;
	if((pin >= 114) && (pin <= 169)){
		reg = gpio1_reg;
	}
	  
	//GPIO44~GPIO49 is special pin for sim
	if((pin >= 44) && (pin <= 46)){
		val = GPIO_RD32(GPIO_BASE+ 0X990);
    	mask = (1L << 1) - 1;    
		//pos = pin / 4;
		bit = (pin % 4) + 4;
		val &= ~(mask << (bit));
		val |= (enable << (bit));
		
		GPIO_WR32(GPIO_BASE+ 0X990, val);
	}else if((pin >= 47) && (pin <= 49)){
		pin -= 3;
		val = GPIO_RD32(GPIO_BASE+ 0X9B0);
    	mask = (1L << 1) - 1;    
		//pos = pin / 4;
		bit = (pin % 4) + 4;
		val &= ~(mask << (bit));
		val |= (enable << (bit));
		
		GPIO_WR32(GPIO_BASE+ 0X9B0, val);
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;

		if (enable == GPIO_PULL_DISABLE)
			GPIO_SET_BITS((1L << bit), &reg->pullen[pos].rst);
		else
			GPIO_SET_BITS((1L << bit), &reg->pullen[pos].set);
	}

    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int mt_get_gpio_pull_enable_base(unsigned long pin)
{
    u32 pos;
    u32 bit;
    u32 val;
    GPIO_REGS *reg = gpio_reg;

	//GPIO114~GPIO169 BASE_ADDRESS is different with others
	if((pin >= 114) && (pin <= 169)){
		reg = gpio1_reg;
	}

	//GPIO44~GPIO49 is special pin for sim
	if((pin >= 44) && (pin <= 46)){
		val = GPIO_RD32(GPIO_BASE+ 0X990);
		bit = (pin % 4) + 4;
	}else if((pin >= 47) && (pin <= 49)){
		pin -= 3;
		val = GPIO_RD32(GPIO_BASE+ 0X9B0);
		bit = (pin % 4) + 4;
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;

		val = GPIO_RD32(&reg->pullen[pos].val);
	}
    return (((val & (1L << bit)) != 0)? 1: 0);         
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_ies_base(unsigned long pin, unsigned long enable)
{
    u32 pos;
    u32 bit;
    GPIO_REGS *reg = gpio_reg;

	if((pin >= 114) && (pin <= 169)){
		reg = gpio1_reg;
	}
    
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (enable == GPIO_IES_DISABLE)
		GPIO_SET_BITS((1L << bit), &reg->ies[pos].rst);
	else
		GPIO_SET_BITS((1L << bit), &reg->ies[pos].set);

    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_ies_base(unsigned long pin)
{
    u32 pos;
    u32 bit;
    u32 val;
    GPIO_REGS *reg = gpio_reg;

	//GPIO114~GPIO169 BASE_ADDRESS is different with others
	if((pin >= 114) && (pin <= 169)){
		reg = gpio1_reg;
	}

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	val = GPIO_RD32(&reg->ies[pos].val);
    return (((val & (1L << bit)) != 0)? 1: 0);   
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_pull_select_base(unsigned long pin, unsigned long select)
{
    u32 pos;
    u32 bit;
    u32 val;
    u32 mask;
	GPIO_REGS *reg = gpio_reg;

	//GPIO114~GPIO169 BASE_ADDRESS is different with others
	if((pin >= 114) && (pin <= 169)){
		reg = gpio1_reg;
	}

	//GPIO44~GPIO49 is special pin for sim
	if((pin >= 44) && (pin <= 46)){
		val = GPIO_RD32(GPIO_BASE+ 0X990);
    	mask = (1L << 1) - 1;    
		//pos = pin / 4;
		bit = (pin % 4) + 8;
		val &= ~(mask << (bit));
		val |= (select << (bit));
		
		GPIO_WR32(GPIO_BASE+ 0X990, val);
	}else if((pin >= 47) && (pin <= 49)){
		pin -= 3;
		val = GPIO_RD32(GPIO_BASE+ 0X9B0);
    	mask = (1L << 1) - 1;    
		//pos = pin / 4;
		bit = (pin % 4) + 8;
		val &= ~(mask << (bit));
		val |= (select << (bit));
		
		GPIO_WR32(GPIO_BASE+ 0X9B0, val);
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;
		
		if (select == GPIO_PULL_DOWN)
			GPIO_SET_BITS((1L << bit), &reg->pullsel[pos].rst);
		else
			GPIO_SET_BITS((1L << bit), &reg->pullsel[pos].set);
	}
	
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_pull_select_base(unsigned long pin)
{
    u32 pos;
    u32 bit;
    u32 val;
	GPIO_REGS *reg = gpio_reg;

	if((pin >= 114) && (pin <= 169)){
		reg = gpio1_reg;
	}
    
	//GPIO44~GPIO49 is special pin for sim
	if((pin >= 44) && (pin <= 46)){
		val = GPIO_RD32(GPIO_BASE+ 0X990);
		bit = (pin % 4) + 8;
	}else if((pin >= 47) && (pin <= 49)){
		pin -= 3;
		val = GPIO_RD32(GPIO_BASE+ 0X9B0);
		bit = (pin % 4) + 8;
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;

		val = GPIO_RD32(&reg->pullsel[pos].val);
	}
    return (((val & (1L << bit)) != 0)? 1: 0);   
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_inversion_base(unsigned long pin, unsigned long enable)
{
    u32 pos;
    u32 bit;
    u32 val;
    u32 mask;
	GPIO_REGS *reg = gpio_reg;

	//GPIO44~GPIO49 is special pin for sim
	if((pin >= 44) && (pin <= 46)){
		val = GPIO_RD32(GPIO_BASE+ 0X990);
    	mask = (1L << 1) - 1;    
		//pos = pin / 4;
		bit = pin % 4;
		val &= ~(mask << (bit));
		val |= (enable << (bit));
		
		GPIO_WR32(GPIO_BASE+ 0X990, val);
	}else if((pin >= 47) && (pin <= 49)){
		pin -= 3;
		val = GPIO_RD32(GPIO_BASE+ 0X9B0);
    	mask = (1L << 1) - 1;    
		//pos = pin / 4;
		bit = pin % 4;
		val &= ~(mask << (bit));
		val |= (enable << (bit));
		
		GPIO_WR32(GPIO_BASE+ 0X9B0, val);
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;
		
		if (enable == GPIO_DATA_UNINV)
			GPIO_SET_BITS((1L << bit), &reg->dinv[pos].rst);
		else
			GPIO_SET_BITS((1L << bit), &reg->dinv[pos].set);
	}
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_inversion_base(unsigned long pin)
{
	u32 pos;
    u32 bit;
    u32 val;
	GPIO_REGS *reg = gpio_reg;

	//GPIO44~GPIO49 is special pin for sim
	if((pin >= 44) && (pin <= 46)){
		val = GPIO_RD32(GPIO_BASE+ 0X990);
		bit = (pin % 4);
	}else if((pin >= 47) && (pin <= 49)){
		pin -= 3;
		val = GPIO_RD32(GPIO_BASE+ 0X9B0);
		bit = (pin % 4);
	}else{
		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;

		val = GPIO_RD32(&reg->dinv[pos].val);
	}
    return (((val & (1L << bit)) != 0)? 1: 0);  
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_out_base(unsigned long pin, unsigned long output)
{
    unsigned long pos;
    unsigned long bit;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;
    
    if (output == GPIO_OUT_ZERO)
        GPIO_SET_BITS((1L << bit), &reg->dout[pos].rst);
    else
        GPIO_SET_BITS((1L << bit), &reg->dout[pos].set);
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_out_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    data = GPIO_RD32(&reg->dout[pos].val);
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_in_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    GPIO_REGS *reg = gpio_reg;

    pos = pin / MAX_GPIO_REG_BITS;
    bit = pin % MAX_GPIO_REG_BITS;

    data = GPIO_RD32(&reg->din[pos].val);
    return (((data & (1L << bit)) != 0)? 1: 0);        
}
/*---------------------------------------------------------------------------*/
int mt_set_gpio_mode_base(unsigned long pin, unsigned long mode)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    unsigned long mask = (1L << GPIO_MODE_BITS) - 1;    
    GPIO_REGS *reg = gpio_reg;
	
	//GPIO44~GPIO49 is special pin for sim
	if((pin >= 44) && (pin <= 46)){
		data = GPIO_RD32(GPIO_BASE+ 0X980);
    	mask = (1L << 4) - 1;    
		//pos = pin / 4;
		bit = pin % 4;
		data &= ~(mask << (4*bit));
		data |= (mode << (4*bit));
		
		GPIO_WR32(GPIO_BASE+ 0X980, data);
	}else if((pin >= 47) && (pin <= 49)){
		pin -= 3;
		data = GPIO_RD32(GPIO_BASE+ 0X9A0);
    	mask = (1L << 4) - 1;    
		//pos = pin / 4;
		bit = pin % 4;
		data &= ~(mask << (4*bit));
		data |= (mode << (4*bit));
		
		GPIO_WR32(GPIO_BASE+ 0X9A0, data);
	}else{
		pos = pin / MAX_GPIO_MODE_PER_REG;
		bit = pin % MAX_GPIO_MODE_PER_REG;
	   
		data = GPIO_RD32(&reg->mode[pos].val);

		data &= ~(mask << (GPIO_MODE_BITS*bit));
		data |= (mode << (GPIO_MODE_BITS*bit));
		
		GPIO_WR32(&reg->mode[pos].val, data);
	}
    return RSUCCESS;
}
/*---------------------------------------------------------------------------*/
int mt_get_gpio_mode_base(unsigned long pin)
{
    unsigned long pos;
    unsigned long bit;
    unsigned long data;
    unsigned long mask = (1L << GPIO_MODE_BITS) - 1;    
    GPIO_REGS *reg = gpio_reg;
	
	//GPIO44~GPIO49 is special pin for sim
	if((pin >= 44) && (pin <= 46)){
		data = GPIO_RD32(GPIO_BASE+ 0X980);
    	mask = (1L << 4) - 1;    
		//pos = pin / 4;
		bit = pin % 4;
		
		return ((data >> (4*bit)) & mask);
	}else if((pin >= 47) && (pin <= 49)){
		pin -= 3;
		data = GPIO_RD32(GPIO_BASE+ 0X9A0);
    	mask = (1L << 4) - 1;    
		//pos = pin / 4;
		bit = pin % 4;
		
		return ((data >> (4*bit)) & mask);
	}else{
		pos = pin / MAX_GPIO_MODE_PER_REG;
		bit = pin % MAX_GPIO_MODE_PER_REG;

		data = GPIO_RD32(&reg->mode[pos].val);
		
		return ((data >> (GPIO_MODE_BITS*bit)) & mask);
	}
}
/*---------------------------------------------------------------------------*/
#ifdef CONFIG_PM 
/*---------------------------------------------------------------------------*/
void mt_gpio_suspend(void)
{
    return;
}
/*---------------------------------------------------------------------------*/
void mt_gpio_resume(void)
{
    return;
}
/*---------------------------------------------------------------------------*/
#endif /*CONFIG_PM*/
/*---------------------------------------------------------------------------*/
