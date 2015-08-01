/*
 * drivers/leds/sn3193_leds.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * sn3193 leds driver
 *
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/leds-mt65xx.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <mach/mt_gpio.h>
#include <linux/uaccess.h>
#include <linux/fb.h>
#include <linux/debugfs.h>


/****************************************************************************
 * defined
 ***************************************************************************/
#define SN3193_OLD_CODE
#define LED_NAME "rgbled"
#define I2C_MASTER_CLOCK       400
#ifdef GPIO_LED_EN
#undef GPIO_LED_EN
#endif
#define GPIO_LED_EN GPIO74

#define SN3193_EXTEND_OFFSET_BIT 8
/*00h*/
#define SN3193_SSD_OFFSET_BIT 0 //software shut down bit; 1:software shut down; 0:working mode
#define SN3193_OUT_EN_OFFSET_BIT 5 //out enable bit; 1:enable out; 0:disable out;

/*01h*/
#define SN3193_CSS_OFFSET_BIT 0 // channel select bit; 0:OUT1; 1:OUT2; 2:OUT3;
#define SN3193_BME_OFFSET_BIT 2 // breath mark enable bit; 0:disable; 1:enable;
#define SN3193_HT_OFFSET_BIT 4 //dead time bit; 0:dead on T2; 1:dead on T4;
#define SN3193_RM_OFFSET_BIT 5 //dead mode enable bit; 0:disable; 1:enable;

/*02h*/
#define SN3193_RGB_MODE_OFFSET_BIT 5 //LED mode set; 0:PWM control mode; 1:one programe mode;

/*03h*/
#define SN3193_CS_OFFSET_BIT 2 //current set; 0:42mA; 1:10mA; 2:5mA; 3:30mA; 4:17.5mA;

/*0Ah~0Ch is T0 setting register*/
/*10h~12h is T1 & T2 setting register*/
/*16h~18h is T3 & T4 setting register*/
#define SN3193_T0_OFFSET_BIT 4// 4bit
#define SN3193_T1_OFFSET_BIT 5// 3bit
#define SN3193_T2_OFFSET_BIT 1// 4bit
#define SN3193_T3_OFFSET_BIT 5// 3bit
#define SN3193_T4_OFFSET_BIT 1// 4bit



/*register define*/
#define SN3193_SSD_EN_REG 0x00
#define SN3193_BREATH_MODE_REG 0x01
#define SN3193_LED_MODE_REG 0x02
#define SN3193_CURRENT_SET_REG 0x03
/*04h~06h is PWM level setting register*/
#define SN3193_PWM_BLUE_REG 0x04
#define SN3193_PWM_GREEN_REG 0x05
#define SN3193_PWM_RED_REG 0x06
#define SN3193_PWM_DATA_REFRESH_REG 0x07
/*0Ah~0Ch is T0 setting register*/
#define SN3193_T0_BLUE_REG 0x0A
#define SN3193_T0_GREEN_REG 0x0B
#define SN3193_T0_RED_REG 0x0C
/*10h~12h is T1 & T2 setting register*/
#define SN3193_T1_T2_BLUE_REG 0x10
#define SN3193_T1_T2_GREEN_REG 0x11
#define SN3193_T1_T2_RED_REG 0x012
/*16h~18h is T3 & T4 setting register*/
#define SN3193_T3_T4_BLUE_REG 0x16
#define SN3193_T3_T4_GREEN_REG 0x17
#define SN3193_T3_T4_RED_REG 0x18

#define SN3193_TIME_REFRESH_REG 0x1C
#define SN3193_LED_OUT_CONTROL_REG 0x1D
#define SN3193_RESET_REG 0x2F

/*register function define*/
#define SN3193_RM_ENABLE 1<<SN3193_RM_OFFSET_BIT
#define SN3193_RM_DISABLE 0<<SN3193_RM_OFFSET_BIT
#define SN3193_CSS_OUT1_BLUE 0<<SN3193_CSS_OFFSET_BIT
#define SN3193_CSS_OUT2_GREEN 1<<SN3193_CSS_OFFSET_BIT
#define SN3193_CSS_OUT3_RED 2<<SN3193_CSS_OFFSET_BIT
#define SN3193_CSS_OUT_ALL 3<<SN3193_CSS_OFFSET_BIT
#define SN3193_RGB_MODE_ENABLE 1<<SN3193_RGB_MODE_OFFSET_BIT
#define SN3193_RGB_MODE_DISABLE 0<<SN3193_RGB_MODE_OFFSET_BIT
#define SN3193_CS_SET_42mA 0<<SN3193_CS_OFFSET_BIT
#define SN3193_CS_SET_10mA 1<<SN3193_CS_OFFSET_BIT
#define SN3193_CS_SET_5mA 2<<SN3193_CS_OFFSET_BIT
#define SN3193_CS_SET_30mA 3<<SN3193_CS_OFFSET_BIT
#define SN3193_CS_SET_17_5mA 4<<SN3193_CS_OFFSET_BIT
#define SN3193_LED_OUT_DISABLE_ALL 0
#define SN3193_LED_OUT_ENABLE_ALL 7
#define SN3193_SSD_ENABLE 1<<SN3193_SSD_OFFSET_BIT 
#define SN3193_SSD_DISABLE 0<<SN3193_SSD_OFFSET_BIT //normal work mode
#define SN3193_EN_OUT_CLOSE 0<<<SN3193_OUT_EN_OFFSET_BIT
#define SN3193_EN_OUT_OPEN 1<<SN3193_OUT_EN_OFFSET_BIT

