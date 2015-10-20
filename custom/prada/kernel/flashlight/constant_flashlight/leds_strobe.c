#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include "kd_flashlight.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include "kd_camera_hw.h"
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/xlog.h>
#include <mach/upmu_common.h>


#if defined(LENOVO_FLASH_ADP1650)
#include <linux/i2c.h>
#include <linux/leds.h>
#include "leds-adp1650.h"
#endif
/******************************************************************************
 * Debug configuration
******************************************************************************/
// availible parameter
// ANDROID_LOG_ASSERT
// ANDROID_LOG_ERROR
// ANDROID_LOG_WARNING
// ANDROID_LOG_INFO
// ANDROID_LOG_DEBUG
// ANDROID_LOG_VERBOSE
#define TAG_NAME "leds_strobe.c"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    xlog_printk(ANDROID_LOG_DEBUG  , TAG_NAME, KERN_INFO  "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_WARN(fmt, arg...)        xlog_printk(ANDROID_LOG_WARNING, TAG_NAME, KERN_WARNING  "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_NOTICE(fmt, arg...)      xlog_printk(ANDROID_LOG_DEBUG  , TAG_NAME, KERN_NOTICE  "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_INFO(fmt, arg...)        xlog_printk(ANDROID_LOG_INFO   , TAG_NAME, KERN_INFO  "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_TRC_FUNC(f)              xlog_printk(ANDROID_LOG_DEBUG  , TAG_NAME,  "<%s>\n", __FUNCTION__);
#define PK_TRC_VERBOSE(fmt, arg...) xlog_printk(ANDROID_LOG_VERBOSE, TAG_NAME,  fmt, ##arg)
#define PK_ERROR(fmt, arg...)       xlog_printk(ANDROID_LOG_ERROR  , TAG_NAME, KERN_ERR "%s: " fmt, __FUNCTION__ ,##arg)


#define DEBUG_LEDS_STROBE
#ifdef  DEBUG_LEDS_STROBE
	#define PK_DBG PK_DBG_FUNC
	#define PK_VER PK_TRC_VERBOSE
	#define PK_ERR PK_ERROR
#else
	#define PK_DBG(a,...)
	#define PK_VER(a,...)
	#define PK_ERR(a,...)
#endif
#if defined(LENOVO_FLASH_ADP1650)
#define EN_PIN     137  //HWEN
#define FLASH_PIN  136  //STROBE
#define TORCH_PIN  134   //TX2
#elif defined(LENOVO_FLASH_RT9387)
#define FLASH_PIN GPIO_CAMERA_FLASH_EN_PIN
#define TORCH_PIN GPIO_CAMERA_FLASH_MODE_PIN
#endif
/******************************************************************************
 * local variables
******************************************************************************/
static DEFINE_SPINLOCK(g_strobeSMPLock);
static u32 strobe_Res = 0;
static BOOL g_strobe_On = 0;
static int g_duty=-1;
static int g_step=-1;
static int g_timeOutTimeMs=0;

static struct work_struct workTimeOut;
/*****************************************************************************
Functions
*****************************************************************************/
static void work_timeOutFunc(struct work_struct *data);
/* for seine exterl flash driver ic support , temp modfy  by liaoxl.lenovo 12.21.2012  start*/


/* device name and major number */
#define FLASHLIGHT_DEVNAME            "RT9387"
struct flash_chip_data {
	struct led_classdev cdev_flash;
	struct led_classdev cdev_torch;

	struct mutex lock;    

	int mode;
	int torch_level;
};


static struct flash_chip_data chipconf;

#if defined(LENOVO_FLASH_ADP1650)
/***********************ADI ADP1650 flash driver chip************************************************************/
//#define ADP1650_ENVM_PIN   107  //ENVM
static struct i2c_client *adp1650_i2c_client = NULL;


struct adp1650_chip_data {
	struct i2c_client *client;

	struct led_classdev cdev_flash;
	struct led_classdev cdev_torch;
	struct led_classdev cdev_indicator;

	struct adp1650_platform_data *pdata;
	struct mutex lock;    

	u8 last_flag; 
	u8 no_pdata; 
};

