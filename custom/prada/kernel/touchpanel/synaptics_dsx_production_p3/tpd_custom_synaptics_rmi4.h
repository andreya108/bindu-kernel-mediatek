/*
* Copyright (C) 2011-2014 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TOUCHPANEL_H__
#define TOUCHPANEL_H__

/* Pre-defined definition */

/* Register */
#define FD_ADDR_MAX    	0xE9
#define FD_ADDR_MIN    	0xDD
#define FD_BYTE_COUNT 	6

#define CUSTOM_MAX_WIDTH (720)
#define CUSTOM_MAX_HEIGHT (1280)

#define TPD_HAVE_BUTTON
#define TPD_BUTTON_HEIGH        (100)
#define TPD_KEY_COUNT           3
#define TPD_KEYS                { KEY_MENU, KEY_HOMEPAGE, KEY_BACK}
#define TPD_KEYS_DIM		{{145,1330,120,TPD_BUTTON_HEIGH},\
							{360,1330,120,TPD_BUTTON_HEIGH},\
							{600,1330,120,TPD_BUTTON_HEIGH}}

#define TPD_POWER_SOURCE_CUSTOM		MT65XX_POWER_LDO_VGP4
#define TPD_POWER_SOURCE_1800		MT65XX_POWER_LDO_VGP6

#define LCD_X           720
#define LCD_Y           1280

//#define TPD_UPDATE_FIRMWARE
#define HAVE_TOUCH_KEY
#define TPD_DEBUG

//#define TPD_HAVE_CALIBRATION
//#define TPD_CALIBRATION_MATRIX  {2680,0,0,0,2760,0,0,0};
//#define TPD_WARP_START
//#define TPD_WARP_END

//#define TPD_RESET_ISSUE_WORKAROUND
//#define TPD_MAX_RESET_COUNT 3
//#define TPD_WARP_Y(y) ( TPD_Y_RES - 1 - y )
//#define TPD_WARP_X(x) ( x )

#define DSX_I2C_ADDR 0x38
#define DSX_ATTN_GPIO GPIO_CTP_EINT_PIN
#define DSX_ATTN_MUX_NAME "gpmc_ad15.gpio_39"
#define DSX_POWER_GPIO -1
#define DSX_POWER_ON_STATE 0
#define DSX_POWER_DELAY_MS 160
#define DSX_RESET_GPIO GPIO_CTP_RST_PIN
#define DSX_RESET_ON_STATE 0
#define DSX_RESET_DELAY_MS 100
#define DSX_RESET_ACTIVE_MS 20
#define DSX_IRQ_FLAGS EINTF_TRIGGER_FALLING

#endif /* TOUCHPANEL_H__ */
