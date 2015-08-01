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

static bool lcm_is_init = false;
static unsigned int lcm_cabcmode_index = 3;
static unsigned int lcm_iemode_index = 0;
static unsigned int lcm_gammamode_index = 0;
static unsigned int lcm_gammamode_index2 = 0;
static bool esd_check=false;
#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu
extern int rtc_value;
#endif
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
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TC358768_DCS_write_1A_1P(0xFF,0x20);// Switch to CMD2 Page 0 --> Angus Modified
	
	MDELAY(1);
	TC358768_DCS_write_1A_1P(0xfb,0x01);// Don't reload MTP --> Angus Modified
	
	TC358768_DCS_write_1A_1P(0xFF,0x21);
	MDELAY(1);
	TC358768_DCS_write_1A_1P(0xfb,0x01);
	
	//Angus Modified
	TC358768_DCS_write_1A_1P(0xFF,0x23); //CMD2 Page 3
	MDELAY(1);
	TC358768_DCS_write_1A_1P(0xfb,0x01);
	TC358768_DCS_write_1A_1P(0xFF,0x24);
	MDELAY(1);
	TC358768_DCS_write_1A_1P(0xfb,0x01);
	TC358768_DCS_write_1A_1P(0x6E,0x10);//disable ram keep add by jixu
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////);
	
	// SRGB/SMX_REG/CTB/CRL
	//TC358768_DCS_write_1A_1P(0x9D,0xB0);// FWD
	TC358768_DCS_write_1A_1P(0x9D,0xB0);// BWD
	
	// Resolution Setting (1920x1080)
	TC358768_DCS_write_1A_1P(0x72,0x00);
	
	// BP/FP Setting for Command Mode (BP=4, FP=4)
	TC358768_DCS_write_1A_1P(0x93,0x0F);// FP
	TC358768_DCS_write_1A_1P(0x94,0x0F);// BP
	
	// Set Inversion Type
	TC358768_DCS_write_1A_1P(0x9B,0x0F);// Sub-pixel Column inversion
	TC358768_DCS_write_1A_1P(0x8A,0x33);// MUX sequence option2 
	
	// Less Transition Function Setting 
	TC358768_DCS_write_1A_1P(0x86,0x1B);// Alternation
	TC358768_DCS_write_1A_1P(0x87,0x39);// Alternation
	TC358768_DCS_write_1A_1P(0x88,0x1B);// Alternation
	TC358768_DCS_write_1A_1P(0x89,0x39);// Alternation
	TC358768_DCS_write_1A_1P(0x8B,0xF4);// MUX_LTF (Less Transition) 
	TC358768_DCS_write_1A_1P(0x8C,0x01);// Alternation
	
	// RTN Setting
	TC358768_DCS_write_1A_1P(0x90,0x73);// 0x79 RTNA_V = DisplayClock / (FrameRate * (DisplayLines+BP+FP))	// 14M / (60 * (1920+4+4)) = 121 = 79h
	TC358768_DCS_write_1A_1P(0x91,0x44);// DIVB/RTNA_V/DIVA/RTNB/RTNA2/RTNA
	TC358768_DCS_write_1A_1P(0x92,0x73);// 0x79 RTNA
	
	// CGOUT Mapping 
	TC358768_DCS_write_1A_1P(0x00,0x0F);
	TC358768_DCS_write_1A_1P(0x01,0x00);
	TC358768_DCS_write_1A_1P(0x02,0x00);
	TC358768_DCS_write_1A_1P(0x03,0x00);
	TC358768_DCS_write_1A_1P(0x04,0x0B);
	TC358768_DCS_write_1A_1P(0x05,0x0C);
	TC358768_DCS_write_1A_1P(0x06,0x00);
	TC358768_DCS_write_1A_1P(0x07,0x00);
	TC358768_DCS_write_1A_1P(0x08,0x00);
	TC358768_DCS_write_1A_1P(0x09,0x00);
	TC358768_DCS_write_1A_1P(0x0A,0x03);
	TC358768_DCS_write_1A_1P(0x0B,0x04);
	TC358768_DCS_write_1A_1P(0x0C,0x01);
	TC358768_DCS_write_1A_1P(0x0D,0x13);
	TC358768_DCS_write_1A_1P(0x0E,0x15);
	TC358768_DCS_write_1A_1P(0x0F,0x17);
	TC358768_DCS_write_1A_1P(0x10,0x0F);
	TC358768_DCS_write_1A_1P(0x11,0x00);
	TC358768_DCS_write_1A_1P(0x12,0x00);
	TC358768_DCS_write_1A_1P(0x13,0x00);
	TC358768_DCS_write_1A_1P(0x14,0x0B);
	TC358768_DCS_write_1A_1P(0x15,0x0C);
	TC358768_DCS_write_1A_1P(0x16,0x00);
	TC358768_DCS_write_1A_1P(0x17,0x00);
	TC358768_DCS_write_1A_1P(0x18,0x00);
	TC358768_DCS_write_1A_1P(0x19,0x00);
	TC358768_DCS_write_1A_1P(0x1A,0x03);
	TC358768_DCS_write_1A_1P(0x1B,0x04);
	TC358768_DCS_write_1A_1P(0x1C,0x01);
	TC358768_DCS_write_1A_1P(0x1D,0x13);
	TC358768_DCS_write_1A_1P(0x1E,0x15);
	TC358768_DCS_write_1A_1P(0x1F,0x17);
	
	// Start Pulse related(GVST), GVST Setting 
	TC358768_DCS_write_1A_1P(0x20,0x09);// Start Pulse related (GVST)
	TC358768_DCS_write_1A_1P(0x21,0x01);// STV_RISE_SEL 
	TC358768_DCS_write_1A_1P(0x22,0x00);// STV_FALL_SEL
	TC358768_DCS_write_1A_1P(0x23,0x00);// FS_REG
	TC358768_DCS_write_1A_1P(0x24,0x00);// FW_REG
	TC358768_DCS_write_1A_1P(0x25,0x6D);// What ? ==> GVST setting for power-on sequence (pls. set to,0x6D)
	TC358768_DCS_write_1A_1P(0x26,0x00);// FS_REG_V
	TC358768_DCS_write_1A_1P(0x27,0x00);// FW_REG_V
	
	// GCK(GCLK1/GCLK2), GCLK Setting
	TC358768_DCS_write_1A_1P(0x2F,0x02);// GCK(GCLK1/GCLK2)
	TC358768_DCS_write_1A_1P(0x30,0x04);		 // What ?	==> Set H/L toogle line numbers of each GCK and GCK toogle sequence number
	TC358768_DCS_write_1A_1P(0x31,0x49);		 // What ?	==> Set GCK Sequence in Forward/Backward scan
	TC358768_DCS_write_1A_1P(0x32,0x23);		 // What ?	==> Set GCK Sequence in Forward/Backward scan
	TC358768_DCS_write_1A_1P(0x33,0x01);// STI_REG/SWI_REG/STI_REG_V/SWI_REG_V/GCK_VPM
	TC358768_DCS_write_1A_1P(0x34,0x00);// STI_REG
	TC358768_DCS_write_1A_1P(0x35,0x69);// SWI_REG
	TC358768_DCS_write_1A_1P(0x36, 0x00);		  // What ?  ==> GCK power-on/off sequence setting
	TC358768_DCS_write_1A_1P(0x37, 0x2D);		  // What ?  ==> GCK power-on/off sequence setting
	TC358768_DCS_write_1A_1P(0x38, 0x08);		  // What ?  ==> GCK power-on/off sequence setting
	TC358768_DCS_write_1A_1P(0x39,0x00);// STI_REG_V
	TC358768_DCS_write_1A_1P(0x3A,0x69);// SWI_REG_V
	
	// UD (U2D/D2U), U2D,D2U 
	TC358768_DCS_write_1A_1P(0x29,0x58);// UD(U2D, D2U)
	TC358768_DCS_write_1A_1P(0x2A,0x16);
	
	// CTRL (APO, use CTRL1), APO);););    
	TC358768_DCS_write_1A_1P(0x5B,0x00);// CTRL (APO, use CTRL1)
	TC358768_DCS_write_1A_1P(0x5F,0x75);
	TC358768_DCS_write_1A_1P(0x63,0x00);
	TC358768_DCS_write_1A_1P(0x67,0x04);
	
	// MUX (SERx/xSELx), MUX 
	//TC358768_DCS_write_1A_1P(0x7A, 0x01); 	   // What? MUX(SERx/xSELx) ==> Not used, pls. remove it
	TC358768_DCS_write_1A_1P(0x7B, 0x80);		  // What?				   ==> MUX sequence setting in Display Line
	TC358768_DCS_write_1A_1P(0x7C, 0xD8);		  // What?				   ==> MUX power-on/off sequencesetting
	TC358768_DCS_write_1A_1P(0x7D, 0x60);		  // What?				   ==> MUX power-on/off sequencesetting
	TC358768_DCS_write_1A_1P(0x7E,0x0b);// MUXS
	TC358768_DCS_write_1A_1P(0x7F,0x17);// MUXW
	TC358768_DCS_write_1A_1P(0x80,0x00);// SD_SHIFT_POS (MUX Rising EQ)
	TC358768_DCS_write_1A_1P(0x81,0x06);// MUXG1
	TC358768_DCS_write_1A_1P(0x82,0x03);// MUXG2
	TC358768_DCS_write_1A_1P(0x83,0x00);// EQT_POS (Source Rising EQ)
	TC358768_DCS_write_1A_1P(0x84,0x03);// SOEHT (Source OP enable Time) MUXG1+MUXG2-SOEHT>=4
	TC358768_DCS_write_1A_1P(0x85,0x07);// SDT_REG (Source Output Hold Time)
	TC358768_DCS_write_1A_1P(0x74,0x0b);// MUXS_V (MUX Rising Portion)
	TC358768_DCS_write_1A_1P(0x75,0x17);// MUXW_V (MUX On Time)
	TC358768_DCS_write_1A_1P(0x76,0x06);// MUXG1_V (MUX non-overlap Time)
	TC358768_DCS_write_1A_1P(0x77,0x03);// MUXG2_V (MUX non-overlap Time / b wo RGB switch)
	
	// Source Control
	TC358768_DCS_write_1A_1P(0x78,0x00);// SD_SHIFT_NEG (MUX Falling EQ)
	TC358768_DCS_write_1A_1P(0x79,0x00);// EQ_NEG (Source Falling EQ)
	TC358768_DCS_write_1A_1P(0x99,0x33);// PTA/SPDUMA
	//TC358768_DCS_write_1A_1P(0xA0,0x33);// PTB/SPDUMB
	TC358768_DCS_write_1A_1P(0x98,0x00);
	
	// LTPS Abnormal PWROFF Ctrl. 
	TC358768_DCS_write_1A_1P(0xB3,0x28);
	TC358768_DCS_write_1A_1P(0xB4,0x05);
	TC358768_DCS_write_1A_1P(0xB5,0x10);
	
	// CTRL PIN
	//TC358768_DCS_write_1A_1P(0xC4,0x24);// FTE = FTE, FTE1 = STB Signal
	//TC358768_DCS_write_1A_1P(0xC5,0x30);// LEDPWM = LEDPWM, VSOUT = GND
	//TC358768_DCS_write_1A_1P(0xC6,0x00);// HSOUT = GND
	
	TC358768_DCS_write_1A_1P(0xFF,0x20);// CMD2 Page0, power-related setting 
	MDELAY(1); 
	
	
	TC358768_DCS_write_1A_1P(0x00,0x01);// Panel Type (Normally Black)
	TC358768_DCS_write_1A_1P(0x01,0x55);// DCA_2 / DCA_3
	TC358768_DCS_write_1A_1P(0x02,0x45);// DCA_4 / DCB_2
	TC358768_DCS_write_1A_1P(0x03,0x55);// DCB_3 / DCB_4
	TC358768_DCS_write_1A_1P(0x05,0x50);// VGH(2AVDD) / VGL(2AVEE) Voltage 
	TC358768_DCS_write_1A_1P(0x06,0xA8);// VGH w/Clamping (+10V)
	TC358768_DCS_write_1A_1P(0x07,0xB2);// VGL w/Clamping (-10V) 
	TC358768_DCS_write_1A_1P(0x08,0x0C);// VCI1_regulator/VCL_regulator/VCL_chargepump/BTA3(VCI/-VCI)
	TC358768_DCS_write_1A_1P(0x0B,0xB3);// GVDDP 72 3.99 / 88 4.21 / A8 4.53 / B9 4.7 / C3 4.8 / CD 4.9 / D7 5.0 / E1 5..1 / EB 5.2 / F0 5.25
	TC358768_DCS_write_1A_1P(0x0C,0xB3);// GVDDL 72 3.99 / 88 4.21 / A8 4.53 / B9 4.7 / C3 4.8 / CD 4.9 / D7 5.0 / E1 5..1 / EB 5.2 / F0 5.25
	TC358768_DCS_write_1A_1P(0x0E,0xB5);// VGHO_REG (+10V)
	TC358768_DCS_write_1A_1P(0x0F,0xB8);// VGLO_REG (-10V)
	TC358768_DCS_write_1A_1P(0x11,0x2F);// Forward Vcom,0x11F (-0.32V)
	TC358768_DCS_write_1A_1P(0x12,0x2F);// Backward Vcom,0x11F (-0.32V)
	TC358768_DCS_write_1A_1P(0x13,0x03);// Vcom Last Bit
	TC358768_DCS_write_1A_1P(0x14,0x0A);// pre-regulating disable
	TC358768_DCS_write_1A_1P(0x15,0x15);// AVDDR +5.0V (Off)
	TC358768_DCS_write_1A_1P(0x16,0x15);// AVEER -5.0V (Off)
	TC358768_DCS_write_1A_1P(0x6D,0x44);// Source OP Bias Current Control  
	TC358768_DCS_write_1A_1P(0x58,0x05);// GCKR_EQT1 (EQ On HighBit 8) - U2D
	TC358768_DCS_write_1A_1P(0x59,0x05);// GCKR_EQT2   
	TC358768_DCS_write_1A_1P(0x5A,0x05);// GCKF_EQT1 
	TC358768_DCS_write_1A_1P(0x5B,0x05);// GCKF_EQT2 
	TC358768_DCS_write_1A_1P(0x5C,0x00);// MUXR_EQT1 (1EQ On HighBit 8) - GCLK
	TC358768_DCS_write_1A_1P(0x5D,0x00);// MUXR_EQT2 (2EQ On HighBit 8) - MUX
	TC358768_DCS_write_1A_1P(0x5E,0x00);// MUXF_EQT1
	TC358768_DCS_write_1A_1P(0x5F,0x00);// MUXF_EQT2
	
	// PWM Control 
	TC358768_DCS_write_1A_1P(0x1B,0x39);
	TC358768_DCS_write_1A_1P(0x1C,0x39);
	TC358768_DCS_write_1A_1P(0x1D,0x47);
	 
	TC358768_DCS_write_1A_1P(0x18,0x00);//jixu add for fix lcm fps not same between pvt and mp module;
			 
	TC358768_DCS_write_1A_1P(0xFF,0x20);
	TC358768_DCS_write_1A_1P(0xFB,0x01);
	MDELAY(1);

 TC358768_DCS_write_1A_1P(0x72, 0x11);//0x31 //D-IC TA-GO

