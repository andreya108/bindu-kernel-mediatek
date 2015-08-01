/* drivers/hwmon/mt6516/amit/rpr400.c - RPR400 ALS/PS driver
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
#include <linux/mutex.h>
//#include <mach/mt_gpio.h>


//#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>


#define POWER_NONE_MACRO MT65XX_POWER_NONE
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
#include "rpr400.h"

/******************************************************************************
 * configuration
*******************************************************************************/
/*----------------------------------------------------------------------------*/

#define RPR400_DEV_NAME     "RPR400"
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[ALS/PS] "
#define APS_FUN(f)               printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(KERN_INFO APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_INFO APS_TAG fmt, ##args)                 
/******************************************************************************
 * extern functions
*******************************************************************************/
/*for interrup work mode support --add by liaoxl.lenovo 12.08.2011*/
extern void mt_eint_unmask(unsigned int line);
extern void mt_eint_mask(unsigned int line);
extern void mt_eint_set_polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
extern void mt_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt_eint_set_sens(kal_uint8 eintno, kal_bool sens);

extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
/*----------------------------------------------------------------------------*/
static struct i2c_client *rpr400_i2c_client = NULL;
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id rpr400_i2c_id[] = {{RPR400_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_RPR400={ I2C_BOARD_INFO("RPR400", (0X74>>1))};
/*the adapter id & i2c address will be available in customization*/
//static unsigned short rpr400_force[] = {0x03, 0X72, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const rpr400_forces[] = { rpr400_force, NULL };
//static struct i2c_client_address_data rpr400_addr_data = { .forces = rpr400_forces,};
/*----------------------------------------------------------------------------*/
static int rpr400_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int rpr400_i2c_remove(struct i2c_client *client);
//static int rpr400_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
/*----------------------------------------------------------------------------*/
static int rpr400_i2c_suspend(struct i2c_client *client, pm_message_t msg);
static int rpr400_i2c_resume(struct i2c_client *client);

static int rpr400_local_init(void);
static int rpr400_remove(void);
static int rpr400_init_flag =-1; // 0<==>OK -1 <==> fail

static struct rpr400_priv *g_rpr400_ptr = NULL;

 struct PS_CALI_DATA_STRUCT
{
    int close;
    int far_away;
    int valid;
} ;

static struct PS_CALI_DATA_STRUCT ps_cali={0,0,0};
static int intr_flag_value = 0;
/*----------------------------------------------------------------------------*/
typedef struct {
    unsigned long long data;
    unsigned long long data0;
    unsigned long long data1;
    unsigned char      gain_data0;
    unsigned char      gain_data1;
    unsigned long      dev_unit;
    unsigned char      als_time;
    unsigned short     als_data0;
    unsigned short     als_data1;
} CALC_DATA;

typedef struct {
    unsigned long positive;
    unsigned long decimal;
} CALC_ANS;
typedef enum {
    CMC_BIT_ALS    = 1,
    CMC_BIT_PS     = 2,
} CMC_BIT;
/*----------------------------------------------------------------------------*/
struct rpr400_i2c_addr {    /*define a series of i2c slave address*/
    u8  write_addr;  
    u8  ps_thd;     /*PS INT threshold*/
};
/*----------------------------------------------------------------------------*/
struct rpr400_priv {
    struct alsps_hw  *hw;
    struct i2c_client *client;
    struct work_struct  eint_work;

    /*i2c address group*/
    struct rpr400_i2c_addr  addr;
    
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


    /*data*/
    u16         als;
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

    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver rpr400_i2c_driver = {	
	.probe      = rpr400_i2c_probe,
	.remove     = rpr400_i2c_remove,
	//.detect     = rpr400_i2c_detect,
	.suspend    = rpr400_i2c_suspend,
	.resume     = rpr400_i2c_resume,
	.id_table   = rpr400_i2c_id,
//	.address_data = &rpr400_addr_data,
	.driver = {
//		.owner          = THIS_MODULE,
		.name           = RPR400_DEV_NAME,
	},
};
static struct sensor_init_info rpr400_init_info = {
		.name = "RPR400",
		.init = rpr400_local_init,
		.uninit = rpr400_remove,
};
static DEFINE_MUTEX(rpr400_mutex);

static struct rpr400_priv *rpr400_obj = NULL;
//static struct platform_driver rpr400_alsps_driver;
/*----------------------------------------------------------------------------*/
int rpr400_get_addr(struct alsps_hw *hw, struct rpr400_i2c_addr *addr)
{
	if(!hw || !addr)
	{
		return -EFAULT;
	}
	addr->write_addr= hw->i2c_addr[0];
	return 0;
}
/*----------------------------------------------------------------------------*/
static void rpr400_power(struct alsps_hw *hw, unsigned int on) 
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
			if(!hwPowerOn(hw->power_id, hw->power_vol, "RPR400")) 
			{
				APS_ERR("power on fails!!\n");
			}
		}
		else
		{
			if(!hwPowerDown(hw->power_id, "RPR400")) 
			{
				APS_ERR("power off fail!!\n");   
			}
		}
	}
	power_on = on;
}
/*----------------------------------------------------------------------------*/
static long rpr400_enable_als(struct i2c_client *client, int enable)
{
	struct rpr400_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];	  
	int res = 0;
	u8 buffer[1];
	u8 reg_value[1];	
	u8 power_state, power_set;
	PWR_ST  pwr_st;
		
	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	client->addr = 0x39;
	mutex_lock(&rpr400_mutex);
	buffer[0] = REG_MODECONTROL;
        client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client, buffer, (1<<8) | 1);
        client->addr = client->addr& I2C_MASK_FLAG;
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
        power_state = buffer[0] & 0xF;
        if (MCTL_TABLE[power_state].PS == 0) 
	{
            	pwr_st.ps_state = CTL_STANDBY;
        } 
	else 
        {
            	pwr_st.ps_state = CTL_STANDALONE;
        }
	#if 1
	/*yucong MTK enable_als function modified for fixing reading register error problem 2012.2.16*/
	if(enable)
	{
		if (pwr_st.ps_state == CTL_STANDALONE)
		{
			power_set = PWRON_PS_ALS;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}	
		}
		else if (pwr_st.ps_state == CTL_STANDBY)
		{
			power_set = PWRON_ONLY_ALS;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
	}
	else
	{	
		if (pwr_st.ps_state == CTL_STANDALONE)
		{
			power_set = PWRON_ONLY_PS;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
		else if (pwr_st.ps_state == CTL_STANDBY)
		{
			power_set = PWRON_STANDBY;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}			
		}		
	}
	#endif
		
	return 0;
		
	EXIT_ERR:
		APS_ERR("rpr400_enable_als fail\n");
		return -1; //return correct value when iic error -- by liaoxl.lenovo 3.15.2012
}

/*----------------------------------------------------------------------------*/
static long rpr400_enable_ps(struct i2c_client *client, int enable)
{
	struct rpr400_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
	u8 buffer[1];
	u8 reg_value[1];
	u8 power_state, power_set;
	PWR_ST  pwr_st;	

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	client->addr = 0x39;
	mutex_lock(&rpr400_mutex);
	buffer[0] = REG_MODECONTROL;
        client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client, buffer, (1<<8) | 1);
        client->addr = client->addr& I2C_MASK_FLAG;
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		APS_LOG("rpr400_enable_ps step 1 ret=%d", res);
		goto EXIT_ERR;
	}
        power_state = buffer[0] & 0xF;
        if (MCTL_TABLE[power_state].ALS == 0) 
	{
            	pwr_st.als_state = CTL_STANDBY;
        } 
	else 
        {
            	pwr_st.als_state = CTL_STANDALONE;
        }
