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

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (854)

#ifndef TRUE
    #define   TRUE     1
#endif
 
#ifndef FALSE
    #define   FALSE    0
#endif

#define REGFLAG_DELAY                                       0XFFE
#define REGFLAG_END_OF_TABLE                                0xFFF   // END OF REGISTERS MARKER


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
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)         


static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
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
	

	{0xB9,	3,	{0xFF, 0x83, 0x79}},

	
	{0xBA,	2,	{0x51,0x93}},

	{0xB1,	19,	{0x00,0x50,0x44,0xEA,0x8D,0x08,0x11,0x11,0x11,0x27,0x2F,0x9A,0x1A,0x42,0x0B,0x6E,0xF1,0x00,0xE6}},



	{0xB2,	13,	{0x00, 0x00, 0xFE, 0x08,0x04, 0x19, 0x22,0x00, 0xFF, 0x08, 0x04,0x19, 0x20}},

	{0xB4,	31,	{0x82,0x08,0x00,0x32,0x10,0x03,0x32,0x13,0x70,0x32,0x10,0x08,0x37,0x01,0x28,0x06,0x37,0x02,0x54,0x08,0x5C,0x5E,0x08,0x00,0x40,0x08,0x28,0x08,0x30,0x30,0x04}},


	{0xD5,    47,     {0x00,0x00,0x0a,0x00,0x01,0x05,0x00,0x03,0x00,0x88,0x88,0x88,0x88,0x23,0x01,0x67,0x45,0x02,0x13,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x54,0x76,0x10,0x32,0x31,0x20,0x88,0x88,0x88,0x88,0x88,0x88,0x00,0x00,0x00,0x00,0x00,0x00,}},	


	// SET GAMMA
	{0xE0,	35,	{0x79,0x00,0x04,0x0E,0x23,0x25,0x3F,0x2C,0x44,0x07,0x0E,0x10,0x14,0x17,0x15,0x16,0x11,0x17,0x00,0x04,0x0E,0x23,0x25,0x3F,0x2C,0x44,0x07,0x0E,0x10,0x14,0x17,0x15,0x16,0x11,0x17,}},

	{0xCC,1,{0x02}},

	{0xB6,4,{0x00,0x96,0x00,0x96}},

	{0x3A,1,{0x77}},
		
	{0x35,1,{0x00}},
                                     
	{0x11,0,{0x00}}, //Tearing Effect On        Sleep Out
	{REGFLAG_DELAY, 150, {}},                            
	                                                     
	{0x29,0,{0x00}}, //Display On   Display On.            		
	{REGFLAG_DELAY, 40, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void lcm_init_register(void)
{
        unsigned int data_array[16];	


	 data_array[0] = 0x00043902;                          
        data_array[1] = 0x7983FFB9;
        dsi_set_cmdq(&data_array, 2, 1);
       // MDELAY(1);  
	
	data_array[0] = 0x00033902;                          
       data_array[1] = 0x009351BA; //009201ba
        dsi_set_cmdq(&data_array, 2, 1);
      //  MDELAY(1);  
      

	data_array[0] = 0x00143902;                          
       data_array[1] = 0x445000B1;
	data_array[2] = 0x11088dea; 
	data_array[3] = 0x2f271111; 
	data_array[4] = 0x0b421a9a; 
	data_array[5] = 0xE600f16e;   			       
        dsi_set_cmdq(&data_array, 6, 1);	
      //  MDELAY(1);  



	data_array[0] = 0x000e3902;                          
       data_array[1] = 0xfe0000B2;
	data_array[2] = 0x22190408;
       data_array[3] = 0x0408ff00;
	data_array[4] = 0x00002019;	
        dsi_set_cmdq(&data_array, 5, 1);
        MDELAY(1);  		

	data_array[0] = 0x00203902;                          
       data_array[1] = 0x000882B4;
	data_array[2] = 0x32031032; 
	data_array[3] = 0x10327013; 
	data_array[4] = 0x28013708; 
	data_array[5] = 0x54023706;
	data_array[6] = 0x085e5c08;
	data_array[7] = 0x28084000;
	data_array[8] = 0x04303008;   		
        dsi_set_cmdq(&data_array,9, 1);	
        MDELAY(1);  

	
	data_array[0] = 0x00303902;                          
       data_array[1] = 0x0a0000D5;
	data_array[2] = 0x00050100; 
	data_array[3] = 0x88880003; 
	data_array[4] = 0x01238888; 
	data_array[5] = 0x13024567;
	data_array[6] = 0x88888888; 
       data_array[7] = 0x88888888;
	data_array[8] = 0x76548888; 
	data_array[9] = 0x20313210; 
	data_array[10] = 0x88888888; 
	data_array[11] = 0x00008888;
	data_array[12] = 0x00000000; 

        dsi_set_cmdq(&data_array, 13, 1);	
        MDELAY(1);  


	data_array[0] = 0x00243902;                          
        data_array[1] = 0x040079E0;
	data_array[2] = 0x3f25230e; 
	data_array[3] = 0x0e07442c; 
	data_array[4] = 0x15171410; 
	data_array[5] = 0x00171116;
	data_array[6] = 0x25230e04; 
        data_array[7] = 0x07442c3f;
	data_array[8] = 0x1714100e; 
	data_array[9] = 0x17111615; 
        dsi_set_cmdq(&data_array, 10, 1);	
        MDELAY(1);  


	data_array[0] = 0x00023902;                          
        data_array[1] = 0x000002CC; // 02 cc
        dsi_set_cmdq(&data_array, 2, 1);
        MDELAY(1);  
		
	  
	data_array[0] = 0x00053902;                          
        data_array[1] = 0x009600B6;
	data_array[2] = 0x00000096;
        dsi_set_cmdq(&data_array, 3, 1);
        MDELAY(1);  
	                     
	data_array[0] = 0x00110500; // Sleep Out
	dsi_set_cmdq(&data_array, 1, 1);

	MDELAY(150);

	data_array[0] = 0x00290500; // Display On
	dsi_set_cmdq(&data_array, 1, 1); 
        MDELAY(10);  		

}



static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_in_setting[] = {
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
             //   dsi_set_cmdq_dcs(cmd, table[i].count, table[i].para_list, force_update);
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
              params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
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
	           params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	    params->dsi.vertical_sync_active     = 5;
	    params->dsi.vertical_backporch   = 5;
	    params->dsi.vertical_frontporch  = 5;
			params->dsi.vertical_active_line				= FRAME_HEIGHT; 
	
	    params->dsi.horizontal_sync_active   = 12;
	    params->dsi.horizontal_backporch     = 208;
	    params->dsi.horizontal_frontporch    = 208;
			params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	  //  params->dsi.compatibility_for_nvk = 0;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's

        //	params->dsi.HS_PRPR=6;
	   //params->dsi.LPX=4; 
		//params->dsi.HS_PRPR=5;
		//params->dsi.HS_TRAIL=13;
	//	params->dsi.CLK_TRAIL = 10;
		// Bit rate calculation
		//1 Every lane speed
	//	params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	//	params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
	//	params->dsi.fbk_div = 19;//16;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

		params->dsi.pll_div1=	1;//22	// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps	
		params->dsi.pll_div2=	1;	// 1	// div2=0,1,2,3;div1_real=1,2,4,4		
		params->dsi.fbk_div =30;   // 20

			//	params->dsi.noncont_clock = TRUE;		
			//	params->dsi.noncont_clock_period = 2;
		

}

static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  
	unsigned char lcd_id = 0;

	    SET_RESET_PIN(1);
	    SET_RESET_PIN(0);
	    MDELAY(1);
	    SET_RESET_PIN(1);
	    MDELAY(10);//Must over 6 ms

	array[0]=0x00043902;
	array[1]=0x7983FFB9;// page enable
	dsi_set_cmdq(&array, 2, 1);
	MDELAY(10);

	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(&array, 1, 1);
    	MDELAY(10);

	read_reg_v2(0xf4, buffer, 2);
	id = buffer[0]; 
	lcd_id =  mt_get_gpio_in(GPIO_LCM_ID_PIN);	
	
#ifdef BUILD_LK
	printf("zhuoshineng LK HX8379 debug: HX8369 lcd_id = %d\n",lcd_id);
	printf("zhuoshineng LK HX8379 debug: HX8369 id = %x\n",id);

#else
	printk("zhuoshineng kernel HX8379  debug: HX8369 lcd_id= %d\n", lcd_id);
	printk("zhuoshineng kernel HX8379  debug: HX8369 id = %x\n", id);
#endif


	if(1 == lcd_id)
		return 1;
	else
		return 0;
}