// R+
TC358768_DCS_write_1A_1P(0x75, 0x00);
TC358768_DCS_write_1A_1P(0x76, 0x5C);
TC358768_DCS_write_1A_1P(0x77, 0x00);
TC358768_DCS_write_1A_1P(0x78, 0x62);
TC358768_DCS_write_1A_1P(0x79, 0x00);
TC358768_DCS_write_1A_1P(0x7A, 0x6E);
TC358768_DCS_write_1A_1P(0x7B, 0x00);
TC358768_DCS_write_1A_1P(0x7C, 0x79);
TC358768_DCS_write_1A_1P(0x7D, 0x00);
TC358768_DCS_write_1A_1P(0x7E, 0x83);
TC358768_DCS_write_1A_1P(0x7F, 0x00);
TC358768_DCS_write_1A_1P(0x80, 0x8E);
TC358768_DCS_write_1A_1P(0x81, 0x00);
TC358768_DCS_write_1A_1P(0x82, 0x98);
TC358768_DCS_write_1A_1P(0x83, 0x00);
TC358768_DCS_write_1A_1P(0x84, 0xA2);
TC358768_DCS_write_1A_1P(0x85, 0x00);
TC358768_DCS_write_1A_1P(0x86, 0xAC);
TC358768_DCS_write_1A_1P(0x87, 0x00);
TC358768_DCS_write_1A_1P(0x88, 0xD0);
TC358768_DCS_write_1A_1P(0x89, 0x00);
TC358768_DCS_write_1A_1P(0x8A, 0xF1);
TC358768_DCS_write_1A_1P(0x8B, 0x01);
TC358768_DCS_write_1A_1P(0x8C, 0x2C);
TC358768_DCS_write_1A_1P(0x8D, 0x01);
TC358768_DCS_write_1A_1P(0x8E, 0x5F);
TC358768_DCS_write_1A_1P(0x8F, 0x01);
TC358768_DCS_write_1A_1P(0x90, 0xB5);
TC358768_DCS_write_1A_1P(0x91, 0x01);
TC358768_DCS_write_1A_1P(0x92, 0xFC);
TC358768_DCS_write_1A_1P(0x93, 0x01);
TC358768_DCS_write_1A_1P(0x94, 0xFE);
TC358768_DCS_write_1A_1P(0x95, 0x02);
TC358768_DCS_write_1A_1P(0x96, 0x3E);
TC358768_DCS_write_1A_1P(0x97, 0x02);
TC358768_DCS_write_1A_1P(0x98, 0x83);
TC358768_DCS_write_1A_1P(0x99, 0x02);
TC358768_DCS_write_1A_1P(0x9A, 0xB0);
TC358768_DCS_write_1A_1P(0x9B, 0x02);
TC358768_DCS_write_1A_1P(0x9C, 0xFB);
TC358768_DCS_write_1A_1P(0x9D, 0x03);
TC358768_DCS_write_1A_1P(0x9E, 0x1F);
TC358768_DCS_write_1A_1P(0x9F, 0x03);
TC358768_DCS_write_1A_1P(0xA0, 0x45);
TC358768_DCS_write_1A_1P(0xA2, 0x03);
TC358768_DCS_write_1A_1P(0xA3, 0x53);
TC358768_DCS_write_1A_1P(0xA4, 0x03);
TC358768_DCS_write_1A_1P(0xA5, 0x61);
TC358768_DCS_write_1A_1P(0xA6, 0x03);
TC358768_DCS_write_1A_1P(0xA7, 0x74);
TC358768_DCS_write_1A_1P(0xA9, 0x03);
TC358768_DCS_write_1A_1P(0xAA, 0x84);
TC358768_DCS_write_1A_1P(0xAB, 0x03);
TC358768_DCS_write_1A_1P(0xAC, 0x97);
TC358768_DCS_write_1A_1P(0xAD, 0x03);
TC358768_DCS_write_1A_1P(0xAE, 0xAE);
TC358768_DCS_write_1A_1P(0xAF, 0x03);
TC358768_DCS_write_1A_1P(0xB0, 0xC6);
TC358768_DCS_write_1A_1P(0xB1, 0x03);
TC358768_DCS_write_1A_1P(0xB2, 0xD4);





// R-
TC358768_DCS_write_1A_1P(0xB3, 0x00);
TC358768_DCS_write_1A_1P(0xB4, 0x5C);
TC358768_DCS_write_1A_1P(0xB5, 0x00);
TC358768_DCS_write_1A_1P(0xB6, 0x62);
TC358768_DCS_write_1A_1P(0xB7, 0x00);
TC358768_DCS_write_1A_1P(0xB8, 0x6E);
TC358768_DCS_write_1A_1P(0xB9, 0x00);
TC358768_DCS_write_1A_1P(0xBA, 0x79);
TC358768_DCS_write_1A_1P(0xBB, 0x00);
TC358768_DCS_write_1A_1P(0xBC, 0x83);
TC358768_DCS_write_1A_1P(0xBD, 0x00);
TC358768_DCS_write_1A_1P(0xBE, 0x8E);
TC358768_DCS_write_1A_1P(0xBF, 0x00);
TC358768_DCS_write_1A_1P(0xC0, 0x98);
TC358768_DCS_write_1A_1P(0xC1, 0x00);
TC358768_DCS_write_1A_1P(0xC2, 0xA2);
TC358768_DCS_write_1A_1P(0xC3, 0x00);
TC358768_DCS_write_1A_1P(0xC4, 0xAC);
TC358768_DCS_write_1A_1P(0xC5, 0x00);
TC358768_DCS_write_1A_1P(0xC6, 0xD0);
TC358768_DCS_write_1A_1P(0xC7, 0x00);
TC358768_DCS_write_1A_1P(0xC8, 0xF1);
TC358768_DCS_write_1A_1P(0xC9, 0x01);
TC358768_DCS_write_1A_1P(0xCA, 0x2C);
TC358768_DCS_write_1A_1P(0xCB, 0x01);
TC358768_DCS_write_1A_1P(0xCC, 0x5F);
TC358768_DCS_write_1A_1P(0xCD, 0x01);
TC358768_DCS_write_1A_1P(0xCE, 0xB5);
TC358768_DCS_write_1A_1P(0xCF, 0x01);
TC358768_DCS_write_1A_1P(0xD0, 0xFC);
TC358768_DCS_write_1A_1P(0xD1, 0x01);
TC358768_DCS_write_1A_1P(0xD2, 0xFE);
TC358768_DCS_write_1A_1P(0xD3, 0x02);
TC358768_DCS_write_1A_1P(0xD4, 0x3E);
TC358768_DCS_write_1A_1P(0xD5, 0x02);
TC358768_DCS_write_1A_1P(0xD6, 0x83);
TC358768_DCS_write_1A_1P(0xD7, 0x02);
TC358768_DCS_write_1A_1P(0xD8, 0xB0);
TC358768_DCS_write_1A_1P(0xD9, 0x02);
TC358768_DCS_write_1A_1P(0xDA, 0xFB);
TC358768_DCS_write_1A_1P(0xDB, 0x03);
TC358768_DCS_write_1A_1P(0xDC, 0x1F);
TC358768_DCS_write_1A_1P(0xDD, 0x03);
TC358768_DCS_write_1A_1P(0xDE, 0x45);
TC358768_DCS_write_1A_1P(0xDF, 0x03);
TC358768_DCS_write_1A_1P(0xE0, 0x53);
TC358768_DCS_write_1A_1P(0xE1, 0x03);
TC358768_DCS_write_1A_1P(0xE2, 0x61);
TC358768_DCS_write_1A_1P(0xE3, 0x03);
TC358768_DCS_write_1A_1P(0xE4, 0x74);
TC358768_DCS_write_1A_1P(0xE5, 0x03);
TC358768_DCS_write_1A_1P(0xE6, 0x84);
TC358768_DCS_write_1A_1P(0xE7, 0x03);
TC358768_DCS_write_1A_1P(0xE8, 0x97);
TC358768_DCS_write_1A_1P(0xE9, 0x03);
TC358768_DCS_write_1A_1P(0xEA, 0xAE);
TC358768_DCS_write_1A_1P(0xEB, 0x03);
TC358768_DCS_write_1A_1P(0xEC, 0xC6);
TC358768_DCS_write_1A_1P(0xED, 0x03);
TC358768_DCS_write_1A_1P(0xEE, 0xD4);

// G+
TC358768_DCS_write_1A_1P(0xEF, 0x00);
TC358768_DCS_write_1A_1P(0xF0, 0x00);
TC358768_DCS_write_1A_1P(0xF1, 0x00);
TC358768_DCS_write_1A_1P(0xF2, 0x08);
TC358768_DCS_write_1A_1P(0xF3, 0x00);
TC358768_DCS_write_1A_1P(0xF4, 0x18);
TC358768_DCS_write_1A_1P(0xF5, 0x00);
TC358768_DCS_write_1A_1P(0xF6, 0x24);
TC358768_DCS_write_1A_1P(0xF7, 0x00);
TC358768_DCS_write_1A_1P(0xF8, 0x32);
TC358768_DCS_write_1A_1P(0xF9, 0x00);
TC358768_DCS_write_1A_1P(0xFA, 0x42);

