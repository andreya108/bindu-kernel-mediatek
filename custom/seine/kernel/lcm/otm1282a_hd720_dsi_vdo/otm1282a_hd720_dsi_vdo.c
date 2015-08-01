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
   BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
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

#define FRAME_WIDTH  				(720)
#define FRAME_HEIGHT 				(1280)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//#define LCM_DSI_CMD_MODE			0

#define LCM_ID     0x1282

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static unsigned int lcm_esd_test = FALSE; ///only for ESD test

static LCM_UTIL_FUNCS lcm_util = {0};
static bool lcm_is_init = false;

#define SET_RESET_PIN(v)    	(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 		(lcm_util.udelay(n))
#define MDELAY(n) 		(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)						lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)			lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) 						lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   		lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
       
static unsigned int lcm_compare_id(void);
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

		params->dsi.vertical_sync_active				= 5;
		params->dsi.vertical_backporch					= 5;
		params->dsi.vertical_frontporch					= 5;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 50;
		params->dsi.horizontal_frontporch				= 50;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		params->dsi.pll_div1=0;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
		params->dsi.fbk_div =16;    //fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
		
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


static void lcm_init_register(void)
{
	unsigned int data_array[16];

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00043902; //EXTC=1
    	data_array[1] = 0x018212FF;                 
    	dsi_set_cmdq(&data_array, 2, 1);    

    	data_array[0] = 0x00023902; //ORISE mode enable
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x008212FF;    
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
   	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000301C; //video mode, bypass
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x00EE00C1;
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000000A5;    
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B300;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000044C0;  //0x11=2dot   ;0x88=4dot
    	dsi_set_cmdq(&data_array, 2, 1);     
//------------------
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000021F5;    
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008100;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000018F5;    
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009100;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009EB0;    
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000000A5;    
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B400;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000040C0;   //10
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000D200;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000020F5;    
    	dsi_set_cmdq(&data_array, 2, 1);     
//--idle mode source setting-------------------------
//begin for disable ram power need remove this
/*
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000009B4;    
    	dsi_set_cmdq(&data_array, 2, 1);     
*///end disable ram power
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000020F6;    
    	dsi_set_cmdq(&data_array, 2, 1);  
   
	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008100;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000066C1;//55=60Hz ;66=65Hz
    	dsi_set_cmdq(&data_array, 2, 1); 

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008100;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000004C4;    
    	dsi_set_cmdq(&data_array, 2, 1);     
//--LTPS Timing setting--------------------------
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008200;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000002A5;    
    	dsi_set_cmdq(&data_array, 2, 1);     

//C01
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000B3902; //11
    	data_array[1] = 0x008B00C0;
	data_array[2] = 0x8B000A0A;
	data_array[3] = 0x00000A0A;
    	dsi_set_cmdq(&data_array, 4, 1);     

//C02
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000D3902; //13
    	data_array[1] = 0x000000C0;
	data_array[2] = 0x061E0003;
	data_array[3] = 0x00000000;
	data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1);     
//C21
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00093902; //9
    	data_array[1] = 0x000382C2;
	data_array[2] = 0x00000000;
	data_array[3] = 0x00000000;
    	dsi_set_cmdq(&data_array, 4, 1);     
//C22
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x010C87C2;
	data_array[2] = 0x0C860607;
	data_array[3] = 0x85060701;
	data_array[4] = 0x0607010D;
    	dsi_set_cmdq(&data_array, 5, 1);     
//C23
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x010D84C2;
	data_array[2] = 0x0B870607;
	data_array[3] = 0x86060701;
	data_array[4] = 0x0607010B;
    	dsi_set_cmdq(&data_array, 5, 1);     
//C24
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000B3902; //11
    	data_array[1] = 0x010D85C2;
	data_array[2] = 0x0D840607;
	data_array[3] = 0x00060701;
    	dsi_set_cmdq(&data_array, 4, 1);     
//
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000EA00;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00043902;
    	data_array[1] = 0x110222C2;
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000FA00;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00043902;
    	data_array[1] = 0x010C00C2;
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000F900;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000008C2;
    	dsi_set_cmdq(&data_array, 2, 1);     
//---------------------------
//C03
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000D000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000D3902; //13
    	data_array[1] = 0x000000C0;
	data_array[2] = 0x061E0003;
	data_array[3] = 0x00000000;
	data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1);     
