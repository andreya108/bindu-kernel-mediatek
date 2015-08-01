/* KXTF9 motion sensor driver
 *
 *
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

#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#define POWER_NONE_MACRO MT65XX_POWER_NONE



#include <cust_acc.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include "kxtf9.h"
#include <linux/hwmsen_helper.h>


#define LENOVO_DTAP_FEATURE  1

#ifdef LENOVO_DTAP_FEATURE
#include <cust_eint.h>
#endif

/*----------------------------------------------------------------------------*/
#define I2C_DRIVERID_KXTF9 150
/*----------------------------------------------------------------------------*/
#define DEBUG 1
/*----------------------------------------------------------------------------*/
//#define CONFIG_KXTF9_LOWPASS   /*apply low pass filter on output*/       
#define SW_CALIBRATION

/*----------------------------------------------------------------------------*/
#define KXTF9_AXIS_X          0
#define KXTF9_AXIS_Y          1
#define KXTF9_AXIS_Z          2
#define KXTF9_AXES_NUM        3
#define KXTF9_DATA_LEN        6
#define KXTF9_DEV_NAME        "KXTF9"
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static const struct i2c_device_id kxtf9_i2c_id[] = {{KXTF9_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_kxtf9={ I2C_BOARD_INFO(KXTF9_DEV_NAME, (KXTF9_I2C_SLAVE_ADDR>>1))};
/*the adapter id will be available in customization*/
//static unsigned short kxtf9_force[] = {0x00, KXTF9_I2C_SLAVE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const kxtf9_forces[] = { kxtf9_force, NULL };
//static struct i2c_client_address_data kxtf9_addr_data = { .forces = kxtf9_forces,};
#ifdef LENOVO_DTAP_FEATURE
/******************************************************************************
 * extern functions
*******************************************************************************/
	extern void mt_eint_unmask(unsigned int line);
	extern void mt_eint_mask(unsigned int line);
	extern void mt_eint_set_polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
	extern void mt_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
	extern kal_uint32 mt_eint_set_sens(kal_uint8 eintno, kal_bool sens);
	extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
#endif

/*----------------------------------------------------------------------------*/
static int kxtf9_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int kxtf9_i2c_remove(struct i2c_client *client);
static int kxtf9_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);

/*----------------------------------------------------------------------------*/
typedef enum {
    ADX_TRC_FILTER  = 0x01,
    ADX_TRC_RAWDATA = 0x02,
    ADX_TRC_IOCTL   = 0x04,
    ADX_TRC_CALI	= 0X08,
    ADX_TRC_INFO	= 0X10,
} ADX_TRC;
/*----------------------------------------------------------------------------*/
struct scale_factor{
    u8  whole;
    u8  fraction;
};
/*----------------------------------------------------------------------------*/
struct data_resolution {
    struct scale_factor scalefactor;
    int                 sensitivity;
};
/*----------------------------------------------------------------------------*/
#define C_MAX_FIR_LENGTH (32)
/*----------------------------------------------------------------------------*/
struct data_filter {
    s16 raw[C_MAX_FIR_LENGTH][KXTF9_AXES_NUM];
    int sum[KXTF9_AXES_NUM];
    int num;
    int idx;
};
/*----------------------------------------------------------------------------*/
struct kxtf9_i2c_data {
    struct i2c_client *client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;
    
    /*misc*/
    struct data_resolution *reso;
    atomic_t                trace;
    atomic_t                suspend;
    atomic_t                selftest;
	atomic_t				filter;
    s16                     cali_sw[KXTF9_AXES_NUM+1];

    /*data*/
    s8                      offset[KXTF9_AXES_NUM+1];  /*+1: for 4-byte alignment*/
    s16                     data[KXTF9_AXES_NUM+1];

#if defined(CONFIG_KXTF9_LOWPASS)
    atomic_t                firlen;
    atomic_t                fir_en;
    struct data_filter      fir;
#endif 
    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
#ifdef LENOVO_DTAP_FEATURE
	struct work_struct eint_work;
	struct input_dev  *input_dev;
	atomic_t           gs_en;
	atomic_t           lv_en;
#endif
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver kxtf9_i2c_driver = {
    .driver = {
//      .owner          = THIS_MODULE,
        .name           = KXTF9_DEV_NAME,
    },
	.probe      		= kxtf9_i2c_probe,
	.remove    			= kxtf9_i2c_remove,
	.detect				= kxtf9_i2c_detect,
#if !defined(CONFIG_HAS_EARLYSUSPEND)    
    .suspend            = kxtf9_suspend,
    .resume             = kxtf9_resume,
#endif
	.id_table = kxtf9_i2c_id,
//	.address_data = &kxtf9_addr_data,
};

/*----------------------------------------------------------------------------*/
static struct i2c_client *kxtf9_i2c_client = NULL;
static struct platform_driver kxtf9_gsensor_driver;
static struct kxtf9_i2c_data *obj_i2c_data = NULL;
static bool sensor_power = true;
static GSENSOR_VECTOR3D gsensor_gain;
static char selftestRes[8]= {0}; 


/*----------------------------------------------------------------------------*/
#define GSE_TAG                  "[Gsensor] "
#define GSE_FUN(f)               printk(KERN_INFO GSE_TAG"%s\n", __FUNCTION__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define GSE_LOG(fmt, args...)    printk(KERN_INFO GSE_TAG fmt, ##args)
/*----------------------------------------------------------------------------*/
static struct data_resolution kxtf9_data_resolution[1] = {
 /* combination by {FULL_RES,RANGE}*/
    {{ 0, 9}, 1024}, // dataformat +/-2g  in 12-bit resolution;  { 3, 9} = 3.9 = (2*2*1000)/(2^12);  256 = (2^12)/(2*2)          
};
/*----------------------------------------------------------------------------*/
static struct data_resolution kxtf9_offset_resolution = {{15, 6}, 64};
/*----------------------------------------------------------------------------*/
static int KXTF9_SetPowerMode(struct i2c_client *client, bool enable);
/*--------------------KXTF9 power control function----------------------------------*/
static void KXTF9_power(struct acc_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	if(hw->power_id != POWER_NONE_MACRO)		// have externel LDO
	{        
		GSE_LOG("power %s\n", on ? "on" : "off");
		if(power_on == on)	// power status not change
		{
			GSE_LOG("ignore power control: %d\n", on);
		}
		else if(on)	// power on
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "KXTF9"))
			{
				GSE_ERR("power on fails!!\n");
			}
		}
		else	// power off
		{
			if (!hwPowerDown(hw->power_id, "KXTF9"))
			{
				GSE_ERR("power off fail!!\n");
			}			  
		}
	}
	power_on = on;    
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int KXTF9_SetDataResolution(struct kxtf9_i2c_data *obj)
{
	int err;
	u8  databuf[2], reso;

#ifndef LENOVO_DTAP_FEATURE
	KXTF9_SetPowerMode(obj->client, false);
#endif

	if(hwmsen_read_block(obj->client, KXTF9_REG_DATA_RESOLUTION, databuf, 0x01))
	{
		printk("kxtf9 read Dataformat failt \n");
		return KXTF9_ERR_I2C;
	}

	databuf[0] &= ~KXTF9_RANGE_DATA_RESOLUTION_MASK;
	databuf[0] |= KXTF9_RANGE_DATA_RESOLUTION_MASK;//12bit
	databuf[1] = databuf[0];
	databuf[0] = KXTF9_REG_DATA_RESOLUTION;


	err = i2c_master_send(obj->client, databuf, 0x2);

	if(err <= 0)
	{
		return KXTF9_ERR_I2C;
	}

#ifndef LENOVO_DTAP_FEATURE
	KXTF9_SetPowerMode(obj->client, true);
#endif

	//kxtf9_data_resolution[0] has been set when initialize: +/-2g  in 8-bit resolution:  15.6 mg/LSB*/   
	obj->reso = &kxtf9_data_resolution[0];

	return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_ReadData(struct i2c_client *client, s16 data[KXTF9_AXES_NUM])
{
	struct kxtf9_i2c_data *priv = i2c_get_clientdata(client);        
	u8 addr = KXTF9_REG_DATAX0;
	u8 buf[KXTF9_DATA_LEN] = {0};
	int err = 0;
	int i;
	int tmp=0;
	u8 ofs[3];



	if(NULL == client)
	{
		err = -EINVAL;
	}
	else if(err = hwmsen_read_block(client, addr, buf, 0x06))
	{
		GSE_ERR("error: %d\n", err);
	}
	else
	{
		data[KXTF9_AXIS_X] = (s16)((buf[KXTF9_AXIS_X*2] >> 4) |
		         (buf[KXTF9_AXIS_X*2+1] << 4));
		data[KXTF9_AXIS_Y] = (s16)((buf[KXTF9_AXIS_Y*2] >> 4) |
		         (buf[KXTF9_AXIS_Y*2+1] << 4));
		data[KXTF9_AXIS_Z] = (s16)((buf[KXTF9_AXIS_Z*2] >> 4) |
		         (buf[KXTF9_AXIS_Z*2+1] << 4));

		for(i=0;i<3;i++)				
		{								//because the data is store in binary complement number formation in computer system
			if ( data[i] == 0x0800 )	//so we want to calculate actual number here
				data[i]= -2048;			//10bit resolution, 512= 2^(12-1)
			else if ( data[i] & 0x0800 )//transfor format
			{							//printk("data 0 step %x \n",data[i]);
				data[i] -= 0x1; 		//printk("data 1 step %x \n",data[i]);
				data[i] = ~data[i]; 	//printk("data 2 step %x \n",data[i]);
				data[i] &= 0x07ff;		//printk("data 3 step %x \n\n",data[i]);
				data[i] = -data[i]; 	
			}
		}	


		if(atomic_read(&priv->trace) & ADX_TRC_RAWDATA)
		{
			GSE_LOG("[%08X %08X %08X] => [%5d %5d %5d]\n", data[KXTF9_AXIS_X], data[KXTF9_AXIS_Y], data[KXTF9_AXIS_Z],
		                               data[KXTF9_AXIS_X], data[KXTF9_AXIS_Y], data[KXTF9_AXIS_Z]);
		}
#ifdef CONFIG_KXTF9_LOWPASS
		if(atomic_read(&priv->filter))
		{
			if(atomic_read(&priv->fir_en) && !atomic_read(&priv->suspend))
			{
				int idx, firlen = atomic_read(&priv->firlen);   
				if(priv->fir.num < firlen)
				{                
					priv->fir.raw[priv->fir.num][KXTF9_AXIS_X] = data[KXTF9_AXIS_X];
					priv->fir.raw[priv->fir.num][KXTF9_AXIS_Y] = data[KXTF9_AXIS_Y];
					priv->fir.raw[priv->fir.num][KXTF9_AXIS_Z] = data[KXTF9_AXIS_Z];
					priv->fir.sum[KXTF9_AXIS_X] += data[KXTF9_AXIS_X];
					priv->fir.sum[KXTF9_AXIS_Y] += data[KXTF9_AXIS_Y];
					priv->fir.sum[KXTF9_AXIS_Z] += data[KXTF9_AXIS_Z];
					if(atomic_read(&priv->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d]\n", priv->fir.num,
							priv->fir.raw[priv->fir.num][KXTF9_AXIS_X], priv->fir.raw[priv->fir.num][KXTF9_AXIS_Y], priv->fir.raw[priv->fir.num][KXTF9_AXIS_Z],
							priv->fir.sum[KXTF9_AXIS_X], priv->fir.sum[KXTF9_AXIS_Y], priv->fir.sum[KXTF9_AXIS_Z]);
					}
					priv->fir.num++;
					priv->fir.idx++;
				}
				else
				{
					idx = priv->fir.idx % firlen;
					priv->fir.sum[KXTF9_AXIS_X] -= priv->fir.raw[idx][KXTF9_AXIS_X];
					priv->fir.sum[KXTF9_AXIS_Y] -= priv->fir.raw[idx][KXTF9_AXIS_Y];
					priv->fir.sum[KXTF9_AXIS_Z] -= priv->fir.raw[idx][KXTF9_AXIS_Z];
					priv->fir.raw[idx][KXTF9_AXIS_X] = data[KXTF9_AXIS_X];
					priv->fir.raw[idx][KXTF9_AXIS_Y] = data[KXTF9_AXIS_Y];
					priv->fir.raw[idx][KXTF9_AXIS_Z] = data[KXTF9_AXIS_Z];
					priv->fir.sum[KXTF9_AXIS_X] += data[KXTF9_AXIS_X];
					priv->fir.sum[KXTF9_AXIS_Y] += data[KXTF9_AXIS_Y];
					priv->fir.sum[KXTF9_AXIS_Z] += data[KXTF9_AXIS_Z];
					priv->fir.idx++;
					data[KXTF9_AXIS_X] = priv->fir.sum[KXTF9_AXIS_X]/firlen;
					data[KXTF9_AXIS_Y] = priv->fir.sum[KXTF9_AXIS_Y]/firlen;
					data[KXTF9_AXIS_Z] = priv->fir.sum[KXTF9_AXIS_Z]/firlen;
					if(atomic_read(&priv->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d] : [%5d %5d %5d]\n", idx,
						priv->fir.raw[idx][KXTF9_AXIS_X], priv->fir.raw[idx][KXTF9_AXIS_Y], priv->fir.raw[idx][KXTF9_AXIS_Z],
						priv->fir.sum[KXTF9_AXIS_X], priv->fir.sum[KXTF9_AXIS_Y], priv->fir.sum[KXTF9_AXIS_Z],
						data[KXTF9_AXIS_X], data[KXTF9_AXIS_Y], data[KXTF9_AXIS_Z]);
					}
				}
			}
		}	
#endif         
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_ReadOffset(struct i2c_client *client, s8 ofs[KXTF9_AXES_NUM])
{    
	int err;

	ofs[1]=ofs[2]=ofs[0]=0x00;

	printk("offesx=%x, y=%x, z=%x",ofs[0],ofs[1],ofs[2]);
	
	return err;    
}
/*----------------------------------------------------------------------------*/
static int KXTF9_ResetCalibration(struct i2c_client *client)
{
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);
	u8 ofs[4]={0,0,0,0};
	int err;

	memset(obj->cali_sw, 0x00, sizeof(obj->cali_sw));
	memset(obj->offset, 0x00, sizeof(obj->offset));
	return err;    
}
/*----------------------------------------------------------------------------*/
static int KXTF9_ReadCalibration(struct i2c_client *client, int dat[KXTF9_AXES_NUM])
{
    struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);
    int err;
    int mul;

	#ifdef SW_CALIBRATION
		mul = 0;//only SW Calibration, disable HW Calibration
	#else
	    if ((err = KXTF9_ReadOffset(client, obj->offset))) {
        GSE_ERR("read offset fail, %d\n", err);
        return err;
    	}    
    	mul = obj->reso->sensitivity/kxtf9_offset_resolution.sensitivity;
	#endif

    dat[obj->cvt.map[KXTF9_AXIS_X]] = obj->cvt.sign[KXTF9_AXIS_X]*(obj->offset[KXTF9_AXIS_X]*mul + obj->cali_sw[KXTF9_AXIS_X]);
    dat[obj->cvt.map[KXTF9_AXIS_Y]] = obj->cvt.sign[KXTF9_AXIS_Y]*(obj->offset[KXTF9_AXIS_Y]*mul + obj->cali_sw[KXTF9_AXIS_Y]);
    dat[obj->cvt.map[KXTF9_AXIS_Z]] = obj->cvt.sign[KXTF9_AXIS_Z]*(obj->offset[KXTF9_AXIS_Z]*mul + obj->cali_sw[KXTF9_AXIS_Z]);                        
                                       
    return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_ReadCalibrationEx(struct i2c_client *client, int act[KXTF9_AXES_NUM], int raw[KXTF9_AXES_NUM])
{  
	/*raw: the raw calibration data; act: the actual calibration data*/
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int mul;

 

	#ifdef SW_CALIBRATION
		mul = 0;//only SW Calibration, disable HW Calibration
	#else
		if(err = KXTF9_ReadOffset(client, obj->offset))
		{
			GSE_ERR("read offset fail, %d\n", err);
			return err;
		}   
		mul = obj->reso->sensitivity/kxtf9_offset_resolution.sensitivity;
	#endif
	
	raw[KXTF9_AXIS_X] = obj->offset[KXTF9_AXIS_X]*mul + obj->cali_sw[KXTF9_AXIS_X];
	raw[KXTF9_AXIS_Y] = obj->offset[KXTF9_AXIS_Y]*mul + obj->cali_sw[KXTF9_AXIS_Y];
	raw[KXTF9_AXIS_Z] = obj->offset[KXTF9_AXIS_Z]*mul + obj->cali_sw[KXTF9_AXIS_Z];

	act[obj->cvt.map[KXTF9_AXIS_X]] = obj->cvt.sign[KXTF9_AXIS_X]*raw[KXTF9_AXIS_X];
	act[obj->cvt.map[KXTF9_AXIS_Y]] = obj->cvt.sign[KXTF9_AXIS_Y]*raw[KXTF9_AXIS_Y];
	act[obj->cvt.map[KXTF9_AXIS_Z]] = obj->cvt.sign[KXTF9_AXIS_Z]*raw[KXTF9_AXIS_Z];                        
	                       
	return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_WriteCalibration(struct i2c_client *client, int dat[KXTF9_AXES_NUM])
{
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int cali[KXTF9_AXES_NUM], raw[KXTF9_AXES_NUM];
	int lsb = kxtf9_offset_resolution.sensitivity;
	int divisor = obj->reso->sensitivity/lsb;

	if(err = KXTF9_ReadCalibrationEx(client, cali, raw))	/*offset will be updated in obj->offset*/
	{ 
		GSE_ERR("read offset fail, %d\n", err);
		return err;
	}

	GSE_LOG("OLDOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		raw[KXTF9_AXIS_X], raw[KXTF9_AXIS_Y], raw[KXTF9_AXIS_Z],
		obj->offset[KXTF9_AXIS_X], obj->offset[KXTF9_AXIS_Y], obj->offset[KXTF9_AXIS_Z],
		obj->cali_sw[KXTF9_AXIS_X], obj->cali_sw[KXTF9_AXIS_Y], obj->cali_sw[KXTF9_AXIS_Z]);

	/*calculate the real offset expected by caller*/
	cali[KXTF9_AXIS_X] += dat[KXTF9_AXIS_X];
	cali[KXTF9_AXIS_Y] += dat[KXTF9_AXIS_Y];
	cali[KXTF9_AXIS_Z] += dat[KXTF9_AXIS_Z];

	GSE_LOG("UPDATE: (%+3d %+3d %+3d)\n", 
		dat[KXTF9_AXIS_X], dat[KXTF9_AXIS_Y], dat[KXTF9_AXIS_Z]);

#ifdef SW_CALIBRATION
	obj->cali_sw[KXTF9_AXIS_X] = obj->cvt.sign[KXTF9_AXIS_X]*(cali[obj->cvt.map[KXTF9_AXIS_X]]);
	obj->cali_sw[KXTF9_AXIS_Y] = obj->cvt.sign[KXTF9_AXIS_Y]*(cali[obj->cvt.map[KXTF9_AXIS_Y]]);
	obj->cali_sw[KXTF9_AXIS_Z] = obj->cvt.sign[KXTF9_AXIS_Z]*(cali[obj->cvt.map[KXTF9_AXIS_Z]]);	
#else
	obj->offset[KXTF9_AXIS_X] = (s8)(obj->cvt.sign[KXTF9_AXIS_X]*(cali[obj->cvt.map[KXTF9_AXIS_X]])/(divisor));
	obj->offset[KXTF9_AXIS_Y] = (s8)(obj->cvt.sign[KXTF9_AXIS_Y]*(cali[obj->cvt.map[KXTF9_AXIS_Y]])/(divisor));
	obj->offset[KXTF9_AXIS_Z] = (s8)(obj->cvt.sign[KXTF9_AXIS_Z]*(cali[obj->cvt.map[KXTF9_AXIS_Z]])/(divisor));

	/*convert software calibration using standard calibration*/
	obj->cali_sw[KXTF9_AXIS_X] = obj->cvt.sign[KXTF9_AXIS_X]*(cali[obj->cvt.map[KXTF9_AXIS_X]])%(divisor);
	obj->cali_sw[KXTF9_AXIS_Y] = obj->cvt.sign[KXTF9_AXIS_Y]*(cali[obj->cvt.map[KXTF9_AXIS_Y]])%(divisor);
	obj->cali_sw[KXTF9_AXIS_Z] = obj->cvt.sign[KXTF9_AXIS_Z]*(cali[obj->cvt.map[KXTF9_AXIS_Z]])%(divisor);

	GSE_LOG("NEWOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		obj->offset[KXTF9_AXIS_X]*divisor + obj->cali_sw[KXTF9_AXIS_X], 
		obj->offset[KXTF9_AXIS_Y]*divisor + obj->cali_sw[KXTF9_AXIS_Y], 
		obj->offset[KXTF9_AXIS_Z]*divisor + obj->cali_sw[KXTF9_AXIS_Z], 
		obj->offset[KXTF9_AXIS_X], obj->offset[KXTF9_AXIS_Y], obj->offset[KXTF9_AXIS_Z],
		obj->cali_sw[KXTF9_AXIS_X], obj->cali_sw[KXTF9_AXIS_Y], obj->cali_sw[KXTF9_AXIS_Z]);

	if(err = hwmsen_write_block(obj->client, KXTF9_REG_OFSX, obj->offset, KXTF9_AXES_NUM))
	{
		GSE_ERR("write offset fail: %d\n", err);
		return err;
	}
#endif

	return err;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_CheckDeviceID(struct i2c_client *client)
{
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    
	databuf[0] = KXTF9_REG_DEVID;   

	res = i2c_master_send(client, databuf, 0x1);
	if(res <= 0)
	{
		goto exit_KXTF9_CheckDeviceID;
	}
	
	udelay(500);

	databuf[0] = 0x0;        
	res = i2c_master_recv(client, databuf, 0x01);
	if(res <= 0)
	{
		goto exit_KXTF9_CheckDeviceID;
	}
	

	if(false)
	{
		printk("KXTF9_CheckDeviceID 0x%x failt!\n ", databuf[0]);
		return KXTF9_ERR_IDENTIFICATION;
	}
	else
	{
		printk("KXTF9_CheckDeviceID 0x%x pass!\n ", databuf[0]);
	}
	
	exit_KXTF9_CheckDeviceID:
	if (res <= 0)
	{
		return KXTF9_ERR_I2C;
	}
	
	return KXTF9_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_SetPowerMode(struct i2c_client *client, bool enable)
{
	u8 databuf[2];    
	int res = 0;
	u8 addr = KXTF9_REG_POWER_CTL;
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);
	
	
	if(enable == sensor_power)
	{
		GSE_LOG("Sensor power status is newest!\n");
		return KXTF9_SUCCESS;
	}

#ifndef LENOVO_DTAP_FEATURE
	if(hwmsen_read_block(client, addr, databuf, 0x01))
	{
		GSE_ERR("read power ctl register err!\n");
		return KXTF9_ERR_I2C;
	}

	
	if(enable == TRUE)
	{
		databuf[0] |= KXTF9_MEASURE_MODE;
	}
	else
	{
		databuf[0] &= ~KXTF9_MEASURE_MODE;
	}
	databuf[1] = databuf[0];
	databuf[0] = KXTF9_REG_POWER_CTL;
#else	
	databuf[0] = KXTF9_REG_POWER_CTL;
	if(enable == true)
	{
		databuf[1] = 0xC0;
	}
	else
	{
		databuf[1] = 0x00;
	}
#endif
	

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return KXTF9_ERR_I2C;
	}


	GSE_LOG("KXTF9_SetPowerMode %d!\n ",enable);


	sensor_power = enable;

	mdelay(5);
	
	return KXTF9_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int KXTF9_SetDataFormat(struct i2c_client *client, u8 dataformat)
{
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);  

#ifndef LENOVO_DTAP_FEATURE
	KXTF9_SetPowerMode(client, false);
#endif

	if(hwmsen_read_block(client, KXTF9_REG_DATA_FORMAT, databuf, 0x01))
	{
		printk("kxtf9 read Dataformat failt \n");
		return KXTF9_ERR_I2C;
	}

	databuf[0] &= ~KXTF9_RANGE_MASK;
	databuf[0] |= dataformat;
	databuf[1] = databuf[0];
	databuf[0] = KXTF9_REG_DATA_FORMAT;


	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return KXTF9_ERR_I2C;
	}

#ifdef LENOVO_DTAP_FEATURE
	KXTF9_SetPowerMode(client, true);
#endif
	
	printk("KXTF9_SetDataFormat OK! \n");
	

	return KXTF9_SetDataResolution(obj);    
}
/*----------------------------------------------------------------------------*/
static int KXTF9_SetBWRate(struct i2c_client *client, u8 bwrate)
{
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    

	if(hwmsen_read_block(client, KXTF9_REG_BW_RATE, databuf, 0x01))
	{
		printk("kxtf9 read rate failt \n");
		return KXTF9_ERR_I2C;
	}

	databuf[0] &= 0xf8;
	databuf[0] |= bwrate;
	databuf[1] = databuf[0];
	databuf[0] = KXTF9_REG_BW_RATE;


	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return KXTF9_ERR_I2C;
	}
	
	printk("KXTF9_SetBWRate OK! \n");
	
	return KXTF9_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int KXTF9_SetIntEnable(struct i2c_client *client, u8 intenable)
{
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    
#ifndef LENOVO_DTAP_FEATURE
	databuf[0] = KXTF9_REG_INT_ENABLE;    
	databuf[1] = 0x00;

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return KXTF9_ERR_I2C;
	}
