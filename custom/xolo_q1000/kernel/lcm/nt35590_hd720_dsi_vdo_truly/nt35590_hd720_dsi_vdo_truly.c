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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID_NT35590 (0x90)
#define REGFLAG_DELAY             							0xFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   

#if defined(MTK_WFD_SUPPORT)
#define   LCM_DSI_CMD_MODE							1
#else
#define   LCM_DSI_CMD_MODE							0
#endif

struct LCM_setting_table {
    unsigned int cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_pre_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/
	{0xFF,	1,	{0xEE}},
	{0x26,	1,	{0x08}},
	{REGFLAG_DELAY, 10, {}},
	{0x26,	1,	{0x00}},

	{0xFF,	1,	{0x00}},
	{REGFLAG_DELAY, 20, {}},
	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

	//{0x36,	1,	{0x94}},
	{0xFF,	1,	{0xEE}},////truly add 20130124
	{0xFB,	1,	{0x01}},////truly add 20130124
	{0x12,	1,	{0x50}},////truly add 20130124
	{0x13,	1,	{0x02}},////truly add 20130124
	{0x6A,	1,	{0x60}},////truly add 20130124

	{0xFF,	1,	{0x00}},
	{0xBA,	1,	{0x01}},		// 2 lane
	{0xC2,	1,	{0x08}},
	{0x35,	1,	{0x00}},
	{0x44,    1,    {((FRAME_HEIGHT/2)>>8) , ((FRAME_HEIGHT/2)&0xFF)}},
	{0xFF,	1,	{0x01}},
	{0x00, 	1,	{0x3A}},
	{0x01, 	1,	{0x33}},
	{0x02, 	1,	{0x53}},
	{0x09, 	1,	{0x85}},
	{0x0E, 	1,	{0x25}},
	{0x0F, 	1,	{0x0A}},
	{0x0B, 	1,	{0x97}},
	{0x0C, 	1,	{0x97}},
	{0x11, 	1,	{0x86}},
	{0x12, 	1,	{0x03}},
	{0x36, 	1,	{0x7B}},
	{0xB0, 	1,	{0x80}},
	{0xB1, 	1,	{0x02}},	
	{0x71, 	1,	{0x2C}},

	{0xFF, 	1,	{0x05}},
	{0x01, 	1,	{0x00}},
	#if 1//def __NT35590_HIGH_SPEED__
	{0x02, 	1,	{0x8D}},//0x8D//0xBC
	{0x03, 	1,	{0x8D}},//0x8D//0xBC
	{0x04, 	1,	{0x8D}},//0x8D//0xBC
	#else
	{0x02, 	1,	{0xB8}},//0x8D//0xBC
	{0x03, 	1,	{0xB8}},//0x8D//0xBC
	{0x04, 	1,	{0xB8}},//0x8D//0xBC
	#endif
	{0x05, 	1,	{0x30}},
	{0x06, 	1,	{0x33}},
	{0x07, 	1,	{0x77}},
	{0x08, 	1,	{0x00}},
	{0x09, 	1,	{0x00}},
	{0x0A, 	1,	{0x00}},
	{0x0B, 	1,	{0x80}},
	{0x0C, 	1,	{0xC8}},
	{0x0D, 	1,	{0x00}},
	{0x0E, 	1,	{0x1B}},
	{0x0F, 	1,	{0x07}},
	{0x10, 	1,	{0x57}},
	{0x11, 	1,	{0x00}},
	{0x12, 	1,	{0x00}},
	{0x13, 	1,	{0x1E}},
	{0x14, 	1,	{0x00}},
	{0x15, 	1,	{0x1A}},
	{0x16, 	1,	{0x05}},
	{0x17, 	1,	{0x00}},
	{0x18, 	1,	{0x1E}},
	{0x19, 	1,	{0xFF}},
	{0x1A, 	1,	{0x00}},
	{0x1B, 	1,	{0xFC}},
	{0x1C, 	1,	{0x80}},
	{0x1D, 	1,	{0x00}},
	{0x1E, 	1,	{0x10}},
	{0x1F, 	1,	{0x77}},
	{0x20, 	1,	{0x00}},
	{0x21, 	1,	{0x00}},
	{0x22, 	1,	{0x55}},
	{0x23, 	1,	{0x0D}},
	{0x31, 	1,	{0xA0}},
	{0x32, 	1,	{0x00}},
	{0x33, 	1,	{0xB8}},
	{0x34, 	1,	{0xBB}},
	{0x35, 	1,	{0x11}},
	{0x36, 	1,	{0x01}},
	{0x37, 	1,	{0x0B}},
	{0x38, 	1,	{0x01}},
	{0x39, 	1,	{0x0B}},
	{0x44, 	1,	{0x08}},
	{0x45, 	1,	{0x80}},
	{0x46, 	1,	{0xCC}},
	{0x47, 	1,	{0x04}},
	{0x48, 	1,	{0x00}},
	{0x49, 	1,	{0x00}},
	{0x4A, 	1,	{0x01}},
	{0x6C, 	1,	{0x03}},
	{0x6D, 	1,	{0x03}},
	{0x6E, 	1,	{0x2F}},
	{0x43, 	1,	{0x00}},
	{0x4B, 	1,	{0x23}},
	{0x4C, 	1,	{0x01}},
	{0x50, 	1,	{0x23}},
	{0x51, 	1,	{0x01}},
	{0x58, 	1,	{0x23}},
	{0x59, 	1,	{0x01}},
	{0x5D, 	1,	{0x23}},
	{0x5E, 	1,	{0x01}},
	{0x62, 	1,	{0x23}},
	{0x63, 	1,	{0x01}},
	{0x67, 	1,	{0x23}},
	{0x68, 	1,	{0x01}},
	{0x89, 	1,	{0x00}},
	{0x8D, 	1,	{0x01}},
	{0x8E, 	1,	{0x64}},
	{0x8F, 	1,	{0x20}},
	{0x97, 	1,	{0x8E}},
	{0x82, 	1,	{0x8C}},
	{0x83, 	1,	{0x02}},
	{0xBB, 	1,	{0x0A}},
	{0xBC, 	1,	{0x0A}},
	{0x24, 	1,	{0x25}},
	{0x25, 	1,	{0x55}},
	{0x26, 	1,	{0x05}},
	{0x27, 	1,	{0x23}},
	{0x28, 	1,	{0x01}},
	{0x29, 	1,	{0x31}},
	{0x2A, 	1,	{0x5D}},
	{0x2B, 	1,	{0x01}},
	{0x2F, 	1,	{0x00}},
	{0x30, 	1,	{0x10}},
	{0xA7, 	1,	{0x12}},
	{0x2D, 	1,	{0x03}},
	// Skip Gamma, CABC

