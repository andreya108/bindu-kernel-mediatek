/* drivers/hwmon/mt6516/amit/APDS9930.c - APDS9930 ALS/PS driver
 * 
 * Author: MingHsien Hsieh <minghsien.hsieh@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>


#if 1
//#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#endif

#define POWER_NONE_MACRO MT65XX_POWER_NONE

#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
#include "APDS9930.h"
#include <linux/math64.h>
/******************************************************************************
 * configuration
*******************************************************************************/
/*----------------------------------------------------------------------------*/

#define APDS9930_DEV_NAME     "APDS9930"
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[ALS/PS] "
#define APS_FUN(f)               printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(KERN_ERR APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_INFO APS_TAG fmt, ##args) 

#define I2C_FLAG_WRITE	0
#define I2C_FLAG_READ	1

/*  update lux convert algo from avago FAE --liaoxl.lenovo start */
#define APDS9930_COE_B1		223	/* 2.23 without glass window */
#define APDS9930_COE_C1		70	/* 0.70 without glass window */
#define APDS9930_COE_D1		142	/* 1.42 without glass window */
#define APDS9930_COE_GA1		48	/* 0.48 without glass window */
#define APDS9930_DF		52
#define APDS9930_MAX_LUXVALUE (30000*100)

typedef enum 
{
  APDS9930_ALS_RES_27MS = 0,    /* 27.2ms integration time */ 
  APDS9930_ALS_RES_51MS = 1,    /* 51.68ms integration time */
  APDS9930_ALS_RES_100MS = 2     /* 100.64ms integration time */
} apds9930_als_res_e;

typedef enum 
{
  APDS9930_ALS_GAIN_1X    = 0,    /* 1x AGAIN */ 
  APDS9930_ALS_GAIN_8X    = 1,    /* 8x AGAIN */
  APDS9930_ALS_GAIN_16X   = 2,    /* 16x AGAIN */
  APDS9930_ALS_GAIN_120X  = 3     /* 120x AGAIN */
} apds9930_als_gain_e;

static unsigned char apds9930_als_atime_tb[] = { 0xF6, 0xED, 0xDB };
static unsigned short apds9930_als_integration_tb[] = {2720, 5168, 10064};
static unsigned short apds9930_als_res_tb[] = { 10240, 19456, 37888 };
static unsigned char apds9930_als_again_tb[] = { 1, 8, 16, 120 };
static unsigned char apds9930_als_again_bit_tb[] = { 0x00, 0x01, 0x02, 0x03 };

static void apds_esd_check(struct i2c_client *client);
/*  update lux convert algo from avago FAE --liaoxl.lenovo end */

/******************************************************************************
 * extern functions
*******************************************************************************/
	extern void mt_eint_unmask(unsigned int line);
	extern void mt_eint_mask(unsigned int line);
	extern void mt_eint_set_polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
	extern void mt_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
	extern kal_uint32 mt_eint_set_sens(kal_uint8 eintno, kal_bool sens);

extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
	
/*----------------------------------------------------------------------------*/
static struct i2c_client *APDS9930_i2c_client = NULL;
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id APDS9930_i2c_id[] = {{APDS9930_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_APDS9930={ I2C_BOARD_INFO("APDS9930", 0x39)};
/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int APDS9930_i2c_remove(struct i2c_client *client);
static int APDS9930_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_suspend(struct i2c_client *client, pm_message_t msg);
static int APDS9930_i2c_resume(struct i2c_client *client);

static DEFINE_MUTEX(APDS9930_mutex);


static struct APDS9930_priv *g_APDS9930_ptr = NULL;

 struct PS_CALI_DATA_STRUCT
{
    int close;
    int far_away;
    int valid;
} ;

static int intr_flag_value = 0;
/*----------------------------------------------------------------------------*/
typedef enum {
    CMC_BIT_ALS    = 1,
    CMC_BIT_PS     = 2,
} CMC_BIT;
/*----------------------------------------------------------------------------*/
struct APDS9930_i2c_addr {    /*define a series of i2c slave address*/
    u8  write_addr;  
    u8  ps_thd;     /*PS INT threshold*/
};
/*----------------------------------------------------------------------------*/
struct APDS9930_priv {
    struct alsps_hw  *hw;
    struct i2c_client *client;
    struct work_struct  eint_work;

    /*i2c address group*/
    struct APDS9930_i2c_addr  addr;
    
    /*misc*/
    u16		    als_modulus;
    atomic_t    i2c_retry;
    atomic_t    als_suspend;
    atomic_t    als_debounce;   /*debounce time after enabling als*/
    atomic_t    als_deb_on;     /*indicates if the debounce is on*/
    atomic_t    als_deb_end;    /*the jiffies representing the end of debounce*/
    atomic_t    ps_mask;        /*mask ps: always return far away*/
    atomic_t    ps_debounce;    /*debounce time after enabling ps*/
    atomic_t    ps_deb_on;      /*indicates if the debounce is on*/
    atomic_t    ps_deb_end;     /*the jiffies representing the end of debounce*/
    atomic_t    ps_suspend;

	atomic_t    wq_process;  /* add for esd check func  -- by liaoxl.lenovo 4.11.2013  */

    /*data*/
    int         als;
    u16          ps;
    u8          _align;
    u16         als_level_num;
    u16         als_value_num;
    u32         als_level[C_CUST_ALS_LEVEL];
    u32         als_value[C_CUST_ALS_LEVEL];

    atomic_t    als_cmd_val;    /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_cmd_val;     /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_thd_val_high;     /*the cmd value can't be read, stored in ram*/
	atomic_t    ps_thd_val_low;     /*the cmd value can't be read, stored in ram*/
    ulong       enable;         /*enable mask*/
    ulong       pending_intr;   /*pending interrupt*/
	atomic_t    ps_psat;		/*add for sunlight issue*/

/*  update lux convert algo from avago FAE --liaoxl.lenovo start */
	unsigned int als_atime_index;	/* storage for als integratiion time */
	unsigned int als_again_index;	/* storage for als GAIN */
	int          als_reduce;	/* flag indicate ALS 6x reduction */
	u16          als_ch0;       /* for ps sunlight issue */
	u16          als_ch1;       /* for ps sunlight issue */
/*  update lux convert algo from avago FAE --liaoxl.lenovo end */
    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver APDS9930_i2c_driver = {	
	.probe      = APDS9930_i2c_probe,
	.remove     = APDS9930_i2c_remove,
	.detect     = APDS9930_i2c_detect,
	.suspend    = APDS9930_i2c_suspend,
	.resume     = APDS9930_i2c_resume,
	.id_table   = APDS9930_i2c_id,
	.driver = {
		.name           = APDS9930_DEV_NAME,
	},
};


static struct APDS9930_priv *APDS9930_obj = NULL;
static struct platform_driver APDS9930_alsps_driver;
/*------------------------i2c function for 89-------------------------------------*/
int APDS9930_i2c_master_operate(struct i2c_client *client, const char *buf, int count, int i2c_flag)
{
	//begin, lenovo-sw lumy1 add for screen on failed
	int res = 0, i = 0, retry = 3;
	mutex_lock(&APDS9930_mutex);
	switch(i2c_flag){	
		case I2C_FLAG_WRITE:
		client->addr &=I2C_MASK_FLAG;
		for (i = 0; i< retry; i++) {
			res = i2c_master_send(client, buf, count);
			if (res > 0)
				break;
			APS_LOG("APDS9930_i2c_master_operate write failed, client->addr %x, retry %d ...\n", client->addr, i);
		}
		client->addr &=I2C_MASK_FLAG;
		break;
		
		case I2C_FLAG_READ:
		client->addr &=I2C_MASK_FLAG;
		client->addr |=I2C_WR_FLAG;
		client->addr |=I2C_RS_FLAG;
		for (i = 0; i< retry; i++) {
			res = i2c_master_send(client, buf, count);
			if (res > 0)
				break;
			APS_LOG("APDS9930_i2c_master_operate read failed, client->addr %x, retry %d ...\n", client->addr, i);
		}
		client->addr &=I2C_MASK_FLAG;
		break;
		default:
		APS_LOG("APDS9930_i2c_master_operate i2c_flag command not support!\n");
		break;
	}
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	mutex_unlock(&APDS9930_mutex);
	return res;
	EXIT_ERR:
	mutex_unlock(&APDS9930_mutex);
	APS_ERR("APDS9930_i2c_transfer retry fail, client->addr %x\n", client->addr);
	//end, lenovo-sw lumy1 add for screen on failed
	return res;
}

/*----------------------------------------------------------------------------*/
int APDS9930_get_addr(struct alsps_hw *hw, struct APDS9930_i2c_addr *addr)
{
	if(!hw || !addr)
	{
		return -EFAULT;
	}
	addr->write_addr= hw->i2c_addr[0];
	return 0;
}
/*----------------------------------------------------------------------------*/
static void APDS9930_power(struct alsps_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	//APS_LOG("power %s\n", on ? "on" : "off");

	if(hw->power_id != POWER_NONE_MACRO)
	{
		if(power_on == on)
		{
			APS_LOG("ignore power control: %d\n", on);
		}
		else if(on)
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "APDS9930")) 
			{
				APS_ERR("power on fails!!\n");
			}
		}
		else
		{
			if(!hwPowerDown(hw->power_id, "APDS9930")) 
			{
				APS_ERR("power off fail!!\n");   
			}
		}
	}
	power_on = on;
}
/*----------------------------------------------------------------------------*/
static long APDS9930_enable_als(struct i2c_client *client, int enable)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];	  
	long res = 0;

	if(enable)
	{
		if(test_bit(CMC_BIT_PS, &obj->enable))
			databuf[1] = 0x28|0x03|0x05;
		else
			databuf[1] = 0x08|0x03;
		databuf[0] = APDS9930_CMM_ENABLE;
		APS_LOG("APDS9930_CMM_ENABLE enable als value = %x\n",databuf[1]);
		res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		atomic_set(&obj->als_deb_on, 1);
		atomic_set(&obj->als_deb_end, jiffies+atomic_read(&obj->als_debounce)/(1000/HZ));
	}
	else
	{
		if(test_bit(CMC_BIT_PS, &obj->enable))
			databuf[1] = 0x28|0x03|0x05;
		else
			databuf[1] = 0x08;
	
		databuf[0] = APDS9930_CMM_ENABLE;
		APS_LOG("APDS9930_CMM_ENABLE disable als value = %x\n",databuf[1]);
		res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
	}

	return 0;
		