//C31
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00093902; //9
    	data_array[1] = 0x000382C3;
	data_array[2] = 0x00000000;
	data_array[3] = 0x00000000;
    	dsi_set_cmdq(&data_array, 4, 1);     
//C32
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x010C87C3;
	data_array[2] = 0x0C860607;
	data_array[3] = 0x85060701;
	data_array[4] = 0x0607010D;
    	dsi_set_cmdq(&data_array, 5, 1);     
//C33
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x010D84C3;
	data_array[2] = 0x0B870607;
	data_array[3] = 0x86060701;
	data_array[4] = 0x0607010B;
    	dsi_set_cmdq(&data_array, 5, 1);     
//C34
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000B3902; //11
    	data_array[1] = 0x010D85C3;
	data_array[2] = 0x0D840607;
	data_array[3] = 0x00060701;
    	dsi_set_cmdq(&data_array, 4, 1);     
//
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000EA00;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00043902;
    	data_array[1] = 0x110222C3;
    	dsi_set_cmdq(&data_array, 2, 1);     
//---------------------------
//CB1
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x000000CB;
	data_array[2] = 0x00000000;
	data_array[3] = 0x00000000;
	data_array[4] = 0xF0F00000;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CB2
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x0000F0CB;
	data_array[2] = 0x00C00000;
	data_array[3] = 0x000000F0;
	data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CB3
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x000000CB;
	data_array[2] = 0x00000000;
	data_array[3] = 0xC0505000;
	data_array[4] = 0x00C00000;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CB4
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000C000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x040414CB;
	data_array[2] = 0x04040404;
	data_array[3] = 0x04140404;
	data_array[4] = 0x3B3B1404;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CB5
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000D000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x04143BCB;
	data_array[2] = 0x04353004;
	data_array[3] = 0x141404F7;
	data_array[4] = 0x04041414;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CB6
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000E000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x040404CB;
	data_array[2] = 0x04040404;
	data_array[3] = 0x07151504;
	data_array[4] = 0x00001414;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CB7
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000F000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000D3902; //13
    	data_array[1] = 0x000000CB;
	data_array[2] = 0x0000C000;
	data_array[3] = 0x00000000;
	data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CC1
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000B3902; //11
    	data_array[1] = 0x04030FCC;
	data_array[2] = 0x08070605;
	data_array[3] = 0x00100A09;
    	dsi_set_cmdq(&data_array, 4, 1);     
//CC2
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000B3902; //11
    	data_array[1] = 0x090A0FCC;
	data_array[2] = 0x05060708;
	data_array[3] = 0x00100304;
    	dsi_set_cmdq(&data_array, 4, 1);     
//
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008A00;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000BCD;    
    	dsi_set_cmdq(&data_array, 2, 1);     
//CD1
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x080706CD;
	data_array[2] = 0x142D1409;
	data_array[3] = 0x2D2D2D2D;
	data_array[4] = 0x2D2D2D2D;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CD2
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x2D2D2DCD;
	data_array[2] = 0x0C2C1215;
	data_array[3] = 0x0F0E130B;
	data_array[4] = 0x2D2D2D10;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CD3
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000C000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000B3902; //11
    	data_array[1] = 0x2D2D2DCD;
	data_array[2] = 0x2A292827;
	data_array[3] = 0x002D1D2B;
    	dsi_set_cmdq(&data_array, 4, 1);     
//CD4
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000D000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x040302CD;
	data_array[2] = 0x142D1405;
	data_array[3] = 0x2D2D2D2D;
	data_array[4] = 0x2D2D2D2D;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CD5
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000E000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x2D2D2DCD;
	data_array[2] = 0x0C2C1215;
	data_array[3] = 0x0F0E130B;
	data_array[4] = 0x2D2D2D10;
    	dsi_set_cmdq(&data_array, 5, 1);     
//CD6
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000F000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000B3902; //11
    	data_array[1] = 0x2D2D2DCD;
	data_array[2] = 0x2A292827;
	data_array[3] = 0x002D1D2B;
    	dsi_set_cmdq(&data_array, 4, 1);     
