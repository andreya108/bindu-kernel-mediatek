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
#include <linux/kernel.h>//for printk
#endif

#include "lcm_drv.h"



#if 0
#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif
#endif


#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
    #include <platform/mt_pmic.h>
	#undef printk
	#define printk printf
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
	#undef printk
	#define printk printf	
#else
    #include <mach/mt_pm_ldo.h>
	#include <mach/mt_gpio.h>
	#undef printk
	#define printk printk	
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define REGFLAG_DELAY             							0xFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#define LCM_ID_HX8394A                                      0x94

#define LCM_RESET_PIN           (112)
#define LCM_MODE_SEL_PIN        (123)

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

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size) 
static unsigned int lcm_compare_id(void);

//static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)      

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
};

#if 1
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
#if 0
	//set Password
	{0xB9,	3,	{0xFF,0x83,0x94}},
	{REGFLAG_DELAY, 10, {}},

	//set MIPI 4 Lane
	{0xBA, 17, {0x13,0x82,0x00,0x16,0xC6,0x00,0x08,0xFF,0x0F,0x24,0x03,0x21,0x24,0x25,0x20,0x00,0x00}},
	{REGFLAG_DELAY, 10, {}},

	//set Power
	{0xB1, 15, {0x7C,0x00,0x24,0x09,0x01,0x11,0x11,0x36,0x3E,0x26,0x26,0x57,0x0A,0x01,0xE6}},
	{REGFLAG_DELAY, 10, {}},

	//set CYC
	{0xB4, 18, {0x00,0x00,0x00,0x05,0x06,0x41,0x42,0x02,0x41,0x42,0x43,0x47,0x19,0x58,0x60,0x08,0x85,0x10}},
	{REGFLAG_DELAY, 10, {}},

	//set GIP
	{0xD5, 24, {0x4C,0x01,0x07,0x01,0xCD,0x23,0xEF,0x45,0x67,0x89,0xAB,0x11,0x00,0xDC,0x10,0xFE,0x32,0xBA,0x98,0x76,0x54,0x00,0x11,0x40}},
	{REGFLAG_DELAY, 10, {}},

	//set Display related register
	{0xB2, 6, {0x0F,0xC8,0x04,0x04,0x00,0x81}},
	{REGFLAG_DELAY, 10, {}},

	//set Vcom
	{0xB6, 1, {0x25}},
	{REGFLAG_DELAY, 10, {}},

	//set TCOM Option
	{0xC7, 2, {0x00,0x20}},
	{REGFLAG_DELAY, 10, {}},

	//set panel
	{0xCC, 1, {0x09}},
	{REGFLAG_DELAY, 10, {}},

	{0x11,	0,	{}},     //sleep out
	{REGFLAG_DELAY, 200, {}},

	//display on
	{0x29, 0, {}},
	{REGFLAG_DELAY, 50, {}},

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	#endif
	#if 1/////////////////////////////////////////////////////lrzadd061461308at PD1216T
	
	{0xB0,	1,	{0x04}},
	{REGFLAG_DELAY, 10, {}},

	
	{0xB3, 1, {0x02}},
		{REGFLAG_DELAY, 10, {}},


	{0xB6, 04, {0x52,0x62,0x85,0x00}},
	{REGFLAG_DELAY, 10, {}},

	
	{0xB7, 03, {0x00,0x00,0x01}},
	{REGFLAG_DELAY, 10, {}},

	
	{0xC0, 7, {0x43,0xFF,0x04,0x09,0x02,0x07,0x07}},
	{REGFLAG_DELAY, 10, {}},

	{0xC1, 7, {0x50,0x02,0x22,0x00,0x00,0xED,0x11}},
	{REGFLAG_DELAY, 10, {}},

	
	{0xC3, 16, {0x04,0x00,0x05,0x14,0x80,0x00,0x00,0x00,0x00,0x90,0x00,0x00,0x0C,0x00,0x00,0x5C}},
	{REGFLAG_DELAY, 10, {}},

	
	{0xD0, 9, {0x74,0x29,0xDD,0x15,0x09,0x2B,0x00,0xC0,0xCC}},
	{REGFLAG_DELAY, 10, {}},


	{0xD1, 13, {0x4D,0x24,0x34,0x55,0x55,0x77,0x77,0x66,0x31,0x75,0x42,0x86,0x06}},
	{REGFLAG_DELAY, 10, {}},

	{0xD6,	1,	{0xA8}},     
	{REGFLAG_DELAY, 10, {}},


	{0xDE, 3, {0x03,0x45,0x5A}},
	{REGFLAG_DELAY, 10, {}},
	
	{0xC8,	24,	{0x06,0x1D,0x27,0x2A,0x2F,0x35,0x15,0x13,0x10,0x0D,0x09,0x06,0x06,0x1D,0x27,0x2A,0x2F,0x35,0x15,0x13,0x10,0x0D,0x09,0x06}},     //sleep out
	{REGFLAG_DELAY, 10, {}},

	
	{0xCB, 8, {0x10,0x21,0x42,0x7D,0x9F,0xC3,0xD1,0xDF}},
	{REGFLAG_DELAY, 10, {}},
	
	{0xCC,	3,	{0xEC,0xEC,0xFF}},     
	{REGFLAG_DELAY, 10, {}},


	{0xD5, 2, {0x24,0x24}},
	{REGFLAG_DELAY, 50, {}},
	
	{0xB0,	1,	{0x03}},     
	{REGFLAG_DELAY, 10, {}},
	
	{0x11,	0,	{}},     
	{REGFLAG_DELAY, 200, {}},

	{0x29, 0, {}},
	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
	#endif
};
#endif
#if 1
static void init_lcm_registers(void)///lrz0810
{

#if 1// xinli
	unsigned int data_array[40];
	//////////////////////////////////////////////lrzadd0821////for1222
	

   data_array[0] = 0x00043902;                          
    data_array[1] = 0x9483ffB9; 
    dsi_set_cmdq(&data_array, 2, 1); 
    MDELAY(1);
  

       data_array[0] = 0x13ba1500;
       dsi_set_cmdq(data_array, 1, 1);
       MDELAY(1);
    
	data_array[0] = 0x00113902;                          
       data_array[1] = 0x070001B1; 
	data_array[2] = 0x11110187;
	data_array[3] = 0x3F3F302A;
	data_array[4] = 0xE6011247;
	data_array[5] = 0x000000E2;
    dsi_set_cmdq(&data_array, 6, 1); 
    MDELAY(1);
	 
       data_array[0] = 0x00173902;                          
       data_array[1] = 0x320680B4; 
	data_array[2] = 0x15320310; 
	data_array[3] = 0x08103208;                          
       data_array[4] = 0x05430433; 
	data_array[5] = 0x063F0437; 
	data_array[6] = 0x00066161; 
       dsi_set_cmdq(&data_array, 7, 1);
	MDELAY(1);

	data_array[0] = 0x00073902;                          
       data_array[1] = 0x08C800B2; 
	data_array[2] = 0x00220004; 
       dsi_set_cmdq(&data_array, 3, 1);
       MDELAY(1);
	 
	data_array[0] = 0x00213902;                          
  data_array[1] = 0x000000D5; 
	data_array[2] = 0x01000A00;
	data_array[3] = 0x0000CC00;
	data_array[4] = 0x88888800;
	data_array[5] = 0x88888888;
	data_array[6] = 0x01888888;
	data_array[7] = 0x01234567;
	data_array[8] = 0x88888823;
	data_array[9] = 0x00000088;
    dsi_set_cmdq(&data_array, 10, 1); 
    MDELAY(1);
	 
	data_array[0] = 0x00053902;                          
       data_array[1] = 0x001000C7; 
       data_array[2] = 0x00000010; 
       dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(1);
	
	data_array[0] = 0x00053902;                          
       data_array[1] = 0x100006BF; 
       data_array[2] = 0x00000004; 
       dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(1);

       data_array[0] = 0x09cc1500;
       dsi_set_cmdq(data_array, 1, 1);
       MDELAY(1);


	data_array[0] = 0x002B3902;                          
       data_array[1] = 0x060400E0; 
	data_array[2] = 0x113F332B;                          
       data_array[3] = 0x0D0E0A34;
	data_array[4] = 0x13111311;                          
       data_array[5] = 0x04001710;
	data_array[6] = 0x3F332B06;                          
       data_array[7] = 0x0E0A3411;
	data_array[8] = 0x1113110D;                          
       data_array[9] = 0x0B171013;
       data_array[10] = 0x0B110717;                          
       data_array[11] = 0x00110717;
       dsi_set_cmdq(&data_array, 12, 1);
	MDELAY(1);
	
	data_array[0] = 0x00803902;                          
       data_array[1] = 0x070001C1; 
	data_array[2] = 0x251D150E;                          
       data_array[3] = 0x423C342D;
	data_array[4] = 0x5F585149;                          
       data_array[5] = 0x80776F67;
	data_array[6] = 0x9F988F87;                          
       data_array[7] = 0xC1B7AFA7;
	data_array[8] = 0xE6DDD3CB;                          
       data_array[9] = 0x16FFF6EF;
       data_array[10] = 0xCA627C25;                          
       data_array[11] = 0xC01FC23A;
       data_array[12] = 0x150E0700;                          
       data_array[13] = 0x342D251D; 
	data_array[14] = 0x5149423C;                          
       data_array[15] = 0x6F675F58;
	data_array[16] = 0x8F878077;                          
       data_array[17] = 0xAFA79F98;
	data_array[18] = 0xD3CBC1B7;                          
       data_array[19] = 0xF6EFE6DD;
	data_array[20] = 0x7C2516FF;                          
       data_array[21] = 0xC23ACA62;
       data_array[22] = 0x0700C01F;                          
       data_array[23] = 0x251D150E;
       data_array[24] = 0x423C342D;                          
       data_array[25] = 0x5F585149;
	data_array[26] = 0x80776F67;                          
       data_array[27] = 0x9F988F87;
       data_array[28] = 0xC1B7AFA7;                          
       data_array[29] = 0xE6DDD3CB;
       data_array[30] = 0x16FFF6EF;                          
       data_array[31] = 0xCA627C25;
      /* data_array[32] = 0xC01FC23A;*/
       dsi_set_cmdq(&data_array, 32, 1);
	MDELAY(1);
	
      data_array[0] = 0x00b61500;
      dsi_set_cmdq(data_array, 1, 1);
      MDELAY(1);

      data_array[0] = 0x32D41500;
      dsi_set_cmdq(data_array, 1, 1);
      MDELAY(1);


	//data_array[0] = 0x00033902;                          
       //data_array[1] = 0x00170CC0; 
       //dsi_set_cmdq(&data_array, 2, 1);
	//MDELAY(1);
	


	//data_array[0] = 0x00033902;                          
       //data_array[1] = 0x00800244; 
       //dsi_set_cmdq(&data_array, 2, 1);
	//MDELAY(1);
	
    data_array[0] = 0x00350500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(1);


	
	
    data_array[0] = 0x00110500;
    dsi_set_cmdq(data_array, 1, 1);
  	MDELAY(200);
  	
    data_array[0] = 0x00290500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(1);
#else //for RX
unsigned int data_array[16];
	
	data_array[0]= 0x00043902;
	data_array[1]= 0x9483FFB9;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(1);

	data_array[0]= 0x00023902;
	data_array[1]= 0x000013BA;   //11:2Lane  12:3Lane  13:4Lane
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(1);

	data_array[0]= 0x00113902;
	data_array[1]= 0x070001B1;
	data_array[2]= 0x11110187;
	data_array[3]= 0x3F3f302a;//ad by ricky   0x2d25EF4F
	data_array[4]= 0xE6011247;
	data_array[5]= 0x000000E2;
	dsi_set_cmdq(&data_array, 6, 1);
	MDELAY(1);

	data_array[0]= 0x00073902;
	data_array[1]= 0x08C800B2;
	data_array[2]= 0x00220004;
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(1);

	data_array[0]= 0x00173902;
	data_array[1]= 0x320680B4;//2dot->column  0xA2->0x80.
	data_array[2]= 0x15320310;
	data_array[3]= 0x08103208;
	data_array[4]= 0x05430433;
	data_array[5]= 0x063F0437;
	data_array[6]= 0x00066161;
	dsi_set_cmdq(&data_array, 7, 1);
	MDELAY(1);

	data_array[0]= 0x00213902;
	data_array[1]= 0x000000D5;
	data_array[2]= 0x01000A00;
	data_array[3]= 0x0000CC00;
	data_array[4]= 0x88888800;
	data_array[5]= 0x88888888;
	data_array[6]= 0x01888888;
	data_array[7]= 0x01234567;
	data_array[8]= 0x88888823;
	data_array[9]= 0x00000088;
	dsi_set_cmdq(&data_array, 10, 1);
	MDELAY(1);

	data_array[0]= 0x00023902;
	data_array[1]= 0x000000B6;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(1);

	data_array[0]= 0x00053902;
	data_array[1]= 0x001000C7;
	data_array[2]= 0x00000010;
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(1);

	data_array[0]= 0x00023902;
	data_array[1]= 0x000009CC;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(1);

	data_array[0]= 0x002B3902;
	data_array[1]= 0x060400E0;
	data_array[2]= 0x113F332B;
	data_array[3]= 0x0D0E0A34;
	data_array[4]= 0x13111311;
	data_array[5]= 0x04001710;
	data_array[6]= 0x3F332B06;
	data_array[7]= 0x0E0A3411;
	data_array[8]= 0x1113110D;
	data_array[9]= 0x0B171013;
	data_array[10]= 0x0B110717;
	data_array[11]= 0x00110717;
	dsi_set_cmdq(&data_array, 12, 1);
	MDELAY(1);



//¦Ì¡Â?? RGB ¨¨y¨¦?GM
	data_array[0]= 0x00293902;
	data_array[1]= 0x070001C1;
	data_array[2]= 0x251D150E;
	data_array[3]= 0x423C342D;
	data_array[4]= 0x5F585149;
	data_array[5]= 0x80776F67;
	data_array[6]= 0x9F988F87;
	data_array[7]= 0xC1B7AFA7;
	data_array[8]= 0xE6DDD3CB;
	data_array[9]= 0x16FFF6EF;
	data_array[10]= 0xCA627C25;
	data_array[11]= 0x00000003A;
	dsi_set_cmdq(&data_array, 12, 1);
	MDELAY(1);

	data_array[0]= 0x00292902;
	data_array[1]= 0xC01FC2C1;
	data_array[2]= 0x150E0700;
	data_array[3]= 0x342D251D;
	data_array[4]= 0x5149423C;
	data_array[5]= 0x6F675F58;
	data_array[6]= 0x8F878077;
	data_array[7]= 0xAFA79F98;
	data_array[8]= 0xD3CBC1B7;
	data_array[9]= 0xF6EFE6DD;
	data_array[10]= 0x7C2516FF;
	data_array[11]= 0x000000062;
	dsi_set_cmdq(&data_array, 12, 1);
	MDELAY(1);

	data_array[0]= 0x00292902;
	data_array[1]= 0xC23ACAC1;
	data_array[2]= 0x0700C01F;
	data_array[3]= 0x251D150E;
	data_array[4]= 0x423C342D;
	data_array[5]= 0x5F585149;
	data_array[6]= 0x80776F67;
	data_array[7]= 0x9F988F87;
	data_array[8]= 0xC1B7AFA7;
	data_array[9]= 0xE6DDD3CB;
	data_array[10]= 0x16FFF6EF;
	data_array[11]= 0x000000025;
	dsi_set_cmdq(&data_array, 12, 1);
	MDELAY(1);

	data_array[0]= 0x00082902;
	data_array[1]= 0xCA627CC1;
	data_array[2]= 0xC01FC23A;
	dsi_set_cmdq(&data_array,3, 1);
	MDELAY(1);
//¦Ì¡Â?? RGB ¨¨y¨¦?GM

	data_array[0]= 0x00023902;
	data_array[1]= 0x100006BF;
	data_array[2]= 0x10000004;
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(1);

	data_array[0]= 0x00023902;
	data_array[1]= 0x000032D4;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(1);

	data_array[0]= 0x00023902;
	data_array[1]= 0x0000773A;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(1);

       data_array[0] = 0x00110500;	//exit sleep mode
       dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(150);

	data_array[0] = 0x00290500;	//exit sleep mode
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(20);
	#endif
	
}
#endif

