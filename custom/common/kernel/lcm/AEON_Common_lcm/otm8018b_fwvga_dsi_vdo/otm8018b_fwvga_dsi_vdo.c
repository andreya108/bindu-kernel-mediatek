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

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))




#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)            lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)       lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                      lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                  lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)


#define   LCM_DSI_CMD_MODE							0

static struct LCM_setting_table
{
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = 
{
	{0x00,	1	,{0x00}},																
	{0xff,	3	,{0x80,0x09,0x01}}, 													
	{0x00,	1	,{0x80}},																
	{0xff,	2	,{0x80,0x09}},															
	{0x00,	1	,{0x03}},																
	{0xff,	1	,{0x01}},

	{0x00,	1	,{0x00}},																
	{0xd8,	2	,{0x87,0x87}},		

	{0x00,	1	,{0xB1}},																
	{0xc5,	1	,{0xa9}},

	{0x00,	1	,{0x00}},																
	{0xd9,	1	,{0x24}},																
	{0x00,	1	,{0xb4}},																
	{0xc0,	1	,{0x50}},
													
	{0x00,	1	,{0x90}},																
	{0xc5,	3	,{0x96,0xA7,0x01}},															
	{0x00,	1	,{0x81}},																
	{0xc1,	1	,{0x66}},
											
	{0x00,	1	,{0xa1}},																
	{0xc1,	3	,{0x08,0x02,0x1B}},														
	{0x00,	1	,{0x81}},																
	{0xc4,	1	,{0x83}},

	{0x00,	1	,{0x90}},																
	{0xb3,	1	,{0x02}},

	{0x00,	1	,{0x92}},																
	{0xb3,	1	,{0x45}},

	{0x00,	1	,{0xA7}},																
	{0xb3,	1	,{0x00}},

	{0x00,	1	,{0x90}},																
	{0xc0,	6	,{0x00,0x44,0x00,0x00,0x00,0x03}},	

	{0x00,	1	,{0xa0}},																
	{0xc1,	1	,{0xea}},	

	{0x00,	1	,{0xa6}},																
	{0xc1,	4	,{0x01,0x00,0x00,0x00}},	

	{0x00,	1	,{0xC6}},																
	{0xb0,	1	,{0x03}},

	{0x00,	1	,{0x80}},																
	{0xce,	6	,{0x87,0x03,0x00,0x86,0x03,0x00}},	


	{0x00,	1	,{0x90}},																
	{0xce,	6	,{0x33,0x54,0x00,0x33,0x55,0x00}},	



	{0x00,	1	,{0xa0}},																
	{0xce,	16	,{0x38,0x03,0x03,0x58,0x00,0x00,0x00,0x38,0x02,0x03,0x59,0x00,0x00,0x00}},
			
	{0x00,	1	,{0xb0}},																
	{0xce,	16	,{0x38,0x01,0x03,0x5A,0x00,0x00,0x00,0x38,0x00,0x03,0x5B,0x00,0x00,0x00}},
			
	{0x00,	1	,{0xc0}},																
	{0xce,	16	,{0x30,0x00,0x03,0x5C,0x00,0x00,0x00,0x30,0x01,0x03,0x5D,0x00,0x00,0x00}},
			
	{0x00,	1	,{0xd0}},																
	{0xce,	16	,{0x38,0x05,0x03,0x5E,0x00,0x00,0x00,0x38,0x04,0x03,0x5F,0x00,0x00,0x00}},	



	{0x00,	1	,{0xc0}},																
	{0xcf,	12	,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x09}},	


	{0x00,	1	,{0xc0}},																
	{0xcb,	16	,{0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00}},	

	{0x00,	1	,{0xd0}},																
	{0xcb,	16	,{0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04,0x04}},	