#if 1	
	/*yucong MTK: enable_ps function modified for fixing reading register error problem 2012.2.16*/
	if(enable)
	{
		if (pwr_st.als_state == CTL_STANDALONE)
		{
			power_set = PWRON_PS_ALS;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				APS_LOG("rpr400_enable_ps step 2 ret=%d", res);
				goto EXIT_ERR;
			}
		}
		else if (pwr_st.als_state == CTL_STANDBY)
		{
			power_set = PWRON_ONLY_PS;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				APS_LOG("rpr400_enable_ps step 3 ret=%d", res);
				goto EXIT_ERR;
			}
		}
		/*debug code for reading register value*/

		/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
		if(0 == obj->hw->polling_mode_ps)
		{
		        if(1 == ps_cali.valid)
			{
				databuf[0] = REG_PSTL_LSB;	
				databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) & 0x00FF);
				mutex_lock(&rpr400_mutex);
				res = i2c_master_send(client, databuf, 0x2);
				mutex_unlock(&rpr400_mutex);
				if(res <= 0)
				{
					APS_LOG("rpr400_enable_ps step 4 ret=%d", res);
					goto EXIT_ERR;
					return RPR400_ERR_I2C;
				}
				databuf[0] = REG_PSTL_MBS;	
				databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low) & 0xFF00) >> 8);
				mutex_lock(&rpr400_mutex);
				res = i2c_master_send(client, databuf, 0x2);
				mutex_unlock(&rpr400_mutex);
				if(res <= 0)
				{
					APS_LOG("rpr400_enable_ps step 5 ret=%d", res);
					goto EXIT_ERR;
					return RPR400_ERR_I2C;
				}
				databuf[0] = REG_PSTH_LSB;	
				databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) & 0x00FF);
				mutex_lock(&rpr400_mutex);
				res = i2c_master_send(client, databuf, 0x2);
				mutex_unlock(&rpr400_mutex);
				if(res <= 0)
				{
					APS_LOG("rpr400_enable_ps step 6 ret=%d", res);
					goto EXIT_ERR;
					return RPR400_ERR_I2C;
				}
				databuf[0] = REG_PSTH_MBS; 
				databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high) & 0xFF00) >> 8);
				mutex_lock(&rpr400_mutex);
				res = i2c_master_send(client, databuf, 0x2);
				mutex_unlock(&rpr400_mutex);
				if(res <= 0)
				{
					APS_LOG("rpr400_enable_ps step 6 ret=%d", res);
					goto EXIT_ERR;
					return RPR400_ERR_I2C;
				}
			}
			else
			{
				databuf[0] = REG_PSTL_LSB;	
				databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) & 0x00FF);
				mutex_lock(&rpr400_mutex);
				res = i2c_master_send(client, databuf, 0x2);
				mutex_unlock(&rpr400_mutex);
				if(res <= 0)
				{
					APS_LOG("rpr400_enable_ps step 7 ret=%d", res);
					goto EXIT_ERR;
					return RPR400_ERR_I2C;
				}
				databuf[0] = REG_PSTL_MBS;	
				databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low) & 0xFF00) >> 8);
				mutex_lock(&rpr400_mutex);
				res = i2c_master_send(client, databuf, 0x2);
				mutex_unlock(&rpr400_mutex);
				if(res <= 0)
				{
					APS_LOG("rpr400_enable_ps step 8 ret=%d", res);
					goto EXIT_ERR;
					return RPR400_ERR_I2C;
				}
				databuf[0] = REG_PSTH_LSB;	
				databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) & 0x00FF);
				mutex_lock(&rpr400_mutex);
				res = i2c_master_send(client, databuf, 0x2);
				mutex_unlock(&rpr400_mutex);
				if(res <= 0)
				{
					APS_LOG("rpr400_enable_ps step 9 ret=%d", res);
					goto EXIT_ERR;
					return RPR400_ERR_I2C;
				}
				databuf[0] = REG_PSTH_MBS; 
				databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high) & 0xFF00) >> 8);
				mutex_lock(&rpr400_mutex);
				res = i2c_master_send(client, databuf, 0x2);
				mutex_unlock(&rpr400_mutex);
				if(res <= 0)
				{
					APS_LOG("rpr400_enable_ps step 10 ret=%d", res);
					goto EXIT_ERR;
					return RPR400_ERR_I2C;
				}
		
			}
		
			mutex_lock(&rpr400_mutex);
	                buffer[0] = REG_INTERRUPT;
                        client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	                res = i2c_master_send(client, buffer, (1<<8) | 1);
                        client->addr = client->addr& I2C_MASK_FLAG;
	                mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				APS_LOG("rpr400_enable_ps step 11 ret=%d", res);
				goto EXIT_ERR;
				return RPR400_ERR_I2C;
			}
			
			databuf[0] = REG_INTERRUPT;
			databuf[1] = buffer[0]|MODE_PROXIMITY;
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				APS_LOG("rpr400_enable_ps step 12 ret=%d", res);
				goto EXIT_ERR;
				return RPR400_ERR_I2C;
			}
			/*debug code for reading register value*/
			mt_eint_unmask(CUST_EINT_ALS_NUM);
		}
	}
	else
	{
		/*for interrup work mode support,mask intr first -- by liaoxl.lenovo 12.08.2011*/
		cancel_work_sync(&obj->eint_work);
		mt_eint_mask(CUST_EINT_ALS_NUM);

	/*yucong MTK: enable_ps function modified for fixing reading register error problem 2012.2.16*/
	        if (pwr_st.als_state == CTL_STANDALONE)
	        {
			power_set = PWRON_ONLY_ALS;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				APS_LOG("rpr400_enable_ps step 13 ret=%d", res);
				goto EXIT_ERR;
			}
	        }
	        else if (pwr_st.als_state == CTL_STANDBY)
	        {
			power_set = PWRON_STANDBY;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				APS_LOG("rpr400_enable_ps step 14 ret=%d", res);
				goto EXIT_ERR;
			}			
		 }

	}
#endif
	return 0;
	
EXIT_ERR:
	APS_ERR("rpr400_enable_ps fail\n");
	return -1; //return correct value when iic error -- by liaoxl.lenovo 3.15.2012
}

static long rpr400_enable_ps_for_cali(struct i2c_client *client, int enable)
{
	struct rpr400_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
	u8 buffer[1];
	u8 reg_value[1];
	u8 power_state, power_set;
	PWR_ST  pwr_st;	

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	client->addr = 0x39;
        mutex_lock(&rpr400_mutex);
	buffer[0] = REG_MODECONTROL;
        client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client, buffer, (1<<8) | 1);
        client->addr = client->addr& I2C_MASK_FLAG;
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
        power_state = buffer[0] & 0xF;
        if (MCTL_TABLE[power_state].ALS == 0) 
	{
            	pwr_st.als_state = CTL_STANDBY;
        } 
	else 
        {
            	pwr_st.als_state = CTL_STANDALONE;
        }