TC358768_DCS_write_1A_1P(0xFF, 0x21); // Page 0, power-related setting    
TC358768_DCS_write_1A_1P(0xFB, 0x01); //Don't Reload MTP
MDELAY(1);
TC358768_DCS_write_1A_1P(0x00, 0x00);
TC358768_DCS_write_1A_1P(0x01, 0x4F);
TC358768_DCS_write_1A_1P(0x02, 0x00);
TC358768_DCS_write_1A_1P(0x03, 0x5D);
TC358768_DCS_write_1A_1P(0x04, 0x00);
TC358768_DCS_write_1A_1P(0x05, 0x6C);
TC358768_DCS_write_1A_1P(0x06, 0x00);
TC358768_DCS_write_1A_1P(0x07, 0x9F);
TC358768_DCS_write_1A_1P(0x08, 0x00);
TC358768_DCS_write_1A_1P(0x09, 0xC9);
TC358768_DCS_write_1A_1P(0x0A, 0x01);
TC358768_DCS_write_1A_1P(0x0B, 0x11);
TC358768_DCS_write_1A_1P(0x0C, 0x01);
TC358768_DCS_write_1A_1P(0x0D, 0x4C);
TC358768_DCS_write_1A_1P(0x0E, 0x01);
TC358768_DCS_write_1A_1P(0x0F, 0xAC);
TC358768_DCS_write_1A_1P(0x10, 0x01);
TC358768_DCS_write_1A_1P(0x11, 0xF8);
TC358768_DCS_write_1A_1P(0x12, 0x01);
TC358768_DCS_write_1A_1P(0x13, 0xF9);
TC358768_DCS_write_1A_1P(0x14, 0x02);
TC358768_DCS_write_1A_1P(0x15, 0x3C);
TC358768_DCS_write_1A_1P(0x16, 0x02);
TC358768_DCS_write_1A_1P(0x17, 0x80);
TC358768_DCS_write_1A_1P(0x18, 0x02);
TC358768_DCS_write_1A_1P(0x19, 0xAA);
TC358768_DCS_write_1A_1P(0x1A, 0x02);
TC358768_DCS_write_1A_1P(0x1B, 0xF2);
TC358768_DCS_write_1A_1P(0x1C, 0x03);
TC358768_DCS_write_1A_1P(0x1D, 0x14);
TC358768_DCS_write_1A_1P(0x1E, 0x03);
TC358768_DCS_write_1A_1P(0x1F, 0x42);
TC358768_DCS_write_1A_1P(0x20, 0x03);
TC358768_DCS_write_1A_1P(0x21, 0x4F);
TC358768_DCS_write_1A_1P(0x22, 0x03);
TC358768_DCS_write_1A_1P(0x23, 0x60);
TC358768_DCS_write_1A_1P(0x24, 0x03);
TC358768_DCS_write_1A_1P(0x25, 0x74);
TC358768_DCS_write_1A_1P(0x26, 0x03);
TC358768_DCS_write_1A_1P(0x27, 0x84);
TC358768_DCS_write_1A_1P(0x28, 0x03);
TC358768_DCS_write_1A_1P(0x29, 0x93);
TC358768_DCS_write_1A_1P(0x2A, 0x03);
TC358768_DCS_write_1A_1P(0x2B, 0xAB);
TC358768_DCS_write_1A_1P(0x2D, 0x03);
TC358768_DCS_write_1A_1P(0x2F, 0xC5);
TC358768_DCS_write_1A_1P(0x30, 0x03);
TC358768_DCS_write_1A_1P(0x31, 0xD4);
// G-
TC358768_DCS_write_1A_1P(0x32, 0x00);
TC358768_DCS_write_1A_1P(0x33, 0x00);
TC358768_DCS_write_1A_1P(0x34, 0x00);
TC358768_DCS_write_1A_1P(0x35, 0x08);
TC358768_DCS_write_1A_1P(0x36, 0x00);
TC358768_DCS_write_1A_1P(0x37, 0x18);
TC358768_DCS_write_1A_1P(0x38, 0x00);
TC358768_DCS_write_1A_1P(0x39, 0x24);
TC358768_DCS_write_1A_1P(0x3A, 0x00);
TC358768_DCS_write_1A_1P(0x3B, 0x32);
TC358768_DCS_write_1A_1P(0x3D, 0x00);
TC358768_DCS_write_1A_1P(0x3F, 0x42);
TC358768_DCS_write_1A_1P(0x40, 0x00);
TC358768_DCS_write_1A_1P(0x41, 0x4F);
TC358768_DCS_write_1A_1P(0x42, 0x00);
TC358768_DCS_write_1A_1P(0x43, 0x5D);
TC358768_DCS_write_1A_1P(0x44, 0x00);
TC358768_DCS_write_1A_1P(0x45, 0x6C);
TC358768_DCS_write_1A_1P(0x46, 0x00);
TC358768_DCS_write_1A_1P(0x47, 0x9F);
TC358768_DCS_write_1A_1P(0x48, 0x00);
TC358768_DCS_write_1A_1P(0x49, 0xC9);
TC358768_DCS_write_1A_1P(0x4A, 0x01);
TC358768_DCS_write_1A_1P(0x4B, 0x11);
TC358768_DCS_write_1A_1P(0x4C, 0x01);
TC358768_DCS_write_1A_1P(0x4D, 0x4C);
TC358768_DCS_write_1A_1P(0x4E, 0x01);
TC358768_DCS_write_1A_1P(0x4F, 0xAC);
TC358768_DCS_write_1A_1P(0x50, 0x01);
TC358768_DCS_write_1A_1P(0x51, 0xF8);
TC358768_DCS_write_1A_1P(0x52, 0x01);
TC358768_DCS_write_1A_1P(0x53, 0xF9);
TC358768_DCS_write_1A_1P(0x54, 0x02);
TC358768_DCS_write_1A_1P(0x55, 0x3C);
TC358768_DCS_write_1A_1P(0x56, 0x02);
TC358768_DCS_write_1A_1P(0x58, 0x80);
TC358768_DCS_write_1A_1P(0x59, 0x02);
TC358768_DCS_write_1A_1P(0x5A, 0xAA);
TC358768_DCS_write_1A_1P(0x5B, 0x02);
TC358768_DCS_write_1A_1P(0x5C, 0xF2);
TC358768_DCS_write_1A_1P(0x5D, 0x03);
TC358768_DCS_write_1A_1P(0x5E, 0x14);
TC358768_DCS_write_1A_1P(0x5F, 0x03);
TC358768_DCS_write_1A_1P(0x60, 0x42);
TC358768_DCS_write_1A_1P(0x61, 0x03);
TC358768_DCS_write_1A_1P(0x62, 0x4F);
TC358768_DCS_write_1A_1P(0x63, 0x03);
TC358768_DCS_write_1A_1P(0x64, 0x60);
TC358768_DCS_write_1A_1P(0x65, 0x03);
TC358768_DCS_write_1A_1P(0x66, 0x74);
TC358768_DCS_write_1A_1P(0x67, 0x03);
TC358768_DCS_write_1A_1P(0x68, 0x84);
TC358768_DCS_write_1A_1P(0x69, 0x03);
TC358768_DCS_write_1A_1P(0x6A, 0x93);
TC358768_DCS_write_1A_1P(0x6B, 0x03);
TC358768_DCS_write_1A_1P(0x6C, 0xAB);
TC358768_DCS_write_1A_1P(0x6D, 0x03);
TC358768_DCS_write_1A_1P(0x6E, 0xC5);
TC358768_DCS_write_1A_1P(0x6F, 0x03);
TC358768_DCS_write_1A_1P(0x70, 0xD4);
// B+
TC358768_DCS_write_1A_1P(0x71, 0x00);
TC358768_DCS_write_1A_1P(0x72, 0x00);
TC358768_DCS_write_1A_1P(0x73, 0x00);
TC358768_DCS_write_1A_1P(0x74, 0x08);
TC358768_DCS_write_1A_1P(0x75, 0x00);
TC358768_DCS_write_1A_1P(0x76, 0x18);
TC358768_DCS_write_1A_1P(0x77, 0x00);
TC358768_DCS_write_1A_1P(0x78, 0x24);
TC358768_DCS_write_1A_1P(0x79, 0x00);
TC358768_DCS_write_1A_1P(0x7A, 0x32);
TC358768_DCS_write_1A_1P(0x7B, 0x00);
TC358768_DCS_write_1A_1P(0x7C, 0x42);
TC358768_DCS_write_1A_1P(0x7D, 0x00);
TC358768_DCS_write_1A_1P(0x7E, 0x4F);
TC358768_DCS_write_1A_1P(0x7F, 0x00);
TC358768_DCS_write_1A_1P(0x80, 0x5D);
TC358768_DCS_write_1A_1P(0x81, 0x00);
TC358768_DCS_write_1A_1P(0x82, 0x6C);
TC358768_DCS_write_1A_1P(0x83, 0x00);
TC358768_DCS_write_1A_1P(0x84, 0x9F);
TC358768_DCS_write_1A_1P(0x85, 0x00);
TC358768_DCS_write_1A_1P(0x86, 0xC9);
TC358768_DCS_write_1A_1P(0x87, 0x01);
TC358768_DCS_write_1A_1P(0x88, 0x11);
TC358768_DCS_write_1A_1P(0x89, 0x01);
TC358768_DCS_write_1A_1P(0x8A, 0x4C);
TC358768_DCS_write_1A_1P(0x8B, 0x01);
TC358768_DCS_write_1A_1P(0x8C, 0xAC);
TC358768_DCS_write_1A_1P(0x8D, 0x01);
TC358768_DCS_write_1A_1P(0x8E, 0xF8);
TC358768_DCS_write_1A_1P(0x8F, 0x01);
TC358768_DCS_write_1A_1P(0x90, 0xF9);
TC358768_DCS_write_1A_1P(0x91, 0x02);
TC358768_DCS_write_1A_1P(0x92, 0x3C);
TC358768_DCS_write_1A_1P(0x93, 0x02);
TC358768_DCS_write_1A_1P(0x94, 0x80);
TC358768_DCS_write_1A_1P(0x95, 0x02);
TC358768_DCS_write_1A_1P(0x96, 0xAA);
TC358768_DCS_write_1A_1P(0x97, 0x02);
TC358768_DCS_write_1A_1P(0x98, 0xF2);
TC358768_DCS_write_1A_1P(0x99, 0x03);
TC358768_DCS_write_1A_1P(0x9A, 0x14);
TC358768_DCS_write_1A_1P(0x9B, 0x03);
TC358768_DCS_write_1A_1P(0x9C, 0x42);
TC358768_DCS_write_1A_1P(0x9D, 0x03);
TC358768_DCS_write_1A_1P(0x9E, 0x4F);
TC358768_DCS_write_1A_1P(0x9F, 0x03);
TC358768_DCS_write_1A_1P(0xA0, 0x60);
TC358768_DCS_write_1A_1P(0xA2, 0x03);
TC358768_DCS_write_1A_1P(0xA3, 0x74);
TC358768_DCS_write_1A_1P(0xA4, 0x03);
TC358768_DCS_write_1A_1P(0xA5, 0x84);
TC358768_DCS_write_1A_1P(0xA6, 0x03);
TC358768_DCS_write_1A_1P(0xA7, 0x93);
TC358768_DCS_write_1A_1P(0xA9, 0x03);
TC358768_DCS_write_1A_1P(0xAA, 0xAB);
TC358768_DCS_write_1A_1P(0xAB, 0x03);
TC358768_DCS_write_1A_1P(0xAC, 0xC5);
TC358768_DCS_write_1A_1P(0xAD, 0x03);
TC358768_DCS_write_1A_1P(0xAE, 0xD4);
// B-
TC358768_DCS_write_1A_1P(0xAF, 0x00);
TC358768_DCS_write_1A_1P(0xB0, 0x00);
TC358768_DCS_write_1A_1P(0xB1, 0x00);
TC358768_DCS_write_1A_1P(0xB2, 0x08);
TC358768_DCS_write_1A_1P(0xB3, 0x00);
TC358768_DCS_write_1A_1P(0xB4, 0x18);
TC358768_DCS_write_1A_1P(0xB5, 0x00);
TC358768_DCS_write_1A_1P(0xB6, 0x24);
TC358768_DCS_write_1A_1P(0xB7, 0x00);
TC358768_DCS_write_1A_1P(0xB8, 0x32);
TC358768_DCS_write_1A_1P(0xB9, 0x00);
TC358768_DCS_write_1A_1P(0xBA, 0x42);
TC358768_DCS_write_1A_1P(0xBB, 0x00);
TC358768_DCS_write_1A_1P(0xBC, 0x4F);
TC358768_DCS_write_1A_1P(0xBD, 0x00);
TC358768_DCS_write_1A_1P(0xBE, 0x5D);
TC358768_DCS_write_1A_1P(0xBF, 0x00);
TC358768_DCS_write_1A_1P(0xC0, 0x6C);
TC358768_DCS_write_1A_1P(0xC1, 0x00);
TC358768_DCS_write_1A_1P(0xC2, 0x9F);
TC358768_DCS_write_1A_1P(0xC3, 0x00);
TC358768_DCS_write_1A_1P(0xC4, 0xC9);
TC358768_DCS_write_1A_1P(0xC5, 0x01);
TC358768_DCS_write_1A_1P(0xC6, 0x11);
TC358768_DCS_write_1A_1P(0xC7, 0x01);
TC358768_DCS_write_1A_1P(0xC8, 0x4C);
TC358768_DCS_write_1A_1P(0xC9, 0x01);
TC358768_DCS_write_1A_1P(0xCA, 0xAC);
TC358768_DCS_write_1A_1P(0xCB, 0x01);
TC358768_DCS_write_1A_1P(0xCC, 0xF8);
TC358768_DCS_write_1A_1P(0xCD, 0x01);
TC358768_DCS_write_1A_1P(0xCE, 0xF9);
TC358768_DCS_write_1A_1P(0xCF, 0x02);
TC358768_DCS_write_1A_1P(0xD0, 0x3C);
TC358768_DCS_write_1A_1P(0xD1, 0x02);
TC358768_DCS_write_1A_1P(0xD2, 0x80);
TC358768_DCS_write_1A_1P(0xD3, 0x02);
TC358768_DCS_write_1A_1P(0xD4, 0xAA);
TC358768_DCS_write_1A_1P(0xD5, 0x02);
TC358768_DCS_write_1A_1P(0xD6, 0xF2);
TC358768_DCS_write_1A_1P(0xD7, 0x03);
TC358768_DCS_write_1A_1P(0xD8, 0x14);
TC358768_DCS_write_1A_1P(0xD9, 0x03);
TC358768_DCS_write_1A_1P(0xDA, 0x42);
TC358768_DCS_write_1A_1P(0xDB, 0x03);
TC358768_DCS_write_1A_1P(0xDC, 0x4F);
TC358768_DCS_write_1A_1P(0xDD, 0x03);
TC358768_DCS_write_1A_1P(0xDE, 0x60);
TC358768_DCS_write_1A_1P(0xDF, 0x03);
TC358768_DCS_write_1A_1P(0xE0, 0x74);
TC358768_DCS_write_1A_1P(0xE1, 0x03);
TC358768_DCS_write_1A_1P(0xE2, 0x84);
TC358768_DCS_write_1A_1P(0xE3, 0x03);
TC358768_DCS_write_1A_1P(0xE4, 0x93);
TC358768_DCS_write_1A_1P(0xE5, 0x03);
TC358768_DCS_write_1A_1P(0xE6, 0xAB);
TC358768_DCS_write_1A_1P(0xE7, 0x03);
TC358768_DCS_write_1A_1P(0xE8, 0xC5);
TC358768_DCS_write_1A_1P(0xE9, 0x03);
TC358768_DCS_write_1A_1P(0xEA, 0xD4);





	
	//Angus Modified
	TC358768_DCS_write_1A_1P(0xFF,0x23);// CMD2 Page 3
	MDELAY(1);
	//PWM Frequency divided by 4
	TC358768_DCS_write_1A_1P(0x03,0x02);//dimming step for FABC
	TC358768_DCS_write_1A_1P(0x05,0x29);//DIM_STEP_MOV = 64 step
	TC358768_DCS_write_1A_1P(0x08,0x04);
	TC358768_DCS_write_1A_1P(0xFB, 0x01);
	
	TC358768_DCS_write_1A_1P(0xFF, 0x22);
	TC358768_DCS_write_1A_1P(0xFB, 0x01); 
	TC358768_DCS_write_1A_1P(0x56, 0x77); //smart contrast
	TC358768_DCS_write_1A_1P(0x1A, 0x77); //color enhancement
	TC358768_DCS_write_1A_1P(0x68, 0x77); //edge enhancement
	
	TC358768_DCS_write_1A_1P(0xFF,0x10);// CMD1
	MDELAY(1);
	
	TC358768_DCS_write_1A_1P(0x35, 0x00);//te on
	// MIPI Video Mode Bypass RAM
#if (LCM_DSI_CMD_MODE)
	TC358768_DCS_write_1A_1P(0xBB, 0x10);
#else
	TC358768_DCS_write_1A_1P(0xBB, 0x03);
#endif
	
	// Video Mode Porch Setting (VBP,VFP,HBP,HFP)
	//mipi.write,0x39,0x3B,0x03,0x0a,0x0a,0x0a,0x0a
		data_array[0]= 0x00063902;
		data_array[1]= 0x0A0A033B;
		data_array[2]= 0x00000A0A;
		dsi_set_cmdq(data_array, 3, 1);
	
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
	//
	
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
		#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu
		#ifndef BUILD_LK
		printk("[JX] %s rtc_value=%d\n",__func__,rtc_value);
		#else
		printf("[JX] %s rtc_value=%d\n",__func__,rtc_value);
		#endif
		#endif
		params->type   = LCM_TYPE_DSI;
#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu
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
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;	

		
		params->dsi.vertical_sync_active				= 2;
		params->dsi.vertical_backporch					= 8;
		params->dsi.vertical_frontporch					= 10;
#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu
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
#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu
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
#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu
		if(rtc_value==0){	
		params->dsi.fbk_div =14;//0x12;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
		}else{
			params->dsi.fbk_div =6; 
		}
#else
		params->dsi.fbk_div =14;//0x12;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
#endif
	params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_481;//481; 312 305_5
	
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
	//unsigned char buffer[2];

#if 0//ndef BUILD_LK
	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
	
	read_reg_v2(0xFE, buffer, 1);
	printk("%s, kernel nt35596 horse debug: nt35596 id = 0x%08x\n", __func__, buffer[0]);
#endif

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
//		TC358768_DCS_write_1A_0P(0x11);
//		MDELAY(150);
//		TC358768_DCS_write_1A_0P(0x29);
//		MDELAY(10);
//		lcm_esd_check();
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
#ifdef LENOVO_DYNAMIC_RESOLUTION_SUPPORT//jixu
	if(rtc_value==0){
		TC358768_DCS_write_1A_1P(0x58,0x00); 
	}else{
		TC358768_DCS_write_1A_1P(0x58,0x02); 
	}

	#ifdef BUILD_LK
		printf("[JX] %s %d lcm_is_init=%d\n",__func__,__LINE__,lcm_is_init);
	#else
		printk("[JX] %s %d lcm_is_init=%d \n",__func__,__LINE__,lcm_is_init);
	#endif	
#endif
#if 0//ndef BUILD_LK
	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
	
	read_reg_v2(0xFE, buffer, 1);
	printk("%s, kernel nt35596 horse debug: nt35596 id = 0x%08x\n", __func__, buffer[0]);
#endif

//	TC358768_DCS_write_1A_0P(0x11); // Sleep Out
//	MDELAY(150);

//	TC358768_DCS_write_1A_0P(0x29); // Display On

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
	printf("%s, [jx]nt35595 lg GPIO154 = %d \n", __func__, ret);
#endif	

	return (ret == 1)?1:0;

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
	printf("+ %s \n",__func__);
	#else
	printk("+ %s \n",__func__);
	#endif

	lcm_init();

	#if BUILD_LK
	printf("- %s \n",__func__);
	#else
	printk("- %s \n",__func__);
	#endif 
}

