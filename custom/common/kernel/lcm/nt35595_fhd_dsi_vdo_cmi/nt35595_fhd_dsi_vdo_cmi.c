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

#define FRAME_WIDTH  (1080)
#define FRAME_HEIGHT (1920)
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define LCM_ID_NT35595 (0x95)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static unsigned int lcm_esd_test = FALSE; ///only for ESD test

static LCM_UTIL_FUNCS lcm_util = {0};
static unsigned int lcm_cabcmode_index = 3;
static unsigned int lcm_iemode_index = 0;
static unsigned int lcm_gammamode_index = 0;
static unsigned int lcm_gammamode_index2 = 0;

#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu add
extern int rtc_value;
#endif
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

#define dsi_lcm_set_gpio_out(pin, out)										lcm_util.set_gpio_out(pin, out)
#define dsi_lcm_set_gpio_mode(pin, mode)									lcm_util.set_gpio_mode(pin, mode)
#define dsi_lcm_set_gpio_dir(pin, dir)										lcm_util.set_gpio_dir(pin, dir)
#define dsi_lcm_set_gpio_pull_enable(pin, en)								lcm_util.set_gpio_pull_enable(pin, en)

#define   LCM_DSI_CMD_MODE							0
#define LCM_RGB565 0
static bool lcm_is_init = false;
static bool esd_check=false;
static unsigned int lcm_esd_check(void);
static void lcm_set_pq_mode(void);
static void lcm_setgammamode_in_suspend(unsigned int mode);


static void TC358768_DCS_write_1A_1P(unsigned char cmd, unsigned char para)
{
	unsigned int data_array[16];
	//unsigned char buffer;

#if 0//ndef BUILD_LK

	do {
		data_array[0] =(0x00001500 | (para<<24) | (cmd<<16));
		dsi_set_cmdq(data_array, 1, 1);

		if (cmd == 0xFF)
			break;

		read_reg_v2(cmd, &buffer, 1);

		if(buffer != para)
			printk("%s, data_array = 0x%08x, (cmd, para, back) = (0x%02x, 0x%02x, 0x%02x)\n", __func__, data_array[0], cmd, para, buffer);	

		MDELAY(1);

	} while (buffer != para);

#else

	data_array[0] =(0x00001500 | (para<<24) | (cmd<<16));
	dsi_set_cmdq(data_array, 1, 1);

	//MDELAY(1);

#endif

}
static void TC358768_DCS_write_1A_3P(unsigned char cmd, unsigned char para1, unsigned char para2, unsigned char para3)
{
	unsigned int data_array[16];
	//unsigned char buffer;

#if 0//ndef BUILD_LK

	do {
		data_array[0] =(0x00001500 | (para<<24) | (cmd<<16));
		dsi_set_cmdq(data_array, 1, 1);

		if (cmd == 0xFF)
			break;

		read_reg_v2(cmd, &buffer, 1);

		if(buffer != para)
			printk("%s, data_array = 0x%08x, (cmd, para, back) = (0x%02x, 0x%02x, 0x%02x)\n", __func__, data_array[0], cmd, para, buffer);	

		MDELAY(1);

	} while (buffer != para);

#else
    	data_array[0] = 0x00023902; //ORISE mode enable
	data_array[1] =(0x00000000 | (para3<<24) | (para2<<16) | (para1<<8) | (cmd));
	dsi_set_cmdq(data_array, 2, 1);

	//MDELAY(1);

#endif

}
#define TC358768_DCS_write_1A_0P(cmd)							data_array[0]=(0x00000500 | (cmd<<16)); \
																dsi_set_cmdq(data_array, 1, 1);																									