#if 1	
	/*yucong MTK: enable_ps function modified for fixing reading register error problem 2012.2.16*/
	if(enable)
	{
		/* tune led plus count for costdown chip -- by liaoxl.lenovo 2.12.2012 start */
		if (pwr_st.als_state == CTL_STANDALONE)
		{
			power_set = PWRON_PS_ALS;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
		else if (pwr_st.als_state == CTL_STANDBY)
		{
			power_set = PWRON_ONLY_PS;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
	}
	else
	{
	/*yucong MTK: enable_ps function modified for fixing reading register error problem 2012.2.16*/
	        if (pwr_st.als_state == CTL_STANDALONE)
	        {
			power_set = PWRON_ONLY_ALS;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
	        }
	        else if (pwr_st.als_state == CTL_STANDBY)
	        {
			power_set = PWRON_STANDBY;
			databuf[0] = REG_MODECONTROL;	
			databuf[1] = power_set | (reg_value[0] & CLR_LOW4BIT);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}			
		 }
	}
#endif
	return 0;
	
EXIT_ERR:
	APS_ERR("rpr400_enable_ps fail\n");
	return -1; //return correct value when iic error -- by liaoxl.lenovo 3.15.2012
}
/*----------------------------------------------------------------------------*/
#if 1
static int rpr400_enable(struct i2c_client *client, int enable)
{
	struct rpr400_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
	u8 buffer[1];
	u8 reg_value[1];

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

        client->addr = 0x39;
	if(enable)
	{
	
	}
	else
	{
		databuf[0] = REG_MODECONTROL;	
		databuf[1] = PWRON_STANDBY;
		mutex_lock(&rpr400_mutex);
		res = i2c_master_send(client, databuf, 0x2);
		mutex_unlock(&rpr400_mutex);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
	}
	return 0;
	
EXIT_ERR:
	APS_ERR("rpr400_enable fail\n");
	return -1;
}
#endif

/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
static int rpr400_check_and_clear_intr(struct i2c_client *client) 
{
	//struct rpr400_priv *obj = i2c_get_clientdata(client);
	u8 buffer[1];
        int res = 0;

        client->addr = 0x39;
	buffer[0] = REG_INTERRUPT;
	mutex_lock(&rpr400_mutex);
	client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client,buffer, (1<<8) | 1);
	client->addr = client->addr& I2C_MASK_FLAG;
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	

	return buffer[0];

EXIT_ERR:
	APS_ERR("rpr400_check_and_clear_intr fail\n");
	return -1;
}


/*-----------------------------------------------------------------------------*/
void rpr400_eint_func(void)
{
	struct rpr400_priv *obj = g_rpr400_ptr;
	if(!obj)
	{
		return;
	}
	
	schedule_work(&obj->eint_work);
}

/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
int rpr400_setup_eint(struct i2c_client *client)
{
	struct rpr400_priv *obj = i2c_get_clientdata(client);        

	g_rpr400_ptr = obj;
	
	mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
	mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, TRUE);
	mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);

	mt_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_TYPE, rpr400_eint_func, 0);

	mt_eint_unmask(CUST_EINT_ALS_NUM);  
    return 0;
}

/*----------------------------------------------------------------------------*/

#if 1
static int rpr400_init_client_for_cali(struct i2c_client *client, u8 pulse)
{

	struct rpr400_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
        u8 buffer[1];
	u8 ps_id_value[1];

        client->addr = 0x39;
	databuf[0] = REG_SYSTEMCONTROL;    
	databuf[1] = REG_SW_RESET | REG_INT_RESET;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}
	
	databuf[0] = REG_MODECONTROL;    
	databuf[1] = PS_ALS_SET_MODE_CONTROL|PWRON_ONLY_ALS;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}

	databuf[0] = REG_ALSPSCONTROL;    
	databuf[1] = PS_ALS_SET_ALSPS_CONTROL;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}

	databuf[0] = REG_ALSDATA0TH_LSB;    
	databuf[1] = PS_ALS_SET_ALS_TH & 0x00FF ;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}

       /*Lenovo-sw chenlj2 add 2011-06-03,modified pulse 2  to 4 */
	databuf[0] = REG_ALSDATA0TH_MBS;    
	databuf[1] = (PS_ALS_SET_ALS_TH& 0xFF00) >> 8;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}

	return RPR400_SUCCESS;

EXIT_ERR:
	APS_ERR("init dev: %d\n", res);
	return -1; //return correct value when iic error -- by liaoxl.lenovo 3.15.2012

}
#endif

static int rpr400_init_client(struct i2c_client *client)
{
	struct rpr400_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
        u8 buffer[1];
	u8 ps_id_value[1];

        client->addr = 0x39;
	databuf[0] = REG_SYSTEMCONTROL;    
	databuf[1] = REG_SW_RESET | REG_INT_RESET;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}
	
	databuf[0] = REG_MODECONTROL;    
	databuf[1] = PS_ALS_SET_MODE_CONTROL|PWRON_ONLY_ALS;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}

	databuf[0] = REG_ALSPSCONTROL;    
	databuf[1] = PS_ALS_SET_ALSPS_CONTROL;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}

	databuf[0] = REG_PERSISTENCE;    
	databuf[1] = PS_ALS_SET_INTR_PERSIST;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if(0 == obj->hw->polling_mode_ps)
	{
		if(1 == ps_cali.valid)
		{
			databuf[0] = REG_PSTL_LSB;	
			databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) & 0x00FF);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return RPR400_ERR_I2C;
			}
			databuf[0] = REG_PSTL_MBS;	
			databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low) & 0xFF00) >> 8);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return RPR400_ERR_I2C;
			}
			databuf[0] = REG_PSTH_LSB;	
			databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) & 0x00FF);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return RPR400_ERR_I2C;
			}
			databuf[0] = REG_PSTH_MBS;	
			databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high) & 0xFF00) >> 8);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return RPR400_ERR_I2C;
			}
		}
		else
		{
			databuf[0] = REG_PSTL_LSB;	
			databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) & 0x00FF);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return RPR400_ERR_I2C;
			}
			databuf[0] = REG_PSTL_MBS;	
			databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low) & 0xFF00) >> 8);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return RPR400_ERR_I2C;
			}
			databuf[0] = REG_PSTH_LSB;	
			databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) & 0x00FF);
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return RPR400_ERR_I2C;
			}
			databuf[0] = REG_PSTH_MBS;	
			databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high) & 0xFF00) >> 8);;
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return RPR400_ERR_I2C;
			}

		}

		databuf[0] = REG_INTERRUPT;
		databuf[1] = PS_ALS_SET_INTR | MODE_PROXIMITY;  /* low-pass filter of PS --- by liaoxl.lenovo 2.16.2012 */
		mutex_lock(&rpr400_mutex);
		res = i2c_master_send(client, databuf, 0x2);
		mutex_unlock(&rpr400_mutex);
		if(res <= 0)
		{
			goto EXIT_ERR;
			return RPR400_ERR_I2C;
		}
	}

	databuf[0] = REG_ALSDATA0TH_LSB;    
	databuf[1] = PS_ALS_SET_ALS_TH & 0x00FF ;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}

       /*Lenovo-sw chenlj2 add 2011-06-03,modified pulse 2  to 4 */
	databuf[0] = REG_ALSDATA0TH_MBS;    
	databuf[1] = (PS_ALS_SET_ALS_TH& 0xFF00) >> 8;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}

        /*Lenovo-sw chenlj2 add 2011-06-03,modified gain 16  to 1 */
	mutex_lock(&rpr400_mutex);
	buffer[0] = REG_SYSTEMCONTROL;
        client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client, buffer, (1<<8) | 1);
        client->addr = client->addr& I2C_MASK_FLAG;
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}
	if(buffer[0]  != 0x09)
	{
		return RPR400_CHECKID_FAIL;
	}
	
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if((res = rpr400_setup_eint(client))!=0)
	{
		APS_ERR("setup eint: %d\n", res);
		return res;
	}
	databuf[0] = REG_MODECONTROL;    
	databuf[1] = 0x00;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return RPR400_ERR_I2C;
	}
	#if 0
	if((res = rpr400_check_and_clear_intr(client)))
	{
		APS_ERR("check/clear intr: %d\n", res);
		//    return res;
	}
	#endif
	return RPR400_SUCCESS;

