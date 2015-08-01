/*
 * drivers/leds/lp5560_leds.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * lp5560 leds driver
 *
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/leds-mt65xx.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/slab.h>

#include <cust_leds.h>

#if defined (CONFIG_ARCH_MT6577) || defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)
#include <mach/mt_pwm.h>
#include <mach/mt_gpio.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pmic_feature_api.h>
#include <mach/mt_boot.h>

#elif defined (CONFIG_ARCH_MT6589)
#include <mach/mt_pwm.h>
#include <mach/mt_gpio.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
//#include <mach/mt_pmic_feature_api.h>
//#include <mach/mt_boot.h>

#endif

/****************************************************************************
 * defined
 ***************************************************************************/
#define LED_NAME "blueled"
#define CTRL_PIN 74
#define COMMAND_PULSE_ON_TIME 100 // 1ms    min 15us
#define COMMAND_PULSE_OFF_TIME 100 // 1ms   min 30us
#define CALIBRATION_PULSE_LENGTH 400// 2ms min 350us max 8000us

#define MINIMUM_TRAINING_PULSE_ON_TIME 400//min 200us
#define MINIMUM_TRAINING_PULSE_OFF_TIME 400//min 200us
#define MINMUM_COMMAND_ENTERING_PERIOD 500
#define BLANK_PERIOD  600// 1ms
#define TIMEOUT_COUNTS 127 //127*TCAL


#define LOW_KEEP 1
#define HIGH_KEEP 2


static int debug_enable = 1;

#define LEDS_DEBUG(format, args...) do{ \
	if(debug_enable) \
	{\
		printk(KERN_EMERG format,##args);\
	}\
}while(0)

struct lp5560_leds_priv {
	struct led_classdev cdev;
	struct work_struct work;
	unsigned gpio;
	u8 level;
	
};

struct lp5560_leds_data {
	u8 count;
	u8 delay;
	u8 polarity;
};


enum lp5560_command_mode {
	LED_STAND_BY = 0,
	LED_RUN,
	LED_TRAINING_START,
	LED_TRAINING_END,
	LED_RUN_ONCE,
};


/****************************************************************************
 * local functions
 ***************************************************************************/
static struct hrtimer g_timeOutTimer;

static signed long timeout = 50;
static signed long IsGPIOHigh = 0;
static int total_array = 0;
static int array_num= 0;
static int current_count =0;
static int last_brightness=0;

static struct lp5560_leds_data *curr_data=NULL;
static struct lp5560_leds_data slow_data[]=
{
//reset
{4,1,0},// training start
{1,4,LOW_KEEP},
{6,1,0},//training end
{1,2000,LOW_KEEP},
//begin training
{4,1,0},// training start
{1,4,LOW_KEEP},
{2,4,0},// c
{2,2,0},// i
{9,4,HIGH_KEEP},// R1
{14,4,LOW_KEEP},// ON1
{9,4,HIGH_KEEP},// F1
{19,4,LOW_KEEP},// OFF1
{9,4,HIGH_KEEP},// R2
{14,4,LOW_KEEP},// ON2
{9,4,HIGH_KEEP},// F2
{19,4,LOW_KEEP},// OFF2
{9,4,HIGH_KEEP},// R3
{14,4,LOW_KEEP},// ON3
{9,4,HIGH_KEEP},// F3
{19,4,LOW_KEEP},// OFF3
{6,1,0},//training end
{1,4,HIGH_KEEP},//keep high
};

static struct lp5560_leds_data fast_data[]=
{
//reset
{4,1,0},// training start
{1,4,LOW_KEEP},
{6,1,0},//training end
{1,2000,LOW_KEEP},
//begin training
{4,1,0},// training start
{1,4,LOW_KEEP},
{2,4,0},// c
{2,2,0},// i
{1,4,HIGH_KEEP},// R1
{2,4,LOW_KEEP},// ON1
{1,4,HIGH_KEEP},// F1
{2,4,LOW_KEEP},// OFF1
{1,4,HIGH_KEEP},// R2
{2,4,LOW_KEEP},// ON2
{1,4,HIGH_KEEP},// F2
{2,4,LOW_KEEP},// OFF2
{1,4,HIGH_KEEP},// R3
{2,4,LOW_KEEP},// ON3
{1,4,HIGH_KEEP},// F3
{2,4,LOW_KEEP},// OFF3
{6,1,0},//training end
{1,4,HIGH_KEEP},//keep high
};
struct lp5560_leds_priv *g_lp5560_leds_data[2];