static void init_lcm_registers(void)
{
unsigned int data_array[16];
//CMD2_P0           
TC358768_DCS_write_1A_1P(0xFF, 0x20);       

TC358768_DCS_write_1A_1P(0x00, 0x01);

TC358768_DCS_write_1A_1P(0x01, 0x55);
TC358768_DCS_write_1A_1P(0x02, 0x45);
TC358768_DCS_write_1A_1P(0x03, 0x55);	

TC358768_DCS_write_1A_1P(0x05, 0x50);

TC358768_DCS_write_1A_1P(0x06, 0xBC);

TC358768_DCS_write_1A_1P(0x07, 0x9E);

TC358768_DCS_write_1A_1P(0x08, 0x0C);

TC358768_DCS_write_1A_1P(0x0B, 0xC3);  //GVDDP  C3 4.8V
TC358768_DCS_write_1A_1P(0x0C, 0xC3);  // GVDDN  C3 4.8V 


TC358768_DCS_write_1A_1P(0x0E, 0xBF);
TC358768_DCS_write_1A_1P(0x0F, 0x9A);

TC358768_DCS_write_1A_1P(0x11, 0x13);
TC358768_DCS_write_1A_1P(0x12, 0x13);

TC358768_DCS_write_1A_1P(0x13, 0x83);

TC358768_DCS_write_1A_1P(0x14, 0x0A);  
TC358768_DCS_write_1A_1P(0x15, 0x1A);
TC358768_DCS_write_1A_1P(0x16, 0x1A);	

TC358768_DCS_write_1A_1P(0x23, 0X00);
TC358768_DCS_write_1A_1P(0x24, 0X00);
TC358768_DCS_write_1A_1P(0x25, 0X00);
TC358768_DCS_write_1A_1P(0x26, 0X00);
TC358768_DCS_write_1A_1P(0x27, 0X00);
TC358768_DCS_write_1A_1P(0x28, 0X00);

TC358768_DCS_write_1A_1P(0x30, 0X60);
TC358768_DCS_write_1A_1P(0x31, 0X52);
TC358768_DCS_write_1A_1P(0x32, 0X3D);

TC358768_DCS_write_1A_1P(0x35, 0X00);

TC358768_DCS_write_1A_1P(0x58, 0x00);      
TC358768_DCS_write_1A_1P(0x59, 0x00);      
TC358768_DCS_write_1A_1P(0x5A, 0x00);      
TC358768_DCS_write_1A_1P(0x5B, 0x00);      
TC358768_DCS_write_1A_1P(0x5C, 0x00);      
TC358768_DCS_write_1A_1P(0x5D, 0x00);      
TC358768_DCS_write_1A_1P(0x5E, 0x00);      
TC358768_DCS_write_1A_1P(0x5F, 0x00);
TC358768_DCS_write_1A_1P(0x60, 0x00);

TC358768_DCS_write_1A_1P(0x69, 0xAA);
TC358768_DCS_write_1A_1P(0x6A, 0x33);
TC358768_DCS_write_1A_1P(0x6B, 0x40);
TC358768_DCS_write_1A_1P(0x6C, 0x33);
TC358768_DCS_write_1A_1P(0x6D, 0x33);

TC358768_DCS_write_1A_1P(0x72, 0x11);//0x31 //D-IC TA-GO

	//7300
	//R+
	TC358768_DCS_write_1A_1P(0x75, 0x01);	
	TC358768_DCS_write_1A_1P(0x76, 0x02);	
	TC358768_DCS_write_1A_1P(0x77, 0x01);
	TC358768_DCS_write_1A_1P(0x78, 0x09);	
	TC358768_DCS_write_1A_1P(0x79, 0x01);
	TC358768_DCS_write_1A_1P(0x7A, 0x17);	
	TC358768_DCS_write_1A_1P(0x7B, 0x01);
	TC358768_DCS_write_1A_1P(0x7C, 0x22);	
	TC358768_DCS_write_1A_1P(0x7D, 0x01);
	TC358768_DCS_write_1A_1P(0x7E, 0x2D);	
	TC358768_DCS_write_1A_1P(0x7F, 0x01);
	TC358768_DCS_write_1A_1P(0x80, 0x38);  
	TC358768_DCS_write_1A_1P(0x81, 0x01);
	TC358768_DCS_write_1A_1P(0x82, 0x43);  
	TC358768_DCS_write_1A_1P(0x83, 0x01);
	TC358768_DCS_write_1A_1P(0x84, 0x4D);  
	TC358768_DCS_write_1A_1P(0x85, 0x01);
	TC358768_DCS_write_1A_1P(0x86, 0x57);  
	TC358768_DCS_write_1A_1P(0x87, 0x01);
	TC358768_DCS_write_1A_1P(0x88, 0x79);  
	TC358768_DCS_write_1A_1P(0x89, 0x01);
	TC358768_DCS_write_1A_1P(0x8A, 0x97);  
	TC358768_DCS_write_1A_1P(0x8B, 0x01);
	TC358768_DCS_write_1A_1P(0x8C, 0xC7);  
	TC358768_DCS_write_1A_1P(0x8D, 0x01);
	TC358768_DCS_write_1A_1P(0x8E, 0xEF);  
	TC358768_DCS_write_1A_1P(0x8F, 0x02);
	TC358768_DCS_write_1A_1P(0x90, 0x2F);  
	TC358768_DCS_write_1A_1P(0x91, 0x02);
	TC358768_DCS_write_1A_1P(0x92, 0x63);  
	TC358768_DCS_write_1A_1P(0x93, 0x02);
	TC358768_DCS_write_1A_1P(0x94, 0x65);  
	TC358768_DCS_write_1A_1P(0x95, 0x02);
	TC358768_DCS_write_1A_1P(0x96, 0x94);  
	TC358768_DCS_write_1A_1P(0x97, 0x02);
	TC358768_DCS_write_1A_1P(0x98, 0xCA);  
	TC358768_DCS_write_1A_1P(0x99, 0x02);
	TC358768_DCS_write_1A_1P(0x9A, 0xE9);  
	TC358768_DCS_write_1A_1P(0x9B, 0x03);
	TC358768_DCS_write_1A_1P(0x9C, 0x12);  
	TC358768_DCS_write_1A_1P(0x9D, 0x03);
	TC358768_DCS_write_1A_1P(0x9E, 0x2B);  
	TC358768_DCS_write_1A_1P(0x9F, 0x03);
	TC358768_DCS_write_1A_1P(0xA0, 0x50);	
	TC358768_DCS_write_1A_1P(0xA2, 0x03);
	TC358768_DCS_write_1A_1P(0xA3, 0x56);  
	TC358768_DCS_write_1A_1P(0xA4, 0x03);
	TC358768_DCS_write_1A_1P(0xA5, 0x64);	
	TC358768_DCS_write_1A_1P(0xA6, 0x03);
	TC358768_DCS_write_1A_1P(0xA7, 0x6B);	
	TC358768_DCS_write_1A_1P(0xA9, 0x03);
	TC358768_DCS_write_1A_1P(0xAA, 0x7A);	
	TC358768_DCS_write_1A_1P(0xAB, 0x03);
	TC358768_DCS_write_1A_1P(0xAC, 0x84);
	TC358768_DCS_write_1A_1P(0xAD, 0x03);
	TC358768_DCS_write_1A_1P(0xAE, 0x8B);	
	TC358768_DCS_write_1A_1P(0xAF, 0x03);
	TC358768_DCS_write_1A_1P(0xB0, 0x96);	
	TC358768_DCS_write_1A_1P(0xB1, 0x03);
	TC358768_DCS_write_1A_1P(0xB2, 0xB9);	
	// R-																				
	TC358768_DCS_write_1A_1P(0xB3, 0x01); 
	TC358768_DCS_write_1A_1P(0xB4, 0x02); 
	TC358768_DCS_write_1A_1P(0xB5, 0x01);
	TC358768_DCS_write_1A_1P(0xB6, 0x09); 
	TC358768_DCS_write_1A_1P(0xB7, 0x01);
	TC358768_DCS_write_1A_1P(0xB8, 0x17); 
	TC358768_DCS_write_1A_1P(0xB9, 0x01);
	TC358768_DCS_write_1A_1P(0xBA, 0x22); 
	TC358768_DCS_write_1A_1P(0xBB, 0x01);
	TC358768_DCS_write_1A_1P(0xBC, 0x2D);
	TC358768_DCS_write_1A_1P(0xBD, 0x01);
	TC358768_DCS_write_1A_1P(0xBE, 0x38);
	TC358768_DCS_write_1A_1P(0xBF, 0x01);
	TC358768_DCS_write_1A_1P(0xC0, 0x43);
	TC358768_DCS_write_1A_1P(0xC1, 0x01);
	TC358768_DCS_write_1A_1P(0xC2, 0x4D);
	TC358768_DCS_write_1A_1P(0xC3, 0x01);
	TC358768_DCS_write_1A_1P(0xC4, 0x57);
	TC358768_DCS_write_1A_1P(0xC5, 0x01);
	TC358768_DCS_write_1A_1P(0xC6, 0x79);
	TC358768_DCS_write_1A_1P(0xC7, 0x01);
	TC358768_DCS_write_1A_1P(0xC8, 0x97);
	TC358768_DCS_write_1A_1P(0xC9, 0x01);
	TC358768_DCS_write_1A_1P(0xCA, 0xC7);
	TC358768_DCS_write_1A_1P(0xCB, 0x01);
	TC358768_DCS_write_1A_1P(0xCC, 0xEF);
	TC358768_DCS_write_1A_1P(0xCD, 0x02);
	TC358768_DCS_write_1A_1P(0xCE, 0x2F);
	TC358768_DCS_write_1A_1P(0xCF, 0x02);
	TC358768_DCS_write_1A_1P(0xD0, 0x63);
	TC358768_DCS_write_1A_1P(0xD1, 0x02);
	TC358768_DCS_write_1A_1P(0xD2, 0x65);
	TC358768_DCS_write_1A_1P(0xD3, 0x02);
	TC358768_DCS_write_1A_1P(0xD4, 0x94);
	TC358768_DCS_write_1A_1P(0xD5, 0x02);
	TC358768_DCS_write_1A_1P(0xD6, 0xCA);
	TC358768_DCS_write_1A_1P(0xD7, 0x02);
	TC358768_DCS_write_1A_1P(0xD8, 0xE9);
	TC358768_DCS_write_1A_1P(0xD9, 0x03);
	TC358768_DCS_write_1A_1P(0xDA, 0x12);
	TC358768_DCS_write_1A_1P(0xDB, 0x03);
	TC358768_DCS_write_1A_1P(0xDC, 0x2B);
	TC358768_DCS_write_1A_1P(0xDD, 0x03);
	TC358768_DCS_write_1A_1P(0xDE, 0x50);
	TC358768_DCS_write_1A_1P(0xDF, 0x03);
	TC358768_DCS_write_1A_1P(0xE0, 0x56);
	TC358768_DCS_write_1A_1P(0xE1, 0x03);
	TC358768_DCS_write_1A_1P(0xE2, 0x64);
	TC358768_DCS_write_1A_1P(0xE3, 0x03);
	TC358768_DCS_write_1A_1P(0xE4, 0x6B);
	TC358768_DCS_write_1A_1P(0xE5, 0x03);
	TC358768_DCS_write_1A_1P(0xE6, 0x7A);
	TC358768_DCS_write_1A_1P(0xE7, 0x03);
	TC358768_DCS_write_1A_1P(0xE8, 0x84);
	TC358768_DCS_write_1A_1P(0xE9, 0x03);
	TC358768_DCS_write_1A_1P(0xEA, 0x8B);
	TC358768_DCS_write_1A_1P(0xEB, 0x03);
	TC358768_DCS_write_1A_1P(0xEC, 0x96);
	TC358768_DCS_write_1A_1P(0xED, 0x03);
	TC358768_DCS_write_1A_1P(0xEE, 0xB9);
	//G+
	TC358768_DCS_write_1A_1P(0xEF, 0x00);  
	TC358768_DCS_write_1A_1P(0xF0, 0x80); 
	TC358768_DCS_write_1A_1P(0xF1, 0x00);
	TC358768_DCS_write_1A_1P(0xF2, 0x92);
	TC358768_DCS_write_1A_1P(0xF3, 0x00);
	TC358768_DCS_write_1A_1P(0xF4, 0xAD);
	TC358768_DCS_write_1A_1P(0xF5, 0x00);
	TC358768_DCS_write_1A_1P(0xF6, 0xC6);
	TC358768_DCS_write_1A_1P(0xF7, 0x00);
	TC358768_DCS_write_1A_1P(0xF8, 0xDA);
	TC358768_DCS_write_1A_1P(0xF9, 0x00);
	TC358768_DCS_write_1A_1P(0xFA, 0xEC);
	
		//Don't Reload MTP
	TC358768_DCS_write_1A_1P(0xFB, 0x01);
	
		//CMD2_P1
	TC358768_DCS_write_1A_1P(0xFF, 0x21);
	
		//Green Gamma Code_Continuous 
	TC358768_DCS_write_1A_1P(0x00, 0x00);
	TC358768_DCS_write_1A_1P(0x01, 0xFC);
	TC358768_DCS_write_1A_1P(0x02, 0x01);
	TC358768_DCS_write_1A_1P(0x03, 0x0B);
	TC358768_DCS_write_1A_1P(0x04, 0x01);
	TC358768_DCS_write_1A_1P(0x05, 0x18);
	TC358768_DCS_write_1A_1P(0x06, 0x01);
	TC358768_DCS_write_1A_1P(0x07, 0x46);
	TC358768_DCS_write_1A_1P(0x08, 0x01);
	TC358768_DCS_write_1A_1P(0x09, 0x6C);
	TC358768_DCS_write_1A_1P(0x0A, 0x01);
	TC358768_DCS_write_1A_1P(0x0B, 0xA7);
	TC358768_DCS_write_1A_1P(0x0C, 0x01);
	TC358768_DCS_write_1A_1P(0x0D, 0xD6);
	TC358768_DCS_write_1A_1P(0x0E, 0x02);
	TC358768_DCS_write_1A_1P(0x0F, 0x1F);
	TC358768_DCS_write_1A_1P(0x10, 0x02);
	TC358768_DCS_write_1A_1P(0x11, 0x57);
	TC358768_DCS_write_1A_1P(0x12, 0x02);
	TC358768_DCS_write_1A_1P(0x13, 0x59);
	TC358768_DCS_write_1A_1P(0x14, 0x02);
	TC358768_DCS_write_1A_1P(0x15, 0x8B);
	TC358768_DCS_write_1A_1P(0x16, 0x02);
	TC358768_DCS_write_1A_1P(0x17, 0xBF);
	TC358768_DCS_write_1A_1P(0x18, 0x02);
	TC358768_DCS_write_1A_1P(0x19, 0xE1);
	TC358768_DCS_write_1A_1P(0x1A, 0x03);
	TC358768_DCS_write_1A_1P(0x1B, 0x0A);
	TC358768_DCS_write_1A_1P(0x1C, 0x03);
	TC358768_DCS_write_1A_1P(0x1D, 0x24);
	TC358768_DCS_write_1A_1P(0x1E, 0x03);
	TC358768_DCS_write_1A_1P(0x1F, 0x48);
	TC358768_DCS_write_1A_1P(0x20, 0x03);
	TC358768_DCS_write_1A_1P(0x21, 0x52);
	TC358768_DCS_write_1A_1P(0x22, 0x03);
	TC358768_DCS_write_1A_1P(0x23, 0x5F);
	TC358768_DCS_write_1A_1P(0x24, 0x03);
	TC358768_DCS_write_1A_1P(0x25, 0x6B);
	TC358768_DCS_write_1A_1P(0x26, 0x03);
	TC358768_DCS_write_1A_1P(0x27, 0x74);
	TC358768_DCS_write_1A_1P(0x28, 0x03);
	TC358768_DCS_write_1A_1P(0x29, 0x7A);
	TC358768_DCS_write_1A_1P(0x2A, 0x03);
	TC358768_DCS_write_1A_1P(0x2B, 0x88);
	TC358768_DCS_write_1A_1P(0x2D, 0x03);
	TC358768_DCS_write_1A_1P(0x2F, 0x96);
	TC358768_DCS_write_1A_1P(0x30, 0x03);
	TC358768_DCS_write_1A_1P(0x31, 0xB9);
	//G-
	TC358768_DCS_write_1A_1P(0x32, 0x00);  
	TC358768_DCS_write_1A_1P(0x33, 0x80);
	TC358768_DCS_write_1A_1P(0x34, 0x00);
	TC358768_DCS_write_1A_1P(0x35, 0x92);
	TC358768_DCS_write_1A_1P(0x36, 0x00);
	TC358768_DCS_write_1A_1P(0x37, 0xAD);
	TC358768_DCS_write_1A_1P(0x38, 0x00);
	TC358768_DCS_write_1A_1P(0x39, 0xC6);
	TC358768_DCS_write_1A_1P(0x3A, 0x00);
	TC358768_DCS_write_1A_1P(0x3B, 0xDA);
	TC358768_DCS_write_1A_1P(0x3D, 0x00);
	TC358768_DCS_write_1A_1P(0x3F, 0xEC);
	TC358768_DCS_write_1A_1P(0x40, 0x00);
	TC358768_DCS_write_1A_1P(0x41, 0xFC);
	TC358768_DCS_write_1A_1P(0x42, 0x01);
	TC358768_DCS_write_1A_1P(0x43, 0x0B);
	TC358768_DCS_write_1A_1P(0x44, 0x01);
	TC358768_DCS_write_1A_1P(0x45, 0x18);
	TC358768_DCS_write_1A_1P(0x46, 0x01);
	TC358768_DCS_write_1A_1P(0x47, 0x46);
	TC358768_DCS_write_1A_1P(0x48, 0x01);
	TC358768_DCS_write_1A_1P(0x49, 0x6C);
	TC358768_DCS_write_1A_1P(0x4A, 0x01);
	TC358768_DCS_write_1A_1P(0x4B, 0xA7);
	TC358768_DCS_write_1A_1P(0x4C, 0x01);
	TC358768_DCS_write_1A_1P(0x4D, 0xD6);
	TC358768_DCS_write_1A_1P(0x4E, 0x02);
	TC358768_DCS_write_1A_1P(0x4F, 0x1F);
	TC358768_DCS_write_1A_1P(0x50, 0x02);
	TC358768_DCS_write_1A_1P(0x51, 0x57);
	TC358768_DCS_write_1A_1P(0x52, 0x02);
	TC358768_DCS_write_1A_1P(0x53, 0x59);
	TC358768_DCS_write_1A_1P(0x54, 0x02);
	TC358768_DCS_write_1A_1P(0x55, 0x8B);
	TC358768_DCS_write_1A_1P(0x56, 0x02);
	TC358768_DCS_write_1A_1P(0x58, 0xBF);
	TC358768_DCS_write_1A_1P(0x59, 0x02);
	TC358768_DCS_write_1A_1P(0x5A, 0xE1);
	TC358768_DCS_write_1A_1P(0x5B, 0x03);
	TC358768_DCS_write_1A_1P(0x5C, 0x0A);
	TC358768_DCS_write_1A_1P(0x5D, 0x03);
	TC358768_DCS_write_1A_1P(0x5E, 0x24);
	TC358768_DCS_write_1A_1P(0x5F, 0x03);
	TC358768_DCS_write_1A_1P(0x60, 0x48);
	TC358768_DCS_write_1A_1P(0x61, 0x03);
	TC358768_DCS_write_1A_1P(0x62, 0x52);
	TC358768_DCS_write_1A_1P(0x63, 0x03);
	TC358768_DCS_write_1A_1P(0x64, 0x5F);
	TC358768_DCS_write_1A_1P(0x65, 0x03);
	TC358768_DCS_write_1A_1P(0x66, 0x6B);
	TC358768_DCS_write_1A_1P(0x67, 0x03);
	TC358768_DCS_write_1A_1P(0x68, 0x74);
	TC358768_DCS_write_1A_1P(0x69, 0x03);
	TC358768_DCS_write_1A_1P(0x6A, 0x7A);
	TC358768_DCS_write_1A_1P(0x6B, 0x03);
	TC358768_DCS_write_1A_1P(0x6C, 0x88);
	TC358768_DCS_write_1A_1P(0x6D, 0x03);
	TC358768_DCS_write_1A_1P(0x6E, 0x96);
	TC358768_DCS_write_1A_1P(0x6F, 0x03);
	TC358768_DCS_write_1A_1P(0x70, 0xB9);
	
	//B+	
	TC358768_DCS_write_1A_1P(0x71, 0x00);  
	TC358768_DCS_write_1A_1P(0x72, 0x0A);
	TC358768_DCS_write_1A_1P(0x73, 0x00);
	TC358768_DCS_write_1A_1P(0x74, 0x41);
	TC358768_DCS_write_1A_1P(0x75, 0x00);
	TC358768_DCS_write_1A_1P(0x76, 0x77);
	TC358768_DCS_write_1A_1P(0x77, 0x00);
	TC358768_DCS_write_1A_1P(0x78, 0x9B);
	TC358768_DCS_write_1A_1P(0x79, 0x00);
	TC358768_DCS_write_1A_1P(0x7A, 0xB6);
	TC358768_DCS_write_1A_1P(0x7B, 0x00);
	TC358768_DCS_write_1A_1P(0x7C, 0xCD);
	TC358768_DCS_write_1A_1P(0x7D, 0x00);
	TC358768_DCS_write_1A_1P(0x7E, 0xE2);
	TC358768_DCS_write_1A_1P(0x7F, 0x00);
	TC358768_DCS_write_1A_1P(0x80, 0xF4);
	TC358768_DCS_write_1A_1P(0x81, 0x01);
	TC358768_DCS_write_1A_1P(0x82, 0x03);
	TC358768_DCS_write_1A_1P(0x83, 0x01);
	TC358768_DCS_write_1A_1P(0x84, 0x38);
	TC358768_DCS_write_1A_1P(0x85, 0x01);
	TC358768_DCS_write_1A_1P(0x86, 0x61);
	TC358768_DCS_write_1A_1P(0x87, 0x01);
	TC358768_DCS_write_1A_1P(0x88, 0xA1);
	TC358768_DCS_write_1A_1P(0x89, 0x01);
	TC358768_DCS_write_1A_1P(0x8A, 0xD2);
	TC358768_DCS_write_1A_1P(0x8B, 0x02);
	TC358768_DCS_write_1A_1P(0x8C, 0x16);
	TC358768_DCS_write_1A_1P(0x8D, 0x02);
	TC358768_DCS_write_1A_1P(0x8E, 0x53);
	TC358768_DCS_write_1A_1P(0x8F, 0x02);
	TC358768_DCS_write_1A_1P(0x90, 0x58);
	TC358768_DCS_write_1A_1P(0x91, 0x02);
	TC358768_DCS_write_1A_1P(0x92, 0x8A);
	TC358768_DCS_write_1A_1P(0x93, 0x02);
	TC358768_DCS_write_1A_1P(0x94, 0xC0);
	TC358768_DCS_write_1A_1P(0x95, 0x02);
	TC358768_DCS_write_1A_1P(0x96, 0xDE);
	TC358768_DCS_write_1A_1P(0x97, 0x03);
	TC358768_DCS_write_1A_1P(0x98, 0x0A);
	TC358768_DCS_write_1A_1P(0x99, 0x03);
	TC358768_DCS_write_1A_1P(0x9A, 0x22);
	TC358768_DCS_write_1A_1P(0x9B, 0x03);
	TC358768_DCS_write_1A_1P(0x9C, 0x48);
	TC358768_DCS_write_1A_1P(0x9D, 0x03);
	TC358768_DCS_write_1A_1P(0x9E, 0x52);
	TC358768_DCS_write_1A_1P(0x9F, 0x03);
	TC358768_DCS_write_1A_1P(0xA0, 0x5D);
	TC358768_DCS_write_1A_1P(0xA2, 0x03);
	TC358768_DCS_write_1A_1P(0xA3, 0x6A);
	TC358768_DCS_write_1A_1P(0xA4, 0x03);
	TC358768_DCS_write_1A_1P(0xA5, 0x76);
	TC358768_DCS_write_1A_1P(0xA6, 0x03);
	TC358768_DCS_write_1A_1P(0xA7, 0x80);
	TC358768_DCS_write_1A_1P(0xA9, 0x03);
	TC358768_DCS_write_1A_1P(0xAA, 0x87);
	TC358768_DCS_write_1A_1P(0xAB, 0x03);
	TC358768_DCS_write_1A_1P(0xAC, 0x96);
	TC358768_DCS_write_1A_1P(0xAD, 0x03);
	TC358768_DCS_write_1A_1P(0xAE, 0xB9);
	//B-
	TC358768_DCS_write_1A_1P(0xAF, 0x00); 
	TC358768_DCS_write_1A_1P(0xB0, 0x0A);
	TC358768_DCS_write_1A_1P(0xB1, 0x00);
	TC358768_DCS_write_1A_1P(0xB2, 0x41);
	TC358768_DCS_write_1A_1P(0xB3, 0x00);
	TC358768_DCS_write_1A_1P(0xB4, 0x77);
	TC358768_DCS_write_1A_1P(0xB5, 0x00);
	TC358768_DCS_write_1A_1P(0xB6, 0x9B);
	TC358768_DCS_write_1A_1P(0xB7, 0x00);
	TC358768_DCS_write_1A_1P(0xB8, 0xB6);
	TC358768_DCS_write_1A_1P(0xB9, 0x00);
	TC358768_DCS_write_1A_1P(0xBA, 0xCD);
	TC358768_DCS_write_1A_1P(0xBB, 0x00);
	TC358768_DCS_write_1A_1P(0xBC, 0xE2);
	TC358768_DCS_write_1A_1P(0xBD, 0x00);
	TC358768_DCS_write_1A_1P(0xBE, 0xF4);
	TC358768_DCS_write_1A_1P(0xBF, 0x01);
	TC358768_DCS_write_1A_1P(0xC0, 0x03);
	TC358768_DCS_write_1A_1P(0xC1, 0x01);
	TC358768_DCS_write_1A_1P(0xC2, 0x38);
	TC358768_DCS_write_1A_1P(0xC3, 0x01);
	TC358768_DCS_write_1A_1P(0xC4, 0x61);
	TC358768_DCS_write_1A_1P(0xC5, 0x01);
	TC358768_DCS_write_1A_1P(0xC6, 0xA1);
	TC358768_DCS_write_1A_1P(0xC7, 0x01);
	TC358768_DCS_write_1A_1P(0xC8, 0xD2);
	TC358768_DCS_write_1A_1P(0xC9, 0x02);
	TC358768_DCS_write_1A_1P(0xCA, 0x16);
	TC358768_DCS_write_1A_1P(0xCB, 0x02);
	TC358768_DCS_write_1A_1P(0xCC, 0x53);
	TC358768_DCS_write_1A_1P(0xCD, 0x02);
	TC358768_DCS_write_1A_1P(0xCE, 0x58);
	TC358768_DCS_write_1A_1P(0xCF, 0x02);
	TC358768_DCS_write_1A_1P(0xD0, 0x8A);
	TC358768_DCS_write_1A_1P(0xD1, 0x02);
	TC358768_DCS_write_1A_1P(0xD2, 0xC0);
	TC358768_DCS_write_1A_1P(0xD3, 0x02);
	TC358768_DCS_write_1A_1P(0xD4, 0xDE);
	TC358768_DCS_write_1A_1P(0xD5, 0x03);
	TC358768_DCS_write_1A_1P(0xD6, 0x0A);
	TC358768_DCS_write_1A_1P(0xD7, 0x03);
	TC358768_DCS_write_1A_1P(0xD8, 0x22);
	TC358768_DCS_write_1A_1P(0xD9, 0x03);
	TC358768_DCS_write_1A_1P(0xDA, 0x48);
	TC358768_DCS_write_1A_1P(0xDB, 0x03);
	TC358768_DCS_write_1A_1P(0xDC, 0x52);
	TC358768_DCS_write_1A_1P(0xDD, 0x03);
	TC358768_DCS_write_1A_1P(0xDE, 0x5D);
	TC358768_DCS_write_1A_1P(0xDF, 0x03);
	TC358768_DCS_write_1A_1P(0xE0, 0x6A);
	TC358768_DCS_write_1A_1P(0xE1, 0x03);
	TC358768_DCS_write_1A_1P(0xE2, 0x76);
	TC358768_DCS_write_1A_1P(0xE3, 0x03);
	TC358768_DCS_write_1A_1P(0xE4, 0x80);
	TC358768_DCS_write_1A_1P(0xE5, 0x03);
	TC358768_DCS_write_1A_1P(0xE6, 0x87);
	TC358768_DCS_write_1A_1P(0xE7, 0x03);
	TC358768_DCS_write_1A_1P(0xE8, 0x96);
	TC358768_DCS_write_1A_1P(0xE9, 0x03);
	TC358768_DCS_write_1A_1P(0xEA, 0xB9);
		 

//Don't Reload MTP
TC358768_DCS_write_1A_1P(0xFB, 0x01);
/////add for PWM frequence
TC358768_DCS_write_1A_1P(0xFF, 0x23);
TC358768_DCS_write_1A_1P(0x03,0x02);//dimming step for FABC
TC358768_DCS_write_1A_1P(0x05,0x29);//DIM_STEP_MOV = 64 step
TC358768_DCS_write_1A_1P(0x08, 0x04);
TC358768_DCS_write_1A_1P(0xFB, 0x01);
/////

//CMD2_P4           
TC358768_DCS_write_1A_1P(0xFF, 0x24);        

TC358768_DCS_write_1A_1P(0x00, 0x01);

TC358768_DCS_write_1A_1P(0x01, 0x03);

TC358768_DCS_write_1A_1P(0x02, 0x04);

TC358768_DCS_write_1A_1P(0x03, 0x05);

TC358768_DCS_write_1A_1P(0x04, 0x06);

TC358768_DCS_write_1A_1P(0x05, 0x00);

TC358768_DCS_write_1A_1P(0x06, 0x00);

TC358768_DCS_write_1A_1P(0x07, 0x0F);

TC358768_DCS_write_1A_1P(0x08, 0x10);

TC358768_DCS_write_1A_1P(0x09, 0x0D);

TC358768_DCS_write_1A_1P(0x0A, 0x13);

TC358768_DCS_write_1A_1P(0x0B, 0x14);

TC358768_DCS_write_1A_1P(0x0C, 0x15);

TC358768_DCS_write_1A_1P(0x0D, 0x16);

TC358768_DCS_write_1A_1P(0x0E, 0x17);

TC358768_DCS_write_1A_1P(0x0F, 0x18);

TC358768_DCS_write_1A_1P(0x10, 0x01);

TC358768_DCS_write_1A_1P(0x11, 0x03);

TC358768_DCS_write_1A_1P(0x12, 0x04);

TC358768_DCS_write_1A_1P(0x13, 0x05);

TC358768_DCS_write_1A_1P(0x14, 0x06);

TC358768_DCS_write_1A_1P(0x15, 0x00);

TC358768_DCS_write_1A_1P(0x16, 0x00);

TC358768_DCS_write_1A_1P(0x17, 0x0F);

TC358768_DCS_write_1A_1P(0x18, 0x10);

TC358768_DCS_write_1A_1P(0x19, 0x0D);

TC358768_DCS_write_1A_1P(0x1A, 0x13);

TC358768_DCS_write_1A_1P(0x1B, 0x14);

TC358768_DCS_write_1A_1P(0x1C, 0x15);

TC358768_DCS_write_1A_1P(0x1D, 0x16);

TC358768_DCS_write_1A_1P(0x1E, 0x17);

TC358768_DCS_write_1A_1P(0x1F, 0x18);

TC358768_DCS_write_1A_1P(0x20, 0x49);

TC358768_DCS_write_1A_1P(0x21, 0x03);

TC358768_DCS_write_1A_1P(0x22, 0x01);

TC358768_DCS_write_1A_1P(0x23, 0x00);

TC358768_DCS_write_1A_1P(0x24, 0x68);

TC358768_DCS_write_1A_1P(0x26, 0x00);

TC358768_DCS_write_1A_1P(0x27, 0x68);

TC358768_DCS_write_1A_1P(0x25, 0x5D); 

TC358768_DCS_write_1A_1P(0x2F, 0x02);

TC358768_DCS_write_1A_1P(0x30, 0x23);

TC358768_DCS_write_1A_1P(0x31, 0x49);

TC358768_DCS_write_1A_1P(0x32, 0x48);

TC358768_DCS_write_1A_1P(0x33, 0x41);

TC358768_DCS_write_1A_1P(0x34, 0x01);

TC358768_DCS_write_1A_1P(0x35, 0x75);

TC358768_DCS_write_1A_1P(0x36, 0x00);

TC358768_DCS_write_1A_1P(0x37, 0x1D);

TC358768_DCS_write_1A_1P(0x38, 0x08);

TC358768_DCS_write_1A_1P(0x39, 0x01);

TC358768_DCS_write_1A_1P(0x3A, 0x75);


TC358768_DCS_write_1A_1P(0x5B, 0x1A);//0x54 //power on sequence

TC358768_DCS_write_1A_1P(0x5C, 0x00);
TC358768_DCS_write_1A_1P(0x5D, 0x00);
TC358768_DCS_write_1A_1P(0x5E, 0x00);

TC358768_DCS_write_1A_1P(0x5F, 0x6D);

TC358768_DCS_write_1A_1P(0x60, 0x6D);
TC358768_DCS_write_1A_1P(0x61, 0x00);
TC358768_DCS_write_1A_1P(0x62, 0x00);
TC358768_DCS_write_1A_1P(0x63, 0x00);
TC358768_DCS_write_1A_1P(0x64, 0x00);
TC358768_DCS_write_1A_1P(0x65, 0x00);
TC358768_DCS_write_1A_1P(0x66, 0x00);
TC358768_DCS_write_1A_1P(0x67, 0x06);//0x04 //power off sequence
TC358768_DCS_write_1A_1P(0x68, 0x06);//0x02 //power off sequence
TC358768_DCS_write_1A_1P(0x69, 0x04);
TC358768_DCS_write_1A_1P(0x6A, 0x04);
TC358768_DCS_write_1A_1P(0x6E,0x10);//disable ram keep add by jixu

TC358768_DCS_write_1A_1P(0xF4, 0xD0);
TC358768_DCS_write_1A_1P(0xF5, 0x01);
TC358768_DCS_write_1A_1P(0xF6, 0x01);
TC358768_DCS_write_1A_1P(0xF7, 0x00);
TC358768_DCS_write_1A_1P(0xF8, 0x01);
TC358768_DCS_write_1A_1P(0xF9, 0x00);

TC358768_DCS_write_1A_1P(0x74, 0x02);
TC358768_DCS_write_1A_1P(0x75, 0x1C);
TC358768_DCS_write_1A_1P(0x76, 0x04);
TC358768_DCS_write_1A_1P(0x77, 0x03);

TC358768_DCS_write_1A_1P(0x7A, 0x00);
TC358768_DCS_write_1A_1P(0x7B, 0x91);
TC358768_DCS_write_1A_1P(0x7C, 0xD8);
TC358768_DCS_write_1A_1P(0x7D, 0x10);//0x50 //power off sequence

TC358768_DCS_write_1A_1P(0x7E, 0x02);
TC358768_DCS_write_1A_1P(0x7F, 0x1C);

TC358768_DCS_write_1A_1P(0x81, 0x04);
TC358768_DCS_write_1A_1P(0x82, 0x03);

TC358768_DCS_write_1A_1P(0x86, 0x1B);
TC358768_DCS_write_1A_1P(0x87, 0x1B);
TC358768_DCS_write_1A_1P(0x88, 0x1B);
TC358768_DCS_write_1A_1P(0x89, 0x1B);

TC358768_DCS_write_1A_1P(0x8A, 0x00);
TC358768_DCS_write_1A_1P(0x8B, 0x00);
TC358768_DCS_write_1A_1P(0x8C, 0x00);

TC358768_DCS_write_1A_1P(0x90, 0x77);//RTN 0x77=61Hz 0x81=55Hz 0x92
TC358768_DCS_write_1A_1P(0x91, 0x44);
TC358768_DCS_write_1A_1P(0x92, 0x77);//0x79
TC358768_DCS_write_1A_1P(0x93, 0x14);
TC358768_DCS_write_1A_1P(0x94, 0x14);
TC358768_DCS_write_1A_1P(0x95, 0x79);
TC358768_DCS_write_1A_1P(0x96, 0x79);
#if (LCM_RGB565)
TC358768_DCS_write_1A_1P(0x9A, 0x0B); //R G order change
#endif
TC358768_DCS_write_1A_1P(0x9B, 0x0F);

TC358768_DCS_write_1A_1P(0x9D, 0x30);//0x34

TC358768_DCS_write_1A_1P(0xB3, 0x28);
TC358768_DCS_write_1A_1P(0xB4, 0x0A);
TC358768_DCS_write_1A_1P(0xB5, 0x45);

//Don't Reload MTP
TC358768_DCS_write_1A_1P(0xFB, 0x01);


TC358768_DCS_write_1A_1P(0xFF, 0x22);
TC358768_DCS_write_1A_1P(0xFB, 0x01); 
TC358768_DCS_write_1A_1P(0x56, 0x77); //smart contrast
TC358768_DCS_write_1A_1P(0x1A, 0x77); //color enhancement
TC358768_DCS_write_1A_1P(0x68, 0x77); //edge enhancement


//CMD1
TC358768_DCS_write_1A_1P(0xFF, 0x10); 
TC358768_DCS_write_1A_3P(0x3B, 0x03, 0x08, 0x08);
TC358768_DCS_write_1A_1P(0x35, 0x00);
#if (LCM_RGB565)
TC358768_DCS_write_1A_1P(0x3A, 0x55);
#endif
#if (LCM_DSI_CMD_MODE)
TC358768_DCS_write_1A_1P(0xBB, 0x10);
#else
TC358768_DCS_write_1A_1P(0xBB, 0x03);
#endif
//TC358768_DCS_write_1A_1P(0xBB, 0x10);

//TC358768_DCS_write_1A_1P(0xBB, 0x10);
// Video Mode Porch Setting (VBP,VFP,HBP,HFP)
//mipi.write,0x39,0x3B,0x03,0x0a,0x0a,0x0a,0x0a
	data_array[0]= 0x00063902;
	data_array[1]= 0x0808033B;
	data_array[2]= 0x00000A0A;
	dsi_set_cmdq(data_array, 3, 1);

//TC358768_DCS_write_1A_1P(0x21, 0x00); //for test
// Backlight & CABC Setting
if(esd_check) {
	TC358768_DCS_write_1A_1P(0x51,0xff);
	esd_check = false;
	}
else
	TC358768_DCS_write_1A_1P(0x51,0x00);
TC358768_DCS_write_1A_1P(0x53,0x2C); // BCTRL, DD,BL (when CABC On,0x2C)
TC358768_DCS_write_1A_1P(0x55,0x01); // Image_Enhancement, CABC (1-UI,2-Still,3-Moving)
TC358768_DCS_write_1A_1P(0x5E,0x0A); // CABC minimum brightness

TC358768_DCS_write_1A_0P(0x11);
MDELAY(150);
TC358768_DCS_write_1A_0P(0x29);


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
	#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu add
		#ifndef BUILD_LK
		printk("[JX] %s rtc_value=%d\n",__func__,rtc_value);
		#else
		printf("[JX] %s rtc_value=%d\n",__func__,rtc_value);
		#endif
	#endif
		params->type   = LCM_TYPE_DSI;
	#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu add
		if(rtc_value==0){
			params->width  = FRAME_WIDTH;
			params->height = FRAME_HEIGHT;
		}else{
			params->width  = 540;
			params->height = 960;

		}
	#else
			params->width  = FRAME_WIDTH;
			params->height = FRAME_HEIGHT;	
	#endif

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
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		#if (LCM_RGB565)
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB565;
		#else
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
		#endif

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		#if (LCM_RGB565)
		params->dsi.PS=LCM_PACKED_PS_16BIT_RGB565;
		#else
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		#endif
		params->dsi.word_count=720*3;	

		
		params->dsi.vertical_sync_active				= 2;
		params->dsi.vertical_backporch					= 6;
		params->dsi.vertical_frontporch					= 8;
	#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu add
		if(rtc_value==0){
			params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		}else{
			params->dsi.vertical_active_line				= 960;
		}
	#else
			params->dsi.vertical_active_line				= FRAME_HEIGHT; 
	#endif
		params->dsi.horizontal_sync_active				= 4;
		params->dsi.horizontal_backporch				= 118;
		params->dsi.horizontal_frontporch				= 118;
	#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu add
		if(rtc_value==0){
			params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		}else{
			params->dsi.horizontal_active_pixel				= 540;
		}	
	#else
			params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	#endif
//		params->dsi.compatibility_for_nvk	= 1;

		// Bit rate calculation
		//1 Every lane speed
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4	
	#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu add
		if(rtc_value==0){	
		#if (LCM_RGB565)
		params->dsi.fbk_div =7;//0x12;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
		#else
		params->dsi.fbk_div =14;
		#endif
		}else{
			#if (LCM_RGB565)
			params->dsi.fbk_div =7; 
			#else
			params->dsi.fbk_div =1; 
			#endif
		}
	#else
		#if (LCM_RGB565)
		params->dsi.fbk_div =7;//0x12;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
		#else
		params->dsi.fbk_div =14;
		#endif
	#endif
	params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_481;//481; 305_5
//	params->dsi.pll_select = 1;
#ifdef LENOVO_BACKLIGHT_LIMIT
params->bl_app.min =1;
params->bl_app.def =102;
params->bl_app.max =255;

params->bl_bsp.min =5;
params->bl_bsp.def =102;
params->bl_bsp.max =255;
#endif
}