static void lcm_setgamma_normal(void)
{
	TC358768_DCS_write_1A_1P(0xFF,0x20);
	TC358768_DCS_write_1A_1P(0xFB,0x01);
	MDELAY(1);

 
// R+
TC358768_DCS_write_1A_1P(0x75, 0x00);
TC358768_DCS_write_1A_1P(0x76, 0x5C);
TC358768_DCS_write_1A_1P(0x77, 0x00);
TC358768_DCS_write_1A_1P(0x78, 0x62);
TC358768_DCS_write_1A_1P(0x79, 0x00);
TC358768_DCS_write_1A_1P(0x7A, 0x6E);
TC358768_DCS_write_1A_1P(0x7B, 0x00);
TC358768_DCS_write_1A_1P(0x7C, 0x79);
TC358768_DCS_write_1A_1P(0x7D, 0x00);
TC358768_DCS_write_1A_1P(0x7E, 0x83);
TC358768_DCS_write_1A_1P(0x7F, 0x00);
TC358768_DCS_write_1A_1P(0x80, 0x8E);
TC358768_DCS_write_1A_1P(0x81, 0x00);
TC358768_DCS_write_1A_1P(0x82, 0x98);
TC358768_DCS_write_1A_1P(0x83, 0x00);
TC358768_DCS_write_1A_1P(0x84, 0xA2);
TC358768_DCS_write_1A_1P(0x85, 0x00);
TC358768_DCS_write_1A_1P(0x86, 0xAC);
TC358768_DCS_write_1A_1P(0x87, 0x00);
TC358768_DCS_write_1A_1P(0x88, 0xD0);
TC358768_DCS_write_1A_1P(0x89, 0x00);
TC358768_DCS_write_1A_1P(0x8A, 0xF1);
TC358768_DCS_write_1A_1P(0x8B, 0x01);
TC358768_DCS_write_1A_1P(0x8C, 0x2C);
TC358768_DCS_write_1A_1P(0x8D, 0x01);
TC358768_DCS_write_1A_1P(0x8E, 0x5F);
TC358768_DCS_write_1A_1P(0x8F, 0x01);
TC358768_DCS_write_1A_1P(0x90, 0xB5);
TC358768_DCS_write_1A_1P(0x91, 0x01);
TC358768_DCS_write_1A_1P(0x92, 0xFC);
TC358768_DCS_write_1A_1P(0x93, 0x01);
TC358768_DCS_write_1A_1P(0x94, 0xFE);
TC358768_DCS_write_1A_1P(0x95, 0x02);
TC358768_DCS_write_1A_1P(0x96, 0x3E);
TC358768_DCS_write_1A_1P(0x97, 0x02);
TC358768_DCS_write_1A_1P(0x98, 0x83);
TC358768_DCS_write_1A_1P(0x99, 0x02);
TC358768_DCS_write_1A_1P(0x9A, 0xB0);
TC358768_DCS_write_1A_1P(0x9B, 0x02);
TC358768_DCS_write_1A_1P(0x9C, 0xFB);
TC358768_DCS_write_1A_1P(0x9D, 0x03);
TC358768_DCS_write_1A_1P(0x9E, 0x1F);
TC358768_DCS_write_1A_1P(0x9F, 0x03);
TC358768_DCS_write_1A_1P(0xA0, 0x45);
TC358768_DCS_write_1A_1P(0xA2, 0x03);
TC358768_DCS_write_1A_1P(0xA3, 0x53);
TC358768_DCS_write_1A_1P(0xA4, 0x03);
TC358768_DCS_write_1A_1P(0xA5, 0x61);
TC358768_DCS_write_1A_1P(0xA6, 0x03);
TC358768_DCS_write_1A_1P(0xA7, 0x74);
TC358768_DCS_write_1A_1P(0xA9, 0x03);
TC358768_DCS_write_1A_1P(0xAA, 0x84);
TC358768_DCS_write_1A_1P(0xAB, 0x03);
TC358768_DCS_write_1A_1P(0xAC, 0x97);
TC358768_DCS_write_1A_1P(0xAD, 0x03);
TC358768_DCS_write_1A_1P(0xAE, 0xAE);
TC358768_DCS_write_1A_1P(0xAF, 0x03);
TC358768_DCS_write_1A_1P(0xB0, 0xC6);
TC358768_DCS_write_1A_1P(0xB1, 0x03);
TC358768_DCS_write_1A_1P(0xB2, 0xD4);





// R-
TC358768_DCS_write_1A_1P(0xB3, 0x00);
TC358768_DCS_write_1A_1P(0xB4, 0x5C);
TC358768_DCS_write_1A_1P(0xB5, 0x00);
TC358768_DCS_write_1A_1P(0xB6, 0x62);
TC358768_DCS_write_1A_1P(0xB7, 0x00);
TC358768_DCS_write_1A_1P(0xB8, 0x6E);
TC358768_DCS_write_1A_1P(0xB9, 0x00);
TC358768_DCS_write_1A_1P(0xBA, 0x79);
TC358768_DCS_write_1A_1P(0xBB, 0x00);
TC358768_DCS_write_1A_1P(0xBC, 0x83);
TC358768_DCS_write_1A_1P(0xBD, 0x00);
TC358768_DCS_write_1A_1P(0xBE, 0x8E);
TC358768_DCS_write_1A_1P(0xBF, 0x00);
TC358768_DCS_write_1A_1P(0xC0, 0x98);
TC358768_DCS_write_1A_1P(0xC1, 0x00);
TC358768_DCS_write_1A_1P(0xC2, 0xA2);
TC358768_DCS_write_1A_1P(0xC3, 0x00);
TC358768_DCS_write_1A_1P(0xC4, 0xAC);
TC358768_DCS_write_1A_1P(0xC5, 0x00);
TC358768_DCS_write_1A_1P(0xC6, 0xD0);
TC358768_DCS_write_1A_1P(0xC7, 0x00);
TC358768_DCS_write_1A_1P(0xC8, 0xF1);
TC358768_DCS_write_1A_1P(0xC9, 0x01);
TC358768_DCS_write_1A_1P(0xCA, 0x2C);
TC358768_DCS_write_1A_1P(0xCB, 0x01);
TC358768_DCS_write_1A_1P(0xCC, 0x5F);
TC358768_DCS_write_1A_1P(0xCD, 0x01);
TC358768_DCS_write_1A_1P(0xCE, 0xB5);
TC358768_DCS_write_1A_1P(0xCF, 0x01);
TC358768_DCS_write_1A_1P(0xD0, 0xFC);
TC358768_DCS_write_1A_1P(0xD1, 0x01);
TC358768_DCS_write_1A_1P(0xD2, 0xFE);
TC358768_DCS_write_1A_1P(0xD3, 0x02);
TC358768_DCS_write_1A_1P(0xD4, 0x3E);
TC358768_DCS_write_1A_1P(0xD5, 0x02);
TC358768_DCS_write_1A_1P(0xD6, 0x83);
TC358768_DCS_write_1A_1P(0xD7, 0x02);
TC358768_DCS_write_1A_1P(0xD8, 0xB0);
TC358768_DCS_write_1A_1P(0xD9, 0x02);
TC358768_DCS_write_1A_1P(0xDA, 0xFB);
TC358768_DCS_write_1A_1P(0xDB, 0x03);
TC358768_DCS_write_1A_1P(0xDC, 0x1F);
TC358768_DCS_write_1A_1P(0xDD, 0x03);
TC358768_DCS_write_1A_1P(0xDE, 0x45);
TC358768_DCS_write_1A_1P(0xDF, 0x03);
TC358768_DCS_write_1A_1P(0xE0, 0x53);
TC358768_DCS_write_1A_1P(0xE1, 0x03);
TC358768_DCS_write_1A_1P(0xE2, 0x61);
TC358768_DCS_write_1A_1P(0xE3, 0x03);
TC358768_DCS_write_1A_1P(0xE4, 0x74);
TC358768_DCS_write_1A_1P(0xE5, 0x03);
TC358768_DCS_write_1A_1P(0xE6, 0x84);
TC358768_DCS_write_1A_1P(0xE7, 0x03);
TC358768_DCS_write_1A_1P(0xE8, 0x97);
TC358768_DCS_write_1A_1P(0xE9, 0x03);
TC358768_DCS_write_1A_1P(0xEA, 0xAE);
TC358768_DCS_write_1A_1P(0xEB, 0x03);
TC358768_DCS_write_1A_1P(0xEC, 0xC6);
TC358768_DCS_write_1A_1P(0xED, 0x03);
TC358768_DCS_write_1A_1P(0xEE, 0xD4);

// G+
TC358768_DCS_write_1A_1P(0xEF, 0x00);
TC358768_DCS_write_1A_1P(0xF0, 0x00);
TC358768_DCS_write_1A_1P(0xF1, 0x00);
TC358768_DCS_write_1A_1P(0xF2, 0x08);
TC358768_DCS_write_1A_1P(0xF3, 0x00);
TC358768_DCS_write_1A_1P(0xF4, 0x18);
TC358768_DCS_write_1A_1P(0xF5, 0x00);
TC358768_DCS_write_1A_1P(0xF6, 0x24);
TC358768_DCS_write_1A_1P(0xF7, 0x00);
TC358768_DCS_write_1A_1P(0xF8, 0x32);
TC358768_DCS_write_1A_1P(0xF9, 0x00);
TC358768_DCS_write_1A_1P(0xFA, 0x42);

TC358768_DCS_write_1A_1P(0xFF, 0x21); // Page 0, power-related setting    
TC358768_DCS_write_1A_1P(0xFB, 0x01); //Don't Reload MTP
MDELAY(1);
TC358768_DCS_write_1A_1P(0x00, 0x00);
TC358768_DCS_write_1A_1P(0x01, 0x4F);
TC358768_DCS_write_1A_1P(0x02, 0x00);
TC358768_DCS_write_1A_1P(0x03, 0x5D);
TC358768_DCS_write_1A_1P(0x04, 0x00);
TC358768_DCS_write_1A_1P(0x05, 0x6C);
TC358768_DCS_write_1A_1P(0x06, 0x00);
TC358768_DCS_write_1A_1P(0x07, 0x9F);
TC358768_DCS_write_1A_1P(0x08, 0x00);
TC358768_DCS_write_1A_1P(0x09, 0xC9);
TC358768_DCS_write_1A_1P(0x0A, 0x01);
TC358768_DCS_write_1A_1P(0x0B, 0x11);
TC358768_DCS_write_1A_1P(0x0C, 0x01);
TC358768_DCS_write_1A_1P(0x0D, 0x4C);
TC358768_DCS_write_1A_1P(0x0E, 0x01);
TC358768_DCS_write_1A_1P(0x0F, 0xAC);
TC358768_DCS_write_1A_1P(0x10, 0x01);
TC358768_DCS_write_1A_1P(0x11, 0xF8);
TC358768_DCS_write_1A_1P(0x12, 0x01);
TC358768_DCS_write_1A_1P(0x13, 0xF9);
TC358768_DCS_write_1A_1P(0x14, 0x02);
TC358768_DCS_write_1A_1P(0x15, 0x3C);
TC358768_DCS_write_1A_1P(0x16, 0x02);
TC358768_DCS_write_1A_1P(0x17, 0x80);
TC358768_DCS_write_1A_1P(0x18, 0x02);
TC358768_DCS_write_1A_1P(0x19, 0xAA);
TC358768_DCS_write_1A_1P(0x1A, 0x02);
TC358768_DCS_write_1A_1P(0x1B, 0xF2);
TC358768_DCS_write_1A_1P(0x1C, 0x03);
TC358768_DCS_write_1A_1P(0x1D, 0x14);
TC358768_DCS_write_1A_1P(0x1E, 0x03);
TC358768_DCS_write_1A_1P(0x1F, 0x42);
TC358768_DCS_write_1A_1P(0x20, 0x03);
TC358768_DCS_write_1A_1P(0x21, 0x4F);
TC358768_DCS_write_1A_1P(0x22, 0x03);
TC358768_DCS_write_1A_1P(0x23, 0x60);
TC358768_DCS_write_1A_1P(0x24, 0x03);
TC358768_DCS_write_1A_1P(0x25, 0x74);
TC358768_DCS_write_1A_1P(0x26, 0x03);
TC358768_DCS_write_1A_1P(0x27, 0x84);
TC358768_DCS_write_1A_1P(0x28, 0x03);
TC358768_DCS_write_1A_1P(0x29, 0x93);
TC358768_DCS_write_1A_1P(0x2A, 0x03);
TC358768_DCS_write_1A_1P(0x2B, 0xAB);
TC358768_DCS_write_1A_1P(0x2D, 0x03);
TC358768_DCS_write_1A_1P(0x2F, 0xC5);
TC358768_DCS_write_1A_1P(0x30, 0x03);
TC358768_DCS_write_1A_1P(0x31, 0xD4);
// G-
TC358768_DCS_write_1A_1P(0x32, 0x00);
TC358768_DCS_write_1A_1P(0x33, 0x00);
TC358768_DCS_write_1A_1P(0x34, 0x00);
TC358768_DCS_write_1A_1P(0x35, 0x08);
TC358768_DCS_write_1A_1P(0x36, 0x00);
TC358768_DCS_write_1A_1P(0x37, 0x18);
TC358768_DCS_write_1A_1P(0x38, 0x00);
TC358768_DCS_write_1A_1P(0x39, 0x24);
TC358768_DCS_write_1A_1P(0x3A, 0x00);
TC358768_DCS_write_1A_1P(0x3B, 0x32);
TC358768_DCS_write_1A_1P(0x3D, 0x00);
TC358768_DCS_write_1A_1P(0x3F, 0x42);
TC358768_DCS_write_1A_1P(0x40, 0x00);
TC358768_DCS_write_1A_1P(0x41, 0x4F);
TC358768_DCS_write_1A_1P(0x42, 0x00);
TC358768_DCS_write_1A_1P(0x43, 0x5D);
TC358768_DCS_write_1A_1P(0x44, 0x00);
TC358768_DCS_write_1A_1P(0x45, 0x6C);
TC358768_DCS_write_1A_1P(0x46, 0x00);
TC358768_DCS_write_1A_1P(0x47, 0x9F);
TC358768_DCS_write_1A_1P(0x48, 0x00);
TC358768_DCS_write_1A_1P(0x49, 0xC9);
TC358768_DCS_write_1A_1P(0x4A, 0x01);
TC358768_DCS_write_1A_1P(0x4B, 0x11);
TC358768_DCS_write_1A_1P(0x4C, 0x01);
TC358768_DCS_write_1A_1P(0x4D, 0x4C);
TC358768_DCS_write_1A_1P(0x4E, 0x01);
TC358768_DCS_write_1A_1P(0x4F, 0xAC);
TC358768_DCS_write_1A_1P(0x50, 0x01);
TC358768_DCS_write_1A_1P(0x51, 0xF8);
TC358768_DCS_write_1A_1P(0x52, 0x01);
TC358768_DCS_write_1A_1P(0x53, 0xF9);
TC358768_DCS_write_1A_1P(0x54, 0x02);
TC358768_DCS_write_1A_1P(0x55, 0x3C);
TC358768_DCS_write_1A_1P(0x56, 0x02);
TC358768_DCS_write_1A_1P(0x58, 0x80);
TC358768_DCS_write_1A_1P(0x59, 0x02);
TC358768_DCS_write_1A_1P(0x5A, 0xAA);
TC358768_DCS_write_1A_1P(0x5B, 0x02);
TC358768_DCS_write_1A_1P(0x5C, 0xF2);
TC358768_DCS_write_1A_1P(0x5D, 0x03);
TC358768_DCS_write_1A_1P(0x5E, 0x14);
TC358768_DCS_write_1A_1P(0x5F, 0x03);
TC358768_DCS_write_1A_1P(0x60, 0x42);
TC358768_DCS_write_1A_1P(0x61, 0x03);
TC358768_DCS_write_1A_1P(0x62, 0x4F);
TC358768_DCS_write_1A_1P(0x63, 0x03);
TC358768_DCS_write_1A_1P(0x64, 0x60);
TC358768_DCS_write_1A_1P(0x65, 0x03);
TC358768_DCS_write_1A_1P(0x66, 0x74);
TC358768_DCS_write_1A_1P(0x67, 0x03);
TC358768_DCS_write_1A_1P(0x68, 0x84);
TC358768_DCS_write_1A_1P(0x69, 0x03);
TC358768_DCS_write_1A_1P(0x6A, 0x93);
TC358768_DCS_write_1A_1P(0x6B, 0x03);
TC358768_DCS_write_1A_1P(0x6C, 0xAB);
TC358768_DCS_write_1A_1P(0x6D, 0x03);
TC358768_DCS_write_1A_1P(0x6E, 0xC5);
TC358768_DCS_write_1A_1P(0x6F, 0x03);
TC358768_DCS_write_1A_1P(0x70, 0xD4);
// B+
TC358768_DCS_write_1A_1P(0x71, 0x00);
TC358768_DCS_write_1A_1P(0x72, 0x00);
TC358768_DCS_write_1A_1P(0x73, 0x00);
TC358768_DCS_write_1A_1P(0x74, 0x08);
TC358768_DCS_write_1A_1P(0x75, 0x00);
TC358768_DCS_write_1A_1P(0x76, 0x18);
TC358768_DCS_write_1A_1P(0x77, 0x00);
TC358768_DCS_write_1A_1P(0x78, 0x24);
TC358768_DCS_write_1A_1P(0x79, 0x00);
TC358768_DCS_write_1A_1P(0x7A, 0x32);
TC358768_DCS_write_1A_1P(0x7B, 0x00);
TC358768_DCS_write_1A_1P(0x7C, 0x42);
TC358768_DCS_write_1A_1P(0x7D, 0x00);
TC358768_DCS_write_1A_1P(0x7E, 0x4F);
TC358768_DCS_write_1A_1P(0x7F, 0x00);
TC358768_DCS_write_1A_1P(0x80, 0x5D);
TC358768_DCS_write_1A_1P(0x81, 0x00);
TC358768_DCS_write_1A_1P(0x82, 0x6C);
TC358768_DCS_write_1A_1P(0x83, 0x00);
TC358768_DCS_write_1A_1P(0x84, 0x9F);
TC358768_DCS_write_1A_1P(0x85, 0x00);
TC358768_DCS_write_1A_1P(0x86, 0xC9);
TC358768_DCS_write_1A_1P(0x87, 0x01);
TC358768_DCS_write_1A_1P(0x88, 0x11);
TC358768_DCS_write_1A_1P(0x89, 0x01);
TC358768_DCS_write_1A_1P(0x8A, 0x4C);
TC358768_DCS_write_1A_1P(0x8B, 0x01);
TC358768_DCS_write_1A_1P(0x8C, 0xAC);
TC358768_DCS_write_1A_1P(0x8D, 0x01);
TC358768_DCS_write_1A_1P(0x8E, 0xF8);
TC358768_DCS_write_1A_1P(0x8F, 0x01);
TC358768_DCS_write_1A_1P(0x90, 0xF9);
TC358768_DCS_write_1A_1P(0x91, 0x02);
TC358768_DCS_write_1A_1P(0x92, 0x3C);
TC358768_DCS_write_1A_1P(0x93, 0x02);
TC358768_DCS_write_1A_1P(0x94, 0x80);
TC358768_DCS_write_1A_1P(0x95, 0x02);
TC358768_DCS_write_1A_1P(0x96, 0xAA);
TC358768_DCS_write_1A_1P(0x97, 0x02);
TC358768_DCS_write_1A_1P(0x98, 0xF2);
TC358768_DCS_write_1A_1P(0x99, 0x03);
TC358768_DCS_write_1A_1P(0x9A, 0x14);
TC358768_DCS_write_1A_1P(0x9B, 0x03);
TC358768_DCS_write_1A_1P(0x9C, 0x42);
TC358768_DCS_write_1A_1P(0x9D, 0x03);
TC358768_DCS_write_1A_1P(0x9E, 0x4F);
TC358768_DCS_write_1A_1P(0x9F, 0x03);
TC358768_DCS_write_1A_1P(0xA0, 0x60);
TC358768_DCS_write_1A_1P(0xA2, 0x03);
TC358768_DCS_write_1A_1P(0xA3, 0x74);
TC358768_DCS_write_1A_1P(0xA4, 0x03);
TC358768_DCS_write_1A_1P(0xA5, 0x84);
TC358768_DCS_write_1A_1P(0xA6, 0x03);
TC358768_DCS_write_1A_1P(0xA7, 0x93);
TC358768_DCS_write_1A_1P(0xA9, 0x03);
TC358768_DCS_write_1A_1P(0xAA, 0xAB);
TC358768_DCS_write_1A_1P(0xAB, 0x03);
TC358768_DCS_write_1A_1P(0xAC, 0xC5);
TC358768_DCS_write_1A_1P(0xAD, 0x03);
TC358768_DCS_write_1A_1P(0xAE, 0xD4);
// B-
TC358768_DCS_write_1A_1P(0xAF, 0x00);
TC358768_DCS_write_1A_1P(0xB0, 0x00);
TC358768_DCS_write_1A_1P(0xB1, 0x00);
TC358768_DCS_write_1A_1P(0xB2, 0x08);
TC358768_DCS_write_1A_1P(0xB3, 0x00);
TC358768_DCS_write_1A_1P(0xB4, 0x18);
TC358768_DCS_write_1A_1P(0xB5, 0x00);
TC358768_DCS_write_1A_1P(0xB6, 0x24);
TC358768_DCS_write_1A_1P(0xB7, 0x00);
TC358768_DCS_write_1A_1P(0xB8, 0x32);
TC358768_DCS_write_1A_1P(0xB9, 0x00);
TC358768_DCS_write_1A_1P(0xBA, 0x42);
TC358768_DCS_write_1A_1P(0xBB, 0x00);
TC358768_DCS_write_1A_1P(0xBC, 0x4F);
TC358768_DCS_write_1A_1P(0xBD, 0x00);
TC358768_DCS_write_1A_1P(0xBE, 0x5D);
TC358768_DCS_write_1A_1P(0xBF, 0x00);
TC358768_DCS_write_1A_1P(0xC0, 0x6C);
TC358768_DCS_write_1A_1P(0xC1, 0x00);
TC358768_DCS_write_1A_1P(0xC2, 0x9F);
TC358768_DCS_write_1A_1P(0xC3, 0x00);
TC358768_DCS_write_1A_1P(0xC4, 0xC9);
TC358768_DCS_write_1A_1P(0xC5, 0x01);
TC358768_DCS_write_1A_1P(0xC6, 0x11);
TC358768_DCS_write_1A_1P(0xC7, 0x01);
TC358768_DCS_write_1A_1P(0xC8, 0x4C);
TC358768_DCS_write_1A_1P(0xC9, 0x01);
TC358768_DCS_write_1A_1P(0xCA, 0xAC);
TC358768_DCS_write_1A_1P(0xCB, 0x01);
TC358768_DCS_write_1A_1P(0xCC, 0xF8);
TC358768_DCS_write_1A_1P(0xCD, 0x01);
TC358768_DCS_write_1A_1P(0xCE, 0xF9);
TC358768_DCS_write_1A_1P(0xCF, 0x02);
TC358768_DCS_write_1A_1P(0xD0, 0x3C);
TC358768_DCS_write_1A_1P(0xD1, 0x02);
TC358768_DCS_write_1A_1P(0xD2, 0x80);
TC358768_DCS_write_1A_1P(0xD3, 0x02);
TC358768_DCS_write_1A_1P(0xD4, 0xAA);
TC358768_DCS_write_1A_1P(0xD5, 0x02);
TC358768_DCS_write_1A_1P(0xD6, 0xF2);
TC358768_DCS_write_1A_1P(0xD7, 0x03);
TC358768_DCS_write_1A_1P(0xD8, 0x14);
TC358768_DCS_write_1A_1P(0xD9, 0x03);
TC358768_DCS_write_1A_1P(0xDA, 0x42);
TC358768_DCS_write_1A_1P(0xDB, 0x03);
TC358768_DCS_write_1A_1P(0xDC, 0x4F);
TC358768_DCS_write_1A_1P(0xDD, 0x03);
TC358768_DCS_write_1A_1P(0xDE, 0x60);
TC358768_DCS_write_1A_1P(0xDF, 0x03);
TC358768_DCS_write_1A_1P(0xE0, 0x74);
TC358768_DCS_write_1A_1P(0xE1, 0x03);
TC358768_DCS_write_1A_1P(0xE2, 0x84);
TC358768_DCS_write_1A_1P(0xE3, 0x03);
TC358768_DCS_write_1A_1P(0xE4, 0x93);
TC358768_DCS_write_1A_1P(0xE5, 0x03);
TC358768_DCS_write_1A_1P(0xE6, 0xAB);
TC358768_DCS_write_1A_1P(0xE7, 0x03);
TC358768_DCS_write_1A_1P(0xE8, 0xC5);
TC358768_DCS_write_1A_1P(0xE9, 0x03);
TC358768_DCS_write_1A_1P(0xEA, 0xD4);

TC358768_DCS_write_1A_1P(0xFF, 0x10);





}