EXIT_ERR:
	APS_ERR("APDS9930_enable_als fail\n");
	return res;
}

/*----------------------------------------------------------------------------*/
static long APDS9930_enable_ps(struct i2c_client *client, int enable)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	long res = 0;

	if(enable)
	{
		if(1)
			databuf[1] = 0x28|0x03|0x05;
		else
			databuf[1] = 0x28|0x05;
		databuf[0] = APDS9930_CMM_ENABLE;
		APS_LOG("APDS9930_CMM_ENABLE enable ps value = %x ps_cali high=%d  ps_cali low\n",databuf[1],obj->ps_thd_val_high,obj->ps_thd_val_low);		
		res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		atomic_set(&obj->ps_deb_on, 1);
		atomic_set(&obj->ps_deb_end, jiffies+atomic_read(&obj->ps_debounce)/(1000/HZ));
		//mt_eint_set_polarity(CUST_EINT_ALS_NUM, 0);
		mt_eint_unmask(CUST_EINT_ALS_NUM);
	}
	else
	{
		if(test_bit(CMC_BIT_ALS, &obj->enable))
			databuf[1] = 0x08|0x03;
		else
			databuf[1] = 0x08;
	
		databuf[0] = APDS9930_CMM_ENABLE;
		APS_LOG("APDS9930_CMM_ENABLE disable ps value = %x\n",databuf[1]);	
		res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
	}

	return 0;
	
EXIT_ERR:
	APS_ERR("APDS9930_enable_ps fail\n");
	return res;
}
/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
static int APDS9930_check_and_clear_intr(struct i2c_client *client) 
{
	int res,intp,intl;
	u8 buffer[2];

	if (mt_get_gpio_in(GPIO_ALS_EINT_PIN) == 1) /*skip if no interrupt*/  
	    return 0;

	buffer[0] = APDS9930_CMM_STATUS;
	res = APDS9930_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	res = 0;
	intp = 0;
	intl = 0;
	if(0 != (buffer[0] & 0x20))
	{
		res = 1;
		intp = 1;
	}
	if(0 != (buffer[0] & 0x10))
	{
		res = 1;
		intl = 1;		
	}

	if(1 == res)
	{
		if((1 == intp) && (0 == intl))
		{
			buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x05);
		}
		else if((0 == intp) && (1 == intl))
		{
			buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x06);
		}
		else
		{
			buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x07);
		}

		res = APDS9930_i2c_master_operate(client, buffer, 0x1, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		else
		{
			res = 0;
		}
	}

	return res;

EXIT_ERR:
	APS_ERR("APDS9930_check_and_clear_intr fail\n");
	return 1;
}
/*----------------------------------------------------------------------------*/

/*yucong add for interrupt mode support MTK inc 2012.3.7*/
static int APDS9930_check_intr(struct i2c_client *client) 
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	int res,intp,intl;
	u8 buffer[2];

	if (mt_get_gpio_in(GPIO_ALS_EINT_PIN) == 1) /*skip if no interrupt*/  
	return 0;

	buffer[0] = APDS9930_CMM_STATUS;
	res = APDS9930_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = 0;
	intp = 0;
	intl = 0;
	if(0 != (buffer[0] & 0x20))
	{
		res = 0;
		intp = 1;
	}
	if(0 != (buffer[0] & 0x10))
	{
		res = 0;
		intl = 1;		
	}
	/*lenovo sw molg1 add for sun issue 20130521 begin*/
	if(0 != (buffer[0] & 0x40))
	{
		atomic_set(&obj->ps_psat, 1);		
	}
	else
	{
		atomic_set(&obj->ps_psat, 0);
	}
	/*lenovo sw molg1 add for sun issue 20130521 end*/
	APS_LOG("APDS9930_check_intr status=0x%x\n", buffer[0]);

	return res;

EXIT_ERR:
	APS_ERR("APDS9930_check_intr fail\n");
	return 1;
}

static int APDS9930_clear_intr(struct i2c_client *client) 
{
	int res;
	u8 buffer[2];
	
	buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x07);
	res = APDS9930_i2c_master_operate(client, buffer, 0x1, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	else
	{
		res = 0;
	}
	return res;

EXIT_ERR:
	APS_ERR("APDS9930_check_and_clear_intr fail\n");
	return 1;
}


/*-----------------------------------------------------------------------------*/
void APDS9930_eint_func(void)
{
	struct APDS9930_priv *obj = g_APDS9930_ptr;
	if(!obj)
	{
		return;
	}
	//APS_LOG(" debug eint function performed!\n");
	atomic_set(&obj->wq_process, 1);/* add for esd check func  -- by liaoxl.lenovo 4.11.2013  */
	schedule_work(&obj->eint_work);
}

/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
int APDS9930_setup_eint(struct i2c_client *client)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);        

	g_APDS9930_ptr = obj;
	
	mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
	mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, TRUE);
	mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);

	//mt65xx_eint_set_sens(CUST_EINT_ALS_NUM, CUST_EINT_ALS_SENSITIVE);
	//mt65xx_eint_set_polarity(CUST_EINT_ALS_NUM, CUST_EINT_ALS_POLARITY);
	mt_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_TYPE, APDS9930_eint_func, 0);

	mt_eint_mask(CUST_EINT_ALS_NUM);  
    return 0;
}

/*----------------------------------------------------------------------------*/

static int APDS9930_init_client(struct i2c_client *client)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;

	databuf[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x00);
	res = APDS9930_i2c_master_operate(client, databuf, 0x1, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	databuf[0] = APDS9930_CMM_ENABLE;
	databuf[1] = 0x08;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	databuf[0] = APDS9930_CMM_ATIME;    
	databuf[1] = apds9930_als_atime_tb[obj->als_atime_index];/*  update lux convert algo from avago FAE --liaoxl.lenovo */
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	databuf[0] = APDS9930_CMM_PTIME;    
	databuf[1] = 0xFF;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	databuf[0] = APDS9930_CMM_WTIME;    
	databuf[1] = 0xFC;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if(0 == obj->hw->polling_mode_ps)
	{
		databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;	
		databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low)) & 0x00FF);
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		databuf[0] = APDS9930_CMM_INT_LOW_THD_HIGH;	
		databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_low)) & 0xFF00) >> 8);
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;	
		databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high)) & 0x00FF);
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		
		databuf[0] = APDS9930_CMM_INT_HIGH_THD_HIGH; 
		databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_high)) & 0xFF00) >> 8);
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}

		databuf[0] = APDS9930_CMM_Persistence;
		databuf[1] = 0x20;
		res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}

	}

	databuf[0] = APDS9930_CMM_CONFIG;    
	databuf[1] = 0x00;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

       /*Lenovo-sw chenlj2 add 2011-06-03,modified pulse 2  to 4 */
	databuf[0] = APDS9930_CMM_PPCOUNT;    
	databuf[1] = APDS9930_CMM_PPCOUNT_VALUE;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

        /*Lenovo-sw chenlj2 add 2011-06-03,modified gain 16  to 1 */
	databuf[0] = APDS9930_CMM_CONTROL;    
	databuf[1] = (APDS9930_CMM_CONTROL_VALUE & 0xFC) | apds9930_als_again_bit_tb[obj->als_again_index];/* add for esd check func  -- by liaoxl.lenovo 4.11.2013  */
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if((res = APDS9930_setup_eint(client))!=0)
	{
		APS_ERR("setup eint: %d\n", res);
		return res;
	}
	if((res = APDS9930_check_and_clear_intr(client)))
	{
		APS_ERR("check/clear intr: %d\n", res);
	    return res;
	}
	
	return APDS9930_SUCCESS;