/*backlight value analyze*/
#define SN3193_BL_CUR_OFFSET_BIT 24// 3bit

#define SN3193_BL_R_OFFSET_BIT 16// 8bit
#define SN3193_BL_G_OFFSET_BIT 8// 8bit
#define SN3193_BL_B_OFFSET_BIT 0// 8bit
/*delay_on*/
#define SN3193_BL_RT2_OFFSET_BIT 0
#define SN3193_BL_RT3_OFFSET_BIT 4
#define SN3193_BL_RT1_OFFSET_BIT 7
#define SN3193_BL_GT2_OFFSET_BIT 10
#define SN3193_BL_GT3_OFFSET_BIT 14
#define SN3193_BL_GT1_OFFSET_BIT 17
#define SN3193_BL_BT2_OFFSET_BIT 20
#define SN3193_BL_BT3_OFFSET_BIT 24
#define SN3193_BL_BT1_OFFSET_BIT 27
/*delay_off*/
#define SN3193_BL_RT4_OFFSET_BIT 0
#define SN3193_BL_RT0_OFFSET_BIT 4
#define SN3193_BL_GT4_OFFSET_BIT 8
#define SN3193_BL_GT0_OFFSET_BIT 12
#define SN3193_BL_BT4_OFFSET_BIT 16
#define SN3193_BL_BT0_OFFSET_BIT 20
/*masks*/
#define SN3193_BL_RGB_MASK 0xff
#define SN3193_BL_T0_MASK 0x0f
#define SN3193_BL_T1_MASK 0x07
#define SN3193_BL_T2_MASK 0x0f
#define SN3193_BL_T3_MASK 0x07
#define SN3193_BL_T4_MASK 0x0f

static const char *cur_text[] = {
	"42mA",
	"10mA",
	"5mA",
	"30mA",
	"17.5mA",
};
static const char *t0_t4_text[] = {
	"0s",
	"0.13s",
	"0.26s",
	"0.52s",
	"1.04s",
	"2.08s",
	"4.16s",
	"8.32s",
	"16.64s",
	"33.28s",
	"66.56s",
};
static const char *t1_t3_text[] = {
	"0.13s",
	"0.26s",
	"0.52s",
	"1.04s",
	"2.08s",
	"4.16s",
	"8.32s",
	"16.64s",
};
static const char *t2_text[] = {
	"0s",
	"0.13s",
	"0.26s",
	"0.52s",
	"1.04s",
	"2.08s",
	"4.16s",
	"8.32s",
	"16.64s",
};



static int sn3193_is_init= 0;
static int debug_enable = 1;

#define LEDS_DEBUG(format, args...) do{ \
	if(debug_enable) \
	{\
		printk(KERN_EMERG format,##args);\
	}\
}while(0)

struct sn3193_leds_priv {
	struct led_classdev cdev;
	struct work_struct work;
	int gpio;
	int level;
	int delay_on;
	int delay_off;
};
typedef struct
{
	void (*Charging_RGB_LED)(unsigned int value);
   
} LENOVO_LED_CONTROL_FUNCS;

//extern void lenovo_register_led_control(LENOVO_LED_CONTROL_FUNCS * ctrl);


/****************************************************************************
 * local functions
 ***************************************************************************/

static int	sn3193_leds_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
//static int  sn3193_leds_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int  sn3193_leds_i2c_remove(struct i2c_client *client);


static int __init sn3193_leds_platform_probe(struct platform_device *pdev);
static int sn3193_leds_platform_remove(struct platform_device *pdev);
////////////////

#define SN3193_I2C_ADDR 0xD0
#define SN3193_BUS_NUM 3
struct i2c_client *sn3193_i2c_cilent = NULL;

static struct i2c_board_info __initdata sn3193_i2c_board_info = { I2C_BOARD_INFO("sn3193", (SN3193_I2C_ADDR >> 1))};

static const struct i2c_device_id sn3193_i2c_id[] = {{"sn3193",0}, {}};
static struct i2c_driver sn3193_i2c_driver = {
    .probe = sn3193_leds_i2c_probe,
    .remove = sn3193_leds_i2c_remove,
    .driver.name = "sn3193",
    .id_table = sn3193_i2c_id,
    //.address_list = (const unsigned short *) forces,
};


static struct platform_driver sn3193_leds_platform_driver = {
	.driver		= {
		.name	= "leds-sn3193",
		//.owner	= THIS_MODULE,
	},
	.probe		= sn3193_leds_platform_probe,
	.remove		= sn3193_leds_platform_remove,
	//.suspend	= sn3193_leds_platform_suspend,
	//.shutdown   = sn3193_leds_platform_shutdown,
};


static void sn3193_udelay(UINT32 us)
{
	udelay(us);
}

static void sn3193_mdelay(UINT32 ms)
{
	msleep(ms);
}

