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
// Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE    1
#define FRAME_WIDTH (720)
#define FRAME_HEIGHT (1280)
#define LCM_ID (0x90)


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


static unsigned int lcm_esd_test = FALSE; ///only for ESD test
static bool lcm_is_init = false;
//#define LCM_DSI_CMD_MODE									1

// ---------------------------------------------------------------------------
// Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#ifdef BUILD_LK
#define DBG(fmt, arg...) printf(fmt,##arg);
#else
#define DBG(fmt, arg...) printk(fmt,##arg);
#endif

// ---------------------------------------------------------------------------
// Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size) 

struct LCM_setting_table {
unsigned char cmd;
unsigned char count;
unsigned char para_list[64];
};

static void Generic_Short_Write_1P(unsigned char cmd, unsigned char para)
{
	unsigned int data_array[16];
	//unsigned char buffer;



	data_array[0] =(0x00001500 | (para<<24) | (cmd<<16));
	dsi_set_cmdq(data_array, 1, 1);

	//MDELAY(1);

}
#define Generic_Short_Write_0P(cmd)	data_array[0]=(0x00000500 | (cmd<<16)); \
					dsi_set_cmdq(data_array,1,1);


// ---------------------------------------------------------------------------
// LCM Driver Implementations
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
params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
//params->dbi.te_mode = LCM_DBI_TE_MODE_DISABLED;
params->dbi.te_edge_polarity		= LCM_POLARITY_FALLING;
params->dsi.word_count=720*3;	//DSI CMD mode need set these two bellow params, different to 6577
params->dsi.vertical_active_line=1280;
#endif


        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
        #endif
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_THREE_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
		params->dsi.vertical_sync_active				= 1;// 3    2
		params->dsi.vertical_backporch					= 1;// 20   1
		params->dsi.vertical_frontporch					= 2; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 2;// 50  2
		params->dsi.horizontal_backporch				= 12;
		params->dsi.horizontal_frontporch				= 80;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	    //params->dsi.LPX=8; 

		// Bit rate calculation
		//1 Every lane speed
		//params->dsi.pll_select=1;
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
#if (LCM_DSI_CMD_MODE)
		//params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_22;//this value must be in MTK suggested table
		params->dsi.fbk_div =18;//22;
#else
		params->dsi.fbk_div =16;//20;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
#endif
		params->dsi.compatibility_for_nvk = 1;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's


}