EXIT_ERR:
	APS_ERR("init dev: %d\n", res);
	return -1; //return correct value when iic error -- by liaoxl.lenovo 3.15.2012
}
/******************************************************************************
 * NAME       : long_long_divider
 * FUNCTION   : calc divider of unsigned long long int or unsgined long
 * REMARKS    :
 *****************************************************************************/
static int long_long_divider(long long data, unsigned long base_divier, unsigned long *answer, unsigned long long *overplus)
{
    volatile long long divier;
    volatile long      unit_sft;

    if ((data < 0) || (base_divier == 0)) {
        *answer   = 0;
        *overplus = 0;
        return (CALC_ERROR);
    }

    divier = base_divier;
    if (data > MASK_LONG) {
        unit_sft = 0;
        while ((data > divier) && (divier > 0)) {
            unit_sft++;
            divier = divier << 1;
        }
        while ((data > base_divier) && (unit_sft > 0)) {
            if (data > divier) {
                *answer += 1 << unit_sft;
                data    -= divier;
            }
            unit_sft--;
            divier = divier >> 1;
        }
        *overplus = data;
    } else {
        *answer = (unsigned long)(data & MASK_LONG) / base_divier;
        /* calculate over plus and shift 16bit */
        *overplus = (unsigned long long)(data - (*answer * base_divier));
    }

    return (0);
}

/******************************************************************************
 * NAME       : calc_rohm_als_data
 * FUNCTION   : calculate illuminance data for RPR400
 * REMARKS    : final_data is 1000 times, which is defined as CUT_UNIT, of the actual lux value
 *****************************************************************************/
static int calc_rohm_als_data(READ_DATA_BUF data, DEVICE_VAL dev_val)
{
#define DECIMAL_BIT      (15)
#define JUDGE_FIXED_COEF (100)
#define MAX_OUTRANGE     (11357)
#define MAXRANGE_NMODE   (0xFFFF)
#define MAXSET_CASE      (4)
#define CUT_UNIT          20

	int                result,final_data, mid_data;
	CALC_DATA          calc_data;
	CALC_ANS           calc_ans;
	unsigned long      calc_judge;
	unsigned char      set_case;
	unsigned long      div_answer;
	unsigned long long div_overplus;
	unsigned long long overplus;
	unsigned long      max_range;

	/* set the value of measured als data */
	calc_data.als_data0  = data.als_data0;
	calc_data.als_data1  = data.als_data1;
	calc_data.gain_data0 = GAIN_TABLE[dev_val.gain].DATA0;

	/* set max range */
	if (calc_data.gain_data0 == 0) 
	{
		/* issue error value when gain is 0 */
		return (CALC_ERROR);
	}
	else
	{
		max_range = MAX_OUTRANGE / calc_data.gain_data0;
	}
	
	/* calculate data */
	if (calc_data.als_data0 == MAXRANGE_NMODE) 
	{
		calc_ans.positive = max_range;
		calc_ans.decimal  = 0;
	} 
	else 
	{
		/* get the value which is measured from power table */
		calc_data.als_time = MCTL_TABLE[dev_val.time].ALS;
		if (calc_data.als_time == 0) 
		{
			/* issue error value when time is 0 */
			return (CALC_ERROR);
		}

		calc_judge = calc_data.als_data1 * JUDGE_FIXED_COEF;
		if (calc_judge < (calc_data.als_data0 * judge_coefficient[0])) 
		{
			set_case = 0;
		} 
		else if (calc_judge < (data.als_data0 * judge_coefficient[1]))
		{
			set_case = 1;
		} 
		else if (calc_judge < (data.als_data0 * judge_coefficient[2])) 
		{
			set_case = 2;
		}
		else if (calc_judge < (data.als_data0 * judge_coefficient[3])) 
		{
			 set_case = 3;
		} 
		else
		{
			set_case = MAXSET_CASE;
		}
		calc_ans.positive = 0;
		if (set_case >= MAXSET_CASE) 
		{
			calc_ans.decimal = 0;	//which means that lux output is 0
		}
		else
		{
			calc_data.gain_data1 = GAIN_TABLE[dev_val.gain].DATA1;
			if (calc_data.gain_data1 == 0) 
			{
				/* issue error value when gain is 0 */
				return (CALC_ERROR);
			}
                calc_data.data0      = (long long )(data0_coefficient[set_case] * calc_data.als_data0) * calc_data.gain_data1;
                calc_data.data1      = (long long )(data1_coefficient[set_case] * calc_data.als_data1) * calc_data.gain_data0;
			if (calc_data.data0 < calc_data.data1) 
			{
				/* issue error value when data is negtive */
				return (CALC_ERROR);
			}
			calc_data.data       = (calc_data.data0 - calc_data.data1);
			calc_data.dev_unit   = calc_data.gain_data0 * calc_data.gain_data1 * calc_data.als_time * 10;
			if (calc_data.dev_unit == 0) 
			{
				/* issue error value when dev_unit is 0 */
				return (CALC_ERROR);
			}

			/* calculate a positive number */
			div_answer   = 0;
			div_overplus = 0;
                result = long_long_divider(calc_data.data, calc_data.dev_unit, &div_answer, &div_overplus);
                if (result == CALC_ERROR) {
                    return (result);
                }
			calc_ans.positive = div_answer;
			/* calculate a decimal number */
			calc_ans.decimal = 0;
			overplus         = div_overplus;
			if (calc_ans.positive < max_range)
			{
				if (overplus != 0)
				{
					overplus     = overplus << DECIMAL_BIT;
					div_answer   = 0;
					div_overplus = 0;
					long_long_divider(overplus, calc_data.dev_unit, &div_answer, &div_overplus);
					calc_ans.decimal = div_answer;
				}
			}
			else
			{
				calc_ans.positive = max_range;
			}
		}
	}
	
	mid_data = (calc_ans.positive << DECIMAL_BIT) + calc_ans.decimal;
	final_data = calc_ans.positive * CUT_UNIT + ((calc_ans.decimal * CUT_UNIT) >> DECIMAL_BIT);
					
	return (final_data);

#undef CUT_UNIT
#undef DECIMAL_BIT
#undef JUDGE_FIXED_COEF
#undef MAX_OUTRANGE
#undef MAXRANGE_NMODE
#undef MAXSET_CASE
}

/************************************************************
 *                      logic function                      *
 ***********************************************************/
/******************************************************************************
 * NAME       : get_from_device
 * FUNCTION   : periodically reads the data from sensor(thread of work)
 * REMARKS    :
 *****************************************************************************/