/* i2c access*/
static int adp1650_read_reg(struct i2c_client *client, u8 reg,u8 *val)
{
	int ret;
	struct adp1650_chip_data *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->lock);
	ret = i2c_smbus_read_byte_data(client, reg);
	mutex_unlock(&chip->lock);

	if (ret < 0) {
		PK_ERR("failed reading at 0x%02x error %d\n",reg, ret);
		return ret;
	}
	*val = ret&0xff; 

	return 0;
}

static int adp1650_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	int ret=0;
	struct adp1650_chip_data *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->lock);
	ret =  i2c_smbus_write_byte_data(client, reg, val);
	mutex_unlock(&chip->lock);
	
	if (ret < 0)
		PK_ERR("failed writting at 0x%02x\n", reg);
	return ret;
}

static int adp1650_chip_init(struct adp1650_chip_data *chip)
{
	int ret =0;
	struct i2c_client *client = chip->client;
	struct adp1650_platform_data *pdata = chip->pdata; 
	PK_DBG("adp1650_chip_init start--->.\n");

    mt_set_gpio_mode(EN_PIN, 0);
    mt_set_gpio_dir(EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(EN_PIN, GPIO_OUT_ONE);

    mt_set_gpio_mode(FLASH_PIN, 0);
    mt_set_gpio_dir(FLASH_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(FLASH_PIN, GPIO_OUT_ZERO);

    mt_set_gpio_mode(TORCH_PIN, 0);
    mt_set_gpio_dir(TORCH_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(TORCH_PIN, GPIO_OUT_ZERO);
	mdelay(10);

	do {
		ret = adp1650_write_reg(client, 0x02, 0x18);
		if(ret < 0)
			break;
		ret = adp1650_write_reg(client, 0x03, 0x4D);
		if(ret < 0)
			break;
		ret = adp1650_write_reg(client, 0x04, 0xA4);
			break;
	}while(0);

	if(ret >= 0)
	{
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	PK_DBG("adp1650_chip_init end ret=%d.\n", ret);

	return ret;
}

static int adp1650_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct adp1650_chip_data *chip;
	struct adp1650_platform_data *pdata = client->dev.platform_data;

	int err = -1;

	PK_DBG("adp1650_probe start--->.\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		printk(KERN_ERR  "ADP1650 i2c functionality check fail.\n");
		return err; 
	}

	chip = kzalloc(sizeof(struct adp1650_chip_data), GFP_KERNEL);
	chip->client = client;	

	mutex_init(&chip->lock);	
	i2c_set_clientdata(client, chip);

	if(pdata == NULL){ //values are set to Zero. 
		PK_ERR("ADP1650 Platform data does not exist\n");
		pdata = kzalloc(sizeof(struct adp1650_platform_data),GFP_KERNEL);
		chip->pdata  = pdata;
		chip->no_pdata = 1; 
	}
	
	chip->pdata  = pdata;
	if(adp1650_chip_init(chip)<0)
		goto err_chip_init;

	adp1650_i2c_client = client;
	PK_DBG("ADP1650 Initializing is done \n");

	return 0;

err_chip_init:	
	i2c_set_clientdata(client, NULL);
	kfree(chip);
	PK_ERR("ADP1650 probe is failed \n");
	return -ENODEV;
}

static int adp1650_remove(struct i2c_client *client)
{
	struct adp1650_chip_data *chip = i2c_get_clientdata(client);

    if(chip->no_pdata)
		kfree(chip->pdata);
	kfree(chip);
	return 0;
}


#define ADP1650_NAME "leds-ADP1650"
static const struct i2c_device_id adp1650_id[] = {
	{ADP1650_NAME, 0},
	{}
};

static struct i2c_driver adp1650_i2c_driver = {
	.driver = {
		.name  = ADP1650_NAME,
	},
	.probe	= adp1650_probe,
	.remove   = __devexit_p(adp1650_remove),
	.id_table = adp1650_id,
};

struct adp1650_platform_data adp1650_pdata = {0, 0, 0, 0, 0};
static struct i2c_board_info __initdata i2c_ADP1650={ I2C_BOARD_INFO(ADP1650_NAME, 0x30), \
													.platform_data = &adp1650_pdata,};

static int __init adp1650_init(void)
{
	printk(KERN_INFO "%s\n","adp1650_init");
	i2c_register_board_info(3, &i2c_ADP1650, 1);

	return i2c_add_driver(&adp1650_i2c_driver);
}

static void __exit adp1650_exit(void)
{
	i2c_del_driver(&adp1650_i2c_driver);
}


module_init(adp1650_init);
module_exit(adp1650_exit);

MODULE_DESCRIPTION("Flash Lighting driver for adp1650");
MODULE_AUTHOR("zhangjiano <zhangjiano@lenovo.com>");
MODULE_LICENSE("GPL v2");

#define MULTI_FLASH

#if defined(MULTI_FLASH)
#define TORCH_BRIGHTNESS 0
#define FLASH_BRIGHTNESS 15
//extern void IMX135MIPI_strobe_control(bool on);
#else
#define TORCH_BRIGHTNESS 1
#define FLASH_BRIGHTNESS 5
#endif
#if defined(MULTI_FLASH)
int FL_enable(void)
{
	struct flash_chip_data *chip = &chipconf;
	int brightness = 0;
	u8 tmp4,tmp5;
	PK_DBG("FL_enable g_duty=%d\n",g_duty);
	#if 0
	upmu_set_rg_bst_drv_1m_ck_pdn(0);
	upmu_set_flash_en(1);
	#else
	if(g_duty == TORCH_BRIGHTNESS)//torch
	{
		adp1650_write_reg(adp1650_i2c_client, 0x03, 0x04);
		adp1650_write_reg(adp1650_i2c_client, 0x04, 0xAC); //75ma torch output_en'
		udelay(50);
		mt_set_gpio_out(FLASH_PIN, GPIO_OUT_ZERO);
		//IMX135MIPI_strobe_control(0);

		mt_set_gpio_out(TORCH_PIN, GPIO_OUT_ONE);
		
		PK_DBG("FL_enable  torch brightness=%d\n",brightness);
		
		chip->torch_level = brightness;
		chip->mode = 1;
	}
	else if((g_duty >= (TORCH_BRIGHTNESS+1))&&(g_duty <= FLASH_BRIGHTNESS))
	{
		brightness =g_duty<<3;
		PK_DBG("FL_enable flash brightness=%d\n",brightness);
		adp1650_write_reg(adp1650_i2c_client, 0x03, brightness&0xF8); //75ma torch output_en'
		adp1650_write_reg(adp1650_i2c_client, 0x04, 0xAF); //750ma flash output_en
		mt_set_gpio_out(FLASH_PIN, GPIO_OUT_ONE);
		//IMX135MIPI_strobe_control(1);
		
		mt_set_gpio_out(TORCH_PIN, GPIO_OUT_ZERO);
		//udelay(100);
		adp1650_read_reg(adp1650_i2c_client, 0x05, &tmp5);
        	adp1650_read_reg(adp1650_i2c_client, 0x04, &tmp4);
		PK_DBG("FL_enable tmp4=%d,tmp5=%d\n",tmp4,tmp5);

		chip->torch_level = 0;
		chip->mode = 2;
	}
	#endif
    return 0;
}
#else
int FL_enable(void)
{
	struct flash_chip_data *chip = &chipconf;
	int brightness = 0;
	u8 tmp4,tmp5;
	PK_DBG("FL_enable g_duty=%d\n",g_duty);
	#if 0
	upmu_set_rg_bst_drv_1m_ck_pdn(0);
	upmu_set_flash_en(1);
	#else
	if(g_duty == TORCH_BRIGHTNESS)//torch
	{
		adp1650_write_reg(adp1650_i2c_client, 0x03, 0x05);
		adp1650_write_reg(adp1650_i2c_client, 0x04, 0xAC); //75ma torch output_en'
		udelay(50);
		mt_set_gpio_out(FLASH_PIN, GPIO_OUT_ZERO);
		mt_set_gpio_out(TORCH_PIN, GPIO_OUT_ONE);
		
		PK_DBG("FL_enable  torch brightness=%d\n",brightness);
		
		chip->torch_level = brightness;
		chip->mode = 1;
	}
	else if((g_duty > TORCH_BRIGHTNESS)||(g_duty <= FLASH_BRIGHTNESS))
	{
		brightness =0xE<<3;
		PK_DBG("FL_enable flash brightness=%d\n",brightness);
		adp1650_write_reg(adp1650_i2c_client, 0x03, brightness&0xF8); //75ma torch output_en'
		adp1650_write_reg(adp1650_i2c_client, 0x04, 0xAF); //750ma flash output_en
		udelay(50);
		mt_set_gpio_out(FLASH_PIN, GPIO_OUT_ONE);
		mt_set_gpio_out(TORCH_PIN, GPIO_OUT_ZERO);
		//udelay(100);
		adp1650_read_reg(adp1650_i2c_client, 0x05, &tmp5);
        	adp1650_read_reg(adp1650_i2c_client, 0x04, &tmp4);
		PK_DBG("FL_enable tmp4=%d,tmp5=%d\n",tmp4,tmp5);

		chip->torch_level = 0;
		chip->mode = 2;
	}
	#endif
    return 0;
}
#endif
int FL_disable(void)
{
	struct flash_chip_data *chip = &chipconf;
	PK_DBG("FL_disable g_duty=%d\n",g_duty);
	//IMX135MIPI_strobe_control(0);
	mt_set_gpio_out(FLASH_PIN, GPIO_OUT_ZERO);
	mt_set_gpio_out(TORCH_PIN, GPIO_OUT_ZERO);
	udelay(50);
	chip->torch_level = 0;
	chip->mode = 0;

	//upmu_set_flash_en(0);
	//upmu_set_rg_bst_drv_1m_ck_pdn(1);

    return 0;
}

int FL_dim_duty(kal_uint32 duty)
{
	PK_DBG("FL_dim_duty\n");
	//upmu_set_flash_dim_duty(duty);
    return 0;
}

int FL_step(kal_uint32 step)
{
	PK_DBG("FL_step\n");
	//upmu_set_flash_sel(step);
    return 0;
}

int FL_init(void)
{
	PK_DBG("FL_init\n");
	#if 0
	upmu_set_flash_dim_duty(0);
	upmu_set_flash_sel(0);
	FL_disable();

	#else
	if(adp1650_i2c_client == NULL)
    {
    	return 0;
    }
	adp1650_write_reg(adp1650_i2c_client, 0x04, 0xAC); //75ma torch output_en
	#endif

	INIT_WORK(&workTimeOut, work_timeOutFunc);

    return 0;
}


int FL_uninit(void)
{
	PK_DBG("FL_uninit\n");
	FL_disable();
    return 0;
}

#elif defined(LENOVO_FLASH_RT9387)


int FL_enable(void)
{
	int i, cc;
	struct flash_chip_data *chip = &chipconf;
	int brightness = 6;
	PK_ERR("FL_enable\n");
	if(g_duty == 1)
	{
		switch(chip->mode)
		{
			case 0:
				mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ZERO);
				mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ONE);
				udelay(50);

				if(brightness >= 16)
				{
					/* keep current */
					brightness = 16;
				}
				else
				{
					cc = 16 - brightness;

					for(i = 0; i < cc; i++)
					{
						udelay(15);
						mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
						udelay(15);
						mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ONE);
					}
				}
				chip->torch_level = brightness;
				chip->mode = 1;
				PK_ERR("[flashchip] init level=%d cc=%d\n", brightness, cc);
				break;

			case 1:
				if(brightness >= 16)
				{
					brightness = 16;
				}
				if(brightness == chip->torch_level)
				{
					/* keep current */
					cc = 0;
				}
				else if(brightness < chip->torch_level)
				{
					cc = chip->torch_level - brightness;
				}
				else /* (brightness > chip->torch_level) */
				{
					cc = chip->torch_level + 16 - brightness;
				}
				for(i = 0; i < cc; i++)
				{
					udelay(15);
					mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
					udelay(15);
					mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ONE);
				}
				PK_ERR("[flashchip] old=%d level=%d cc=%d\n", chip->torch_level, brightness, cc);
				chip->torch_level = brightness;
				break;
				
			case 2:
				/* not support now */
				break;

			default:
				break;
		}
	}
	else if((g_duty == 2)||(g_duty == 0))
	{
/*lenovo-sw xuegb1 2013022 -for lenovoTorch light for long time begin*/		
		mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
		mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ONE);