static void lcm_init(void)
{
	lcm_is_init = true;
	// Enable EN_PWR for NT50198 PMIC
	//dsi_lcm_set_gpio_mode(GPIO139, GPIO_MODE_GPIO);
	//dsi_lcm_set_gpio_dir(GPIO139, GPIO_DIR_OUT);
	//dsi_lcm_set_gpio_out(GPIO139, GPIO_OUT_ONE);
	       mt_set_gpio_mode(GPIO73, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO73, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO73, GPIO_OUT_ONE);
        mt_set_gpio_mode(GPIO132, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO132, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO132, GPIO_OUT_ONE);

	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(10);
	
	SET_RESET_PIN(1);
	MDELAY(20);      

	init_lcm_registers();

	TC358768_DCS_write_1A_1P(0x55,0x3);
}

static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
//	TC358768_DCS_write_1A_1P(0x4F,0x01);
lcm_setgammamode_in_suspend(lcm_gammamode_index);

//	MDELAY(10);
//	SET_RESET_PIN(0);
#if 1
        mt_set_gpio_mode(GPIO132, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO132, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO132, GPIO_OUT_ZERO);

        mt_set_gpio_mode(GPIO73, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO73, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO73, GPIO_OUT_ZERO);
#else
	dsi_lcm_set_gpio_out(GPIO132, GPIO_OUT_ZERO);
	dsi_lcm_set_gpio_out(GPIO73, GPIO_OUT_ZERO);
#endif
	lcm_is_init = false;
}


static void lcm_resume(void)
{
	unsigned int data_array[16];
	//unsigned char buffer[2];
	if(!lcm_is_init) {
#if 1
        mt_set_gpio_mode(GPIO73, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO73, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO73, GPIO_OUT_ONE);
#else
		dsi_lcm_set_gpio_out(GPIO73, GPIO_OUT_ONE);
#endif
	//		lcm_init();
	//	TC358768_DCS_write_1A_0P(0x11);
	//	MDELAY(150);
	//	TC358768_DCS_write_1A_0P(0x29);
	//	MDELAY(10);
	//	if(lcm_esd_check())
			lcm_init();
		lcm_set_pq_mode();
		
		lcm_setgammamode_in_suspend(lcm_gammamode_index);
#if 1
        mt_set_gpio_mode(GPIO132, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO132, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO132, GPIO_OUT_ONE);
#else
		dsi_lcm_set_gpio_out(GPIO132, GPIO_OUT_ONE);
#endif
	}
#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu add
	if(rtc_value==0){
		TC358768_DCS_write_1A_1P(0x58,0x00); 
	}else{
		TC358768_DCS_write_1A_1P(0x58,0x02); 
	}
#endif
#if 0//ndef BUILD_LK
	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
	
	read_reg_v2(0xFE, buffer, 1);
	printk("%s, kernel nt35596 horse debug: nt35596 id = 0x%08x\n", __func__, buffer[0]);
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

	//data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	//dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif
#if 1

static int lcm_id = 0;

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
		printf("%s, LK nt35590 debug: nt35590 id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel nt35590 horse debug: nt35590 id = 0x%08x\n", __func__, id);
    #endif

    if(id == LCM_ID_NT35590)
    	return 1;
    else
        return 0;
#else
		unsigned int ret = 0;
		unsigned char buffer[2];
	
		TC358768_DCS_write_1A_1P(0xFF,0x10);// CMD1
		MDELAY(1);
	
		read_reg_v2(0xF4, buffer,2);
#ifdef BUILD_LK
		printf("[JX] %s 0xF4 0=0x%x 1=0x%x \n",__func__,buffer[0],buffer[1]);
		
#endif
		lcm_id = buffer[0];

	ret = mt_get_gpio_in(GPIO154);
#if defined(BUILD_LK)
	printf("%s, [jx]nt35595 GPIO154 = %d \n", __func__, ret);
#endif	

	return (ret == 0)?1:0;

#endif


}
#endif
static void lcm_set_pq_mode(void)
{
	unsigned int value=0x00;
	value = lcm_iemode_index<<4 | lcm_cabcmode_index;
/*	#if BUILD_LK
	printf("%s value=0x%x\n",__func__,value);
	#else
	printk("%s value=0x%x\n",__func__,value);
	#endif	
	*/
	TC358768_DCS_write_1A_1P(0x55,value);
}

static void lcm_setbacklight(unsigned int level)
{
	unsigned int data_array[16];
#if defined(BUILD_LK)
	printf("%s, %d\n", __func__, level);
#elif defined(BUILD_UBOOT)
    printf("%s, %d\n", __func__, level);
#else
    printk("lcm_setbacklight = %d\n", level);
#endif

	if(level > 255) 
	    level = 255;

	data_array[0]= 0x00023902;
	data_array[1] =(0x51|(level<<8));
	dsi_set_cmdq(data_array, 2, 1);
/*
	if(level < 50){
		TC358768_DCS_write_1A_1P(0x55,0x01);
		}
	else{
		lcm_set_pq_mode();
		}

*/		
}
extern BOOL is_early_suspended;

