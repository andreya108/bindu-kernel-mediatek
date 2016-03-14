#include <cust_leds.h>
#include <cust_leds_def.h>
#include <mach/mt_pwm.h>
#include <mach/mt_gpio.h>

#include <linux/kernel.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>

//extern int mtkfb_set_backlight_level(unsigned int level);
//extern int mtkfb_set_backlight_pwm(int div);
extern int disp_bls_set_backlight(unsigned int level);

// Only support 64 levels of backlight (when lcd-backlight = MT65XX_LED_MODE_PWM)
#define BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT 64 
// Support 256 levels of backlight (when lcd-backlight = MT65XX_LED_MODE_PWM)
#define BACKLIGHT_LEVEL_PWM_256_SUPPORT 256 

// Configure the support type "BACKLIGHT_LEVEL_PWM_256_SUPPORT" or "BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT" !!
#define BACKLIGHT_LEVEL_PWM_MODE_CONFIG BACKLIGHT_LEVEL_PWM_256_SUPPORT

unsigned int Cust_GetBacklightLevelSupport_byPWM(void)
{
	return BACKLIGHT_LEVEL_PWM_MODE_CONFIG;
}

unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;
    
    mapped_level = level;
       
	return mapped_level;
}

#if defined(SLT_DRV_AW992_CONFIG)
#define SGM3727_BACKLIGHT_IC
#endif

#ifdef  SGM3727_BACKLIGHT_IC	
//#define SGM3727_DEBUG
#define SGM3727_LEVEL_MIN  0xfe
#define SGM3727_LEVEL_MAX  0xff
#define SGM3727_LEVEL_STEP 32

static BOOL light_first_set_flag = FALSE;
static unsigned int g_pre_pulse=0;
static unsigned int pulse_num_temp= 0xFFF;
static DEFINE_SPINLOCK(backlight_lock);

static unsigned int SGM3727_brightness_mapping(unsigned int level)
{

	 if ((level >= 20) && (level <= 255))
	{
		#if 0//def SLT_DRV_AW890_CONFIG
		return 31-(level-20)*8/75;// 20% decreased for power supply test
		#else
		return 31-(level-20)*2/15;
		#endif
	}
       else if(level < 20)
	       return SGM3727_LEVEL_MIN;
	else 
	       return SGM3727_LEVEL_MAX;
	
}

static unsigned int SGM3727_SetBacklight(unsigned int level)
{
	unsigned int pulse_num = 0;
	unsigned int pulse_diff = 0;
	unsigned int i;
       static int flags;
	   
#if defined(SGM3727_DEBUG)	
	printk("kernel SGM3727_SetBacklight level=%d \n",level);
#endif

	pulse_num = SGM3727_brightness_mapping(level);

#if defined(SGM3727_DEBUG)	
	printk("kernel SGM3727_SetBacklight pulse_num=%d \n",pulse_num);
#endif

	if (light_first_set_flag == FALSE)
	{
		if (mt_set_gpio_out(GPIO_LCM_BL_EN, 0))
		{
		#if defined(SGM3727_DEBUG)	
			printk("kernel SGM3727_SetBacklight first set low error \n");
		#endif
		}

		mdelay(3);
		udelay(100);
		light_first_set_flag = TRUE;

	}

	if (pulse_num == pulse_num_temp)
	{
		return;
	}

	if (pulse_num - SGM3727_LEVEL_MIN == 0)
	{
		if (mt_set_gpio_out(GPIO_LCM_BL_EN, 0))
		{
			#if defined(SGM3727_DEBUG)	
			printk("kernel SGM3727_SetBacklight set low error \n");
			#endif
		}
		g_pre_pulse = 0;

		mdelay(4);
	}
	else if(pulse_num - SGM3727_LEVEL_MAX == 0)
	{
		if (mt_set_gpio_out(GPIO_LCM_BL_EN, 1))
		{
			#if defined(SGM3727_DEBUG)	
			printk("kernel SGM3727_SetBacklight set high error \n");
			#endif
		}
		g_pre_pulse = 0;
	}	
	else if((pulse_num>=0) && (pulse_num<= 31))
	{

		
		if (pulse_num >= g_pre_pulse)
		{
			pulse_diff = pulse_num - g_pre_pulse;
		}
		else
		{
			pulse_diff = pulse_num + SGM3727_LEVEL_STEP  - g_pre_pulse;
		}

		#if defined(SGM3727_DEBUG)	
		printk("kernel SGM3727_SetBacklight pulse_diff=%d \n", pulse_diff);
		#endif

		if(g_pre_pulse == 0)
		{
			mt_set_gpio_out(GPIO_LCM_BL_EN, 1);
			udelay(31);
		}
		
		spin_lock_irqsave(&backlight_lock,flags);
		for (i = 0; i< pulse_diff ; i++)
		{

			if (mt_set_gpio_out(GPIO_LCM_BL_EN, 0))
			{
			#if defined(SGM3727_DEBUG)	
				printk("kernel SGM3727_SetBacklight set low error \n");
			#endif
			}
			udelay(10);
			if (mt_set_gpio_out(GPIO_LCM_BL_EN, 1))
			{
			#if defined(SGM3727_DEBUG)	
				printk("kernel SGM3727_SetBacklight set high error \n");
			#endif
			}
			udelay(10);
			
		}
	       spin_unlock_irqrestore(&backlight_lock,flags);
	       g_pre_pulse = pulse_num;

	}

       pulse_num_temp = pulse_num;
	   
	#if defined(SGM3727_DEBUG)	
	printk("kernel SGM3727_SetBacklight g_pre_pulse=%d \n", g_pre_pulse);
	#endif	

	return 0;
}
#endif