#else
	if(0 == intenable)
	{
		databuf[0] = KXTF9_REG_INT_ENABLE;    
		databuf[1] = 0x00;
		res = i2c_master_send(client, databuf, 0x02);

		if(res <= 0)
		{
			return KXTF9_ERR_I2C;
		}
	}
	else
	{
		databuf[0] = KXTF9_REG_INT_ENABLE;    
		databuf[1] = 0x20;
		res = i2c_master_send(client, databuf, 0x02);
		if(res <= 0)
		{
			return KXTF9_ERR_I2C;
		}

		databuf[0] = KXTF9_REG_INT_ENABLE+1;
		databuf[1] = 0x00;
		res = i2c_master_send(client, databuf, 0x02);
		if(res <= 0)
		{
			return KXTF9_ERR_I2C;
		}

		databuf[0] = KXTF9_REG_INT_ENABLE+2;    
		databuf[1] = 0x03;
		res = i2c_master_send(client, databuf, 0x02);
		if(res <= 0)
		{
			return KXTF9_ERR_I2C;
		}
        /* double tap threshold setting */
        databuf[0] = 0x2C;
        databuf[1] = 0x64;  //high default 0xCB  --0x60
        res = i2c_master_send(client, databuf, 0x02);
        if(res <= 0)
        {
                return KXTF9_ERR_I2C;
        }
        databuf[0] = 0x2D;
        databuf[1] = 0x22;  //low  default 0x1A  --0x20
        res = i2c_master_send(client, databuf, 0x02);
        if(res <= 0)
        {
                return KXTF9_ERR_I2C;
        }
		/* TDT_LATENCY_TIME */
        databuf[0] = 0x30;
        databuf[1] = 0x30;  //default 0x28  == 100mS
        res = i2c_master_send(client, databuf, 0x02);
        if(res <= 0)
        {
                return KXTF9_ERR_I2C;
        }
		/* TDT_WINDOW_TIME */
        databuf[0] = 0x31;
        databuf[1] = 0x90;  //default 0xA0  == 400mS
        res = i2c_master_send(client, databuf, 0x02);
        if(res <= 0)
        {
                return KXTF9_ERR_I2C;
        }

	}
