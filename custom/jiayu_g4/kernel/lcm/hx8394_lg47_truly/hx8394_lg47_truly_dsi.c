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
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <linux/xlog.h>
#include <mach/mt_pm_ldo.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)


#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER

#ifndef FALSE
    #define FALSE 0
#endif
#define GPIO_LCD_RST_EN      GPIO131

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
#define LCM_DSI_CMD_MODE									0
#define LCM_ID_HX8394 0x0094
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

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
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
    {0xB9,	3,	{ 0xFF, 0x83, 0x94}},    //TE ON 	
	{0xBC,	1,	{0x07}},    //SLEEP OUT
	{0xBA,	1,	{0x13}}, 
	{0xB1,	15,	{0x01,0x00,0x07,0x87,0x01,0x11,0x11,0x2A,0x30,0x3F,
                 0x3F,0x47,0x12,0x01,0xE6}},//0x29, 0x29,
	{0xB2,	6,	{0x00,0xC8,0x08,0x04,0x00,0x22}},
	{0xD5,	32,	{0x00,0x00,0x00,0x00,0x0A,0x00,0x01,0x00,0xCC,0x00,
				0x00,0x00,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
				0x88,0x88,0x01,0x67,0x45,0x23,0x01,0x23,0x88,0x88,
				0x88,0x88}},
	{0xB4,	22,	{0x80,0x06,0x32,0x10,0x03,0x32,0x15,0x08,0x32,0x10,
                 0x08,0x33,0x04,0x43,0x05,0x37,0x04,0x3F,0x06,0x61,0x61,0x06}},
	{0xB6,	1,	{0x00}},
	{0xE0,	42,	{0x00,0x04,0x06,0x2B,0x33,0x3F,0x13,0x34,0x0A,0x0E,
				 0x0D,0x11,0x13,0x11,0x13,0x10,0x17,0x00,0x04,0x06,
				 0x2B,0x33,0x3F,0x13,0x34,0x0A,0x0E,0x0D,0x11,0x13,
				 0x11,0x13,0x10,0x17,0x0B,0x17,0x07,0x11,0x0B,0x17,
				 0x07,0x11}},//GMMA 2.5
	
	{REGFLAG_DELAY, 5, {}},
                            
           {0xC1,127,{0x01,
                                0x02,0x0D,0x16,0x1E,0x27,0x2F,0x37,0x40,0x47,0x4E,0x56,0x5E,0x65,0x6D,0x75,0x7D,0x84,0x8C,0x93,0x9B,
                                0xA2,0xAA,0xB2,0xB9,0xC1,0xCA,0xD2,0xDA,0xE2,0xE9,0xF1,0xF7,0xFF,0x41,0xA4,0x29,0xF9,0xDE,0x82,0xE0,
                                0xDD,0xC0,
				0x02,0x0C,0x15,0x1D,0x26,0x2E,0x36,0x3E,0x45,0x4D,0x54,0x5C,0x63,0x6B,0x73,0x7A,0x82,0x8A,0x90,0x98,
				0x9F,0xA7,0xAE,0xB6,0xBD,0xC6,0xCC,0xD5,0xDD,0xE5,0xEC,0xF3,0xF8,0x74,0x46,0xFB,0x73,0x57,0x03,0x4B,
				0x77,0xC0,
				0x02,0x0D,0x16,0x1E,0x27,0x2F,0x37,0x40,0x47,0x4E,0x56,0x5E,0x65,0x6D,0x75,0x7D,0x84,0x8C,0x93,0x9B,
				0xA2,0xAA,0xB2,0xB9,0xC1,0xCA,0xD2,0xDA,0xE2,0xE9,0xF1,0xF7,0xFF,0x41,0xA4,0x29,0xF9,0xDE,0x82,0xE0,
				0xDD,0xC0}},
	{REGFLAG_DELAY, 5, {}},


	{0xCC,	1,	{0x09}},	
	{0xC0,	2,	{0x0C,0x17}},			
	{0xC7,	4,	{0x00, 0x10, 0x00, 0x10}},			
	{0x44,	2,	{((FRAME_HEIGHT/2)>>8), ((FRAME_HEIGHT/2)&0xFF)}},
	{0x35,	1,	{0x00}},			
	{0xBF,	4,	{0x06,0x00,0x10,0x04}},

    {0x11,0,{0x00}},
    {REGFLAG_DELAY, 200, {}},
    
	{0x29,0,{0x00}},
    {REGFLAG_DELAY, 50, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


#if 0
static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 50, {}},
    // Sleep Mode On
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},

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
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        //params->dsi.mode   = BURST_VDO_MODE; 
        params->dsi.mode   = SYNC_EVENT_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	
		params->dsi.word_count=720*3;
		params->dsi.vertical_sync_active				= 4;//2//
		params->dsi.vertical_backporch					= 5;//8
		params->dsi.vertical_frontporch					= 20;//6
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		params->dsi.horizontal_sync_active				= 50;//86 20
		params->dsi.horizontal_backporch				= 90;//55 50
		params->dsi.horizontal_frontporch				= 90;//55	50
		params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
		

		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.pll_select=1;
	 params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_253_5;//LCM_DSI_6589_PLL_CLOCK_240_5;//LCM_DSI_6589_PLL_CLOCK_227_5;//this value must be in MTK suggested table 227_5

      // params->dsi.pll_div1 = 0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps  0
	//params->dsi.pll_div2 = 1;		// div2=0,1,2,3;div1_real=1,2,4,4	          	
	//params->dsi.fbk_div = 17;      // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)      17	
		
}

static void lcm_init(void)
{
	unsigned int data_array[16];
#ifdef BUILD_LK
        upmu_set_rg_vgp6_vosel(6);
        upmu_set_rg_vgp6_en(1);
#else
        hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_3000, "LCM");
