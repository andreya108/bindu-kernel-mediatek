/*
 * drivers/leds/leds_gpio.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * leds leds driver
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
#define LED_NAME "greenled"
//#define LED_NAME "green"
#define CTRL_PIN 149

static int debug_enable = 1;

#define LEDS_DEBUG(format, args...) do{ \
	if(debug_enable) \
	{\
		printk(KERN_EMERG format,##args);\
	}\
}while(0)

struct leds_gpio_priv {
	struct led_classdev cdev;
	struct work_struct work;
	int gpio;
	int level;
	int delay_on;
	int delay_off;
};


/****************************************************************************
 * local functions
 ***************************************************************************/
struct leds_gpio_priv *g_leds_gpio_data[2];
static struct hrtimer g_timeOutTimer;
static int g_timeOutTimeMs =0;
static int timerInited = 0;
static int timerRepeat = 0;
static int is_leds_on = 0;
static int gdelay_on = 0;
static int gdelay_off =0;
static int timercanceled = 0;


static void leds_udelay(UINT32 us)
{
	udelay(us);
}

static void leds_mdelay(UINT32 ms)
{
	msleep(ms);
}

static void leds_on(void)
{
	mt_set_gpio_out(CTRL_PIN, 1);
	is_leds_on = 1;
	
}

static void leds_off(void)
{
	mt_set_gpio_out(CTRL_PIN, 0);
	is_leds_on = 0;
}

static enum hrtimer_restart ledTimeOutCallback(struct hrtimer *timer)
{
	if(!timercanceled){
		if(is_leds_on)
			leds_off();
		else
			leds_on();
		
		if (timerRepeat) {
			if(is_leds_on)
				hrtimer_add_expires_ns(&g_timeOutTimer, gdelay_on*1000000);
			else
				hrtimer_add_expires_ns(&g_timeOutTimer, gdelay_off*1000000);
			return HRTIMER_RESTART;
		} else
			return HRTIMER_NORESTART;
	}else{
		timercanceled = 0;
		return HRTIMER_NORESTART;
	}
}