static int get_from_device(DEVICE_VAL *dev_val, struct i2c_client *client)
{
#define LEDBIT_MASK   (3)
#define GAIN_VAL_MASK (0xF)
#if 0
	struct rpr400_priv *obj = i2c_get_clientdata(client);	 
	u8 buffer[1];
	int res = 0;
    	unsigned char alsps_ctl[1], read_time[1];

   	 /* initalize the returning value */
    	dev_val->time        = 6;
    	dev_val->gain        = (PS_ALS_SET_ALSPS_CONTROL >> 2) & GAIN_VAL_MASK;
    	dev_val->led_current = PS_ALS_SET_ALSPS_CONTROL & LEDBIT_MASK;


	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	mutex_lock(&rpr400_mutex);
	buffer[0]=REG_MODECONTROL;
        client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client, buffer, (1<<8) | 1);
        client->addr = client->addr& I2C_MASK_FLAG;
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	dev_val->time = buffer[0] & 0xF;

	mutex_lock(&rpr400_mutex);
	buffer[0]=REG_ALSPSCONTROL;
        client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client, buffer, (1<<8) | 1);
        client->addr = client->addr& I2C_MASK_FLAG;
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

    	dev_val->led_current = buffer[0] & LEDBIT_MASK;
    	dev_val->gain        = (buffer[0] >> 2) & GAIN_VAL_MASK;
#else
    	dev_val->time        = 6;
    	dev_val->gain        = (PS_ALS_SET_ALSPS_CONTROL >> 2) & GAIN_VAL_MASK;
    	dev_val->led_current = PS_ALS_SET_ALSPS_CONTROL & LEDBIT_MASK;
#endif
    return (0);
		
//EXIT_ERR:
//	APS_ERR("rpr400_read_ps fail\n");
//	return res;

#undef LEDBIT_MASK
#undef GAIN_VAL_MASK
}

/****************************************************************************** 
 * Function Configuration
******************************************************************************/
int rpr400_read_als(struct i2c_client *client, u16 *data)
{
	struct rpr400_priv *obj = i2c_get_clientdata(client);	 
	u8 buffer[2];
	u16 prev_als_value = *data;
	int res = 0;
	READ_DATA_BUF   als_data;
	DEVICE_VAL  dev_val;

	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	/*debug tag for yucong*/
//get adc channel 0 value
       client->addr = 0x39;
	buffer[0] = REG_ALSDATA0_LSB;
	mutex_lock(&rpr400_mutex);
        client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client, buffer, (2<<8) | 1);
        client->addr = client->addr& I2C_MASK_FLAG;
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
        als_data.als_data0 = buffer[0] | (buffer[1]<<8);
	//APS_DBG("c0_value=%d, c0_nf=%d, als_modulus=%d\n", c0_value, c0_nf, obj->als_modulus);

//get adc channel 1 value
	buffer[0] = REG_ALSDATA1_LSB;
	mutex_lock(&rpr400_mutex);
        client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client, buffer, (2<<8) | 1);
        client->addr = client->addr& I2C_MASK_FLAG;
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
        als_data.als_data1 = buffer[0] | (buffer[1]<<8);	
	//APS_DBG("c1_value=%d, c1_nf=%d, als_modulus=%d\n", c1_value, c1_nf, obj->als_modulus);
        get_from_device(&dev_val, client);

	*data = calc_rohm_als_data(als_data, dev_val);
		if(*data == 0)
		*data ++;
	if(*data == CALC_ERROR)
		*data = prev_als_value;	//Report same value as previous.

	return 0;	 

	
	
EXIT_ERR:
	APS_ERR("rpr400_read_als fail\n");
	return -1; //return correct value when iic error -- by liaoxl.lenovo 3.15.2012
}
/*----------------------------------------------------------------------------*/

static int rpr400_get_als_value(struct rpr400_priv *obj, u16 als)
{
	int idx;
	int invalid = 0;
	for(idx = 0; idx < obj->als_level_num; idx++)
	{
		if(als <= obj->hw->als_level[idx])
		{
			break;
		}
	}
	
	if(idx >= obj->als_value_num)
	{
		APS_ERR("exceed range\n"); 
		idx = obj->als_value_num - 1;
	}
	
	if(1 == atomic_read(&obj->als_deb_on))
	{
		unsigned long endt = atomic_read(&obj->als_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->als_deb_on, 0);
		}
		
		if(1 == atomic_read(&obj->als_deb_on))
		{
			invalid = 1;
		}
	}

	if(!invalid)
	{
		//APS_DBG("ALS: %05d => %05d\n", als, obj->hw->als_value[idx]);	
		return obj->hw->als_value[idx];
	}
	else
	{
		//APS_ERR("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);    
		return -1;
	}
}
/*----------------------------------------------------------------------------*/
/*  lenovo esd check -- liaoxl.lenovo 1.13.2013 start */
#define I2C_FLAG_WRITE	0
#define I2C_FLAG_READ	1


int chip_i2c_master_operate(struct i2c_client *client, const char *buf, int count, int i2c_flag)
{
	int res = 0;

        client->addr = 0x39;
	mutex_lock(&rpr400_mutex);
	switch(i2c_flag)
	{	
		case I2C_FLAG_WRITE:
		client->addr &=I2C_MASK_FLAG;
		res = i2c_master_send(client, buf, count);
		client->addr &=I2C_MASK_FLAG;
		break;
		
		case I2C_FLAG_READ:
		client->addr &=I2C_MASK_FLAG;
		client->addr |=I2C_WR_FLAG;
		client->addr |=I2C_RS_FLAG;
		res = i2c_master_send(client, buf, count);
		client->addr &=I2C_MASK_FLAG;
		break;

		default:
		APS_LOG("chip_i2c_master_operate i2c_flag command not support!\n");
		break;
	}
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	mutex_unlock(&rpr400_mutex);
	return res;
	EXIT_ERR:
	mutex_unlock(&rpr400_mutex);
	APS_ERR("chip_i2c_master_operate fail\n");
	return res;
}

static int chip_check_regs(struct i2c_client *client)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);	 
	u8 buffer[2];
	int res = 0;
	int i;

    APS_LOG("chip_check_regs start\n");
	client->addr = 0x39;
	buffer[0] = REG_ALSPSCONTROL;
	res = chip_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		return -EIO;
	}
	if(buffer[0] != PS_ALS_SET_ALSPS_CONTROL)
	{
		return -EINVAL;
	}
    APS_LOG("chip_check_regs start 1\n");
	buffer[0] = REG_PERSISTENCE;
	res = chip_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		return -EIO;
	}
	if(buffer[0] != PS_ALS_SET_INTR_PERSIST)
	{
	   return -EINVAL;
	}
	
    APS_LOG("chip_check_regs start 2\n");
	buffer[0] = REG_INTERRUPT;
	res = chip_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		return -EIO;
	}

	if((buffer[0]&0x3F) != (PS_ALS_SET_INTR | MODE_PROXIMITY))
	{
		return -EINVAL;
	}
    APS_LOG("chip_check_regs end\n");

	return 0;
}


static void chip_esd_check(struct i2c_client *client)
{
	int err;
	struct rpr400_priv *obj = i2c_get_clientdata(client);

	err = chip_check_regs(obj->client);
	APS_LOG("chip_esd_check err =%d\n",err);
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
		err = rpr400_init_client(obj->client);
		if(err != 0)
		{
			APS_ERR("initialize client fail!!\n");
		}
		else
		{
			atomic_set(&obj->als_suspend, 0);
			if(test_bit(CMC_BIT_ALS, &obj->enable))
			{
				err = rpr400_enable_als(obj->client, 1);
				if(err != 0)
				{
				APS_ERR("enable als fail: %d\n", err);
				}
			}
			atomic_set(&obj->ps_suspend, 0);
			if(test_bit(CMC_BIT_PS, &obj->enable))
			{
				err = rpr400_enable_ps(obj->client, 1);
				if(err != 0)
				{
				   APS_ERR("enable ps fail: %d\n", err);
				}
			}
		}
	}
}
/*  lenovo esd check -- liaoxl.lenovo 1.13.2013 end */