static void lcm_setgamma_cold(void)
{
	TC358768_DCS_write_1A_1P(0xFF,0x20);
	TC358768_DCS_write_1A_1P(0xFB,0x01);
	MDELAY(1);	
	// R+
	TC358768_DCS_write_1A_1P(0x75,0x00);
	TC358768_DCS_write_1A_1P(0x76,0x8D);
	TC358768_DCS_write_1A_1P(0x77,0x00);
	TC358768_DCS_write_1A_1P(0x78,0x94);
	TC358768_DCS_write_1A_1P(0x79,0x00);
	TC358768_DCS_write_1A_1P(0x7A,0xA2);
	TC358768_DCS_write_1A_1P(0x7B,0x00);
	TC358768_DCS_write_1A_1P(0x7C,0xAC);
	TC358768_DCS_write_1A_1P(0x7D,0x00);
	TC358768_DCS_write_1A_1P(0x7E,0xB8);
	TC358768_DCS_write_1A_1P(0x7F,0x00);
	TC358768_DCS_write_1A_1P(0x80,0xC6);
	TC358768_DCS_write_1A_1P(0x81,0x00);
	TC358768_DCS_write_1A_1P(0x82,0xD1);
	TC358768_DCS_write_1A_1P(0x83,0x00);
	TC358768_DCS_write_1A_1P(0x84,0xDD);
	TC358768_DCS_write_1A_1P(0x85,0x00);
	TC358768_DCS_write_1A_1P(0x86,0xEA);
	TC358768_DCS_write_1A_1P(0x87,0x01);
	TC358768_DCS_write_1A_1P(0x88,0x16);
	TC358768_DCS_write_1A_1P(0x89,0x01);
	TC358768_DCS_write_1A_1P(0x8A,0x3A);
	TC358768_DCS_write_1A_1P(0x8B,0x01);
	TC358768_DCS_write_1A_1P(0x8C,0x78);
	TC358768_DCS_write_1A_1P(0x8D,0x01);
	TC358768_DCS_write_1A_1P(0x8E,0xAB);
	TC358768_DCS_write_1A_1P(0x8F,0x01);
	TC358768_DCS_write_1A_1P(0x90,0xFE);
	TC358768_DCS_write_1A_1P(0x91,0x02);
	TC358768_DCS_write_1A_1P(0x92,0x3F);
	TC358768_DCS_write_1A_1P(0x93,0x02);
	TC358768_DCS_write_1A_1P(0x94,0x40);
	TC358768_DCS_write_1A_1P(0x95,0x02);
	TC358768_DCS_write_1A_1P(0x96,0x7A);
	TC358768_DCS_write_1A_1P(0x97,0x02);
	TC358768_DCS_write_1A_1P(0x98,0xB5);
	TC358768_DCS_write_1A_1P(0x99,0x02);
	TC358768_DCS_write_1A_1P(0x9A,0xD9);
	TC358768_DCS_write_1A_1P(0x9B,0x03);
	TC358768_DCS_write_1A_1P(0x9C,0x17);
	TC358768_DCS_write_1A_1P(0x9D,0x03);
	TC358768_DCS_write_1A_1P(0x9E,0x34);
	TC358768_DCS_write_1A_1P(0x9F,0x03);
	TC358768_DCS_write_1A_1P(0xA0,0x5C);
	TC358768_DCS_write_1A_1P(0xA2,0x03);
	TC358768_DCS_write_1A_1P(0xA3,0x67);
	TC358768_DCS_write_1A_1P(0xA4,0x03);
	TC358768_DCS_write_1A_1P(0xA5,0x76);
	TC358768_DCS_write_1A_1P(0xA6,0x03);
	TC358768_DCS_write_1A_1P(0xA7,0x87);
	TC358768_DCS_write_1A_1P(0xA9,0x03);
	TC358768_DCS_write_1A_1P(0xAA,0x95);
	TC358768_DCS_write_1A_1P(0xAB,0x03);
	TC358768_DCS_write_1A_1P(0xAC,0xA2);
	TC358768_DCS_write_1A_1P(0xAD,0x03);
	TC358768_DCS_write_1A_1P(0xAE,0xB6);
	TC358768_DCS_write_1A_1P(0xAF,0x03);
	TC358768_DCS_write_1A_1P(0xB0,0xCD);
	TC358768_DCS_write_1A_1P(0xB1,0x03);
	TC358768_DCS_write_1A_1P(0xB2,0xD4);
		 
		 
		 
	// R- 
	TC358768_DCS_write_1A_1P(0xB3,0x00);
	TC358768_DCS_write_1A_1P(0xB4,0x8D);
	TC358768_DCS_write_1A_1P(0xB5,0x00);
	TC358768_DCS_write_1A_1P(0xB6,0x94);
	TC358768_DCS_write_1A_1P(0xB7,0x00);
	TC358768_DCS_write_1A_1P(0xB8,0xa2);
	TC358768_DCS_write_1A_1P(0xB9,0x00);
	TC358768_DCS_write_1A_1P(0xBA,0xac);
	TC358768_DCS_write_1A_1P(0xBB,0x00);
	TC358768_DCS_write_1A_1P(0xBC,0xb8);
	TC358768_DCS_write_1A_1P(0xBD,0x00);
	TC358768_DCS_write_1A_1P(0xBE,0xc6);
	TC358768_DCS_write_1A_1P(0xBF,0x00);
	TC358768_DCS_write_1A_1P(0xC0,0xd1);
	TC358768_DCS_write_1A_1P(0xC1,0x00);
	TC358768_DCS_write_1A_1P(0xC2,0xdd);
	TC358768_DCS_write_1A_1P(0xC3,0x00);
	TC358768_DCS_write_1A_1P(0xC4,0xea);
	TC358768_DCS_write_1A_1P(0xC5,0x01);
	TC358768_DCS_write_1A_1P(0xC6,0x16);
	TC358768_DCS_write_1A_1P(0xC7,0x01);
	TC358768_DCS_write_1A_1P(0xC8,0x3a);
	TC358768_DCS_write_1A_1P(0xC9,0x01);
	TC358768_DCS_write_1A_1P(0xCA,0x78);
	TC358768_DCS_write_1A_1P(0xCB,0x01);
	TC358768_DCS_write_1A_1P(0xCC,0xab);
	TC358768_DCS_write_1A_1P(0xCD,0x01);
	TC358768_DCS_write_1A_1P(0xCE,0xFe);
	TC358768_DCS_write_1A_1P(0xCF,0x02);
	TC358768_DCS_write_1A_1P(0xD0,0x3f);
	TC358768_DCS_write_1A_1P(0xD1,0x02);
	TC358768_DCS_write_1A_1P(0xD2,0x40);
	TC358768_DCS_write_1A_1P(0xD3,0x02);
	TC358768_DCS_write_1A_1P(0xD4,0x73);
	TC358768_DCS_write_1A_1P(0xD5,0x02);
	TC358768_DCS_write_1A_1P(0xD6,0xb5);
	TC358768_DCS_write_1A_1P(0xD7,0x02);
	TC358768_DCS_write_1A_1P(0xD8,0xd9);
	TC358768_DCS_write_1A_1P(0xD9,0x03);
	TC358768_DCS_write_1A_1P(0xDA,0x17);
	TC358768_DCS_write_1A_1P(0xDB,0x03);
	TC358768_DCS_write_1A_1P(0xDC,0x34);
	TC358768_DCS_write_1A_1P(0xDD,0x03);
	TC358768_DCS_write_1A_1P(0xDE,0x5c);
	TC358768_DCS_write_1A_1P(0xDF,0x03);
	TC358768_DCS_write_1A_1P(0xE0,0x67);
	TC358768_DCS_write_1A_1P(0xE1,0x03);
	TC358768_DCS_write_1A_1P(0xE2,0x76);
	TC358768_DCS_write_1A_1P(0xE3,0x03);
	TC358768_DCS_write_1A_1P(0xE4,0x87);
	TC358768_DCS_write_1A_1P(0xE5,0x03);
	TC358768_DCS_write_1A_1P(0xE6,0x95);
	TC358768_DCS_write_1A_1P(0xE7,0x03);
	TC358768_DCS_write_1A_1P(0xE8,0xa2);
	TC358768_DCS_write_1A_1P(0xE9,0x03);
	TC358768_DCS_write_1A_1P(0xEA,0xB6);
	TC358768_DCS_write_1A_1P(0xEB,0x03);
	TC358768_DCS_write_1A_1P(0xEC,0xCd);
	TC358768_DCS_write_1A_1P(0xED,0x03);
	TC358768_DCS_write_1A_1P(0xEE,0xD4);
		 
	// G+ 
	TC358768_DCS_write_1A_1P(0xEF,0x00);
	TC358768_DCS_write_1A_1P(0xF0,0x42);
	TC358768_DCS_write_1A_1P(0xF1,0x00);
	TC358768_DCS_write_1A_1P(0xF2,0x4B);
	TC358768_DCS_write_1A_1P(0xF3,0x00);
	TC358768_DCS_write_1A_1P(0xF4,0x5E);
	TC358768_DCS_write_1A_1P(0xF5,0x00);
	TC358768_DCS_write_1A_1P(0xF6,0x6D);
	TC358768_DCS_write_1A_1P(0xF7,0x00);
	TC358768_DCS_write_1A_1P(0xF8,0x7C);
	TC358768_DCS_write_1A_1P(0xF9,0x00);
	TC358768_DCS_write_1A_1P(0xFA,0x8A);
		 
	TC358768_DCS_write_1A_1P(0xFF,0x21); // Page 0,power-related setting	
	
	TC358768_DCS_write_1A_1P(0xFB,0x01);
	MDELAY(1);
	TC358768_DCS_write_1A_1P(0x00,0x00);
	TC358768_DCS_write_1A_1P(0x01,0x99);
	TC358768_DCS_write_1A_1P(0x02,0x00);
	TC358768_DCS_write_1A_1P(0x03,0xA8);
	TC358768_DCS_write_1A_1P(0x04,0x00);
	TC358768_DCS_write_1A_1P(0x05,0xB3);
	TC358768_DCS_write_1A_1P(0x06,0x00);
	TC358768_DCS_write_1A_1P(0x07,0xE0);
	TC358768_DCS_write_1A_1P(0x08,0x01);
	TC358768_DCS_write_1A_1P(0x09,0x05);
	TC358768_DCS_write_1A_1P(0x0A,0x01);
	TC358768_DCS_write_1A_1P(0x0B,0x48);
	TC358768_DCS_write_1A_1P(0x0C,0x01);
	TC358768_DCS_write_1A_1P(0x0D,0x7C);
	TC358768_DCS_write_1A_1P(0x0E,0x01);
	TC358768_DCS_write_1A_1P(0x0F,0xD3);
	TC358768_DCS_write_1A_1P(0x10,0x02);
	TC358768_DCS_write_1A_1P(0x11,0x1A);
	TC358768_DCS_write_1A_1P(0x12,0x02);
	TC358768_DCS_write_1A_1P(0x13,0x1B);
	TC358768_DCS_write_1A_1P(0x14,0x02);
	TC358768_DCS_write_1A_1P(0x15,0x59);
	TC358768_DCS_write_1A_1P(0x16,0x02);
	TC358768_DCS_write_1A_1P(0x17,0x98);
	TC358768_DCS_write_1A_1P(0x18,0x02);
	TC358768_DCS_write_1A_1P(0x19,0xC0);
	TC358768_DCS_write_1A_1P(0x1A,0x03);
	TC358768_DCS_write_1A_1P(0x1B,0x03);
	TC358768_DCS_write_1A_1P(0x1C,0x03);
	TC358768_DCS_write_1A_1P(0x1D,0x23);
	TC358768_DCS_write_1A_1P(0x1E,0x03);
	TC358768_DCS_write_1A_1P(0x1F,0x4E);
	TC358768_DCS_write_1A_1P(0x20,0x03);
	TC358768_DCS_write_1A_1P(0x21,0x5A);
	TC358768_DCS_write_1A_1P(0x22,0x03);
	TC358768_DCS_write_1A_1P(0x23,0x6A);
	TC358768_DCS_write_1A_1P(0x24,0x03);
	TC358768_DCS_write_1A_1P(0x25,0x7D);
	TC358768_DCS_write_1A_1P(0x26,0x03);
	TC358768_DCS_write_1A_1P(0x27,0x8C);
	TC358768_DCS_write_1A_1P(0x28,0x03);
	TC358768_DCS_write_1A_1P(0x29,0x9A);
	TC358768_DCS_write_1A_1P(0x2A,0x03);
	TC358768_DCS_write_1A_1P(0x2B,0xB0);
	TC358768_DCS_write_1A_1P(0x2D,0x03);
	TC358768_DCS_write_1A_1P(0x2F,0xC8);
	TC358768_DCS_write_1A_1P(0x30,0x03);
	TC358768_DCS_write_1A_1P(0x31,0xD4);
	// G- 
	TC358768_DCS_write_1A_1P(0x32,0x00);
	TC358768_DCS_write_1A_1P(0x33,0x42);
	TC358768_DCS_write_1A_1P(0x34,0x00);
	TC358768_DCS_write_1A_1P(0x35,0x4B);
	TC358768_DCS_write_1A_1P(0x36,0x00);
	TC358768_DCS_write_1A_1P(0x37,0x5E);
	TC358768_DCS_write_1A_1P(0x38,0x00);
	TC358768_DCS_write_1A_1P(0x39,0x6D);
	TC358768_DCS_write_1A_1P(0x3A,0x00);
	TC358768_DCS_write_1A_1P(0x3B,0x7C);
	TC358768_DCS_write_1A_1P(0x3D,0x00);
	TC358768_DCS_write_1A_1P(0x3F,0x8A);
	TC358768_DCS_write_1A_1P(0x40,0x00);
	TC358768_DCS_write_1A_1P(0x41,0x99);
	TC358768_DCS_write_1A_1P(0x42,0x00);
	TC358768_DCS_write_1A_1P(0x43,0xA8);
	TC358768_DCS_write_1A_1P(0x44,0x00);
	TC358768_DCS_write_1A_1P(0x45,0xB3);
	TC358768_DCS_write_1A_1P(0x46,0x00);
	TC358768_DCS_write_1A_1P(0x47,0xE0);
	TC358768_DCS_write_1A_1P(0x48,0x01);
	TC358768_DCS_write_1A_1P(0x49,0x05);
	TC358768_DCS_write_1A_1P(0x4A,0x01);
	TC358768_DCS_write_1A_1P(0x4B,0x48);
	TC358768_DCS_write_1A_1P(0x4C,0x01);
	TC358768_DCS_write_1A_1P(0x4D,0x7C);
	TC358768_DCS_write_1A_1P(0x4E,0x01);
	TC358768_DCS_write_1A_1P(0x4F,0xD3);
	TC358768_DCS_write_1A_1P(0x50,0x02);
	TC358768_DCS_write_1A_1P(0x51,0x1A);
	TC358768_DCS_write_1A_1P(0x52,0x02);
	TC358768_DCS_write_1A_1P(0x53,0x1B);
	TC358768_DCS_write_1A_1P(0x54,0x02);
	TC358768_DCS_write_1A_1P(0x55,0x59);
	TC358768_DCS_write_1A_1P(0x56,0x02);
	TC358768_DCS_write_1A_1P(0x58,0x98);
	TC358768_DCS_write_1A_1P(0x59,0x02);
	TC358768_DCS_write_1A_1P(0x5A,0xC0);
	TC358768_DCS_write_1A_1P(0x5B,0x03);
	TC358768_DCS_write_1A_1P(0x5C,0x03);
	TC358768_DCS_write_1A_1P(0x5D,0x03);
	TC358768_DCS_write_1A_1P(0x5E,0x23);
	TC358768_DCS_write_1A_1P(0x5F,0x03);
	TC358768_DCS_write_1A_1P(0x60,0x4E);
	TC358768_DCS_write_1A_1P(0x61,0x03);
	TC358768_DCS_write_1A_1P(0x62,0x5A);
	TC358768_DCS_write_1A_1P(0x63,0x03);
	TC358768_DCS_write_1A_1P(0x64,0x6A);
	TC358768_DCS_write_1A_1P(0x65,0x03);
	TC358768_DCS_write_1A_1P(0x66,0x7D);
	TC358768_DCS_write_1A_1P(0x67,0x03);
	TC358768_DCS_write_1A_1P(0x68,0x8C);
	TC358768_DCS_write_1A_1P(0x69,0x03);
	TC358768_DCS_write_1A_1P(0x6A,0x9A);
	TC358768_DCS_write_1A_1P(0x6B,0x03);
	TC358768_DCS_write_1A_1P(0x6C,0xB0);
	TC358768_DCS_write_1A_1P(0x6D,0x03);
	TC358768_DCS_write_1A_1P(0x6E,0xC8);
	TC358768_DCS_write_1A_1P(0x6F,0x03);
	TC358768_DCS_write_1A_1P(0x70,0xD4);
		 
		 
	// B+ 
	TC358768_DCS_write_1A_1P(0x71,0x00);
	TC358768_DCS_write_1A_1P(0x72,0x00);
	TC358768_DCS_write_1A_1P(0x73,0x00);
	TC358768_DCS_write_1A_1P(0x74,0x08);
	TC358768_DCS_write_1A_1P(0x75,0x00);
	TC358768_DCS_write_1A_1P(0x76,0x16);
	TC358768_DCS_write_1A_1P(0x77,0x00);
	TC358768_DCS_write_1A_1P(0x78,0x24);
	TC358768_DCS_write_1A_1P(0x79,0x00);
	TC358768_DCS_write_1A_1P(0x7A,0x32);
	TC358768_DCS_write_1A_1P(0x7B,0x00);
	TC358768_DCS_write_1A_1P(0x7C,0x42);
	TC358768_DCS_write_1A_1P(0x7D,0x00);
	TC358768_DCS_write_1A_1P(0x7E,0x4F);
	TC358768_DCS_write_1A_1P(0x7F,0x00);
	TC358768_DCS_write_1A_1P(0x80,0x5D);
	TC358768_DCS_write_1A_1P(0x81,0x00);
	TC358768_DCS_write_1A_1P(0x82,0x6C);
	TC358768_DCS_write_1A_1P(0x83,0x00);
	TC358768_DCS_write_1A_1P(0x84,0x9F);
	TC358768_DCS_write_1A_1P(0x85,0x00);
	TC358768_DCS_write_1A_1P(0x86,0xC9);
	TC358768_DCS_write_1A_1P(0x87,0x01);
	TC358768_DCS_write_1A_1P(0x88,0x11);
	TC358768_DCS_write_1A_1P(0x89,0x01);
	TC358768_DCS_write_1A_1P(0x8A,0x4C);
	TC358768_DCS_write_1A_1P(0x8B,0x01);
	TC358768_DCS_write_1A_1P(0x8C,0xAC);
	TC358768_DCS_write_1A_1P(0x8D,0x01);
	TC358768_DCS_write_1A_1P(0x8E,0xF8);
	TC358768_DCS_write_1A_1P(0x8F,0x01);
	TC358768_DCS_write_1A_1P(0x90,0xF9);
	TC358768_DCS_write_1A_1P(0x91,0x02);
	TC358768_DCS_write_1A_1P(0x92,0x3C);
	TC358768_DCS_write_1A_1P(0x93,0x02);
	TC358768_DCS_write_1A_1P(0x94,0x80);
	TC358768_DCS_write_1A_1P(0x95,0x02);
	TC358768_DCS_write_1A_1P(0x96,0xAA);
	TC358768_DCS_write_1A_1P(0x97,0x02);
	TC358768_DCS_write_1A_1P(0x98,0xF2);
	TC358768_DCS_write_1A_1P(0x99,0x03);
	TC358768_DCS_write_1A_1P(0x9A,0x14);
	TC358768_DCS_write_1A_1P(0x9B,0x03);
	TC358768_DCS_write_1A_1P(0x9C,0x42);
	TC358768_DCS_write_1A_1P(0x9D,0x03);
	TC358768_DCS_write_1A_1P(0x9E,0x4F);
	TC358768_DCS_write_1A_1P(0x9F,0x03);
	TC358768_DCS_write_1A_1P(0xA0,0x60);
	TC358768_DCS_write_1A_1P(0xA2,0x03);
	TC358768_DCS_write_1A_1P(0xA3,0x74);
	TC358768_DCS_write_1A_1P(0xA4,0x03);
	TC358768_DCS_write_1A_1P(0xA5,0x84);
	TC358768_DCS_write_1A_1P(0xA6,0x03);
	TC358768_DCS_write_1A_1P(0xA7,0x93);
	TC358768_DCS_write_1A_1P(0xA9,0x03);
	TC358768_DCS_write_1A_1P(0xAA,0xAB);
	TC358768_DCS_write_1A_1P(0xAB,0x03);
	TC358768_DCS_write_1A_1P(0xAC,0xC5);
	TC358768_DCS_write_1A_1P(0xAD,0x03);
	TC358768_DCS_write_1A_1P(0xAE,0xD4);
	// B- 
	TC358768_DCS_write_1A_1P(0xAF,0x00);
	TC358768_DCS_write_1A_1P(0xB0,0x00);
	TC358768_DCS_write_1A_1P(0xB1,0x00);
	TC358768_DCS_write_1A_1P(0xB2,0x08);
	TC358768_DCS_write_1A_1P(0xB3,0x00);
	TC358768_DCS_write_1A_1P(0xB4,0x16);
	TC358768_DCS_write_1A_1P(0xB5,0x00);
	TC358768_DCS_write_1A_1P(0xB6,0x24);
	TC358768_DCS_write_1A_1P(0xB7,0x00);
	TC358768_DCS_write_1A_1P(0xB8,0x32);
	TC358768_DCS_write_1A_1P(0xB9,0x00);
	TC358768_DCS_write_1A_1P(0xBA,0x42);
	TC358768_DCS_write_1A_1P(0xBB,0x00);
	TC358768_DCS_write_1A_1P(0xBC,0x4F);
	TC358768_DCS_write_1A_1P(0xBD,0x00);
	TC358768_DCS_write_1A_1P(0xBE,0x5D);
	TC358768_DCS_write_1A_1P(0xBF,0x00);
	TC358768_DCS_write_1A_1P(0xC0,0x6C);
	TC358768_DCS_write_1A_1P(0xC1,0x00);
	TC358768_DCS_write_1A_1P(0xC2,0x9F);
	TC358768_DCS_write_1A_1P(0xC3,0x00);
	TC358768_DCS_write_1A_1P(0xC4,0xC9);
	TC358768_DCS_write_1A_1P(0xC5,0x01);
	TC358768_DCS_write_1A_1P(0xC6,0x11);
	TC358768_DCS_write_1A_1P(0xC7,0x01);
	TC358768_DCS_write_1A_1P(0xC8,0x4C);
	TC358768_DCS_write_1A_1P(0xC9,0x01);
	TC358768_DCS_write_1A_1P(0xCA,0xAC);
	TC358768_DCS_write_1A_1P(0xCB,0x01);
	TC358768_DCS_write_1A_1P(0xCC,0xF8);
	TC358768_DCS_write_1A_1P(0xCD,0x01);
	TC358768_DCS_write_1A_1P(0xCE,0xF9);
	TC358768_DCS_write_1A_1P(0xCF,0x02);
	TC358768_DCS_write_1A_1P(0xD0,0x3C);
	TC358768_DCS_write_1A_1P(0xD1,0x02);
	TC358768_DCS_write_1A_1P(0xD2,0x80);
	TC358768_DCS_write_1A_1P(0xD3,0x02);
	TC358768_DCS_write_1A_1P(0xD4,0xAA);
	TC358768_DCS_write_1A_1P(0xD5,0x02);
	TC358768_DCS_write_1A_1P(0xD6,0xF2);
	TC358768_DCS_write_1A_1P(0xD7,0x03);
	TC358768_DCS_write_1A_1P(0xD8,0x14);
	TC358768_DCS_write_1A_1P(0xD9,0x03);
	TC358768_DCS_write_1A_1P(0xDA,0x42);
	TC358768_DCS_write_1A_1P(0xDB,0x03);
	TC358768_DCS_write_1A_1P(0xDC,0x4F);
	TC358768_DCS_write_1A_1P(0xDD,0x03);
	TC358768_DCS_write_1A_1P(0xDE,0x60);
	TC358768_DCS_write_1A_1P(0xDF,0x03);
	TC358768_DCS_write_1A_1P(0xE0,0x74);
	TC358768_DCS_write_1A_1P(0xE1,0x03);
	TC358768_DCS_write_1A_1P(0xE2,0x84);
	TC358768_DCS_write_1A_1P(0xE3,0x03);
	TC358768_DCS_write_1A_1P(0xE4,0x93);
	TC358768_DCS_write_1A_1P(0xE5,0x03);
	TC358768_DCS_write_1A_1P(0xE6,0xAB);
	TC358768_DCS_write_1A_1P(0xE7,0x03);
	TC358768_DCS_write_1A_1P(0xE8,0xC5);
	TC358768_DCS_write_1A_1P(0xE9,0x03);
	TC358768_DCS_write_1A_1P(0xEA,0xD4);
	TC358768_DCS_write_1A_1P(0xFF, 0x10);

}