/*lenovo-sw xuegb1 2013022 -for lenovoTorch light for long time end */
		//mdelay(4);
		chip->torch_level = 0;
		chip->mode = 2;
		PK_ERR("[flashchip] flash level = 1\n");
	}

    return 0;
}

int FL_disable(void)
{
	struct flash_chip_data *chip = &chipconf;
	PK_ERR("FL_disable\n");

	mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ZERO);
	mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
	mdelay(4);
	chip->torch_level = 0;
	chip->mode = 0;
    return 0;
}

int FL_dim_duty(kal_uint32 duty)
{

    return 0;
}

int FL_step(kal_uint32 step)
{
    return 0;
}

int FL_init(void)
{
	if(mt_set_gpio_mode(FLASH_PIN,GPIO_MODE_00)){PK_DBG("[CAMERA flash] set gpio ENF mode failed!! \n");}		 		
	if(mt_set_gpio_dir(FLASH_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA flash] set gpio ENF dir failed!! \n");}			
	if(mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA flash] set gpio ENF failed!! \n");} 			
	if(mt_set_gpio_mode(TORCH_PIN,GPIO_MODE_00)){PK_DBG("[CAMERA flash] set gpio ENT mode failed!! \n");}		 		
	if(mt_set_gpio_dir(TORCH_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA flash] set gpio ENT dir failed!! \n");}			
	if(mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA flash] set gpio ENT failed!! \n");} 			

	FL_disable();
	INIT_WORK(&workTimeOut, work_timeOutFunc);
    return 0;
}


int FL_uninit(void)
{
	FL_disable();
    return 0;
}

#else
int FL_enable(void)
{
	upmu_set_rg_bst_drv_1m_ck_pdn(0);
	upmu_set_flash_en(1);
    return 0;
}

int FL_disable(void)
{

	upmu_set_flash_en(0);
	//upmu_set_rg_bst_drv_1m_ck_pdn(1);

    return 0;
}

int FL_dim_duty(kal_uint32 duty)
{
	upmu_set_flash_dim_duty(duty);
    return 0;
}

int FL_step(kal_uint32 step)
{
	int sTab[8]={0,2,4,6,9,11,13,15};
	upmu_set_flash_sel(sTab[step]);
    return 0;
}

int FL_init(void)
{
	upmu_set_flash_dim_duty(0);
	upmu_set_flash_sel(0);
	FL_disable();
	INIT_WORK(&workTimeOut, work_timeOutFunc);
    return 0;
}


int FL_uninit(void)
{
	FL_disable();
    return 0;
}
#endif
/* for seine exterl flash driver ic support , temp modfy  by liaoxl.lenovo 12.21.2012 end*/

/*****************************************************************************
User interface
*****************************************************************************/


static void work_timeOutFunc(struct work_struct *data)
{
	FL_disable();
    PK_DBG("ledTimeOut_callback\n");
    //printk(KERN_ALERT "work handler function./n");
}
enum hrtimer_restart ledTimeOutCallback(struct hrtimer *timer)
{
	PK_DBG("ledTimeOut_callback\n");
	schedule_work(&workTimeOut);

    return HRTIMER_NORESTART;
}
static struct hrtimer g_timeOutTimer;
void timerInit(void)
{
	g_timeOutTimeMs=1000; //1s
	hrtimer_init( &g_timeOutTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	g_timeOutTimer.function=ledTimeOutCallback;

}



static int constant_flashlight_ioctl(MUINT32 cmd, MUINT32 arg)
{
	int i4RetValue = 0;
	int ior;
	int iow;
	int iowr;
	ior = _IOR(FLASHLIGHT_MAGIC,0, int);
	iow = _IOW(FLASHLIGHT_MAGIC,0, int);
	iowr = _IOWR(FLASHLIGHT_MAGIC,0, int);
	PK_DBG("constant_flashlight_ioctl() line=%d cmd=%d, ior=%d, iow=%d iowr=%d arg=%d\n",__LINE__, cmd, ior, iow, iowr, arg);
	PK_DBG("constant_flashlight_ioctl() line=%d cmd-ior=%d, cmd-iow=%d cmd-iowr=%d arg=%d\n",__LINE__, cmd-ior, cmd-iow, cmd-iowr, arg);
    switch(cmd)
    {

		case FLASH_IOC_SET_TIME_OUT_TIME_MS:
			PK_DBG("FLASH_IOC_SET_TIME_OUT_TIME_MS: %d\n",arg);
			g_timeOutTimeMs=arg;
		break;


    	case FLASH_IOC_SET_DUTY :
    		PK_DBG("FLASHLIGHT_DUTY: %d\n",arg);
    		g_duty=arg;
    		FL_dim_duty(arg);
    		break;


    	case FLASH_IOC_SET_STEP:
    		PK_DBG("FLASH_IOC_SET_STEP: %d\n",arg);
    		g_step=arg;
    		FL_step(arg);
    		break;

    	case FLASH_IOC_SET_ONOFF :
    		PK_DBG("FLASHLIGHT_ONOFF: %d\n",arg);
    		if(arg==1)
    		{
				if(g_timeOutTimeMs!=0)
	            {
	            	ktime_t ktime;
					ktime = ktime_set( 0, g_timeOutTimeMs*1000000 );
					hrtimer_start( &g_timeOutTimer, ktime, HRTIMER_MODE_REL );
	            }
    			FL_enable();
    			g_strobe_On=1;
    		}
    		else
    		{
    			FL_disable();
				hrtimer_cancel( &g_timeOutTimer );
				g_strobe_On=0;
    		}
    		break;
		default :
    		PK_DBG(" No such command \n");
    		i4RetValue = -EPERM;
    		break;
    }
    return i4RetValue;
}




static int constant_flashlight_open(void *pArg)
{
    int i4RetValue = 0;
    PK_DBG("constant_flashlight_open line=%d\n", __LINE__);

	if (0 == strobe_Res)
	{
	    FL_init();
		timerInit();
	}
	spin_lock_irq(&g_strobeSMPLock);

    if(strobe_Res)
    {
        PK_ERR(" busy!\n");
        i4RetValue = -EBUSY;
    }
    else
    {
        strobe_Res += 1;
    }

    spin_unlock_irq(&g_strobeSMPLock);

    return i4RetValue;

}


static int constant_flashlight_release(void *pArg)
{
    PK_DBG(" constant_flashlight_release\n");

    if (strobe_Res)
    {
        spin_lock_irq(&g_strobeSMPLock);

        strobe_Res = 0;

        /* LED On Status */
        g_strobe_On = FALSE;

        spin_unlock_irq(&g_strobeSMPLock);

    	FL_uninit();
    }

    PK_DBG(" Done\n");

    return 0;

}


FLASHLIGHT_FUNCTION_STRUCT	constantFlashlightFunc=
{
	constant_flashlight_open,
	constant_flashlight_release,
	constant_flashlight_ioctl
};


MUINT32 constantFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc)
{
    if (pfFunc != NULL)
    {
        *pfFunc = &constantFlashlightFunc;
    }
    return 0;
}



/* LED flash control for high current capture mode*/
ssize_t strobe_VDIrq(void)
{

    return 0;
}

EXPORT_SYMBOL(strobe_VDIrq);


/***************                   *******************/
/* for seine flash light driver support -- by liaoxl.lenovo 12.27.2012 start */
#if 1


static void chip_torch_brightness_set(struct led_classdev *cdev,
				  enum led_brightness brightness)
{
	int i, cc;
	struct flash_chip_data *chip = &chipconf;
	u8 tmp4,tmp5;
#if defined(LENOVO_FLASH_RT9387)
	if(brightness == 0)
	{
		mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ZERO);
		mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
		mdelay(4);
		chip->torch_level = 0;
		chip->mode = 0;
		PK_ERR("[flashchip] level = 0\n");

		return;
	}

	switch(chip->mode)
	{
		case 0:
			mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ZERO);
			mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ONE);
			udelay(50);

			if(brightness >= 16)
			{
				/* keep current */
				brightness = 16;
			}
			else
			{
				cc = 16 - brightness;

				for(i = 0; i < cc; i++)
				{
					udelay(15);
					mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
					udelay(15);
					mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ONE);
				}
			}
			chip->torch_level = brightness;
			chip->mode = 1;
			PK_ERR("[flashchip] init level=%d cc=%d\n", brightness, cc);
			break;

		case 1:
			if(brightness >= 16)
			{
				brightness = 16;
			}
			if(brightness == chip->torch_level)
			{
				/* keep current */
				cc = 0;
			}
			else if(brightness < chip->torch_level)
			{
				cc = chip->torch_level - brightness;
			}
			else /* (brightness > chip->torch_level) */
			{
				cc = chip->torch_level + 16 - brightness;
			}
			for(i = 0; i < cc; i++)
			{
				udelay(15);
				mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
				udelay(15);
				mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ONE);
			}
			PK_ERR("[flashchip] old=%d level=%d cc=%d\n", chip->torch_level, brightness, cc);
			chip->torch_level = brightness;
			break;
			
		case 2:
			/* not support now */
			break;

		default:
			break;
	}
