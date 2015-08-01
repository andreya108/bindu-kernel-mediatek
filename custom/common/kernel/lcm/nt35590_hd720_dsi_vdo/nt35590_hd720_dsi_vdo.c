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
/* Lenovo-sw yexm1 add, 20130819 begin */
#if defined(LENOVO_ID_READ)
#ifdef BUILD_LK
#else
	#include <linux/proc_fs.h>
#endif
#endif
/* Lenovo-sw yexm1 add, 20130819 end */

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

#define LCM_DSI_CMD_MODE			0

#define LCM_ID       (0x80)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static unsigned int lcm_esd_test = FALSE; ///only for ESD test

static LCM_UTIL_FUNCS lcm_util = {0};

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
/* Lenovo-sw yexm1 add, 20130819 begin */
#if defined(LENOVO_ID_READ)
#ifdef BUILD_LK
#else
#define NT35590_CONFIG_PROC_FILE     "lcm_config"
static struct proc_dir_entry *nt35590_config_proc = NULL;
#endif
#endif
/* Lenovo-sw yexm1 add, 20130819 end */ 
       
static unsigned int lcm_compare_id(void);
static unsigned int lcm_check_status(void);
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
#if (LCM_DSI_CMD_MODE)
		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
#endif

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = BURST_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_THREE_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		//params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;

		//params->dsi.word_count=720*3;	//DSI CMD mode need set these two bellow params, different to 6577
		//params->dsi.vertical_active_line=1280;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		/* Lenovo-sw yexm1 modify, disable the LVDS_PLL lane, 20130922 */
		params->dsi.pll_select=0;	//0: MIPI_PLL; 1: LVDS_PLL
#if 1
		params->dsi.vertical_sync_active				= 1;//2
		params->dsi.vertical_backporch					= 1;//14
		params->dsi.vertical_frontporch					= 6;//16
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 2;//2
		params->dsi.horizontal_backporch				= 50;//34
		params->dsi.horizontal_frontporch				= 50;//24
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
#endif
		// Bit rate calculation
		params->dsi.pll_div1=0;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
		/*Lenovo-sw yexm1 modify, 20130821 */
		params->dsi.fbk_div =16;    //fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
		params->dsi.compatibility_for_nvk = 1;
		/*Lenovo-sw yexm1 add for ESD recovery, 20130710 */
		params->dsi.noncont_clock = TRUE;
		params->dsi.noncont_clock_period = 1;

}