static int sn3193_i2c_txdata(char *txdata, int len)
{
    int ret;
    struct i2c_msg msg[] = {
            {
                .addr = sn3193_i2c_cilent->addr,
                .flags = 0,
                .len =len,
                .buf = txdata,
            },
    };
    if(sn3193_i2c_cilent != NULL) {
        ret = i2c_transfer(sn3193_i2c_cilent->adapter, msg, 1);
        if(ret < 0)
            pr_err("%s i2c write erro: %d\n", __func__, ret);
    } else {
        LEDS_DEBUG("sn3193_i2c_cilent null\n");
    }
    return ret;
}

static int sn3193_write_reg(u8 addr, u8 para)
{
	LEDS_DEBUG("[LED]%s\n", __func__);

    u8 buf[3];
    int ret = -1;

    buf[0] = addr;
    buf[1] = para;
    ret = sn3193_i2c_txdata(buf,2);
    if(ret < 0) {
        LEDS_DEBUG("%s write reg failed! addr=0x%x para=0x%x ret=%d\n", __func__,buf[0],buf[1],ret);
        return -1;
    }
    return 0;
}

#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
static int fake_ic = 0;
static int sn3193_test(void)
{
	int ret = -1;
	int retry = 0;
    mt_set_gpio_out(GPIO_LED_EN,GPIO_OUT_ONE);
detect_retry:
    sn3193_mdelay(1);
    ret = sn3193_write_reg(0x00, 0x01);
	if(ret<0) {
		if(retry++ < 3)
			goto detect_retry;
		fake_ic = 1;
	}
	return ret;
}
#endif

static int sn3193_init(void)
{
	LEDS_DEBUG("[LED]+%s\n", __func__);
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
	if(sn3193_test()<0)
		return -1;
#endif
    mt_set_gpio_mode(GPIO_LED_EN,GPIO_MODE_GPIO);
    mt_set_gpio_dir(GPIO_LED_EN,GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_LED_EN,GPIO_OUT_ONE);

    sn3193_mdelay(10);

    sn3193_write_reg(0x2F, 1);
    sn3193_mdelay(10);
    sn3193_write_reg(0x1D, 0x00);
    sn3193_write_reg(0x03, 0x01 << 2); // 011:30mA
	
    sn3193_write_reg(0x04, 170); //B
    sn3193_write_reg(0x05, 130); //G
    sn3193_write_reg(0x06, 170); //R

    sn3193_write_reg(0x0A, 0x00);
    sn3193_write_reg(0x0B, 0x00);
    sn3193_write_reg(0x0C, 0x00);

    sn3193_write_reg(0x10, 0x04 << 1);
    sn3193_write_reg(0x11, 0x04 << 1);
    sn3193_write_reg(0x12, 0x04 << 1);

    sn3193_write_reg(0x16, 0x04 << 1);  
    sn3193_write_reg(0x17, 0x04 << 1);  
    sn3193_write_reg(0x18, 0x04 << 1);  
	
    sn3193_write_reg(0x02, 0x01 << 5);
    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x07, 1);	
    sn3193_write_reg(0x00, 0x01);
	sn3193_is_init = 1;

    LEDS_DEBUG("[LED]-%s\n", __func__);
	return 0;
}
void sn3193_off(void)
{
	LEDS_DEBUG("[LED]%s\n", __func__);

    sn3193_write_reg(0x1D, 0x00);
    sn3193_write_reg(0x07, 1);	
    sn3193_write_reg(0x00, 0x01);
}

void sn3193_rgb_factory_test(void)
{
	LEDS_DEBUG("[LED]%s\n", __func__);

    sn3193_write_reg(0x00, 0x20);
    sn3193_write_reg(0x02, 0x01 << 5); //RGB mode

    sn3193_write_reg(0x06, 170); //DOUT3,R
    sn3193_write_reg(0x05, 130); //DOUT2,G
    sn3193_write_reg(0x04, 170); //DOUT1,B

    sn3193_write_reg(0x0C, 0x00);	//R
    sn3193_write_reg(0x12, 0x04);
    sn3193_write_reg(0x18, 0x08);  

    sn3193_write_reg(0x0B, 0x30);	//G
    sn3193_write_reg(0x11, 0x04);
    sn3193_write_reg(0x17, 0x08);  

    sn3193_write_reg(0x0A, 0x40);	//B
    sn3193_write_reg(0x10, 0x04);
    sn3193_write_reg(0x16, 0x08);  
    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x07, 1);	 

    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x01, 0x03); 
    sn3193_write_reg(0x1D, 0x07);
    sn3193_write_reg(0x07, 1);	 
}


static int  sn3193_blink_set(struct led_classdev *led_cdev,
									unsigned long *delay_on,
									unsigned long *delay_off)
{
	LEDS_DEBUG("[LED]%s delay_on=0x%x, delay_off=0x%x\n", __func__,*delay_on,*delay_off);

	struct sn3193_leds_priv *led_data =
		container_of(led_cdev, struct sn3193_leds_priv, cdev);


	if (*delay_on != led_data->delay_on || *delay_off != led_data->delay_off) {
		led_data->delay_on = *delay_on;
		led_data->delay_off = *delay_off;

	
	}
	
	return 0;
}