#elif defined(LENOVO_FLASH_ADP1650)

	if(brightness == 0)
	{
		mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ZERO);
		mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
		udelay(50);
		chip->torch_level = 0;
		chip->mode = 0;
		PK_ERR("[flashchip] PRADA level = 0\n");

		return;
	}

	switch(chip->mode)
	{
		case 0:
			
			
			

			if(brightness >= 7)
			{
				/* keep current */
				brightness = 7;
			}
			else
			{
				cc = brightness;
				adp1650_write_reg(adp1650_i2c_client, 0x04, 0xAC); //75ma torch output_en
				adp1650_write_reg(adp1650_i2c_client, 0x03, cc); //75ma torch output_en

			}
			udelay(100);
			mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ZERO);
			mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ONE);
			adp1650_read_reg(adp1650_i2c_client, 0x05, &tmp5);
        		adp1650_read_reg(adp1650_i2c_client, 0x04, &tmp4);
			chip->torch_level = brightness;
			chip->mode = 1;
			PK_ERR("[flashchip] init PRADA tmp5=%d tmp4=%d\n", tmp5, tmp4);
			PK_ERR("[flashchip] init PRADA level=%d cc=%d\n", brightness, cc);
			break;

		case 1:
			if(brightness >= 7)
			{
				brightness = 7;
			}
			if(brightness == chip->torch_level)
			{
				/* keep current */
				cc = 0;
			}
			else if(brightness < chip->torch_level)
			{
				cc = chip->torch_level - brightness;
			}
			else /* (brightness > chip->torch_level) */
			{
				cc = chip->torch_level - brightness;
			}
			adp1650_write_reg(adp1650_i2c_client, 0x04, 0xAC); //75ma torch output_en
			adp1650_write_reg(adp1650_i2c_client, 0x03, cc); //75ma torch output_en
			PK_ERR("[flashchip] old=%d level=%d cc=%d\n", chip->torch_level, brightness, cc);
			chip->torch_level = brightness;
			break;
			
		case 2:
			/* not support now */
			break;

		default:
			break;
	}