long rpr400_read_ps(struct i2c_client *client, u16 *data)
{
	struct rpr400_priv *obj = i2c_get_clientdata(client);    
	//u16 ps_value;      
	u8 buffer[2];
	int res = 0;

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}
	/*  lenovo esd check -- liaoxl.lenovo 1.13.2013 start */
	chip_esd_check(client);
	/*  lenovo esd check -- liaoxl.lenovo 1.13.2013 end */

        client->addr = 0x39;
	buffer[0] = REG_PSDATA_LSB;
	mutex_lock(&rpr400_mutex);
        client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client, buffer, (2<<8) | 1);
        client->addr = client->addr& I2C_MASK_FLAG;
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
        *data = buffer[0] | (buffer[1]<<8);
	//APS_DBG("ps_data=%d, low:%d  high:%d", *data, ps_value_low[0], ps_value_high[0]);
	return 0;    

EXIT_ERR:
	APS_ERR("rpr400_read_ps fail\n");
	return -1; //return correct value when iic error -- by liaoxl.lenovo 3.15.2012
}
/*----------------------------------------------------------------------------*/
static int rpr400_get_ps_value(struct rpr400_priv *obj, u16 ps)
{
	int val, mask = atomic_read(&obj->ps_mask);
	int invalid = 0;
	static int val_temp = -1;


	if(ps_cali.valid == 1)
	{
		//APS_LOG("rpr400_get_ps_value val_temp  = %d",val_temp);
		if(ps >ps_cali.close)
		{
			val = 0;  /*close*/
			val_temp = 0;
		}
		else if(ps <ps_cali.far_away)
		{
			val = 1;  /*far away*/
			val_temp = 1;
		}
		else
		        val = val_temp;

			//APS_LOG("rpr400_get_ps_value val  = %d",val);
	}
	else
	{
		if(ps > PS_ALS_SET_PS_TH)
		{
			val = 0;  /*close*/
			val_temp = 0;
		}
		else if(ps < PS_ALS_SET_PS_TL)
		{
			val = 1;  /*far away*/
			val_temp = 1;
		}
		else
		       val = val_temp;	
			
	}
	
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


/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
static void rpr400_ps_change_thd(struct i2c_client *client, int ps)
{
        struct rpr400_priv *obj = i2c_get_clientdata(client);
	u8 lowt[2],hight[2],databuf[2];
	int chg, res;

    client->addr = 0x39;
	switch(ps)
	{
		case 0:

			lowt[0] = (u8)(atomic_read(&obj->ps_thd_val_low) & 0x00FF);
			lowt[1] = (u8)((atomic_read(&obj->ps_thd_val_low) & 0xFF00) >> 8);
			hight[0] = 0xFF;
			hight[1] = 0xFF;
				
			chg = 1;
			break;

		case 1:

			lowt[0] = 0;
			lowt[1] = 0;
			hight[0] = (u8)(atomic_read(&obj->ps_thd_val_high) & 0x00FF);
			hight[1] = (u8)((atomic_read(&obj->ps_thd_val_high) & 0xFF00) >> 8);
			chg = 1;
			break;

		default:
			chg = 0;
			break;
	}

	if(0 != chg)
	{
			databuf[0] = REG_PSTL_LSB;	
			databuf[1] = lowt[0];
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				return;
			}
			databuf[0] = REG_PSTL_MBS;	
			databuf[1] = lowt[1];
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				return;
			}
			databuf[0] = REG_PSTH_LSB;	
			databuf[1] = hight[0];
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				return;
			}
			databuf[0] = REG_PSTH_MBS; 
			databuf[1] = hight[1];
			mutex_lock(&rpr400_mutex);
			res = i2c_master_send(client, databuf, 0x2);
			mutex_unlock(&rpr400_mutex);
			if(res <= 0)
			{
				return;
			}
	}
}

static void rpr400_eint_work(struct work_struct *work)
{
	struct rpr400_priv *obj = (struct rpr400_priv *)container_of(work, struct rpr400_priv, eint_work);
	int err=-1;
	int ints,temp;
	hwm_sensor_data sensor_data;

	ints = rpr400_check_and_clear_intr(obj->client);
	if(ints < 0)
	{
		APS_ERR("rpr400_eint_work check intrs: %d\n", ints);
	}
	else
	{
		if(ints & PS_INT_MASK)
		{		
			err=rpr400_read_ps(obj->client, &obj->ps);
		}
		if(ints & ALS_INT_MASK) // 2 kinds of interrupt may occur at same time
		{
				
			err=rpr400_read_ps(obj->client, &obj->ps);
				
		}
		if(!((ints & ALS_INT_MASK) || (ints & PS_INT_MASK)))
		{
			APS_DBG( "Unknown interrupt source.\n");
		}
		/* proximity */
		
		if(0 == err) 
		{

			APS_DBG("rpr400_eint_work rawdata ps=%d als_ch0=%d!\n",obj->ps,obj->als);
			/* to filter sensor rawdata noise -- liaoxl.lenovo 4.19.2012 start */
			temp = rpr400_get_ps_value(obj, obj->ps);
			//APS_LOG("rpr400_get_ps_value  ps =%d!\n", temp);
		        if(temp == 0)
			{
				sensor_data.values[0] = 0;
				sensor_data.value_divide = 1;
				sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM; 		
						
				//let up layer to know
				//APS_LOG("rpr400_eint_work	ps close!\n");
				if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
				{
				     APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
				}

				//rpr400_ps_change_thd(obj->client, 0);
			}
			else if(temp == 1)
			{
			        sensor_data.values[0] = 1;
				sensor_data.value_divide = 1;
				sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM; 		
						
				//let up layer to know
				//APS_LOG("rpr400_eint_work	ps far away!\n");
				if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
				{
					APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
				}

				//rpr400_ps_change_thd(obj->client, 1);
			}
		}
	}
	
	mt_eint_unmask(CUST_EINT_ALS_NUM);      
}


/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int rpr400_open(struct inode *inode, struct file *file)
{
	file->private_data = rpr400_i2c_client;

	if (!file->private_data)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}
	
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int rpr400_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

/*for ps cali work mode support -- by liaoxl.lenovo 2.08.2011 start*/
static void rpr400_WriteCalibration(HWMON_PS_STRUCT *data_cali)
{
    struct PS_CALI_DATA_STRUCT *ps_data_cali;
	APS_LOG("rpr400_WriteCalibration  1 %d," ,data_cali->close);
	APS_LOG("rpr400_WriteCalibration  2 %d," ,data_cali->far_away);
	APS_LOG("rpr400_WriteCalibration  3 %d,", data_cali->valid);
	//APS_LOG("rpr400_WriteCalibration  4 %d,", data_cali->pulse);

	if(data_cali->valid == 1)
	{
		ps_data_cali = &ps_cali;
		ps_data_cali->valid = 1;
		ps_data_cali->close = data_cali->close;
		ps_data_cali->far_away = data_cali->far_away;
	}

}
#define THRES_TOLERANCE		9	//I think this is a proper value. It should not be too big.
#define THRES_DEFAULT_DIFF	5 //15 //35
#define REG_PSTH_MAX	0xFFF
#define REG_PSTL_MAX	0xFFF