/******************************
*func:sn3193_proc_backlight_value

level:
 D[7-0] -> B(8bit)
 D[15-8] -> G(8bit)
 D[23-16] -> R(8bit)
 
 0x ff ff ff
     | |  |---B
     | |-----G
     |-------R
     
delay_on:
 D[3-0] -> RT2(4bit)
 D[6-4] -> RT3(3bit)
 D[9-7] -> RT1(3bit)

 D[13-10] -> GT2
 D[16-14] -> GT3
 D[19-17] -> GT1

 D[23-20] -> BT2
 D[26-24] -> BT3
 D[29-27] -> BT1


delay_off:
 D[3-0] -> RT4
 D[7-4] -> RT0

 D[11-8] -> GT4
 D[15-12] -> GT0

 D[19-16] -> BT4
 D[23-20] -> BT0

period of time
			    ______			    ______
			   |		 |			   |		 |
	               |		  |			  |		  |
	              |         	   |                |         	   |
	             |               |              |               |
	            |                 |            |                 |
___________|                   |______|                   |______

           T0   |T1 |  T2   |T3|   T4   |T1|    T2  |T3|  T4    	
 
*******************************/
static void sn3193_proc_backlight_value(int level, int delay_on, int delay_off)
{
	int led_cur;
	int pwm_red,pwm_green,pwm_blue;
	int Rt0,Rt1,Rt2,Rt3,Rt4;
	int Gt0,Gt1,Gt2,Gt3,Gt4;
	int Bt0,Bt1,Bt2,Bt3,Bt4;

	printk("%s level=0x%x delay_on=0x%x delay_off=0x%x\n",__func__,level,delay_on,delay_off);

	led_cur = (level>>SN3193_BL_CUR_OFFSET_BIT)&0x07;
	if(led_cur>4) led_cur=4;
	if(led_cur==0) led_cur=1;//def is 1:10mA
	
	pwm_red = (level>>SN3193_BL_R_OFFSET_BIT)&0xff;
	pwm_green = (level>>SN3193_BL_G_OFFSET_BIT)&0xff;	
	pwm_blue = (level>>SN3193_BL_B_OFFSET_BIT)&0xff;

	Rt2=(delay_on>>SN3193_BL_RT2_OFFSET_BIT)&SN3193_BL_T2_MASK;
	if(Rt2>8) Rt2=8;
	Rt3=(delay_on>>SN3193_BL_RT3_OFFSET_BIT)&SN3193_BL_T3_MASK;
	Rt1=(delay_on>>SN3193_BL_RT1_OFFSET_BIT)&SN3193_BL_T1_MASK;
	Gt2=(delay_on>>SN3193_BL_GT2_OFFSET_BIT)&SN3193_BL_T2_MASK;
	if(Gt2>8) Gt2=8;
	Gt3=(delay_on>>SN3193_BL_GT3_OFFSET_BIT)&SN3193_BL_T3_MASK;
	Gt1=(delay_on>>SN3193_BL_GT1_OFFSET_BIT)&SN3193_BL_T1_MASK;
	Bt2=(delay_on>>SN3193_BL_BT2_OFFSET_BIT)&SN3193_BL_T2_MASK;
	if(Bt2>8) Bt2=8;
	Bt3=(delay_on>>SN3193_BL_BT3_OFFSET_BIT)&SN3193_BL_T3_MASK;
	Bt1=(delay_on>>SN3193_BL_BT1_OFFSET_BIT)&SN3193_BL_T1_MASK;

	Rt4=(delay_off>>SN3193_BL_RT4_OFFSET_BIT)&SN3193_BL_T4_MASK;
	if(Rt4>10) Rt4=10;
	Rt0=(delay_off>>SN3193_BL_RT0_OFFSET_BIT)&SN3193_BL_T0_MASK;
	if(Rt0>10) Rt0=10;
	Gt4=(delay_off>>SN3193_BL_GT4_OFFSET_BIT)&SN3193_BL_T4_MASK;
	if(Gt4>10) Gt4=10;
	Gt0=(delay_off>>SN3193_BL_GT0_OFFSET_BIT)&SN3193_BL_T0_MASK;	
	if(Gt0>10) Gt0=10;
	Bt4=(delay_off>>SN3193_BL_BT4_OFFSET_BIT)&SN3193_BL_T4_MASK;
	if(Bt4>10) Bt4=10;
	Bt0=(delay_off>>SN3193_BL_BT0_OFFSET_BIT)&SN3193_BL_T0_MASK;	
	if(Bt0>10) Bt0=10;
	
	LEDS_DEBUG("[LED]%s cur:%s rgb:0x%x 0x%x 0x%x; R:t0=%s t1=%s t2=%s t3=%s t4=%s; G:t0=%s t1=%s t2=%s t3=%s t4=%s; B:t0=%s t1=%s t2=%s t3=%s t4=%s;\n",
				__func__,
				cur_text[led_cur],
				pwm_red,pwm_green,pwm_blue,
				t0_t4_text[Rt0],t1_t3_text[Rt1],t2_text[Rt2],t1_t3_text[Rt3],t0_t4_text[Rt4],
				t0_t4_text[Gt0],t1_t3_text[Gt1],t2_text[Gt2],t1_t3_text[Gt3],t0_t4_text[Gt4],
				t0_t4_text[Bt0],t1_t3_text[Bt1],t2_text[Bt2],t1_t3_text[Bt3],t0_t4_text[Bt4]);

	sn3193_write_reg(SN3193_SSD_EN_REG, SN3193_EN_OUT_OPEN | SN3193_SSD_DISABLE);

	sn3193_write_reg(SN3193_BREATH_MODE_REG, SN3193_RM_DISABLE | SN3193_CSS_OUT_ALL);
	sn3193_write_reg(SN3193_CURRENT_SET_REG, led_cur<<SN3193_CS_OFFSET_BIT);
	
	sn3193_write_reg(SN3193_PWM_BLUE_REG, pwm_blue);
	sn3193_write_reg(SN3193_PWM_GREEN_REG, pwm_green);
	sn3193_write_reg(SN3193_PWM_RED_REG, pwm_red);

	if((delay_on!=0) && (delay_off!=0)){
	sn3193_write_reg(SN3193_LED_MODE_REG, SN3193_RGB_MODE_ENABLE);
	
	sn3193_write_reg(SN3193_T0_BLUE_REG, Bt0<<SN3193_T0_OFFSET_BIT);
	sn3193_write_reg(SN3193_T1_T2_BLUE_REG, (Bt1<<SN3193_T1_OFFSET_BIT)|(Bt2<<SN3193_T2_OFFSET_BIT));
	sn3193_write_reg(SN3193_T3_T4_BLUE_REG, (Bt3<<SN3193_T3_OFFSET_BIT)|(Bt4<<SN3193_T4_OFFSET_BIT));
	
	sn3193_write_reg(SN3193_T0_GREEN_REG, Gt0<<SN3193_T0_OFFSET_BIT);
	sn3193_write_reg(SN3193_T1_T2_GREEN_REG, (Gt1<<SN3193_T1_OFFSET_BIT)|(Gt2<<SN3193_T2_OFFSET_BIT));
	sn3193_write_reg(SN3193_T3_T4_GREEN_REG, (Gt3<<SN3193_T3_OFFSET_BIT)|(Gt4<<SN3193_T4_OFFSET_BIT));
	
	sn3193_write_reg(SN3193_T0_RED_REG, Rt0<<SN3193_T0_OFFSET_BIT);
	sn3193_write_reg(SN3193_T1_T2_RED_REG, (Rt1<<SN3193_T1_OFFSET_BIT)|(Rt2<<SN3193_T2_OFFSET_BIT));
	sn3193_write_reg(SN3193_T3_T4_RED_REG, (Rt3<<SN3193_T3_OFFSET_BIT)|(Rt4<<SN3193_T4_OFFSET_BIT));
	}else{
	sn3193_write_reg(SN3193_LED_MODE_REG, SN3193_RGB_MODE_DISABLE);	
	}
	sn3193_write_reg(SN3193_TIME_REFRESH_REG, 1);
    sn3193_write_reg(SN3193_PWM_DATA_REFRESH_REG, 1);

	sn3193_write_reg(SN3193_LED_OUT_CONTROL_REG, SN3193_LED_OUT_ENABLE_ALL);
	
}
static void SN3193_PowerOff_Charging_RGB_LED(unsigned int level)
{
    if(!sn3193_is_init) {
        sn3193_init();
    }
	printk("[JX] %s\n",__func__);
	sn3193_proc_backlight_value(level,0,0);

}
// ms  T0 & T4 0s~66.56s for sn3193
static const unsigned int rgb_time[] = {0,130,260,520,1040,2080,4160,8320,16640,33280,66560};
static void sn3193_value_preprocess(struct sn3193_leds_priv * leds_data)
{
	int color, delay_on, delay_off;
	int def_on, def_off;
	int level_time = 0;

	if(leds_data==NULL)
		return;
	
	color = leds_data->level;
	delay_on  = leds_data->delay_on;
	delay_off = leds_data->delay_off;
	if(delay_on==0 || delay_off==0){
		delay_on=0;
		delay_off=0;
	}
	printk("[JX] %s color=0x%x on=0x%x off=0x%x\n",__func__,color,delay_on,delay_off);

			def_on=0x9024090;// BT1=001 BT3=001 BT2=0000 GT1=001 GT3=001 GT2=0000 RT1=001 RT3=001 RT2=0000
			def_off= 0x0;// T0=0000 T4=0000;
			//we only change T2 and T4
			for(level_time=10;level_time>=0;level_time--){
				if(delay_on>=rgb_time[level_time])
					break;
			}
			delay_on = def_on | (level_time<<20) | (level_time<<10) | level_time;
			
			for(level_time=10;level_time>=0;level_time--){
				if(delay_off>=rgb_time[level_time])
					break;
			}
			delay_off = def_off | (level_time<<16) | (level_time<<8) | level_time;
	

		leds_data->level=color;
		leds_data->delay_on=delay_on;
		leds_data->delay_off=delay_off;
}
#ifdef SN3193_OLD_CODE