#endif
}


static void chip_flash_brightness_set(struct led_classdev *cdev,
				  enum led_brightness brightness)
{
	struct flash_chip_data *chip = &chipconf;
	u8 tmp4,tmp5;
	PK_ERR("[flashchip] flash brightness = %d\n",brightness);
#if defined(LENOVO_FLASH_RT9387)
	if(brightness == 0)
	{
		mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ZERO);
		mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
		mdelay(4);
		chip->torch_level = 0;
		chip->mode = 0;
		PK_ERR("[flashchip] flash level = 0\n");
	}
	else
	{
		mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ONE);
		mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
		//mdelay(4);
		chip->torch_level = 0;
		chip->mode = 2;
		PK_ERR("[flashchip] flash level = 1\n");
	}
	return;
#elif defined(LENOVO_FLASH_ADP1650)
	if(brightness == 0)
	{
		mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ZERO);
		mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
		mdelay(4);
		chip->torch_level = 0;
		chip->mode = 0;
		PK_ERR("[flashchip] PRADA flash level = 0\n");
	}
	else
	{
		adp1650_write_reg(adp1650_i2c_client, 0x04, 0xAF); //750ma flash output_en
		udelay(50);
		mt_set_gpio_out(FLASH_PIN,GPIO_OUT_ONE);
		mt_set_gpio_out(TORCH_PIN,GPIO_OUT_ZERO);
		adp1650_read_reg(adp1650_i2c_client, 0x05, &tmp5);
        	adp1650_read_reg(adp1650_i2c_client, 0x04, &tmp4);
		
	       //mdelay(4);
        chip->torch_level = 0;
        chip->mode = 2;
        PK_ERR("[flashchip] PRADA flash level = 1,tmp4=%d,tmp5=%d\n",tmp4,tmp5);
	
	}
	return;