static void leds_timer_init(void)
{
	if(!timerInited) {
		g_timeOutTimeMs=500; //1s//0.1s
		hrtimer_init( &g_timeOutTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
		g_timeOutTimer.function=ledTimeOutCallback;
		timerInited=1;
		timercanceled = 0;
	}

}
static void leds_timer_deinit(void)
{
	if(timerInited) {
		timerRepeat = 0;
		timercanceled = 1;
		hrtimer_cancel( &g_timeOutTimer );	
		timerInited=0;
	}

}

static void leds_proc_backlight_value(int level, int delay_on, int delay_off)
{
	ktime_t ktime;
	leds_timer_deinit();
	leds_timer_init();
	
	gdelay_on=delay_on;
	gdelay_off=delay_off;

//	leds_off();
	if(level){
		if(delay_on!=0 && delay_off!=0){
			timerRepeat = 1;
			ktime = ktime_set( 0, 0 );
			hrtimer_start( &g_timeOutTimer, ktime, HRTIMER_MODE_REL );
		}else{
			leds_on();
		}
			
	}else{

		leds_timer_deinit();
		//leds_mdelay(100);//for timer cancel done
		leds_off();
	}

}

static int  leds_blink_set(struct led_classdev *led_cdev,
									unsigned long *delay_on,
									unsigned long *delay_off)
{
	LEDS_DEBUG("[LED]%s delay_on=0x%x, delay_off=0x%x\n", __func__,*delay_on,*delay_off);

	struct leds_gpio_priv *led_data =
		container_of(led_cdev, struct leds_gpio_priv, cdev);


	if (*delay_on != led_data->delay_on || *delay_off != led_data->delay_off) {
		led_data->delay_on = *delay_on;
		led_data->delay_off = *delay_off;

	
	}
	
	return 0;
}

static void leds_led_work(struct work_struct *work)
{
	struct leds_gpio_priv	*led_data =
		container_of(work, struct leds_gpio_priv, work);
	if(led_data->level==0){
		led_data->delay_on=0;
		led_data->delay_off=0;
	}
	//else{
	//	led_data->delay_on = 1000;
	//	led_data->delay_off = 5000;
	//}
	//fix delay_on = 1000, delay_off=5000
	//leds_proc_backlight_value(led_data->level,led_data->delay_on,led_data->delay_off);
	leds_proc_backlight_value(led_data->level,0,0);
}


static void leds_led_set(struct led_classdev *led_cdev,enum led_brightness value)
{
	struct leds_gpio_priv *led_data =
		container_of(led_cdev, struct leds_gpio_priv, cdev);
	
	led_data->level = value;
	schedule_work(&led_data->work);

}

static void leds_led_work_factory_test(struct work_struct *work)
{
	struct leds_gpio_priv	*led_data =
		container_of(work, struct leds_gpio_priv, work);

	leds_proc_backlight_value(led_data->level,500,500);

}

static void leds_led_set_factory_test(struct led_classdev *led_cdev,enum led_brightness value)
{
	struct leds_gpio_priv *led_data =
		container_of(led_cdev, struct leds_gpio_priv, cdev);
	
	led_data->level = value;
	schedule_work(&led_data->work);

}



/****************************************************************************
 * driver functions
 ***************************************************************************/
static int __init leds_gpio_probe(struct platform_device *pdev)
{
	struct leds_gpio_priv *priv;
	int ret=0,i=0;
	LEDS_DEBUG("[LED]%s\n", __func__);

	mt_set_gpio_mode(CTRL_PIN, 0);		
	mt_set_gpio_dir(CTRL_PIN, 1);

	g_leds_gpio_data[0] = kzalloc(sizeof(struct leds_gpio_priv), GFP_KERNEL);
	if (!g_leds_gpio_data[0]) {
		ret = -ENOMEM;
		goto err;
	}
	
	g_leds_gpio_data[0]->cdev.name = LED_NAME;
	g_leds_gpio_data[0]->cdev.brightness_set = leds_led_set;
	g_leds_gpio_data[0]->cdev.blink_set = leds_blink_set;
	INIT_WORK(&g_leds_gpio_data[0]->work, leds_led_work);
	g_leds_gpio_data[0]->gpio = CTRL_PIN;
	g_leds_gpio_data[0]->level = 0;
	
	ret = led_classdev_register(&pdev->dev, &g_leds_gpio_data[0]->cdev);
	if (ret)
		goto err;
	platform_set_drvdata(pdev, g_leds_gpio_data[0]);

	//for factory test
	g_leds_gpio_data[1] = kzalloc(sizeof(struct leds_gpio_priv), GFP_KERNEL);
	if (!g_leds_gpio_data[1]) {
		ret = -ENOMEM;
		goto err;
	}
	g_leds_gpio_data[1]->cdev.name = "test-led";
	g_leds_gpio_data[1]->cdev.brightness_set = leds_led_set_factory_test;
	INIT_WORK(&g_leds_gpio_data[1]->work, leds_led_work_factory_test);
	g_leds_gpio_data[1]->gpio = CTRL_PIN;//GPIO_LED_EN;
	g_leds_gpio_data[1]->level = 0;
	
	ret = led_classdev_register(&pdev->dev, &g_leds_gpio_data[1]->cdev);
	//end for factory test
	if (ret)
		goto err;

	//leds_timer_init();
		
	return 0;
err:
	
	for (i = 1; i >=0; i--) {
			if (!g_leds_gpio_data[i])
				continue;
			led_classdev_unregister(&g_leds_gpio_data[i]->cdev);
			cancel_work_sync(&g_leds_gpio_data[i]->work);
			kfree(g_leds_gpio_data[i]);
			g_leds_gpio_data[i] = NULL;
	}
	return ret;
}

static int leds_gpio_remove(struct platform_device *pdev)
{
	struct leds_gpio_priv *priv = dev_get_drvdata(&pdev->dev);
	LEDS_DEBUG("[LED]%s\n", __func__);

	dev_set_drvdata(&pdev->dev, NULL);

	kfree(priv);

	return 0;
}


/*
static int leds_gpio_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}
static int leds_gpio_shutdown(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

*/


static struct platform_driver leds_gpio_driver = {
	.driver		= {
		.name	= "leds-gpio",
		.owner	= THIS_MODULE,
	},
	.probe		= leds_gpio_probe,
	.remove		= leds_gpio_remove,
	//.suspend	= leds_gpio_suspend,
	//.shutdown   = leds_gpio_shutdown,
};


/***********************************************************************************
* please add platform device in mt_devs.c
*
************************************************************************************/
static int __init leds_gpio_init(void)
{
	int ret;

	LEDS_DEBUG("[LED]%s\n", __func__);

	ret = platform_driver_register(&leds_gpio_driver);

	if (ret)
	{
		printk("[LED]leds_gpio_init:drv:E%d\n", ret);
		return ret;
	}

	return ret;
}

static void __exit leds_gpio_exit(void)
{
	platform_driver_unregister(&leds_gpio_driver);
}

module_param(debug_enable, int,0644);

late_initcall(leds_gpio_init);
module_exit(leds_gpio_exit);

MODULE_AUTHOR("jixu@lenovo.com");
MODULE_DESCRIPTION("led gpio driver");
MODULE_LICENSE("GPL");


