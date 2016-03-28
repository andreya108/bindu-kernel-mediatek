#include <lcm_drv.h>
#ifdef BUILD_LK
#include <platform/disp_drv_platform.h>
#else
#include <linux/delay.h>
#include <mach/mt_gpio.h>
#endif
#include <cust_gpio_usage.h>
//used to identify float ID PIN status
#define LCD_HW_ID_STATUS_LOW      0
#define LCD_HW_ID_STATUS_HIGH     1
#define LCD_HW_ID_STATUS_FLOAT 0x02
#define LCD_HW_ID_STATUS_ERROR  0x03

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif
extern LCM_DRIVER otm1282a_hd720_dsi_vdo_lcm_drv;
extern LCM_DRIVER r63319_wqhd_dsi_vdo_truly_lcm_drv;
extern LCM_DRIVER nt35598_wqhd_dsi_vdo_truly_lcm_drv;
extern LCM_DRIVER lp079x01_lcm_drv;
extern LCM_DRIVER hx8369_lcm_drv;
extern LCM_DRIVER hx8369_6575_lcm_drv;
extern LCM_DRIVER hx8363_6575_dsi_lcm_drv;
extern LCM_DRIVER hx8363_6575_dsi_hvga_lcm_drv;
extern LCM_DRIVER hx8363_6575_dsi_qvga_lcm_drv;
extern LCM_DRIVER hx8363b_wvga_dsi_cmd_drv;
extern LCM_DRIVER bm8578_lcm_drv;
extern LCM_DRIVER nt35582_mcu_lcm_drv;
extern LCM_DRIVER nt35582_mcu_6575_lcm_drv;
extern LCM_DRIVER nt35582_rgb_6575_lcm_drv;
extern LCM_DRIVER hx8357b_lcm_drv;
extern LCM_DRIVER hx8357c_hvga_dsi_cmd_drv;
extern LCM_DRIVER hx8369_dsi_lcm_drv;
extern LCM_DRIVER hx8369_dsi_6575_lcm_drv;
extern LCM_DRIVER hx8369_dsi_6575_hvga_lcm_drv;
extern LCM_DRIVER hx8369_dsi_6575_qvga_lcm_drv;
extern LCM_DRIVER hx8369_dsi_vdo_lcm_drv;
extern LCM_DRIVER hx8369b_dsi_vdo_lcm_drv;
extern LCM_DRIVER hx8369b_wvga_dsi_vdo_drv;
extern LCM_DRIVER hx8389b_qhd_dsi_vdo_drv;
extern LCM_DRIVER hx8369_hvga_lcm_drv;
extern LCM_DRIVER ili9481_lcm_drv;
extern LCM_DRIVER nt35582_lcm_drv;
extern LCM_DRIVER s6d0170_lcm_drv;
extern LCM_DRIVER spfd5461a_lcm_drv;
extern LCM_DRIVER ta7601_lcm_drv;
extern LCM_DRIVER tft1p3037_lcm_drv;
extern LCM_DRIVER ha5266_lcm_drv;
extern LCM_DRIVER hsd070idw1_lcm_drv;
extern LCM_DRIVER lg4571_lcm_drv;
extern LCM_DRIVER lg4573b_wvga_dsi_vdo_lh430mv1_drv;
extern LCM_DRIVER lvds_wsvga_lcm_drv;
extern LCM_DRIVER lvds_wsvga_ti_lcm_drv;
extern LCM_DRIVER lvds_wsvga_ti_n_lcm_drv;
extern LCM_DRIVER nt35565_3d_lcm_drv;
extern LCM_DRIVER tm070ddh03_lcm_drv;
extern LCM_DRIVER r61408_lcm_drv;
extern LCM_DRIVER r61408_wvga_dsi_cmd_drv;
extern LCM_DRIVER nt35510_lcm_drv;
extern LCM_DRIVER nt35510_dpi_lcm_drv;
extern LCM_DRIVER nt35510_hvga_lcm_drv;
extern LCM_DRIVER nt35510_qvga_lcm_drv;
extern LCM_DRIVER nt35510_wvga_dsi_cmd_drv;
extern LCM_DRIVER nt35510_6517_lcm_drv;
extern LCM_DRIVER nt35510_dsi_cmd_6572_drv;
extern LCM_DRIVER nt35510_dsi_cmd_6572_hvga_drv;
extern LCM_DRIVER nt35510_dsi_cmd_6572_fwvga_drv;
extern LCM_DRIVER nt35510_dsi_cmd_6572_qvga_drv;
extern LCM_DRIVER nt35510_dsi_vdo_6572_drv;
extern LCM_DRIVER nt35510_dpi_6572_lcm_drv;
extern LCM_DRIVER nt35510_mcu_6572_lcm_drv;
extern LCM_DRIVER nt51012_hd720_dsi_vdo_lcm_drv;
extern LCM_DRIVER r63303_idisplay_lcm_drv;
extern LCM_DRIVER hj080ia_lcm_drv;
extern LCM_DRIVER hj101na02a_lcm_drv;
extern LCM_DRIVER hj101na02a_8135_lcm_drv;
extern LCM_DRIVER hsd070pfw3_lcm_drv;
extern LCM_DRIVER hsd070pfw3_8135_lcm_drv;
extern LCM_DRIVER cm_n070ice_dsi_vdo_lcm_drv;
extern LCM_DRIVER ej101ia_lcm_drv;
extern LCM_DRIVER scf0700m48ggu02_lcm_drv;
extern LCM_DRIVER nt35510_fwvga_lcm_drv;
#if defined(GN_SSD2825_SMD_S6E8AA)
extern LCM_DRIVER gn_ssd2825_smd_s6e8aa;
#endif
extern LCM_DRIVER nt35517_dsi_vdo_lcm_drv;
extern LCM_DRIVER hx8369_dsi_bld_lcm_drv;
extern LCM_DRIVER hx8369_dsi_tm_lcm_drv;
extern LCM_DRIVER otm1280a_hd720_dsi_cmd_drv;	 
extern LCM_DRIVER otm8018b_dsi_vdo_lcm_drv;	 
extern LCM_DRIVER otm8018b_dsi_vdo_txd_fwvga_lcm_drv;
extern LCM_DRIVER nt35512_dsi_vdo_lcm_drv;
extern LCM_DRIVER nt35512_wvga_dsi_vdo_boe_drv;
extern LCM_DRIVER hx8369_rgb_6585_fpga_lcm_drv;
extern LCM_DRIVER hx8369_rgb_6572_fpga_lcm_drv;
extern LCM_DRIVER hx8369_mcu_6572_lcm_drv;
extern LCM_DRIVER hx8369a_wvga_dsi_cmd_drv;
extern LCM_DRIVER hx8369a_wvga_dsi_vdo_drv;
extern LCM_DRIVER hx8392a_dsi_cmd_lcm_drv;
extern LCM_DRIVER nt35590_hd720_dsi_vdo_truly_lcm_drv;
extern LCM_DRIVER ssd2075_hd720_dsi_vdo_truly_lcm_drv;
extern LCM_DRIVER nt35590_hd720_dsi_cmd_drv;
extern LCM_DRIVER nt35590_hd720_dsi_cmd_auo_lcm_drv;
extern LCM_DRIVER nt35590_hd720_dsi_cmd_auo_fwvga_lcm_drv;
extern LCM_DRIVER nt35590_hd720_dsi_cmd_auo_wvga_lcm_drv;
extern LCM_DRIVER nt35590_hd720_dsi_cmd_auo_qhd_lcm_drv;
extern LCM_DRIVER nt35590_hd720_dsi_cmd_cmi_lcm_drv;
extern LCM_DRIVER nt35516_qhd_dsi_cmd_ipsboe_lcm_drv;
extern LCM_DRIVER nt35516_qhd_dsi_cmd_ipsboe_wvga_lcm_drv;
extern LCM_DRIVER nt35516_qhd_dsi_cmd_ipsboe_fwvga_lcm_drv;
extern LCM_DRIVER nt35516_qhd_dsi_cmd_ips9k1431_drv;
extern LCM_DRIVER nt35516_qhd_dsi_cmd_tft9k1342_drv;
extern LCM_DRIVER bp070ws1_lcm_drv;
extern LCM_DRIVER bp101wx1_lcm_drv;
extern LCM_DRIVER bp101wx1_n_lcm_drv;
extern LCM_DRIVER nt35516_qhd_rav4_lcm_drv;
extern LCM_DRIVER r63311_fhd_dsi_vdo_sharp_lcm_drv;
extern LCM_DRIVER r81592_hvga_dsi_cmd_drv;
extern LCM_DRIVER rm68190_dsi_vdo_lcm_drv;
extern LCM_DRIVER nt35596_fhd_dsi_vdo_truly_lcm_drv;
extern LCM_DRIVER nt35595_fhd_dsi_vdo_truly_lcm_drv;
extern LCM_DRIVER nt35595_fhd_dsi_cmd_truly_lcm_drv;
extern LCM_DRIVER nt35595_fhd_dsi_cmd_truly_tps65132_lcm_drv;
extern LCM_DRIVER nt35596_fhd_dsi_vdo_yassy_lcm_drv;
extern LCM_DRIVER nt35590_hd720_dsi_cmd_truly2_lcm_drv;
extern LCM_DRIVER otm9608_wvga_dsi_cmd_drv;
extern LCM_DRIVER nt35510_dbi_18bit_gionee_lcm_drv;
extern LCM_DRIVER otm8009a_fwvga_dsi_cmd_tianma_lcm_drv;
extern LCM_DRIVER otm8009a_fwvga_dsi_vdo_tianma_lcm_drv;
extern LCM_DRIVER hx8389b_qhd_dsi_vdo_tianma_lcm_drv;
extern LCM_DRIVER cm_otc3108bhv161_dsi_vdo_lcm_drv;
extern LCM_DRIVER auo_b079xat02_dsi_vdo_lcm_drv;
extern LCM_DRIVER hx8389b_qhd_dsi_vdo_tianma055xdhp_lcm_drv;
extern LCM_DRIVER cpt_claa101fp01_dsi_vdo_lcm_drv;
extern LCM_DRIVER h070d_18dm_lcm_drv;
extern LCM_DRIVER hx8394a_hd720_dsi_vdo_tianma_lcm_drv;
extern LCM_DRIVER cpt_clap070wp03xg_sn65dsi83_lcm_drv;
extern LCM_DRIVER nt35520_hd720_tm_lcm_drv;
extern LCM_DRIVER nt35520_hd720_boe_lcm_drv;
extern LCM_DRIVER nt35521_hd720_dsi_vdo_boe_lcm_drv;
extern LCM_DRIVER nt35521_hd720_tm_lcm_drv;
extern LCM_DRIVER r69429_wuxga_dsi_vdo_lcm_drv;
extern LCM_DRIVER r69429_wuxga_dsi_cmd_lcm_drv;
extern LCM_DRIVER rm68210_hd720_dsi_ufoe_cmd_lcm_drv;
extern LCM_DRIVER r63311_fhd_dsi_vedio_lcm_drv;
extern LCM_DRIVER cpt_clap070wp03xg_lvds_lcm_drv;
extern LCM_DRIVER otm8018b_dsi_vdo_lcsh72_lcm_drv;
extern LCM_DRIVER hx8369_dsi_cmd_6571_lcm_drv;
extern LCM_DRIVER hx8369_dsi_vdo_6571_lcm_drv;
extern LCM_DRIVER hx8369_dbi_6571_lcm_drv;
extern LCM_DRIVER hx8369_dpi_6571_lcm_drv;
extern LCM_DRIVER nt35510_dsi_cmd_6571_lcm_drv;
extern LCM_DRIVER nt35510_dsi_cmd_6571_hvga_lcm_drv;
extern LCM_DRIVER nt35510_dsi_cmd_6571_qvga_lcm_drv;
extern LCM_DRIVER nt35510_dsi_vdo_6571_lcm_drv;
extern LCM_DRIVER nt35510_dbi_6571_lcm_drv;
extern LCM_DRIVER nt35510_dpi_6571_lcm_drv;
extern LCM_DRIVER nt35590_dsi_cmd_6571_fwvga_lcm_drv;
extern LCM_DRIVER nt35590_dsi_cmd_6571_qhd_lcm_drv;
extern LCM_DRIVER it6151_edp_dsi_video_sharp_lcm_drv; 
extern LCM_DRIVER nt35517_qhd_dsi_vdo_lcm_drv;
extern LCM_DRIVER r63311_fhd_dsi_vdo_malata_lcm_drv;

