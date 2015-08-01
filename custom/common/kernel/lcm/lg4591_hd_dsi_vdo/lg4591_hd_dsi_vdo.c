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
	#include <string.h>
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

#define LCM_ID_lg4591 (0x90)

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

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0x00   // END OF REGISTERS MARKER

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
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
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

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},

	{REGFLAG_DELAY, 50, {}},
	{0x4F, 1, {0x01}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void init_lcm_registers(void)
{
	unsigned int data_array[16];

	data_array[0] = 0x00063902;                          
	data_array[1] = 0x800043E0; 
	data_array[2] = 0x00000000;                
	dsi_set_cmdq(data_array, 3, 1); 

	data_array[0] = 0x00063902;                          
	data_array[1] = 0x402034B5; 
	data_array[2] = 0x00002000;                
	dsi_set_cmdq(data_array, 3, 1); 

	data_array[0] = 0x00063902;                          
	data_array[1] = 0x0F7404B6; 
	data_array[2] = 0x00001316;                
	dsi_set_cmdq(data_array, 3, 1); 

	data_array[0] = 0x00033902;                          
	data_array[1] = 0x000801C0;               
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00C11500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x000A3902;                          
	data_array[1] = 0x100900C3; 
	data_array[2] = 0x00660002;  
	data_array[3] = 0x00000013;               
	dsi_set_cmdq(data_array, 4, 1); 

	data_array[0] = 0x00063902;                          
	data_array[1] = 0x122423C4; 
	data_array[2] = 0x00006012;                
	dsi_set_cmdq(data_array, 3, 1); 

	data_array[0] = 0x000A3902;                          
	data_array[1] = 0x672521D0; 
	data_array[2] = 0x61060A36;  
	data_array[3] = 0x00000323;               
	dsi_set_cmdq(data_array, 4, 1); 

	data_array[0] = 0x000A3902;                          
	data_array[1] = 0x662531D1; 
	data_array[2] = 0x61060536;  
	data_array[3] = 0x00000323;               
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0] = 0x000A3902;                          
	data_array[1] = 0x562641D2; 
	data_array[2] = 0x61060A36;  
	data_array[3] = 0x00000323;               
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0] = 0x000A3902;                          
	data_array[1] = 0x552651D3; 
	data_array[2] = 0x61060536;  
	data_array[3] = 0x00000323;               
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0] = 0x000A3902;                          
	data_array[1] = 0x562641D4; 
	data_array[2] = 0x61060A36;  
	data_array[3] = 0x00000323;               
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0] = 0x000A3902;                          
	data_array[1] = 0x552651D5; 
	data_array[2] = 0x61060536;  
	data_array[3] = 0x00000323;               
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0] = 0x08361500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00F91500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);

	data_array[0] = 0x02C21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x06C21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x4EC21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00110500;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(120);

	data_array[0] = 0x80F91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00290500;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(20);
}