static void lcm_init_register(void)
{
unsigned int data_array[16];
//CMD1
Generic_Short_Write_1P(0xFF,0x00);
//MIPI 3 lane                   
Generic_Short_Write_1P(0xBA,0x02);//0x01---2LANE   0x02---3LANE    0x03---4LANE 
//MIPI command mode            
Generic_Short_Write_1P(0xC2,0x08);//0x08 command
                                
//CMD2,Page0                 
Generic_Short_Write_1P(0xFF,0x01);
//Turn on test pump_2012/09/15_update
Generic_Short_Write_1P(0x0A,0x3B);
//Set VGSW/NB/720x480         
Generic_Short_Write_1P(0x00,0x3A);
//Charge Pump clock³]©w        
Generic_Short_Write_1P(0x01,0x33);
Generic_Short_Write_1P(0x02,0x53);
//VGL=-6V                      
Generic_Short_Write_1P(0x09,0x85);
//VGH=+8.6V                    
Generic_Short_Write_1P(0x0E,0x25);
//turn off VGLO regulator     
Generic_Short_Write_1P(0x0F,0x0A);
//GVDDP=4V                    
Generic_Short_Write_1P(0x0B,0x97);
//GVDDN=-4V                    
Generic_Short_Write_1P(0x0C,0x97);
//VCOMDC3=-0.24V_2012/07/06   
Generic_Short_Write_1P(0x11,0x8A);
//Open Gate_EQ_Cut2_2012/04/13_u);pdate
Generic_Short_Write_1P(0x36,0x7B);
//³]©wMIPI TA_GO_2011/11/29_upda);te
Generic_Short_Write_1P(0x71,0x2C);
//CMD2,Page4                 
Generic_Short_Write_1P(0xFF,0x05);
//Turn off SRAM power in sleep i);n_2012/09/15
Generic_Short_Write_1P(0xA2,0x11);
//LTPS timing                 
Generic_Short_Write_1P(0x01,0x00);
Generic_Short_Write_1P(0x02,0x8D);
Generic_Short_Write_1P(0x03,0x8D);
Generic_Short_Write_1P(0x04,0x8D);
Generic_Short_Write_1P(0x05,0x30);
Generic_Short_Write_1P(0x06,0x33);
Generic_Short_Write_1P(0x07,0x77);
Generic_Short_Write_1P(0x08,0x00);
Generic_Short_Write_1P(0x09,0x00);
Generic_Short_Write_1P(0x0A,0x00);
Generic_Short_Write_1P(0x0B,0x80);
Generic_Short_Write_1P(0x0C,0xC8);
Generic_Short_Write_1P(0x0D,0x00);
Generic_Short_Write_1P(0x0E,0x1B);
Generic_Short_Write_1P(0x0F,0x07);
Generic_Short_Write_1P(0x10,0x57);
Generic_Short_Write_1P(0x11,0x00);
Generic_Short_Write_1P(0x12,0x00);
Generic_Short_Write_1P(0x13,0x1E);
Generic_Short_Write_1P(0x14,0x00);
Generic_Short_Write_1P(0x15,0x1A);
Generic_Short_Write_1P(0x16,0x05);
Generic_Short_Write_1P(0x17,0x00);
Generic_Short_Write_1P(0x18,0x1E);
Generic_Short_Write_1P(0x19,0xFF);
Generic_Short_Write_1P(0x1A,0x00);
Generic_Short_Write_1P(0x1B,0xFC);
//2012/08/28 update           
Generic_Short_Write_1P(0x1C,0x90);
Generic_Short_Write_1P(0x1D,0x00);
Generic_Short_Write_1P(0x1E,0x00);
//2012/08/28 update            
Generic_Short_Write_1P(0x1F,0x77);
Generic_Short_Write_1P(0x20,0x00);
//2012/07/02 Gate inverse      
Generic_Short_Write_1P(0x21,0x00);
Generic_Short_Write_1P(0x22,0x55);
Generic_Short_Write_1P(0x23,0x0D);
Generic_Short_Write_1P(0x31,0xA0);
Generic_Short_Write_1P(0x32,0x00);
Generic_Short_Write_1P(0x33,0xB8);
Generic_Short_Write_1P(0x34,0xBB);
Generic_Short_Write_1P(0x35,0x11);
Generic_Short_Write_1P(0x36,0x01);
Generic_Short_Write_1P(0x37,0x0B);
Generic_Short_Write_1P(0x38,0x01);
Generic_Short_Write_1P(0x39,0x0B);
Generic_Short_Write_1P(0x44,0x08);
Generic_Short_Write_1P(0x45,0x80);
Generic_Short_Write_1P(0x46,0xCC); 
Generic_Short_Write_1P(0x47,0x04);
Generic_Short_Write_1P(0x48,0x00);
Generic_Short_Write_1P(0x49,0x00);
Generic_Short_Write_1P(0x4A,0x01);
Generic_Short_Write_1P(0x6C,0x03);
Generic_Short_Write_1P(0x6D,0x03);
Generic_Short_Write_1P(0x6E,0x2F);
Generic_Short_Write_1P(0x43,0x00);
Generic_Short_Write_1P(0x4B,0x23);
Generic_Short_Write_1P(0x4C,0x01);
Generic_Short_Write_1P(0x50,0x23);
Generic_Short_Write_1P(0x51,0x01);
Generic_Short_Write_1P(0x58,0x23);
Generic_Short_Write_1P(0x59,0x01);
Generic_Short_Write_1P(0x5D,0x23);
Generic_Short_Write_1P(0x5E,0x01);
Generic_Short_Write_1P(0x62,0x23);
Generic_Short_Write_1P(0x63,0x01);
Generic_Short_Write_1P(0x67,0x23);
Generic_Short_Write_1P(0x68,0x01);
Generic_Short_Write_1P(0x89,0x00);
Generic_Short_Write_1P(0x8D,0x01);
Generic_Short_Write_1P(0x8E,0x64);
Generic_Short_Write_1P(0x8F,0x20);
Generic_Short_Write_1P(0x97,0x8E);
Generic_Short_Write_1P(0x82,0x8C);
Generic_Short_Write_1P(0x83,0x02);
Generic_Short_Write_1P(0xBB,0x0A);
Generic_Short_Write_1P(0xBC,0x0A);
Generic_Short_Write_1P(0x24,0x25);
Generic_Short_Write_1P(0x25,0x55);
Generic_Short_Write_1P(0x26,0x05);
Generic_Short_Write_1P(0x27,0x23);
Generic_Short_Write_1P(0x28,0x01);
Generic_Short_Write_1P(0x29,0x31);
Generic_Short_Write_1P(0x2A,0x5D);
Generic_Short_Write_1P(0x2B,0x01);
Generic_Short_Write_1P(0x2F,0x00);
Generic_Short_Write_1P(0x30,0x10);
Generic_Short_Write_1P(0xA7,0x12);
Generic_Short_Write_1P(0x2D,0x03);
                                
//CMD2,Page0_Gamma2.2_2012/07/06);
Generic_Short_Write_1P(0xFF,0x01);
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
Generic_Short_Write_1P(0xFF,0x00);
Generic_Short_Write_1P(0xFB,0x01);
//CMD2,Page0                  
Generic_Short_Write_1P(0xFF,0x01);
Generic_Short_Write_1P(0xFB,0x01);
//CMD2,Page1                  
Generic_Short_Write_1P(0xFF,0x02);
Generic_Short_Write_1P(0xFB,0x01);
//CMD2,Page2                  
Generic_Short_Write_1P(0xFF,0x03);
Generic_Short_Write_1P(0xFB,0x01);
//CMD2,Page3                   
Generic_Short_Write_1P(0xFF,0x04);
Generic_Short_Write_1P(0xFB,0x01);
//CMD2,Page4                   
Generic_Short_Write_1P(0xFF,0x05);
Generic_Short_Write_1P(0xFB,0x01);
//CMD1                        
Generic_Short_Write_1P(0xFF,0x00);
Generic_Short_Write_0P(0x11);
MDELAY (150);                      
//not open CABC                
Generic_Short_Write_1P(0x51,0xFF);
Generic_Short_Write_1P(0x53,0x2C);
Generic_Short_Write_1P(0x55,0x00);
Generic_Short_Write_0P(0x29);
//CMD1                         
Generic_Short_Write_1P(0xFF,0x00);
Generic_Short_Write_1P(0x35,0x00);
}                                       

