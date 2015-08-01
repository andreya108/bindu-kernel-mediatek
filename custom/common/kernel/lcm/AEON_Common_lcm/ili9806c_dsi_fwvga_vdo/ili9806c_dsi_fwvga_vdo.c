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

	{0xFF,3,{0xFF,0x98,0x16}},

	{0xBA,1,{0x60}},
	
	{0xB0,1,{0x01}},
	
	{0XBC,18,{0x03,0x0D,0x61,0x69,0x16,0x16,0x1B,0x11,0x70,0x00,0x00,0x00,0x16,0x16,0x09,0x00,0xFF,0xF0}},

	{0XBD,8,{0x01,0x45,0x45,0x67,0x01,0x23,0x45,0x67}},

	{0XBE,17,{0x13,0x22,0x11,0x00,0x66,0x77,0x22,0x22,0xBA,0xDC,0xCB,0xAD,0x22,0x22,0x22,0x22,0x22}},
	
	{0xED,2,{0x7F,0x7F}},

	{0xB4,1,{0x00}}, 
	
	{0xC0,3,{0x0F,0x0B,0x0A}},
	
	{0xC1,4,{0x17,0x8E,0x87,0x20}},	
	
	{0xD8,1,{0x50}},
	
	{0xF3,1,{0x70}},
	
	{0xFC,1,{0x07}},
	
	{0XE0,16,{0x00,0x17,0x24,0x10,0x12,0x18,0xCC,0x09,0x00,0x08,0x04,0x0C,0x0E,0x2D,0x2A,0x00}}, 
	
        {0XE1,16,{0x00,0x0D,0x1A,0x11,0x12,0x16,0x76,0x06,0x06,0x09,0x07,0x0C,0x0B,0x26,0x23,0x00}}, 	
	
        {0XD5,8,{0x0F,0x09,0x08,0x0A,0xCB,0xA5,0x01,0x04}}, 	
	
	//{0xF3,1,{0x70}},
		
	{0x36,1,{0x00}},	
		
	{0x35,1,{0x00}},		
	    
        {0xF7,1,{0x89}},
	
	{0xC7,1,{0x44}},
	
     	{0X11,0,{}},
     	{REGFLAG_DELAY, 150, {}},
        {0xEE,9,{0x0A, 0x1B, 0x5F, 0x40, 0x00, 0x00, 0x10, 0x00, 0x58}},
        {0xD6,8,{0xFF,0xA0,0x88,0x14,0x04,0x64,0x28,0x3A}},
        
	{0X29,0,{}},
	{REGFLAG_DELAY, 50, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}	

    

};


static struct LCM_setting_table lcm_sleep_in_setting[] =
{
    // Display off sequence
    {0x28, 0, {0x00}},

    {REGFLAG_DELAY, 10, {}},
    // Sleep Mode On
    {0x10, 0, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] =
{
    // Sleep Out
    {0x11, 0, {0x00}},
    {REGFLAG_DELAY, 170, {}},
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
		//params->dsi.compatibility_for_nvk = 0;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 5;
		params->dsi.vertical_backporch					= 15;
		params->dsi.vertical_frontporch					= 15;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 6;
		params->dsi.horizontal_backporch				= 26;
		params->dsi.horizontal_frontporch				= 28;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		//1 Every lane speed
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
		params->dsi.fbk_div =14; // 19;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

}

static void lcm_init(void)
{

    SET_RESET_PIN(1);
	MDELAY(10);	
    SET_RESET_PIN(0);
	MDELAY(20);
	
    SET_RESET_PIN(1);
	MDELAY(120);      

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
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(5);

	SET_RESET_PIN(1);
	MDELAY(120); 	 
     // lcm_initialization_setting[19].para_list[0] +=1;	
	init_lcm_registers();
    
    //push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
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

static struct LCM_setting_table lcm_compare_id_setting[] = {
        {0xFF, 3 ,{0xFF,0x98,0x06}},
        {REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
//      {0xC3, 1, {0xFF}},

        {REGFLAG_END_OF_TABLE, 0x00, {}}
};


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
	printf("thl_ykl:%s, LK ili9806c_dsi_fwvga_cmd debug: ili9806c_dsi_fwvga_cmd id = 0x%08x\n", __func__, lcd_id);
#else
	printk("thl_ykl:%s, kernel ili9806c_dsi_fwvga_cmd horse debug: ili9806c_dsi_fwvga_cmd id = 0x%08x\n", __func__, lcd_id);
#endif

	if(lcd_id)
		return 1;
	else
		return 0;
	#else
	volatile unsigned char lcd_id = 4;
	volatile unsigned int id = 0;
	volatile unsigned char buffer[4];
	unsigned int data_array[16];
	
	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(2);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(50); 

	push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);
	data_array[0] = 0x00043700;
    dsi_set_cmdq(data_array, 1, 1);

	//data_array[0] = 0x00023700;// read id return two byte,version and id
	//dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10); 
       memset(buffer,0x0,4);
	read_reg_v2(0xD3, buffer, 4);  //Read  2 bytes 5510 means NT35510 from IC Spec
	#if defined(BUILD_LK)
	printf("### leanda   ili9806 lcm_compare_id buffer[0] = 0x%x,0x%x,0x%x,0x%x\r\n",buffer[0], buffer[1], buffer[2], buffer[3]);
	#endif
	id = buffer[1]<<8 | buffer[2];
	//lcd_id =  mt_get_gpio_in(GPIO_LCD_ID);
	//printk("### zhaoshaopeng  DC lcm_compare_id id=0x%x,lcd_id=%d\r\n",id,lcd_id);
//	printk("### leanda lide lcd_id = %d \r\n",lcd_id);
//	if((buffer[0]==0x80))
	if(id == 0x9816)//zhaoshaopeng from 9806
        return 1;
        else
        return 0;
	#endif

}
static unsigned int lcm_esd_check(void)
{

	char  buffer[4];
	int   array[4];
	char  buffer_2[1];
	array[0] = 0x00043700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x09, buffer, 4);
	read_reg_v2(0x0a, buffer_2, 1);
#if defined(BUILD_LK)
    printf("[ili9806C] buffer[0] = %x; buffer[1]= %x; buffer[2]= %x;  buffer_2[0] = %x;\n",buffer[0],buffer[1],buffer[2], buffer_2[0]);
#elif defined(BUILD_UBOOT)
    printf("[ili9806C] buffer[0] = %x; buffer[1]= %x; buffer[2]= %x;  buffer_2[0] = %x;\n",buffer[0],buffer[1],buffer[2], buffer_2[0]);
#else
    printk("[ili9806C] buffer[0] = %x; buffer[1]= %x; buffer[2]= %x;  buffer_2[0] = %x;\n",buffer[0],buffer[1],buffer[2], buffer_2[0]);
#endif		
	if((0x80 == buffer[0]) && (0x73 == buffer[1]) && (0x06 ==buffer[2]) && (0x9C ==buffer_2[0]))
	{
		return 0;
	}
	else
	{	
		return 1;
	}

}



static unsigned int lcm_esd_recover(void)
{
	
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(100); 	
	init_lcm_registers();
	return 1;
}

LCM_DRIVER ili9806c_dsi_fwvga_vdo_lcm_drv = 
{
    .name			= "ili9806c_dsi_fwvga_vdo",
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
