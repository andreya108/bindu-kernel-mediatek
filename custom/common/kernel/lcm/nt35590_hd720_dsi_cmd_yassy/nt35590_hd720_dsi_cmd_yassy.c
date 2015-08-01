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
#define LCM_ID (0x69)
#define REGFLAG_DELAY 0xAB
#define REGFLAG_END_OF_TABLE 0xAA // END OF REGISTERS MARKER

#define LCM_ID1 0x00
#define LCM_ID2 0x00
#define LCM_ID3 0x00

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
#define NT35590    1

static unsigned int lcm_esd_test = FALSE; ///only for ESD test
//#define LCM_DSI_CMD_MODE									1

// ---------------------------------------------------------------------------
// Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


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


static struct LCM_setting_table lcm_initialization_setting[] = {

/*
Note :

Data ID will depends on the following rule.

count of parameters > 1 => Data ID = 0x39
count of parameters = 1 => Data ID = 0x15
count of parameters = 0 => Data ID = 0x05

Structure Format :

{DCS command, count of parameters, {parameter list}}
{REGFLAG_DELAY, milliseconds of time, {}},

...

Setting ending by predefined flag

{REGFLAG_END_OF_TABLE, 0x00, {}}
*/


};

static struct LCM_setting_table lcm_initialization_ret[] = {
{0xFF, 1, {0x00}},
{0xBA, 1, {0x03}}, // 4 lane
{0xC2, 1, {0x08}},

{0xFF, 1, {0x01}},
{0x00, 1, {0x3A}},
{0x01, 1, {0x33}},
{0x02, 1, {0x53}},
{0x09, 1, {0x85}},
{0x0E, 1, {0x25}},
{0x0F, 1, {0x0A}},
{0x0B, 1, {0x97}},
{0x0C, 1, {0x97}},
{0x11, 1, {0x8C}},
{0x36, 1, {0x7B}},
{0x71, 1, {0x2C}},

{0xFF, 1, {0x05}},
{0x01, 1, {0x00}},
{0x02, 1, {0x8D}},
{0x03, 1, {0x8D}},
{0x04, 1, {0x8D}},
{0x05, 1, {0x30}},
{0x06, 1, {0x33}},
{0x07, 1, {0x77}},
{0x08, 1, {0x00}},
{0x09, 1, {0x00}},
{0x0A, 1, {0x00}},
{0x0B, 1, {0x80}},
{0x0C, 1, {0xC8}},
{0x0D, 1, {0x00}},
{0x0E, 1, {0x1B}},
{0x0F, 1, {0x07}},
{0x10, 1, {0x57}},
{0x11, 1, {0x00}},
{0x12, 1, {0x00}},
{0x13, 1, {0x1E}},
{0x14, 1, {0x00}},
{0x15, 1, {0x1A}},
{0x16, 1, {0x05}},
{0x17, 1, {0x00}},
{0x18, 1, {0x1E}},
{0x19, 1, {0xFF}},
{0x1A, 1, {0x00}},
{0x1B, 1, {0xFC}},
{0x1C, 1, {0x80}},
{0x1D, 1, {0x00}},
{0x1E, 1, {0x00}},
{0x1F, 1, {0x77}},
{0x20, 1, {0x00}},
{0x21, 1, {0x00}},
{0x22, 1, {0x55}},
{0x23, 1, {0x0D}},
{0x31, 1, {0xA0}},
{0x32, 1, {0x00}},
{0x33, 1, {0xB8}},
{0x34, 1, {0xBB}},
{0x35, 1, {0x11}},
{0x36, 1, {0x01}},
{0x37, 1, {0x0B}},
{0x38, 1, {0x01}},
{0x39, 1, {0x0B}},
{0x44, 1, {0x08}},
{0x45, 1, {0x80}},
{0x46, 1, {0xCC}},
{0x47, 1, {0x04}},
{0x48, 1, {0x00}},
{0x49, 1, {0x00}},
{0x4A, 1, {0x01}},
{0x6C, 1, {0x03}},
{0x6D, 1, {0x03}},
{0x6E, 1, {0x2F}},
{0x43, 1, {0x00}},
{0x4B, 1, {0x23}},
{0x4C, 1, {0x01}},
{0x50, 1, {0x23}},
{0x51, 1, {0x01}},
{0x58, 1, {0x23}},
{0x59, 1, {0x01}},
{0x5D, 1, {0x23}},
{0x5E, 1, {0x01}},
//{0x62, 1, {0x23}},//
//{0x63, 1, {0x01}},//
//{0x67, 1, {0x23}},//
//{0x68, 1, {0x01}},//
{0x89, 1, {0x00}},
{0x8D, 1, {0x01}},
{0x8E, 1, {0x64}},
{0x8F, 1, {0x20}},
{0x97, 1, {0x8E}},
{0x82, 1, {0x8C}},
{0x83, 1, {0x02}},
{0xBB, 1, {0x0A}},
{0xBC, 1, {0x0A}},
{0x24, 1, {0x25}},
{0x25, 1, {0x55}},
{0x26, 1, {0x05}},
{0x27, 1, {0x23}},
{0x28, 1, {0x01}},
{0x29, 1, {0x31}},
{0x2A, 1, {0x5D}},
{0x2B, 1, {0x01}},
{0x2F, 1, {0x00}},
{0x30, 1, {0x10}},
{0xA7, 1, {0x12}},
{0x2D, 1, {0x03}},
// Skip Gamma, CABC

{0xFF, 1, {0x00}},
{0xFB, 1, {0x01}},

{0xFF, 1, {0x01}},
{0xFB, 1, {0x01}},

{0xFF, 1, {0x02}},
{0xFB, 1, {0x01}},

{0xFF, 1, {0x03}},
{0xFB, 1, {0x01}},

{0xFF, 1, {0x04}},
{0xFB, 1, {0x01}},

{0xFF, 1, {0x05}},
{0xFB, 1, {0x01}},

{0xFF, 1, {0x00}},
{0x3A, 1, {0x77}},
{0x36, 1, {0xD4}},

{0x11, 1, {0x00}},
{REGFLAG_DELAY, 120, {}},

{0x51, 1, {0xFF}},
{0x53, 1, {0x2C}},
{0x55, 1, {0x00}},
// Display ON
{0x29, 1, {0x00}},
//{REGFLAG_END_OF_TABLE, 0x00, {}},
{REGFLAG_DELAY, 20, {}},

{0xFF, 1, {0x00}},
{0x35, 1, {0x00}},
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
// Sleep Out
{0x11, 1, {0x00}},
{REGFLAG_DELAY, 120, {}},

// Display ON
{0x29, 1, {0x00}},
{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
// Display off sequence
{0x28, 1, {0x00}},

// Sleep Mode On
{0x10, 1, {0x00}},

{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_compare_id_setting[] = {
// Display off sequence
{0xB9, 3, {0xFF, 0x83, 0x69}},
{REGFLAG_DELAY, 10, {}},

// Sleep Mode On
// {0xC3, 1, {0xFF}},

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

params->type = LCM_TYPE_DSI;

params->width = FRAME_WIDTH;
params->height = FRAME_HEIGHT;


#if (LCM_DSI_CMD_MODE)
params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
//params->dbi.te_mode = LCM_DBI_TE_MODE_DISABLED;
params->dbi.te_edge_polarity		= LCM_POLARITY_FALLING;
#endif

#if (LCM_DSI_CMD_MODE)
params->dsi.mode = CMD_MODE;
#else
params->dsi.mode = SYNC_PULSE_VDO_MODE;
#endif

// DSI
/* Command mode setting */
params->dsi.LANE_NUM = LCM_FOUR_LANE;
//The following defined the fomat for data coming from LCD engine.
params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

params->dsi.word_count=720*3;	//DSI CMD mode need set these two bellow params, different to 6577
params->dsi.vertical_active_line=1280;

// Video mode setting 
params->dsi.intermediat_buffer_num = 0;

params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
params->dsi.pll_select=1;	//0: MIPI_PLL; 1: LVDS_PLL

//bit rate
	params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4
	params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4
	params->dsi.fbk_div =18;//13;//18; //15;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		

	params->dsi.compatibility_for_nvk = 1;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's

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
#if 1
//yassy:20130114 gamma+0130 color enhance,pump
//聯想4.7_AUO Gamma code
//page selection cmd start

data_array[0] = 0x00023902;
data_array[1] = 0x00FF;    
dsi_set_cmdq(&data_array, 2, 1);

//MIPI 4 lane
data_array[0] = 0x00023902;
data_array[1] = 0x03BA;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x08C2;
dsi_set_cmdq(&data_array, 2, 1);

//power start
//CMD2 Page0 -Power 
data_array[0] = 0x00023902;
data_array[1] = 0x01FF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01FB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4A00;
dsi_set_cmdq(&data_array, 2, 1);

//charge pump frequence modify
data_array[0] = 0x00023902;
data_array[1] = 0x3301;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5302;
dsi_set_cmdq(&data_array, 2, 1);
//charge pump frequence modify

data_array[0] = 0x00023902;
data_array[1] = 0x5503;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5504;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3305;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2206;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5608;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8F09;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x7336;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9F0B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9F0C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2F0D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x240E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8111; //86, for flick
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0312;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2C71;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x036F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0312;
dsi_set_cmdq(&data_array, 2, 1);

//NVT ENGINEER COMMAND
data_array[0] = 0x00023902;
data_array[1] = 0x0A0F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01FB; //////////AUO add
dsi_set_cmdq(&data_array, 2, 1);
//////////////////////////NTK ENGINEER COMMAND w/o setting ?


//////////
//CMD2 Page 4 -Display Register
data_array[0] = 0x00023902;
data_array[1] = 0x05FF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01FB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0001;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9902; //78=70Hz,0x82=65Hz,0xa6=51Hz,0x8D=60Hz, 0x99=56Hz
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9903; //78
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9904; //78
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3005;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3306;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0107;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0008;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4609;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x460A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0B0D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1D0E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x080F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5310;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0011;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0012;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0114;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0015;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0516;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0017;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x7F19;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xFF1A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0F1B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x001C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x001D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x001E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x071F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0020;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0021;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2522; //05:1dot,15:2dot,25:4dot,55:column
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4D23;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x022D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0183;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x589E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6A9F;
dsi_set_cmdq(&data_array, 2, 1);

//REGW 0x9F80;0x000A
data_array[0] = 0x00023902;
data_array[1] = 0x01A0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x10A2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0ABB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0ABC;
dsi_set_cmdq(&data_array, 2, 1);

//NVT ENGINEER COMMAND
data_array[0] = 0x00023902;
data_array[1] = 0x0128; // RGB switch
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x022F; // RGB switch
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0832;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xB833;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0136;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0037;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0043;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x006C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x006D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00FF;
dsi_set_cmdq(&data_array, 2, 1);

//power end


//gamma start
data_array[0] = 0x00023902;
data_array[1] = 0x01FF; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01FB;   ///////////////AUO add
dsi_set_cmdq(&data_array, 2, 1);

//page selection cmd end
//R(+) MCR cmd
data_array[0] = 0x00023902;
data_array[1] = 0x0075;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6676; //0176; //EA76;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0077; //0177;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6878; //0178;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0079; //0179;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x797a; //297A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007b; //017B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x877c; // 2B7C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007d; //017D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xa37e; // 2D7E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007f; //017F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xc780; // 2F80;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0081; //0181;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd182; //3182;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0083; //0183;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe184; //3884;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0085; //0185;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf386; // 3F86;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0187;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2688; //5C88;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0189;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0X4b8a; //778A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x018B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x878c; //A08C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x018D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xb68e; //C58E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x018f; //028F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfa90; //0290;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0291;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3192; //3492;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0293;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3494; //3694;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0295;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6496; //6496;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0297;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9798; //9398;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0299;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xb69a; //B49A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x029B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe59c; //E09C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x079e; //FF9E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x34a0; //28A0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x45a3; //31A3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x55a5; // 3FA5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x56a7; // 4DA7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x57aa; //53AA;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03AB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x58ac; //54AC;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03AD;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd6ae; //BDAE;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03AF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xdcb0; //C1B0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03B1;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe6b2; //C7B2;
dsi_set_cmdq(&data_array, 2, 1);


//R(-) MCR cmd
data_array[0] = 0x00023902;
data_array[1] = 0x00B3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x66b4; //EAB4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00b5; //01B5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x68b6; //01B6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00b7; //01B7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x79b8; //29B8;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00b9; //01B9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x87ba; // 2BBA;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00bb; //01BB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xa3bc; // 2DBC;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00bd; //01BD;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xc7be; // 2FBE;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00bf; //01BF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd1c0; //31C0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00c1; //01C1;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe1c2; //38C2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00c3; //01C3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf3c4; // 3FC4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x26c6; //5CC6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0X4bc8; //77C8;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x87ca; //A0CA;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01CB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xb6cc; //C5CC;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01cd; //02CD;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xface; //02CE;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02CF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x31d0; //34D0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D1;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x34d2; //36D2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x64d4; //64D4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x97d6; //93D6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xb6d8; //B4D8;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe5da; //E0DA;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x07dc; //FFDC;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DD;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x34de; //28DE;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x45e0; //31E0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E1;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x55e2; // 3FE2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x56e4; // 4DE4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x57e6; //53E6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x58e8; //54E8;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd6ea; //BDEA;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03EB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xdcec; //01EC;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03ED;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe6ee; //C7EE;
dsi_set_cmdq(&data_array, 2, 1);

//G(+) MCR cmd
data_array[0] = 0x00023902;
data_array[1] = 0x00EF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf3f0; //F6F0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00F1;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf5f2; //F7F2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00F3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf9f4; //F8F4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00f5; //01F5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfdf6; //06F6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01f7; //01F7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01f8; //13F8;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01f9; //01F9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0bfa; // 1FFA;
dsi_set_cmdq(&data_array, 2, 1);

//page selection cmd start
data_array[0] = 0x00023902;
data_array[1] = 0x02FF; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01FB;  //////////AUO add
dsi_set_cmdq(&data_array, 2, 1);

//page selection cmd end
data_array[0] = 0x00023902;
data_array[1] = 0x0100; //0100;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1801; // 2A01;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0102; //0102;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2203; //3303;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0104; //0104;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2b05; // 3B05;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0106;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4d07; //5807;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0108;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0X6a09; //7109;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x010A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9a0b; //9E0B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x010C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xc10d; //C30D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x020E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x010f; //FF0F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0210;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3411; //3111;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0212;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3513; //3213;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0214;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6315; //6015;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0216;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9717; //9417;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0218;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xb919; //B519;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x021A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe81b; //E31B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x031c; //031C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x091d; //021D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x031E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x391f; // 2D1F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0320;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4921; // 3B21;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0322;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5923; // 4A23;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0324;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5a25; //5C25;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0326;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5b27; //7327;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0328;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5c29; //7429;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x032A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xcb2b; //BF2B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x032D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd52f; //C02F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0330;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xda31; //C331;
dsi_set_cmdq(&data_array, 2, 1);

//G(-) MCR cmd
data_array[0] = 0x00023902;
data_array[1] = 0x0032;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf333; //F633;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0034;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf535; //F735;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0036;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf937; //F837;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0038; //0138;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfd39; //0639;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x013a; //013A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x013b; //133B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x013d; //013D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0b3f; // 1F3F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0140; //0140;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1841; // 2A41;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0142; //0142;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2243; //3343;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0144; //0144;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2b45; // 3B45;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0146;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4d47; //5847;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0148;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0X6a49; //7149;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x014A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9a4b; //9E4B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x014C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xc14d; //C34D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x024E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x014f; //FF4F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0250;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3451; //3151;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0252;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3553; //3253;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0254;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6355; //6055;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0256;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9758; //9458;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0259;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xb95a; //B55A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x025B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe85c; //E35C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x035d; //035D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x095e; //025E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x035F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3960; // 2D60;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0361;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4962; // 3B62;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0363;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5964; // 4A64;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0365;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5a66; //5C66;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0367;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5b68; //7368;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0369;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5c6a; //746A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x036B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xcb6c; //BF6C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x036D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd56e; //C06E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x036F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xda70; //C370;
dsi_set_cmdq(&data_array, 2, 1);


//B(+) MCR cmd
data_array[0] = 0x00023902;
data_array[1] = 0x0071;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1672;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0073;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2174; //0274;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0075;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3376; //0376;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0077;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4378; //0878;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0079;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x527a; //0A7A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x607c; //147C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6f7e; //247E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8c80; //4380;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0081;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xa282; //6382;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0083;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe584; //B384;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0185; //0085;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0X1686; //EB86;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0187;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5d88; // 3D88;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0189;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x948a; //778A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x018B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe58c; //D18C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x028D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x228e; //128E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x028F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2490; //1590;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0291;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5892; // 4D92;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0293;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8f94; //8394;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0295;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xb196; //A696;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0297;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe498; //D698;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0399;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0b9a; //F99A;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039B;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4e9c; // 2C9C;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039D;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x709e; // 3F9E;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039F;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe3a0; //55A0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe4a3; //7BA3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe5a5; //EDA5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe6a7; //EEA7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfcaa; //F6AA;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03AB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfdac; //F9AC;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03AD;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf1ae; //FCAE;
dsi_set_cmdq(&data_array, 2, 1);

//B(-) MCR cmd
data_array[0] = 0x00023902;
data_array[1] = 0x00AF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x16B0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B1;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x21b2; //02B2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x33b4; //03B4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x43b6; //08B6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x52b8; //0AB8;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x60ba; //14BA;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00BB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6fbc; //24BC;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00BD;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8cbe; //43BE;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00BF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xa2c0; //63C0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00C1;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe5c2; //B3C2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01c3; //00C3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0X16c4; //EBC4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5dc6; // 3DC6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x94c8; //77C8;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe5ca; //D1CA;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02CB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x22cc; //12CC;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02CD;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x24ce; //15CE;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02CF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x58d0; // 4DD0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D1;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8fd2; //83D2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xb1d4; //A6D4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe4d6; //D6D6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03D7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0bd8; //F9D8;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03D9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4eda; // 2CDA;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DB;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x70dc; // 3FDC;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DD;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe3de; //55DE;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DF;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe4e0; //7BE0;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E1;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe5e2; //EDE2;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E3;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe6e4; //EEE4;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E5;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfce6; //F6E6;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E7;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfde8; //F9E8;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E9;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfeea; //FCEA;
dsi_set_cmdq(&data_array, 2, 1);
//gamma end


//////////
#if 0
//進\u20acJCMD 2,PAGE2		
data_array[0] = 0x00023902;
data_array[1] = 0x03FF; 
dsi_set_cmdq(&data_array, 2, 1);

//MTP no reload		
data_array[0] = 0x00023902;
data_array[1] = 0x01FB;	
dsi_set_cmdq(&data_array, 2, 1);

//VIVID COLOR		
data_array[0] = 0x00023902;
data_array[1] = 0x0800;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0801;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1002;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2003;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2F04;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2F05;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2F06;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2F07;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2F08;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2F09;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2F0A;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2F0B;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x200C;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x100D;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x080E;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x080F;	
dsi_set_cmdq(&data_array, 2, 1);

//SMART COLOR		
data_array[0] = 0x00023902;
data_array[1] = 0x001B;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x001C;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x001D;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x001E;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x001F;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0020;	
dsi_set_cmdq(&data_array, 2, 1);

//Others		
data_array[0] = 0x00023902;
data_array[1] = 0x0024;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2025;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0018;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0019;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x001A;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0021;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0022;	
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0023;	
dsi_set_cmdq(&data_array, 2, 1);
#endif

//CMD1
data_array[0] = 0x00023902;
data_array[1] = 0x00FF;    
dsi_set_cmdq(&data_array, 2, 1);

#if 1 //TE
data_array[0] = 0x00023902;                  
data_array[1] = 0x0035;                 
dsi_set_cmdq(&data_array, 2, 1);    

data_array[0] = 0x00033902;
data_array[1] = (((FRAME_HEIGHT/2)&0xFF) << 16) | (((FRAME_HEIGHT/2)>>8) << 8) | 0x44;
dsi_set_cmdq(data_array, 2, 1);
#endif

data_array[0] = 0x00110500;
dsi_set_cmdq(&data_array, 1, 1);
MDELAY(100);

data_array[0] = 0x00023902;
data_array[1] = 0xFF51;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2C53;
dsi_set_cmdq(&data_array, 2, 1);

//disable CE
data_array[0] = 0x00023902;
data_array[1] = 0x0055;  //80:enable CE
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00290500;
dsi_set_cmdq(&data_array, 1, 1);
#endif

}                                       

static void lcm_init(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(5);
	SET_RESET_PIN(1);
	MDELAY(120);

	//push_table(lcm_initialization_ret, sizeof(lcm_initialization_ret) / sizeof(struct LCM_setting_table), 1);
	lcm_init_register();
}


static void lcm_suspend(void)
{
	//push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	unsigned int data_array[16];

	data_array[0] = 0x00023902;      
	data_array[1] = 0x00000028;      //display off
	dsi_set_cmdq(&data_array, 2, 1); 
	//MDELAY(20);

	data_array[0] = 0x00023902;      
	data_array[1] = 0x00000010;      //enter sleep
	dsi_set_cmdq(&data_array, 2, 1); 
	MDELAY(150);
}


static void lcm_resume(void)
{
	lcm_init();
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


static void lcm_write_reg(unsigned int x, unsigned int y,
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

dsi_set_cmdq(data_array, 7, 0);

}


static void lcm_setbacklight(unsigned int level)
{
//unsigned int default_level = 145;
//unsigned int mapped_level = 0;

#if  0
if(level > 255) 
level = 255;

if(level >0) 
mapped_level = default_level+(level)*(255-default_level)/(255);
else
mapped_level=0;
#endif
// Refresh value of backlight level.
//lcm_backlight_level_setting[0].para_list[0] = level;

//push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
	unsigned int data_array[16];

	if(level > 255) 
	level = 255;

	data_array[0] = 0x00023902;      
	data_array[1] = (0x51|(level<<8)); 
	dsi_set_cmdq(&data_array, 2, 1); 

}

static unsigned int lcm_esd_check(void)
{
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

if(read_reg(0xB6) == 0x42)
{
return FALSE;
}
else
{ 
return TRUE;
}
#endif
}

static unsigned int lcm_esd_recover(void)
{
unsigned char para = 0;

SET_RESET_PIN(1);
SET_RESET_PIN(0);
MDELAY(5);
SET_RESET_PIN(1);
MDELAY(120);
//push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
//MDELAY(10);
//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
//MDELAY(10);
lcm_init_register();
dsi_set_cmdq_V2(0x35, 1, &para, 1); ///enable TE
MDELAY(10);

return TRUE;
}

static unsigned int lcm_compare_id(void)
{
	unsigned int ret = 0;
	
	mt_set_gpio_mode(GPIO154, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO154, GPIO_DIR_IN);
	ret = mt_get_gpio_in(GPIO154);
#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, id, GPIO154 = %d \n", __func__, ret);
#else
	printk("%s, ID, GPIO154 = %d \n", __func__, ret);
#endif	

	return (ret == 0)?1:0;

}

// ---------------------------------------------------------------------------
// Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER nt35590_hd720_dsi_cmd_yassy_lcm_drv = 
{
	.name = "nt35590_hd720_yassy",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
#if (LCM_DSI_CMD_MODE)
	.update = lcm_update,
        .set_backlight = lcm_setbacklight,
// .set_pwm = lcm_setpwm,
// .get_pwm = lcm_getpwm,
//.esd_check = lcm_esd_check,
//.esd_recover = lcm_esd_recover,
	.compare_id = lcm_compare_id,
#endif
};