static void lcm_init(void)
{
lcm_is_init = true;
	SET_RESET_PIN(1);
	MDELAY(10);
Generic_Short_Write_1P(0xFF,0xee);
Generic_Short_Write_1P(0x26,0x08);
	MDELAY(10);
Generic_Short_Write_1P(0x26,0x00);
Generic_Short_Write_1P(0xFB,0x01);
Generic_Short_Write_1P(0xFF,0x00);
	MDELAY(10);

//	SET_RESET_PIN(1);
//	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(5);
	SET_RESET_PIN(1);
	MDELAY(120);

	//push_table(lcm_initialization_ret, sizeof(lcm_initialization_ret) / sizeof(struct LCM_setting_table), 1);
	lcm_init_register();
	DBG("[JX] %s \n",__func__);
}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

	mt_set_gpio_out(GPIO155, 0);
Generic_Short_Write_1P(0xFF,0x00);
Generic_Short_Write_0P(0x28);
                    
Generic_Short_Write_0P(0x10)

	MDELAY(150);

//SET_RESET_PIN(0);

lcm_is_init = false;
DBG("[JX] nt35590 %s \n",__func__);
}


static void lcm_resume(void)
{
	int i;
if(!lcm_is_init){
	unsigned int data_array[16];

	mt_set_gpio_out(GPIO155, 0);
Generic_Short_Write_1P(0xFF,0x00);
Generic_Short_Write_0P(0x11);
MDELAY (150);                      
Generic_Short_Write_0P(0x29);
lcm_is_init = true;
//	lcm_init();

	for(i = 0; i < 6; i++)
	{
		mt_set_gpio_out(155, 1);
		UDELAY(1);
		mt_set_gpio_out(155, 0);
		UDELAY(1);
	}
	mt_set_gpio_out(155, 1);

}

DBG("[JX] nt35590 %s \n",__func__);
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

//	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
//	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);


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
    if(lcm_esd_test)
    {
        lcm_esd_test = FALSE;
        return TRUE;
    }
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
}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();

	return TRUE;
}

static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int data_array[16];  

    SET_RESET_PIN(1);
   MDELAY(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(50);//Must over 6 ms
Generic_Short_Write_1P(0xFF,0x00);
	data_array[0] = 0x00013700;
	dsi_set_cmdq(&data_array, 1, 1);

	read_reg_v2(0xF4, buffer, 1);
	id = buffer[0]; //we only need ID
//        id = read_reg(0xDA);
#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, nt35590 id = 0x%x\n", __func__, id);
#else
       printk("%s, nt35590 id = 0x%x\n", __func__, id);
#endif

	return (LCM_ID == id)?1:0;

}

// ---------------------------------------------------------------------------
// Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER nt35590_hd720_dsi_cmd_yassy_seine_lcm_drv = 
{
	.name = "nt35590_hd720_dsi_cmd_yassy_seine",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.update = lcm_update,
        .set_backlight = lcm_setbacklight,
// .set_pwm = lcm_setpwm,
// .get_pwm = lcm_getpwm,
.esd_check = lcm_esd_check,
.esd_recover = lcm_esd_recover,

#endif
};