static void lcm_setgamma_warm(void)
{
	TC358768_DCS_write_1A_1P(0xFF,0x20);
	TC358768_DCS_write_1A_1P(0xFB,0x01);
	MDELAY(1);		  
	// R+
	TC358768_DCS_write_1A_1P(0x75,0x00);
	TC358768_DCS_write_1A_1P(0x76,0x8D);
	TC358768_DCS_write_1A_1P(0x77,0x00);
	TC358768_DCS_write_1A_1P(0x78,0x94);
	TC358768_DCS_write_1A_1P(0x79,0x00);
	TC358768_DCS_write_1A_1P(0x7A,0xA2);
	TC358768_DCS_write_1A_1P(0x7B,0x00);
	TC358768_DCS_write_1A_1P(0x7C,0xAC);
	TC358768_DCS_write_1A_1P(0x7D,0x00);
	TC358768_DCS_write_1A_1P(0x7E,0xB8);
	TC358768_DCS_write_1A_1P(0x7F,0x00);
	TC358768_DCS_write_1A_1P(0x80,0xC6);
	TC358768_DCS_write_1A_1P(0x81,0x00);
	TC358768_DCS_write_1A_1P(0x82,0xD1);
	TC358768_DCS_write_1A_1P(0x83,0x00);
	TC358768_DCS_write_1A_1P(0x84,0xDD);
	TC358768_DCS_write_1A_1P(0x85,0x00);
	TC358768_DCS_write_1A_1P(0x86,0xEA);
	TC358768_DCS_write_1A_1P(0x87,0x01);
	TC358768_DCS_write_1A_1P(0x88,0x16);
	TC358768_DCS_write_1A_1P(0x89,0x01);
	TC358768_DCS_write_1A_1P(0x8A,0x3A);
	TC358768_DCS_write_1A_1P(0x8B,0x01);
	TC358768_DCS_write_1A_1P(0x8C,0x78);
	TC358768_DCS_write_1A_1P(0x8D,0x01);
	TC358768_DCS_write_1A_1P(0x8E,0xAB);
	TC358768_DCS_write_1A_1P(0x8F,0x01);
	TC358768_DCS_write_1A_1P(0x90,0xFE);
	TC358768_DCS_write_1A_1P(0x91,0x02);
	TC358768_DCS_write_1A_1P(0x92,0x3F);
	TC358768_DCS_write_1A_1P(0x93,0x02);
	TC358768_DCS_write_1A_1P(0x94,0x40);
	TC358768_DCS_write_1A_1P(0x95,0x02);
	TC358768_DCS_write_1A_1P(0x96,0x7A);
	TC358768_DCS_write_1A_1P(0x97,0x02);
	TC358768_DCS_write_1A_1P(0x98,0xB5);
	TC358768_DCS_write_1A_1P(0x99,0x02);
	TC358768_DCS_write_1A_1P(0x9A,0xD9);
	TC358768_DCS_write_1A_1P(0x9B,0x03);
	TC358768_DCS_write_1A_1P(0x9C,0x17);
	TC358768_DCS_write_1A_1P(0x9D,0x03);
	TC358768_DCS_write_1A_1P(0x9E,0x34);
	TC358768_DCS_write_1A_1P(0x9F,0x03);
	TC358768_DCS_write_1A_1P(0xA0,0x5C);
	TC358768_DCS_write_1A_1P(0xA2,0x03);
	TC358768_DCS_write_1A_1P(0xA3,0x67);
	TC358768_DCS_write_1A_1P(0xA4,0x03);
	TC358768_DCS_write_1A_1P(0xA5,0x76);
	TC358768_DCS_write_1A_1P(0xA6,0x03);
	TC358768_DCS_write_1A_1P(0xA7,0x87);
	TC358768_DCS_write_1A_1P(0xA9,0x03);
	TC358768_DCS_write_1A_1P(0xAA,0x95);
	TC358768_DCS_write_1A_1P(0xAB,0x03);
	TC358768_DCS_write_1A_1P(0xAC,0xA2);
	TC358768_DCS_write_1A_1P(0xAD,0x03);
	TC358768_DCS_write_1A_1P(0xAE,0xB6);
	TC358768_DCS_write_1A_1P(0xAF,0x03);
	TC358768_DCS_write_1A_1P(0xB0,0xCD);
	TC358768_DCS_write_1A_1P(0xB1,0x03);
	TC358768_DCS_write_1A_1P(0xB2,0xD4);
		  
		  
		  
	// R- 
	TC358768_DCS_write_1A_1P(0xB3,0x00);
	TC358768_DCS_write_1A_1P(0xB4,0x8D);
	TC358768_DCS_write_1A_1P(0xB5,0x00);
	TC358768_DCS_write_1A_1P(0xB6,0x94);
	TC358768_DCS_write_1A_1P(0xB7,0x00);
	TC358768_DCS_write_1A_1P(0xB8,0xa2);
	TC358768_DCS_write_1A_1P(0xB9,0x00);
	TC358768_DCS_write_1A_1P(0xBA,0xac);
	TC358768_DCS_write_1A_1P(0xBB,0x00);
	TC358768_DCS_write_1A_1P(0xBC,0xb8);
	TC358768_DCS_write_1A_1P(0xBD,0x00);
	TC358768_DCS_write_1A_1P(0xBE,0xc6);
	TC358768_DCS_write_1A_1P(0xBF,0x00);
	TC358768_DCS_write_1A_1P(0xC0,0xd1);
	TC358768_DCS_write_1A_1P(0xC1,0x00);
	TC358768_DCS_write_1A_1P(0xC2,0xdd);
	TC358768_DCS_write_1A_1P(0xC3,0x00);
	TC358768_DCS_write_1A_1P(0xC4,0xea);
	TC358768_DCS_write_1A_1P(0xC5,0x01);
	TC358768_DCS_write_1A_1P(0xC6,0x16);
	TC358768_DCS_write_1A_1P(0xC7,0x01);
	TC358768_DCS_write_1A_1P(0xC8,0x3a);
	TC358768_DCS_write_1A_1P(0xC9,0x01);
	TC358768_DCS_write_1A_1P(0xCA,0x78);
	TC358768_DCS_write_1A_1P(0xCB,0x01);
	TC358768_DCS_write_1A_1P(0xCC,0xab);
	TC358768_DCS_write_1A_1P(0xCD,0x01);
	TC358768_DCS_write_1A_1P(0xCE,0xFe);
	TC358768_DCS_write_1A_1P(0xCF,0x02);
	TC358768_DCS_write_1A_1P(0xD0,0x3f);
	TC358768_DCS_write_1A_1P(0xD1,0x02);
	TC358768_DCS_write_1A_1P(0xD2,0x40);
	TC358768_DCS_write_1A_1P(0xD3,0x02);
	TC358768_DCS_write_1A_1P(0xD4,0x73);
	TC358768_DCS_write_1A_1P(0xD5,0x02);
	TC358768_DCS_write_1A_1P(0xD6,0xb5);
	TC358768_DCS_write_1A_1P(0xD7,0x02);
	TC358768_DCS_write_1A_1P(0xD8,0xd9);
	TC358768_DCS_write_1A_1P(0xD9,0x03);
	TC358768_DCS_write_1A_1P(0xDA,0x17);
	TC358768_DCS_write_1A_1P(0xDB,0x03);
	TC358768_DCS_write_1A_1P(0xDC,0x34);
	TC358768_DCS_write_1A_1P(0xDD,0x03);
	TC358768_DCS_write_1A_1P(0xDE,0x5c);
	TC358768_DCS_write_1A_1P(0xDF,0x03);
	TC358768_DCS_write_1A_1P(0xE0,0x67);
	TC358768_DCS_write_1A_1P(0xE1,0x03);
	TC358768_DCS_write_1A_1P(0xE2,0x76);
	TC358768_DCS_write_1A_1P(0xE3,0x03);
	TC358768_DCS_write_1A_1P(0xE4,0x87);
	TC358768_DCS_write_1A_1P(0xE5,0x03);
	TC358768_DCS_write_1A_1P(0xE6,0x95);
	TC358768_DCS_write_1A_1P(0xE7,0x03);
	TC358768_DCS_write_1A_1P(0xE8,0xa2);
	TC358768_DCS_write_1A_1P(0xE9,0x03);
	TC358768_DCS_write_1A_1P(0xEA,0xB6);
	TC358768_DCS_write_1A_1P(0xEB,0x03);
	TC358768_DCS_write_1A_1P(0xEC,0xCd);
	TC358768_DCS_write_1A_1P(0xED,0x03);
	TC358768_DCS_write_1A_1P(0xEE,0xD4);
		  
	// G+ 
	TC358768_DCS_write_1A_1P(0xEF,0x00);
	TC358768_DCS_write_1A_1P(0xF0,0x42);
	TC358768_DCS_write_1A_1P(0xF1,0x00);
	TC358768_DCS_write_1A_1P(0xF2,0x4B);
	TC358768_DCS_write_1A_1P(0xF3,0x00);
	TC358768_DCS_write_1A_1P(0xF4,0x5E);
	TC358768_DCS_write_1A_1P(0xF5,0x00);
	TC358768_DCS_write_1A_1P(0xF6,0x6D);
	TC358768_DCS_write_1A_1P(0xF7,0x00);
	TC358768_DCS_write_1A_1P(0xF8,0x7C);
	TC358768_DCS_write_1A_1P(0xF9,0x00);
	TC358768_DCS_write_1A_1P(0xFA,0x8A);
		  
	TC358768_DCS_write_1A_1P(0xFF,0x21); // Page 0,power-related setting	
	MDELAY(1);
	TC358768_DCS_write_1A_1P(0x00,0x00);
	TC358768_DCS_write_1A_1P(0x01,0x99);
	TC358768_DCS_write_1A_1P(0x02,0x00);
	TC358768_DCS_write_1A_1P(0x03,0xA8);
	TC358768_DCS_write_1A_1P(0x04,0x00);
	TC358768_DCS_write_1A_1P(0x05,0xB3);
	TC358768_DCS_write_1A_1P(0x06,0x00);
	TC358768_DCS_write_1A_1P(0x07,0xE0);
	TC358768_DCS_write_1A_1P(0x08,0x01);
	TC358768_DCS_write_1A_1P(0x09,0x05);
	TC358768_DCS_write_1A_1P(0x0A,0x01);
	TC358768_DCS_write_1A_1P(0x0B,0x48);
	TC358768_DCS_write_1A_1P(0x0C,0x01);
	TC358768_DCS_write_1A_1P(0x0D,0x7C);
	TC358768_DCS_write_1A_1P(0x0E,0x01);
	TC358768_DCS_write_1A_1P(0x0F,0xD3);
	TC358768_DCS_write_1A_1P(0x10,0x02);
	TC358768_DCS_write_1A_1P(0x11,0x1A);
	TC358768_DCS_write_1A_1P(0x12,0x02);
	TC358768_DCS_write_1A_1P(0x13,0x1B);
	TC358768_DCS_write_1A_1P(0x14,0x02);
	TC358768_DCS_write_1A_1P(0x15,0x59);
	TC358768_DCS_write_1A_1P(0x16,0x02);
	TC358768_DCS_write_1A_1P(0x17,0x98);
	TC358768_DCS_write_1A_1P(0x18,0x02);
	TC358768_DCS_write_1A_1P(0x19,0xC0);
	TC358768_DCS_write_1A_1P(0x1A,0x03);
	TC358768_DCS_write_1A_1P(0x1B,0x03);
	TC358768_DCS_write_1A_1P(0x1C,0x03);
	TC358768_DCS_write_1A_1P(0x1D,0x23);
	TC358768_DCS_write_1A_1P(0x1E,0x03);
	TC358768_DCS_write_1A_1P(0x1F,0x4E);
	TC358768_DCS_write_1A_1P(0x20,0x03);
	TC358768_DCS_write_1A_1P(0x21,0x5A);
	TC358768_DCS_write_1A_1P(0x22,0x03);
	TC358768_DCS_write_1A_1P(0x23,0x6A);
	TC358768_DCS_write_1A_1P(0x24,0x03);
	TC358768_DCS_write_1A_1P(0x25,0x7D);
	TC358768_DCS_write_1A_1P(0x26,0x03);
	TC358768_DCS_write_1A_1P(0x27,0x8C);
	TC358768_DCS_write_1A_1P(0x28,0x03);
	TC358768_DCS_write_1A_1P(0x29,0x9A);
	TC358768_DCS_write_1A_1P(0x2A,0x03);
	TC358768_DCS_write_1A_1P(0x2B,0xB0);
	TC358768_DCS_write_1A_1P(0x2D,0x03);
	TC358768_DCS_write_1A_1P(0x2F,0xC8);
	TC358768_DCS_write_1A_1P(0x30,0x03);
	TC358768_DCS_write_1A_1P(0x31,0xD4);
	// G- 
	TC358768_DCS_write_1A_1P(0x32,0x00);
	TC358768_DCS_write_1A_1P(0x33,0x42);
	TC358768_DCS_write_1A_1P(0x34,0x00);
	TC358768_DCS_write_1A_1P(0x35,0x4B);
	TC358768_DCS_write_1A_1P(0x36,0x00);
	TC358768_DCS_write_1A_1P(0x37,0x5E);
	TC358768_DCS_write_1A_1P(0x38,0x00);
	TC358768_DCS_write_1A_1P(0x39,0x6D);
	TC358768_DCS_write_1A_1P(0x3A,0x00);
	TC358768_DCS_write_1A_1P(0x3B,0x7C);
	TC358768_DCS_write_1A_1P(0x3D,0x00);
	TC358768_DCS_write_1A_1P(0x3F,0x8A);
	TC358768_DCS_write_1A_1P(0x40,0x00);
	TC358768_DCS_write_1A_1P(0x41,0x99);
	TC358768_DCS_write_1A_1P(0x42,0x00);
	TC358768_DCS_write_1A_1P(0x43,0xA8);
	TC358768_DCS_write_1A_1P(0x44,0x00);
	TC358768_DCS_write_1A_1P(0x45,0xB3);
	TC358768_DCS_write_1A_1P(0x46,0x00);
	TC358768_DCS_write_1A_1P(0x47,0xE0);
	TC358768_DCS_write_1A_1P(0x48,0x01);
	TC358768_DCS_write_1A_1P(0x49,0x05);
	TC358768_DCS_write_1A_1P(0x4A,0x01);
	TC358768_DCS_write_1A_1P(0x4B,0x48);
	TC358768_DCS_write_1A_1P(0x4C,0x01);
	TC358768_DCS_write_1A_1P(0x4D,0x7C);
	TC358768_DCS_write_1A_1P(0x4E,0x01);
	TC358768_DCS_write_1A_1P(0x4F,0xD3);
	TC358768_DCS_write_1A_1P(0x50,0x02);
	TC358768_DCS_write_1A_1P(0x51,0x1A);
	TC358768_DCS_write_1A_1P(0x52,0x02);
	TC358768_DCS_write_1A_1P(0x53,0x1B);
	TC358768_DCS_write_1A_1P(0x54,0x02);
	TC358768_DCS_write_1A_1P(0x55,0x59);
	TC358768_DCS_write_1A_1P(0x56,0x02);
	TC358768_DCS_write_1A_1P(0x58,0x98);
	TC358768_DCS_write_1A_1P(0x59,0x02);
	TC358768_DCS_write_1A_1P(0x5A,0xC0);
	TC358768_DCS_write_1A_1P(0x5B,0x03);
	TC358768_DCS_write_1A_1P(0x5C,0x03);
	TC358768_DCS_write_1A_1P(0x5D,0x03);
	TC358768_DCS_write_1A_1P(0x5E,0x23);
	TC358768_DCS_write_1A_1P(0x5F,0x03);
	TC358768_DCS_write_1A_1P(0x60,0x4E);
	TC358768_DCS_write_1A_1P(0x61,0x03);
	TC358768_DCS_write_1A_1P(0x62,0x5A);
	TC358768_DCS_write_1A_1P(0x63,0x03);
	TC358768_DCS_write_1A_1P(0x64,0x6A);
	TC358768_DCS_write_1A_1P(0x65,0x03);
	TC358768_DCS_write_1A_1P(0x66,0x7D);
	TC358768_DCS_write_1A_1P(0x67,0x03);
	TC358768_DCS_write_1A_1P(0x68,0x8C);
	TC358768_DCS_write_1A_1P(0x69,0x03);
	TC358768_DCS_write_1A_1P(0x6A,0x9A);
	TC358768_DCS_write_1A_1P(0x6B,0x03);
	TC358768_DCS_write_1A_1P(0x6C,0xB0);
	TC358768_DCS_write_1A_1P(0x6D,0x03);
	TC358768_DCS_write_1A_1P(0x6E,0xC8);
	TC358768_DCS_write_1A_1P(0x6F,0x03);
	TC358768_DCS_write_1A_1P(0x70,0xD4);
		  
		  
	// B+ 
	TC358768_DCS_write_1A_1P(0x71,0x00);
	TC358768_DCS_write_1A_1P(0x72,0x8D);
	TC358768_DCS_write_1A_1P(0x73,0x00);
	TC358768_DCS_write_1A_1P(0x74,0x94);
	TC358768_DCS_write_1A_1P(0x75,0x00);
	TC358768_DCS_write_1A_1P(0x76,0xA2);
	TC358768_DCS_write_1A_1P(0x77,0x00);
	TC358768_DCS_write_1A_1P(0x78,0xAC);
	TC358768_DCS_write_1A_1P(0x79,0x00);
	TC358768_DCS_write_1A_1P(0x7A,0xB8);
	TC358768_DCS_write_1A_1P(0x7B,0x00);
	TC358768_DCS_write_1A_1P(0x7C,0xC6);
	TC358768_DCS_write_1A_1P(0x7D,0x00);
	TC358768_DCS_write_1A_1P(0x7E,0xD1);
	TC358768_DCS_write_1A_1P(0x7F,0x00);
	TC358768_DCS_write_1A_1P(0x80,0xDD);
	TC358768_DCS_write_1A_1P(0x81,0x00);
	TC358768_DCS_write_1A_1P(0x82,0xEA);
	TC358768_DCS_write_1A_1P(0x83,0x01);
	TC358768_DCS_write_1A_1P(0x84,0x16);
	TC358768_DCS_write_1A_1P(0x85,0x01);
	TC358768_DCS_write_1A_1P(0x86,0x3A);
	TC358768_DCS_write_1A_1P(0x87,0x01);
	TC358768_DCS_write_1A_1P(0x88,0x78);
	TC358768_DCS_write_1A_1P(0x89,0x01);
	TC358768_DCS_write_1A_1P(0x8A,0xAB);
	TC358768_DCS_write_1A_1P(0x8B,0x01);
	TC358768_DCS_write_1A_1P(0x8C,0xFE);
	TC358768_DCS_write_1A_1P(0x8D,0x02);
	TC358768_DCS_write_1A_1P(0x8E,0x3F);
	TC358768_DCS_write_1A_1P(0x8F,0x02);
	TC358768_DCS_write_1A_1P(0x90,0x40);
	TC358768_DCS_write_1A_1P(0x91,0x02);
	TC358768_DCS_write_1A_1P(0x92,0x7A);
	TC358768_DCS_write_1A_1P(0x93,0x02);
	TC358768_DCS_write_1A_1P(0x94,0xB5);
	TC358768_DCS_write_1A_1P(0x95,0x02);
	TC358768_DCS_write_1A_1P(0x96,0xD9);
	TC358768_DCS_write_1A_1P(0x97,0x03);
	TC358768_DCS_write_1A_1P(0x98,0x17);
	TC358768_DCS_write_1A_1P(0x99,0x03);
	TC358768_DCS_write_1A_1P(0x9A,0x34);
	TC358768_DCS_write_1A_1P(0x9B,0x03);
	TC358768_DCS_write_1A_1P(0x9C,0x5C);
	TC358768_DCS_write_1A_1P(0x9D,0x03);
	TC358768_DCS_write_1A_1P(0x9E,0x67);
	TC358768_DCS_write_1A_1P(0x9F,0x03);
	TC358768_DCS_write_1A_1P(0xA0,0x76);
	TC358768_DCS_write_1A_1P(0xA2,0x03);
	TC358768_DCS_write_1A_1P(0xA3,0x87);
	TC358768_DCS_write_1A_1P(0xA4,0x03);
	TC358768_DCS_write_1A_1P(0xA5,0x95);
	TC358768_DCS_write_1A_1P(0xA6,0x03);
	TC358768_DCS_write_1A_1P(0xA7,0xA2);
	TC358768_DCS_write_1A_1P(0xA9,0x03);
	TC358768_DCS_write_1A_1P(0xAA,0xB6);
	TC358768_DCS_write_1A_1P(0xAB,0x03);
	TC358768_DCS_write_1A_1P(0xAC,0xCD);
	TC358768_DCS_write_1A_1P(0xAD,0x03);
	TC358768_DCS_write_1A_1P(0xAE,0xD4);
		  
		  
	// B- 
	TC358768_DCS_write_1A_1P(0xAF,0x00);
	TC358768_DCS_write_1A_1P(0xB0,0x8D);
	TC358768_DCS_write_1A_1P(0xB1,0x00);
	TC358768_DCS_write_1A_1P(0xB2,0x94);
	TC358768_DCS_write_1A_1P(0xB3,0x00);
	TC358768_DCS_write_1A_1P(0xB4,0xA2);
	TC358768_DCS_write_1A_1P(0xB5,0x00);
	TC358768_DCS_write_1A_1P(0xB6,0xAC);
	TC358768_DCS_write_1A_1P(0xB7,0x00);
	TC358768_DCS_write_1A_1P(0xB8,0xB8);
	TC358768_DCS_write_1A_1P(0xB9,0x00);
	TC358768_DCS_write_1A_1P(0xBA,0xC6);
	TC358768_DCS_write_1A_1P(0xBB,0x00);
	TC358768_DCS_write_1A_1P(0xBC,0xD1);
	TC358768_DCS_write_1A_1P(0xBD,0x00);
	TC358768_DCS_write_1A_1P(0xBE,0xDD);
	TC358768_DCS_write_1A_1P(0xBF,0x00);
	TC358768_DCS_write_1A_1P(0xC0,0xEA);
	TC358768_DCS_write_1A_1P(0xC1,0x01);
	TC358768_DCS_write_1A_1P(0xC2,0x16);
	TC358768_DCS_write_1A_1P(0xC3,0x01);
	TC358768_DCS_write_1A_1P(0xC4,0x3A);
	TC358768_DCS_write_1A_1P(0xC5,0x01);
	TC358768_DCS_write_1A_1P(0xC6,0x78);
	TC358768_DCS_write_1A_1P(0xC7,0x01);
	TC358768_DCS_write_1A_1P(0xC8,0xAB);
	TC358768_DCS_write_1A_1P(0xC9,0x01);
	TC358768_DCS_write_1A_1P(0xCA,0xFE);
	TC358768_DCS_write_1A_1P(0xCB,0x02);
	TC358768_DCS_write_1A_1P(0xCC,0x3F);
	TC358768_DCS_write_1A_1P(0xCD,0x02);
	TC358768_DCS_write_1A_1P(0xCE,0x40);
	TC358768_DCS_write_1A_1P(0xCF,0x02);
	TC358768_DCS_write_1A_1P(0xD0,0x7A);
	TC358768_DCS_write_1A_1P(0xD1,0x02);
	TC358768_DCS_write_1A_1P(0xD2,0xB5);
	TC358768_DCS_write_1A_1P(0xD3,0x02);
	TC358768_DCS_write_1A_1P(0xD4,0xD9);
	TC358768_DCS_write_1A_1P(0xD5,0x03);
	TC358768_DCS_write_1A_1P(0xD6,0x17);
	TC358768_DCS_write_1A_1P(0xD7,0x03);
	TC358768_DCS_write_1A_1P(0xD8,0x34);
	TC358768_DCS_write_1A_1P(0xD9,0x03);
	TC358768_DCS_write_1A_1P(0xDA,0x5C);
	TC358768_DCS_write_1A_1P(0xDB,0x03);
	TC358768_DCS_write_1A_1P(0xDC,0x67);
	TC358768_DCS_write_1A_1P(0xDD,0x03);
	TC358768_DCS_write_1A_1P(0xDE,0x76);
	TC358768_DCS_write_1A_1P(0xDF,0x03);
	TC358768_DCS_write_1A_1P(0xE0,0x87);
	TC358768_DCS_write_1A_1P(0xE1,0x03);
	TC358768_DCS_write_1A_1P(0xE2,0x95);
	TC358768_DCS_write_1A_1P(0xE3,0x03);
	TC358768_DCS_write_1A_1P(0xE4,0xA2);
	TC358768_DCS_write_1A_1P(0xE5,0x03);
	TC358768_DCS_write_1A_1P(0xE6,0xB6);
	TC358768_DCS_write_1A_1P(0xE7,0x03);
	TC358768_DCS_write_1A_1P(0xE8,0xCD);
	TC358768_DCS_write_1A_1P(0xE9,0x03);
	TC358768_DCS_write_1A_1P(0xEA,0xD4);
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
	}*/
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
		printf("%s, id = 0x%x 0x%x \n", __func__, id,lcm_id);
    #else
		printk("%s, id = 0x%x 0x%x \n", __func__, id,lcm_id);
    #endif

	if(lcm_id==LCM_ID_NT35595)
		return 1;
	else if(id == LCM_ID_NT35595)
    	return 1;
    else
        return 0;
}
LCM_DRIVER nt35595_fhd_dsi_vdo_lg_lcm_drv = 
{
    .name			= "nt35595_fhd_dsi_vdo_lg",
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
	#endif
    };