EXIT_ERR:
	APS_ERR("init dev: %d\n", res);
	return res;
}

/****************************************************************************** 
 * Function Configuration
******************************************************************************/
/*  update lux convert algo from avago FAE --liaoxl.lenovo start */
static int apds9930_set_als_poll_delay(struct i2c_client *client, int val)
{
	struct APDS9930_priv *data = i2c_get_clientdata(client);
	int ret;
	int atime_index=0;
	unsigned long flags;
	u8 databuf[2];

	//printk("%s : %d\n", __func__, val);
	if (val <= 100) {
		atime_index = APDS9930_ALS_RES_27MS;
	}
	else if (val <= 1000) {
		atime_index = APDS9930_ALS_RES_51MS;
	}
	else {	// APDS_ALS_POLL_SLOW
		atime_index = APDS9930_ALS_RES_100MS;
	}

	databuf[0] = APDS9930_CMM_ATIME;    
	databuf[1] = apds9930_als_atime_tb[atime_index];
	ret = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(ret <= 0)
	{
		return -1;
	}
	else
	{
		data->als_atime_index = atime_index;
		APS_LOG("poll delay %d, atime_index %d\n", val, data->als_atime_index);
	}

	return 0;
}


static int LuxCalculationOrg(struct i2c_client *client, int ch0data, int ch1data)
{
/*  update lux convert algo from avago FAE --20130817 molg1.lenovo begin*/
	struct APDS9930_priv *data = i2c_get_clientdata(client);
	int luxValue=0;
	int IAC1=0;
	int IAC2=0;
	int IAC=0;
	int ch1ch0_ratio=0;
	int APDS9930_COE_B, APDS9930_COE_C, APDS9930_COE_D, APDS9930_GA;
	int APDS9930_OUTDOOR_GA=250;//125;

	if (ch0data >= apds9930_als_res_tb[data->als_atime_index]) {
		luxValue = 30000;
		return (luxValue);
	}
	else if (ch1data >= apds9930_als_res_tb[data->als_atime_index]) {
		luxValue = 30000;
		return (luxValue);
	}

	if (ch0data == 0) {
		luxValue = 0;
		return (luxValue);
	}

	ch1ch0_ratio = (ch1data*100)/ch0data;	// scale up by 100

	if ( (ch1ch0_ratio >= 30) && (ch1ch0_ratio <= 45) ) {
		// sunglight
		luxValue = (ch0data*100)/((apds9930_als_integration_tb[data->als_atime_index]/100)*apds9930_als_again_tb[data->als_again_index]);
		luxValue = (luxValue * APDS9930_OUTDOOR_GA)/100;

		// DO NOT USE ALS_REDUCE 

		return luxValue;
	}
	else {
		// white light/Incand
		APDS9930_COE_B = APDS9930_COE_B1;
		APDS9930_COE_C = APDS9930_COE_C1;
		APDS9930_COE_D = APDS9930_COE_D1;
		APDS9930_GA = APDS9930_COE_GA1;		
	}

	IAC1 = (ch0data - (APDS9930_COE_B*ch1data)/100);			// re-adjust COE_B to avoid 2 decimal point
	IAC2 = ((APDS9930_COE_C*ch0data)/100 - (APDS9930_COE_D*ch1data)/100); 	// re-adjust COE_C and COE_D to void 2 decimal point

	if (IAC1 > IAC2)
		IAC = IAC1;
	else if (IAC1 <= IAC2)
		IAC = IAC2;
	else
		IAC = 0;

	if ((IAC1<0) && (IAC2<0)) {
		if (ch0data < (apds9930_als_res_tb[data->als_atime_index]/2)) {
			IAC = 0;	// cdata and irdata saturated
		}
		else {
			luxValue = 30000;
			return (luxValue);
		}
		
		//APS_ERR("LuxCalculation error with iac1=%d iac2=%d atime=%d again=%d\n", IAC1, IAC2, data->als_atime_index, data->als_again_index);
		//return -1; 	// don't report first, change gain may help
	}
/*  update lux convert algo from avago FAE --20130817 molg1.lenovo end*/
	if (data->als_reduce) {
		luxValue = 10 * ((IAC*APDS9930_GA*APDS9930_DF)/100)*4/((apds9930_als_integration_tb[data->als_atime_index]/100)*apds9930_als_again_tb[data->als_again_index]);
	}
	else {
		luxValue = 10 * ((IAC*APDS9930_GA*APDS9930_DF)/100)/((apds9930_als_integration_tb[data->als_atime_index]/100)*apds9930_als_again_tb[data->als_again_index]);
	}
	
	return luxValue;
}


static int LuxCalculation(struct i2c_client *client, int ch0data, int ch1data)
{
/*  update lux convert algo from avago FAE --20130817 molg1.lenovo begin*/
	struct APDS9930_priv *data = i2c_get_clientdata(client);
	int luxValue=0;
	int IAC1=0;
	int IAC2=0;
	int IAC=0;
	u64 data1;
	u32 data2;
	int ch1ch0_ratio=0;
	int APDS9930_COE_B, APDS9930_COE_C, APDS9930_COE_D, APDS9930_GA;
	int APDS9930_OUTDOOR_GA=250;//125;

	if (ch0data >= apds9930_als_res_tb[data->als_atime_index]) {
		luxValue = APDS9930_MAX_LUXVALUE;
		return (luxValue);
	}
	else if (ch1data >= apds9930_als_res_tb[data->als_atime_index]) {
		luxValue = APDS9930_MAX_LUXVALUE;
		return (luxValue);
	}

	if (ch0data == 0) {
		luxValue = 0;
		return (luxValue);
	}

	ch1ch0_ratio = (ch1data*100)/ch0data;	// scale up by 100

	if ( (ch1ch0_ratio >= 30) && (ch1ch0_ratio <= 45) ) {
		// sunglight
		luxValue = (ch0data*100)/((apds9930_als_integration_tb[data->als_atime_index]/100)*apds9930_als_again_tb[data->als_again_index]);
		luxValue = (luxValue * APDS9930_OUTDOOR_GA)/100;

		// DO NOT USE ALS_REDUCE 

		return luxValue;
	}
	else {
		// white light/Incand
		APDS9930_COE_B = APDS9930_COE_B1;
		APDS9930_COE_C = APDS9930_COE_C1;
		APDS9930_COE_D = APDS9930_COE_D1;
		APDS9930_GA = APDS9930_COE_GA1;		
	}

	IAC1 = (ch0data*100 - (APDS9930_COE_B*ch1data));			// re-adjust COE_B to avoid 2 decimal point
	IAC2 = ((APDS9930_COE_C*ch0data) - (APDS9930_COE_D*ch1data));	// re-adjust COE_C and COE_D to void 2 decimal point

	if (IAC1 > IAC2)
		IAC = IAC1;
	else if (IAC1 <= IAC2)
		IAC = IAC2;
	else
		IAC = 0;

	if ((IAC1<0) && (IAC2<0)) {
		if (ch0data < (apds9930_als_res_tb[data->als_atime_index]/2)) {
			IAC = 0;	// cdata and irdata saturated
		}
		else {
			luxValue = APDS9930_MAX_LUXVALUE;
			return (luxValue);
		}
		
		//APS_ERR("LuxCalculation error with iac1=%d iac2=%d atime=%d again=%d\n", IAC1, IAC2, data->als_atime_index, data->als_again_index);
		//return -1; 	// don't report first, change gain may help
	}
/*  update lux convert algo from avago FAE --20130817 molg1.lenovo end*/
	data1 = (u64)IAC;
	data1 = (data1*APDS9930_GA*APDS9930_DF);
	data1 = div_u64(data1, 10);
	data2 = ((apds9930_als_integration_tb[data->als_atime_index]/100)*apds9930_als_again_tb[data->als_again_index]);
	if (data->als_reduce) {
		//luxValue = 10 * ((IAC*APDS9930_GA*APDS9930_DF)/100)*4/((apds9930_als_integration_tb[data->als_atime_index]/100)*apds9930_als_again_tb[data->als_again_index]);
		data1 = data1 * 4;
		luxValue = div_u64(data1,data2);
	}
	else {
		//luxValue = 10 * ((IAC*APDS9930_GA*APDS9930_DF)/100)/((apds9930_als_integration_tb[data->als_atime_index]/100)*apds9930_als_again_tb[data->als_again_index]);
		luxValue = div_u64(data1,data2);
	}
	
	return luxValue;
}

/*  update lux convert algo from avago FAE --liaoxl.lenovo ends */