static struct LCM_setting_table lcm_initialization_setting[] = {
#if 0
	{0xE0,  5 ,{0x43,0x00,0x80,0x00,0x00}},
	{0xE2,  2 ,{0x22,0x16}},
	{0xB5,  5 ,{0x59,0x20,0x40,0x00,0x20}},//0x34,0x20,0x40,0x00,0x20, //0x59,0x20,0x40,0x00,0x20	                                       
	{0xB6,  5 ,{0x04,0x74,0x0F,0x16,0x13}},
	{0xC0,  2 ,{0x01,0x08}},
	{0xC1,  1 ,{0x00}},
	{0xC3,  9 ,{0x00,0x09,0x10,0x02,0x00,0x66,0x00,0x12,0x00}},//0x13,0x00
	{0xC4,  5 ,{0x23,0x24,0x12,0x12,0x60}},
	{0xC7,  3 ,{0x10,0x00,0x14}},//LGD ADD 2013 03 26
	{0xD0,  9 ,{0x21,0x25,0x67,0x36,0x0A,0x06,0x61,0x23,0x03}},// LGD changed
	{0xD1,  9 ,{0x31,0x25,0x66,0x36,0x05,0x06,0x61,0x23,0x03}},// LGD changed
	{0xD2,  9 ,{0x41,0x26,0x56,0x36,0x0A,0x06,0x61,0x23,0x03}},// LGD changed
	{0xD3,  9 ,{0x51,0x26,0x55,0x36,0x05,0x06,0x61,0x23,0x03}},// LGD changed
	{0xD4,  9 ,{0x41,0x26,0x56,0x36,0x0A,0x06,0x61,0x23,0x03}},// LGD changed
	{0xD5,  9 ,{0x51,0x26,0x55,0x36,0x05,0x06,0x61,0x23,0x03}},// LGD changed
	{0x36,  1 ,{0x08}},
	{0xF9,  1 ,{0x00}},
	{REGFLAG_DELAY, 20, {}},
#else
	{0xE0,  5 ,{0x43,0x00,0x80,0x00,0x00}},
	{0xE2,  2 ,{0x22,0x16}}, // LGD add: TE is GND; (2013 04 16)
	{0xB5,  5 ,{0x59,0x20,0x40,0x00,0x20}},//0x34,0x20,0x40,0x00,0x20, //0x59,0x20,0x40,0x00,0x20	                                       
	{0xB6,  5 ,{0x04,0x74,0x0F,0x16,0x13}},
	{0xC0,  2 ,{0x01,0x08}},
	{0xC1,  1 ,{0x00}},
	{0xC3,  9 ,{0x00,0x09,0x10,0x02,0x00,0x66,0x00,0x12,0x00}},//0x13,0x00
	{0xC4,  5 ,{0x23,0x24,0x12,0x12,0x60}},
	{0xC7,  3 ,{0x10,0x00,0x14}},//LGD ADD 2013 03 26
	{0xD0,  9 ,{0x21,0x24,0x67,0x46,0x0A,0x06,0x61,0x23,0x03}},// LGD changed
	{0xD1,  9 ,{0x31,0x25,0x66,0x46,0x05,0x06,0x61,0x23,0x03}},// LGD changed
	{0xD2,  9 ,{0x41,0x25,0x56,0x46,0x0A,0x06,0x61,0x23,0x03}},// LGD changed
	{0xD3,  9 ,{0x51,0x26,0x55,0x46,0x05,0x06,0x61,0x23,0x03}},// LGD changed
	{0xD4,  9 ,{0x41,0x25,0x56,0x46,0x0A,0x06,0x61,0x23,0x03}},// LGD changed
	{0xD5,  9 ,{0x51,0x26,0x55,0x46,0x05,0x06,0x61,0x23,0x03}},// LGD changed
	{0x36,  1 ,{0x08}},
	{0xF9,  1 ,{0x00}},
	{REGFLAG_DELAY, 20, {}},
#endif
/*	
	{0x72,  2 ,{0x00,0x0E}},
	{0x73,  3 ,{0x34,0x52,0x00}},
	{0x74,  3 ,{0x05,0x03,0x85}},
	{0x75,  3 ,{0x03,0x00,0x03}},
	{0x76,  3 ,{0x07,0x00,0x03}},
	{0x77,  8 ,{0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F}},
	{0x78,  8 ,{0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40}},
	{0x79,  8 ,{0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40}},
	{0x7A,  8 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0x7B,  8 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0x7C,  8 ,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x70,  1 ,{0x00}},
	{0x71,  4 ,{0x00,0x00,0x01,0x01}},
*/
	{0xC2,  1 ,{0x02}},
	{REGFLAG_DELAY, 20, {}},

	{0xC2,  1 ,{0x06}},
	{REGFLAG_DELAY, 100, {}},

	{REGFLAG_DELAY, 150, {}},

	{0xC2,   1,{0x4E}},
	{REGFLAG_DELAY, 20, {}},

	{0x11,1,{0x00}},
	{REGFLAG_DELAY, 120, {}},

	{0xF9,  1 ,{0x80}},
   	 {REGFLAG_DELAY, 10, {}}, // LGD add

	{0x29,1,{0x00}},
	{REGFLAG_DELAY, 1, {}},
	
//	{0x2C,1,{0x00}},// LGD deleted, Question: What is purpose of this register?

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

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
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = SYNC_EVENT_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
        #endif
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
		params->dsi.vertical_sync_active= 10;
		params->dsi.vertical_backporch= 40;
		params->dsi.vertical_frontporch= 10;
		params->dsi.vertical_active_line=FRAME_HEIGHT;

		params->dsi.horizontal_sync_active = 10; //
		params->dsi.horizontal_backporch = 184; // 60
		params->dsi.horizontal_frontporch = 54; //
		params->dsi.horizontal_active_pixel = FRAME_WIDTH;