extern LCM_DRIVER otm9605a_dsi_vdo_lg_lcm_drv;
extern LCM_DRIVER nt35517_dsi_vdo_qhd_lcm_drv;
extern LCM_DRIVER hx8389b_qhd_dsi_vdo_drv;

extern LCM_DRIVER nt35590_hd720_dsi_vdo_s800_lcm_drv;
extern LCM_DRIVER nt35590_hd720_dsi_vdo_s800_chuanma_lcm_drv;

extern LCM_DRIVER hx8394a_dsi_vdo_lcm_drv;

extern LCM_DRIVER otm9605a_qhd_dsi_vdo_u701b_ykl_lcm_drv;
LCM_DRIVER* lcm_driver_list[] = 
{ 
#if defined(HX8394A_DSI_VDO)
	&hx8394a_dsi_vdo_lcm_drv,
#endif

};

#define LCM_COMPILE_ASSERT(condition) LCM_COMPILE_ASSERT_X(condition, __LINE__)
#define LCM_COMPILE_ASSERT_X(condition, line) LCM_COMPILE_ASSERT_XX(condition, line)
#define LCM_COMPILE_ASSERT_XX(condition, line) char assertion_failed_at_line_##line[(condition)?1:-1]

unsigned int lcm_count = sizeof(lcm_driver_list)/sizeof(LCM_DRIVER*);
LCM_COMPILE_ASSERT(0 != sizeof(lcm_driver_list)/sizeof(LCM_DRIVER*));
#if defined(NT35520_HD720_DSI_CMD_TM) | defined(NT35520_HD720_DSI_CMD_BOE) | defined(NT35521_HD720_DSI_VDO_BOE) | defined(NT35521_HD720_DSI_VIDEO_TM)
#ifdef BUILD_LK
extern void mdelay(unsigned long msec);
#endif
static unsigned char lcd_id_pins_value = 0xFF;