#endif
	
	return KXTF9_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int kxtf9_init_client(struct i2c_client *client, int reset_cali)
{
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);
	int res = 0;

	res = KXTF9_CheckDeviceID(client); 
	if(res != KXTF9_SUCCESS)
	{
		return res;
	}	

	res = KXTF9_SetPowerMode(client, false);
	if(res != KXTF9_SUCCESS)
	{
		return res;
	}
#ifdef LENOVO_DTAP_FEATURE
	res = KXTF9_SetIntEnable(client, 1);        
	if(res != KXTF9_SUCCESS)//0x2E->0x80
	{
		return res;
	}
#endif
	res = KXTF9_SetBWRate(client, KXTF9_BW_100HZ);
	if(res != KXTF9_SUCCESS ) //0x2C->BW=100Hz
	{
		return res;
	}

	res = KXTF9_SetDataFormat(client, KXTF9_RANGE_2G);
	if(res != KXTF9_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}

	gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = obj->reso->sensitivity;

#ifndef LENOVO_DTAP_FEATURE
	res = KXTF9_SetIntEnable(client, 0x00);        
	if(res != KXTF9_SUCCESS)//0x2E->0x80
	{
		return res;
	}
#endif

	if(0 != reset_cali)
	{ 
		/*reset calibration only in power on*/
		res = KXTF9_ResetCalibration(client);
		if(res != KXTF9_SUCCESS)
		{
			return res;
		}
	}
	printk("kxtf9_init_client OK!\n");