static void lp5560_gpio_set(int setting)
{
	if(setting)
	{
		mt_set_gpio_out(CTRL_PIN, 1);
		IsGPIOHigh = 1;

	}else
	{
		mt_set_gpio_out(CTRL_PIN, 0);
		IsGPIOHigh = 0;
	}
}

static enum hrtimer_restart lp5560_Callback(struct hrtimer *timer)
{
	int delay,count,polarity;
	int i=array_num;

	if((i>=total_array) || (i>timeout))
		return HRTIMER_NORESTART;
	count = curr_data[i].count;
	delay = curr_data[i].delay;
	polarity = curr_data[i].polarity;
//	printk("[JX] %s count=%d i=%d total_array=%d current_count=%d\n",__func__,count,i,total_array,current_count);
	if(current_count>=count)
	{
		current_count=0;
		if(i>=total_array)
			return HRTIMER_NORESTART;
		array_num++;
		hrtimer_add_expires_ns(&g_timeOutTimer, delay*100000); // 100us
		return HRTIMER_RESTART;
	}
	else
	{
		if(polarity==LOW_KEEP) {
			lp5560_gpio_set(0);
		}
		else if(polarity==HIGH_KEEP){
			lp5560_gpio_set(1);
		}
		else{
			if(IsGPIOHigh)
			{
				lp5560_gpio_set(0);
			}
			else
			{
				lp5560_gpio_set(1);
			}
		}
		current_count++;
		hrtimer_add_expires_ns(&g_timeOutTimer, delay*100000); // 100us 
		return HRTIMER_RESTART;
	}

}
static void lp5560_udelay(UINT32 us)
{
	udelay(us);
}

static void lp5560_mdelay(UINT32 ms)
{
	msleep(ms);
}

static void lp5560_slow_flash(void)
{
	ktime_t ktime;
	//ktime = ktime_set( 0, 2*1000000 );// 2ms
	ktime = ktime_set( 0, 0 );

	curr_data = slow_data;
	total_array = sizeof(slow_data) / sizeof(struct lp5560_leds_data);
	array_num=0;
	lp5560_gpio_set(0);
	hrtimer_start( &g_timeOutTimer, ktime, HRTIMER_MODE_REL );


}

static void lp5560_fast_flash(void)
{
	ktime_t ktime;
	//ktime = ktime_set( 0, 2*1000000 );// 2ms
	ktime = ktime_set( 0, 0 );// 
	curr_data = fast_data;
	total_array = sizeof(fast_data) / sizeof(struct lp5560_leds_data);
	array_num=0;
	lp5560_gpio_set(0);
	hrtimer_start( &g_timeOutTimer, ktime, HRTIMER_MODE_REL );
}

static void lp5560_stand_by(void)
{
	lp5560_gpio_set(0);
	lp5560_mdelay(TIMEOUT_COUNTS);
}


static void lp5560_run_command(void)
{
	lp5560_gpio_set(1);

}

static void lp5560_led_work(struct work_struct *work)
{
	struct lp5560_leds_priv	*led_dat =
		container_of(work, struct lp5560_leds_priv, work);

	switch(led_dat->level)
	{
		case 0:
			lp5560_stand_by();
			break;
		case 1:
			lp5560_run_command();
			break;
		case 4:
			lp5560_slow_flash();
			break;
		case 5:
			lp5560_fast_flash();
			break;
		default:
			break;
	}
}


static void lp5560_led_set(struct led_classdev *led_cdev,enum led_brightness value)
{
	struct lp5560_leds_priv *led_dat =
		container_of(led_cdev, struct lp5560_leds_priv, cdev);

	if((value != 0)&&(value!=1)&&(last_brightness == value))
		led_dat->level = 1;
	else
		led_dat->level = value;

	if((value!=0)&&(value!=1))
		last_brightness = value;
	
	schedule_work(&led_dat->work);

}
static void lp5560_led_work_test(struct work_struct *work)
{
	struct lp5560_leds_priv	*led_dat =
		container_of(work, struct lp5560_leds_priv, work);

	switch(led_dat->level)
	{
		case 0:
			lp5560_stand_by();
			break;
		case 255:
			lp5560_fast_flash();
			break;
		default:
			break;
	}
}