static void lcm_init_register(void)
{
	unsigned int data_array[16];

//CMD1
data_array[0] = 0x00023902;
data_array[1] = 0x000000FF;
dsi_set_cmdq(&data_array, 2, 1);
//MIPI 3 lane                   
data_array[0] = 0x00023902;
data_array[1] = 0x000002BA;//0x01---2LANE   0x02---3LANE    0x03---4LANE 
dsi_set_cmdq(&data_array, 2, 1);
//MIPI command mode            
data_array[0] = 0x00023902;
data_array[1] = 0x000003C2;//03
dsi_set_cmdq(&data_array, 2, 1);
                                
//CMD2,Page0                 
data_array[0] = 0x00023902;
data_array[1] = 0x000001FF;
dsi_set_cmdq(&data_array, 2, 1);
//Turn on test pump_2012/09/15_update
//data_array[0] = 0x00023902;
//data_array[1] = 0x00003B0A;
//dsi_set_cmdq(&data_array, 2, 1);
//Set VGSW/NB/720x480         
data_array[0] = 0x00023902;
data_array[1] = 0x00003A00;
dsi_set_cmdq(&data_array, 2, 1);
//Charge Pump clock      
data_array[0] = 0x00023902;
data_array[1] = 0x00003301;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005302;
dsi_set_cmdq(&data_array, 2, 1);
//VGL=-6V                      
data_array[0] = 0x00023902;
data_array[1] = 0x00008509;
dsi_set_cmdq(&data_array, 2, 1);
//VGH=+8.6V                    
data_array[0] = 0x00023902;
data_array[1] = 0x0000250E;
dsi_set_cmdq(&data_array, 2, 1);
//turn off VGLO regulator     
data_array[0] = 0x00023902;
data_array[1] = 0x00000A0F;
dsi_set_cmdq(&data_array, 2, 1);
//GVDDP=4V                    
data_array[0] = 0x00023902;
data_array[1] = 0x0000970B;
dsi_set_cmdq(&data_array, 2, 1);
//GVDDN=-4V                    
data_array[0] = 0x00023902;
data_array[1] = 0x0000970C;
dsi_set_cmdq(&data_array, 2, 1);
//VCOMDC3=-0.24V_2012/07/06   
data_array[0] = 0x00023902;
data_array[1] = 0x00008A11;
dsi_set_cmdq(&data_array, 2, 1);
//Open Gate_EQ_Cut2_2012/04/13_u);pdate
data_array[0] = 0x00023902;
data_array[1] = 0x00007B36;
dsi_set_cmdq(&data_array, 2, 1);
//MIPI TA_GO_2011/11/29_upda);te
data_array[0] = 0x00023902;
data_array[1] = 0x00002C71;
dsi_set_cmdq(&data_array, 2, 1);
//CMD2,Page4                 
data_array[0] = 0x00023902;
data_array[1] = 0x000005FF;
dsi_set_cmdq(&data_array, 2, 1);
//Turn off SRAM power in sleep i);n_2012/09/15
//data_array[0] = 0x00023902;
//data_array[1] = 0x000011A2;
//dsi_set_cmdq(&data_array, 2, 1);
//LTPS timing                 
data_array[0] = 0x00023902;
data_array[1] = 0x00000001;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008D02;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008D03;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008D04;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00003005;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00003306;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00007707;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000008;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000009;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000000A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000800B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000C80C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000000D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00001B0E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000070F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005710;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000011;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000012;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00001E13;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000014;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00001A15;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000516;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000017;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00001E18;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000FF19;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000001A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000FC1B;
dsi_set_cmdq(&data_array, 2, 1);
//2012/08/28 update           
data_array[0] = 0x00023902;
data_array[1] = 0x0000901C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000001D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000001E;
dsi_set_cmdq(&data_array, 2, 1);
//2012/08/28 update            
data_array[0] = 0x00023902;
data_array[1] = 0x0000771F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000020;
dsi_set_cmdq(&data_array, 2, 1);
//2012/07/02 Gate inverse      
data_array[0] = 0x00023902;
data_array[1] = 0x00000021;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005522;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000D23;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000A031;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000032;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000B833;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000BB34;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00001135;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000136;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000B37;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000138;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000B39;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000844;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008045;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CC46; 
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000447;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000048;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000049;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000014A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000036C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000036D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00002F6E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000043;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000234B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000014C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00002350;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000151;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00002358;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000159;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000235D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000015E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00002362;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000163;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00002367;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000168;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000089;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000018D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000648E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000208F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008E97;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008C82;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000283;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000ABB;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000ABC;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00002524;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005525;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000526;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00002327;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000128;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00003129;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005D2A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000012B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000002F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00001030;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000012A7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000032D;
dsi_set_cmdq(&data_array, 2, 1);                           
//CMD2,Page0_Gamma2.2_2012/07/06);
data_array[0] = 0x00023902;
data_array[1] = 0x000001FF;
dsi_set_cmdq(&data_array, 2, 1);
//R+                          
data_array[0] = 0x00023902;
data_array[1] = 0x00000075;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00004276;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000077;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005078;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000079;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000687A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000007B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00007D7C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000007D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000907E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000007F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000A080;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000081;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AF82;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000083;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000BD84;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000085;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000C986;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000087;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000F688;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000189;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000198A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000018B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000548C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000018D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000828E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000018F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CD90;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000291;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000A92;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000293;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000B94;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000295;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00004496;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000297;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008298;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000299;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AA9A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000029B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000E09C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000039D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000049E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000039F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000037A0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003A2;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000047A3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003A4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000058A5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003A6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00006BA7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003A9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000082AA;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003AB;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00009CAC;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003AD;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000B7AE;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003AF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CBB0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003B1;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000D1B2;
dsi_set_cmdq(&data_array, 2, 1);
//R-                           
data_array[0] = 0x00023902;
data_array[1] = 0x000000B3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000042B4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000B5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000050B6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000B7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000068B8;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000B9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00007DBA;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000BB;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000090BC;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000BD;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000A0BE;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000BF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AFC0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000C1;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000BDC2;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000C3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000C9C4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000C5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000F6C6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001C7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000019C8;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001C9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000054CA;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001CB;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000082CC;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001CD;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CDCE;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002CF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000AD0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002D1;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000BD2;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002D3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000044D4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002D5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000082D6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002D7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AAD8;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002D9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000E0DA;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003DB;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000004DC;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003DD;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000037DE;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003DF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000047E0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003E1;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000058E2;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003E3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00006BE4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003E5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000082E6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003E7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00009CE8;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003E9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000B7EA;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003EB;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CBEC;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003ED;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000D1EE;
dsi_set_cmdq(&data_array, 2, 1);
//G+                           
data_array[0] = 0x00023902;
data_array[1] = 0x000000EF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000042F0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000F1;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000050F2;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000F3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000068F4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000F5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00007DF6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000F7; 
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000090F8;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000F9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000A0FA;
dsi_set_cmdq(&data_array, 2, 1);
//CMD2,Page1                  
data_array[0] = 0x00023902;
data_array[1] = 0x000002FF; 
dsi_set_cmdq(&data_array, 2, 1);
//CMD2,Page1                  
data_array[0] = 0x00023902;
data_array[1] = 0x00000000;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AF01;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000002;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000BD03;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000004;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000C905;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000006;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000F607;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000108;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00001909;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000010A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000540B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000010C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000820D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000010E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CD0F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000210;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000A11;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000212;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000B13;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000214;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00004415;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000216;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008217;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000218;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AA19;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000021A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000E01B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000031C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000041D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000031E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000371F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000320;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00004721;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000322;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005823;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000324;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00006B25;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000326;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008227;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000328;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00009C29;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000032A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000B72B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000032D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CB2F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000330;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000D131;
dsi_set_cmdq(&data_array, 2, 1);
//G-                            
data_array[0] = 0x00023902;
data_array[1] = 0x00000032;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00004233;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000034;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005035;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000036;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00006837;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000038;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00007D39;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000003A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000903B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000003D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000A03F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000040;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AF41;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000042;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000BD43;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000044;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000C945;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000046;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000F647;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000148;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00001949;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000014A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000544B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000014C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000824D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000014E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CD4F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000250;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000A51;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000252;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000B53;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000254;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00004455;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000256;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008258;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000259;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AA5A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000025B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000E05C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000035D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000045E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000035F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00003760;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000361;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00004762;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000363;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005864;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000365;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00006B66;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000367;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008268;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000369;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00009C6A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000036B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000B76C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000036D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CB6E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000036F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000D170;
dsi_set_cmdq(&data_array, 2, 1);
//B+                           
data_array[0] = 0x00023902;
data_array[1] = 0x00000071;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00004272;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000073;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005074;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000075;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00006876;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000077;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00007D78;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000079;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000907A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000007B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000A07C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000007D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AF7E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000007F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000BD80;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000081;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000C982;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000083;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000F684;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000185;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00001986;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000187;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00005488;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000189;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000828A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000018B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CD8C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000028D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000A8E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000028F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000B90;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000291;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00004492;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000293;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008294;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000295;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AA96;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000297;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000E098;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000399;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000049A;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000039B;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000379C;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000039D;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000479E;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000039F;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000058A0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003A2;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00006BA3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003A4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000082A5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003A6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00009CA7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003A9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000B7AA;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003AB;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CBAC;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003AD;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000D1AE;
dsi_set_cmdq(&data_array, 2, 1);
//B-                            
data_array[0] = 0x00023902;
data_array[1] = 0x000000AF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000042B0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000B1;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000050B2;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000B3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000068B4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000B5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00007DB6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000B7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000090B8;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000B9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000A0BA;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000BB;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AFBC;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000BD;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000BDBE;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000BF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000C9C0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000000C1;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000F6C2;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001C3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000019C4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001C5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000054C6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001C7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000082C8;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001C9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CDCA;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002CB;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000ACC;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002CD;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000BCE;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002CF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000044D0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002D1;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000082D2;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002D3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000AAD4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000002D5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000E0D6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003D7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000004D8;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003D9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000037DA;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003DB;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000047DC;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003DD;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000058DE;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003DF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00006BE0;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003E1;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000082E2;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003E3;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00009CE4;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003E5;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000B7E6;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003E7;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000CBE8;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000003E9;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x0000D1EA;
dsi_set_cmdq(&data_array, 2, 1);
                  
//CMD1                       
data_array[0] = 0x00023902;
data_array[1] = 0x000000FF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001FB;
dsi_set_cmdq(&data_array, 2, 1);
//CMD2,Page0                  
data_array[0] = 0x00023902;
data_array[1] = 0x000001FF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001FB;
dsi_set_cmdq(&data_array, 2, 1);
//CMD2,Page1                  
data_array[0] = 0x00023902;
data_array[1] = 0x000002FF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001FB;
dsi_set_cmdq(&data_array, 2, 1);
//CMD2,Page2                  
data_array[0] = 0x00023902;
data_array[1] = 0x000003FF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001FB;
dsi_set_cmdq(&data_array, 2, 1);
//CMD2,Page3                   
data_array[0] = 0x00023902;
data_array[1] = 0x000004FF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001FB;
dsi_set_cmdq(&data_array, 2, 1);
//CMD2,Page4                   
data_array[0] = 0x00023902;
data_array[1] = 0x000005FF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000001FB;
dsi_set_cmdq(&data_array, 2, 1);
//CMD1                        
data_array[0] = 0x00023902;
data_array[1] = 0x000000FF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00110500;
dsi_set_cmdq(&data_array, 1, 1);
MDELAY(120);                      
//not open CABC                
data_array[0] = 0x00023902;
data_array[1] = 0x0000FF51;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00002C53;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000055;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00290500;
dsi_set_cmdq(&data_array, 1, 1);
//CMD1                         
data_array[0] = 0x00023902;
data_array[1] = 0x000000FF;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00000035;
dsi_set_cmdq(&data_array, 2, 1);

} 

