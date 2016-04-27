//#include <platform/cust_leds.h>
#include <cust_leds.h>
#include <platform/mt_gpio.h>
#include <platform/mt_gpt.h>
#include <platform/mt_pwm.h>
//#include <asm/arch/mt6577_pwm.h>

//extern int DISP_SetBacklight(int level);

#define BACKLIGHT_LEVEL_PWM_256_SUPPORT 256

#define BACKLIGHT_LEVEL_PWM_MODE_CONFIG BACKLIGHT_LEVEL_PWM_256_SUPPORT

static unsigned int back_level = 255;

unsigned int Cust_GetBacklightLevelSupport_byPWM(void)
{
	return BACKLIGHT_LEVEL_PWM_MODE_CONFIG;
}

#if defined(DCT_K7T) || defined(DCT_K7W)
unsigned int disp_set_backlight(int level)
{
    int num ,now_level,pre_level;
    int i;
    mt_set_gpio_pull_enable(GPIO129,GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO129,GPIO_PULL_UP);

    if(level == 0) {
        mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
        //mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
        //mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO); /* GPIO mode */
    } else {
        if(back_level == 0) {
            mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
            //mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
            //mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT); /* GPIO mode */
            //udelay(20);
            msleep(2);
        }

        now_level = 64 -(level >> 2);
        pre_level = 64 -(back_level >> 2);
        if(now_level >= pre_level)
            num = now_level - pre_level;
        else if (now_level < pre_level)
            num = 64 + now_level - pre_level;
        for(i=0 ;i < num;i++)
        {
            mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
            udelay(2);
            mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
            udelay(2);
        }
    }
    back_level = level ;
    return 0;
}
#else
unsigned int disp_set_backlight(int level)
{
    int now_level,addr = 0x72;
    int i;
#if (defined(V6_X2))
	if(level > 239) {
		level = 239;
	}
#else
	;
#endif
    now_level = (level >> 3);
    if(level == 0) {
        mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
    } else {
            if(back_level == 0) {
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                mdelay(3);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(150);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                udelay(450);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(580);
            }
        mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
        udelay(4);
        for(i=0 ;i < 8;i++)
        {
            if(addr&0x80)
            {
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                udelay(3);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(7);
            }
            else
            {
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                udelay(7);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(3);
            }
            addr <<= 1;
        }
        mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
        udelay(4);
        mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
        udelay(4);
        for(i=0 ;i < 8;i++)
        {
            if(now_level&0x80)
            {
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                udelay(3);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(7);
            }
            else
            {
                mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
                udelay(7);
                mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
                udelay(3);
            }
            now_level <<= 1;
        }
        mt_set_gpio_out(GPIO129, GPIO_OUT_ZERO);
        udelay(4);
        mt_set_gpio_out(GPIO129, GPIO_OUT_ONE);
        udelay(4);
    }
        back_level = level ;
    return 0;
}
#endif
//unsigned int brightness_mapping(unsigned int level)
//{
//    unsigned int mapped_level;

//    mapped_level = level;

//        return mapped_level;
//}

static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"green",             MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK0,{0}},
	{"blue",              MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK2,{0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_BUTTON,{0}},
	{"lcd-backlight",     MT65XX_LED_MODE_CUST_BLS_PWM, (int)disp_set_backlight,{0}},
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