int APDS9930_read_als(struct i2c_client *client, int *data)
{	 
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	int ch0data, ch1data, luxValue;	 
	u8 buffer[2];
	u8 change_again=0;
	u8 change_reduce=0;
	u8 control_data=0;
	int res = 0;
	

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}
	/*lenovo sw molg1 add for auto_brightness issue 20130808  begin*/
	if(1 == atomic_read(&obj->als_suspend))
	{
		return -1;
	}
	/*lenovo sw molg1 add for auto_brightness issue 20130808  end*/
	if(1 == atomic_read(&obj->als_deb_on))
	{
		unsigned long endt = atomic_read(&obj->als_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->als_deb_on, 0);
		}
		
		if(1 == atomic_read(&obj->als_deb_on))
		{
			APS_ERR("APDS9930_read_als @deboun\n");
			return -1;
		}
	}

	buffer[0]=APDS9930_CMM_C0DATA_L;
	res = APDS9930_i2c_master_operate(client, buffer, 0x201, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	ch0data = buffer[0] | (buffer[1]<<8);
	//APS_LOG("c0_value=%d, c0_nf=%d, als_modulus=%d\n", c0_value, c0_nf, obj->als_modulus);

	buffer[0]=APDS9930_CMM_C1DATA_L;
	res = APDS9930_i2c_master_operate(client, buffer, 0x201, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	ch1data = buffer[0] | (buffer[1]<<8);
	luxValue = LuxCalculation(client, ch0data, ch1data);
	//APS_LOG("APDS9930_read_als get ch0=%d ch1=%d lux100=%d @gain=%d\n", ch0data, ch1data, luxValue, obj->als_again_index);
	
	if (luxValue >= 0) {
		luxValue = (luxValue < APDS9930_MAX_LUXVALUE) ? luxValue : APDS9930_MAX_LUXVALUE;
	}
	else {
		if (obj->als_reduce) {
			//lux_is_valid = 1;
			luxValue = APDS9930_MAX_LUXVALUE;	// report anyway since this is the lowest gain
			APS_LOG("APDS9930_read_als max value!\n");
		}
		else
		{
			APS_LOG("APDS9930_read_als retry with ch0=%d ch1=%d ret=%d\n", ch0data, ch1data, luxValue);
		}
	}

	*data = luxValue;
	
	if (ch0data >= (apds9930_als_res_tb[obj->als_atime_index]*90)/100) {
		// lower AGAIN if possible
		if (obj->als_again_index != APDS9930_ALS_GAIN_1X) {
			obj->als_again_index--;
			change_again = 1;
		}
		else {
			if(obj->als_reduce == 0)
			{
/*  update lux convert algo from avago FAE --20130817 molg1.lenovo begin*/
				/*
				buffer[0] = APDS9930_CMM_CONFIG;    
				buffer[1] = 0x04;
				res = APDS9930_i2c_master_operate(client, buffer, 0x2, I2C_FLAG_WRITE);
			
				if (res > 0) {			
					obj->als_reduce = 1;
					change_reduce = 1;
				}
				*/
/*  update lux convert algo from avago FAE --20130817 molg1.lenovo end*/
			}
			else
			{
			}
		}	
	}
	else if (ch0data <= (apds9930_als_res_tb[obj->als_atime_index]*10)/100) {
		// increase AGAIN if possible
		if (obj->als_reduce) {
			if(obj->als_reduce == 1)
			{
				buffer[0] = APDS9930_CMM_CONFIG;    
				buffer[1] = 0x00;
				res = APDS9930_i2c_master_operate(client, buffer, 0x2, I2C_FLAG_WRITE);
				if (res > 0) {			
					obj->als_reduce = 0;
					change_reduce = 1;
				}
			}
			else
			{
			}
		}
		else if (obj->als_again_index != APDS9930_ALS_GAIN_120X) {
			obj->als_again_index++;
			change_again = 1;
		}
	}

	if (change_again) {
		buffer[0] = APDS9930_CMM_CONTROL;    
		buffer[1] = (APDS9930_CMM_CONTROL_VALUE & 0xFC) | apds9930_als_again_bit_tb[obj->als_again_index];
		res = APDS9930_i2c_master_operate(client, buffer, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			/* do nothing now */
		}
	}

	if(change_again | change_reduce)
	{
		atomic_set(&obj->als_deb_on, 1);
		atomic_set(&obj->als_deb_end, jiffies+atomic_read(&obj->als_debounce)*HZ/1000);
	}
	else
	{

	}

	//APS_LOG("APDS9930_read_als als_value_lux = %d\n", *data);
	if(luxValue < 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}

EXIT_ERR:
	APS_ERR("APDS9930_read_als fail\n");
	return -1;
}
int APDS9930_read_als_ch0(struct i2c_client *client, u16 *data)
{	 
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	u16 c0_value;	 
	u8 buffer[2];
	int res = 0;
	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

//get adc channel 0 value
	buffer[0]=APDS9930_CMM_C0DATA_L;
	res = APDS9930_i2c_master_operate(client, buffer, 0x201, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	c0_value = buffer[0] | (buffer[1]<<8);
	if(obj->als_reduce == 0) 
	{
		*data = c0_value;
	} else {
		*data = c0_value * 4;
	}

	//APS_LOG("c0_value=%d\n", c0_value);
	return 0;	 

	
	
EXIT_ERR:
	APS_ERR("APDS9930_read_als_ch0 fail\n");
	return res;
}
/*----------------------------------------------------------------------------*/

int APDS9930_read_als_ch1(struct i2c_client *client, u16 *data)
{	 
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	u16 c1_value;	 
	u8 buffer[2];
	int res = 0;
	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

//get adc channel 1 value
	buffer[0]=APDS9930_CMM_C1DATA_L;
	res = APDS9930_i2c_master_operate(client, buffer, 0x201, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	c1_value = buffer[0] | (buffer[1]<<8);
#if 1
	*data = c1_value;
#else
	if(obj->als_reduce == 0) 
	{
		*data = c1_value;
	} else {
		*data = c1_value * 4;
	}
#endif
	//APS_LOG("c0_value=%d\n", c0_value);
	return 0;	 
	
EXIT_ERR:
	APS_ERR("APDS9930_read_als_ch1 fail\n");
	return res;
}

static int APDS9930_get_als_value(struct APDS9930_priv *obj, int als)
{
	int idx;
	int invalid = 0;
	for(idx = 0; idx < obj->als_level_num; idx++)
	{
		if(als <= obj->hw->als_level[idx]) /* light sensor lux value convert-algo  modify   -- liaoxl.lenovo 1.11.2013 */
		{
			break;
		}
	}
	
	if(idx >= obj->als_value_num)
	{
		APS_ERR("APDS9930_get_als_value exceed range\n"); 
		idx = obj->als_value_num - 1;
	}

	if(1 == atomic_read(&obj->als_deb_on))
	{
		invalid = 1;
	}

	if(!invalid)
	{
        u32 level_high = obj->hw->als_level[idx];
    	u32 level_low = (idx > 0) ? obj->hw->als_level[idx-1] : 0;
        u32 level_diff = level_high - level_low;
		u32 value_high = obj->hw->als_value[idx];
        u32 value_low = (idx > 0) ? obj->hw->als_value[idx-1] : 0;
        u64 value_diff = value_high - value_low;
        u64 temp;
        int value = 0;

        temp = (als - level_low) * value_diff;
        temp = div_u64(temp, level_diff);
        value = value_low + temp;
        if(value > value_high)
        {
        	value = value_high;
        }

		//APS_DBG("ALS: %d [%d, %d] => %d [%d, %d] \n", als, level_low, level_high, value, value_low, value_high);
		return value;
	}
	else
	{
		//APS_ERR("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);    
		return -1;
	}
}
/*----------------------------------------------------------------------------*/
/* for lenovo board esd check solution --liaoxl.lenovo 12.11.2012 start */
static int apds_read_regs(struct i2c_client *client)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);	 
	u8 buffer[2];
	int res = 0;
	int i;

    APS_ERR("apds_read_regs start\n");
	for(i = 0; i < 0x1F; i++)
	{
		buffer[0] = i|0x80;
		res = APDS9930_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
		if(res <= 0)
		{
			APS_ERR("apds_read_regs error iic opera!!\n");
		}
		else
		{
			APS_ERR("apds_read_regs reg[0x%x] = 0x%x!!\n", i, buffer[0]);
		}
	}

	return 0;
}

static int apds_check_regs(struct i2c_client *client)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);	 
	u8 buffer[2];
	int res = 0;
	int i;

    //APS_DBG("apds_check_regs start\n");
	buffer[0] = APDS9930_CMM_ATIME;
	res = APDS9930_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		return -EIO;
	}
	if(buffer[0] != apds9930_als_atime_tb[obj->als_atime_index])/*  update lux convert algo from avago FAE --liaoxl.lenovo */
	{
	    APS_ERR("apds_check_regs step1 failed for 0x%x\n", buffer[0]);
		return -EINVAL;
	}
	buffer[0] = APDS9930_CMM_PPCOUNT;
	res = APDS9930_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		return -EIO;
	}
	if(buffer[0] != APDS9930_CMM_PPCOUNT_VALUE)
	{
		APS_ERR("apds_check_regs step2 failed for 0x%x\n", buffer[0]);
        return -EINVAL;
	}
	
	buffer[0] = APDS9930_CMM_CONTROL;
	res = APDS9930_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		return -EIO;
	}

	if(buffer[0] != ((APDS9930_CMM_CONTROL_VALUE & 0xFC) | apds9930_als_again_bit_tb[obj->als_again_index]))/*  update lux convert algo from avago FAE --liaoxl.lenovo  */
	{
		APS_ERR("apds_check_regs step3 failed for 0x%x\n", buffer[0]);
		return -EINVAL;
	}
    //APS_DBG("apds_check_regs end\n");

	return 0;
}