#ifdef CONFIG_KXTF9_LOWPASS
	memset(&obj->fir, 0x00, sizeof(obj->fir));  
#endif

	return KXTF9_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
{
	u8 databuf[10];    

	memset(databuf, 0, sizeof(u8)*10);

	if((NULL == buf)||(bufsize<=30))
	{
		return -1;
	}
	
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	sprintf(buf, "KXTF9 Chip");
	return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_ReadSensorData(struct i2c_client *client, char *buf, int bufsize)
{
	struct kxtf9_i2c_data *obj = (struct kxtf9_i2c_data*)i2c_get_clientdata(client);
	u8 databuf[20];
	int acc[KXTF9_AXES_NUM];
	int res = 0;
	memset(databuf, 0, sizeof(u8)*10);

	if(NULL == buf)
	{
		return -1;
	}
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

#ifndef LENOVO_DTAP_FEATURE
    if(sensor_power == FALSE)
#else
	if(0 == atomic_read(&obj->suspend))
#endif
	{
		res = KXTF9_SetPowerMode(client, true);
		if(res)
		{
			GSE_ERR("Power on kxtf9 error %d!\n", res);
		}
	}

	if(res = KXTF9_ReadData(client, obj->data))
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return -3;
	}
	else
	{
		//printk("raw data x=%d, y=%d, z=%d \n",obj->data[KXTF9_AXIS_X],obj->data[KXTF9_AXIS_Y],obj->data[KXTF9_AXIS_Z]);
		obj->data[KXTF9_AXIS_X] += obj->cali_sw[KXTF9_AXIS_X];
		obj->data[KXTF9_AXIS_Y] += obj->cali_sw[KXTF9_AXIS_Y];
		obj->data[KXTF9_AXIS_Z] += obj->cali_sw[KXTF9_AXIS_Z];
		
		//printk("cali_sw x=%d, y=%d, z=%d \n",obj->cali_sw[KXTF9_AXIS_X],obj->cali_sw[KXTF9_AXIS_Y],obj->cali_sw[KXTF9_AXIS_Z]);
		
		/*remap coordinate*/
		acc[obj->cvt.map[KXTF9_AXIS_X]] = obj->cvt.sign[KXTF9_AXIS_X]*obj->data[KXTF9_AXIS_X];
		acc[obj->cvt.map[KXTF9_AXIS_Y]] = obj->cvt.sign[KXTF9_AXIS_Y]*obj->data[KXTF9_AXIS_Y];
		acc[obj->cvt.map[KXTF9_AXIS_Z]] = obj->cvt.sign[KXTF9_AXIS_Z]*obj->data[KXTF9_AXIS_Z];
		//printk("cvt x=%d, y=%d, z=%d \n",obj->cvt.sign[KXTF9_AXIS_X],obj->cvt.sign[KXTF9_AXIS_Y],obj->cvt.sign[KXTF9_AXIS_Z]);


		//GSE_LOG("Mapped gsensor data: %d, %d, %d!\n", acc[KXTF9_AXIS_X], acc[KXTF9_AXIS_Y], acc[KXTF9_AXIS_Z]);

		//Out put the mg
		//printk("mg acc=%d, GRAVITY=%d, sensityvity=%d \n",acc[KXTF9_AXIS_X],GRAVITY_EARTH_1000,obj->reso->sensitivity);
		acc[KXTF9_AXIS_X] = acc[KXTF9_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[KXTF9_AXIS_Y] = acc[KXTF9_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[KXTF9_AXIS_Z] = acc[KXTF9_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;		
		
	

		sprintf(buf, "%04x %04x %04x", acc[KXTF9_AXIS_X], acc[KXTF9_AXIS_Y], acc[KXTF9_AXIS_Z]);
		if(atomic_read(&obj->trace) & ADX_TRC_IOCTL)
		{
			GSE_LOG("gsensor data: %s!\n", buf);
		}
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_ReadRawData(struct i2c_client *client, char *buf)
{
	struct kxtf9_i2c_data *obj = (struct kxtf9_i2c_data*)i2c_get_clientdata(client);
	int res = 0;

	if (!buf || !client)
	{
		return EINVAL;
	}
	
	if(res = KXTF9_ReadData(client, obj->data))
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return EIO;
	}
	else
	{
		sprintf(buf, "KXTF9_ReadRawData %04x %04x %04x", obj->data[KXTF9_AXIS_X], 
			obj->data[KXTF9_AXIS_Y], obj->data[KXTF9_AXIS_Z]);
	
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_InitSelfTest(struct i2c_client *client)
{
	int res = 0;
	u8  data,result;
	
	res = hwmsen_read_byte(client, KXTF9_REG_CTL_REG3, &data);
	if(res != KXTF9_SUCCESS)
	{
		return res;
	}
//enable selftest bit
	res = hwmsen_write_byte(client, KXTF9_REG_CTL_REG3,  KXTF9_SELF_TEST|data);
	if(res != KXTF9_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
//step 1
	res = hwmsen_read_byte(client, KXTF9_DCST_RESP, &result);
	if(res != KXTF9_SUCCESS)
	{
		return res;
	}
	printk("step1: result = %x",result);
	if(result != 0xaa)
		return -EINVAL;

//step 2
	res = hwmsen_write_byte(client, KXTF9_REG_CTL_REG3,  KXTF9_SELF_TEST|data);
	if(res != KXTF9_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
//step 3
	res = hwmsen_read_byte(client, KXTF9_DCST_RESP, &result);
	if(res != KXTF9_SUCCESS)
	{
		return res;
	}
	printk("step3: result = %x",result);
	if(result != 0xAA)
		return -EINVAL;
		
//step 4
	res = hwmsen_read_byte(client, KXTF9_DCST_RESP, &result);
	if(res != KXTF9_SUCCESS)
	{
		return res;
	}
	printk("step4: result = %x",result);
	if(result != 0x55)
		return -EINVAL;
	else
		return KXTF9_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int KXTF9_JudgeTestResult(struct i2c_client *client, s32 prv[KXTF9_AXES_NUM], s32 nxt[KXTF9_AXES_NUM])
{

    int res=0;
	u8 test_result=0;
    if(res = hwmsen_read_byte(client, 0x0c, &test_result))
        return res;

	printk("test_result = %x \n",test_result);
    if ( test_result != 0xaa ) 
	{
        GSE_ERR("KXTF9_JudgeTestResult failt\n");
        res = -EINVAL;
    }
    return res;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtf9_i2c_client;
	char strbuf[KXTF9_BUFSIZE];
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	
	KXTF9_ReadChipInfo(client, strbuf, KXTF9_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);        
}

static ssize_t gsensor_init(struct device_driver *ddri, char *buf, size_t count)
	{
		struct i2c_client *client = kxtf9_i2c_client;
		char strbuf[KXTF9_BUFSIZE];
		
		if(NULL == client)
		{
			GSE_ERR("i2c client is null!!\n");
			return 0;
		}
		kxtf9_init_client(client, 1);
		return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);			
	}



/*----------------------------------------------------------------------------*/
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtf9_i2c_client;
	char strbuf[KXTF9_BUFSIZE];
	
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	KXTF9_ReadSensorData(client, strbuf, KXTF9_BUFSIZE);
	//KXTF9_ReadRawData(client, strbuf);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);            
}

static ssize_t show_sensorrawdata_value(struct device_driver *ddri, char *buf, size_t count)
	{
		struct i2c_client *client = kxtf9_i2c_client;
		char strbuf[KXTF9_BUFSIZE];
		
		if(NULL == client)
		{
			GSE_ERR("i2c client is null!!\n");
			return 0;
		}
		//KXTF9_ReadSensorData(client, strbuf, KXTF9_BUFSIZE);
		KXTF9_ReadRawData(client, strbuf);
		return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);			
	}

/*----------------------------------------------------------------------------*/
static ssize_t show_cali_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtf9_i2c_client;
	struct kxtf9_i2c_data *obj;
	int err, len = 0, mul;
	int tmp[KXTF9_AXES_NUM];

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);



	if(err = KXTF9_ReadOffset(client, obj->offset))
	{
		return -EINVAL;
	}
	else if(err = KXTF9_ReadCalibration(client, tmp))
	{
		return -EINVAL;
	}
	else
	{    
		mul = obj->reso->sensitivity/kxtf9_offset_resolution.sensitivity;
		len += snprintf(buf+len, PAGE_SIZE-len, "[HW ][%d] (%+3d, %+3d, %+3d) : (0x%02X, 0x%02X, 0x%02X)\n", mul,                        
			obj->offset[KXTF9_AXIS_X], obj->offset[KXTF9_AXIS_Y], obj->offset[KXTF9_AXIS_Z],
			obj->offset[KXTF9_AXIS_X], obj->offset[KXTF9_AXIS_Y], obj->offset[KXTF9_AXIS_Z]);
		len += snprintf(buf+len, PAGE_SIZE-len, "[SW ][%d] (%+3d, %+3d, %+3d)\n", 1, 
			obj->cali_sw[KXTF9_AXIS_X], obj->cali_sw[KXTF9_AXIS_Y], obj->cali_sw[KXTF9_AXIS_Z]);

		len += snprintf(buf+len, PAGE_SIZE-len, "[ALL]    (%+3d, %+3d, %+3d) : (%+3d, %+3d, %+3d)\n", 
			obj->offset[KXTF9_AXIS_X]*mul + obj->cali_sw[KXTF9_AXIS_X],
			obj->offset[KXTF9_AXIS_Y]*mul + obj->cali_sw[KXTF9_AXIS_Y],
			obj->offset[KXTF9_AXIS_Z]*mul + obj->cali_sw[KXTF9_AXIS_Z],
			tmp[KXTF9_AXIS_X], tmp[KXTF9_AXIS_Y], tmp[KXTF9_AXIS_Z]);
		
		return len;
    }
}
/*----------------------------------------------------------------------------*/
static ssize_t store_cali_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct i2c_client *client = kxtf9_i2c_client;  
	int err, x, y, z;
	int dat[KXTF9_AXES_NUM];

	if(!strncmp(buf, "rst", 3))
	{
		if(err = KXTF9_ResetCalibration(client))
		{
			GSE_ERR("reset offset err = %d\n", err);
		}	
	}
	else if(3 == sscanf(buf, "0x%02X 0x%02X 0x%02X", &x, &y, &z))
	{
		dat[KXTF9_AXIS_X] = x;
		dat[KXTF9_AXIS_Y] = y;
		dat[KXTF9_AXIS_Z] = z;
		if(err = KXTF9_WriteCalibration(client, dat))
		{
			GSE_ERR("write calibration err = %d\n", err);
		}		
	}
	else
	{
		GSE_ERR("invalid format\n");
	}
	
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_self_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtf9_i2c_client;
	struct kxtf9_i2c_data *obj;

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	//obj = i2c_get_clientdata(client);
	
    return snprintf(buf, 8, "%s\n", selftestRes);
}
/*----------------------------------------------------------------------------*/
static ssize_t store_self_value(struct device_driver *ddri, char *buf, size_t count)
{   /*write anything to this register will trigger the process*/
	struct item{
	s16 raw[KXTF9_AXES_NUM];
	};
	
	struct i2c_client *client = kxtf9_i2c_client;  
	int idx, res, num;
	struct item *prv = NULL, *nxt = NULL;
	s32 avg_prv[KXTF9_AXES_NUM] = {0, 0, 0};
	s32 avg_nxt[KXTF9_AXES_NUM] = {0, 0, 0};
	u8 data;


	if(1 != sscanf(buf, "%d", &num))
	{
		GSE_ERR("parse number fail\n");
		return count;
	}
	else if(num == 0)
	{
		GSE_ERR("invalid data count\n");
		return count;
	}

	prv = kzalloc(sizeof(*prv) * num, GFP_KERNEL);
	nxt = kzalloc(sizeof(*nxt) * num, GFP_KERNEL);
	if (!prv || !nxt)
	{
		goto exit;
	}


	GSE_LOG("NORMAL:\n");
	KXTF9_SetPowerMode(client,true); 

	/*initial setting for self test*/
	if(!KXTF9_InitSelfTest(client))
	{
		GSE_LOG("SELFTEST : PASS\n");
		strcpy(selftestRes,"y");
	}	
	else
	{
		GSE_LOG("SELFTEST : FAIL\n");		
		strcpy(selftestRes,"n");
	}

	res = hwmsen_read_byte(client, KXTF9_REG_CTL_REG3, &data);
	if(res != KXTF9_SUCCESS)
	{
		return res;
	}

	res = hwmsen_write_byte(client, KXTF9_REG_CTL_REG3,  ~KXTF9_SELF_TEST&data);
	if(res != KXTF9_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
	
	exit:
	/*restore the setting*/    
	kxtf9_init_client(client, 0);
	kfree(prv);
	kfree(nxt);
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_selftest_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtf9_i2c_client;
	struct kxtf9_i2c_data *obj;

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->selftest));
}
/*----------------------------------------------------------------------------*/
static ssize_t store_selftest_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct kxtf9_i2c_data *obj = obj_i2c_data;
	int tmp;

	if(NULL == obj)
	{
		GSE_ERR("i2c data obj is null!!\n");
		return 0;
	}
	
	
	if(1 == sscanf(buf, "%d", &tmp))
	{        
		if(atomic_read(&obj->selftest) && !tmp)
		{
			/*enable -> disable*/
			kxtf9_init_client(obj->client, 0);
		}
		else if(!atomic_read(&obj->selftest) && tmp)
		{
			/*disable -> enable*/
			KXTF9_InitSelfTest(obj->client);            
		}
		
		GSE_LOG("selftest: %d => %d\n", atomic_read(&obj->selftest), tmp);
		atomic_set(&obj->selftest, tmp); 
	}
	else
	{ 
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);   
	}
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_firlen_value(struct device_driver *ddri, char *buf)
{
#ifdef CONFIG_KXTF9_LOWPASS
	struct i2c_client *client = kxtf9_i2c_client;
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);
	if(atomic_read(&obj->firlen))
	{
		int idx, len = atomic_read(&obj->firlen);
		GSE_LOG("len = %2d, idx = %2d\n", obj->fir.num, obj->fir.idx);

		for(idx = 0; idx < len; idx++)
		{
			GSE_LOG("[%5d %5d %5d]\n", obj->fir.raw[idx][KXTF9_AXIS_X], obj->fir.raw[idx][KXTF9_AXIS_Y], obj->fir.raw[idx][KXTF9_AXIS_Z]);
		}
		
		GSE_LOG("sum = [%5d %5d %5d]\n", obj->fir.sum[KXTF9_AXIS_X], obj->fir.sum[KXTF9_AXIS_Y], obj->fir.sum[KXTF9_AXIS_Z]);
		GSE_LOG("avg = [%5d %5d %5d]\n", obj->fir.sum[KXTF9_AXIS_X]/len, obj->fir.sum[KXTF9_AXIS_Y]/len, obj->fir.sum[KXTF9_AXIS_Z]/len);
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->firlen));
#else
	return snprintf(buf, PAGE_SIZE, "not support\n");