static void lcm_init(void)
{
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(20);//Must over 6 ms,SPEC request

	lcm_init_register();

}


static void lcm_suspend(void)
{
           unsigned int data_array[16];	   

        SET_RESET_PIN(1);
        MDELAY(10);       	
        SET_RESET_PIN(0);	
        MDELAY(10);
        SET_RESET_PIN(1);
	MDELAY(120);   
	
}


static void lcm_resume(void)
{
/*
        unsigned int data_array[16];	
   	data_array[0] = 0x00110500; // Sleep Out
	dsi_set_cmdq(&data_array, 1, 1);

	MDELAY(120);

	data_array[0] = 0x00290500; // Display On
	dsi_set_cmdq(&data_array, 1, 1); 
        MDELAY(20);  
*/


    SET_RESET_PIN(1);
    MDELAY(10);    
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(20); 
     	lcm_init_register();
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
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

}
#endif

#if 1
static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
//#if 1
	char  buffer[5];
	int   array[4];

	array[0] = 0x00043700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x09, buffer, 4);
	printk("zhuoshineng test lcm_esd_check buffer[0]=0x%x\n",buffer[0]);
	printk("zhuoshineng test lcm_esd_check buffer[1]=0x%x\n",buffer[1]);
	printk("zhuoshineng test lcm_esd_check buffer[2]=0x%x\n",buffer[2]);
	printk("zhuoshineng test lcm_esd_check buffer[3]=0x%x\n",buffer[3]);			
	if((buffer[0]==0x80)&&(buffer[1]==0x73))
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
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(100); 	
	lcm_init_register();
#endif
	return TRUE;
}
#endif

LCM_DRIVER hx8379_fwvga_dsi_vdo_lcm_drv = 
{
    .name			= "hx8379_fwvga_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
	.esd_check   = lcm_esd_check,
        .esd_recover   = lcm_esd_recover,	
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};