static void breath_green_led_on(void) //Dout2
{
     sn3193_write_reg(0x00, 0x20);
     sn3193_write_reg(0x05, 130);
     sn3193_write_reg(0x02, 0x00 << 5);
     sn3193_write_reg(0x01, 0x01); //Dout3:10->red on, Dout2:01->green, Dout1:00->blue
     sn3193_write_reg(0x1D, 0x02); //Dout3 enable, xxx:Dout3-Dout1
     sn3193_write_reg(0x07, 1);	
}
static void breath_red_led_on(void) //Dout3
{
     sn3193_write_reg(0x00, 0x20);
     sn3193_write_reg(0x06, 170);
     sn3193_write_reg(0x02, 0x00 << 5);
     sn3193_write_reg(0x01, 0x02); //Dout3:10->red on, Dout2:01->green, Dout1:00->blue
     sn3193_write_reg(0x1D, 0x04); //Dout3 enable, xxx:Dout3-Dout1
     sn3193_write_reg(0x07, 1);	
}

static void breath_red_led_slow_blink(void)
{
    sn3193_write_reg(0x00, 0x20);
    sn3193_write_reg(0x06, 170);
    sn3193_write_reg(0x0C, 0x00);	//R
    sn3193_write_reg(0x12, 0x68); //0x04<< 1);
    sn3193_write_reg(0x18, 0x6a); //0x04 << 1);  
    sn3193_write_reg(0x02, 0x01 << 5);
    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x01, 0x02); 
    sn3193_write_reg(0x1D, 0x04);
    sn3193_write_reg(0x07, 1);	 
}
static void breath_green_led_slow_blink(void)
{
#if 0
    sn3193_write_reg(0x00, 0x20);
    sn3193_write_reg(0x11, 0x60); //0x04<< 1);
    sn3193_write_reg(0x17, 0x6a); //0x04 << 1);  
    sn3193_write_reg(0x02, 0x01 << 5);
    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x01, 0x01); 
    sn3193_write_reg(0x1D, 0x02);
    sn3193_write_reg(0x07, 1);	 