//---------------------------		
//C51
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000B3902; //11
    	data_array[1] = 0xBAD692C5;
	data_array[2] = 0x440A92AB;
	data_array[3] = 0x00884044;
    	dsi_set_cmdq(&data_array, 4, 1);     
//C52
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000B3902; //11
    	data_array[1] = 0xBAD692C5;
	data_array[2] = 0x440A12AB;
	data_array[3] = 0x00884044;
    	dsi_set_cmdq(&data_array, 4, 1);     
//
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x003030D8;    
    	dsi_set_cmdq(&data_array, 2, 1);     
	MDELAY(200);	

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000079D9;    
    	dsi_set_cmdq(&data_array, 2, 1);    
//--Gamma-------------------------
//(0xE1,0x00,0x14,0x25,0x31,0x3b,0x42,0x4e,0x5f,0x69,0x79,0x84,0x8c,0x6e,0x69,0x65,0x5a,0x4b,0x3d,0x33,0x2d,0x27,0x20,0x1e,0x03
//E1
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00193902; //25
    	data_array[1] = 0x251400E1;    
    	data_array[2] = 0x4e423b31;    
    	data_array[3] = 0x8479695F;    
    	data_array[4] = 0x65696e8c;    
    	data_array[5] = 0x333d4b5a;    
    	data_array[6] = 0x1e20272d;    
    	data_array[7] = 0x00000003;    
    	dsi_set_cmdq(&data_array, 8, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1); 
//(0xE2,0x00,0x17,0x23,   0x2f,0x39,0x41,0x4d,  0x5f,0x6b,0x7c,0x87, //0x8f,0x6a,0x65,0x60, 0x56,0x47,0x3a,0x32,0x2d,0x29,0x24,0x23,0x03);
//E2
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
 	data_array[0] = 0x00193902; //25
    	data_array[1] = 0x251400E2;    
    	data_array[2] = 0x4d413a31;    
    	data_array[3] = 0x8479695e;    
    	data_array[4] = 0x64696e8c;    
    	data_array[5] = 0x333c4b59;    
    	data_array[6] = 0x1e20272d;    
    	data_array[7] = 0x00000003;         
    	dsi_set_cmdq(&data_array, 8, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);   
//(0xE3,0x00,0x17,0x23,0x30,0x39,0x41,0x4d,0x5f,0x6b,0x7c,0x88,0x90,0x6a, //0x65,0x60,0x55,0x47,0x3a,0x32,0x2c,0x28,0x25,0x22,0x03);  
//E3
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00193902; //25
    	data_array[1] = 0x251400E3;    
    	data_array[2] = 0x4e423b31;    
    	data_array[3] = 0x8479695F;    
    	data_array[4] = 0x65696e8c;    
    	data_array[5] = 0x333d4b5a;    
    	data_array[6] = 0x1e20272d;    
    	data_array[7] = 0x00000003;     
    	dsi_set_cmdq(&data_array, 8, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1); 
//(0xE4,0x00,0x17,0x23,0x2f,0x39,0x41,0x4d,0x5f,0x6b,0x7c,0x87,0x8f,0x6a, //0x65,0x60,0x56,0x47,0x3a,0x32,0x2d,0x29,0x24,0x23,0x03);        
//E4
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
 	data_array[0] = 0x00193902; //25
    	data_array[1] = 0x251400E4;    
    	data_array[2] = 0x4d413a31;    
    	data_array[3] = 0x8479695e;    
    	data_array[4] = 0x64696e8c;    
    	data_array[5] = 0x333c4b59;    
    	data_array[6] = 0x1e20272d;    
    	data_array[7] = 0x00000003;    
    	dsi_set_cmdq(&data_array, 8, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1); 
//(0xE5,0x00,0x17,0x23,0x30,0x39,0x41,0x4d,0x5f,0x6b,0x7c,0x88,0x90,0x6a, //0x65,0x60,0x55,0x47,0x3a,0x32,0x2c,0x28,0x25,0x22,0x03);    
//E5
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00193902; //25
    	data_array[1] = 0x251400E5;    
    	data_array[2] = 0x4e423b31;    
    	data_array[3] = 0x8479695F;    
    	data_array[4] = 0x65696e8c;    
    	data_array[5] = 0x333d4b5a;    
    	data_array[6] = 0x1e20272d;    
    	data_array[7] = 0x00000003;     
    	dsi_set_cmdq(&data_array, 8, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1); 