static void apds_esd_check(struct i2c_client *client)
{
	int err;
	struct APDS9930_priv *obj = i2c_get_clientdata(client);

	err = apds_check_regs(obj->client);
	//APS_DBG("apds_esd_check err =%d\n",err);
	if(err == -EIO)
	{
		APS_ERR("check regs error IIC need reset!!\n");
#if 0
		hwPowerDown(MT65XX_POWER_LDO_VGP, "TMD");
		msleep(20);
		hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_2800, "TMD");
		msleep(100);
#endif
	}
	if(0 != err)
	{
		APS_ERR("check regs error data need init!!\n");
		err = APDS9930_init_client(obj->client);
		if(err != 0)
		{
			APS_ERR("initialize client fail!!\n");
		}
		else
		{
			/*lenovo sw molg1 add for auto_brightness issue 20130808 */
			//atomic_set(&obj->als_suspend, 0);
			if(test_bit(CMC_BIT_ALS, &obj->enable))
			{
				err = APDS9930_enable_als(obj->client, 1);
				if(err != 0)
				{
				APS_ERR("enable als fail: %d\n", err);
				}
			}
				/*lenovo sw molg1 add for auto_brightness issue 20130808 */
			//atomic_set(&obj->ps_suspend, 0);
			if(test_bit(CMC_BIT_PS, &obj->enable))
			{
				err = APDS9930_enable_ps(obj->client, 1);
				if(err != 0)
				{
				   APS_ERR("enable ps fail: %d\n", err);
				}
			}
		}
	}
}
/* for lenovo board esd check solution -- liaoxl.lenovo 12.11.2012 end */

long APDS9930_read_ps(struct i2c_client *client, u16 *data)
{
	//struct APDS9930_priv *obj = i2c_get_clientdata(client);	
	u8 buffer[2];
	long res = 0;

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	buffer[0]=APDS9930_CMM_PDATA_L;
	res = APDS9930_i2c_master_operate(client, buffer, 0x201, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	*data = buffer[0] | (buffer[1]<<8);
	//APS_LOG("yucong APDS9930_read_ps ps_data=%d, low:%d  high:%d", *data, buffer[0], buffer[1]);
	return 0;    

EXIT_ERR:
	APS_ERR("APDS9930_read_ps fail\n");
	return res;
}
/*----------------------------------------------------------------------------*/
static int APDS9930_get_ps_value(struct APDS9930_priv *obj, u16 ps)
{
	int val;// mask = atomic_read(&obj->ps_mask);
	int invalid = 0;
	static int val_temp=1;

	if((ps  > atomic_read(&obj->ps_thd_val_high)))
	{
		val = 0;  /*close*/
		val_temp = 0;
		intr_flag_value = 1;
	}
	else if((ps  < atomic_read(&obj->ps_thd_val_low)))
	{
		val = 1;  /*far away*/
		val_temp = 1;
		intr_flag_value = 0;
	}
	else
	    val = val_temp;	
			
	
	if(atomic_read(&obj->ps_suspend))
	{
		invalid = 1;
	}
	else if(1 == atomic_read(&obj->ps_deb_on))
	{
		unsigned long endt = atomic_read(&obj->ps_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->ps_deb_on, 0);
		}
		
		if (1 == atomic_read(&obj->ps_deb_on))
		{
			invalid = 1;
		}
	}
#if 0
	else if(obj->als_ch0 > (75*(1024*(256-apds9930_als_atime_tb[obj->als_atime_index]))/100))/*  update lux convert algo from avago FAE --liaoxl.lenovo  */
	{
		//invalid = 1;
		APS_DBG("ligh too high will result to failt proximiy\n");
		return 1;  /*far away*/
	}
#endif
	else if( 0 != atomic_read(&obj->ps_psat) )
	{
		APS_DBG("ligh too high will result to failt proximiy\n");
		return 1;		
	}

	if(!invalid)
	{
		//APS_DBG("PS:  %05d => %05d\n", ps, val);
		return val;
	}	
	else
	{
		return -1;
	}	
}

/*lenovo sw molg1 add for sun issue 20130521 begin*/
static int  APDS9930_ADC_EN(struct i2c_client *client, int enable)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
		u8 databuf[3];
		int res = 0;
		
		databuf[0] = APDS9930_CMM_ENABLE;

		if (enable)
		{
				if(1)
				{
					databuf[1] = 0x28|0x03|0x05;
				}
				else
				{
					databuf[1] = 0x28|0x05;
				}
		}
		else
		{
			databuf[1] = 0x01;
		}
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			return -1;
		}
		return 0;
		
}
/*lenovo sw molg1 add for sun issue 20130521 end*/

/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
//#define DEBUG_APDS9930
static void APDS9930_eint_work(struct work_struct *work)
{
	struct APDS9930_priv *obj = (struct APDS9930_priv *)container_of(work, struct APDS9930_priv, eint_work);
	int err;
	hwm_sensor_data sensor_data;
	u8 databuf[3];
	int res = 0;

	if((err = APDS9930_check_intr(obj->client)))
	{
		APS_ERR("APDS9930_eint_work check intrs: %d\n", err);
				goto EXIT_ERR;
	}
	else
	{
		APDS9930_clear_intr(obj->client);	
		APDS9930_ADC_EN(obj->client, 0);
		//get raw data
		APDS9930_read_ps(obj->client, &obj->ps);
/*  update lux convert algo from avago FAE --liaoxl.lenovo start */
#if 0 
		APDS9930_read_als_ch0(obj->client, &obj->als);
		APS_LOG("APDS9930_eint_work rawdata ps=%d als_ch0=%d!\n",obj->ps,obj->als);
		
		if(obj->als > 40000)
			{
			APS_LOG("APDS9930_eint_work ALS too large may under lighting als_ch0=%d!\n",obj->als);
			return;
			}

#else		
		APDS9930_read_als_ch0(obj->client, &obj->als_ch0);
	//	APDS9930_read_als_ch1(obj->client, &obj->als_ch1);
		APS_LOG("APDS9930_eint_work rawdata ps=%d als_ch0=%d PSAT=%d!\n",obj->ps,obj->als_ch0,atomic_read(&obj->ps_psat));
	//	APS_LOG("APDS9930_eint_work rawdata ps=%d als_ch0=%d als_ch1=%d PSAT=%d als_reduce=%d!\n",obj->ps,obj->als_ch0,obj->als_ch1,atomic_read(&obj->ps_psat),obj->als_reduce);

#endif
/*  update lux convert algo from avago FAE --liaoxl.lenovo end */
		sensor_data.values[0] = APDS9930_get_ps_value(obj, obj->ps);
		sensor_data.value_divide = 1;
		sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;	

#ifdef DEBUG_APDS9930
		databuf[0]= APDS9930_CMM_ENABLE;
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x101, I2C_FLAG_READ);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_LOG("APDS9930_eint_work APDS9930_CMM_ENABLE ps value = %x\n",databuf[0]);
		
		databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_READ);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_LOG("APDS9930_eint_work APDS9930_CMM_INT_LOW_THD_LOW before databuf[0]=%d databuf[1]=%d!\n",databuf[0],databuf[1]);

		databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_READ);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_LOG("APDS9930_eint_work APDS9930_CMM_INT_HIGH_THD_LOW before databuf[0]=%d databuf[1]=%d!\n",databuf[0],databuf[1]);
#endif
/*singal interrupt function add*/
		if(intr_flag_value){
						databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;	
						databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low)) & 0x00FF);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
						
						databuf[0] = APDS9930_CMM_INT_LOW_THD_HIGH;	
						databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_low)) & 0xFF00) >> 8);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}


							databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;	
							databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high)) & 0x00FF);
							res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
							if(res <= 0)
							{
								goto EXIT_ERR;
							}
						
							databuf[0] = APDS9930_CMM_INT_HIGH_THD_HIGH; 
							databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_high)) & 0xFF00) >> 8);
							res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
							if(res <= 0)
							{
								goto EXIT_ERR;
							}
				}
				else{	
						databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;	
						databuf[1] = (u8)(0 & 0x00FF);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
						
						databuf[0] = APDS9930_CMM_INT_LOW_THD_HIGH;	
						databuf[1] = (u8)((0 & 0xFF00) >> 8);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
						
						databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;	
						databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high)) & 0x00FF);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
					
						databuf[0] = APDS9930_CMM_INT_HIGH_THD_HIGH; 
						databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_high)) & 0xFF00) >> 8);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
