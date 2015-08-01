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
{0x02, 1, {0x82}},
{0x03, 1, {0x82}},
{0x04, 1, {0x82}},
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
	params->dsi.fbk_div =18; //15;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		

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

#if 1  //power
    	data_array[0] = 0x00023902;//CMD1
    	data_array[1] = 0x000000FF;
    	dsi_set_cmdq(data_array, 2, 1);     
    	
    	data_array[0] = 0x00023902;//MIPI 4 Lane
    	data_array[1] = 0x000003BA;
    	dsi_set_cmdq(data_array, 2, 1);    
    	
      data_array[0] = 0x00023902;//MIPI command mode
      data_array[1] = 0x000008C2;
    	dsi_set_cmdq(data_array, 2, 1);   
    	
      data_array[0] = 0x00023902;//CMD2,Page0
      data_array[1] = 0x000001FF;
    	dsi_set_cmdq(data_array, 2, 1);   
    	
      data_array[0] = 0x00023902; 
      data_array[1] = 0x00003A00;
    	dsi_set_cmdq(data_array, 2, 1);  
    	
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00003301;
    	dsi_set_cmdq(data_array, 2, 1);  
    	
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00005302;
    	dsi_set_cmdq(data_array, 2, 1);  
    	
    	data_array[0] = 0x00023902;//VGL=-6V 
      data_array[1] = 0x00008509;
    	dsi_set_cmdq(data_array, 2, 1);  
    	
    	data_array[0] = 0x00023902;//VGH=+8.6V 
      data_array[1] = 0x0000250E;
    	dsi_set_cmdq(data_array, 2, 1);  
    	
    	data_array[0] = 0x00023902;//turn off VGLO regulator
      data_array[1] = 0x00000A0F;
    	dsi_set_cmdq(data_array, 2, 1);  
    	
    	data_array[0] = 0x00023902;//GVDDP=4V
      data_array[1] = 0x0000970B;
    	dsi_set_cmdq(data_array, 2, 1);  
    	
    	data_array[0] = 0x00023902;//
      data_array[1] = 0x0000970C;
    	dsi_set_cmdq(data_array, 2, 1);  

    	data_array[0] = 0x00023902;//
      data_array[1] = 0x00008C11;
    	dsi_set_cmdq(data_array, 2, 1);  
    	
    	data_array[0] = 0x00023902;//   
      data_array[1] = 0x00007B36;
    	dsi_set_cmdq(data_array, 2, 1);  
    	
    	data_array[0] = 0x00023902;//GVDDP=4V     
      data_array[1] = 0x00002C71;
    	dsi_set_cmdq(data_array, 2, 1);  

    	data_array[0] = 0x00023902;
      data_array[1] = 0x000005FF;
    	dsi_set_cmdq(data_array, 2, 1);    
    	
    	data_array[0] = 0x00023902; /////////////LTPS 
      data_array[1] = 0x0001;
    	dsi_set_cmdq(data_array, 2, 1);              
    	data_array[0] = 0x00023902;
      data_array[1] = 0x8D02;
    	dsi_set_cmdq(data_array, 2, 1);              
    	data_array[0] = 0x00023902;
      data_array[1] = 0x8D03;
    	dsi_set_cmdq(data_array, 2, 1);              
    	data_array[0] = 0x00023902;
      data_array[1] = 0x8D04;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902; 
      data_array[1] = 0x3005;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;//06
      data_array[1] = 0x3306;
    	dsi_set_cmdq(data_array, 2, 1);        
      data_array[0] = 0x00023902;
      data_array[1] = 0x00007707;             
    	dsi_set_cmdq(data_array, 2, 1);        
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000008;
    	dsi_set_cmdq(data_array, 2, 1);   
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000009;
    	dsi_set_cmdq(data_array, 2, 1);   
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000000A;
    	dsi_set_cmdq(&data_array, 2, 1);   
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000800B;
    	dsi_set_cmdq(&data_array, 2, 1);
    	data_array[0] = 0x00023902;//0C 
      data_array[1] = 0x0000C80C;
      dsi_set_cmdq(&data_array, 2, 1);
      data_array[0] = 0x00023902; //0D
      data_array[1] = 0x0000000D;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00001B0E;
    	dsi_set_cmdq(&data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000070F;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;     
      data_array[1] = 0x00005710;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;     
      data_array[1] = 0x00000011;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;//12 
      data_array[1] = 0x00000012;
    	dsi_set_cmdq(data_array, 2, 1);
      data_array[0] = 0x00023902;
      data_array[1] = 0x00001E13;
    	dsi_set_cmdq(&data_array, 2, 1);       
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000014;
    	dsi_set_cmdq(data_array, 2, 1);       
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00001A15;
    	dsi_set_cmdq(data_array, 2, 1);       
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000516;
    	dsi_set_cmdq(data_array, 2, 1);       
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000017;
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;//12
      data_array[1] = 0x00001E18;
    	dsi_set_cmdq(data_array, 2, 1);
      data_array[0] = 0x00023902;
      data_array[1] = 0x0000FF19;
    	dsi_set_cmdq(data_array, 2, 1);       
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000001A;
    	dsi_set_cmdq(data_array, 2, 1);       
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000FC1B;
    	dsi_set_cmdq(data_array, 2, 1);       
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000801C;
    	dsi_set_cmdq(data_array, 2, 1);       
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000001D;
    	dsi_set_cmdq(data_array, 2, 1);  
    	   
    	data_array[0] = 0x00023902;//18         
      data_array[1] = 0x0000001E;
	dsi_set_cmdq(&data_array, 2, 1);     
			                                     
	data_array[0] = 0x00023902;
      data_array[1] = 0x0000771F;
    	dsi_set_cmdq(data_array, 2, 1);     
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000020;
    	dsi_set_cmdq(data_array, 2, 1);     
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000021;
    	dsi_set_cmdq(data_array, 2, 1);     
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00005522;
    	dsi_set_cmdq(data_array, 2, 1);      
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000D23;
    	dsi_set_cmdq(data_array, 2, 1);        
    	data_array[0] = 0x00023902;//06 
      data_array[1] = 0x0000A031;
    	dsi_set_cmdq(data_array, 2, 1);
      data_array[0] = 0x00023902;
      data_array[1] = 0x00000032;
    	dsi_set_cmdq(&data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000B833;
    	dsi_set_cmdq(data_array, 2, 1);    
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000BB34;
    	dsi_set_cmdq(data_array, 2, 1);        
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00001135;
    	dsi_set_cmdq(data_array, 2, 1);        
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000136;
    	dsi_set_cmdq(data_array, 2, 1);        
    	data_array[0] = 0x00023902;//0C
      data_array[1] = 0x00000B37;
      dsi_set_cmdq(data_array, 2, 1);        
      data_array[0] = 0x00023902; //0D
      data_array[1] = 0x00000138;
    	dsi_set_cmdq(data_array, 2, 1);        
    	data_array[0] = 0x00023902;             
      data_array[1] = 0x00000B39;             
    	dsi_set_cmdq(data_array, 2, 1);        
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000844;
    	dsi_set_cmdq(data_array, 2, 1);        
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00008045;
    	dsi_set_cmdq(data_array, 2, 1);        
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000CC46;
    	dsi_set_cmdq(data_array, 2, 1);           
    	data_array[0] = 0x00023902;//12            
      data_array[1] = 0x00000447;
    	dsi_set_cmdq(data_array, 2, 1);           
      data_array[0] = 0x00023902;
      data_array[1] = 0x00000048;
    	dsi_set_cmdq(data_array, 2, 1);                     
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000049;
    	dsi_set_cmdq(data_array, 2, 1);                            
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000014A;
    	dsi_set_cmdq(&data_array, 2, 1);                            
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000036C;
    	dsi_set_cmdq(&data_array, 2, 1);                            
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000036D;
    	dsi_set_cmdq(data_array, 2, 1);                            
    	data_array[0] = 0x00023902;//18                            
      data_array[1] = 0x00002F6E;
	dsi_set_cmdq(&data_array, 2, 1); 		
			
    	data_array[0] = 0x00023902; ////
      data_array[1] = 0x00000043;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000234B;
    	dsi_set_cmdq(&data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000014C;
    	dsi_set_cmdq(&data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00002350;
    	dsi_set_cmdq(data_array, 2, 1); 
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000151;
    	dsi_set_cmdq(data_array, 2, 1); 
    	data_array[0] = 0x00023902;//06  
      data_array[1] = 0x00002358;
    	dsi_set_cmdq(data_array, 2, 1); 
      data_array[0] = 0x00023902;
      data_array[1] = 0x00000159;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000235D;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000015E;
    	dsi_set_cmdq(&data_array, 2, 1);
    	data_array[0] = 0x00023902;     
      data_array[1] = 0x00000089;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;     
      data_array[1] = 0x0000018D;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000648E;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x0000208F;
    	dsi_set_cmdq(data_array, 2, 1);                  
    	data_array[0] = 0x00023902;//12
      data_array[1] = 0x00008E97;
    	dsi_set_cmdq(data_array, 2, 1);                  
      data_array[0] = 0x00023902;
      data_array[1] = 0x00008C82;
    	dsi_set_cmdq(data_array, 2, 1);                            
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000283;
    	dsi_set_cmdq(data_array, 2, 1);                            
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000ABB;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00000ABC;
    	dsi_set_cmdq(&data_array, 2, 1);                            
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00002524;
    	dsi_set_cmdq(data_array, 2, 1);                            
    	data_array[0] = 0x00023902;//18
      data_array[1] = 0x00005525;
	dsi_set_cmdq(&data_array, 2, 1); 	
			
	data_array[0] = 0x00023902;
      data_array[1] = 0x00000526;
    	dsi_set_cmdq(data_array, 2, 1); 
    	data_array[0] = 0x00023902;
      data_array[1] = 0x00002327;
    	dsi_set_cmdq(data_array, 2, 1); 
    	data_array[0] = 0x00023902;      
      data_array[1] = 0x00000128;
    	dsi_set_cmdq(data_array, 2, 1); 
    	data_array[0] = 0x00023902;      
      data_array[1] = 0x00003129;      
    	dsi_set_cmdq(data_array, 2, 1); 
    	data_array[0] = 0x00023902;      
      data_array[1] = 0x00005D2A;
    	dsi_set_cmdq(data_array, 2, 1); 
    	data_array[0] = 0x00023902;//06 
      data_array[1] = 0x0000012B;
    	dsi_set_cmdq(data_array, 2, 1);
      data_array[0] = 0x00023902;     
      data_array[1] = 0x0000002F;
    	dsi_set_cmdq(data_array, 2, 1);
    	data_array[0] = 0x00023902;     
      data_array[1] = 0x00001030;          
    	dsi_set_cmdq(data_array, 2, 1);     
    	data_array[0] = 0x00023902;             
      data_array[1] = 0x000012A7;             
    	dsi_set_cmdq(data_array, 2, 1);        
    	data_array[0] = 0x00023902;             
      data_array[1] = 0x0000032D;
    	dsi_set_cmdq(data_array, 2, 1);
#endif

#if 1 //gamma
//CMI 20130301, gamma settings
// CMD2, page 0
data_array[0] = 0x00023902;
data_array[1] = 0x01ff;
dsi_set_cmdq(&data_array, 2, 1);

//R+
data_array[0] = 0x00023902;
data_array[1] = 0x0075; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xc976; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0077; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd378; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0079; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe27A;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007B; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xef7C; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007D; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf97E; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x017F;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0380; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0181; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0e82; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0183; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1884;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0185; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2186; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0187; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4188; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0189; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5f8A; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x018B; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8e8C;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x018D; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xb98E; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x018F; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfe90; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0291;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3b92; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0293; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3d94; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0295; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x7596;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0297; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xae98; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0299; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd89A; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039B; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0f9C;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039D; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x299E; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039F; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4fA0; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A2; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x55A3; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A4; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x61A5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A6;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6cA7; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A9; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x79AA; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03AB; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x7cAC;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03AD; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x80AE; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03AF; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8fB0; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03B1;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x95B2; 
dsi_set_cmdq(&data_array, 2, 1);

//R-
data_array[0] = 0x00023902;
data_array[1] = 0x00B3; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xc9B4; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd3B6; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B7; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe2B8;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B9; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xefBA; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00BB; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf9BC; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01BD;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03BE; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01BF; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0eC0; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C1; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x18C2;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C3; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x21C4; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x41C6; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C7; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5fC8; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C9; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8eCA;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01CB; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xb9CC; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01CD; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfeCE; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02CF;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3bD0; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D1; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3dD2; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D3; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x75D4;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xaeD6; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D7; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd8D8; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03D9; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0fDA;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DB; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x29DC; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DD; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4fDE; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DF; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x55E0; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E1; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x61E2; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E3;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6cE4; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x79E6; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E7; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x7cE8;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E9; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x80EA; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03EB; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8fEC; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03ED;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x95EE; 
dsi_set_cmdq(&data_array, 2, 1);

//G+
data_array[0] = 0x00023902;
data_array[1] = 0x00EF; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8fF0;     
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00F1; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x97F2;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00F3; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xacF4;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00F5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xbdF6;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00F7; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xcaF8;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00F9;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd5FA;   
dsi_set_cmdq(&data_array, 2, 1);

// CMD2, page 1
data_array[0] = 0x00023902;
data_array[1] = 0x02ff; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0000; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe301;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0002; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xec03;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0004; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf805;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0106; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1f07;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0108; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4009;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x010A; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x780B;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x010C; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xa40D;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x010E; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf10F;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0210;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2e11;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0212; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3013;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0214; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6915;     
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0216; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xa517;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0218; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xcf19;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x031A; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x081B;     
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x031C; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x281D;     
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x031E; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x501F;     
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0320; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5b21;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0322; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6723;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0324;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x7725;     
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0326; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8027;     
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0328; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9229;      
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x032A; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf12B;      
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x032D; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfa2F;      
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0330;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfb31;     
dsi_set_cmdq(&data_array, 2, 1);

//G-
data_array[0] = 0x00023902;
data_array[1] = 0x0032; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8f33;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0034; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9735;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0036; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xac37;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0038; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xbd39;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x003A; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xca3B;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x003D;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xd53F;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0040; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xe341;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0042; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xec43;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0044; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf845;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0146; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1f47;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0148; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4049;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x014A; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x784B;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x014C; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xa44D;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x014E; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf14F;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0250;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2e51;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0252; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x3053;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0254; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6955;     
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0256; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xa558;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0259; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xcf5A;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x035B; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x085C;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x035D; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x285E;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x035F; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5060;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0361; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5b62;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0363; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6764;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0365;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x7766;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0367; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8068;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0369; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x926A;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x036B; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xf16C;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x036D; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfa6E;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x036F;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xfb70;   
dsi_set_cmdq(&data_array, 2, 1);

//B+

data_array[0] = 0x00023902;
data_array[1] = 0x0071; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0A72; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0073; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2074; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0075; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4D76;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0077; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6678; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0079; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x807A; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007B;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x937C; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007D; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xA77E; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x007F; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xB880;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0081; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xC882; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0083; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xFC84; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0185; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x2586; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0187; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x6088;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0189; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x928A; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x018B; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xDE8C; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x028D;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1A8E; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x028F; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1C90; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0291; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x5292;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0293; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8B94; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0295; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xB096; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0297; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xE198;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0399; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x059A; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039B; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x359C; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039D; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x449E; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x039F; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x53A0; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A2;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x63A3; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A4; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x74A5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A6; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x86A7;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03A9; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9AAA; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03AB; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xAFAC; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03AD;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xB9AE; 
dsi_set_cmdq(&data_array, 2, 1);

//B-
data_array[0] = 0x00023902;
data_array[1] = 0x00AF; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x0AB0; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B1; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x20B2; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B3; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x4DB4;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x66B6; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B7; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x80B8; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00B9;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x93BA; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00BB; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xA7BC; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00BD; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xB8BE;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00BF; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xC8C0; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x00C1; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xFCC2; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C3; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x25C4; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x60C6;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C7; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x92C8; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x01C9; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xDECA; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02CB;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1ACC; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02CD; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x1CCE; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02CF; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x52D0;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D1; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x8BD2; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D3; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xB0D4; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x02D5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xE1D6;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03D7; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x05D8; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03D9; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x35DA; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DB; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x44DC; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DD; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x53DE; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03DF;  
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x63E0; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E1; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x74E2;
dsi_set_cmdq(&data_array, 2, 1);
 
data_array[0] = 0x00023902;
data_array[1] = 0x03E3; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x86E4;   
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E5; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x9AE6; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E7; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xAFE8; 
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x03E9;    
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0xB9EA;   
dsi_set_cmdq(&data_array, 2, 1);
#endif

#if 1
    	data_array[0] = 0x00023902;////CMD1 
      data_array[1] = 0x000000FF;                 
    	dsi_set_cmdq(data_array, 2, 1);    
    	
    	data_array[0] = 0x00023902;
      data_array[1] = 0x000001FB;                 
    	dsi_set_cmdq(data_array, 2, 1);    
    	
    	data_array[0] = 0x00023902;//CMD2,Page0 
      data_array[1] = 0x000001FF;                 
    	dsi_set_cmdq(data_array, 2, 1);       
    	
    	data_array[0] = 0x00023902;
      data_array[1] = 0x000001FB;                 
    	dsi_set_cmdq(data_array, 2, 1);   
    	
    	data_array[0] = 0x00023902;//CMD2,Page1 
      data_array[1] = 0x000002FF;                 
    	dsi_set_cmdq(data_array, 2, 1);       
    	
    	data_array[0] = 0x00023902;
      data_array[1] = 0x000001FB;                 
    	dsi_set_cmdq(data_array, 2, 1);       
    	
    	data_array[0] = 0x00023902;//CMD2,Page2 
      data_array[1] = 0x000003FF;                 
    	dsi_set_cmdq(data_array, 2, 1);       
    	
    	data_array[0] = 0x00023902;
      data_array[1] = 0x000001FB;                 
    	dsi_set_cmdq(data_array, 2, 1);     

    	data_array[0] = 0x00023902;//CMD2,Page3
      data_array[1] = 0x000004FF;         
    	dsi_set_cmdq(data_array, 2, 1);    
    	                                    
    	data_array[0] = 0x00023902;         
      data_array[1] = 0x000001FB;         
    	dsi_set_cmdq(data_array, 2, 1);    
    	
    	data_array[0] = 0x00023902;//CMD2,Page4
      data_array[1] = 0x000005FF;         
    	dsi_set_cmdq(data_array, 2, 1);    
    	                                    
    	data_array[0] = 0x00023902;         
      data_array[1] = 0x000001FB;         
    	dsi_set_cmdq(data_array, 2, 1); 
    	

#if 0 //color enhance
//¶i\u20acJCMD 2,PAGE2		
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

    	data_array[0] = 0x00023902;     ////CMD1     
      data_array[1] = 0x000000FF;         
    	dsi_set_cmdq(data_array, 2, 1);  

#if 1 //TE
    	data_array[0] = 0x00023902;                  
    	data_array[1] = 0x0035;                 
    	dsi_set_cmdq(&data_array, 2, 1);    
#endif
   	
     	data_array[0] = 0x00110500;                
    	dsi_set_cmdq(data_array, 1, 1); 
    	MDELAY(100); 
    	
	data_array[0] = 0x00023902;
      data_array[1] = 0x0000FF51;                 
    	dsi_set_cmdq(&data_array, 2, 1);    
    	
	data_array[0] = 0x00023902; 
      data_array[1] = 0x00002C53;                 
    	dsi_set_cmdq(&data_array, 2, 1);    
    	
	data_array[0] = 0x00023902; 
      data_array[1] = 0x00000055;  //80:enable CE               
    	dsi_set_cmdq(&data_array, 2, 1);    
    	
    	data_array[0] = 0x00290500;                
    	dsi_set_cmdq(data_array, 1, 1);    
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

	return (ret == 1)?1:0;

}

// ---------------------------------------------------------------------------
// Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER nt35590_hd720_dsi_cmd_cmi_lcm_drv = 
{
	.name = "nt35590_hd720_cmi",
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