static int rpr400_ps_average_val = 0; 
static int rpr400_read_data_for_cali(struct i2c_client *client, HWMON_PS_STRUCT *ps_data_cali)
{
	struct rpr400_priv *obj = i2c_get_clientdata(client);
        u8 databuf[2];
        u8 buffer[2];  
	int i=0 ,res = 0;
	u16 data[32],sum=0,data_cali=0;

	ps_data_cali->valid = 0;
	client->addr = 0x39;
	databuf[0] = REG_MODECONTROL;    
	databuf[1] = 0x01;
	mutex_lock(&rpr400_mutex);
	res = i2c_master_send(client, databuf, 0x2);
	mutex_unlock(&rpr400_mutex);
	if(res <= 0)
	{
		return -1;
	}
	for(i = 0; i < 20; i++)
	{
		mdelay(5);//50
		res = rpr400_read_ps(client,&data[i]);
		if(res != 0)
		{
			APS_ERR("rpr400_read_data_for_cali fail: %d\n", i); 
			break;
		}
		else
		{
			APS_LOG("[%d]sample = %d\n", i, data[i]);
			sum += data[i];
		}
	 }
	 
				
	if(i < 20)
	{
		res=  -1;

		return res;
	}
	else
	{
		data_cali = sum / 20;
		rpr400_ps_average_val = data_cali;
		APS_LOG("rpr400_read_data_for_cali data = %d",data_cali);
		 if( data_cali>600)
		{
			APS_ERR("rpr400_read_data_for_cali fail value to high: %d\n", data_cali);
			return -2;
		}	

		ps_data_cali->close =data_cali + THRES_TOLERANCE + THRES_DEFAULT_DIFF;
		ps_data_cali->far_away =data_cali + THRES_TOLERANCE;
		
		ps_data_cali->valid = 1;
		atomic_set(&rpr400_obj->ps_thd_val_high, ps_data_cali->close);
		atomic_set(&rpr400_obj->ps_thd_val_low, ps_data_cali->far_away);
		APS_LOG("rpr400_read_data_for_cali close = %d,far_away = %d,valid = %d\n",ps_data_cali->close,ps_data_cali->far_away,ps_data_cali->valid);
		//APS_LOG("rpr400_read_data_for_cali avg=%d max=%d min=%d\n",data_cali, max, min);
	}

	return res;
}

/*for ps cali work mode support -- by liaoxl.lenovo 2.08.2011 end*/