#if 0  /* remove redundant iic operation code  -- by liaoxl.lenovo 3.18.2013  */
						res = i2c_master_send(obj->client, databuf, 0x2);
#endif
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
				}
				
		//let up layer to know
		#ifdef DEBUG_APDS9930
		databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_READ);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_LOG("APDS9930_eint_work APDS9930_CMM_INT_LOW_THD_LOW after databuf[0]=%d databuf[1]=%d!\n",databuf[0],databuf[1]);

		databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_READ);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_LOG("APDS9930_eint_work APDS9930_CMM_INT_HIGH_THD_LOW after databuf[0]=%d databuf[1]=%d!\n",databuf[0],databuf[1]);
		#endif
		if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
		{
		  APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
		}
	}
	
	//APDS9930_clear_intr(obj->client);
	atomic_set(&obj->wq_process, 0);/* add for esd check func  -- by liaoxl.lenovo 4.11.2013  */
	mt_eint_unmask(CUST_EINT_ALS_NUM); 
	APDS9930_ADC_EN(obj->client, 1);
	return;

	EXIT_ERR:
	APDS9930_clear_intr(obj->client);
	atomic_set(&obj->wq_process, 0);/* add for esd check func  -- by liaoxl.lenovo 4.11.2013  */
	//begin, lenovo-sw lumy1 add for screen on failed
	databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;
	databuf[1] = 0;
	res = APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_WRITE);
	databuf[0] = APDS9930_CMM_INT_LOW_THD_HIGH;
	databuf[1] = 0;
	res += APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_WRITE);
	databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;
	databuf[1] = 0;
	res += APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_WRITE);
	databuf[0] = APDS9930_CMM_INT_HIGH_THD_HIGH;
	databuf[1] = 0;
	res += APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_WRITE);
	APS_ERR ("force interrupt ret =%d\n", res);
	//end, lenovo-sw lumy1 add for screen on failed
	mt_eint_unmask(CUST_EINT_ALS_NUM); 
	//begin, lenovo-sw lumy1 add for screen on failed
	APDS9930_ADC_EN(obj->client, 1); 
	//end, lenovo-sw lumy1 add for screen on failed
	APS_ERR("i2c_transfer error = %d\n", res);
	return;
}


/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int APDS9930_open(struct inode *inode, struct file *file)
{
	file->private_data = APDS9930_i2c_client;

	if (!file->private_data)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}
	
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int APDS9930_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

/*----------------------------------------------------------------------------*/
static int l_ps_average_val = 0;

static void le_WriteCalibration(struct APDS9930_priv *obj, HWMON_PS_STRUCT *data_cali)
{
    struct PS_CALI_DATA_STRUCT *ps_data_cali;
	APS_LOG("le_WriteCalibration  1 %d," ,data_cali->close);
	APS_LOG("le_WriteCalibration  2 %d," ,data_cali->far_away);
	APS_LOG("le_WriteCalibration  3 %d,", data_cali->valid);
	//APS_LOG("le_WriteCalibration  4 %d,", data_cali->pulse);

	if(data_cali->valid == 1)
	{
		atomic_set(&obj->ps_thd_val_high, data_cali->close);
		atomic_set(&obj->ps_thd_val_low, data_cali->far_away);
	}

}

static int le_read_data_for_cali(struct i2c_client *client, HWMON_PS_STRUCT *ps_data_cali)
{
	int i=0 ,err = 0;
	u16 data[32],sum,data_cali,max,min;

	ps_data_cali->valid = 0;
	sum = 0;
	max = 0;
	min = 1023;
	for(i = 0; i < 32; i++)
	{
			mdelay(5);//50
			err = APDS9930_read_ps(client,&data[i]);
			if((err != 0) || (data[i] >= 1024))
			{
				APS_ERR("le_read_data_for_cali fail: %d\n", i); 
				break;
			}
			else
			{
				APS_LOG("[%d]sample = %d\n", i, data[i]);
				sum += data[i];
				if(data[i] > max)
				{
					max = data[i];
				}
				if(data[i] < min)
				{
					min = data[i];
				}
			}
			mdelay(55);//160
	 }
	 
				
	if(i < 32)
	{
		err=  -1;

		return err;
	}
	else
	{
		data_cali = sum / 32;
		l_ps_average_val = data_cali;
		APS_LOG("le_read_data_for_cali data = %d sum=%d",data_cali,sum);
//		ps_data_cali->avg = data_cali;
//		ps_data_cali->max = max;
//		ps_data_cali->min = min;
		 if( data_cali>600)
		{
			APS_ERR("le_read_data_for_cali fail value to high: %d\n", data_cali);
			return -2;
		}	

		 if( data_cali<=150)
		{
			ps_data_cali->close =330;
			ps_data_cali->far_away =300;
		}
		else if( data_cali<=300)
		{
			ps_data_cali->close =data_cali *2;
			ps_data_cali->far_away =data_cali *17/10;
		}
		else if( data_cali<600)
		{
			ps_data_cali->close =data_cali *18/10;
			ps_data_cali->far_away =data_cali *15/10;
		}
		if(ps_data_cali->close >900)
		{
			ps_data_cali->close = 900;
			ps_data_cali->far_away = 750;
		}

		ps_data_cali->valid = 1;
		APS_LOG("le_read_data_for_cali close = %d,far_away = %d,valid = %d\n",ps_data_cali->close,ps_data_cali->far_away,ps_data_cali->valid);
		APS_LOG("le_read_data_for_cali avg=%d max=%d min=%d\n",data_cali, max, min);
	}

	return err;
}