#endif
}
/*----------------------------------------------------------------------------*/
static ssize_t store_firlen_value(struct device_driver *ddri, char *buf, size_t count)
{
#ifdef CONFIG_KXTF9_LOWPASS
	struct i2c_client *client = kxtf9_i2c_client;  
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);
	int firlen;

	if(1 != sscanf(buf, "%d", &firlen))
	{
		GSE_ERR("invallid format\n");
	}
	else if(firlen > C_MAX_FIR_LENGTH)
	{
		GSE_ERR("exceeds maximum filter length\n");
	}
	else
	{ 
		atomic_set(&obj->firlen, firlen);
		if(NULL == firlen)
		{
			atomic_set(&obj->fir_en, 0);
		}
		else
		{
			memset(&obj->fir, 0x00, sizeof(obj->fir));
			atomic_set(&obj->fir_en, 1);
		}
	}
#endif    
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct kxtf9_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));     
	return res;    
}
/*----------------------------------------------------------------------------*/
static ssize_t store_trace_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct kxtf9_i2c_data *obj = obj_i2c_data;
	int trace;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&obj->trace, trace);
	}	
	else
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	
	return count;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;    
	struct kxtf9_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}	
	
	if(obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d %d (%d %d)\n", 
	            obj->hw->i2c_num, obj->hw->direction, obj->hw->power_id, obj->hw->power_vol);   
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}
#ifdef LENOVO_DTAP_FEATURE
{
	int i,res;
	u8 buffer[2];
	struct i2c_client *client = obj->client;

	for(i=0x1b; i<0x22; i++)
	{
		buffer[0] = i;
	    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
		res = i2c_master_send(client, buffer, (1<<8) | 1);
	    client->addr = client->addr& I2C_MASK_FLAG;
		len += snprintf(buf+len, PAGE_SIZE-len, "reg[0x%x]: 0x%x\n", i, buffer[0]);
	}

	for(i=0x28; i<0x32; i++)
	{
		buffer[0] = i;
	    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
		res = i2c_master_send(client, buffer, (1<<8) | 1);
	    client->addr = client->addr& I2C_MASK_FLAG;
		len += snprintf(buf+len, PAGE_SIZE-len, "reg[0x%x]: 0x%x\n", i, buffer[0]);
	}

}
#endif
	return len;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_power_status_value(struct device_driver *ddri, char *buf)
{
	if(sensor_power)
		printk("G sensor is in work mode, sensor_power = %d\n", sensor_power);
	else
		printk("G sensor is in standby mode, sensor_power = %d\n", sensor_power);

	return 0;
}