//(0xE6,0x00,0x17,0x23,0x2f,0x39,0x41,0x4d,0x5f,0x6b,0x7c,0x87,0x8f,0x6a, //0x65,0x60,0x56,0x47,0x3a,0x32,0x2d,0x29,0x24,0x23,0x03);    
//E6
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
 	data_array[0] = 0x00193902; //25
    	data_array[1] = 0x251400E6;    
    	data_array[2] = 0x4d413a31;    
    	data_array[3] = 0x8479695e;    
    	data_array[4] = 0x64696e8c;    
    	data_array[5] = 0x333c4b59;    
    	data_array[6] = 0x1e20272d;    
    	data_array[7] = 0x00000003;   
    	dsi_set_cmdq(&data_array, 8, 1);     

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
//---------------------------
//image processing setting
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x000000C6;    
    	dsi_set_cmdq(&data_array, 2, 1);     
//orise mode disable
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x000000FF;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00043902;
    	data_array[1] = 0x000000FF;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     

    	data_array[0] = 0x00053902;
    	data_array[1] = 0x0200002A;    
	data_array[2] = 0x000000CF;    	
    	dsi_set_cmdq(&data_array, 3, 1);     
    	data_array[0] = 0x00053902;
    	data_array[1] = 0x0400002B;    
	data_array[2] = 0x000000FF;    	
    	dsi_set_cmdq(&data_array, 3, 1);   
  
	data_array[0] = 0x00351500;//te on
    	dsi_set_cmdq(&data_array, 1, 1); 

     	data_array[0] = 0x00110500;                
    	dsi_set_cmdq(&data_array, 1, 1); 
    	MDELAY(150); 
    	
    	data_array[0] = 0x00290500;                
    	dsi_set_cmdq(&data_array, 1, 1);    
    	MDELAY(20); 

//    	data_array[0] = 0x002C0500;                
//    	dsi_set_cmdq(&data_array, 1, 1);    
//    	MDELAY(20); 

} 

static void lcm_init(void)
{
	mt_set_gpio_out(155, 0);//disable backlight ic
	lcm_is_init = true;

    SET_RESET_PIN(1);
	MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(10);

    lcm_init_register();
}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, lcm_init \n", __func__);
#else
	printk("%s, lcm_init \n", __func__);
#endif	
//begin lenovo jixu add for power consume 20130131
mt_set_gpio_out(GPIO155, 0);//disable backlight ic
//end lenovo jixu add for power consume 20130131
    	data_array[0] = 0x00220500;                
    	dsi_set_cmdq(&data_array, 1, 1);   
MDELAY(50);
    	data_array[0] = 0x00280500;                
    	dsi_set_cmdq(&data_array, 1, 1);    
MDELAY(20);
    	data_array[0] = 0x00100500;                
    	dsi_set_cmdq(&data_array, 1, 1);   
//begin lenovo jixu add for power consume 20130206
		SET_RESET_PIN(0);
//end lenovo jixu add for power consume 20130206
lcm_is_init = false;

}


static void lcm_resume(void)
{
	//lenovo jixu add data_array for fix lcd blurred screen when power on and system resume;
	unsigned int data_array[16];
	int i;

#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, lcm_resume otm1282a\n", __func__);
#else
	printk("%s, lcm_resume otm1282a\n", __func__);
#endif	
	#if 1
	
	if(!lcm_is_init){
	mt_set_gpio_out(GPIO155, 0);
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(50);

	lcm_init_register();
	lcm_is_init = true;
//begin lenovo jixu add for power consume 20130206
for(i = 0; i < 6; i++)
	{
		mt_set_gpio_out(155, 1);
		UDELAY(1);
		mt_set_gpio_out(155, 0);
		UDELAY(1);
	}
	mt_set_gpio_out(155, 1);
	}
//end lenovo jixu add for power consume 20130206
	#else
	data_array[0] = 0x00110500; 			   
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(150); 
	
	data_array[0] = 0x00290500; 			   
	dsi_set_cmdq(&data_array, 1, 1);	
	MDELAY(20); 

//begin lenovo jixu add for power consume 20130131
mt_set_gpio_out(GPIO155, 1);//enable backlight ic
//end lenovo jixu add for power consume 20130131
	#endif
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
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

}