#else
    sn3193_write_reg(0x00, 0x20);

    sn3193_write_reg(0x02, 0x01 << 5); //RGB mode

    sn3193_write_reg(0x06, 130); //DOUT3,R
    sn3193_write_reg(0x05, 0); //DOUT2,G
    sn3193_write_reg(0x04, 170); //DOUT1,B
	
    sn3193_write_reg(0x0B, 0x00);
    sn3193_write_reg(0x11, 0x68); //0x04 << 1);
    sn3193_write_reg(0x17, 0x6a); //0x04 << 1);  

    sn3193_write_reg(0x0C, 0x00);	//R
    sn3193_write_reg(0x12, 0x68); //0x04 << 1);
    sn3193_write_reg(0x18, 0x6a); //0x04 << 1);  

    sn3193_write_reg(0x0A, 0x00);
    sn3193_write_reg(0x10, 0x68); //0x04 << 1);
    sn3193_write_reg(0x16, 0x6a); //0x04 << 1);  

    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x01, 0x03); 
    sn3193_write_reg(0x1D, 0x07);
    sn3193_write_reg(0x07, 1);	 
#endif
}

static void breath_blue_led_slow_blink(void)
{
#if 0
    sn3193_write_reg(0x00, 0x20);
    sn3193_write_reg(0x0A, 0x00);
    sn3193_write_reg(0x10, 0x60); //0x04<< 1);
    sn3193_write_reg(0x16, 0x6a); //0x04 << 1);  
    sn3193_write_reg(0x02, 0x01 << 5);
    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x01, 0x00); 
    sn3193_write_reg(0x1D, 0x01);
    sn3193_write_reg(0x07, 1);	 
#else
    sn3193_write_reg(0x00, 0x20);
    sn3193_write_reg(0x02, 0x01 << 5); //RGB mode

    sn3193_write_reg(0x06, 0); //DOUT3,R
    sn3193_write_reg(0x05, 100); //DOUT2,G
    sn3193_write_reg(0x04, 170); //DOUT1,B
	
    sn3193_write_reg(0x0B, 0x00);
    sn3193_write_reg(0x11, 0x68); //0x04 << 1);
    sn3193_write_reg(0x17, 0x6a); //0x04 << 1);

    sn3193_write_reg(0x0C, 0x00);
    sn3193_write_reg(0x12, 0x68); //0x04 << 1);
    sn3193_write_reg(0x18, 0x6a); //0x04 << 1);

    sn3193_write_reg(0x0A, 0x00);
    sn3193_write_reg(0x10, 0x68); //0x04 << 1);
    sn3193_write_reg(0x16, 0x6a); //0x04 << 1);

    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x01, 0x03); 
    sn3193_write_reg(0x1D, 0x07);
    sn3193_write_reg(0x07, 1);	 
#endif
}

static void breath_green_led_quick_blink(void)
{
#if 0
    sn3193_write_reg(0x00, 0x20);
    sn3193_write_reg(0x11, 0x40); //0x00 << 1);
    sn3193_write_reg(0x17, 0x44); //0x00 << 1);
    sn3193_write_reg(0x02, 0x01 << 5);
    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x01, 0x01); 
    sn3193_write_reg(0x1D, 0x02);
    sn3193_write_reg(0x07, 1);	  