#if 1
static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {}},
    {REGFLAG_DELAY, 200, {}},

    // Display ON
	{0x29, 0, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 0, {}},

    // Sleep Mode On
	{0x10, 0, {}},
	{REGFLAG_DELAY, 120, {}},

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
       	}
    }
	
}

#endif
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
#if 0
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
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;//SYNC_PULSE_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 5;
		params->dsi.vertical_backporch					= 5;
		params->dsi.vertical_frontporch					= 5;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 20;
		params->dsi.horizontal_backporch				= 46;
		params->dsi.horizontal_frontporch				= 21;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		params->dsi.pll_div1=34;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
#else
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;
		
#if 1 //89 no need
		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	   // params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
#endif

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = BURST_VDO_MODE;////SYNC_PULSE_VDO_MODE;////BURST_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
	
		#if 1
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Highly depends on LCD driver capability.
		// Not support in MT6573
		//params->dsi.packet_size=256;
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.vertical_active_line				= FRAME_HEIGHT;
		params->dsi.intermediat_buffer_num = 0;//2

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;
		params->dsi.vertical_sync_active				= 2;
		params->dsi.vertical_backporch					= 10;
		params->dsi.vertical_frontporch					= 7;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		params->dsi.horizontal_sync_active				= 60;
		params->dsi.horizontal_backporch				= 50;//59//50
		params->dsi.horizontal_frontporch				= 50;///79;	//50
		params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
		
			
		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
        //params->dsi.pll_select=1;	//0: MIPI_PLL; 1: LVDS_PLL
		//params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_234;//LCM_DSI_6589_PLL_CLOCK_221;//LCM_DSI_6589_PLL_CLOCK_208;///LCM_DSI_6589_PLL_CLOCK_201_5;//;//this value must be in MTK suggested table
	
        params->dsi.pll_div1=0;//0;                // div1=0,1,2,3;div1_real=1,2,4,4
        params->dsi.pll_div2=1;//1;                // div2=0,1,2,3;div2_real=1,2,4,4
        params->dsi.fbk_div =18; //19;              // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)           