		// Bit rate calculation
		params->dsi.pll_div1=0; // div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps 1:273Mbps
		params->dsi.pll_div2=1; // div2=0,1,2,3;div1_real=1,2,4,4 
		params->dsi.fbk_div =17; 

		params->dsi.hs_read =1;//lenovo jixu add 


/* Lenovo-sw2 houdz1 add, 20140124 begin */
#ifdef LENOVO_BACKLIGHT_LIMIT
 		params->bl_app.min =1;
  		params->bl_app.def =102;
  		params->bl_app.max =255;
  		params->bl_bsp.min =7;
  		params->bl_bsp.def =102;
  		params->bl_bsp.max =255;
#endif
/* Lenovo-sw2 houdz1 add, 20140124 end */

}

static void init_lcm_registers_v2(void)
{
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_init(void)
{
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(50);
	init_lcm_registers_v2();

}



static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00C21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00063902;
	data_array[1] = 0x000000C4;
	data_array[2] = 0x00000000;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x02C11500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);
	data_array[0] = 0x03C11500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);

SET_RESET_PIN(0);
}


static void lcm_resume(void)
{
	lcm_init();

    #ifdef BUILD_LK
	  printf("[LK]---cmd---lg4591----%s------\n",__func__);
    #else
	  printk("[KERNEL]---cmd---lg4591----%s------\n",__func__);
    #endif	
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
#if 0
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
		printf("%s, LK lg4591 debug: lg4591 id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel lg4591 horse debug: lg4591 id = 0x%08x\n", __func__, id);
    #endif
/*
    if(id == LCM_ID_lg4591)
    	return 1;
    else
*/
        return 0;
#else
	unsigned int ret = 0;
	ret = mt_get_gpio_in(GPIO154);
#if defined(BUILD_LK)
	printf("%s, [wj]lg4591 GPIO154 = %d \n", __func__, ret);
#endif	

	return (ret == 1)?1:0;

#endif

}


static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
	char  buffer[4];
	int   array[4];
	int ret;

	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}
	read_reg_v2(0x0A, buffer,4);
	#ifndef BUILD_LK
	printk("[JX] %s 0x0A 0=0x%x 1=0x%x 2=0x%x 3=0x%x \n",__func__,buffer[0],buffer[1],buffer[2],buffer[3]);
	#endif
	ret = buffer[0]==0x14?0:1;
	#ifndef BUILD_LK
	printk("[JX] %s ret=%d \n",__func__,ret);
	#endif
	if(ret) return TRUE;

	read_reg_v2(0x0B, buffer,4);
	#ifndef BUILD_LK
	printk("[JX] %s 0x0B 0=0x%x 1=0x%x 2=0x%x 3=0x%x \n",__func__,buffer[0],buffer[1],buffer[2],buffer[3]);
	#endif
	ret = buffer[0]==0x08?0:1;
	#ifndef BUILD_LK
	printk("[JX] %s ret=%d \n",__func__,ret);
	#endif
	if(ret) return TRUE;

	read_reg_v2(0xE0, buffer,4);
	#ifndef BUILD_LK
	printk("[JX] %s 0xE0 0=0x%x 1=0x%x 2=0x%x 3=0x%x \n",__func__,buffer[0],buffer[1],buffer[2],buffer[3]);
	#endif
	ret = buffer[0]==0x43?0:1;
	#ifndef BUILD_LK
	printk("[JX] %s ret=%d \n",__func__,ret);
	#endif
	if(ret) return TRUE;

	read_reg_v2(0xE2, buffer,4);
	#ifndef BUILD_LK
	printk("[JX] %s 0xE2 0=0x%x 1=0x%x 2=0x%x 3=0x%x \n",__func__,buffer[0],buffer[1],buffer[2],buffer[3]);
	#endif
	ret = buffer[0]==0x22?0:1;
	#ifndef BUILD_LK
	printk("[JX] %s ret=%d \n",__func__,ret);
	#endif

	if(ret) return TRUE;
	else return FALSE;
	
 #endif

}

static unsigned int lcm_esd_recover(void)
{
	//lcm_suspend();
	//MDELAY(50); 
	lcm_resume();

	return TRUE;
}



LCM_DRIVER lg4591_hd_dsi_vdo_lcm_drv = 
{
    	.name		= "lg4591_hd_dsi_vdo",
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