/*
unsigned int Cust_SetBacklight(int level, int div)
{
	kal_uint32 ret=0;
//    mtkfb_set_backlight_pwm(div);
//    mtkfb_set_backlight_level(brightness_mapping(level));


 * To explain How to set these para for cust_led_list[] of led/backlight
 * "name" para: led or backlight
 * "mode" para:which mode for led/backlight
 *	such as:
 *			MT65XX_LED_MODE_NONE,	
 *			MT65XX_LED_MODE_PWM,	
 *			MT65XX_LED_MODE_GPIO,	
 *			MT65XX_LED_MODE_PMIC,	
 *			MT65XX_LED_MODE_CUST_LCM,	
 *			MT65XX_LED_MODE_CUST_BLS_PWM
 *
 *"data" para: control methord for led/backlight
 *   such as:
 *			MT65XX_LED_PMIC_LCD_ISINK=0,	
 *			MT65XX_LED_PMIC_NLED_ISINK0,
 *			MT65XX_LED_PMIC_NLED_ISINK1,
 *			MT65XX_LED_PMIC_NLED_ISINK2,
 *			MT65XX_LED_PMIC_NLED_ISINK3
 * 
 *"PWM_config" para:PWM(AP side Or BLS module), by default setting{0,0,0,0,0} Or {0}
 *struct PWM_config {	 
 *  int clock_source;
 *  int div; 
 *  int low_duration;
 *  int High_duration;
 *  BOOL pmic_pad;//AP side PWM pin in PMIC chip (only 89 needs confirm); 1:yes 0:no(default)
 *};
 *-------------------------------------------------------------------------------------------
 *   for AP PWM setting as follow:
 *1.	 PWM config data
 *  clock_source: clock source frequency, can be 0/1
 *  div: clock division, can be any value within 0~7 (i.e. 1/2^(div) = /1, /2, /4, /8, /16, /32, /64, /128)
 *  low_duration: only for BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT
 *  High_duration: only for BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT
 *
 *2.	 PWM freq.
 * If BACKLIGHT_LEVEL_PWM_MODE_CONFIG = BACKLIGHT_LEVEL_PWM_256_SUPPORT,
 *	 PWM freq. = clock source / 2^(div) / 256  
 *
 * If BACKLIGHT_LEVEL_PWM_MODE_CONFIG = BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT,
 *	 PWM freq. = clock source / 2^(div) / [(High_duration+1)(Level')+(low_duration+1)(64 - Level')]
 *	           = clock source / 2^(div) / [(High_duration+1)*64]     (when low_duration = High_duration)
 *Clock source: 
 *	 0: block clock/1625 = 26M/1625 = 16K (MT6571)
 *	 1: block clock = 26M (MT6571)
 *Div: 0~7
 *
 *For example, in MT6571, PWM_config = {1,1,0,0,0} 
 *	 ==> PWM freq. = 26M/2^1/256 	 =	50.78 KHz ( when BACKLIGHT_LEVEL_PWM_256_SUPPORT )
 *	 ==> PWM freq. = 26M/2^1/(0+1)*64 = 203.13 KHz ( when BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT )
 *-------------------------------------------------------------------------------------------
 *   for BLS PWM setting as follow:
 *1.	 PWM config data
 *	 clock_source: clock source frequency, can be 0/1/2/3
 *	 div: clock division, can be any value within 0~1023
 *	 low_duration: non-use
 *	 High_duration: non-use
 *	 pmic_pad: non-use
 *
 *2.	 PWM freq.= clock source / (div + 1) /1024
 *Clock source: 
 *	 0: 26 MHz
 *	 1: 104 MHz
 *	 2: 124.8 MHz
 *	 3: 156 MHz
 *Div: 0~1023
 *
 *By default, clock_source = 0 and div = 0 => PWM freq. = 26 KHz 
 *-------------------------------------------------------------------------------------------
 */
static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_NONE, -1,{0}},
	{"green",             MT65XX_LED_MODE_NONE, -1,{0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_BUTTON,{0}},
#if defined(SLT_DRV_AW992_CONFIG)
	{"lcd-backlight",     MT65XX_LED_MODE_GPIO, (int)SGM3727_SetBacklight,{0}},
#else
	{"lcd-backlight",     MT65XX_LED_MODE_CUST_BLS_PWM, (int)disp_bls_set_backlight,{0}},
#endif
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