/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(chipinfo,   S_IWUSR | S_IRUGO, show_chipinfo_value,      NULL);
static DRIVER_ATTR(sensordata, S_IWUSR | S_IRUGO, show_sensordata_value,    NULL);
static DRIVER_ATTR(cali,       S_IWUSR | S_IRUGO, show_cali_value,          store_cali_value);
static DRIVER_ATTR(selftest, S_IWUSR | S_IRUGO, show_self_value,  store_self_value);
static DRIVER_ATTR(self,   S_IWUSR | S_IRUGO, show_selftest_value,      store_selftest_value);
static DRIVER_ATTR(firlen,     S_IWUSR | S_IRUGO, show_firlen_value,        store_firlen_value);
static DRIVER_ATTR(trace,      S_IWUSR | S_IRUGO, show_trace_value,         store_trace_value);
static DRIVER_ATTR(status,               S_IRUGO, show_status_value,        NULL);
static DRIVER_ATTR(powerstatus,               S_IRUGO, show_power_status_value,        NULL);

#ifdef LENOVO_DTAP_FEATURE
/*********************************************************************************/
static int kx_request_input_dev(struct kxtf9_i2c_data *obj)
{
    int ret = -1;
    s8 phys[32];
    u8 index = 0;
  
    obj->input_dev = input_allocate_device();
    if (obj->input_dev == NULL)
    {
        printk("kx_request_input_dev Failed to allocate input device.");
        return -ENOMEM;
    }

    obj->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) ;
    input_set_capability(obj->input_dev,EV_KEY, KEY_POWER);	

    obj->input_dev->name = "LEV_KPD";
//    sprintf(phys, "input/obj");
//    obj->input_dev->phys = phys;
    obj->input_dev->id.bustype = BUS_I2C;
//    obj->input_dev->id.vendor = 0xDEAD;
//    obj->input_dev->id.product = 0xBEEF;
//    obj->input_dev->id.version = 10427;
	
    ret = input_register_device(obj->input_dev);
    if (ret)
    {
        printk("kx_request_input_dev Register %s input device failed", obj->input_dev->name);
        return -ENODEV;
    }
    
    return KXTF9_SUCCESS;
}

static void kx_eint_func(void)
{
	struct kxtf9_i2c_data *obj = obj_i2c_data;
	if(!obj)
	{
		return;
	}
	GSE_FUN();
	schedule_work(&obj->eint_work);
}


static int kx_setup_eint(struct i2c_client *client)
{
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);        

	mt_set_gpio_dir(GPIO_GSE_1_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_mode(GPIO_GSE_1_EINT_PIN, GPIO_GSE_1_EINT_PIN_M_EINT);
	mt_set_gpio_pull_enable(GPIO_GSE_1_EINT_PIN, TRUE);
	mt_set_gpio_pull_select(GPIO_GSE_1_EINT_PIN, GPIO_PULL_UP);

	//mt_eint_set_sens(CUST_EINT_GSE_1_NUM, CUST_EINT_GSE_1_SENSITIVE);
	//mt_eint_set_polarity(CUST_EINT_GSE_1_NUM, CUST_EINT_GSE_1_POLARITY);
	mt_eint_set_hw_debounce(CUST_EINT_GSE_1_NUM, CUST_EINT_GSE_1_DEBOUNCE_EN);
	mt_eint_registration(CUST_EINT_GSE_1_NUM, CUST_EINT_GSE_1_TYPE, kx_eint_func, 0);

	mt_eint_mask(CUST_EINT_GSE_1_NUM);  

    return KXTF9_SUCCESS;
}


static int kx_lv_enable(bool val)
{
	struct i2c_client *client = kxtf9_i2c_client;
	u8 buffer[2];
	int res;

	if(client == NULL)
	{
		return KXTF9_ERR_I2C;
	}

	if(true == val)
	{
		GSE_LOG("kx_lv_enable enable!\n");
		//res = KXTF9_SetIntEnable(client, 1);
		buffer[0] = KXTF9_REG_POWER_CTL;
		buffer[1] = 0x84;
		res = i2c_master_send(client, buffer, 0x2);
		if(res <= 0)
		{
			return KXTF9_ERR_I2C;
		}
		/* clear intr status */
		buffer[0] = 0x1A;
	    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
		res = i2c_master_send(client, buffer, (1<<8) | 1);
	    client->addr = client->addr& I2C_MASK_FLAG;
		if(res <= 0)
		{
			return KXTF9_ERR_I2C;
		}

		mt_eint_unmask(CUST_EINT_GSE_1_NUM);
	}
	else
	{
		GSE_LOG("kx_lv_enable disable!\n");
		mt_eint_mask(CUST_EINT_GSE_1_NUM);
		//KXTF9_SetIntEnable(client, 0);
	}

	return KXTF9_SUCCESS;
}

static void kx_eint_work(struct work_struct *work)
{
	struct kxtf9_i2c_data *obj = (struct kxtf9_i2c_data *)container_of(work, struct kxtf9_i2c_data, eint_work);
	struct i2c_client *client = obj->client;
	int res;
	u8 buffer[2], src, dir;

	GSE_FUN();
	buffer[0] = 0x18;
    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
	res = i2c_master_send(client, buffer, (1<<8) | 1);
    client->addr = client->addr& I2C_MASK_FLAG;
	if(res <= 0)
	{
		GSE_ERR("kx_eint_work read status reg failed!\n");
		goto EXIT_ERR;
	}
	else
	{
		GSE_LOG("kx_eint_work read status=0x%x", buffer[0]);
	}
	if((buffer[0] & 0x10) != 0)
	{
		buffer[0] = 0x16;
	    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
		res = i2c_master_send(client, buffer, (1<<8) | 1);
	    client->addr = client->addr& I2C_MASK_FLAG;
		src = buffer[0];
		GSE_LOG("kx_eint_work read src2=0x%x", buffer[0]);

		buffer[0] = 0x15;
	    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
		res = i2c_master_send(client, buffer, (1<<8) | 1);
	    client->addr = client->addr& I2C_MASK_FLAG;
		dir = buffer[0];
		GSE_LOG("kx_eint_work read src1=0x%x", buffer[0]);
		/* clear intr status */
		buffer[0] = 0x1A;
	    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
		res = i2c_master_send(client, buffer, (1<<8) | 1);
	    client->addr = client->addr& I2C_MASK_FLAG;

		//let up layer to know
		if((src == 8) && ((dir == 1) || (dir == 2)))
		{
			input_report_key(obj->input_dev, KEY_POWER, 1);
			input_report_key(obj->input_dev, KEY_POWER, 0);
			input_sync(obj->input_dev);
		}

	}

	mt_eint_unmask(CUST_EINT_GSE_1_NUM);

	return;

EXIT_ERR:
	GSE_ERR("kx_eint_work error break!\n");
}


