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

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#define REGFLAG_DELAY                                       0XFFE
#define REGFLAG_END_OF_TABLE                                0xFFF   // END OF REGISTERS MARKER


#define LCM_ID_NT35590 (0x90)

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


#define   LCM_DSI_CMD_MODE							0

static struct LCM_setting_table
{
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {

	{0xE0,5,{0x43, 0x00, 0x80, 0x00, 0x00}},                               
	                                                          
	{0x36,1,{0x08}},                                                 
	{0xB3,1,{0x00}},                                                 
	{0xB5,5,{0x20, 0x20, 0x40, 0x00, 0x20}},                         
	{0xB6,5,{0x04, 0x04, 0x0F, 0x16, 0x13}},                         
	                                                           
	{0xD0,9,{0x00, 0x11, 0x64, 0x35, 0x18, 0x06, 0x51, 0x32, 0x02}},
	{0xD1,9,{0x20, 0x14, 0x64, 0x34, 0x01, 0x05, 0x71, 0x33, 0x04}},
	{0xD2,9,{0x00, 0x11, 0x64, 0x35, 0x18, 0x06, 0x51, 0x32, 0x02}},
	{0xD3,9,{0x20, 0x14, 0x64, 0x34, 0x01, 0x05, 0x71, 0x33, 0x04}},
	{0xD4,9,{0x00, 0x11, 0x64, 0x35, 0x18, 0x06, 0x51, 0x32, 0x02}},
	{0xD5,9,{0x20, 0x14, 0x64, 0x34, 0x01, 0x05, 0x71, 0x33, 0x04}},       
	                                                           
	{0x51,1,{0xFF}},                                                
	{0x53,1,{0x2C}},                                                
	{0x55,1,{0x02}},                                                
	{0x5E,1,{0x00}},                                                
	{0xC8,4,{0x82, 0x86, 0x01, 0x11}},                          
	                                                          
	{0x51,1,{0xFF}},                                                
	{0x55,1,{0x00}},                                         
	                                                         
	{0xC0,2,{0x00, 0x00}},                                          
	{0xC3,9,{0x01, 0x09, 0x10, 0x02, 0x00, 0x66, 0x20, 0x13, 0x00}},
	{0xC4,5,{0x23, 0x24, 0x10, 0x10,0x66}},                        
	{0xC6,2,{0x24, 0x40}},                                          
	{0xF9,1,{0x00}},                                                
	{0x11,1,{0x00}},
	{REGFLAG_DELAY, 150,{}},                                                     
	{0x29,1,{0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_in_setting[] =
{
    // Display off sequence
    {0x28, 0, {0x00}},

    // Sleep Mode On
    {0x10, 0, {0x00}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] =
{
    // Sleep Out
    {0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 0, {0x00}},
    {REGFLAG_DELAY, 10, {}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++)
    {

        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd)
        {

            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
				
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
//                dsi_set_cmdq_dcs(cmd, table[i].count, table[i].para_list, force_update);
        }
    }

}

static void init_lcm_registers(void)
{
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void init_lcm_registers2(void)
{
    // NEW CODE FOR TCL FPC5004-7
	unsigned int data_array[16];
#if 1


	data_array[0] = 0x00062902;                          
    data_array[1] = 0x800043E0;
	data_array[2] = 0x00000000;
    dsi_set_cmdq(&data_array, 3, 1);
	//MDELAY(1);
	
	data_array[0] = 0x00022902;                          
    data_array[1] = 0x00000836;
    dsi_set_cmdq(&data_array, 2, 1);
	//MDELAY(1);

	data_array[0] = 0x00022902;                          
    data_array[1] = 0x000000B3;
    dsi_set_cmdq(&data_array, 2, 1);
	//MDELAY(1);

	data_array[0] = 0x00062902;                          
    data_array[1] = 0x402019b5;// 40 20 14 b5
	data_array[2] = 0x00002000;
    dsi_set_cmdq(&data_array, 3, 1);
	//MDELAY(1);

	
	data_array[0] = 0x00062902;                          
    data_array[1] = 0x0F7404B6; 
	data_array[2] = 0x00001316;
    dsi_set_cmdq(&data_array, 3, 1); 
    //MDELAY(1);
	
	data_array[0] = 0x000A2902;                          
    data_array[1] = 0x671321D0; 
	data_array[2] = 0x62060C37;
	data_array[3] = 0x00000323; 
    dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	data_array[0] = 0x000A2902;                          
    data_array[1] = 0x661332D1; 
	data_array[2] = 0x62060237;
	data_array[3] = 0x00000323; 
    dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	data_array[0] = 0x000A2902;                          
    data_array[1] = 0x561341D2; 
	data_array[2] = 0x62060C37;
	data_array[3] = 0x00000323; 
    dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	data_array[0] = 0x000A2902;                          
    data_array[1] = 0x551352D3; 
	data_array[2] = 0x62060237;
	data_array[3] = 0x00000323; 
    dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	data_array[0] = 0x000A2902;                          
    data_array[1] = 0x561341D4; 
	data_array[2] = 0x62060C37;
	data_array[3] = 0x00000323; 
    dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	data_array[0] = 0x000A2902;                          
    data_array[1] = 0x551352D5; 
	data_array[2] = 0x62060237;
	data_array[3] = 0x00000323; 
    dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	

	data_array[0] = 0x00032902;                          
    data_array[1] = 0x000701C0;      // 0a  6   5
    dsi_set_cmdq(&data_array, 2, 1);
    //MDELAY(1);
	 
	data_array[0] = 0x000A2902;                          
    data_array[1] = 0x100901C3; 
	data_array[2] = 0x20660002;
	data_array[3] = 0x00000013;        // 13
    dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);
	
	data_array[0] = 0x00062902;                          
       data_array[1] = 0x172423C4;
	data_array[2] = 0x00006517;
    dsi_set_cmdq(&data_array, 3, 1);
	//MDELAY(1);

	data_array[0] = 0x00032902;                          
    data_array[1] = 0x004024C6; 
    dsi_set_cmdq(&data_array, 2, 1);
    //MDELAY(1);

	data_array[0] = 0x00F92300;                          
    //data_array[1] = 0x000000F9; 
    dsi_set_cmdq(&data_array, 1, 1);
	//MDELAY(1);

	data_array[0] = 0x00022902;                          
    	data_array[1] = 0x00000035; 
    	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00110500; // Sleep Out
	dsi_set_cmdq(&data_array, 1, 1);

	MDELAY(150);

	data_array[0] = 0x00290500; // Display On
	dsi_set_cmdq(&data_array, 1, 1); 
#else
	//unsigned int data_array[16];

	data_array[0] = 0x00062902;                          
       data_array[1] = 0x800043E0;
	data_array[2] = 0x00000000;
       dsi_set_cmdq(&data_array, 3, 1);
	//MDELAY(1);
	

	data_array[0] = 0x00022902;                          
       data_array[1] = 0x00000836;
//	data_array[2] = 0x00002000;
	//data_array[3] = 0x03240FFF;
	//data_array[4] = 0x20252421;
	//data_array[5] = 0x00000000;
      dsi_set_cmdq(&data_array, 2, 1);
	//MDELAY(1);

	data_array[0] = 0x00022902;                          
       data_array[1] = 0x000000B3;
//	data_array[2] = 0x00002000;
	//data_array[3] = 0x03240FFF;
	//data_array[4] = 0x20252421;
	//data_array[5] = 0x00000000;
       dsi_set_cmdq(&data_array, 2, 1);


	data_array[0] = 0x00062902;                          
       data_array[1] = 0x402020B5;
	data_array[2] = 0x00002000;
	//data_array[3] = 0x03240FFF;
	//data_array[4] = 0x20252421;
	//data_array[5] = 0x00000000;
       dsi_set_cmdq(&data_array, 3, 1);
	//MDELAY(1);

	
	data_array[0] = 0x00062902;                          
       data_array[1] = 0x0F0404B6; 
	data_array[2] = 0x00001316;
	//data_array[3] = 0x26263E36;
	//data_array[4] = 0xE6010A57;
       dsi_set_cmdq(&data_array, 3, 1); 
      //MDELAY(1);
	 



	data_array[0] = 0x000A2902;                          
       data_array[1] = 0x641100D0; 
	data_array[2] = 0x51061835;
	data_array[3] = 0x00000232; 
       dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	data_array[0] = 0x000A2902;                          
       data_array[1] = 0x641420D1; 
	data_array[2] = 0x71050134;
	data_array[3] = 0x00000433; 
       dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	data_array[0] = 0x000A2902;                          
       data_array[1] = 0x641100D2; 
	data_array[2] = 0x51061835;
	data_array[3] = 0x00000232; 
       dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	data_array[0] = 0x000A2902;                          
       data_array[1] = 0x641420D3; 
	data_array[2] = 0x71050134;
	data_array[3] = 0x00000433; 
       dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	data_array[0] = 0x000A2902;                          
       data_array[1] = 0x641100D4; 
	data_array[2] = 0x51061835;
	data_array[3] = 0x00000232; 
       dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);

	data_array[0] = 0x000A2902;                          
       data_array[1] = 0x641420D5; 
	data_array[2] = 0x71050134;
	data_array[3] = 0x00000433; 
       dsi_set_cmdq(&data_array, 4, 1);
	//MDELAY(1);
	
	data_array[0] = 0x00022902;                          
       data_array[1] = 0x0000ff51;
       dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00022902;                          
       data_array[1] = 0x00002c53;
       dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00022902;                          
       data_array[1] = 0x00000255;
       dsi_set_cmdq(&data_array, 2, 1);


	data_array[0] = 0x00022902;                          
       data_array[1] = 0x0000005e;
       dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00052902;                          
       data_array[1] = 0x018682c8;
       data_array[2] = 0x00000011;	   
       dsi_set_cmdq(&data_array, 3, 1);

	data_array[0] = 0x00022902;                          
       data_array[1] = 0x0000ff51;
       dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00022902;                          
       data_array[1] = 0x00000055;
       dsi_set_cmdq(&data_array, 2, 1);


	data_array[0] = 0x00032902;                          
       data_array[1] = 0x000000c0;
       dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x000A3902;                          
       data_array[1] = 0x100901C3; 
	data_array[2] = 0x20660002;
	data_array[3] = 0x00000013;        // 13
       dsi_set_cmdq(&data_array, 4, 1);

	data_array[0] = 0x00063902;                          
       data_array[1] = 0x172423C4;
	data_array[2] = 0x00005717;
       dsi_set_cmdq(&data_array, 3, 1);

	data_array[0] = 0x00033902;                          
       data_array[1] = 0x004024C6; 
       dsi_set_cmdq(&data_array, 2, 1);



	data_array[0] = 0x00F92300;                          
    //data_array[1] = 0x000000F9; 
    dsi_set_cmdq(&data_array, 1, 1);
	//MDELAY(1);

	//data_array[0] = 0x00B32300;                          
    //data_array[1] = 0x000002C2; 
    //dsi_set_cmdq(&data_array, 1, 1);


	data_array[0] = 0x00110500; // Sleep Out
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(150);


	
	data_array[0] = 0x00290500; // Display On
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(10);
#endif
}


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

        //SSD2075 has no TE Pin
    // enable tearing-free
        params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
        params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        params->dsi.mode   = BURST_VDO_MODE;	

	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;
	
	// Video mode setting		
        params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

        params->dsi.vertical_sync_active				= 10;  //---3
        params->dsi.vertical_backporch					= 20 ; //---14   12	zhuxiankun 12
        params->dsi.vertical_frontporch					= 10;  //----8
    params->dsi.vertical_active_line				= FRAME_HEIGHT;

        params->dsi.horizontal_sync_active				= 23;  //----2
        params->dsi.horizontal_backporch				= 120; //----28
        params->dsi.horizontal_frontporch				= 16; //----50
	params->dsi.horizontal_active_pixel 			= FRAME_WIDTH;
        params->dsi.pll_select=1;//0: MIPI_PLL; 1: LVDS_PLL

        params->dsi.HS_PRPR=3;
        params->dsi.CLK_HS_POST = 29;//45;//22;//22
        params->dsi.DA_HS_EXIT =35;

        // Bit rate calculation
        params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
        params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
        params->dsi.fbk_div =17;    // 19fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
        params->dsi.cont_clock = 1;//continue clk
}

static void lcm_init(void)
{

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	
	SET_RESET_PIN(1);
	MDELAY(130);      

	init_lcm_registers();

//	init_lcm_registers2();

}


static void lcm_suspend(void)
{
	/*
	unsigned int data_array[16];


//	push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
//	data_array[0]=0x00280500; // Display Off
//	dsi_set_cmdq(data_array, 1, 1);
//	
//	data_array[0] = 0x00100500; // Sleep In
//	dsi_set_cmdq(data_array, 1, 1);


       MDELAY(11);
	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(&data_array, 1, 1);
	
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(&data_array, 1, 1);
        MDELAY(120);
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(5);

	SET_RESET_PIN(1);
	MDELAY(130); 	 */

		unsigned int data_array[16];




	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(&data_array, 1, 1);
	
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(&data_array, 1, 1);
        MDELAY(100);
//	SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
//    SET_RESET_PIN(1);
  //  MDELAY(20);

}


static void lcm_resume(void)
{
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(5);

	SET_RESET_PIN(1);
	MDELAY(130); 	 
//lcm_initialization_setting[20].para_list[4] +=1;
	init_lcm_registers();
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
	dsi_set_cmdq(&data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(&data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(&data_array, 1, 0);

}
#endif

static unsigned int lcm_compare_id(void)
{
	return 1;
//	unsigned int id=0;
//	unsigned char buffer[2];
//	unsigned int array[16];  

//	SET_RESET_PIN(1);
//	SET_RESET_PIN(0);
//	MDELAY(1);
//	
//	SET_RESET_PIN(1);
//	MDELAY(20); 

//	array[0] = 0x00023700;// read id return two byte,version and id
//	dsi_set_cmdq(array, 1, 1);
//	
//	read_reg_v2(0xF4, buffer, 2);
//	id = buffer[0]; //we only need ID
//    #ifdef BUILD_LK
//		printf("%s, LK nt35590 debug: nt35590 id = 0x%08x\n", __func__, id);
//    #else
//		printk("%s, kernel nt35590 horse debug: nt35590 id = 0x%08x\n", __func__, id);
//    #endif

//    if(id == LCM_ID_NT35590)
//    	return 1;
//    else
//        return 0;
}

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
//#if 1
	char  buffer[3];
	int   array[4];

	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x0a, buffer, 1);
	printk("zhuoshineng test lcm_esd_check buffer[0]=0x%x\n",buffer[0]);
	if(buffer[0]==0x9c)
	{
		return FALSE;
	}
	else
	{	
		return TRUE;
	}
#else
	return FALSE;
#endif
}



static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK	
	init_lcm_registers();
#endif
	return TRUE;
}

LCM_DRIVER lg4591_dsi_vdo_hd_lcm_drv = 
{
    .name			= "lg4591_dsi_vdo_hd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	//.esd_check   = lcm_esd_check,
     //   .esd_recover   = lcm_esd_recover,	
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };