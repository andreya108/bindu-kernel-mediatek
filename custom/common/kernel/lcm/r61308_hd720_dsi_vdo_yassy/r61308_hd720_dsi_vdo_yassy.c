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

//#define LCM_DSI_CMD_MODE			0

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
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
       

static void lcm_init_registers(void)
{
unsigned int data_array[16];

data_array[0] = 0x00022902;
data_array[1] = 0x04B0;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00022902;
data_array[1] = 0x02B3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00052902;
data_array[1] = 0x856352B6;
data_array[2] = 0x00;
dsi_set_cmdq(&data_array, 3, 1);

data_array[0] = 0x00042902;
data_array[1] = 0x010000B7;
dsi_set_cmdq(&data_array, 2, 1);    

data_array[0] = 0x000C2902;
data_array[1] = 0x04FF43C0;
data_array[2] = 0x0707020B;
data_array[3] = 0x00002940;
dsi_set_cmdq(&data_array, 4, 1);

data_array[0] = 0x00082902;
data_array[1] = 0x220250C1;
data_array[2] = 0x11ED0000;
dsi_set_cmdq(&data_array, 3, 1);

data_array[0] = 0x00112902;
data_array[1] = 0x050004C3;
data_array[2] = 0x00008014;
data_array[3] = 0x00900000;
data_array[4] = 0x00000C00;
data_array[5] = 0x5C;
dsi_set_cmdq(&data_array, 6, 1);

data_array[0] = 0x000C2902;
data_array[1] = 0xDD2974D0; //992974D0;
data_array[2] = 0x002B0915;
data_array[3] = 0x0090CCC0;
dsi_set_cmdq(&data_array, 4, 1);

data_array[0] = 0x00102902;
data_array[1] = 0x34244DD1; //22E22ED1;
data_array[2] = 0x77775555; //12111211;
data_array[3] = 0x42753166; //11121122;
data_array[4] = 0x12010686; //0612;
dsi_set_cmdq(&data_array, 5, 1);

data_array[0] = 0x00022902;
data_array[1] = 0xA8D6;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00192902;
data_array[1] = 0x29251CC8;
data_array[2] = 0x1535322D;
data_array[3] = 0x0B0D1014;
data_array[4] = 0x29251C06;
data_array[5] = 0x1535322D;
data_array[6] = 0x0B0D1014;
data_array[7] = 0x06;
dsi_set_cmdq(&data_array, 8, 1);


data_array[0] = 0x00092902;
data_array[1] = 0x422110CB;
data_array[2] = 0xD1C39F7D;
data_array[3] = 0xDF;
dsi_set_cmdq(&data_array, 4, 1);

data_array[0] = 0x00042902;
data_array[1] = 0xFFE8CFCC;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00032902;
data_array[1] = 0x1818D5;
dsi_set_cmdq(data_array, 2, 1);

data_array[0] = 0x00042902;
data_array[1] = 0x313103DE;
dsi_set_cmdq(data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0F51;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2453;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0155;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00062902;
data_array[1] = 0x181A00B8;
data_array[2] = 0x4002;
dsi_set_cmdq(&data_array, 3, 1);

data_array[0] = 0x00072902;
data_array[1] = 0x000009BB;
data_array[2] = 0x560A02;
dsi_set_cmdq(&data_array, 3, 1);

data_array[0] = 0x00351500; 
dsi_set_cmdq(&data_array, 1, 1);

data_array[0] = 0x00110500; //sleep out
dsi_set_cmdq(&data_array, 1, 1);
MDELAY(200);

data_array[0] = 0x00290500; //display on
dsi_set_cmdq(&data_array, 1, 1);
MDELAY(40);

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

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity		= LCM_POLARITY_FALLING;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
#else
	params->dsi.mode = BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;
#endif

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;
	
	// Video mode setting 
	params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 2;
	params->dsi.vertical_backporch					= 9;
	params->dsi.vertical_frontporch					= 5;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 10;
	params->dsi.horizontal_backporch				= 50;
	params->dsi.horizontal_frontporch				= 120;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	//params->dsi.pll_select=1;	//0: MIPI_PLL; 1: LVDS_PLL

	//bit rate
	params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4
	params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4
	params->dsi.fbk_div =13;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		
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



static void lcm_init(void)
{
#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, r61308 \n", __func__);
#else
	printk("%s, r61308\n", __func__);
#endif	

	SET_RESET_PIN(1);
	MDELAY(15);
	SET_RESET_PIN(0);
	MDELAY(15);
	SET_RESET_PIN(1);
	MDELAY(15);

	lcm_init_registers();
}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(20);

	data_array[0] = 0x00280500;  //display off
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(60);

	data_array[0] = 0x00100500;  //sleep in
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(150);
#if 1	
	data_array[0] = 0x00022902;
	data_array[1] = 0x04B0;    
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(60);
	
	data_array[0] = 0x00022902;
	data_array[1] = 0x01B1;    //deep sleep
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(60);
#endif	

}


static void lcm_resume(void)
{
#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, r61308 \n", __func__);
#else
	printk("%s, r61308 \n", __func__);
#endif	
	lcm_init();
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
	data_array[3]= 0x00000000;
	data_array[4]= 0x00053902;
	data_array[5]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[6]= (y1_LSB);
	data_array[7]= 0x00000000;
	data_array[8]= 0x002c3909;

	dsi_set_cmdq(&data_array, 9, 0);

}

#if 1
static unsigned int lcm_compare_id(void)
{
	unsigned int id=0,id1=0;
	unsigned char buffer[3];
	unsigned int data_array[16];  

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(10);//Must over 6 ms
	
	data_array[0] = 0x00022902;
	data_array[1] = 0x04B0;    
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(60);

	data_array[0] = 0x00043700;// return byte number
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xBF, buffer, 4);
	id = buffer[2]; 
	id1= buffer[3];
	id=(id<<8)|id1;
#if defined(BUILD_LK)
	printf("%s, id = 0x%08x\n", __func__, id); //1308
#endif
	return (0x1308 == id)?1:0;

}
#endif



LCM_DRIVER r61308_hd720_dsi_vdo_yassy_lcm_drv = 
{
	.name = "r61308_hd720_yassy",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
};