	{0x00,	1	,{0xe0}},																
	{0xcb,	12	,{0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},	



	{0x00,	1	,{0x80}},																
	{0xcc,	12	,{0x00,0x26,0x25,0x02,0x06,0x00,0x00,0x0A,0x0E,0x0C}},	
										
	{0x00,	1	,{0x90}},																
	{0xcc,	16	,{0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x25,0x01,0x05}},	
										
	{0x00,	1	,{0xa0}},																
	{0xcc,	16	,{0x00,0x00,0x09,0x0D,0x0B,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},		

	{0x00,	1	,{0xb0}},																
	{0xcc,	12	,{0x00,0x25,0x26,0x05,0x01,0x00,0x00,0x0D,0x09,0x0B}},		

	{0x00,	1	,{0xc0}},																
	{0xcc,	16	,{0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x26,0x06,0x02}},		


	{0x00,	1	,{0xd0}},																
	{0xcc,	16	,{0x00,0x00,0x0E,0x0A,0x0C,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},		


														
	{0x00,	1	,{0x00}},																
	{0xe1,	16	,{0x00,0x05,0x0E,0x11,0x0B,0x1F,0x0E,0x0D,0x00,0x05,0x02,0x08,0x0F,0x20,0x1B,0x0B}},	
	
	{0x00,	1	,{0x00}},																
	{0xe2,	16	,{0x00,0x05,0x0D,0x11,0x0B,0x1F,0x0E,0x0E,0x00,0x04,0x02,0x07,0x0F,0x1F,0x1C,0x0B}},	
																

	{0x00,	1	,{0x00}},																
	{0xff,	3	,{0xff,0xff,0xff}}, 		
																			
																														
	{0x11,0,{								}},
	{REGFLAG_DELAY, 200, {}},
	{0x29,0,{								}},
	{REGFLAG_DELAY, 200, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_in_setting[] =
{
    // Display off sequence
    {0x28, 0, {0x00}},

    {REGFLAG_DELAY, 10, {}},
    // Sleep Mode On
    {0x10, 0, {0x00}},
	{REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] =
{
    // Sleep Out
    {0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 0, {0x00}},
    {REGFLAG_DELAY, 20, {}},

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
       	}
    }
	
}

static void init_lcm_registers(void)
{
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
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
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
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
                params->dsi.word_count=480*3;   
		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 5;
		params->dsi.vertical_backporch					= 5;
		params->dsi.vertical_frontporch					= 15;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 6;
		params->dsi.horizontal_backporch				= 56;
		params->dsi.horizontal_frontporch				= 58;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;



		//1 Every lane speed
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
		params->dsi.fbk_div =13; // 19;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

}

static void lcm_init(void)
{

    SET_RESET_PIN(1);
	MDELAY(10);	
    SET_RESET_PIN(0);
	MDELAY(20);
	
    SET_RESET_PIN(1);
	MDELAY(100);      

	init_lcm_registers();

}


static void lcm_suspend(void)
{

        push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
	SET_RESET_PIN(1);
        MDELAY(20);
	SET_RESET_PIN(0);
	MDELAY(20);
}


static void lcm_resume(void)
{
	lcm_init();
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
#define LCM_ID_OTM8018B  0x8009

static unsigned int lcm_compare_id(void)
{
#if 0
	unsigned char lcd_id = 0;
    mt_set_gpio_mode(GPIO_LCM_ID_PIN, GPIO_LCM_ID_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_LCM_ID_PIN, GPIO_DIR_IN);	
	mt_set_gpio_pull_enable(GPIO_LCM_ID_PIN,GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_LCM_ID_PIN, GPIO_PULL_UP);
	MDELAY(1);
	lcd_id =  mt_get_gpio_in(GPIO_LCM_ID_PIN);

#ifdef BUILD_LK
	printf("zhuoshineng LK otm8018b debug: otm8018b id = 0x%08x\n", __func__, lcd_id);
#else
	printk("zhuoshineng kernel otm8018b horse debug: otm8018b id = 0x%08x\n", __func__, lcd_id);
#endif

	if(lcd_id == 1)
		return 1;
	else
		return 0;
	#else
	int array[4];
	char buffer[5];
	char id_high=0;
	char id_low=0;
	int id=0;

	SET_RESET_PIN(1);
	MDELAY(20);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(120);

	array[0] = 0x00053700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xa1, buffer, 5);

	id_high = buffer[2];
	id_low = buffer[3];
	id = (id_high<<8) | id_low;

	#ifdef BUILD_LK
		printf("OTM8018B uboot %s \n", __func__);
		printf("%s leanda id = 0x%08x id_high =%x id_low =%x \n", __func__, id,id_high,id_low);
	#else
		printk("OTM8018B kernel %s \n", __func__);
	    printk("%s leanda id = 0x%08x id_high =%x id_low =%x \n", __func__, id,id_high,id_low);
	#endif

	return (LCM_ID_OTM8018B == id)?1:0;
	#endif

}
static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
//#if 1
	char  buffer[3];
	int   array[4];

	array[0] = 0x00013700;
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


LCM_DRIVER otm8018b_fwvga_dsi_vdo_lcm_drv = 
{
    .name			= "otm8018b_fwvga_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
	//.esd_check   = lcm_esd_check,
        //.esd_recover   = lcm_esd_recover,		
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};