/*----------------------------------------------------------------------------*/
static ssize_t show_lvenable(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct kxtf9_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	res = snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->lv_en));     
	return res;    
}
/*----------------------------------------------------------------------------*/
static ssize_t store_lvenable(struct device_driver *ddri, char *buf, size_t count)
{
	struct kxtf9_i2c_data *obj = obj_i2c_data;
	int trace;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}

	trace = simple_strtol(buf, NULL, 10);
	
	if(1 == trace)
	{
		atomic_set(&obj->lv_en, 1);
	}	
	else
	{
		atomic_set(&obj->lv_en, 0);
	}
	
	return count;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_lvthd(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct kxtf9_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));     
	return res;    
}
/*----------------------------------------------------------------------------*/
static ssize_t store_lvthd(struct device_driver *ddri, char *buf, size_t count)
{
	struct kxtf9_i2c_data *obj = obj_i2c_data;
	int trace;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&obj->trace, trace);
	}	
	else
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	
	return count;    
}


static DRIVER_ATTR(lvenable,    S_IWUSR | S_IRUGO, show_lvenable,       store_lvenable);
static DRIVER_ATTR(lvthd,       S_IWUSR | S_IRUGO, show_lvthd,          store_lvthd);
#endif

/*----------------------------------------------------------------------------*/
static u8 i2c_dev_reg =0 ;

static ssize_t show_register(struct device_driver *pdri, char *buf)
{
	int input_value;
		
	printk("i2c_dev_reg is 0x%2x \n", i2c_dev_reg);

	return 0;
}

static ssize_t store_register(struct device_driver *ddri, char *buf, size_t count)
{
	unsigned long input_value;

	i2c_dev_reg = simple_strtoul(buf, NULL, 16);
	printk("set i2c_dev_reg = 0x%2x \n", i2c_dev_reg);

	return 0;
}
static ssize_t store_register_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct kxtf9_i2c_data *obj = obj_i2c_data;
	u8 databuf[2];  
	unsigned long input_value;
	int res;
	
	memset(databuf, 0, sizeof(u8)*2);    

	input_value = simple_strtoul(buf, NULL, 16);
	printk("input_value = 0x%2x \n", input_value);

	if(NULL == obj)
	{
		GSE_ERR("i2c data obj is null!!\n");
		return 0;
	}

	databuf[0] = i2c_dev_reg;
	databuf[1] = input_value;
	printk("databuf[0]=0x%2x  databuf[1]=0x%2x \n", databuf[0],databuf[1]);

	res = i2c_master_send(obj->client, databuf, 0x2);

	if(res <= 0)
	{
		return KXTF9_ERR_I2C;
	}
	return 0;
	
}

static ssize_t show_register_value(struct device_driver *ddri, char *buf)
{
		struct kxtf9_i2c_data *obj = obj_i2c_data;
		u8 databuf[1];	
		
		memset(databuf, 0, sizeof(u8)*1);	 
	
		if(NULL == obj)
		{
			GSE_ERR("i2c data obj is null!!\n");
			return 0;
		}
		
		if(hwmsen_read_block(obj->client, i2c_dev_reg, databuf, 0x01))
		{
			GSE_ERR("read power ctl register err!\n");
			return KXTF9_ERR_I2C;
		}

		printk("i2c_dev_reg=0x%2x  data=0x%2x \n", i2c_dev_reg,databuf[0]);
	
		return 0;
		
}


static DRIVER_ATTR(i2c,      S_IWUSR | S_IRUGO, show_register_value,         store_register_value);
static DRIVER_ATTR(register,      S_IWUSR | S_IRUGO, show_register,         store_register);


/*----------------------------------------------------------------------------*/
static struct driver_attribute *kxtf9_attr_list[] = {
	&driver_attr_chipinfo,     /*chip information*/
	&driver_attr_sensordata,   /*dump sensor data*/
	&driver_attr_cali,         /*show calibration data*/
	&driver_attr_self,         /*self test demo*/
	&driver_attr_selftest,     /*self control: 0: disable, 1: enable*/
	&driver_attr_firlen,       /*filter length: 0: disable, others: enable*/
	&driver_attr_trace,        /*trace log*/
	&driver_attr_status,
	&driver_attr_powerstatus,
	&driver_attr_register,
	&driver_attr_i2c,
#ifdef LENOVO_DTAP_FEATURE
	&driver_attr_lvenable,
	&driver_attr_lvthd
#endif
};
/*----------------------------------------------------------------------------*/
static int kxtf9_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(kxtf9_attr_list)/sizeof(kxtf9_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if(err = driver_create_file(driver, kxtf9_attr_list[idx]))
		{            
			GSE_ERR("driver_create_file (%s) = %d\n", kxtf9_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
static int kxtf9_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(kxtf9_attr_list)/sizeof(kxtf9_attr_list[0]));

	if(driver == NULL)
	{
		return -EINVAL;
	}
	

	for(idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, kxtf9_attr_list[idx]);
	}
	

	return err;
}

/*----------------------------------------------------------------------------*/
int gsensor_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value, sample_delay;	
	struct kxtf9_i2c_data *priv = (struct kxtf9_i2c_data*)self;
	hwm_sensor_data* gsensor_data;
	char buff[KXTF9_BUFSIZE];
	
	//GSE_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				if(value <= 5)
				{
					sample_delay = KXTF9_BW_200HZ;
				}
				else if(value <= 10)
				{
					sample_delay = KXTF9_BW_100HZ;
				}
				else
				{
					sample_delay = KXTF9_BW_50HZ;
				}
				
				err = KXTF9_SetBWRate(priv->client, sample_delay);
				if(err != KXTF9_SUCCESS ) //0x2C->BW=100Hz
				{
					GSE_ERR("Set delay parameter error!\n");
				}

				if(value >= 50)
				{
					atomic_set(&priv->filter, 0);
				}
				else
				{	
				#if defined(CONFIG_KXTF9_LOWPASS)
					priv->fir.num = 0;
					priv->fir.idx = 0;
					priv->fir.sum[KXTF9_AXIS_X] = 0;
					priv->fir.sum[KXTF9_AXIS_Y] = 0;
					priv->fir.sum[KXTF9_AXIS_Z] = 0;
					atomic_set(&priv->filter, 1);
				#endif
				}
			}
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
#ifndef LENOVO_DTAP_FEATURE
				value = *(int *)buff_in;
				if(((value == 0) && (sensor_power == false)) ||((value == 1) && (sensor_power == true)))
				{
					GSE_LOG("Gsensor device have updated!\n");
				}
				else
				{
					err = KXTF9_SetPowerMode( priv->client, !sensor_power);
				}
#else
				int old;
				value = *(int *)buff_in;
				old = atomic_read(&priv->gs_en);
				atomic_set(&priv->gs_en, value);
				GSE_LOG("gsensor en form %d to %d!\n", old, value);
				if(old == value)
				{
					GSE_LOG("Gsensor device have updated!\n");
				}
				else
				{
					if(0 != atomic_read(&priv->suspend))
					{
						/* system have suspended, nothing to do */
					}
					else
					{
						err = KXTF9_SetPowerMode( priv->client, !sensor_power);
					}
				}