static void lcm_init(void)
{	
#if 0
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(100);

	lcm_check_status();
#ifdef BUILD_LK
    printf("[yexm1]nt35590 yassy init code resume.\n");
#endif
    lcm_init_register();
	lcm_check_status();
#endif
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
mt_set_gpio_out(GPIO155, 0);//diable backlight ic
//end lenovo jixu add for power consume 20130131
    	data_array[0] = 0x00280500;                
    	dsi_set_cmdq(&data_array, 1, 1);    

    	data_array[0] = 0x00100500;                
    	dsi_set_cmdq(&data_array, 1, 1);   
		SET_RESET_PIN(1);
		MDELAY(5);
		SET_RESET_PIN(0);
		MDELAY(10);
		SET_RESET_PIN(1);
		MDELAY(50);
//begin lenovo jixu add for power consume 20130206
		SET_RESET_PIN(0);
//end lenovo jixu add for power consume 20130206
}


static void lcm_resume(void)
{
	int i;
#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, nt35590 yassy lcm_resume \n", __func__);
#else
	printk("%s, nt35590 yassy lcm_resume \n", __func__);
#endif	
#if 1
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	/*Lenovo-sw yexm1 modify 20130923 */
	MDELAY(120);

	lcm_init_register();
//begin lenovo jixu add for power consume 20130206
for(i = 0; i < 6; i++)
	{
		mt_set_gpio_out(155, 1);
		UDELAY(1);
		mt_set_gpio_out(155, 0);
		UDELAY(1);
	}
	mt_set_gpio_out(155, 1);
//end lenovo jixu add for power consume 20130206
#else
	data_array[0] = 0x00110500; 			   
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(150); 
	
	data_array[0] = 0x00290500; 			   
	dsi_set_cmdq(&data_array, 1, 1);	
	MDELAY(20); 
#endif


//begin lenovo jixu add for power consume 20130131
	mt_set_gpio_out(GPIO155, 1);//enable backlight ic
//end lenovo jixu add for power consume 20130131
}