static unsigned int lcm_esd_check(void)
{
unsigned char buffer[2],ret;
unsigned int data_array[16];

#ifndef BUILD_LK
	if(lcm_esd_test)
	{
	lcm_esd_test = FALSE;
	esd_check = true;
	return TRUE;
	}
if(is_early_suspended)
	return FALSE;

    //data_array[0]= 0x00003700 | (1 << 16);    
    //dsi_set_cmdq(&data_array, 1, 1);
				
	TC358768_DCS_write_1A_1P(0xFF,0x10);// CMD1
	//MDELAY(1);

	read_reg_v2(0x0A, buffer,2);
	//printk("[JX] %s 0x0A 0=0x%x 1=0x%x \n",__func__,buffer[0],buffer[1]);
	ret = buffer[0]==0x9C?0:1;
	//printk("[JX] %s ret=%d \n",__func__,ret);
	if(ret){
		esd_check = true;
		return TRUE;
	}

	read_reg_v2(0x0D, buffer,2);
	//printk("[JX] %s 0x0D 0=0x%x 1=0x%x \n",__func__,buffer[0],buffer[1]);
	ret = (buffer[0]&0xdf)==0x00?0:1;
	//printk("[JX] %s ret=%d \n",__func__,ret);
	if(ret){
		esd_check = true;
		return TRUE;
	}

	read_reg_v2(0x0E, buffer,2);
	//printk("[JX] %s 0x0E 0=0x%x 1=0x%x \n",__func__,buffer[0],buffer[1]);
	ret = (buffer[0]&0xFE)==0x80?0:1;
	//printk("[JX] %s ret=%d \n",__func__,ret);
	if(ret){
		esd_check = true;
		return TRUE;
	}
	else return FALSE;
#endif
}