#else
    sn3193_write_reg(0x00, 0x20);
    sn3193_write_reg(0x02, 0x01 << 5); //RGB mode

    sn3193_write_reg(0x06, 130); //DOUT3,R
    sn3193_write_reg(0x05, 0); //DOUT2,G
    sn3193_write_reg(0x04, 170); //DOUT1,B
	
    sn3193_write_reg(0x0B, 0x00);
    sn3193_write_reg(0x11, 0x40); //0x00 << 1);
    sn3193_write_reg(0x17, 0x44); //0x00 << 1);

    sn3193_write_reg(0x0C, 0x00);
    sn3193_write_reg(0x12, 0x40); //0x00 << 1);
    sn3193_write_reg(0x18, 0x44); //0x00 << 1);

    sn3193_write_reg(0x0A, 0x00);
    sn3193_write_reg(0x10, 0x40); //0x00 << 1);
    sn3193_write_reg(0x16, 0x44); //0x00 << 1);

    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x01, 0x03); 
    sn3193_write_reg(0x1D, 0x07);
    sn3193_write_reg(0x07, 1);	 
#endif
}

static void breath_red_led_quick_blink(void)
{
    sn3193_write_reg(0x00, 0x20);
    sn3193_write_reg(0x0C, 0x00);
    sn3193_write_reg(0x12, 0x40); //0x00 << 1);
    sn3193_write_reg(0x18, 0x44); //0x00 << 1);
    sn3193_write_reg(0x02, 0x01 << 5);
    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x01, 0x02); 
    sn3193_write_reg(0x1D, 0x04);
    sn3193_write_reg(0x07, 1);	 
}

static void breath_blue_led_quick_blink(void)
{
    sn3193_write_reg(0x00, 0x20);
    sn3193_write_reg(0x0A, 0x00);
    sn3193_write_reg(0x10, 0x40); //0x00 << 1);
    sn3193_write_reg(0x16, 0x44); //0x00 << 1);
    sn3193_write_reg(0x02, 0x01 << 5);
    sn3193_write_reg(0x1C, 1);
    sn3193_write_reg(0x01, 0x00); 
    sn3193_write_reg(0x1D, 0x01);
    sn3193_write_reg(0x07, 1);	 
}
static void breath_led_off(void)
{
    sn3193_write_reg(0x1D, 0x00);
    sn3193_write_reg(0x07, 1);	
    sn3193_write_reg(0x00, 0x01);
    //breath_factory_mode = KAL_FALSE;
}

static void led_breath_delayed_work(int mode)
{

    LEDS_DEBUG("led_breath_delayed_work mode = %d\n", mode);
    switch(mode)  {
	case 0: //off
		//breath_led_off();
		break;
	case 1: //keypad leds
		//breath_blue_led_on();
		break;
	case 2: //charging full
		breath_green_led_on();
		break;
	case 3: //charging
		breath_red_led_on();
		break;
	case 4: //miss call, unread message
		breath_green_led_slow_blink();
		break;
	case 5: //incoming call
		breath_green_led_quick_blink();
		break;
	case 6: //low battery
		breath_red_led_slow_blink();
		break;
	case 7: 
		breath_blue_led_slow_blink();
		break;
	case 8: 
		breath_blue_led_quick_blink();
		break;
//	case 10:
//		breath_rgb_factory_test();
//		break;
	default:
		breath_led_off();
		break;
        }
}

#endif
static void sn3193_led_work(struct work_struct *work)
{
	struct sn3193_leds_priv	*led_data =
		container_of(work, struct sn3193_leds_priv, work);
#ifdef SN3193_OLD_CODE
	led_breath_delayed_work(led_data->level);
#else
	if((led_data->level)==0)
		sn3193_off();
	else{
		sn3193_value_preprocess(led_data);
		sn3193_proc_backlight_value(led_data->level,led_data->delay_on,led_data->delay_off);
	}
#endif
}

void sn3193_led_set(struct led_classdev *led_cdev,enum led_brightness value)
{
	LEDS_DEBUG("[LED]%s value=0x%x\n", __func__,value);

	struct sn3193_leds_priv *led_data =
		container_of(led_cdev, struct sn3193_leds_priv, cdev);
	

    if(sn3193_i2c_cilent == NULL) {
        printk("sn3193_i2c_cilent null\n");
        return;
    }
    cancel_work_sync(&led_data->work);
	led_data->level = value;
        
    if(!sn3193_is_init) {
        sn3193_init();
    }
    schedule_work(&led_data->work);
}

/*for factory test*/
static void sn3193_led_work_test(struct work_struct *work)
{
	struct sn3193_leds_priv	*led_data =
		container_of(work, struct sn3193_leds_priv, work);

if((led_data->level)==0)
	sn3193_off();
else
	sn3193_rgb_factory_test();
}

/*for factory test*/

void sn3193_led_set_test(struct led_classdev *led_cdev,enum led_brightness value)
{
	LEDS_DEBUG("[LED]%s value=%d\n", __func__,value);

	struct sn3193_leds_priv *led_data =
		container_of(led_cdev, struct sn3193_leds_priv, cdev);
	

    if(sn3193_i2c_cilent == NULL) {
        printk("sn3193_i2c_cilent null\n");
        return;
    }
    cancel_work_sync(&led_data->work);
	led_data->level = value;
        
    if(!sn3193_is_init) {
        sn3193_init();
    }
    schedule_work(&led_data->work);
}