//params->dsi.pll_div1=34;//34;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	//	params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
		#endif
#endif
}


static void lcm_init(void)
{
	#if 1
#ifdef BUILD_LK
    upmu_set_rg_vgp2_vosel(5);
    upmu_set_rg_vgp2_en(1);
    MDELAY(20);    
#else
    if(TRUE != hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_2800, "LCM"))
	{
		printk("Fail to enable lcm 2v8 power!\n");
			   
	}
    MDELAY(2);	
#endif
#endif
    	printk("lrzlrzlrz1111111lcm_init\n");
	/*SET_RESET_PIN(1);
	MDELAY(5);    
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(10);*/


	SET_RESET_PIN(1);
	MDELAY(10); 	
	SET_RESET_PIN(0);
	MDELAY(20);
	
	SET_RESET_PIN(1);
	MDELAY(80);  
	init_lcm_registers();


//	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	#if 0
	//push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	unsigned int data_array[16];
	


/*	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(&data_array, 1, 1);
	
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(&data_array, 1, 1);*/

	//SET_RESET_PIN(0);
#endif
unsigned int data_array[16];
	printk("%s: def\n", __func__);
/*	
	data_array[0] = 0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10); 
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
*/		
//	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

/*
	mt_set_gpio_mode(LCM_RESET_PIN, GPIO_MODE_GPIO);
	mt_set_gpio_dir(LCM_RESET_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(LCM_RESET_PIN, GPIO_OUT_ZERO);
	MDELAY(120);
    */