/******************************************************************************
Function:       which_lcd_module_triple
  Description:    read LCD ID PIN status,could identify three status:high¡¢low¡¢float
  Input:           none
  Output:         none
  Return:         LCD ID1|ID0 value
  Others:         
******************************************************************************/
unsigned char which_lcd_module_triple(void)
{
    unsigned char  high_read0 = 0;
    unsigned char  low_read0 = 0;
    unsigned char  high_read1 = 0;
    unsigned char  low_read1 = 0;
    unsigned char  lcd_id0 = 0;
    unsigned char  lcd_id1 = 0;
    unsigned char  lcd_id = 0;
    //Solve Coverity scan warning : check return value
    unsigned int ret = 0;
    //only recognise once
    if(0xFF != lcd_id_pins_value) 
    {
        return lcd_id_pins_value;
    }
    //Solve Coverity scan warning : check return value
    ret = mt_set_gpio_mode(GPIO_DISP_ID0_PIN, GPIO_MODE_00);
    if(0 != ret)
    {
        LCD_DEBUG("ID0 mt_set_gpio_mode fail\n");
    }
    ret = mt_set_gpio_dir(GPIO_DISP_ID0_PIN, GPIO_DIR_IN);
    if(0 != ret)
    {
        LCD_DEBUG("ID0 mt_set_gpio_dir fail\n");
    }
    ret = mt_set_gpio_pull_enable(GPIO_DISP_ID0_PIN, GPIO_PULL_ENABLE);
    if(0 != ret)
    {
        LCD_DEBUG("ID0 mt_set_gpio_pull_enable fail\n");
    }
    ret = mt_set_gpio_mode(GPIO_DISP_ID1_PIN, GPIO_MODE_00);
    if(0 != ret)
    {
        LCD_DEBUG("ID1 mt_set_gpio_mode fail\n");
    }
    ret = mt_set_gpio_dir(GPIO_DISP_ID1_PIN, GPIO_DIR_IN);
    if(0 != ret)
    {
        LCD_DEBUG("ID1 mt_set_gpio_dir fail\n");
    }
    ret = mt_set_gpio_pull_enable(GPIO_DISP_ID1_PIN, GPIO_PULL_ENABLE);
    if(0 != ret)
    {
        LCD_DEBUG("ID1 mt_set_gpio_pull_enable fail\n");
    }
    //pull down ID0 ID1 PIN    
    ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
    if(0 != ret)
    {
        LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");
    }
    ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_DOWN);
    if(0 != ret)
    {
        LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");
    }
    //delay 100ms , for discharging capacitance 
    mdelay(100);
    //get ID0 ID1 status
    low_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
    low_read1 = mt_get_gpio_in(GPIO_DISP_ID1_PIN);
    //pull up ID0 ID1 PIN 
    ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_UP);
    if(0 != ret)
    {
        LCD_DEBUG("ID0 mt_set_gpio_pull_select->UP fail\n");
    }
    ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_UP);
    if(0 != ret)
    {
        LCD_DEBUG("ID1 mt_set_gpio_pull_select->UP fail\n");
    }
    //delay 100ms , for charging capacitance 
    mdelay(100);
    //get ID0 ID1 status
    high_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
    high_read1 = mt_get_gpio_in(GPIO_DISP_ID1_PIN);

    if( low_read0 != high_read0 )
    {
        /*float status , pull down ID0 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
        if(0 != ret)
        {
            LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");
        }
        lcd_id0 = LCD_HW_ID_STATUS_FLOAT;
    }
    else if((LCD_HW_ID_STATUS_LOW == low_read0) && (LCD_HW_ID_STATUS_LOW == high_read0))
    {
        /*low status , pull down ID0 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
        if(0 != ret)
        {
            LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");
        }
        lcd_id0 = LCD_HW_ID_STATUS_LOW;
    }
    else if((LCD_HW_ID_STATUS_HIGH == low_read0) && (LCD_HW_ID_STATUS_HIGH == high_read0))
    {
        /*high status , pull up ID0 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_UP);
        if(0 != ret)
        {
            LCD_DEBUG("ID0 mt_set_gpio_pull_select->UP fail\n");
        }
        lcd_id0 = LCD_HW_ID_STATUS_HIGH;
    }
    else
    {
        LCD_DEBUG(" Read LCD_id0 error\n");
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DISABLE);
        if(0 != ret)
        {
            LCD_DEBUG("ID0 mt_set_gpio_pull_select->Disbale fail\n");
        }
        lcd_id0 = LCD_HW_ID_STATUS_ERROR;
    }


    if( low_read1 != high_read1 )
    {
        /*float status , pull down ID1 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_DOWN);
        if(0 != ret)
        {
            LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");
        }
        lcd_id1 = LCD_HW_ID_STATUS_FLOAT;
    }
    else if((LCD_HW_ID_STATUS_LOW == low_read1) && (LCD_HW_ID_STATUS_LOW == high_read1))
    {
        /*low status , pull down ID1 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_DOWN);
        if(0 != ret)
        {
            LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");
        }
        lcd_id1 = LCD_HW_ID_STATUS_LOW;
    }
    else if((LCD_HW_ID_STATUS_HIGH == low_read1) && (LCD_HW_ID_STATUS_HIGH == high_read1))
    {
        /*high status , pull up ID1 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_UP);
        if(0 != ret)
        {
            LCD_DEBUG("ID1 mt_set_gpio_pull_select->UP fail\n");
        }
        lcd_id1 = LCD_HW_ID_STATUS_HIGH;
    }
    else
    {

        LCD_DEBUG(" Read LCD_id1 error\n");
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_DISABLE);
        if(0 != ret)
        {
            LCD_DEBUG("ID1 mt_set_gpio_pull_select->Disable fail\n");
        }
        lcd_id1 = LCD_HW_ID_STATUS_ERROR;
    }
#ifdef BUILD_LK
    dprintf(CRITICAL,"which_lcd_module_triple,lcd_id0:%d\n",lcd_id0);
    dprintf(CRITICAL,"which_lcd_module_triple,lcd_id1:%d\n",lcd_id1);
#else
    printk("which_lcd_module_triple,lcd_id0:%d\n",lcd_id0);
    printk("which_lcd_module_triple,lcd_id1:%d\n",lcd_id1);
#endif
    lcd_id =  lcd_id0 | (lcd_id1 << 2);

#ifdef BUILD_LK
    dprintf(CRITICAL,"which_lcd_module_triple,lcd_id:%d\n",lcd_id);
#else
    printk("which_lcd_module_triple,lcd_id:%d\n",lcd_id);
#endif

    lcd_id_pins_value = lcd_id;
    return lcd_id;
}
#endif