static long APDS9930_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct APDS9930_priv *obj = i2c_get_clientdata(client);  
	long err = 0;
	void __user *ptr = (void __user*) arg;
	int dat;
	uint32_t enable;
	int ps_result;
	/*for ps cali work mode support -- by liaoxl.lenovo 2.08.2011 start*/
	HWMON_PS_STRUCT ps_cali_temp;
	/*for ps cali work mode support -- by liaoxl.lenovo 2.08.2011 end*/

	switch (cmd)
	{
		case ALSPS_SET_PS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if((err = APDS9930_enable_ps(obj->client, 1)))
				{
					APS_ERR("enable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				set_bit(CMC_BIT_PS, &obj->enable);
			}
			else
			{
				if((err = APDS9930_enable_ps(obj->client, 0)))
				{
					APS_ERR("disable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				clear_bit(CMC_BIT_PS, &obj->enable);
			}
			break;

		case ALSPS_GET_PS_MODE:
			enable = test_bit(CMC_BIT_PS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_DATA:    
			if((err = APDS9930_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			
			dat = APDS9930_get_ps_value(obj, obj->ps);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;

		case ALSPS_GET_PS_RAW_DATA:    
			if((err = APDS9930_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			
			dat = obj->ps;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;              

		case ALSPS_SET_ALS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if((err = APDS9930_enable_als(obj->client, 1)))
				{
					APS_ERR("enable als fail: %ld\n", err); 
					goto err_out;
				}
				set_bit(CMC_BIT_ALS, &obj->enable);
			}
			else
			{
				if((err = APDS9930_enable_als(obj->client, 0)))
				{
					APS_ERR("disable als fail: %ld\n", err); 
					goto err_out;
				}
				clear_bit(CMC_BIT_ALS, &obj->enable);
			}
			break;

		case ALSPS_GET_ALS_MODE:
			enable = test_bit(CMC_BIT_ALS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_ALS_DATA: 
			if((err = APDS9930_read_als(obj->client, &obj->als)))
			{
				goto err_out;
			}

			dat = APDS9930_get_als_value(obj, obj->als);
			dat /= 100;			
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

		case ALSPS_GET_ALS_RAW_DATA:    
			if((err = APDS9930_read_als(obj->client, &obj->als)))
			{
				goto err_out;
			}

			dat = obj->als;
			dat /= 100;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;
		/*----------------------------------for factory mode test---------------------------------------*/
		case ALSPS_GET_PS_TEST_RESULT:
			if((err = APDS9930_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			if(obj->ps > atomic_read(&obj->ps_thd_val_high))
				{
					ps_result = 0;
				}
			else	ps_result = 1;
				
			if(copy_to_user(ptr, &ps_result, sizeof(ps_result)))
			{
				err = -EFAULT;
				goto err_out;
			}			   
			break;
			/*------------------------------------------------------------------------------------------*/

		case ALSPS_SET_PS_CALI:
			dat = (void __user*)arg;
			if(dat == NULL)
			{
				APS_LOG("dat == NULL\n");
				err = -EINVAL;
				break;	  
			}
			if(copy_from_user(&ps_cali_temp,dat, sizeof(ps_cali_temp)))
			{
				APS_LOG("copy_from_user\n");
				err = -EFAULT;
				break;	  
			}
			le_WriteCalibration(obj, &ps_cali_temp);
			APS_LOG(" ALSPS_SET_PS_CALI %d,%d,%d\t",ps_cali_temp.close,ps_cali_temp.far_away,ps_cali_temp.valid);
			break;

		/*for ps cali work mode support -- by liaoxl.lenovo 2.08.2011 start*/
		case ALSPS_GET_PS_RAW_DATA_FOR_CALI:
		{
			u8 pulse,old;
			#if 0
				cancel_work_sync(&obj->eint_work);
				mt65xx_eint_mask(CUST_EINT_ALS_NUM);

				pulse = TMD2771_CMM_PPCOUNT_VALUE;
				tmd2771_init_client_for_cali(obj->client, pulse);
				msleep(200);
				err = tmd2771_read_data_for_cali(obj->client,&ps_cali_temp);
				if(err < 0 ){
					goto err_out;
				}	
				tmd2771_enable(obj->client, 0);
				msleep(60);
				
				ps_cali_temp.pulse = TMD2771_CMM_PPCOUNT_VALUE;
				tmd2771_WriteCalibration(&ps_cali_temp);

				//tmd2771_init_client(obj->client);
				tmd2771_enable(obj->client, 0);
			#else
				cancel_work_sync(&obj->eint_work);
				mt_eint_mask(CUST_EINT_ALS_NUM);
				err = le_read_data_for_cali(obj->client,&ps_cali_temp);
				if(err < 0 ){
					APS_LOG("APDS9930_unlocked_ioctl == GET_PS_RAW_DATA_FOR_CALI err=%d\n", err);
					goto err_out;
				}	
//				ps_cali_temp.pulse = APDS9930_CMM_PPCOUNT_VALUE;
				//tmd2771_WriteCalibration(&ps_cali_temp);
			#endif
				if(copy_to_user(ptr, &ps_cali_temp, sizeof(ps_cali_temp)))
					{
						err = -EFAULT;
						goto err_out;
					}
					}
			break;
			/*for ps cali work mode support -- by liaoxl.lenovo 2.08.2011 end*/
		//lenovo-sw, shanghai, add by chenlj2, for geting ps average val, 2012-05-14 begin
		case ALSPS_GET_PS_AVERAGE:
			enable = l_ps_average_val;	
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;
		//lenovo-sw, shanghai, add by chenlj2, for geting ps average val, 2012-05-14 end
		case ALSPS_GET_PS_FAR_THRESHOLD:
			enable = atomic_read(&obj->ps_thd_val_low);	
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;
	        case ALSPS_GET_PS_CLOSE_THRESHOLD:
			enable = atomic_read(&obj->ps_thd_val_high);	
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

		default:
			APS_ERR("%s not supported = 0x%04x", __FUNCTION__, cmd);
			err = -ENOIOCTLCMD;
			break;
	}

	err_out:
	return err;    
}
/*----------------------------------------------------------------------------*/
static struct file_operations APDS9930_fops = {
	.owner = THIS_MODULE,
	.open = APDS9930_open,
	.release = APDS9930_release,
	.unlocked_ioctl = APDS9930_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice APDS9930_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &APDS9930_fops,
};
/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_suspend(struct i2c_client *client, pm_message_t msg) 
{
//	struct APDS9930_priv *obj = i2c_get_clientdata(client);    
//	int err;
	APS_FUN();    
#if 0
	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(!obj)
		{
			APS_ERR("null pointer!!\n");
			return -EINVAL;
		}
		
		atomic_set(&obj->als_suspend, 1);
		if(err = APDS9930_enable_als(client, 0))
		{
			APS_ERR("disable als: %d\n", err);
			return err;
		}

		atomic_set(&obj->ps_suspend, 1);
		if(err = APDS9930_enable_ps(client, 0))
		{
			APS_ERR("disable ps:  %d\n", err);
			return err;
		}
		
		APDS9930_power(obj->hw, 0);
	}
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_resume(struct i2c_client *client)
{
//	struct APDS9930_priv *obj = i2c_get_clientdata(client);        
//	int err;
	APS_FUN();
#if 0
	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}

	APDS9930_power(obj->hw, 1);
	if(err = APDS9930_init_client(client))
	{
		APS_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->als_suspend, 0);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if(err = APDS9930_enable_als(client, 1))
		{
			APS_ERR("enable als fail: %d\n", err);        
		}
	}
	atomic_set(&obj->ps_suspend, 0);
	if(test_bit(CMC_BIT_PS,  &obj->enable))
	{
		if(err = APDS9930_enable_ps(client, 1))
		{
			APS_ERR("enable ps fail: %d\n", err);                
		}
	}
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static void APDS9930_early_suspend(struct early_suspend *h) 
{   /*early_suspend is only applied for ALS*/
	struct APDS9930_priv *obj = container_of(h, struct APDS9930_priv, early_drv);   
	int err;
	APS_FUN();    

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

//	apds_read_regs(obj->client);/* add for esd check verfy func  -- by liaoxl.lenovo 4.11.2013  */

	/*lenovo sw molg1 add for auto_brightness issue 20130808 */
	atomic_set(&obj->als_suspend, 1);
	#if 0
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if((err = APDS9930_enable_als(obj->client, 0)))
		{
			APS_ERR("disable als fail: %d\n", err); 
		}
	}
	#endif
}
/*----------------------------------------------------------------------------*/
static void APDS9930_late_resume(struct early_suspend *h)
{   /*early_suspend is only applied for ALS*/
	struct APDS9930_priv *obj = container_of(h, struct APDS9930_priv, early_drv);         
	int err;
	APS_FUN();

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}
/* add for esd check func start -- by liaoxl.lenovo 4.11.2013  */
	if(test_bit(CMC_BIT_PS, &obj->enable))
	{
		if(0 == atomic_read(&obj->wq_process))
		{
			mt_eint_mask(CUST_EINT_ALS_NUM);
			apds_esd_check(obj->client);
			mt_eint_unmask(CUST_EINT_ALS_NUM);

			APS_ERR("APDS9930_late_resume start wq!!\n");
		}
		else
		{
			APS_DBG("APDS9930_late_resume skip 2!!\n");
		}
	}
	else
	{
		APS_DBG("APDS9930_late_resume skip 1!!\n");
	}
/* add for esd check func end -- by liaoxl.lenovo 4.11.2013  */
        #if 0
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if((err = APDS9930_enable_als(obj->client, 1)))
		{
			APS_ERR("enable als fail: %d\n", err);        

		}
	}
	#endif
	/*lenovo sw molg1 add for auto_brightness issue 20130808 */
	atomic_set(&obj->als_suspend, 0);
}
/*----------------------------------------------------------------------------*/
static int temp_als = 0;
static int ALS_FLAG = 0;

int APDS9930_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int value;
	int err = 0;
	
	hwm_sensor_data* sensor_data;
	struct APDS9930_priv *obj = (struct APDS9930_priv *)self;
	
	//APS_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{	
				value = *(int *)buff_in;
				if(value)
				{
					apds_esd_check(obj->client);/* add for esd check func  -- by liaoxl.lenovo 4.11.2013  */

					/* for ps not resp when first time enable issue -- by liaoxl.lenovo 12.27.2012 start */
					if(test_bit(CMC_BIT_PS, &obj->enable))
					{
						/* don't need to reset threshold settings */
					}
					else
					{
						u8 databuf[3];
						int res;

						databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;	
						databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low)) & 0x00FF);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							return -1;
						}
						
						databuf[0] = APDS9930_CMM_INT_LOW_THD_HIGH;	
						databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_low)) & 0xFF00) >> 8);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							return -1;
						}

						databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;	
						databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high)) & 0x00FF);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							return -1;
						}
						
						databuf[0] = APDS9930_CMM_INT_HIGH_THD_HIGH; 
						databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_high)) & 0xFF00) >> 8);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							return -1;
						}
					}
					/* for ps not resp when first time enable issue -- by liaoxl.lenovo 12.27.2012 end */

					if((err = APDS9930_enable_ps(obj->client, 1)))
					{
						APS_ERR("enable ps fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_PS, &obj->enable);

					#if 0
					if(!test_bit(CMC_BIT_ALS, &obj->enable))
					{
						ALS_FLAG = 1;
						if((err = APDS9930_enable_als(obj->client, 1)))
						{
							APS_ERR("enable als fail: %d\n", err); 
							return -1;
						}
					}
					#endif
				}
				else
				{
					if((err = APDS9930_enable_ps(obj->client, 0)))
					{
						APS_ERR("disable ps fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_PS, &obj->enable);
					#if 0
					if(ALS_FLAG == 1)
					{
						/* fix als error status after ps sensor disable  --by liaoxl.lenovo 1.5.2013 start */
						if(!test_bit(CMC_BIT_ALS, &obj->enable))
						{
							if((err = APDS9930_enable_als(obj->client, 0)))
							{
								APS_ERR("disable als fail: %d\n", err); 
								return -1;
							}
						}
						/* fix als error status after ps sensor disable  --by liaoxl.lenovo 1.5.2013 end */
						ALS_FLAG = 0;
					}
					#endif
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;	
				APDS9930_read_ps(obj->client, &obj->ps);
				APDS9930_read_als_ch0(obj->client, &obj->als_ch0);/*  update lux convert algo from avago FAE --liaoxl.lenovo  */
				APS_ERR("APDS9930_ps_operate als data=%d!\n",obj->als);
				sensor_data->values[0] = APDS9930_get_ps_value(obj, obj->ps);
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;			
			}
			break;
		default:
			APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}


int APDS9930_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct APDS9930_priv *obj = (struct APDS9930_priv *)self;

	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
/*  update lux convert algo from avago FAE --liaoxl.lenovo start */
//			value = *(int *)buff_in;
//			apds9930_set_als_poll_delay(obj->client, value);
/*  update lux convert algo from avago FAE --liaoxl.lenovo end */
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;				
				if(value)
				{
					if((err = APDS9930_enable_als(obj->client, 1)))
					{
						APS_ERR("enable als fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_ALS, &obj->enable);
				}
				else
				{
					if((err = APDS9930_enable_als(obj->client, 0)))
					{
						APS_ERR("disable als fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_ALS, &obj->enable);
				}
				
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;
			
				value = APDS9930_read_als(obj->client, &obj->als);
				if(value < 0)
				{
					/*lenovo sw molg1 add for auto_brightness issue 20130808 */
					sensor_data->values[0] = -1;
					APS_ERR("APDS9930_als_operate 1 return als=%d! sus=%d deb=%d\n", sensor_data->values[0], atomic_read(&obj->als_suspend), atomic_read(&obj->als_deb_on));
				}
/*  update lux convert algo from avago FAE --liaoxl.lenovo start */
				else
				{
					sensor_data->values[0] = APDS9930_get_als_value(obj, obj->als);
					if(sensor_data->values[0] < 0)
					{
						/*lenovo sw molg1 add for auto_brightness issue 20130808 */
						sensor_data->values[0] = -1;
					}
					else
					{
						temp_als = sensor_data->values[0];
					}
				}
				sensor_data->value_divide = 100;
/*  update lux convert algo from avago FAE --liaoxl.lenovo end */
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
			}
			break;
		default:
			APS_ERR("light sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}


/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_detect(struct i2c_client *client, struct i2c_board_info *info) 
{    
	strcpy(info->type, APDS9930_DEV_NAME);
	return 0;
}

/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct APDS9930_priv *obj;
	struct hwmsen_object obj_ps, obj_als;
	int err = 0;

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	memset(obj, 0, sizeof(*obj));
	APDS9930_obj = obj;
	obj->hw = get_cust_alsps_hw();
	APDS9930_get_addr(obj->hw, &obj->addr);

	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	INIT_WORK(&obj->eint_work, APDS9930_eint_work);
	obj->client = client;
	i2c_set_clientdata(client, obj);	
	atomic_set(&obj->als_debounce, 130);
	atomic_set(&obj->als_deb_on, 0);
	atomic_set(&obj->als_deb_end, 0);
	atomic_set(&obj->ps_debounce, 10);
	atomic_set(&obj->ps_deb_on, 0);
	atomic_set(&obj->ps_deb_end, 0);
	atomic_set(&obj->ps_mask, 0);
	atomic_set(&obj->als_suspend, 0);
	atomic_set(&obj->als_cmd_val, 0xDF);
	atomic_set(&obj->ps_cmd_val,  0xC1);
	atomic_set(&obj->ps_thd_val_high,  obj->hw->ps_threshold_high);
	atomic_set(&obj->ps_thd_val_low,  obj->hw->ps_threshold_low);
	atomic_set(&obj->wq_process, 0);/* add for esd check func  -- by liaoxl.lenovo 4.11.2013  */
	obj->enable = 0;
	obj->pending_intr = 0;
	obj->als_level_num = sizeof(obj->hw->als_level)/sizeof(obj->hw->als_level[0]);
	obj->als_value_num = sizeof(obj->hw->als_value)/sizeof(obj->hw->als_value[0]);  
	/*Lenovo-sw chenlj2 add 2011-06-03,modified gain 16 to 1/5 accoring to actual thing */
	obj->als_modulus = (400*100*ZOOM_TIME)/(1*150);//(1/Gain)*(400/Tine), this value is fix after init ATIME and CONTROL register value
										//(400)/16*2.72 here is amplify *100 //16
	BUG_ON(sizeof(obj->als_level) != sizeof(obj->hw->als_level));
	memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
	BUG_ON(sizeof(obj->als_value) != sizeof(obj->hw->als_value));
	memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
	atomic_set(&obj->i2c_retry, 3);
	set_bit(CMC_BIT_ALS, &obj->enable);
	set_bit(CMC_BIT_PS, &obj->enable);
/*  update lux convert algo from avago FAE --liaoxl.lenovo start */
	obj->als_atime_index = APDS9930_ALS_RES_51MS;	// 100ms ATIME
	obj->als_again_index = APDS9930_ALS_GAIN_1X;	// 8x AGAIN
	obj->als_reduce = 0;	// no ALS 6x reduction
/*  update lux convert algo from avago FAE --liaoxl.lenovo end */	
	APDS9930_i2c_client = client;
	
	if(1 == obj->hw->polling_mode_ps)
		//if (1)
		{
			obj_ps.polling = 1;
		}
		else
		{
			obj_ps.polling = 0;
		}
	
	if((err = APDS9930_init_client(client)))
	{
		goto exit_init_failed;
	}
	APS_LOG("APDS9930_init_client() OK!\n");

	if((err = misc_register(&APDS9930_device)))
	{
		APS_ERR("APDS9930_device register failed\n");
		goto exit_misc_device_register_failed;
	}
/*
	if(err = APDS9930_create_attr(&APDS9930_alsps_driver.driver))
	{
		APS_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
*/
	obj_ps.self = APDS9930_obj;
	
	obj_ps.sensor_operate = APDS9930_ps_operate;
	if((err = hwmsen_attach(ID_PROXIMITY, &obj_ps)))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}
	
	obj_als.self = APDS9930_obj;
	obj_als.polling = 1;
	obj_als.sensor_operate = APDS9930_als_operate;
	if((err = hwmsen_attach(ID_LIGHT, &obj_als)))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}


#if defined(CONFIG_HAS_EARLYSUSPEND)
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = APDS9930_early_suspend,
	obj->early_drv.resume   = APDS9930_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif

	APS_LOG("%s: OK\n", __func__);
	return 0;

	exit_create_attr_failed:
	misc_deregister(&APDS9930_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(client);
	//exit_kfree:
	kfree(obj);
	exit:
	APDS9930_i2c_client = NULL;           
//	MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  /*mask interrupt if fail*/
	APS_ERR("%s: err = %d\n", __func__, err);
	return err;
}
/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_remove(struct i2c_client *client)
{
	int err;	
/*	
	if(err = APDS9930_delete_attr(&APDS9930_i2c_driver.driver))
	{
		APS_ERR("APDS9930_delete_attr fail: %d\n", err);
	} 
*/
	if((err = misc_deregister(&APDS9930_device)))
	{
		APS_ERR("misc_deregister fail: %d\n", err);    
	}
	
	APDS9930_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));

	return 0;
}
/*----------------------------------------------------------------------------*/
static int APDS9930_probe(struct platform_device *pdev) 
{
	struct alsps_hw *hw = get_cust_alsps_hw();

	APDS9930_power(hw, 1);    
	//APDS9930_force[0] = hw->i2c_num;
	//APDS9930_force[1] = hw->i2c_addr[0];
	//APS_DBG("I2C = %d, addr =0x%x\n",APDS9930_force[0],APDS9930_force[1]);
	if(i2c_add_driver(&APDS9930_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	} 
	return 0;
}
/*----------------------------------------------------------------------------*/
static int APDS9930_remove(struct platform_device *pdev)
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();    
	APDS9930_power(hw, 0);    
	i2c_del_driver(&APDS9930_i2c_driver);
	return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver APDS9930_alsps_driver = {
	.probe      = APDS9930_probe,
	.remove     = APDS9930_remove,    
	.driver     = {
		.name  = "als_ps",
//		.owner = THIS_MODULE,
	}
};
/*----------------------------------------------------------------------------*/
static int __init APDS9930_init(void)
{
	//APS_FUN();
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_LOG("%s: i2c_number=%d\n", __func__,hw->i2c_num); 
	i2c_register_board_info(hw->i2c_num, &i2c_APDS9930, 1);
	if(platform_driver_register(&APDS9930_alsps_driver))
	{
		APS_ERR("failed to register driver");
		return -ENODEV;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit APDS9930_exit(void)
{
	APS_FUN();
	platform_driver_unregister(&APDS9930_alsps_driver);
}
/*----------------------------------------------------------------------------*/
module_init(APDS9930_init);
module_exit(APDS9930_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("Dexiang Liu");
MODULE_DESCRIPTION("APDS9930 driver");
MODULE_LICENSE("GPL");

