/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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

// unused

unsigned int Cust_GetBacklightLevelSupport_byPWM(void)
{
	return 0;
}

unsigned int disp_set_backlight(int level)
{

    return 0;
}

unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;

    mapped_level = level;

    return mapped_level;
}

// CONFIG_BACKLIGHT_AW9910
// 0 - 255 => 32 - 4
#define MAX_BACKLIGHT_BRIGHTNESS 255
#define MY_BACKLIGHT_MAX        32
#define PREV_BK_L   (MY_BACKLIGHT_MAX + 1)

#define PWM_GPIO_EN GPIO129
#define gpio_set_value_cansleep mt_set_gpio_out

static int prev_bl = PREV_BK_L;

static void mipi_otm_set_backlight(int level)
{
	int bl_level;

	int step = 0, i = 0;
	unsigned long flags;
	int value;
	
	if(level > 239) {
		level = 239;
	}

	value = level;
	if (value > MAX_BACKLIGHT_BRIGHTNESS)
		value = MAX_BACKLIGHT_BRIGHTNESS;

	/* This maps android backlight level 0 to 255 into
	   driver backlight level 0 to bl_max with rounding */
	bl_level = (2 * value * MY_BACKLIGHT_MAX + MAX_BACKLIGHT_BRIGHTNESS)
			   / (2 * MAX_BACKLIGHT_BRIGHTNESS);

	if (!bl_level && value)
		bl_level = 1;
		
	bl_level = PREV_BK_L - bl_level;
	
	printk("%s: prev_bl = %d, bl_level = %d\n", __func__, prev_bl, bl_level);
	
	if (bl_level > prev_bl) {
		step = bl_level - prev_bl;
		if (bl_level == PREV_BK_L) {
			step--;
		}
	} else if (bl_level < prev_bl) {
		step = bl_level + MY_BACKLIGHT_MAX - prev_bl;
	} else {
		step = bl_level + MY_BACKLIGHT_MAX - prev_bl;
		return;
	}

	if (bl_level == PREV_BK_L) {
		/* turn off backlight */
		printk("%s: turn off backlight\n", __func__);
		gpio_set_value_cansleep(PWM_GPIO_EN, GPIO_OUT_ZERO);
	} else {
		local_irq_save(flags);

		if (prev_bl == PREV_BK_L) {
			/* turn on backlight */
			gpio_set_value_cansleep(PWM_GPIO_EN, GPIO_OUT_ONE);
			udelay(40);
		}
		/* adjust backlight level */
		for (i = 0; i < step; i++) {
			gpio_set_value_cansleep(PWM_GPIO_EN, GPIO_OUT_ZERO);
			udelay(1);
			gpio_set_value_cansleep(PWM_GPIO_EN, GPIO_OUT_ONE);
			udelay(1);
		}

		local_irq_restore(flags);
	}
	msleep(1);
	prev_bl = bl_level;
}

static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_NONE, -1,{0}},
	{"green",             MT65XX_LED_MODE_NONE, -1,{0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
	
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},

	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_BUTTON,{0}},
	{"lcd-backlight",     MT65XX_LED_MODE_CUST_BLS_PWM, (int)mipi_otm_set_backlight,{0}},
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}