#endif
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				GSE_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				gsensor_data = (hwm_sensor_data *)buff_out;
				KXTF9_ReadSensorData(priv->client, buff, KXTF9_BUFSIZE);
				sscanf(buff, "%x %x %x", &gsensor_data->values[0], 
					&gsensor_data->values[1], &gsensor_data->values[2]);				
				gsensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;				
				gsensor_data->value_divide = 1000;
			}
			break;
		default:
			GSE_ERR("gsensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}

/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int kxtf9_open(struct inode *inode, struct file *file)
{
	file->private_data = kxtf9_i2c_client;

	if(file->private_data == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int kxtf9_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
//static int kxtf9_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
//       unsigned long arg)
static long kxtf9_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct kxtf9_i2c_data *obj = (struct kxtf9_i2c_data*)i2c_get_clientdata(client);	
	char strbuf[KXTF9_BUFSIZE];
	void __user *data;
	SENSOR_DATA sensor_data;
	long err = 0;
	int cali[3];

	//GSE_FUN(f);
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		GSE_ERR("access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case GSENSOR_IOCTL_INIT:
			kxtf9_init_client(client, 0);			
			break;

		case GSENSOR_IOCTL_READ_CHIPINFO:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			
			KXTF9_ReadChipInfo(client, strbuf, KXTF9_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}				 
			break;	  

		case GSENSOR_IOCTL_READ_SENSORDATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			
			KXTF9_ReadSensorData(client, strbuf, KXTF9_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}				 
			break;

		case GSENSOR_IOCTL_READ_GAIN:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &gsensor_gain, sizeof(GSENSOR_VECTOR3D)))
			{
				err = -EFAULT;
				break;
			}				 
			break;

		case GSENSOR_IOCTL_READ_RAW_DATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			KXTF9_ReadRawData(client, strbuf);
			if(copy_to_user(data, &strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}
			break;	  

		case GSENSOR_IOCTL_SET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(copy_from_user(&sensor_data, data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;	  
			}
			if(atomic_read(&obj->suspend))
			{
				GSE_ERR("Perform calibration in suspend state!!\n");
				err = -EINVAL;
			}
			else
			{
				cali[KXTF9_AXIS_X] = sensor_data.x * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[KXTF9_AXIS_Y] = sensor_data.y * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[KXTF9_AXIS_Z] = sensor_data.z * obj->reso->sensitivity / GRAVITY_EARTH_1000;			  
				err = KXTF9_WriteCalibration(client, cali);			 
			}
			break;

		case GSENSOR_IOCTL_CLR_CALI:
			err = KXTF9_ResetCalibration(client);
			break;

		case GSENSOR_IOCTL_GET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(err = KXTF9_ReadCalibration(client, cali))
			{
				break;
			}
			
			sensor_data.x = cali[KXTF9_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.y = cali[KXTF9_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.z = cali[KXTF9_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			if(copy_to_user(data, &sensor_data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;
			}		
			break;
		

		default:
			GSE_ERR("unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;
			
	}

	return err;
}


/*----------------------------------------------------------------------------*/
static struct file_operations kxtf9_fops = {
	.owner = THIS_MODULE,
	.open = kxtf9_open,
	.release = kxtf9_release,
	//.ioctl = kxtf9_ioctl,
	.unlocked_ioctl = kxtf9_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice kxtf9_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &kxtf9_fops,
};
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_HAS_EARLYSUSPEND
/*----------------------------------------------------------------------------*/
static int kxtf9_suspend(struct i2c_client *client, pm_message_t msg) 
{
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);    
	int err = 0;
	GSE_FUN();    

	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(obj == NULL)
		{
			GSE_ERR("null pointer!!\n");
			return -EINVAL;
		}
		atomic_set(&obj->suspend, 1);
		if(err = KXTF9_SetPowerMode(obj->client, false))
		{
			GSE_ERR("write power control fail!!\n");
			return;
		}

		sensor_power = false;      
		KXTF9_power(obj->hw, 0);
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int kxtf9_resume(struct i2c_client *client)
{
	struct kxtf9_i2c_data *obj = i2c_get_clientdata(client);        
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}

	KXTF9_power(obj->hw, 1);
	if(err = kxtf9_init_client(client, 0))
	{
		GSE_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->suspend, 0);

	return 0;
}
/*----------------------------------------------------------------------------*/
#else /*CONFIG_HAS_EARLY_SUSPEND is defined*/
/*----------------------------------------------------------------------------*/
static void kxtf9_early_suspend(struct early_suspend *h) 
{
	struct kxtf9_i2c_data *obj = container_of(h, struct kxtf9_i2c_data, early_drv);   
	int err;
	GSE_FUN();    

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}
	atomic_set(&obj->suspend, 1); 
	if(err = KXTF9_SetPowerMode(obj->client, false))
	{
		GSE_ERR("write power control fail!!\n");
		return;
	}
#ifndef LENOVO_DTAP_FEATURE
	sensor_power = false;
	
	KXTF9_power(obj->hw, 0);
#else
	if(0 == atomic_read(&obj->lv_en))
	{
		KXTF9_power(obj->hw, 0);
	}
	else
	{
		kx_lv_enable(true);
	}
#endif
}
/*----------------------------------------------------------------------------*/
static void kxtf9_late_resume(struct early_suspend *h)
{
	struct kxtf9_i2c_data *obj = container_of(h, struct kxtf9_i2c_data, early_drv);         
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}

#ifndef LENOVO_DTAP_FEATURE
	KXTF9_power(obj->hw, 1);
#else
	if(0 == atomic_read(&obj->lv_en))
	{
		KXTF9_power(obj->hw, 1);
	}
	else
	{
		kx_lv_enable(false);
	}
#endif
	if(err = kxtf9_init_client(obj->client, 0))
	{
		GSE_ERR("initialize client fail!!\n");
		return;        
	}
	atomic_set(&obj->suspend, 0);
#ifdef LENOVO_DTAP_FEATURE
	if(atomic_read(&obj->gs_en))
	{
		KXTF9_SetPowerMode(obj->client, true);
	}	
#endif
}
/*----------------------------------------------------------------------------*/
#endif /*CONFIG_HAS_EARLYSUSPEND*/
/*----------------------------------------------------------------------------*/
static int kxtf9_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) 
{    
	strcpy(info->type, KXTF9_DEV_NAME);
	return 0;
}

/*----------------------------------------------------------------------------*/
static int kxtf9_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_client *new_client;
	struct kxtf9_i2c_data *obj;
	struct hwmsen_object sobj;
	int err = 0;
	GSE_FUN();

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	
	memset(obj, 0, sizeof(struct kxtf9_i2c_data));

	obj->hw = get_cust_acc_hw();
	
	if(err = hwmsen_get_convert(obj->hw->direction, &obj->cvt))
	{
		GSE_ERR("invalid direction: %d\n", obj->hw->direction);
		goto exit;
	}

	obj_i2c_data = obj;
	obj->client = client;
	new_client = obj->client;
	i2c_set_clientdata(new_client,obj);
	
	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);
#ifdef LENOVO_DTAP_FEATURE
	atomic_set(&obj->gs_en, 0);
	atomic_set(&obj->lv_en, 0);

	INIT_WORK(&obj->eint_work, kx_eint_work);
	kx_request_input_dev(obj);
#endif
	
#ifdef CONFIG_KXTF9_LOWPASS
	if(obj->hw->firlen > C_MAX_FIR_LENGTH)
	{
		atomic_set(&obj->firlen, C_MAX_FIR_LENGTH);
	}	
	else
	{
		atomic_set(&obj->firlen, obj->hw->firlen);
	}
	
	if(atomic_read(&obj->firlen) > 0)
	{
		atomic_set(&obj->fir_en, 1);
	}
	
#endif

	kxtf9_i2c_client = new_client;	

	if(err = kxtf9_init_client(new_client, 1))
	{
		goto exit_init_failed;
	}
#ifdef LENOVO_DTAP_FEATURE
	kx_setup_eint(new_client);	
#endif

	if(err = misc_register(&kxtf9_device))
	{
		GSE_ERR("kxtf9_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	if(err = kxtf9_create_attr(&kxtf9_gsensor_driver.driver))
	{
		GSE_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}

	sobj.self = obj;
    sobj.polling = 1;
    sobj.sensor_operate = gsensor_operate;
	if(err = hwmsen_attach(ID_ACCELEROMETER, &sobj))
	{
		GSE_ERR("attach fail = %d\n", err);
		goto exit_kfree;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = kxtf9_early_suspend,
	obj->early_drv.resume   = kxtf9_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif 

	GSE_LOG("%s: OK\n", __func__);    
	return 0;

	exit_create_attr_failed:
	misc_deregister(&kxtf9_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(new_client);
	exit_kfree:
	kfree(obj);
	exit:
	GSE_ERR("%s: err = %d\n", __func__, err);        
	return err;
}

/*----------------------------------------------------------------------------*/
static int kxtf9_i2c_remove(struct i2c_client *client)
{
	int err = 0;	
	
	if(err = kxtf9_delete_attr(&kxtf9_gsensor_driver.driver))
	{
		GSE_ERR("kxtf9_delete_attr fail: %d\n", err);
	}
	
	if(err = misc_deregister(&kxtf9_device))
	{
		GSE_ERR("misc_deregister fail: %d\n", err);
	}

	if(err = hwmsen_detach(ID_ACCELEROMETER))
	    

	kxtf9_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}
/*----------------------------------------------------------------------------*/
static int kxtf9_probe(struct platform_device *pdev) 
{
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_FUN();

	KXTF9_power(hw, 1);
	//kxtf9_force[0] = hw->i2c_num;
	if(i2c_add_driver(&kxtf9_i2c_driver))
	{
		GSE_ERR("add driver error\n");
		return -1;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static int kxtf9_remove(struct platform_device *pdev)
{
    struct acc_hw *hw = get_cust_acc_hw();

    GSE_FUN();    
    KXTF9_power(hw, 0);    
    i2c_del_driver(&kxtf9_i2c_driver);
    return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver kxtf9_gsensor_driver = {
	.probe      = kxtf9_probe,
	.remove     = kxtf9_remove,    
	.driver     = {
		.name  = "gsensor",
//		.owner = THIS_MODULE,
	}
};

/*----------------------------------------------------------------------------*/
static int __init kxtf9_init(void)
{
	GSE_FUN();
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_LOG("%s: i2c_number=%d\n", __func__,hw->i2c_num); 
	i2c_register_board_info(hw->i2c_num, &i2c_kxtf9, 1);
	if(platform_driver_register(&kxtf9_gsensor_driver))
	{
		GSE_ERR("failed to register driver");
		return -ENODEV;
	}
	return 0;    
}
/*----------------------------------------------------------------------------*/
static void __exit kxtf9_exit(void)
{
	GSE_FUN();
	platform_driver_unregister(&kxtf9_gsensor_driver);
}
/*----------------------------------------------------------------------------*/
module_init(kxtf9_init);
module_exit(kxtf9_exit);
/*----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KXTF9 I2C driver");
MODULE_AUTHOR("Dexiang.Liu@mediatek.com");