static unsigned int lcm_esd_recover(void)
{
	#if BUILD_LK
	printf("+ %s %d\n",__func__,esd_check);
	#else
	printk("+ %s %d\n",__func__,esd_check);
	#endif

	lcm_init();

	#if BUILD_LK
	printf("- %s %d\n",__func__,esd_check);
	#else
	printk("- %s %d\n",__func__,esd_check);
	#endif 
}
static void lcm_setgamma_normal(void)
{
 	TC358768_DCS_write_1A_1P(0xFF,0x20); // Page 0, power-related setting
	MDELAY(1);	  

	//7300
	//R+
	TC358768_DCS_write_1A_1P(0x75, 0x01);	
	TC358768_DCS_write_1A_1P(0x76, 0x02);	
	TC358768_DCS_write_1A_1P(0x77, 0x01);
	TC358768_DCS_write_1A_1P(0x78, 0x09);	
	TC358768_DCS_write_1A_1P(0x79, 0x01);
	TC358768_DCS_write_1A_1P(0x7A, 0x17);	
	TC358768_DCS_write_1A_1P(0x7B, 0x01);
	TC358768_DCS_write_1A_1P(0x7C, 0x22);	
	TC358768_DCS_write_1A_1P(0x7D, 0x01);
	TC358768_DCS_write_1A_1P(0x7E, 0x2D);	
	TC358768_DCS_write_1A_1P(0x7F, 0x01);
	TC358768_DCS_write_1A_1P(0x80, 0x38);  
	TC358768_DCS_write_1A_1P(0x81, 0x01);
	TC358768_DCS_write_1A_1P(0x82, 0x43);  
	TC358768_DCS_write_1A_1P(0x83, 0x01);
	TC358768_DCS_write_1A_1P(0x84, 0x4D);  
	TC358768_DCS_write_1A_1P(0x85, 0x01);
	TC358768_DCS_write_1A_1P(0x86, 0x57);  
	TC358768_DCS_write_1A_1P(0x87, 0x01);
	TC358768_DCS_write_1A_1P(0x88, 0x79);  
	TC358768_DCS_write_1A_1P(0x89, 0x01);
	TC358768_DCS_write_1A_1P(0x8A, 0x97);  
	TC358768_DCS_write_1A_1P(0x8B, 0x01);
	TC358768_DCS_write_1A_1P(0x8C, 0xC7);  
	TC358768_DCS_write_1A_1P(0x8D, 0x01);
	TC358768_DCS_write_1A_1P(0x8E, 0xEF);  
	TC358768_DCS_write_1A_1P(0x8F, 0x02);
	TC358768_DCS_write_1A_1P(0x90, 0x2F);  
	TC358768_DCS_write_1A_1P(0x91, 0x02);
	TC358768_DCS_write_1A_1P(0x92, 0x63);  
	TC358768_DCS_write_1A_1P(0x93, 0x02);
	TC358768_DCS_write_1A_1P(0x94, 0x65);  
	TC358768_DCS_write_1A_1P(0x95, 0x02);
	TC358768_DCS_write_1A_1P(0x96, 0x94);  
	TC358768_DCS_write_1A_1P(0x97, 0x02);
	TC358768_DCS_write_1A_1P(0x98, 0xCA);  
	TC358768_DCS_write_1A_1P(0x99, 0x02);
	TC358768_DCS_write_1A_1P(0x9A, 0xE9);  
	TC358768_DCS_write_1A_1P(0x9B, 0x03);
	TC358768_DCS_write_1A_1P(0x9C, 0x12);  
	TC358768_DCS_write_1A_1P(0x9D, 0x03);
	TC358768_DCS_write_1A_1P(0x9E, 0x2B);  
	TC358768_DCS_write_1A_1P(0x9F, 0x03);
	TC358768_DCS_write_1A_1P(0xA0, 0x50);	
	TC358768_DCS_write_1A_1P(0xA2, 0x03);
	TC358768_DCS_write_1A_1P(0xA3, 0x56);  
	TC358768_DCS_write_1A_1P(0xA4, 0x03);
	TC358768_DCS_write_1A_1P(0xA5, 0x64);	
	TC358768_DCS_write_1A_1P(0xA6, 0x03);
	TC358768_DCS_write_1A_1P(0xA7, 0x6B);	
	TC358768_DCS_write_1A_1P(0xA9, 0x03);
	TC358768_DCS_write_1A_1P(0xAA, 0x7A);	
	TC358768_DCS_write_1A_1P(0xAB, 0x03);
	TC358768_DCS_write_1A_1P(0xAC, 0x84);
	TC358768_DCS_write_1A_1P(0xAD, 0x03);
	TC358768_DCS_write_1A_1P(0xAE, 0x8B);	
	TC358768_DCS_write_1A_1P(0xAF, 0x03);
	TC358768_DCS_write_1A_1P(0xB0, 0x96);	
	TC358768_DCS_write_1A_1P(0xB1, 0x03);
	TC358768_DCS_write_1A_1P(0xB2, 0xB9);	
	// R-																				
	TC358768_DCS_write_1A_1P(0xB3, 0x01); 
	TC358768_DCS_write_1A_1P(0xB4, 0x02); 
	TC358768_DCS_write_1A_1P(0xB5, 0x01);
	TC358768_DCS_write_1A_1P(0xB6, 0x09); 
	TC358768_DCS_write_1A_1P(0xB7, 0x01);
	TC358768_DCS_write_1A_1P(0xB8, 0x17); 
	TC358768_DCS_write_1A_1P(0xB9, 0x01);
	TC358768_DCS_write_1A_1P(0xBA, 0x22); 
	TC358768_DCS_write_1A_1P(0xBB, 0x01);
	TC358768_DCS_write_1A_1P(0xBC, 0x2D);
	TC358768_DCS_write_1A_1P(0xBD, 0x01);
	TC358768_DCS_write_1A_1P(0xBE, 0x38);
	TC358768_DCS_write_1A_1P(0xBF, 0x01);
	TC358768_DCS_write_1A_1P(0xC0, 0x43);
	TC358768_DCS_write_1A_1P(0xC1, 0x01);
	TC358768_DCS_write_1A_1P(0xC2, 0x4D);
	TC358768_DCS_write_1A_1P(0xC3, 0x01);
	TC358768_DCS_write_1A_1P(0xC4, 0x57);
	TC358768_DCS_write_1A_1P(0xC5, 0x01);
	TC358768_DCS_write_1A_1P(0xC6, 0x79);
	TC358768_DCS_write_1A_1P(0xC7, 0x01);
	TC358768_DCS_write_1A_1P(0xC8, 0x97);
	TC358768_DCS_write_1A_1P(0xC9, 0x01);
	TC358768_DCS_write_1A_1P(0xCA, 0xC7);
	TC358768_DCS_write_1A_1P(0xCB, 0x01);
	TC358768_DCS_write_1A_1P(0xCC, 0xEF);
	TC358768_DCS_write_1A_1P(0xCD, 0x02);
	TC358768_DCS_write_1A_1P(0xCE, 0x2F);
	TC358768_DCS_write_1A_1P(0xCF, 0x02);
	TC358768_DCS_write_1A_1P(0xD0, 0x63);
	TC358768_DCS_write_1A_1P(0xD1, 0x02);
	TC358768_DCS_write_1A_1P(0xD2, 0x65);
	TC358768_DCS_write_1A_1P(0xD3, 0x02);
	TC358768_DCS_write_1A_1P(0xD4, 0x94);
	TC358768_DCS_write_1A_1P(0xD5, 0x02);
	TC358768_DCS_write_1A_1P(0xD6, 0xCA);
	TC358768_DCS_write_1A_1P(0xD7, 0x02);
	TC358768_DCS_write_1A_1P(0xD8, 0xE9);
	TC358768_DCS_write_1A_1P(0xD9, 0x03);
	TC358768_DCS_write_1A_1P(0xDA, 0x12);
	TC358768_DCS_write_1A_1P(0xDB, 0x03);
	TC358768_DCS_write_1A_1P(0xDC, 0x2B);
	TC358768_DCS_write_1A_1P(0xDD, 0x03);
	TC358768_DCS_write_1A_1P(0xDE, 0x50);
	TC358768_DCS_write_1A_1P(0xDF, 0x03);
	TC358768_DCS_write_1A_1P(0xE0, 0x56);
	TC358768_DCS_write_1A_1P(0xE1, 0x03);
	TC358768_DCS_write_1A_1P(0xE2, 0x64);
	TC358768_DCS_write_1A_1P(0xE3, 0x03);
	TC358768_DCS_write_1A_1P(0xE4, 0x6B);
	TC358768_DCS_write_1A_1P(0xE5, 0x03);
	TC358768_DCS_write_1A_1P(0xE6, 0x7A);
	TC358768_DCS_write_1A_1P(0xE7, 0x03);
	TC358768_DCS_write_1A_1P(0xE8, 0x84);
	TC358768_DCS_write_1A_1P(0xE9, 0x03);
	TC358768_DCS_write_1A_1P(0xEA, 0x8B);
	TC358768_DCS_write_1A_1P(0xEB, 0x03);
	TC358768_DCS_write_1A_1P(0xEC, 0x96);
	TC358768_DCS_write_1A_1P(0xED, 0x03);
	TC358768_DCS_write_1A_1P(0xEE, 0xB9);
	//G+
	TC358768_DCS_write_1A_1P(0xEF, 0x00);  
	TC358768_DCS_write_1A_1P(0xF0, 0x80); 
	TC358768_DCS_write_1A_1P(0xF1, 0x00);
	TC358768_DCS_write_1A_1P(0xF2, 0x92);
	TC358768_DCS_write_1A_1P(0xF3, 0x00);
	TC358768_DCS_write_1A_1P(0xF4, 0xAD);
	TC358768_DCS_write_1A_1P(0xF5, 0x00);
	TC358768_DCS_write_1A_1P(0xF6, 0xC6);
	TC358768_DCS_write_1A_1P(0xF7, 0x00);
	TC358768_DCS_write_1A_1P(0xF8, 0xDA);
	TC358768_DCS_write_1A_1P(0xF9, 0x00);
	TC358768_DCS_write_1A_1P(0xFA, 0xEC);
	
		//Don't Reload MTP
	TC358768_DCS_write_1A_1P(0xFB, 0x01);
	
		//CMD2_P1
	TC358768_DCS_write_1A_1P(0xFF, 0x21);
	
		//Green Gamma Code_Continuous 
	TC358768_DCS_write_1A_1P(0x00, 0x00);
	TC358768_DCS_write_1A_1P(0x01, 0xFC);
	TC358768_DCS_write_1A_1P(0x02, 0x01);
	TC358768_DCS_write_1A_1P(0x03, 0x0B);
	TC358768_DCS_write_1A_1P(0x04, 0x01);
	TC358768_DCS_write_1A_1P(0x05, 0x18);
	TC358768_DCS_write_1A_1P(0x06, 0x01);
	TC358768_DCS_write_1A_1P(0x07, 0x46);
	TC358768_DCS_write_1A_1P(0x08, 0x01);
	TC358768_DCS_write_1A_1P(0x09, 0x6C);
	TC358768_DCS_write_1A_1P(0x0A, 0x01);
	TC358768_DCS_write_1A_1P(0x0B, 0xA7);
	TC358768_DCS_write_1A_1P(0x0C, 0x01);
	TC358768_DCS_write_1A_1P(0x0D, 0xD6);
	TC358768_DCS_write_1A_1P(0x0E, 0x02);
	TC358768_DCS_write_1A_1P(0x0F, 0x1F);
	TC358768_DCS_write_1A_1P(0x10, 0x02);
	TC358768_DCS_write_1A_1P(0x11, 0x57);
	TC358768_DCS_write_1A_1P(0x12, 0x02);
	TC358768_DCS_write_1A_1P(0x13, 0x59);
	TC358768_DCS_write_1A_1P(0x14, 0x02);
	TC358768_DCS_write_1A_1P(0x15, 0x8B);
	TC358768_DCS_write_1A_1P(0x16, 0x02);
	TC358768_DCS_write_1A_1P(0x17, 0xBF);
	TC358768_DCS_write_1A_1P(0x18, 0x02);
	TC358768_DCS_write_1A_1P(0x19, 0xE1);
	TC358768_DCS_write_1A_1P(0x1A, 0x03);
	TC358768_DCS_write_1A_1P(0x1B, 0x0A);
	TC358768_DCS_write_1A_1P(0x1C, 0x03);
	TC358768_DCS_write_1A_1P(0x1D, 0x24);
	TC358768_DCS_write_1A_1P(0x1E, 0x03);
	TC358768_DCS_write_1A_1P(0x1F, 0x48);
	TC358768_DCS_write_1A_1P(0x20, 0x03);
	TC358768_DCS_write_1A_1P(0x21, 0x52);
	TC358768_DCS_write_1A_1P(0x22, 0x03);
	TC358768_DCS_write_1A_1P(0x23, 0x5F);
	TC358768_DCS_write_1A_1P(0x24, 0x03);
	TC358768_DCS_write_1A_1P(0x25, 0x6B);
	TC358768_DCS_write_1A_1P(0x26, 0x03);
	TC358768_DCS_write_1A_1P(0x27, 0x74);
	TC358768_DCS_write_1A_1P(0x28, 0x03);
	TC358768_DCS_write_1A_1P(0x29, 0x7A);
	TC358768_DCS_write_1A_1P(0x2A, 0x03);
	TC358768_DCS_write_1A_1P(0x2B, 0x88);
	TC358768_DCS_write_1A_1P(0x2D, 0x03);
	TC358768_DCS_write_1A_1P(0x2F, 0x96);
	TC358768_DCS_write_1A_1P(0x30, 0x03);
	TC358768_DCS_write_1A_1P(0x31, 0xB9);
	//G-
	TC358768_DCS_write_1A_1P(0x32, 0x00);  
	TC358768_DCS_write_1A_1P(0x33, 0x80);
	TC358768_DCS_write_1A_1P(0x34, 0x00);
	TC358768_DCS_write_1A_1P(0x35, 0x92);
	TC358768_DCS_write_1A_1P(0x36, 0x00);
	TC358768_DCS_write_1A_1P(0x37, 0xAD);
	TC358768_DCS_write_1A_1P(0x38, 0x00);
	TC358768_DCS_write_1A_1P(0x39, 0xC6);
	TC358768_DCS_write_1A_1P(0x3A, 0x00);
	TC358768_DCS_write_1A_1P(0x3B, 0xDA);
	TC358768_DCS_write_1A_1P(0x3D, 0x00);
	TC358768_DCS_write_1A_1P(0x3F, 0xEC);
	TC358768_DCS_write_1A_1P(0x40, 0x00);
	TC358768_DCS_write_1A_1P(0x41, 0xFC);
	TC358768_DCS_write_1A_1P(0x42, 0x01);
	TC358768_DCS_write_1A_1P(0x43, 0x0B);
	TC358768_DCS_write_1A_1P(0x44, 0x01);
	TC358768_DCS_write_1A_1P(0x45, 0x18);
	TC358768_DCS_write_1A_1P(0x46, 0x01);
	TC358768_DCS_write_1A_1P(0x47, 0x46);
	TC358768_DCS_write_1A_1P(0x48, 0x01);
	TC358768_DCS_write_1A_1P(0x49, 0x6C);
	TC358768_DCS_write_1A_1P(0x4A, 0x01);
	TC358768_DCS_write_1A_1P(0x4B, 0xA7);
	TC358768_DCS_write_1A_1P(0x4C, 0x01);
	TC358768_DCS_write_1A_1P(0x4D, 0xD6);
	TC358768_DCS_write_1A_1P(0x4E, 0x02);
	TC358768_DCS_write_1A_1P(0x4F, 0x1F);
	TC358768_DCS_write_1A_1P(0x50, 0x02);
	TC358768_DCS_write_1A_1P(0x51, 0x57);
	TC358768_DCS_write_1A_1P(0x52, 0x02);
	TC358768_DCS_write_1A_1P(0x53, 0x59);
	TC358768_DCS_write_1A_1P(0x54, 0x02);
	TC358768_DCS_write_1A_1P(0x55, 0x8B);
	TC358768_DCS_write_1A_1P(0x56, 0x02);
	TC358768_DCS_write_1A_1P(0x58, 0xBF);
	TC358768_DCS_write_1A_1P(0x59, 0x02);
	TC358768_DCS_write_1A_1P(0x5A, 0xE1);
	TC358768_DCS_write_1A_1P(0x5B, 0x03);
	TC358768_DCS_write_1A_1P(0x5C, 0x0A);
	TC358768_DCS_write_1A_1P(0x5D, 0x03);
	TC358768_DCS_write_1A_1P(0x5E, 0x24);
	TC358768_DCS_write_1A_1P(0x5F, 0x03);
	TC358768_DCS_write_1A_1P(0x60, 0x48);
	TC358768_DCS_write_1A_1P(0x61, 0x03);
	TC358768_DCS_write_1A_1P(0x62, 0x52);
	TC358768_DCS_write_1A_1P(0x63, 0x03);
	TC358768_DCS_write_1A_1P(0x64, 0x5F);
	TC358768_DCS_write_1A_1P(0x65, 0x03);
	TC358768_DCS_write_1A_1P(0x66, 0x6B);
	TC358768_DCS_write_1A_1P(0x67, 0x03);
	TC358768_DCS_write_1A_1P(0x68, 0x74);
	TC358768_DCS_write_1A_1P(0x69, 0x03);
	TC358768_DCS_write_1A_1P(0x6A, 0x7A);
	TC358768_DCS_write_1A_1P(0x6B, 0x03);
	TC358768_DCS_write_1A_1P(0x6C, 0x88);
	TC358768_DCS_write_1A_1P(0x6D, 0x03);
	TC358768_DCS_write_1A_1P(0x6E, 0x96);
	TC358768_DCS_write_1A_1P(0x6F, 0x03);
	TC358768_DCS_write_1A_1P(0x70, 0xB9);
	
	//B+	
	TC358768_DCS_write_1A_1P(0x71, 0x00);  
	TC358768_DCS_write_1A_1P(0x72, 0x0A);
	TC358768_DCS_write_1A_1P(0x73, 0x00);
	TC358768_DCS_write_1A_1P(0x74, 0x41);
	TC358768_DCS_write_1A_1P(0x75, 0x00);
	TC358768_DCS_write_1A_1P(0x76, 0x77);
	TC358768_DCS_write_1A_1P(0x77, 0x00);
	TC358768_DCS_write_1A_1P(0x78, 0x9B);
	TC358768_DCS_write_1A_1P(0x79, 0x00);
	TC358768_DCS_write_1A_1P(0x7A, 0xB6);
	TC358768_DCS_write_1A_1P(0x7B, 0x00);
	TC358768_DCS_write_1A_1P(0x7C, 0xCD);
	TC358768_DCS_write_1A_1P(0x7D, 0x00);
	TC358768_DCS_write_1A_1P(0x7E, 0xE2);
	TC358768_DCS_write_1A_1P(0x7F, 0x00);
	TC358768_DCS_write_1A_1P(0x80, 0xF4);
	TC358768_DCS_write_1A_1P(0x81, 0x01);
	TC358768_DCS_write_1A_1P(0x82, 0x03);
	TC358768_DCS_write_1A_1P(0x83, 0x01);
	TC358768_DCS_write_1A_1P(0x84, 0x38);
	TC358768_DCS_write_1A_1P(0x85, 0x01);
	TC358768_DCS_write_1A_1P(0x86, 0x61);
	TC358768_DCS_write_1A_1P(0x87, 0x01);
	TC358768_DCS_write_1A_1P(0x88, 0xA1);
	TC358768_DCS_write_1A_1P(0x89, 0x01);
	TC358768_DCS_write_1A_1P(0x8A, 0xD2);
	TC358768_DCS_write_1A_1P(0x8B, 0x02);
	TC358768_DCS_write_1A_1P(0x8C, 0x16);
	TC358768_DCS_write_1A_1P(0x8D, 0x02);
	TC358768_DCS_write_1A_1P(0x8E, 0x53);
	TC358768_DCS_write_1A_1P(0x8F, 0x02);
	TC358768_DCS_write_1A_1P(0x90, 0x58);
	TC358768_DCS_write_1A_1P(0x91, 0x02);
	TC358768_DCS_write_1A_1P(0x92, 0x8A);
	TC358768_DCS_write_1A_1P(0x93, 0x02);
	TC358768_DCS_write_1A_1P(0x94, 0xC0);
	TC358768_DCS_write_1A_1P(0x95, 0x02);
	TC358768_DCS_write_1A_1P(0x96, 0xDE);
	TC358768_DCS_write_1A_1P(0x97, 0x03);
	TC358768_DCS_write_1A_1P(0x98, 0x0A);
	TC358768_DCS_write_1A_1P(0x99, 0x03);
	TC358768_DCS_write_1A_1P(0x9A, 0x22);
	TC358768_DCS_write_1A_1P(0x9B, 0x03);
	TC358768_DCS_write_1A_1P(0x9C, 0x48);
	TC358768_DCS_write_1A_1P(0x9D, 0x03);
	TC358768_DCS_write_1A_1P(0x9E, 0x52);
	TC358768_DCS_write_1A_1P(0x9F, 0x03);
	TC358768_DCS_write_1A_1P(0xA0, 0x5D);
	TC358768_DCS_write_1A_1P(0xA2, 0x03);
	TC358768_DCS_write_1A_1P(0xA3, 0x6A);
	TC358768_DCS_write_1A_1P(0xA4, 0x03);
	TC358768_DCS_write_1A_1P(0xA5, 0x76);
	TC358768_DCS_write_1A_1P(0xA6, 0x03);
	TC358768_DCS_write_1A_1P(0xA7, 0x80);
	TC358768_DCS_write_1A_1P(0xA9, 0x03);
	TC358768_DCS_write_1A_1P(0xAA, 0x87);
	TC358768_DCS_write_1A_1P(0xAB, 0x03);
	TC358768_DCS_write_1A_1P(0xAC, 0x96);
	TC358768_DCS_write_1A_1P(0xAD, 0x03);
	TC358768_DCS_write_1A_1P(0xAE, 0xB9);
	//B-
	TC358768_DCS_write_1A_1P(0xAF, 0x00); 
	TC358768_DCS_write_1A_1P(0xB0, 0x0A);
	TC358768_DCS_write_1A_1P(0xB1, 0x00);
	TC358768_DCS_write_1A_1P(0xB2, 0x41);
	TC358768_DCS_write_1A_1P(0xB3, 0x00);
	TC358768_DCS_write_1A_1P(0xB4, 0x77);
	TC358768_DCS_write_1A_1P(0xB5, 0x00);
	TC358768_DCS_write_1A_1P(0xB6, 0x9B);
	TC358768_DCS_write_1A_1P(0xB7, 0x00);
	TC358768_DCS_write_1A_1P(0xB8, 0xB6);
	TC358768_DCS_write_1A_1P(0xB9, 0x00);
	TC358768_DCS_write_1A_1P(0xBA, 0xCD);
	TC358768_DCS_write_1A_1P(0xBB, 0x00);
	TC358768_DCS_write_1A_1P(0xBC, 0xE2);
	TC358768_DCS_write_1A_1P(0xBD, 0x00);
	TC358768_DCS_write_1A_1P(0xBE, 0xF4);
	TC358768_DCS_write_1A_1P(0xBF, 0x01);
	TC358768_DCS_write_1A_1P(0xC0, 0x03);
	TC358768_DCS_write_1A_1P(0xC1, 0x01);
	TC358768_DCS_write_1A_1P(0xC2, 0x38);
	TC358768_DCS_write_1A_1P(0xC3, 0x01);
	TC358768_DCS_write_1A_1P(0xC4, 0x61);
	TC358768_DCS_write_1A_1P(0xC5, 0x01);
	TC358768_DCS_write_1A_1P(0xC6, 0xA1);
	TC358768_DCS_write_1A_1P(0xC7, 0x01);
	TC358768_DCS_write_1A_1P(0xC8, 0xD2);
	TC358768_DCS_write_1A_1P(0xC9, 0x02);
	TC358768_DCS_write_1A_1P(0xCA, 0x16);
	TC358768_DCS_write_1A_1P(0xCB, 0x02);
	TC358768_DCS_write_1A_1P(0xCC, 0x53);
	TC358768_DCS_write_1A_1P(0xCD, 0x02);
	TC358768_DCS_write_1A_1P(0xCE, 0x58);
	TC358768_DCS_write_1A_1P(0xCF, 0x02);
	TC358768_DCS_write_1A_1P(0xD0, 0x8A);
	TC358768_DCS_write_1A_1P(0xD1, 0x02);
	TC358768_DCS_write_1A_1P(0xD2, 0xC0);
	TC358768_DCS_write_1A_1P(0xD3, 0x02);
	TC358768_DCS_write_1A_1P(0xD4, 0xDE);
	TC358768_DCS_write_1A_1P(0xD5, 0x03);
	TC358768_DCS_write_1A_1P(0xD6, 0x0A);
	TC358768_DCS_write_1A_1P(0xD7, 0x03);
	TC358768_DCS_write_1A_1P(0xD8, 0x22);
	TC358768_DCS_write_1A_1P(0xD9, 0x03);
	TC358768_DCS_write_1A_1P(0xDA, 0x48);
	TC358768_DCS_write_1A_1P(0xDB, 0x03);
	TC358768_DCS_write_1A_1P(0xDC, 0x52);
	TC358768_DCS_write_1A_1P(0xDD, 0x03);
	TC358768_DCS_write_1A_1P(0xDE, 0x5D);
	TC358768_DCS_write_1A_1P(0xDF, 0x03);
	TC358768_DCS_write_1A_1P(0xE0, 0x6A);
	TC358768_DCS_write_1A_1P(0xE1, 0x03);
	TC358768_DCS_write_1A_1P(0xE2, 0x76);
	TC358768_DCS_write_1A_1P(0xE3, 0x03);
	TC358768_DCS_write_1A_1P(0xE4, 0x80);
	TC358768_DCS_write_1A_1P(0xE5, 0x03);
	TC358768_DCS_write_1A_1P(0xE6, 0x87);
	TC358768_DCS_write_1A_1P(0xE7, 0x03);
	TC358768_DCS_write_1A_1P(0xE8, 0x96);
	TC358768_DCS_write_1A_1P(0xE9, 0x03);
	TC358768_DCS_write_1A_1P(0xEA, 0xB9);
	TC358768_DCS_write_1A_1P(0xFF, 0x10);	

}
static void lcm_setgamma_cold(void)
{
	TC358768_DCS_write_1A_1P(0xFF,0x20); // Page 0, power-related setting
	MDELAY(1);	  

	//8300
	//R+
	TC358768_DCS_write_1A_1P(0x75, 0x01);	
	TC358768_DCS_write_1A_1P(0x76, 0x48);	
	TC358768_DCS_write_1A_1P(0x77, 0x01);
	TC358768_DCS_write_1A_1P(0x78, 0x4D);	
	TC358768_DCS_write_1A_1P(0x79, 0x01);
	TC358768_DCS_write_1A_1P(0x7A, 0x56);	
	TC358768_DCS_write_1A_1P(0x7B, 0x01);
	TC358768_DCS_write_1A_1P(0x7C, 0x5F);	
	TC358768_DCS_write_1A_1P(0x7D, 0x01);
	TC358768_DCS_write_1A_1P(0x7E, 0x67);	
	TC358768_DCS_write_1A_1P(0x7F, 0x01);
	TC358768_DCS_write_1A_1P(0x80, 0x6F);  
	TC358768_DCS_write_1A_1P(0x81, 0x01);
	TC358768_DCS_write_1A_1P(0x82, 0x77);  
	TC358768_DCS_write_1A_1P(0x83, 0x01);
	TC358768_DCS_write_1A_1P(0x84, 0x7E);  
	TC358768_DCS_write_1A_1P(0x85, 0x01);
	TC358768_DCS_write_1A_1P(0x86, 0x85);  
	TC358768_DCS_write_1A_1P(0x87, 0x01);
	TC358768_DCS_write_1A_1P(0x88, 0xA0);  
	TC358768_DCS_write_1A_1P(0x89, 0x01);
	TC358768_DCS_write_1A_1P(0x8A, 0xB7);  
	TC358768_DCS_write_1A_1P(0x8B, 0x01);
	TC358768_DCS_write_1A_1P(0x8C, 0xDF);  
	TC358768_DCS_write_1A_1P(0x8D, 0x02);
	TC358768_DCS_write_1A_1P(0x8E, 0x03);  
	TC358768_DCS_write_1A_1P(0x8F, 0x02);
	TC358768_DCS_write_1A_1P(0x90, 0x3D);  
	TC358768_DCS_write_1A_1P(0x91, 0x02);
	TC358768_DCS_write_1A_1P(0x92, 0x6D);  
	TC358768_DCS_write_1A_1P(0x93, 0x02);
	TC358768_DCS_write_1A_1P(0x94, 0x6F);  
	TC358768_DCS_write_1A_1P(0x95, 0x02);
	TC358768_DCS_write_1A_1P(0x96, 0x9B);  
	TC358768_DCS_write_1A_1P(0x97, 0x02);
	TC358768_DCS_write_1A_1P(0x98, 0xCD);  
	TC358768_DCS_write_1A_1P(0x99, 0x02);
	TC358768_DCS_write_1A_1P(0x9A, 0xEC);  
	TC358768_DCS_write_1A_1P(0x9B, 0x03);
	TC358768_DCS_write_1A_1P(0x9C, 0x14);  
	TC358768_DCS_write_1A_1P(0x9D, 0x03);
	TC358768_DCS_write_1A_1P(0x9E, 0x2D);  
	TC358768_DCS_write_1A_1P(0x9F, 0x03);
	TC358768_DCS_write_1A_1P(0xA0, 0x4F);	
	TC358768_DCS_write_1A_1P(0xA2, 0x03);
	TC358768_DCS_write_1A_1P(0xA3, 0x58);  
	TC358768_DCS_write_1A_1P(0xA4, 0x03);
	TC358768_DCS_write_1A_1P(0xA5, 0x62);	
	TC358768_DCS_write_1A_1P(0xA6, 0x03);
	TC358768_DCS_write_1A_1P(0xA7, 0x6C);	
	TC358768_DCS_write_1A_1P(0xA9, 0x03);
	TC358768_DCS_write_1A_1P(0xAA, 0x77);	
	TC358768_DCS_write_1A_1P(0xAB, 0x03);
	TC358768_DCS_write_1A_1P(0xAC, 0x80);  
	TC358768_DCS_write_1A_1P(0xAD, 0x03);
	TC358768_DCS_write_1A_1P(0xAE, 0x88);	
	TC358768_DCS_write_1A_1P(0xAF, 0x03);
	TC358768_DCS_write_1A_1P(0xB0, 0x96);	
	TC358768_DCS_write_1A_1P(0xB1, 0x03);
	TC358768_DCS_write_1A_1P(0xB2, 0xB9);	
	
	// R-																						
	TC358768_DCS_write_1A_1P(0xB3, 0x01); 
	TC358768_DCS_write_1A_1P(0xB4, 0x48); 
	TC358768_DCS_write_1A_1P(0xB5, 0x01);
	TC358768_DCS_write_1A_1P(0xB6, 0x4D); 
	TC358768_DCS_write_1A_1P(0xB7, 0x01);
	TC358768_DCS_write_1A_1P(0xB8, 0x56); 
	TC358768_DCS_write_1A_1P(0xB9, 0x01);
	TC358768_DCS_write_1A_1P(0xBA, 0x5F); 
	TC358768_DCS_write_1A_1P(0xBB, 0x01);
	TC358768_DCS_write_1A_1P(0xBC, 0x67);
	TC358768_DCS_write_1A_1P(0xBD, 0x01);
	TC358768_DCS_write_1A_1P(0xBE, 0x6F);
	TC358768_DCS_write_1A_1P(0xBF, 0x01);
	TC358768_DCS_write_1A_1P(0xC0, 0x77);
	TC358768_DCS_write_1A_1P(0xC1, 0x01);
	TC358768_DCS_write_1A_1P(0xC2, 0x7E);
	TC358768_DCS_write_1A_1P(0xC3, 0x01);
	TC358768_DCS_write_1A_1P(0xC4, 0x85);
	TC358768_DCS_write_1A_1P(0xC5, 0x01);
	TC358768_DCS_write_1A_1P(0xC6, 0xA0);
	TC358768_DCS_write_1A_1P(0xC7, 0x01);
	TC358768_DCS_write_1A_1P(0xC8, 0xB7);
	TC358768_DCS_write_1A_1P(0xC9, 0x01);
	TC358768_DCS_write_1A_1P(0xCA, 0xDF);
	TC358768_DCS_write_1A_1P(0xCB, 0x02);
	TC358768_DCS_write_1A_1P(0xCC, 0x03);
	TC358768_DCS_write_1A_1P(0xCD, 0x02);
	TC358768_DCS_write_1A_1P(0xCE, 0x3D);
	TC358768_DCS_write_1A_1P(0xCF, 0x02);
	TC358768_DCS_write_1A_1P(0xD0, 0x6D);
	TC358768_DCS_write_1A_1P(0xD1, 0x02);
	TC358768_DCS_write_1A_1P(0xD2, 0x6F);
	TC358768_DCS_write_1A_1P(0xD3, 0x02);
	TC358768_DCS_write_1A_1P(0xD4, 0x9B);
	TC358768_DCS_write_1A_1P(0xD5, 0x02);
	TC358768_DCS_write_1A_1P(0xD6, 0xCD);
	TC358768_DCS_write_1A_1P(0xD7, 0x02);
	TC358768_DCS_write_1A_1P(0xD8, 0xEC);
	TC358768_DCS_write_1A_1P(0xD9, 0x03);
	TC358768_DCS_write_1A_1P(0xDA, 0x14);
	TC358768_DCS_write_1A_1P(0xDB, 0x03);
	TC358768_DCS_write_1A_1P(0xDC, 0x2D);
	TC358768_DCS_write_1A_1P(0xDD, 0x03);
	TC358768_DCS_write_1A_1P(0xDE, 0x4F);
	TC358768_DCS_write_1A_1P(0xDF, 0x03);
	TC358768_DCS_write_1A_1P(0xE0, 0x58);
	TC358768_DCS_write_1A_1P(0xE1, 0x03);
	TC358768_DCS_write_1A_1P(0xE2, 0x62);
	TC358768_DCS_write_1A_1P(0xE3, 0x03);
	TC358768_DCS_write_1A_1P(0xE4, 0x6C);
	TC358768_DCS_write_1A_1P(0xE5, 0x03);
	TC358768_DCS_write_1A_1P(0xE6, 0x77);
	TC358768_DCS_write_1A_1P(0xE7, 0x03);
	TC358768_DCS_write_1A_1P(0xE8, 0x80);
	TC358768_DCS_write_1A_1P(0xE9, 0x03);
	TC358768_DCS_write_1A_1P(0xEA, 0x88);
	TC358768_DCS_write_1A_1P(0xEB, 0x03);
	TC358768_DCS_write_1A_1P(0xEC, 0x96);
	TC358768_DCS_write_1A_1P(0xED, 0x03);
	TC358768_DCS_write_1A_1P(0xEE, 0xB9);
	//G+
	TC358768_DCS_write_1A_1P(0xEF, 0x00);  
	TC358768_DCS_write_1A_1P(0xF0, 0xE8); 
	TC358768_DCS_write_1A_1P(0xF1, 0x00);
	TC358768_DCS_write_1A_1P(0xF2, 0xF0);
	TC358768_DCS_write_1A_1P(0xF3, 0x01);
	TC358768_DCS_write_1A_1P(0xF4, 0x00);
	TC358768_DCS_write_1A_1P(0xF5, 0x01);
	TC358768_DCS_write_1A_1P(0xF6, 0x0E);
	TC358768_DCS_write_1A_1P(0xF7, 0x01);
	TC358768_DCS_write_1A_1P(0xF8, 0x1A);
	TC358768_DCS_write_1A_1P(0xF9, 0x01);
	TC358768_DCS_write_1A_1P(0xFA, 0x27);
		//Don't Reload MTP
	TC358768_DCS_write_1A_1P(0xFB, 0x01);
	
		//CMD2_P1
	TC358768_DCS_write_1A_1P(0xFF, 0x21);
	
				
	TC358768_DCS_write_1A_1P(0x00, 0x01);
	TC358768_DCS_write_1A_1P(0x01, 0x31);
	TC358768_DCS_write_1A_1P(0x02, 0x01);
	TC358768_DCS_write_1A_1P(0x03, 0x3D);
	TC358768_DCS_write_1A_1P(0x04, 0x01);
	TC358768_DCS_write_1A_1P(0x05, 0x48);
	TC358768_DCS_write_1A_1P(0x06, 0x01);
	TC358768_DCS_write_1A_1P(0x07, 0x6C);
	TC358768_DCS_write_1A_1P(0x08, 0x01);
	TC358768_DCS_write_1A_1P(0x09, 0x8B);
	TC358768_DCS_write_1A_1P(0x0A, 0x01);
	TC358768_DCS_write_1A_1P(0x0B, 0xBE);
	TC358768_DCS_write_1A_1P(0x0C, 0x01);
	TC358768_DCS_write_1A_1P(0x0D, 0xE7);
	TC358768_DCS_write_1A_1P(0x0E, 0x02);
	TC358768_DCS_write_1A_1P(0x0F, 0x2A);
	TC358768_DCS_write_1A_1P(0x10, 0x02);
	TC358768_DCS_write_1A_1P(0x11, 0x5E);
	TC358768_DCS_write_1A_1P(0x12, 0x02);
	TC358768_DCS_write_1A_1P(0x13, 0x60);
	TC358768_DCS_write_1A_1P(0x14, 0x02);
	TC358768_DCS_write_1A_1P(0x15, 0x91);
	TC358768_DCS_write_1A_1P(0x16, 0x02);
	TC358768_DCS_write_1A_1P(0x17, 0xC6);
	TC358768_DCS_write_1A_1P(0x18, 0x02);
	TC358768_DCS_write_1A_1P(0x19, 0xE6);
	TC358768_DCS_write_1A_1P(0x1A, 0x03);
	TC358768_DCS_write_1A_1P(0x1B, 0x0E);
	TC358768_DCS_write_1A_1P(0x1C, 0x03);
	TC358768_DCS_write_1A_1P(0x1D, 0x27);
	TC358768_DCS_write_1A_1P(0x1E, 0x03);
	TC358768_DCS_write_1A_1P(0x1F, 0x4B);
	TC358768_DCS_write_1A_1P(0x20, 0x03);
	TC358768_DCS_write_1A_1P(0x21, 0x54);
	TC358768_DCS_write_1A_1P(0x22, 0x03);
	TC358768_DCS_write_1A_1P(0x23, 0x5F);
	TC358768_DCS_write_1A_1P(0x24, 0x03);
	TC358768_DCS_write_1A_1P(0x25, 0x6B);
	TC358768_DCS_write_1A_1P(0x26, 0x03);
	TC358768_DCS_write_1A_1P(0x27, 0x74);
	TC358768_DCS_write_1A_1P(0x28, 0x03);
	TC358768_DCS_write_1A_1P(0x29, 0x80);
	TC358768_DCS_write_1A_1P(0x2A, 0x03);
	TC358768_DCS_write_1A_1P(0x2B, 0x8B);
	TC358768_DCS_write_1A_1P(0x2D, 0x03);
	TC358768_DCS_write_1A_1P(0x2F, 0x96);
	TC358768_DCS_write_1A_1P(0x30, 0x03);
	TC358768_DCS_write_1A_1P(0x31, 0xB9);
	
	 //G-
	TC358768_DCS_write_1A_1P(0x32, 0x00); 
	TC358768_DCS_write_1A_1P(0x33, 0xE8);
	TC358768_DCS_write_1A_1P(0x34, 0x00);
	TC358768_DCS_write_1A_1P(0x35, 0xF0);
	TC358768_DCS_write_1A_1P(0x36, 0x01);
	TC358768_DCS_write_1A_1P(0x37, 0x00);
	TC358768_DCS_write_1A_1P(0x38, 0x01);
	TC358768_DCS_write_1A_1P(0x39, 0x0E);
	TC358768_DCS_write_1A_1P(0x3A, 0x01);
	TC358768_DCS_write_1A_1P(0x3B, 0x1A);
	TC358768_DCS_write_1A_1P(0x3D, 0x01);
	TC358768_DCS_write_1A_1P(0x3F, 0x27);
	TC358768_DCS_write_1A_1P(0x40, 0x01);
	TC358768_DCS_write_1A_1P(0x41, 0x31);
	TC358768_DCS_write_1A_1P(0x42, 0x01);
	TC358768_DCS_write_1A_1P(0x43, 0x3D);
	TC358768_DCS_write_1A_1P(0x44, 0x01);
	TC358768_DCS_write_1A_1P(0x45, 0x48);
	TC358768_DCS_write_1A_1P(0x46, 0x01);
	TC358768_DCS_write_1A_1P(0x47, 0x6C);
	TC358768_DCS_write_1A_1P(0x48, 0x01);
	TC358768_DCS_write_1A_1P(0x49, 0x8B);
	TC358768_DCS_write_1A_1P(0x4A, 0x01);
	TC358768_DCS_write_1A_1P(0x4B, 0xBE);
	TC358768_DCS_write_1A_1P(0x4C, 0x01);
	TC358768_DCS_write_1A_1P(0x4D, 0xE7);
	TC358768_DCS_write_1A_1P(0x4E, 0x02);
	TC358768_DCS_write_1A_1P(0x4F, 0x2A);
	TC358768_DCS_write_1A_1P(0x50, 0x02);
	TC358768_DCS_write_1A_1P(0x51, 0x5E);
	TC358768_DCS_write_1A_1P(0x52, 0x02);
	TC358768_DCS_write_1A_1P(0x53, 0x60);
	TC358768_DCS_write_1A_1P(0x54, 0x02);
	TC358768_DCS_write_1A_1P(0x55, 0x91);
	TC358768_DCS_write_1A_1P(0x56, 0x02);
	TC358768_DCS_write_1A_1P(0x58, 0xC6);
	TC358768_DCS_write_1A_1P(0x59, 0x02);
	TC358768_DCS_write_1A_1P(0x5A, 0xE6);
	TC358768_DCS_write_1A_1P(0x5B, 0x03);
	TC358768_DCS_write_1A_1P(0x5C, 0x0E);
	TC358768_DCS_write_1A_1P(0x5D, 0x03);
	TC358768_DCS_write_1A_1P(0x5E, 0x27);
	TC358768_DCS_write_1A_1P(0x5F, 0x03);
	TC358768_DCS_write_1A_1P(0x60, 0x4B);
	TC358768_DCS_write_1A_1P(0x61, 0x03);
	TC358768_DCS_write_1A_1P(0x62, 0x54);
	TC358768_DCS_write_1A_1P(0x63, 0x03);
	TC358768_DCS_write_1A_1P(0x64, 0x5F);
	TC358768_DCS_write_1A_1P(0x65, 0x03);
	TC358768_DCS_write_1A_1P(0x66, 0x6B);
	TC358768_DCS_write_1A_1P(0x67, 0x03);
	TC358768_DCS_write_1A_1P(0x68, 0x74);
	TC358768_DCS_write_1A_1P(0x69, 0x03);
	TC358768_DCS_write_1A_1P(0x6A, 0x80);
	TC358768_DCS_write_1A_1P(0x6B, 0x03);
	TC358768_DCS_write_1A_1P(0x6C, 0x8B);
	TC358768_DCS_write_1A_1P(0x6D, 0x03);
	TC358768_DCS_write_1A_1P(0x6E, 0x96);
	TC358768_DCS_write_1A_1P(0x6F, 0x03);
	TC358768_DCS_write_1A_1P(0x70, 0xB9);
	
	//B+	
	TC358768_DCS_write_1A_1P(0x71, 0x00);  
	TC358768_DCS_write_1A_1P(0x72, 0x0A);
	TC358768_DCS_write_1A_1P(0x73, 0x00);
	TC358768_DCS_write_1A_1P(0x74, 0x41);
	TC358768_DCS_write_1A_1P(0x75, 0x00);
	TC358768_DCS_write_1A_1P(0x76, 0x77);
	TC358768_DCS_write_1A_1P(0x77, 0x00);
	TC358768_DCS_write_1A_1P(0x78, 0x9B);
	TC358768_DCS_write_1A_1P(0x79, 0x00);
	TC358768_DCS_write_1A_1P(0x7A, 0xB6);
	TC358768_DCS_write_1A_1P(0x7B, 0x00);
	TC358768_DCS_write_1A_1P(0x7C, 0xCD);
	TC358768_DCS_write_1A_1P(0x7D, 0x00);
	TC358768_DCS_write_1A_1P(0x7E, 0xE2);
	TC358768_DCS_write_1A_1P(0x7F, 0x00);
	TC358768_DCS_write_1A_1P(0x80, 0xF4);
	TC358768_DCS_write_1A_1P(0x81, 0x01);
	TC358768_DCS_write_1A_1P(0x82, 0x03);
	TC358768_DCS_write_1A_1P(0x83, 0x01);
	TC358768_DCS_write_1A_1P(0x84, 0x38);
	TC358768_DCS_write_1A_1P(0x85, 0x01);
	TC358768_DCS_write_1A_1P(0x86, 0x61);
	TC358768_DCS_write_1A_1P(0x87, 0x01);
	TC358768_DCS_write_1A_1P(0x88, 0xA1);
	TC358768_DCS_write_1A_1P(0x89, 0x01);
	TC358768_DCS_write_1A_1P(0x8A, 0xD2);
	TC358768_DCS_write_1A_1P(0x8B, 0x02);
	TC358768_DCS_write_1A_1P(0x8C, 0x16);
	TC358768_DCS_write_1A_1P(0x8D, 0x02);
	TC358768_DCS_write_1A_1P(0x8E, 0x53);
	TC358768_DCS_write_1A_1P(0x8F, 0x02);
	TC358768_DCS_write_1A_1P(0x90, 0x58);
	TC358768_DCS_write_1A_1P(0x91, 0x02);
	TC358768_DCS_write_1A_1P(0x92, 0x8A);
	TC358768_DCS_write_1A_1P(0x93, 0x02);
	TC358768_DCS_write_1A_1P(0x94, 0xC0);
	TC358768_DCS_write_1A_1P(0x95, 0x02);
	TC358768_DCS_write_1A_1P(0x96, 0xDE);
	TC358768_DCS_write_1A_1P(0x97, 0x03);
	TC358768_DCS_write_1A_1P(0x98, 0x0A);
	TC358768_DCS_write_1A_1P(0x99, 0x03);
	TC358768_DCS_write_1A_1P(0x9A, 0x22);
	TC358768_DCS_write_1A_1P(0x9B, 0x03);
	TC358768_DCS_write_1A_1P(0x9C, 0x48);
	TC358768_DCS_write_1A_1P(0x9D, 0x03);
	TC358768_DCS_write_1A_1P(0x9E, 0x52);
	TC358768_DCS_write_1A_1P(0x9F, 0x03);
	TC358768_DCS_write_1A_1P(0xA0, 0x5D);
	TC358768_DCS_write_1A_1P(0xA2, 0x03);
	TC358768_DCS_write_1A_1P(0xA3, 0x6A);
	TC358768_DCS_write_1A_1P(0xA4, 0x03);
	TC358768_DCS_write_1A_1P(0xA5, 0x76);
	TC358768_DCS_write_1A_1P(0xA6, 0x03);
	TC358768_DCS_write_1A_1P(0xA7, 0x80);
	TC358768_DCS_write_1A_1P(0xA9, 0x03);
	TC358768_DCS_write_1A_1P(0xAA, 0x87);
	TC358768_DCS_write_1A_1P(0xAB, 0x03);
	TC358768_DCS_write_1A_1P(0xAC, 0x96);
	TC358768_DCS_write_1A_1P(0xAD, 0x03);
	TC358768_DCS_write_1A_1P(0xAE, 0xB9);
	//B-
	TC358768_DCS_write_1A_1P(0xAF, 0x00); 
	TC358768_DCS_write_1A_1P(0xB0, 0x0A);
	TC358768_DCS_write_1A_1P(0xB1, 0x00);
	TC358768_DCS_write_1A_1P(0xB2, 0x41);
	TC358768_DCS_write_1A_1P(0xB3, 0x00);
	TC358768_DCS_write_1A_1P(0xB4, 0x77);
	TC358768_DCS_write_1A_1P(0xB5, 0x00);
	TC358768_DCS_write_1A_1P(0xB6, 0x9B);
	TC358768_DCS_write_1A_1P(0xB7, 0x00);
	TC358768_DCS_write_1A_1P(0xB8, 0xB6);
	TC358768_DCS_write_1A_1P(0xB9, 0x00);
	TC358768_DCS_write_1A_1P(0xBA, 0xCD);
	TC358768_DCS_write_1A_1P(0xBB, 0x00);
	TC358768_DCS_write_1A_1P(0xBC, 0xE2);
	TC358768_DCS_write_1A_1P(0xBD, 0x00);
	TC358768_DCS_write_1A_1P(0xBE, 0xF4);
	TC358768_DCS_write_1A_1P(0xBF, 0x01);
	TC358768_DCS_write_1A_1P(0xC0, 0x03);
	TC358768_DCS_write_1A_1P(0xC1, 0x01);
	TC358768_DCS_write_1A_1P(0xC2, 0x38);
	TC358768_DCS_write_1A_1P(0xC3, 0x01);
	TC358768_DCS_write_1A_1P(0xC4, 0x61);
	TC358768_DCS_write_1A_1P(0xC5, 0x01);
	TC358768_DCS_write_1A_1P(0xC6, 0xA1);
	TC358768_DCS_write_1A_1P(0xC7, 0x01);
	TC358768_DCS_write_1A_1P(0xC8, 0xD2);
	TC358768_DCS_write_1A_1P(0xC9, 0x02);
	TC358768_DCS_write_1A_1P(0xCA, 0x16);
	TC358768_DCS_write_1A_1P(0xCB, 0x02);
	TC358768_DCS_write_1A_1P(0xCC, 0x53);
	TC358768_DCS_write_1A_1P(0xCD, 0x02);
	TC358768_DCS_write_1A_1P(0xCE, 0x58);
	TC358768_DCS_write_1A_1P(0xCF, 0x02);
	TC358768_DCS_write_1A_1P(0xD0, 0x8A);
	TC358768_DCS_write_1A_1P(0xD1, 0x02);
	TC358768_DCS_write_1A_1P(0xD2, 0xC0);
	TC358768_DCS_write_1A_1P(0xD3, 0x02);
	TC358768_DCS_write_1A_1P(0xD4, 0xDE);
	TC358768_DCS_write_1A_1P(0xD5, 0x03);
	TC358768_DCS_write_1A_1P(0xD6, 0x0A);
	TC358768_DCS_write_1A_1P(0xD7, 0x03);
	TC358768_DCS_write_1A_1P(0xD8, 0x22);
	TC358768_DCS_write_1A_1P(0xD9, 0x03);
	TC358768_DCS_write_1A_1P(0xDA, 0x48);
	TC358768_DCS_write_1A_1P(0xDB, 0x03);
	TC358768_DCS_write_1A_1P(0xDC, 0x52);
	TC358768_DCS_write_1A_1P(0xDD, 0x03);
	TC358768_DCS_write_1A_1P(0xDE, 0x5D);
	TC358768_DCS_write_1A_1P(0xDF, 0x03);
	TC358768_DCS_write_1A_1P(0xE0, 0x6A);
	TC358768_DCS_write_1A_1P(0xE1, 0x03);
	TC358768_DCS_write_1A_1P(0xE2, 0x76);
	TC358768_DCS_write_1A_1P(0xE3, 0x03);
	TC358768_DCS_write_1A_1P(0xE4, 0x80);
	TC358768_DCS_write_1A_1P(0xE5, 0x03);
	TC358768_DCS_write_1A_1P(0xE6, 0x87);
	TC358768_DCS_write_1A_1P(0xE7, 0x03);
	TC358768_DCS_write_1A_1P(0xE8, 0x96);
	TC358768_DCS_write_1A_1P(0xE9, 0x03);
	TC358768_DCS_write_1A_1P(0xEA, 0xB9);
	TC358768_DCS_write_1A_1P(0xFF, 0x10);	

}
static void lcm_setgamma_warm(void)
{
	TC358768_DCS_write_1A_1P(0xFF,0x20); // Page 0, power-related setting
	MDELAY(1);	  

	//R+
	TC358768_DCS_write_1A_1P(0x75, 0x00);	
	TC358768_DCS_write_1A_1P(0x76, 0x0A);	
	TC358768_DCS_write_1A_1P(0x77, 0x00);
	TC358768_DCS_write_1A_1P(0x78, 0x41);	
	TC358768_DCS_write_1A_1P(0x79, 0x00);
	TC358768_DCS_write_1A_1P(0x7A, 0x78);	
	TC358768_DCS_write_1A_1P(0x7B, 0x00);
	TC358768_DCS_write_1A_1P(0x7C, 0x9B);  
	TC358768_DCS_write_1A_1P(0x7D, 0x00);
	TC358768_DCS_write_1A_1P(0x7E, 0xB7);	
	TC358768_DCS_write_1A_1P(0x7F, 0x00);
	TC358768_DCS_write_1A_1P(0x80, 0xCD); 
	TC358768_DCS_write_1A_1P(0x81, 0x00);
	TC358768_DCS_write_1A_1P(0x82, 0xE2); 
	TC358768_DCS_write_1A_1P(0x83, 0x00);
	TC358768_DCS_write_1A_1P(0x84, 0xF5); 
	TC358768_DCS_write_1A_1P(0x85, 0x01);
	TC358768_DCS_write_1A_1P(0x86, 0x05);  
	TC358768_DCS_write_1A_1P(0x87, 0x01);
	TC358768_DCS_write_1A_1P(0x88, 0x38);  
	TC358768_DCS_write_1A_1P(0x89, 0x01);
	TC358768_DCS_write_1A_1P(0x8A, 0x61);  
	TC358768_DCS_write_1A_1P(0x8B, 0x01);
	TC358768_DCS_write_1A_1P(0x8C, 0xA1);  
	TC358768_DCS_write_1A_1P(0x8D, 0x01);
	TC358768_DCS_write_1A_1P(0x8E, 0xD1);  
	TC358768_DCS_write_1A_1P(0x8F, 0x02);
	TC358768_DCS_write_1A_1P(0x90, 0x1C);  
	TC358768_DCS_write_1A_1P(0x91, 0x02);
	TC358768_DCS_write_1A_1P(0x92, 0x56); 
	TC358768_DCS_write_1A_1P(0x93, 0x02);
	TC358768_DCS_write_1A_1P(0x94, 0x58);  
	TC358768_DCS_write_1A_1P(0x95, 0x02);
	TC358768_DCS_write_1A_1P(0x96, 0x8A);  
	TC358768_DCS_write_1A_1P(0x97, 0x02);
	TC358768_DCS_write_1A_1P(0x98, 0xC0);  
	TC358768_DCS_write_1A_1P(0x99, 0x02);
	TC358768_DCS_write_1A_1P(0x9A, 0xDE);  
	TC358768_DCS_write_1A_1P(0x9B, 0x03);
	TC358768_DCS_write_1A_1P(0x9C, 0x0A);  
	TC358768_DCS_write_1A_1P(0x9D, 0x03);
	TC358768_DCS_write_1A_1P(0x9E, 0x22);  
	TC358768_DCS_write_1A_1P(0x9F, 0x03);
	TC358768_DCS_write_1A_1P(0xA0, 0x48);	
	TC358768_DCS_write_1A_1P(0xA2, 0x03);
	TC358768_DCS_write_1A_1P(0xA3, 0x52);  
	TC358768_DCS_write_1A_1P(0xA4, 0x03);
	TC358768_DCS_write_1A_1P(0xA5, 0x5D);	
	TC358768_DCS_write_1A_1P(0xA6, 0x03);
	TC358768_DCS_write_1A_1P(0xA7, 0x6A);  
	TC358768_DCS_write_1A_1P(0xA9, 0x03);
	TC358768_DCS_write_1A_1P(0xAA, 0x76);  
	TC358768_DCS_write_1A_1P(0xAB, 0x03);
	TC358768_DCS_write_1A_1P(0xAC, 0x80);	
	TC358768_DCS_write_1A_1P(0xAD, 0x03);
	TC358768_DCS_write_1A_1P(0xAE, 0x87);	
	TC358768_DCS_write_1A_1P(0xAF, 0x03);
	TC358768_DCS_write_1A_1P(0xB0, 0x96);	
	TC358768_DCS_write_1A_1P(0xB1, 0x03);
	TC358768_DCS_write_1A_1P(0xB2, 0xB9);	
	
	//R-											
	TC358768_DCS_write_1A_1P(0xB3, 0x00); 
	TC358768_DCS_write_1A_1P(0xB4, 0x0A); 
	TC358768_DCS_write_1A_1P(0xB5, 0x00);
	TC358768_DCS_write_1A_1P(0xB6, 0x41); 
	TC358768_DCS_write_1A_1P(0xB7, 0x00);
	TC358768_DCS_write_1A_1P(0xB8, 0x78); 
	TC358768_DCS_write_1A_1P(0xB9, 0x00);
	TC358768_DCS_write_1A_1P(0xBA, 0x9B); 
	TC358768_DCS_write_1A_1P(0xBB, 0x00);
	TC358768_DCS_write_1A_1P(0xBC, 0xB7);
	TC358768_DCS_write_1A_1P(0xBD, 0x00);
	TC358768_DCS_write_1A_1P(0xBE, 0xCD);
	TC358768_DCS_write_1A_1P(0xBF, 0x00);
	TC358768_DCS_write_1A_1P(0xC0, 0xE2);
	TC358768_DCS_write_1A_1P(0xC1, 0x00);
	TC358768_DCS_write_1A_1P(0xC2, 0xF5);
	TC358768_DCS_write_1A_1P(0xC3, 0x01);
	TC358768_DCS_write_1A_1P(0xC4, 0x05);
	TC358768_DCS_write_1A_1P(0xC5, 0x01);
	TC358768_DCS_write_1A_1P(0xC6, 0x38);
	TC358768_DCS_write_1A_1P(0xC7, 0x01);
	TC358768_DCS_write_1A_1P(0xC8, 0x61);
	TC358768_DCS_write_1A_1P(0xC9, 0x01);
	TC358768_DCS_write_1A_1P(0xCA, 0xA1);
	TC358768_DCS_write_1A_1P(0xCB, 0x01);
	TC358768_DCS_write_1A_1P(0xCC, 0xD1);
	TC358768_DCS_write_1A_1P(0xCD, 0x02);
	TC358768_DCS_write_1A_1P(0xCE, 0x1C);
	TC358768_DCS_write_1A_1P(0xCF, 0x02);
	TC358768_DCS_write_1A_1P(0xD0, 0x56);
	TC358768_DCS_write_1A_1P(0xD1, 0x02);
	TC358768_DCS_write_1A_1P(0xD2, 0x58);
	TC358768_DCS_write_1A_1P(0xD3, 0x02);
	TC358768_DCS_write_1A_1P(0xD4, 0x8A);
	TC358768_DCS_write_1A_1P(0xD5, 0x02);
	TC358768_DCS_write_1A_1P(0xD6, 0xC0);
	TC358768_DCS_write_1A_1P(0xD7, 0x02);
	TC358768_DCS_write_1A_1P(0xD8, 0xDE);
	TC358768_DCS_write_1A_1P(0xD9, 0x03);
	TC358768_DCS_write_1A_1P(0xDA, 0x0A);
	TC358768_DCS_write_1A_1P(0xDB, 0x03);
	TC358768_DCS_write_1A_1P(0xDC, 0x22);
	TC358768_DCS_write_1A_1P(0xDD, 0x03);
	TC358768_DCS_write_1A_1P(0xDE, 0x48);
	TC358768_DCS_write_1A_1P(0xDF, 0x03);
	TC358768_DCS_write_1A_1P(0xE0, 0x52);
	TC358768_DCS_write_1A_1P(0xE1, 0x03);
	TC358768_DCS_write_1A_1P(0xE2, 0x5D);
	TC358768_DCS_write_1A_1P(0xE3, 0x03);
	TC358768_DCS_write_1A_1P(0xE4, 0x6A);
	TC358768_DCS_write_1A_1P(0xE5, 0x03);
	TC358768_DCS_write_1A_1P(0xE6, 0x76);
	TC358768_DCS_write_1A_1P(0xE7, 0x03);
	TC358768_DCS_write_1A_1P(0xE8, 0x80);
	TC358768_DCS_write_1A_1P(0xE9, 0x03);
	TC358768_DCS_write_1A_1P(0xEA, 0x87);
	TC358768_DCS_write_1A_1P(0xEB, 0x03);
	TC358768_DCS_write_1A_1P(0xEC, 0x96);
	TC358768_DCS_write_1A_1P(0xED, 0x03);
	TC358768_DCS_write_1A_1P(0xEE, 0xB9);
	
	//G+
	TC358768_DCS_write_1A_1P(0xEF, 0x00);  
	TC358768_DCS_write_1A_1P(0xF0, 0x0A); 
	TC358768_DCS_write_1A_1P(0xF1, 0x00);
	TC358768_DCS_write_1A_1P(0xF2, 0x41);
	TC358768_DCS_write_1A_1P(0xF3, 0x00);
	TC358768_DCS_write_1A_1P(0xF4, 0x78);
	TC358768_DCS_write_1A_1P(0xF5, 0x00);
	TC358768_DCS_write_1A_1P(0xF6, 0x9B);
	TC358768_DCS_write_1A_1P(0xF7, 0x00);
	TC358768_DCS_write_1A_1P(0xF8, 0xB7);
	TC358768_DCS_write_1A_1P(0xF9, 0x00);
	TC358768_DCS_write_1A_1P(0xFA, 0xCD);
	
	TC358768_DCS_write_1A_1P(0xFB, 0x01); //Don't Reload MTP
	
	TC358768_DCS_write_1A_1P(0xFF,0x21); // Page 1, power-related setting	-->Angus Modified
	MDELAY(1);
						
	//Green Gamma Code_Continuous 
	TC358768_DCS_write_1A_1P(0x00, 0x00);
	TC358768_DCS_write_1A_1P(0x01, 0xE2);
	TC358768_DCS_write_1A_1P(0x02, 0x00);
	TC358768_DCS_write_1A_1P(0x03, 0xF5);
	TC358768_DCS_write_1A_1P(0x04, 0x01);
	TC358768_DCS_write_1A_1P(0x05, 0x05);
	TC358768_DCS_write_1A_1P(0x06, 0x01);
	TC358768_DCS_write_1A_1P(0x07, 0x38);
	TC358768_DCS_write_1A_1P(0x08, 0x01);
	TC358768_DCS_write_1A_1P(0x09, 0x61);
	TC358768_DCS_write_1A_1P(0x0A, 0x01);
	TC358768_DCS_write_1A_1P(0x0B, 0xA1);
	TC358768_DCS_write_1A_1P(0x0C, 0x01);
	TC358768_DCS_write_1A_1P(0x0D, 0xD1);
	TC358768_DCS_write_1A_1P(0x0E, 0x02);
	TC358768_DCS_write_1A_1P(0x0F, 0x1C);
	TC358768_DCS_write_1A_1P(0x10, 0x02);
	TC358768_DCS_write_1A_1P(0x11, 0x56);
	TC358768_DCS_write_1A_1P(0x12, 0x02);
	TC358768_DCS_write_1A_1P(0x13, 0x58);
	TC358768_DCS_write_1A_1P(0x14, 0x02);
	TC358768_DCS_write_1A_1P(0x15, 0x8A);
	TC358768_DCS_write_1A_1P(0x16, 0x02);
	TC358768_DCS_write_1A_1P(0x17, 0xC0);
	TC358768_DCS_write_1A_1P(0x18, 0x02);
	TC358768_DCS_write_1A_1P(0x19, 0xDE);
	TC358768_DCS_write_1A_1P(0x1A, 0x03);
	TC358768_DCS_write_1A_1P(0x1B, 0x0A);
	TC358768_DCS_write_1A_1P(0x1C, 0x03);
	TC358768_DCS_write_1A_1P(0x1D, 0x22);
	TC358768_DCS_write_1A_1P(0x1E, 0x03);
	TC358768_DCS_write_1A_1P(0x1F, 0x48);
	TC358768_DCS_write_1A_1P(0x20, 0x03);
	TC358768_DCS_write_1A_1P(0x21, 0x52);
	TC358768_DCS_write_1A_1P(0x22, 0x03);
	TC358768_DCS_write_1A_1P(0x23, 0x5D);
	TC358768_DCS_write_1A_1P(0x24, 0x03);
	TC358768_DCS_write_1A_1P(0x25, 0x6A);
	TC358768_DCS_write_1A_1P(0x26, 0x03);
	TC358768_DCS_write_1A_1P(0x27, 0x76);
	TC358768_DCS_write_1A_1P(0x28, 0x03);
	TC358768_DCS_write_1A_1P(0x29, 0x80);
	TC358768_DCS_write_1A_1P(0x2A, 0x03);
	TC358768_DCS_write_1A_1P(0x2B, 0x87);
	TC358768_DCS_write_1A_1P(0x2D, 0x03);
	TC358768_DCS_write_1A_1P(0x2F, 0x96);
	TC358768_DCS_write_1A_1P(0x30, 0x03);
	TC358768_DCS_write_1A_1P(0x31, 0xB9);
	
	//G-
	TC358768_DCS_write_1A_1P(0x32, 0x00);  
	TC358768_DCS_write_1A_1P(0x33, 0x0A);
	TC358768_DCS_write_1A_1P(0x34, 0x00);
	TC358768_DCS_write_1A_1P(0x35, 0x41);
	TC358768_DCS_write_1A_1P(0x36, 0x00);
	TC358768_DCS_write_1A_1P(0x37, 0x78);
	TC358768_DCS_write_1A_1P(0x38, 0x00);
	TC358768_DCS_write_1A_1P(0x39, 0x9B);
	TC358768_DCS_write_1A_1P(0x3A, 0x00);
	TC358768_DCS_write_1A_1P(0x3B, 0xB7);
	TC358768_DCS_write_1A_1P(0x3D, 0x00);
	TC358768_DCS_write_1A_1P(0x3F, 0xCD);
	TC358768_DCS_write_1A_1P(0x40, 0x00);
	TC358768_DCS_write_1A_1P(0x41, 0xE2);
	TC358768_DCS_write_1A_1P(0x42, 0x00);
	TC358768_DCS_write_1A_1P(0x43, 0xF5);
	TC358768_DCS_write_1A_1P(0x44, 0x01);
	TC358768_DCS_write_1A_1P(0x45, 0x05);
	TC358768_DCS_write_1A_1P(0x46, 0x01);
	TC358768_DCS_write_1A_1P(0x47, 0x38);
	TC358768_DCS_write_1A_1P(0x48, 0x01);
	TC358768_DCS_write_1A_1P(0x49, 0x61);
	TC358768_DCS_write_1A_1P(0x4A, 0x01);
	TC358768_DCS_write_1A_1P(0x4B, 0xA1);
	TC358768_DCS_write_1A_1P(0x4C, 0x01);
	TC358768_DCS_write_1A_1P(0x4D, 0xD1);
	TC358768_DCS_write_1A_1P(0x4E, 0x02);
	TC358768_DCS_write_1A_1P(0x4F, 0x1C);
	TC358768_DCS_write_1A_1P(0x50, 0x02);
	TC358768_DCS_write_1A_1P(0x51, 0x56);
	TC358768_DCS_write_1A_1P(0x52, 0x02);
	TC358768_DCS_write_1A_1P(0x53, 0x58);
	TC358768_DCS_write_1A_1P(0x54, 0x02);
	TC358768_DCS_write_1A_1P(0x55, 0x8A);
	TC358768_DCS_write_1A_1P(0x56, 0x02);
	TC358768_DCS_write_1A_1P(0x58, 0xC0);
	TC358768_DCS_write_1A_1P(0x59, 0x02);
	TC358768_DCS_write_1A_1P(0x5A, 0xDE);
	TC358768_DCS_write_1A_1P(0x5B, 0x03);
	TC358768_DCS_write_1A_1P(0x5C, 0x0A);
	TC358768_DCS_write_1A_1P(0x5D, 0x03);
	TC358768_DCS_write_1A_1P(0x5E, 0x22);
	TC358768_DCS_write_1A_1P(0x5F, 0x03);
	TC358768_DCS_write_1A_1P(0x60, 0x48);
	TC358768_DCS_write_1A_1P(0x61, 0x03);
	TC358768_DCS_write_1A_1P(0x62, 0x52);
	TC358768_DCS_write_1A_1P(0x63, 0x03);
	TC358768_DCS_write_1A_1P(0x64, 0x5D);
	TC358768_DCS_write_1A_1P(0x65, 0x03);
	TC358768_DCS_write_1A_1P(0x66, 0x6A);
	TC358768_DCS_write_1A_1P(0x67, 0x03);
	TC358768_DCS_write_1A_1P(0x68, 0x76);
	TC358768_DCS_write_1A_1P(0x69, 0x03);
	TC358768_DCS_write_1A_1P(0x6A, 0x80);
	TC358768_DCS_write_1A_1P(0x6B, 0x03);
	TC358768_DCS_write_1A_1P(0x6C, 0x87);
	TC358768_DCS_write_1A_1P(0x6D, 0x03);
	TC358768_DCS_write_1A_1P(0x6E, 0x96);
	TC358768_DCS_write_1A_1P(0x6F, 0x03);
	TC358768_DCS_write_1A_1P(0x70, 0xB9);
	
	//B+ 
	TC358768_DCS_write_1A_1P(0x71, 0x00);  
	TC358768_DCS_write_1A_1P(0x72, 0x0A);
	TC358768_DCS_write_1A_1P(0x73, 0x00);
	TC358768_DCS_write_1A_1P(0x74, 0x41);
	TC358768_DCS_write_1A_1P(0x75, 0x00);
	TC358768_DCS_write_1A_1P(0x76, 0x78);
	TC358768_DCS_write_1A_1P(0x77, 0x00);
	TC358768_DCS_write_1A_1P(0x78, 0x9B);
	TC358768_DCS_write_1A_1P(0x79, 0x00);
	TC358768_DCS_write_1A_1P(0x7A, 0xB7);
	TC358768_DCS_write_1A_1P(0x7B, 0x00);
	TC358768_DCS_write_1A_1P(0x7C, 0xCD);
	TC358768_DCS_write_1A_1P(0x7D, 0x00);
	TC358768_DCS_write_1A_1P(0x7E, 0xE2);
	TC358768_DCS_write_1A_1P(0x7F, 0x00);
	TC358768_DCS_write_1A_1P(0x80, 0xF5);
	TC358768_DCS_write_1A_1P(0x81, 0x01);
	TC358768_DCS_write_1A_1P(0x82, 0x05);
	TC358768_DCS_write_1A_1P(0x83, 0x01);
	TC358768_DCS_write_1A_1P(0x84, 0x38);
	TC358768_DCS_write_1A_1P(0x85, 0x01);
	TC358768_DCS_write_1A_1P(0x86, 0x61);
	TC358768_DCS_write_1A_1P(0x87, 0x01);
	TC358768_DCS_write_1A_1P(0x88, 0xA1);
	TC358768_DCS_write_1A_1P(0x89, 0x01);
	TC358768_DCS_write_1A_1P(0x8A, 0xD1);
	TC358768_DCS_write_1A_1P(0x8B, 0x02);
	TC358768_DCS_write_1A_1P(0x8C, 0x1C);
	TC358768_DCS_write_1A_1P(0x8D, 0x02);
	TC358768_DCS_write_1A_1P(0x8E, 0x56);
	TC358768_DCS_write_1A_1P(0x8F, 0x02);
	TC358768_DCS_write_1A_1P(0x90, 0x58);
	TC358768_DCS_write_1A_1P(0x91, 0x02);
	TC358768_DCS_write_1A_1P(0x92, 0x8A);
	TC358768_DCS_write_1A_1P(0x93, 0x02);
	TC358768_DCS_write_1A_1P(0x94, 0xC0);
	TC358768_DCS_write_1A_1P(0x95, 0x02);
	TC358768_DCS_write_1A_1P(0x96, 0xDE);
	TC358768_DCS_write_1A_1P(0x97, 0x03);
	TC358768_DCS_write_1A_1P(0x98, 0x0A);
	TC358768_DCS_write_1A_1P(0x99, 0x03);
	TC358768_DCS_write_1A_1P(0x9A, 0x22);
	TC358768_DCS_write_1A_1P(0x9B, 0x03);
	TC358768_DCS_write_1A_1P(0x9C, 0x48);
	TC358768_DCS_write_1A_1P(0x9D, 0x03);
	TC358768_DCS_write_1A_1P(0x9E, 0x52);
	TC358768_DCS_write_1A_1P(0x9F, 0x03);
	TC358768_DCS_write_1A_1P(0xA0, 0x5D);
	TC358768_DCS_write_1A_1P(0xA2, 0x03);
	TC358768_DCS_write_1A_1P(0xA3, 0x6A);
	TC358768_DCS_write_1A_1P(0xA4, 0x03);
	TC358768_DCS_write_1A_1P(0xA5, 0x76);
	TC358768_DCS_write_1A_1P(0xA6, 0x03);
	TC358768_DCS_write_1A_1P(0xA7, 0x80);
	TC358768_DCS_write_1A_1P(0xA9, 0x03);
	TC358768_DCS_write_1A_1P(0xAA, 0x87);
	TC358768_DCS_write_1A_1P(0xAB, 0x03);
	TC358768_DCS_write_1A_1P(0xAC, 0x96);
	TC358768_DCS_write_1A_1P(0xAD, 0x03);
	TC358768_DCS_write_1A_1P(0xAE, 0xB9);
	
	//B-
	TC358768_DCS_write_1A_1P(0xAF, 0x00); 
	TC358768_DCS_write_1A_1P(0xB0, 0x0A);
	TC358768_DCS_write_1A_1P(0xB1, 0x00);
	TC358768_DCS_write_1A_1P(0xB2, 0x41);
	TC358768_DCS_write_1A_1P(0xB3, 0x00);
	TC358768_DCS_write_1A_1P(0xB4, 0x78);
	TC358768_DCS_write_1A_1P(0xB5, 0x00);
	TC358768_DCS_write_1A_1P(0xB6, 0x9B);
	TC358768_DCS_write_1A_1P(0xB7, 0x00);
	TC358768_DCS_write_1A_1P(0xB8, 0xB7);
	TC358768_DCS_write_1A_1P(0xB9, 0x00);
	TC358768_DCS_write_1A_1P(0xBA, 0xCD);
	TC358768_DCS_write_1A_1P(0xBB, 0x00);
	TC358768_DCS_write_1A_1P(0xBC, 0xE2);
	TC358768_DCS_write_1A_1P(0xBD, 0x00);
	TC358768_DCS_write_1A_1P(0xBE, 0xF5);
	TC358768_DCS_write_1A_1P(0xBF, 0x01);
	TC358768_DCS_write_1A_1P(0xC0, 0x05);
	TC358768_DCS_write_1A_1P(0xC1, 0x01);
	TC358768_DCS_write_1A_1P(0xC2, 0x38);
	TC358768_DCS_write_1A_1P(0xC3, 0x01);
	TC358768_DCS_write_1A_1P(0xC4, 0x61);
	TC358768_DCS_write_1A_1P(0xC5, 0x01);
	TC358768_DCS_write_1A_1P(0xC6, 0xA1);
	TC358768_DCS_write_1A_1P(0xC7, 0x01);
	TC358768_DCS_write_1A_1P(0xC8, 0xD1);
	TC358768_DCS_write_1A_1P(0xC9, 0x02);
	TC358768_DCS_write_1A_1P(0xCA, 0x1C);
	TC358768_DCS_write_1A_1P(0xCB, 0x02);
	TC358768_DCS_write_1A_1P(0xCC, 0x56);
	TC358768_DCS_write_1A_1P(0xCD, 0x02);
	TC358768_DCS_write_1A_1P(0xCE, 0x58);
	TC358768_DCS_write_1A_1P(0xCF, 0x02);
	TC358768_DCS_write_1A_1P(0xD0, 0x8A);
	TC358768_DCS_write_1A_1P(0xD1, 0x02);
	TC358768_DCS_write_1A_1P(0xD2, 0xC0);
	TC358768_DCS_write_1A_1P(0xD3, 0x02);
	TC358768_DCS_write_1A_1P(0xD4, 0xDE);
	TC358768_DCS_write_1A_1P(0xD5, 0x03);
	TC358768_DCS_write_1A_1P(0xD6, 0x0A);
	TC358768_DCS_write_1A_1P(0xD7, 0x03);
	TC358768_DCS_write_1A_1P(0xD8, 0x22);
	TC358768_DCS_write_1A_1P(0xD9, 0x03);
	TC358768_DCS_write_1A_1P(0xDA, 0x48);
	TC358768_DCS_write_1A_1P(0xDB, 0x03);
	TC358768_DCS_write_1A_1P(0xDC, 0x52);
	TC358768_DCS_write_1A_1P(0xDD, 0x03);
	TC358768_DCS_write_1A_1P(0xDE, 0x5D);
	TC358768_DCS_write_1A_1P(0xDF, 0x03);
	TC358768_DCS_write_1A_1P(0xE0, 0x6A);
	TC358768_DCS_write_1A_1P(0xE1, 0x03);
	TC358768_DCS_write_1A_1P(0xE2, 0x76);
	TC358768_DCS_write_1A_1P(0xE3, 0x03);
	TC358768_DCS_write_1A_1P(0xE4, 0x80);
	TC358768_DCS_write_1A_1P(0xE5, 0x03);
	TC358768_DCS_write_1A_1P(0xE6, 0x87);
	TC358768_DCS_write_1A_1P(0xE7, 0x03);
	TC358768_DCS_write_1A_1P(0xE8, 0x96);
	TC358768_DCS_write_1A_1P(0xE9, 0x03);
	TC358768_DCS_write_1A_1P(0xEA, 0xB9);  
	TC358768_DCS_write_1A_1P(0xFF, 0x10);

}