static void lp5560_led_set_test(struct led_classdev *led_cdev,enum led_brightness value)
{
	struct lp5560_leds_priv *led_dat =
		container_of(led_cdev, struct lp5560_leds_priv, cdev);
#if 0
	if((value != 0)&&(value!=1)&&(last_brightness == value))
		led_dat->level = 1;
	else
		led_dat->level = value;

	if((value!=0)&&(value!=1))
		last_brightness = value;
#else
	led_dat->level = value;
#endif	
	schedule_work(&led_dat->work);

}



/****************************************************************************
 * driver functions
 ***************************************************************************/
static int __init lp5560_leds_probe(struct platform_device *pdev)
{
	struct lp5560_leds_priv *priv;
	int ret=0,i=0;
	LEDS_DEBUG("[LED]%s\n", __func__);


	g_lp5560_leds_data[0] = kzalloc(sizeof(struct lp5560_leds_priv), GFP_KERNEL);
	if (!g_lp5560_leds_data[0]) {
		ret = -ENOMEM;
		goto err;
	}
	
	g_lp5560_leds_data[0]->cdev.name = LED_NAME;
	g_lp5560_leds_data[0]->cdev.brightness_set = lp5560_led_set;
	INIT_WORK(&g_lp5560_leds_data[0]->work, lp5560_led_work);
	g_lp5560_leds_data[0]->gpio = CTRL_PIN;
	g_lp5560_leds_data[0]->level = 0;
	
	ret = led_classdev_register(&pdev->dev, &g_lp5560_leds_data[0]->cdev);
	if (ret)
		goto err;
	platform_set_drvdata(pdev, g_lp5560_leds_data[0]);

	hrtimer_init( &g_timeOutTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	g_timeOutTimer.function=lp5560_Callback;

	//for factory test
	g_lp5560_leds_data[1] = kzalloc(sizeof(struct lp5560_leds_priv), GFP_KERNEL);
	if (!g_lp5560_leds_data[1]) {
		ret = -ENOMEM;
		goto err;
	}
	g_lp5560_leds_data[1]->cdev.name = "test-led";
	g_lp5560_leds_data[1]->cdev.brightness_set = lp5560_led_set_test;
	INIT_WORK(&g_lp5560_leds_data[1]->work, lp5560_led_work_test);
	g_lp5560_leds_data[1]->gpio = CTRL_PIN;//GPIO_LED_EN;
	g_lp5560_leds_data[1]->level = 0;
	
	ret = led_classdev_register(&pdev->dev, &g_lp5560_leds_data[1]->cdev);
	//end for factory test
	return 0;
err:
	
	for (i = 1; i >=0; i--) {
			if (!g_lp5560_leds_data[i])
				continue;
			led_classdev_unregister(&g_lp5560_leds_data[i]->cdev);
			cancel_work_sync(&g_lp5560_leds_data[i]->work);
			kfree(g_lp5560_leds_data[i]);
			g_lp5560_leds_data[i] = NULL;
	}
	return ret;
}

static int lp5560_leds_remove(struct platform_device *pdev)
{
	struct lp5560_leds_priv *priv = dev_get_drvdata(&pdev->dev);
	LEDS_DEBUG("[LED]%s\n", __func__);

	dev_set_drvdata(&pdev->dev, NULL);

	kfree(priv);

	return 0;
}


/*
static int lp5560_leds_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}
static int lp5560_leds_shutdown(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

*/


static struct platform_driver lp5560_leds_driver = {
	.driver		= {
		.name	= "leds-lp5560",
		.owner	= THIS_MODULE,
	},
	.probe		= lp5560_leds_probe,
	.remove		= lp5560_leds_remove,
	//.suspend	= lp5560_leds_suspend,
	//.shutdown   = lp5560_leds_shutdown,
};


/***********************************************************************************
* please add platform device in mt_devs.c
*
************************************************************************************/
static int __init lp5560_leds_init(void)
{
	int ret;

	LEDS_DEBUG("[LED]%s\n", __func__);

	ret = platform_driver_register(&lp5560_leds_driver);

	if (ret)
	{
		printk("[LED]lp5560_leds_init:drv:E%d\n", ret);
		return ret;
	}

	return ret;
}

static void __exit lp5560_leds_exit(void)
{
	platform_driver_unregister(&lp5560_leds_driver);
}

module_param(debug_enable, int,0644);

late_initcall(lp5560_leds_init);
module_exit(lp5560_leds_exit);

MODULE_AUTHOR("jixu@lenovo.com");
MODULE_DESCRIPTION("LP5560 led driver");
MODULE_LICENSE("GPL");