static unsigned int lcm_check_status(void)
{
	unsigned char buffer[2];
	int   array[4];
//	unsigned int i = 0;
	
	array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
#if 0
	while(i < 10){
		read_reg_v2(0x0A, buffer, 1);
#ifdef BUILD_LK
		printf("Check LCM Status: 0x%08x\n", buffer[0]);
#else
		printk("Check LCM Status: 0x%08x\n", buffer[0]);
#endif
		if(buffer[0] != 0x9C)
			init_lcm_registers();
		else
			break;
		i++;
	}
#else
	read_reg_v2(0x0A, buffer, 1);
#ifdef BUILD_LK
	printf("Check LCM Status: 0x%08x\n", buffer[0]);
#else
	printk("Check LCM Status: 0x%08x\n", buffer[0]);
#endif
#endif
	return 0;
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
	ret = buffer[0]==0x81?0:1;
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
	unsigned int ret = 0;
	ret = mt_get_gpio_in(GPIO154);
#if defined(BUILD_LK)
	printf("%s, [yexm1]nt35590 GPIO154 = %d \n", __func__, ret);
#endif	

	return (ret == 0)?1:0;
}

/* Lenovo-sw yexm1 add, 20130819 begin */
#if defined(LENOVO_ID_READ)
#ifdef BUILD_LK
#else
extern LCM_DRIVER nt35590_hd720_dsi_vdo_drv;
static int nt35590_config_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    char *ptr = page;

    ptr += sprintf(ptr, "%s\n", nt35590_hd720_dsi_vdo_drv.name);
	ptr += sprintf(ptr, "%s\n", nt35590_hd720_dsi_vdo_drv.module);

    *eof = 1;
    return (ptr - page);
}

static void lcm_read_id(void)
{
	// Create proc file system
	nt35590_config_proc = create_proc_entry(NT35590_CONFIG_PROC_FILE, 0664, NULL);

	if (nt35590_config_proc == NULL)
	{
	    return;
	}
	else
	{
	    nt35590_config_proc->read_proc = nt35590_config_read_proc;
	}
}
#endif
#endif
/* Lenovo-sw yexm1 add, 20130819 end */

LCM_DRIVER nt35590_hd720_dsi_vdo_drv = 
{
	.name = "nt35590",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	//.esd_check   = lcm_esd_check,
  //.esd_recover   = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
	/* Lenovo-sw yexm1 add, 20130819 begin */
	#if defined(LENOVO_ID_READ)
	#ifdef BUILD_LK
	#else
	.module			= "yassy",
	.read_id   		= lcm_read_id,
	#endif
	#endif
	/* Lenovo-sw yexm1 add, 20130819 end */
};