#endif


        mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
        MDELAY(10);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
        MDELAY(1);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
        MDELAY(50);
		
#if 1
		   			
			 data_array[0]=0x00043902;
			 data_array[1]=0x9483ffb9;
			 dsi_set_cmdq(data_array, 2, 1);
			 
			 data_array[0]=0x00023902;
			 data_array[1]=0x000013ba;
			 dsi_set_cmdq(data_array, 2, 1);
			 
			 data_array[0]=0x00103902;
			 data_array[1]=0x070001b1;
			 data_array[2]=0x11110187;
			 data_array[3]=0x3f3f372f;
			 data_array[4]=0xe6011247;
			 dsi_set_cmdq(data_array, 5, 1);
	 	 		 
			 data_array[0]=0x00173902;
			 data_array[1]=0x320880b4;
			 data_array[2]=0x15320310;
			 data_array[3]=0x08103208;
			 data_array[4]=0x05430433;
			 data_array[5]=0x063f0437;
			 data_array[6]=0x00066161;
			 dsi_set_cmdq(data_array, 7, 1);
	 
			 data_array[0]=0x00073902;
			 data_array[1]=0x08c800b2;
			 data_array[2]=0x00220004;
			 dsi_set_cmdq(data_array, 3, 1);
			 
			 data_array[0]=0x00213902;
			 data_array[1]=0x000000d5;
			 data_array[2]=0x01000a00;
			 data_array[3]=0x0000cc00;
			 data_array[4]=0x88888800;
			 data_array[5]=0x88888888;
			 data_array[6]=0x01888888;
			 data_array[7]=0x01234567;
			 data_array[8]=0x88888823;
			 data_array[9]=0x00000088;
			 dsi_set_cmdq(data_array, 10, 1);	 
			 
			 data_array[0]=0x00033902;
			 data_array[1]=0x00170cc0;
			 dsi_set_cmdq(data_array, 2, 1);
			 
			 data_array[0]=0x00053902;
			 data_array[1]=0x001000c7;
			 data_array[2]=0x00000010;
			 dsi_set_cmdq(data_array, 3, 1);
			 
			 data_array[0]=0x00053902;
			 data_array[1]=0x100006bf;
			 data_array[2]=0x00000004;
			 dsi_set_cmdq(data_array, 3, 1);
			 
			//data_array[0]=0x00023902;
			 //data_array[1]=0x000007bc;
			 //dsi_set_cmdq(data_array, 2, 1);
	 

			 data_array[0]=0x00023902;
			 data_array[1]=0x000009cc;
			 dsi_set_cmdq(data_array, 2, 1);

			 //data_array[0]=0x00000500;
			 //dsi_set_cmdq(data_array, 1, 1);
	 			 
			 //data_array[0]=0x00043902;
			 //data_array[1]=0x800004c6;
			 //dsi_set_cmdq(data_array, 2, 1);
	 
			 data_array[0]=0x002b3902;
			 data_array[1]=0x060400e0;
			 data_array[2]=0x133f332b;
			 data_array[3]=0x0d0e0a34;
			 data_array[4]=0x13111311;
			 data_array[5]=0x04001710;
			 data_array[6]=0x3f332b06;
			 data_array[7]=0x0e0a3413;
			 data_array[8]=0x1113110d;
			 data_array[9]=0x0b171013;
 			 data_array[10]=0x0b110717;
			 data_array[11]=0x00110717;
			 dsi_set_cmdq(data_array, 12, 1);
	 
			 data_array[0]=0x00023902;
			 data_array[1]=0x000000b6;
			 dsi_set_cmdq(data_array, 2, 1);
	 
			 //data_array[0]=0x00023902;
			 //data_array[1]=0x000001e6;
			 //dsi_set_cmdq(data_array, 2, 1);
	 
			 data_array[0]=0x00110500;
			 dsi_set_cmdq(data_array, 1, 1);
			 MDELAY(200);
			 //MDELAY(120);
	 
			 data_array[0]=0x00290500;
			 dsi_set_cmdq(data_array, 1, 1);
			 MDELAY(20);
#endif

     //   push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_suspend(void)
{
	//push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
    unsigned int data_array[16];

    data_array[0]=0x00280500; // Display Off
    dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(20);

    data_array[0] = 0x00100500; // Sleep In
    dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(120);

#ifdef BUILD_LK
    upmu_set_rg_vgp6_en(0);
#else
    hwPowerDown(MT65XX_POWER_LDO_VGP6, "LCM");
#endif
}


static void lcm_resume(void)
{
#ifndef BUILD_LK
    lcm_init();
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}

static unsigned int lcm_compare_id(void)
{
    unsigned int id,id0,id1, id2, id3,id4;
    unsigned char buffer[5];
    unsigned int array[5];
#ifdef BUILD_LK
    upmu_set_rg_vgp6_vosel(6);
    upmu_set_rg_vgp6_en(1);
#else
    hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_2800, "LCM");
#endif

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(10);//Must over 6 ms

    array[0]=0x00043902;
    array[1]=0x9483FFB9;// page enable
    dsi_set_cmdq(&array, 2, 1);
    MDELAY(10);

    array[0]=0x00023902;
    array[1]=0x000013ba;
    dsi_set_cmdq(&array, 2, 1);
    MDELAY(10);

    array[0] = 0x00023700;// return byte number
    dsi_set_cmdq(&array, 1, 1);
    MDELAY(10);

    read_reg_v2(0xF4, buffer, 2);
    id = buffer[0]; 

    return (LCM_ID_HX8394 == id)?1:0;
}
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hx8394_lg47_truly_lcm_drv = 
{
    .name			= "hx8394_lg47",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};