#endif
}


static int flashchip_probe(struct platform_device *dev)
{
	struct flash_chip_data *chip;

	PK_ERR("[flashchip_probe] start\n");
	chip = &chipconf;
	chip->mode = 0;
	chip->torch_level = 0;
	mutex_init(&chip->lock);

	//flash
	chip->cdev_flash.name="flash";
	chip->cdev_flash.max_brightness = 1;
	chip->cdev_flash.brightness_set = chip_flash_brightness_set;
	if(led_classdev_register((struct device *)&dev->dev,&chip->cdev_flash)<0)
		goto err_create_flash_file;	
	//torch
	chip->cdev_torch.name="torch";
#if defined(LENOVO_FLASH_RT9387)
	chip->cdev_torch.max_brightness = 16;
#elif defined(LENOVO_FLASH_ADP1650)
	chip->cdev_torch.max_brightness = 7;
#endif
	chip->cdev_torch.brightness_set = chip_torch_brightness_set;
	if(led_classdev_register((struct device *)&dev->dev,&chip->cdev_torch)<0)
		goto err_create_torch_file;

    PK_ERR("[flashchip_probe] Done\n");
    return 0;

err_create_torch_file:
	led_classdev_unregister(&chip->cdev_flash);
err_create_flash_file:
err_chip_init:	
	printk(KERN_ERR "[flashchip_probe] is failed !\n");
	return -ENODEV;



}