/*----------------------------------------------------------------------------*/
static long rpr400_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct rpr400_priv *obj = i2c_get_clientdata(client);  
	long err = 0;
	void __user *ptr = (void __user*) arg;
	int dat;
	uint32_t enable;
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
				if(err = rpr400_enable_ps_for_cali(obj->client, 1))
				{
					APS_ERR("enable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				set_bit(CMC_BIT_PS, &obj->enable);
			}
			else
			{
				if(err = rpr400_enable_ps_for_cali(obj->client, 0))
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
			if(err = rpr400_read_ps(obj->client, &obj->ps))
			{
				goto err_out;
			}
			
			dat = rpr400_get_ps_value(obj, obj->ps);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;

		case ALSPS_GET_PS_RAW_DATA:    
			if(err = rpr400_read_ps(obj->client, &obj->ps))
			{
				goto err_out;
			}
			APS_DBG("IOCTL PS rawdata = %d\n", obj->ps);
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
				if(err = rpr400_enable_als(obj->client, 1))
				{
					APS_ERR("enable als fail: %d\n", err); 
					goto err_out;
				}
				set_bit(CMC_BIT_ALS, &obj->enable);
			}
			else
			{
				if(err = rpr400_enable_als(obj->client, 0))
				{
					APS_ERR("disable als fail: %d\n", err); 
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
			if(err = rpr400_read_als(obj->client, &obj->als))
			{
				goto err_out;
			}

			dat = rpr400_get_als_value(obj, obj->als);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

		case ALSPS_GET_ALS_RAW_DATA:    
			if(err = rpr400_read_als(obj->client, &obj->als))
			{
				goto err_out;
			}

			dat = obj->als;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

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
			rpr400_WriteCalibration(&ps_cali_temp);
			APS_LOG(" ALSPS_SET_PS_CALI %d,%d,%d\t",ps_cali_temp.close,ps_cali_temp.far_away,ps_cali_temp.valid);
			break;

		/*for ps cali work mode support -- by liaoxl.lenovo 2.08.2011 start*/
		case ALSPS_GET_PS_RAW_DATA_FOR_CALI:
		{
			
				cancel_work_sync(&obj->eint_work);
				mt_eint_mask(CUST_EINT_ALS_NUM);

				rpr400_init_client_for_cali(obj->client, 0);
				msleep(200);
				err = rpr400_read_data_for_cali(obj->client,&ps_cali_temp);
				if(err < 0 ){
					goto err_out;
				}	

				
				rpr400_WriteCalibration(&ps_cali_temp);

				rpr400_enable(obj->client, 0);
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
			enable = rpr400_ps_average_val;	
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
static struct file_operations rpr400_fops = {
	.owner = THIS_MODULE,
	.open = rpr400_open,
	.release = rpr400_release,
	.unlocked_ioctl = rpr400_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice rpr400_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &rpr400_fops,
};
/*----------------------------------------------------------------------------*/
static int rpr400_i2c_suspend(struct i2c_client *client, pm_message_t msg) 
{
	//struct rpr400_priv *obj = i2c_get_clientdata(client);    
	//int err;
	APS_FUN();    

	return 0;
}
/*----------------------------------------------------------------------------*/
static int rpr400_i2c_resume(struct i2c_client *client)
{
	//struct rpr400_priv *obj = i2c_get_clientdata(client);        
	//int err;
	APS_FUN();

	return 0;
}
/*----------------------------------------------------------------------------*/
static void rpr400_early_suspend(struct early_suspend *h) 
{   /*early_suspend is only applied for ALS*/
	struct rpr400_priv *obj = container_of(h, struct rpr400_priv, early_drv);   
	int err;
	APS_FUN();    

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

	/* mask this to filter strong sun light generating a high value on lenovo board -- by liaoxl.lenovo 3.2.2012   */
	#if 1
	if(test_bit(CMC_BIT_PS, &obj->enable))
	{
	}
	else
	{
		atomic_set(&obj->als_suspend, 1);
		if(test_bit(CMC_BIT_ALS, &obj->enable))
		{
			if(err = rpr400_enable_als(obj->client, 0))
			{
				APS_ERR("disable als fail: %d\n", err); 
			}
		}
	}
	#endif
}
/*----------------------------------------------------------------------------*/
static void rpr400_late_resume(struct early_suspend *h)
{   /*early_suspend is only applied for ALS*/
	struct rpr400_priv *obj = container_of(h, struct rpr400_priv, early_drv);         
	int err;
	APS_FUN();

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

	/* mask this to filter strong sun light generating a high value on lenovo board -- by liaoxl.lenovo 3.2.2012   */
    #if 1
    if(0 != atomic_read(&obj->als_suspend))
    {
		atomic_set(&obj->als_suspend, 0);
		if(test_bit(CMC_BIT_ALS, &obj->enable))
		{
			if(err = rpr400_enable_als(obj->client, 1))
			{
				APS_ERR("enable als fail: %d\n", err);        

			}
		}
	}
	#endif
}

int rpr400_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct rpr400_priv *obj = (struct rpr400_priv *)self;
	
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
					if(err = rpr400_enable_ps(obj->client, 1))
					{
						APS_ERR("enable ps fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_PS, &obj->enable);
				}
				else
				{
					if(err = rpr400_enable_ps(obj->client, 0))
					{
						APS_ERR("disable ps fail: %d\nREG_PSTH_MBS", err); 
						return -1;
					}
					clear_bit(CMC_BIT_PS, &obj->enable);
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
				rpr400_read_ps(obj->client, &obj->ps);
				
				APS_ERR("rpr400_ps_operate als data=%d!\n",obj->als);
				sensor_data->values[0] = rpr400_get_ps_value(obj, obj->ps);
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

static int temp_als = 0;
int rpr400_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct rpr400_priv *obj = (struct rpr400_priv *)self;

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
					if(err = rpr400_enable_als(obj->client, 1))
					{
						APS_ERR("enable als fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_ALS, &obj->enable);
				}
				else
				{
					if(err = rpr400_enable_als(obj->client, 0))
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
				rpr400_read_als(obj->client, &obj->als);
				sensor_data->values[0] = rpr400_get_als_value(obj, obj->als);
				sensor_data->value_divide = 1;
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
/*static int rpr400_i2c_detect(struct i2c_client *client, struct i2c_board_info *info) 
{    
	strcpy(info->type, RPR400_DEV_NAME);
	return 0;

}*/

/*----------------------------------------------------------------------------*/
extern int alsps_device_index ;// 0 unknow 1 avago 2 rohm
static int rpr400_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct rpr400_priv *obj;
	struct hwmsen_object obj_ps, obj_als;
	int err = 0;

	APS_ERR("enter \n");
	
	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	memset(obj, 0, sizeof(*obj));
	rpr400_obj = obj;
	alsps_device_index = 2;

	obj->hw = get_cust_alsps_hw();
	rpr400_get_addr(obj->hw, &obj->addr);

	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	INIT_WORK(&obj->eint_work, rpr400_eint_work);
	obj->client = client;
	i2c_set_clientdata(client, obj);	
	atomic_set(&obj->als_debounce, 50);
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
	obj->enable = 0;
	obj->pending_intr = 0;
	obj->als_level_num = sizeof(obj->hw->als_level)/sizeof(obj->hw->als_level[0]);
	obj->als_value_num = sizeof(obj->hw->als_value)/sizeof(obj->hw->als_value[0]);  
	
	BUG_ON(sizeof(obj->als_level) != sizeof(obj->hw->als_level));
	memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
	BUG_ON(sizeof(obj->als_value) != sizeof(obj->hw->als_value));
	memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
	atomic_set(&obj->i2c_retry, 3);
	set_bit(CMC_BIT_ALS, &obj->enable);
	set_bit(CMC_BIT_PS, &obj->enable);
	rpr400_i2c_client = client;

	if(err = rpr400_init_client(client))
	{
		goto exit_init_failed;
	}
	APS_LOG("rpr400_init_client() OK!\n");

	if(err = misc_register(&rpr400_device))
	{
		APS_ERR("rpr400_device register failed\n");
		goto exit_misc_device_register_failed;
	}
/*
	if(err = rpr400_create_attr(&rpr400_alsps_driver.driver))
	{
		APS_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
*/
	obj_ps.self = rpr400_obj;
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if(1 == obj->hw->polling_mode_ps)
	//if (1)
	{
		obj_ps.polling = 1;
	}
	else
	{
		obj_ps.polling = 0;
	}

	obj_ps.sensor_operate = rpr400_ps_operate;
	if(err = hwmsen_attach(ID_PROXIMITY, &obj_ps))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}
	
	obj_als.self = rpr400_obj;
	obj_als.polling = 1;
	obj_als.sensor_operate = rpr400_als_operate;
	if(err = hwmsen_attach(ID_LIGHT, &obj_als))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}


#if defined(CONFIG_HAS_EARLYSUSPEND)
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 2,
	obj->early_drv.suspend  = rpr400_early_suspend,
	obj->early_drv.resume   = rpr400_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif

	APS_LOG("%s: OK\n", __func__);
       rpr400_init_flag = 0;
	return 0;

	exit_create_attr_failed:
	misc_deregister(&rpr400_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(client);
	//exit_kfree:
	kfree(obj);
	exit:
	rpr400_i2c_client = NULL;   
	rpr400_init_flag = -1;
	alsps_device_index = 0;
//	MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  /*mask interrupt if fail*/
	APS_ERR("%s: err = %d\n", __func__, err);
	return err;
}
/*----------------------------------------------------------------------------*/
static int rpr400_i2c_remove(struct i2c_client *client)
{
	int err;	
/*	
	if(err = rpr400_delete_attr(&rpr400_i2c_driver.driver))
	{
		APS_ERR("rpr400_delete_attr fail: %d\n", err);
	} 
*/
	if(err = misc_deregister(&rpr400_device))
	{
		APS_ERR("misc_deregister fail: %d\n", err);    
	}
	
	rpr400_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));

	return 0;
}
/*----------------------------------------------------------------------------*/
#if 0
static int rpr400_probe(struct platform_device *pdev) 
{
	struct alsps_hw *hw = get_cust_alsps_hw();

	APS_ERR("enter \n");
	
	rpr400_power(hw, 1);    
	//rpr400_force[0] = hw->i2c_num;
	//rpr400_force[1] = hw->i2c_addr[0];
	//APS_DBG("I2C = %d, addr =0x%x\n",rpr400_force[0],rpr400_force[1]);
	if(i2c_add_driver(&rpr400_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	} 
	return 0;
}
/*----------------------------------------------------------------------------*/
static int rpr400_remove(struct platform_device *pdev)
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();    
	rpr400_power(hw, 0);    
	i2c_del_driver(&rpr400_i2c_driver);
	return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver rpr400_alsps_driver = {
	.probe      = rpr400_probe,
	.remove     = rpr400_remove,    
	.driver     = {
		.name  = "als_ps",
//		.owner = THIS_MODULE,
	}
};
#endif
/*----------------------------------------------------------------------------*/
static int  rpr400_remove(void)
{
    //struct acc_hw *hw =  get_cust_alsps_hw();

    APS_FUN();    
 //  rpr400_power(hw, 0);    
    i2c_del_driver(&rpr400_i2c_driver);
    return 0;
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

static int rpr400_local_init(void)
{
  // struct acc_hw *hw = get_cust_alsps_hw();
	APS_FUN();

	//rpr400_power(hw, 1);
	if(i2c_add_driver(&rpr400_i2c_driver))
	{
		APS_ERR("chenlj2 rpr400 add driver error\n");
		return -1;
	}
	if(-1 == rpr400_init_flag)
	{
	   return -1;
	}
	
	return 0;
}
static int __init rpr400_init(void)
{
	APS_FUN();
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_LOG("%s: i2c_number=%d\n", __func__,hw->i2c_num); 
	i2c_register_board_info(hw->i2c_num, &i2c_RPR400, 1);
	hwmsen_alsps_sensor_add(&rpr400_init_info);
	#if 0
	if(platform_driver_register(&rpr400_alsps_driver))
	{
		APS_ERR("failed to register driver");
		return -ENODEV;
	}
	#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit rpr400_exit(void)
{
	APS_FUN();
	//platform_driver_unregister(&rpr400_alsps_driver);
}
/*----------------------------------------------------------------------------*/
module_init(rpr400_init);
module_exit(rpr400_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("Dexiang Liu");
MODULE_DESCRIPTION("rpr400 driver");
MODULE_LICENSE("GPL");