//#ifdef BUILD_LK
	//upmu_set_rg_vgp5_en(0);//upmu_set_rg_vgp5_en
//#else
	//if(TRUE != hwPowerDown(MT65XX_POWER_LDO_VGP5, "LCM"))
	//{
		//printk("LCM Fail to disable 2v8 power!\n");
			   
	//}
//#endif
	//lcm_init();
  //  MDELAY(10);
//	data_array[0] = 0x00280500; // Display Off
	//dsi_set_cmdq(data_array, 1, 1);
	//MDELAY(10); 
	//data_array[0] = 0x00100500; // Sleep In
	//dsi_set_cmdq(data_array, 1, 1);
	//MDELAY(120);
   	SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);
}


static void lcm_resume(void)
{

	//	lcm_init();

	unsigned int data_array[16];
	//lcm_compare_id();
	//SET_RESET_PIN(1);
	printk("%s: hx8394alrz1308231130\n", __func__);
	lcm_init();
	//lcm_compare_id();
	//data_array[0] = 0x00110500; // Sleep Out
	//dsi_set_cmdq(&data_array, 1, 1);
	//MDELAY(120);
	//data_array[0] = 0x00290500; // Display On
	//dsi_set_cmdq(&data_array, 1, 1); 
	

	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
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
	dsi_set_cmdq(&data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);

//	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
//	dsi_set_cmdq(&data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(&data_array, 1, 0);

}

