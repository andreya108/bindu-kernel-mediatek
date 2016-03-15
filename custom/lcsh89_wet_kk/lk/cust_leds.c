//#include <platform/cust_leds.h>
#include <cust_leds.h>
#include <platform/mt_gpio.h>
#include <platform/mt_gpt.h>
#include <platform/mt_pwm.h>
//#include <asm/arch/mt6577_pwm.h>

//extern int DISP_SetBacklight(int level);

extern int disp_bls_set_backlight(unsigned int level);
// Only support 64 levels of backlight (when lcd-backlight = MT65XX_LED_MODE_PWM)
#define BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT 64 
// Support 256 levels of backlight (when lcd-backlight = MT65XX_LED_MODE_PWM)
#define BACKLIGHT_LEVEL_PWM_256_SUPPORT 256 

// Custom can decide the support type "BACKLIGHT_LEVEL_PWM_256_SUPPORT" or "BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT"
#define BACKLIGHT_LEVEL_PWM_MODE_CONFIG BACKLIGHT_LEVEL_PWM_256_SUPPORT

unsigned int Cust_GetBacklightLevelSupport_byPWM(void)
{
	return BACKLIGHT_LEVEL_PWM_MODE_CONFIG;
}

void one_wire_control(unsigned int count)
{
	mt_set_gpio_mode(129, GPIO_MODE_GPIO);
	mt_set_gpio_dir(129, GPIO_DIR_OUT);
	
	count = 17-count;
		
	while(count--)	//count = 1~16
	{
		
		mt_set_gpio_out(129, 1);
		udelay(100);
		mt_set_gpio_out(129, 0);
		udelay(100);
		//mt_set_gpio_out(gpio_num, 1);
	}
	mt_set_gpio_out(129, 1);
}

int Cust_GPIO_SetBacklight(unsigned int level)
{
	unsigned int mapped_level;

	if(0 != level)
	{
	    mapped_level = level/16 + 1; //1-wire control in S5 phone only has 16 step 

		one_wire_control(mapped_level);
	}
	else
	{
		mt_set_gpio_out(129, 0);
	}
	return 0;
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
	       g_pre_pulse = pulse_num;

	}

       pulse_num_temp = pulse_num;
	   
	#if defined(SGM3727_DEBUG)	
	printk("kernel SGM3727_SetBacklight g_pre_pulse=%d \n", g_pre_pulse);
	#endif	

	return 0;
}
#endif

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