static void lcm_setbacklight(unsigned int level)
{
	unsigned int data_array[16];


	if(level > 255) 
	level = 255;

	data_array[0] = 0x00023902;      
	data_array[1] = (0x51|(level<<8)); 
	dsi_set_cmdq(&data_array, 2, 1); 

}

static unsigned int lcm_esd_check(void)
{
unsigned char buffer[2],ret;

#ifndef BUILD_UBOOT
	if(lcm_esd_test)
	{
	lcm_esd_test = FALSE;
	return TRUE;
	}

	/// please notice: the max return packet size is 1
	/// if you want to change it, you can refer to the following marked code
	/// but read_reg currently only support read no more than 4 bytes....
	/// if you need to read more, please let BinHan knows.
	/*
	unsigned int data_array[16];
	unsigned int max_return_size = 1;

	data_array[0]= 0x00003700 | (max_return_size << 16); 

	dsi_set_cmdq(&data_array, 1, 1);
	*/
	read_reg_v2(0x0A, buffer,2);
	#ifndef BUILD_LK
	printk("[JX] %s 0x0A 0=0x%x 1=0x%x \n",__func__,buffer[0],buffer[1]);
	#endif
	ret = buffer[0]==0x9C?0:1;
	#ifndef BUILD_LK
	printk("[JX] %s ret=%d \n",__func__,ret);
	#endif
	if(ret) return TRUE;

	read_reg_v2(0x0D, buffer,2);
	#ifndef BUILD_LK
	printk("[JX] %s 0x0D 0=0x%x 1=0x%x \n",__func__,buffer[0],buffer[1]);
	#endif
	ret = buffer[0]==0x00?0:1;
	#ifndef BUILD_LK
	printk("[JX] %s ret=%d \n",__func__,ret);
	#endif
	if(ret) return TRUE;

	read_reg_v2(0x0E, buffer,2);
	#ifndef BUILD_LK
	printk("[JX] %s 0x0E 0=0x%x 1=0x%x \n",__func__,buffer[0],buffer[1]);
	#endif
	ret = ((buffer[0])&(0xf0))==0x80?0:1;
	#ifndef BUILD_LK
	printk("[JX] %s ret=%d \n",__func__,ret);
	#endif
	if(ret) return TRUE;
	else return FALSE;
#endif
}

static unsigned int lcm_esd_recover(void)
{
#if 0
	unsigned char para = 0;

	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(50);

	lcm_init_register();

	return TRUE;
#else
lcm_suspend();
MDELAY(50);
lcm_resume();
#endif
}

static unsigned int lcm_compare_id(void)
{
#if 1
	unsigned int id=0;
	unsigned char buffer[4];
	unsigned int data_array[16];  

    SET_RESET_PIN(1);
   MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(50);//Must over 6 ms

   	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00043902; //EXTC=1
    	data_array[1] = 0x018212FF;                 
    	dsi_set_cmdq(&data_array, 2, 1);    

    	data_array[0] = 0x00023902; //ORISE mode enable
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x008212FF;    
    	dsi_set_cmdq(&data_array, 2, 1);  
 
    	data_array[0] = 0x00023902; //ORISE mode enable
    	data_array[1] = 0x00009100;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00009EB0;    
    	dsi_set_cmdq(&data_array, 2, 1);    

	data_array[0] = 0x00043700;
	dsi_set_cmdq(&data_array, 1, 1);

	read_reg_v2(0xA1, buffer, 4);
	id = buffer[3]|(buffer[2]<<8); //we only need ID
//        id = read_reg(0xDA);
#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, otm1282a id = 0x%x\n", __func__, id);
#else
       printk("%s, otm1282a id = 0x%x\n", __func__, id);
#endif

	return (LCM_ID == id)?1:0;
#else
	unsigned int ret = 0;
	ret = mt_get_gpio_in(GPIO154);
#if defined(BUILD_LK)
	printf("%s, [wj]otm1282a GPIO154 = %d \n", __func__, ret);
#endif	

	return (ret == 0)?1:0;
#endif	
}

LCM_DRIVER otm1282a_hd720_dsi_vdo_lcm_drv = 
{
	.name = "otm1282a_hd720_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	.esd_check   = lcm_esd_check,
  .esd_recover   = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
};