static unsigned int lcm_compare_id(void)
{
		unsigned int id1=0x40;
		unsigned int id2=0x94;
		unsigned int id3=0;
		unsigned int id4=0;
		unsigned int TRY_SW_ID=0;
		unsigned char buffer[2];
		unsigned int array[16];  
	//return 1;
/*
		SET_RESET_PIN(1);
		SET_RESET_PIN(0);
		MDELAY(1);
		SET_RESET_PIN(1);
		MDELAY(10);//Must over 6 ms
*/
	/*array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);	
	read_reg_v2(0xda, buffer, 2);
	id3 = buffer[0];
	id4 = buffer[1];

		array[0]=0x00043902;
		array[1]=0x9483FFB9;// page enable
		dsi_set_cmdq(&array, 2, 1);
		MDELAY(10);

		array[0] = 0x00023700;// return byte number
		dsi_set_cmdq(&array, 1, 1);
		MDELAY(10);
	
		read_reg_v2(0xF4, buffer, 2);
		id1 = buffer[0]; 
		id2 = buffer[1];*/
		
		////////////////////////////////////////////////////
		read_reg_v2(0xdb, buffer, 1);
	TRY_SW_ID = buffer[0]; //we only need ID
	//id2 = buffer[1]; //we only need ID
//#endif

		//if(TRY_SW_ID == id1||TRY_SW_ID==id2)
		if(TRY_SW_ID==id2)
		{
		printk("%s: find the LCM drv HX8394 \n", __func__);
			printk("lrzlrzlrz1111111\n");
		printk("TRY_SW_ID=0x%x\n", TRY_SW_ID);
		return 1;
		}
	else
		{
		printk("%s: can not find the lcm drv HX8394 \n", __func__);
	  printk("lrzlrzlrz2222222222\n");
		printk("TRY_SW_ID=0x%x\n", TRY_SW_ID);
		printk("lrzlrzlrz33333333333\n");
		return 0;
		}
		//////////////////////////////////////////////////////////
		
		
		
		
#if defined(BUILD_LK)
		printf("=====>haitao zhou test %s, id1 = 0x%08x\n", __func__, id1);
#endif
		printk("=====>haitao zhou test %s, id1 = 0x%08x, id2 = 0x%08x, id3 = 0x%08x, id4 = 0x%08x\n", __func__, id1,id2,id3,id4);
		return (LCM_ID_HX8394A == id1)?1:1;////1:0

}




// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hx8394_dsi_vdo_hd_lcm_drv = 
{
    .name			= "hx8394_dsi_vdo_hd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
	//.set_backlight	= lcm_setbacklight,
//	.set_pwm        = lcm_setpwm,
//	.get_pwm        = lcm_getpwm,
	.esd_check   = lcm_esd_check,
	.esd_recover   = lcm_esd_recover,
	.compare_id    = lcm_compare_id,
#endif
//	.esd_check   = lcm_esd_check,
//	.esd_recover   = lcm_esd_recover,
	.compare_id    = lcm_compare_id,
//	.set_pwm        = lcm_setpwm,
//	.get_id		= lcm_get_id,
};