	{0xFF, 	1,	{0x00}},
	{0xFB, 	1,	{0x01}},

	{0xFF, 	1,	{0x01}},
	{0xFB, 	1,	{0x01}},

	{0xFF, 	1,	{0x02}},
	{0xFB, 	1,	{0x01}},

	{0xFF, 	1,	{0x03}},
	{0xFB, 	1,	{0x01}},

	{0xFF, 	1,	{0x04}},
	{0xFB, 	1,	{0x01}},

	{0xFF, 	1,	{0x05}},
	{0xFB, 	1,	{0x01}},

	{0xFF, 	1,	{0x00}},
	{0x3A,	1,	{0x77}},

	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	// Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xB9,	3,	{0xFF, 0x83, 0x69}},
	{REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
//	{0xC3, 1, {0xFF}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);	
                break;
       	}
		
    }
	
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

#ifdef SLT_DEVINFO_LCM
		params->module="truly";
		params->vendor="truly";
		params->ic="nt35590";
		params->info="720*1280";
#endif

    #if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
    #else
	params->dsi.mode   = BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
    #endif

	// DSI
	/* Command mode setting */
	//1 Three lane or Four lane
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Video mode setting		
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	
	params->dsi.vertical_sync_active				= 1;// 3    2
	params->dsi.vertical_backporch					= 1;// 20   1
	params->dsi.vertical_frontporch					= 2; // 1  12
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 2;// 50  2
	params->dsi.horizontal_backporch				= 12;
	params->dsi.horizontal_frontporch				= 80;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

    //params->dsi.LPX=8; 

	// Bit rate calculation
	//1 Every lane speed
	params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
	params->dsi.fbk_div =29;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
}

static void lcm_init(void)
{
	int i;
	unsigned char buffer[10];
	unsigned int  array[16];
	unsigned int data_array[16];

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(60);

	push_table(lcm_pre_initialization_setting, sizeof(lcm_pre_initialization_setting) / sizeof(struct LCM_setting_table), 1);

	SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(120);
	
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_suspend(void)
{

	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
	char  buffer[3];

	read_reg_v2(0xAB, buffer, 1); //john add use for esd recovery issue.	
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
        
        
}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif

static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20); 

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; //we only need ID
    #ifdef BUILD_LK
		printf("%s, LK nt35590 debug: nt35590 id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel nt35590 horse debug: nt35590 id = 0x%08x\n", __func__, id);
    #endif

    if(id == LCM_ID_NT35590)
    	return 1;
    else
        return 0;


}
static bool First_Boot_Lcm = FALSE;
unsigned char count;
static char buffer_error[8];
static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
	int  buffer[8];
	int   array[4];
	int i=0;
	unsigned int id=0;
	
    if(lcm_esd_test)
    {
        lcm_esd_test = FALSE;
        return TRUE;
    }
	for(i=0;i<8;i++)
	{
		buffer_error[i]=0;
	}

	
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xAB, buffer, 2);
#if 0
	for(i=0;i<8;i++)
	{
		printk("john buffer_error[%d]=%d\n",i,buffer_error[i]);
	}
#endif
//	id = buffer[0];
//	if(id != 0x9c)
	if(First_Boot_Lcm)
	{
		//buffer_error[2] !=0; due to first time our buffer_error[2] value = 64,so abndon this data.
		First_Boot_Lcm = FALSE;
		return FALSE;
	}
	if((buffer_error[1] != 0x00) || (buffer_error[2] != 0))
	{
		return TRUE;
	}

	/*read_reg_v2(0x0B, buffer, 1);	
	id = buffer[0];
	if(id != 0x0)
		return TRUE;
	read_reg_v2(0x0C, buffer, 1);	
	id = buffer[0];
	if(id != 0x7)
		return TRUE;*/

	return FALSE;
 #endif

}

static unsigned int lcm_esd_recover(void)
{
		lcm_init();
		return TRUE;
}



LCM_DRIVER nt35590_hd720_dsi_vdo_truly_lcm_drv = 
{
    .name			= "nt35590_hd720_dsi_vdo_truly",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	.esd_check = lcm_esd_check,
	.esd_recover = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