static void lcm_setgammamode(unsigned int mode)
{

	unsigned int data_array[16];
	#if BUILD_LK
	printf("%s mode=%d\n",__func__,mode);
	#else
	printk("%s mode=%d\n",__func__,mode);
	#endif

	 if(lcm_gammamode_index == mode)
		return;
	lcm_gammamode_index = mode;
	/*
	switch(mode){
		case 0:
			lcm_setgamma_normal();
			break;
		case 1:
			lcm_setgamma_cold();
			break;
		case 2:
			lcm_setgamma_warm();
			break;
		default:
			break;
	}
	*/
//MDELAY(10);
}
static void lcm_setgammamode_in_suspend(unsigned int mode)
{
	 if(lcm_gammamode_index2 == mode)
		return;
	lcm_gammamode_index2 = mode;

	switch(mode){
		case 0:
			lcm_setgamma_normal();
			break;
		case 1:
			lcm_setgamma_cold();
			break;
		case 2:
			lcm_setgamma_warm();
			break;
		default:
			break;
	}
//MDELAY(10);
}


static void lcm_setiemode(unsigned int mode)
{

	#if BUILD_LK
	printf("%s mode=%d\n",__func__,mode);
	#else
	printk("%s mode=%d\n",__func__,mode);
	#endif

	if(mode)
		lcm_iemode_index=4;
	else
		lcm_iemode_index=0;

	 lcm_set_pq_mode();

	 MDELAY(10);
}
static void lcm_getiemode(unsigned int * mode)
{

	if(lcm_iemode_index == 0){
		*mode = 0;
	}else{
		*mode = 1;
	}
	return;
}
static void lcm_reg_set(lcm_reg * lcmregs)
{
	if(lcmregs==NULL) return;
	#ifndef BUILD_LK
	printk("[JX] %s cmd=0x%x data=0x%x\n",__func__,lcmregs->cmd,lcmregs->data);
	#endif
	 TC358768_DCS_write_1A_1P(lcmregs->cmd,lcmregs->data);

	 MDELAY(10);
}
static void lcm_reg_get(lcm_reg * lcmregs)
{
	unsigned char buffer[2],ret;
	unsigned int data_array[16];

	if(lcmregs==NULL) return;



	data_array[0]= 0x00003700 | (1 << 16);	
	dsi_set_cmdq(&data_array, 1, 1);
					
	TC358768_DCS_write_1A_1P(0xFF,0x10);// CMD1
	MDELAY(1);

	ret = read_reg_v2(lcmregs->cmd, buffer,2);
#ifndef BUILD_LK
	printk("[JX] %s cmd=0x%x bf[0]=0x%x bf[1]=0x%x\n",__func__,lcmregs->cmd,buffer[0],buffer[1]);
#endif
		lcmregs->data = buffer[0];
		
	MDELAY(10);
	return;
}