static int flashchip_remove(struct platform_device *dev)
{
	struct flash_chip_data *chip = &chipconf;
    PK_DBG("[flashchip_remove] start\n");

	led_classdev_unregister(&chip->cdev_torch);
	led_classdev_unregister(&chip->cdev_flash);


    PK_DBG("[flashchip_remove] Done\n");
    return 0;
}


static struct platform_driver flashchip_platform_driver =
{
    .probe      = flashchip_probe,
    .remove     = flashchip_remove,
    .driver     = {
        .name = FLASHLIGHT_DEVNAME,
		.owner	= THIS_MODULE,
    },
};



static struct platform_device flashchip_platform_device = {
    .name = FLASHLIGHT_DEVNAME,
    .id = 0,
    .dev = {
//    	.platform_data	= &chip,
    }
};

static int __init flashchip_init(void)
{
    int ret = 0;
    PK_DBG("[flashchip_init] start\n");

	ret = platform_device_register (&flashchip_platform_device);
	if (ret) {
        PK_ERR("[flashchip_init] platform_device_register fail\n");
        return ret;
	}

    ret = platform_driver_register(&flashchip_platform_driver);
	if(ret){
		PK_ERR("[flashchip_init] platform_driver_register fail\n");
		return ret;
	}

	PK_DBG("[flashchip_init] done!\n");
    return ret;
}

static void __exit flashchip_exit(void)
{
    PK_DBG("[flashchip_exit] start\n");
    platform_driver_unregister(&flashchip_platform_driver);
    PK_DBG("[flashchip_exit] done!\n");
}

/*****************************************************************************/
module_init(flashchip_init);
module_exit(flashchip_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("liaoxl@lenovo.com>");
MODULE_DESCRIPTION("RT9387 control Driver");

#endif
/* for seine flash light driver support -- by liaoxl.lenovo 12.27.2012 end */