static int  sn3193_leds_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret=-1;
	LEDS_DEBUG("[LED]%s\n", __func__);
	sn3193_i2c_cilent = client;

    ret = sn3193_init();

    return ret;
 }

static int  sn3193_leds_i2c_remove(struct i2c_client *client)
{
   
   LEDS_DEBUG("[LED]%s\n", __func__);
    return 0;
}


struct sn3193_leds_priv *g_sn3193_leds_data[2];


static LENOVO_LED_CONTROL_FUNCS led_ctrl  = 
{
	.Charging_RGB_LED = SN3193_PowerOff_Charging_RGB_LED,
};


static int __init sn3193_leds_platform_probe(struct platform_device *pdev)
{
	int ret=0;
	int i;
	LEDS_DEBUG("[LED]%s\n", __func__);

	if(i2c_add_driver(&sn3193_i2c_driver))
	{
		printk("add i2c driver error %s\n",__func__);
		return -1;
	} 

#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
	if(fake_ic)
		goto err;
#endif

	g_sn3193_leds_data[0] = kzalloc(sizeof(struct sn3193_leds_priv), GFP_KERNEL);
	if (!g_sn3193_leds_data[0]) {
		ret = -ENOMEM;
		goto err;
	}

	
	g_sn3193_leds_data[0]->cdev.name = LED_NAME;
	g_sn3193_leds_data[0]->cdev.brightness_set = sn3193_led_set;
	g_sn3193_leds_data[0]->cdev.max_brightness = 0xffffffff;
	g_sn3193_leds_data[0]->cdev.blink_set = sn3193_blink_set;
	INIT_WORK(&g_sn3193_leds_data[0]->work, sn3193_led_work);
	g_sn3193_leds_data[0]->gpio = GPIO_LED_EN;
	g_sn3193_leds_data[0]->level = 0;
	
	ret = led_classdev_register(&pdev->dev, &g_sn3193_leds_data[0]->cdev);
	if (ret)
		goto err;

	//for factory test
	g_sn3193_leds_data[1] = kzalloc(sizeof(struct sn3193_leds_priv), GFP_KERNEL);
	if (!g_sn3193_leds_data[1]) {
		ret = -ENOMEM;
		goto err;
	}

	g_sn3193_leds_data[1]->cdev.name = "test-led";
	g_sn3193_leds_data[1]->cdev.brightness_set = sn3193_led_set_test;
	g_sn3193_leds_data[1]->cdev.max_brightness = 0xff;
	INIT_WORK(&g_sn3193_leds_data[1]->work, sn3193_led_work_test);
	g_sn3193_leds_data[1]->gpio = GPIO_LED_EN;
	g_sn3193_leds_data[1]->level = 0;
	
	ret = led_classdev_register(&pdev->dev, &g_sn3193_leds_data[1]->cdev);
	//end for factory test
	
	if (ret)
		goto err;


//	lenovo_register_led_control(&led_ctrl);
	
	return 0;
	
err:

	for (i = 1; i >=0; i--) {
			if (!g_sn3193_leds_data[i])
				continue;
			led_classdev_unregister(&g_sn3193_leds_data[i]->cdev);
			cancel_work_sync(&g_sn3193_leds_data[i]->work);
			kfree(g_sn3193_leds_data[i]);
			g_sn3193_leds_data[i] = NULL;
		}
	i2c_del_driver(&sn3193_i2c_driver);

	return ret;
}

static int sn3193_leds_platform_remove(struct platform_device *pdev)
{
	struct sn3193_leds_priv *priv = dev_get_drvdata(&pdev->dev);
	LEDS_DEBUG("[LED]%s\n", __func__);

	dev_set_drvdata(&pdev->dev, NULL);

	kfree(priv);

	i2c_del_driver(&sn3193_i2c_driver);

	return 0;
}


/***********************************************************************************
* please add platform device in mt_devs.c
*
************************************************************************************/
static int __init sn3193_leds_init(void)
{
	int ret;

	LEDS_DEBUG("[LED]%s\n", __func__);

	if (i2c_register_board_info(SN3193_BUS_NUM, &sn3193_i2c_board_info, 1) !=0) {
		printk(" cann't register i2c %s\n",__func__);
		return -1;
	}

	ret = platform_driver_register(&sn3193_leds_platform_driver);
	if (ret)
	{
		printk("[LED]%s:drv:E%d\n", __func__,ret);
		return ret;
	}

	return ret;
}

static void __exit sn3193_leds_exit(void)
{
	LEDS_DEBUG("[LED]%s\n", __func__);

	i2c_del_driver(&sn3193_i2c_driver);

	platform_driver_unregister(&sn3193_leds_platform_driver);

	sn3193_is_init = 0;
}

module_param(debug_enable, int,0644);

module_init(sn3193_leds_init);
module_exit(sn3193_leds_exit);

MODULE_AUTHOR("jixu@lenovo.com");
MODULE_DESCRIPTION("sn3193 led driver");
MODULE_LICENSE("GPL");