/***lenovo.sw2 houdz 20131210 add :support lcm inverse **start***/
static void lcm_setInverse(unsigned int on)
{
	unsigned int data_array[16];	
	#if BUILD_LK
	printf("%s on=%d\n",__func__,on);
	#else
	printk("%s on=%d\n",__func__,on);
	#endif

	switch(on){
		case 0:
			data_array[0]=0x00200500; 
			dsi_set_cmdq(data_array, 1, 1);
			break;
		case 1:
			data_array[0]=0x00210500; 
			dsi_set_cmdq(data_array, 1, 1);
			break;
		default:
			break;
		}

	 MDELAY(10);
}
/***lenovo.sw2 houdz 20131210 add :support lcm inverse **end***/

static void lcm_setcabcmode(unsigned int mode)
{

	#if BUILD_LK
	printf("%s mode=%d\n",__func__,mode);
	#else
	printk("%s mode=%d\n",__func__,mode);
	#endif
	lcm_cabcmode_index = mode;
	lcm_set_pq_mode();

	 MDELAY(10);
}



static void lcm_getcabcstate(unsigned int * state)
{

	return lcm_cabcmode_index;
}
static unsigned int lcm_compare_ic_id(void)
{

	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  
/*
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20); 
*/
	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; //we only need ID
    #ifdef BUILD_LK
		printf("%s, id = 0x%x\n", __func__, id);
    #else
		printk("%s, id = 0x%x\n", __func__, id);
    #endif
	if(lcm_id==LCM_ID_NT35595)
		return 1;
	else if(id == LCM_ID_NT35595)
    	return 1;
    else
        return 0;
}


LCM_DRIVER nt35595_fhd_dsi_vdo_cmi_lcm_drv = 
{
    .name			= "nt35595_fhd_dsi_vdo_cmi",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
#ifdef LENOVO_LCD_BACKLIGHT_CONTROL_BY_LCM
	.set_backlight	= lcm_setbacklight,
#endif
	.esd_check	 	= lcm_esd_check,
	.esd_recover	= lcm_esd_recover,
	#ifdef LENOVO_LCM_EFFECT //lenovo add by jixu@lenovo.com
	.set_inversemode = lcm_setInverse, //lenovo.sw2 houdz 20131210 add :support lcm inverse
	.set_cabcmode = lcm_setcabcmode,
	.get_cabcmode = lcm_getcabcstate,
	.compare_ic_id = lcm_compare_ic_id,
	.set_iemode = lcm_setiemode,
	.get_iemode = lcm_getiemode,
	.set_gammamode = lcm_setgammamode,
	.set_lcm_reg = lcm_reg_set,
	.get_lcm_reg = lcm_reg_get,
	#endif
    };
