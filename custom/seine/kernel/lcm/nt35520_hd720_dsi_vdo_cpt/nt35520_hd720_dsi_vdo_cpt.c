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

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#define LCM_ID (0x20)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0x00   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									1

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
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

	
	
//PAGE0
{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
{0xFF,4,{0xAA,0x55,0xA5,0x80}},
{0x6F,1,{0x13}},
{0xF7,1,{0x00}},
{0x6F,1,{0x02}},
{0xB8,1,{0x08}},
{0xBB,2,{0x74,0x44}},
{0xBC,2,{0x00,0x00}},
{0xB6,1,{0x08}},
{0xB1,2,{0x7A,0x21}},
{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},

//PAGE1
{0xB3,2,{0x2F,0x2F}},
{0xB4,2,{0x0F,0x0F}},
{0xB9,2,{0x33,0x33}},
{0xBA,2,{0x23,0x23}},
{0xB5,2,{0x05,0x05}},
{0xC0,1,{0x04}},
{0xBC,2,{0x90,0x01}},
{0xBD,2,{0x90,0x01}},
{0xBE,1,{0x6A}},
{0xCA,1,{0x00}},


{0xF0,5,{0x55,0xAA,0x52,0x08,0x02}},
{0xEE,1,{0x01}},
{0xB0,16,{0x00,0x00,0x00,0x3F,0x00,0x63,0x00,0x82,0x00,0x95,0x00,0xBC,0x00,0xDD,0x01,0x11}},

{0xB1,16,{0x01,0x38,0x01,0x7D,0x01,0xB1,0x02,0x07,0x02,0x50,0x02,0x51,0x02,0x8F,0x02,0xD4}},

{0xB2,16,{0x02,0xFB,0x03,0x30,0x03,0x52,0x03,0x7D,0x03,0x99,0x03,0xBD,0x03,0xD2,0x03,0xE8}},

{0xB3,4,{0x03,0xFD,0x03,0xFF}},

//end puls
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0x6F,1,{0x02}},
{0xBD,1,{0x0F}},

//GOUT Mapping  Page6
{0xF0,5,{0x55,0xAA,0x52,0x08,0x06}},
{0xB0,2,{0x00,0x02}},
{0xB1,2,{0x10,0x16}},
{0xB2,2,{0x12,0x18}},
{0xB3,2,{0x31,0x31}},
{0xB4,2,{0x31,0x34}},
{0xB5,2,{0x34,0x31}},
{0xB6,2,{0x31,0x33}},
{0xB7,2,{0x33,0x33}},
{0xB8,2,{0x0A,0x08}},
{0xB9,2,{0x2D,0x2E}},
{0xBA,2,{0x2E,0x2D}},
{0xBB,2,{0x09,0x0B}},
{0xBC,2,{0x33,0x33}},
{0xBD,2,{0x33,0x31}},
{0xBE,2,{0x31,0x34}},
{0xBF,2,{0x34,0x31}},
{0xC0,2,{0x31,0x31}},
{0xC1,2,{0x19,0x13}},
{0xC2,2,{0x17,0x11}},
{0xC3,2,{0x03,0x01}},
{0xE5,2,{0x31,0x31}},
{0xC4,2,{0x0B,0x09}},
{0xC5,2,{0x19,0x13}},
{0xC6,2,{0x17,0x11}},
{0xC7,2,{0x31,0x31}},
{0xC8,2,{0x31,0x34}},
{0xC9,2,{0x34,0x31}},
{0xCA,2,{0x31,0x33}},
{0xCB,2,{0x33,0x33}},
{0xCC,2,{0x01,0x03}},

{0xCD,2,{0x2E,0x2D}},
{0xCE,2,{0x2D,0x2E}},
{0xCF,2,{0x02,0x00}},
{0xD0,2,{0x33,0x33}},
{0xD1,2,{0x33,0x31}},
{0xD2,2,{0x31,0x34}},
{0xD3,2,{0x34,0x31}},
{0xD4,2,{0x31,0x31}},
{0xD5,2,{0x10,0x16}},
{0xD6,2,{0x12,0x18}},
{0xD7,2,{0x08,0x0A}},
{0xE6,2,{0x31,0x31}},
{0xD8,5,{0x00,0x00,0x00,0x00,0x00}},
{0xD9,5,{0x00,0x00,0x00,0x00,0x00}},
{0xE7,1,{0x00}},

//PAGE5: dof_opt_en, clr_opt_en
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xED,1,{0x30}},

//PAGE3: GateEQ
{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
{0xB1,2,{0x20,0x00}},
{0xB0,2,{0x20,0x00}},

//SETPAGE 5
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xE5,1,{0x00}},

//PAGE5: Initial & On/Off
//SETPAGE 5
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xB0,2,{0x17,0x06}},
{0xB8,1,{0x0C}},
{0xBD,1,{0x03}},
{0xB1,2,{0x17,0x06}},
{0xB9,2,{0x00,0x0F}},
{0xB2,2,{0x17,0x06}},
{0xBA,2,{0x00,0x03}},
{0xB3,2,{0x17,0x06}},
{0xBB,2,{0x0A,0x00}},
{0xB4,2,{0x17,0x06}},
{0xB5,2,{0x17,0x06}},
{0xB6,2,{0x14,0x03}},
{0xB7,2,{0x00,0x00}},
{0xBC,2,{0x02,0x03}},
{0xE5,1,{0x06}},
{0xE6,1,{0x06}},
{0xE7,1,{0x06}},
{0xE8,1,{0x06}},
{0xE9,1,{0x06}},
{0xEA,1,{0x06}},
{0xEB,1,{0x00}},
{0xEC,1,{0x02}},

//STV Settings
//SETPAGE 5
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xC0,1,{0x07}},
{0xC1,1,{0x05}},
{0xC2,1,{0x0C}},
{0xC3,1,{0x08}},

//SETPAGE 3
{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
{0xB2,5,{0x05,0x00,0x17,0x00,0x00}},
{0xB3,5,{0x05,0x00,0x17,0x00,0x00}},
{0xB4,5,{0x05,0x00,0x17,0x00,0x00}},
{0xB5,5,{0x05,0x00,0x17,0x00,0x00}},

//RST Settings
//SETPAGE 5
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xC4,4,{0x82}},
{0xC5,4,{0x80}},
{0xC6,4,{0x82}},
{0xC7,4,{0x80}},

//SETPAGE 3
{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
{0xB6,5,{0x05,0x00,0x17,0x00,0x00}},
{0xB7,5,{0x05,0x00,0x17,0x00,0x00}},
{0xB8,5,{0x15,0x00,0x17,0x00,0x00}},
{0xB9,5,{0x15,0x00,0x17,0x00,0x00}},

//CLK Settings
//SETPAGE 5
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xC8,2,{0x03,0x20}},
{0xC9,2,{0x01,0x21}},
{0xCA,2,{0x01,0x60}},
{0xCB,2,{0x01,0x60}},

//SETPAGE 3
{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
{0xBA,5,{0x53,0x00,0x1A,0x00,0x00}},
{0xBB,5,{0x53,0x00,0x1A,0x00,0x00}},
{0xBC,5,{0x53,0x10,0x1A,0x00,0x00}},
{0xBD,5,{0x53,0x10,0x1A,0x00,0x00}},

//CLK Porch Settings
//SETPAGE 5
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xD1,5,{0x03,0x00,0x3D,0x07,0x14}},
{0xD2,5,{0x13,0x00,0x41,0x03,0x16}},
{0xD3,5,{0x20,0x00,0x43,0x07,0x10}},
{0xD4,5,{0x30,0x00,0x43,0x07,0x10}},

//CLK Mask Settings
//SETPAGE 5
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xD0,7,{0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD5,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD6,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD7,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD8,5,{0x00,0x00,0x00,0x00,0x00}},

//VAC Settings
//SETPAGE 5
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xCC,3,{0x00,0x00,0x02}},
{0xCD,3,{0x00,0x00,0x02}},
{0xCE,3,{0x00,0x00,0x02}},
{0xCF,3,{0x00,0x00,0x02}},

//SETPAGE 3
{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
{0xC0,4,{0x00,0x34,0x00,0x00}},
{0xC1,4,{0x00,0x00,0x34,0x00}},
{0xC2,4,{0x00,0x00,0x34,0x00}},
{0xC3,4,{0x00,0x00,0x34,0x00}},

//VDC Settings
//SETPAGE 3
{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
{0xC4,1,{0x60}},
{0xC5,1,{0xC0}},
{0xC6,1,{0x00}},
{0xC7,1,{0x00}},

{0x35,1,{0x00}},
{0x2a,4,{0x00,0x00,0x02,0xcf}},
{0x2b,4,{0x00,0x00,0x04,0xff}},
{0x11,0,{0x00}},
{REGFLAG_DELAY,  120,{}},
{0x29,0,{0x00}},


	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};



static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
    {REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},

    // Sleep Mode On
	{0x10, 0, {0x00}},

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
				//UDELAY(5);//soso add or it will fail to send register
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

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = BURST_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;	

		
		params->dsi.compatibility_for_nvk	= 1;

		params->dsi.vertical_sync_active				= 5;
		params->dsi.vertical_backporch					= 5;
		params->dsi.vertical_frontporch					= 5;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 50;
		params->dsi.horizontal_frontporch				= 50;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		params->dsi.pll_div1=1;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
		params->dsi.fbk_div =30;    //fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
		
		//params->dsi.pll_select = 1;//lenovo jixu add for frehopping

/* Lenovo-sw2 houdz1 add, 20140318 begin */
#ifdef LENOVO_BACKLIGHT_LIMIT
 		params->bl_app.min =1;
  		params->bl_app.def =102;
  		params->bl_app.max =255;
  		params->bl_bsp.min =7;
  		params->bl_bsp.def =102;
  		params->bl_bsp.max =255;
#endif
/* Lenovo-sw2 houdz1 add, 20140318 end */


}

static unsigned int lcm_compare_id(void);

static void lcm_init(void)
{
    SET_RESET_PIN(1);
	MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(10);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(6);//Must > 5ms
    SET_RESET_PIN(1);
    MDELAY(50);//Must > 50ms

	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
	lcm_init();
	
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}
static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int data_array[16];  

	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(10);	

/*	
	data_array[0] = 0x00110500;		// Sleep Out
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
*/
		
//*************Enable CMD2 Page1  *******************//
	data_array[0]=0x00063902;
	data_array[1]=0x52AA55F0;
	data_array[2]=0x00000108;
	dsi_set_cmdq(data_array, 3, 1);
	MDELAY(10); 

	data_array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10); 
	
	read_reg_v2(0xC5, buffer, 2);

	id = buffer[1];
#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, nt35520 id = 0x%x \n", __func__, id);
#else
       printk("%s, nt35520 id = 0x%x \n", __func__, id);
#endif

	return (LCM_ID == id)?1:0;
//return 1;
}

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

	//data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	//dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}

LCM_DRIVER nt35520_hd720_dsi_vdo_cpt_lcm_drv = 
{
    .name			= "nt35520_hd720_dsi_vdo_cpt_lcm_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
        .update         = lcm_update,
#endif
};

