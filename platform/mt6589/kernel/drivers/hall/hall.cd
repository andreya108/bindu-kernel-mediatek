/************************************************************
 * hall.c - Hall sensor driver
 * 
 * Copyright Lenovo MIDH
 * 
 * DESCRIPTION:
 *     This file provid the hall sensor drivers 
 *
 ***********************************************************/
#if defined(LENOVO_HALL_SENSOR_SUPPORT)

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>


#include <mach/irqs.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_boot.h>
#include <mtk_kpd.h>		
#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>

#define SET_HALL_STATUS _IOW('O', 10, unsigned int)
#define GET_HALL_STATUS _IOR('O', 11, unsigned int)

extern struct input_dev *kpd_input_dev;
extern BOOL is_early_suspended;
extern BOOL is_uboot_refresh;
extern BOOL BL_turn_on;

static u8 hall_cover_state = !CUST_EINT_MHALL_TYPE;
static BOOL has_hall_drv_file = false;

static void switch_hall_eint_handler(void);
static struct work_struct hall_work;
static long hall_ioctl (struct file *file, unsigned int cmd, unsigned long arg);

/******************************************************************************
Device driver structure
*****************************************************************************/
static struct platform_device hall_device =
{
    .name = "hall-sensor",
    .id   = -1,
};

static struct file_operations hall_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = hall_ioctl,
};

static struct miscdevice misc_hall = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "hall",
	.fops = &hall_fops,
};

//static void hall_cover_eint_handler(void);
//static DECLARE_TASKLET(hall_tasklet, hall_cover_eint_handler, 0);


/******************************************************************************
Global Definations
******************************************************************************/
static void hall_cover_eint_handler(struct work_struct *work)
{
	bool cover;
	u8 old_state = hall_cover_state;

	printk("[hall] is_early_suspended = %d, hall_cover_state = %d, BL_turn_on = %d\n", is_early_suspended, hall_cover_state, BL_turn_on);
	
	if((is_early_suspended) && (hall_cover_state))
	{
		hall_cover_state = 0;
		//misc_register(&misc_hall);
		mt_eint_set_polarity(CUST_EINT_MHALL_NUM, 1);
		mt_eint_unmask(CUST_EINT_MHALL_NUM);
		return;
	}
	else if((is_early_suspended == 0) && (hall_cover_state == 0) && (BL_turn_on))
	{
		hall_cover_state = 1;
		//misc_register(&misc_hall);
		mt_eint_set_polarity(CUST_EINT_MHALL_NUM, 0);
		mt_eint_unmask(CUST_EINT_MHALL_NUM);
		return;
	}

	hall_cover_state = !hall_cover_state;
	cover = (hall_cover_state == !!CUST_EINT_MHALL_TYPE);

        /** lenovo-sw zhengpf1 modify: switch status when hall open[2012.10.01] begin*/
        if (hall_cover_state == 1) {
            if (has_hall_drv_file) {
                misc_deregister(&misc_hall);
            } else {
                misc_register(&misc_hall);
            }
            has_hall_drv_file = !has_hall_drv_file;
        }
        /** lenovo-sw zhengpf1 modify end*/

	//if (hall_cover_state == 0) 
	//{
	//	misc_register(&misc_hall);
	//}
	//else 
	//{
	//	misc_deregister(&misc_hall);
	//}

	printk("[hall]report cover_status = %d, %s, next_pol = %d\n", cover, cover ? "close" : "open", old_state);
	input_report_key(kpd_input_dev, KPD_PWRKEY_MAP, 1);
	input_report_key(kpd_input_dev, KPD_PWRKEY_MAP, 0);
	input_sync(kpd_input_dev);

	mt_eint_set_polarity(CUST_EINT_MHALL_NUM, old_state);
	mt_eint_unmask(CUST_EINT_MHALL_NUM);
}

static void switch_hall_eint_handler(void)
{
  	printk("[Hall]switch_hall_eint_handler  \n");
        if(is_uboot_refresh)
	{
		mt_eint_set_polarity(CUST_EINT_MHALL_NUM, hall_cover_state);
		mt_eint_unmask(CUST_EINT_MHALL_NUM);
		hall_cover_state = !hall_cover_state;
		return;
	}
	schedule_work(&hall_work);
}
static void hall_init_hw(void)
{
	mt_set_gpio_mode(GPIO_MHALL_EINT_PIN, GPIO_ACCDET_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_MHALL_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_MHALL_EINT_PIN, GPIO_PULL_DISABLE); 

    	//mt65xx_eint_set_sens(CUST_EINT_MHALL_NUM, CUST_EINT_MHALL_SENSITIVE);
	//mt65xx_eint_set_polarity(CUST_EINT_MHALL_NUM, 0);
	mt_eint_set_hw_debounce(CUST_EINT_MHALL_NUM, CUST_EINT_MHALL_DEBOUNCE_CN);
	//mt65xx_eint_registration(CUST_EINT_MHALL_NUM, CUST_EINT_MHALL_DEBOUNCE_EN, CUST_EINT_MHALL_POLARITY, switch_hall_eint_handler, 0);
        mt_eint_registration(CUST_EINT_MHALL_NUM,CUST_EINT_MHALL_TYPE, switch_hall_eint_handler, 0);
	mt_eint_unmask(CUST_EINT_MHALL_NUM);  
}



static long hall_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("[Hall]enter hall_ioctl, cmd = %x\n", cmd);
	int databuf = 0;
	switch (cmd)
	{
		case SET_HALL_STATUS:
			break;
		case GET_HALL_STATUS:
			//if (copy_to_user((void __user *)arg, &hall_cover_state, sizeof(hall_cover_state)))
			//{
			//	return -EFAULT;
			//}
			return 0;
		default:
			printk("[Hall]ioctl: invalid cmd\n");
			break;
	}
	return 0;
}

/******************************************************************************
 * hall_mod_init
 * 
 * DESCRIPTION:
 *   Register the hall sensor driver ! 
 * 
 * PARAMETERS: 
 *   None
 * 
 * RETURNS: 
 *   None
 * 
 * NOTES: 
 *   0 : Success
 * 
 ******************************************************************************/
static s32 __devinit hall_mod_init(void)
{	
	s32 ret;

	ret = platform_device_register(&hall_device);
	if (ret != 0){
		printk("[Hall]Unable to register hall sensor device (%d)\n", ret);
        	return ret;
	}

	hall_init_hw();
	//ret=misc_register(&misc_hall);
	INIT_WORK(&hall_work,hall_cover_eint_handler);
	
	printk("[Hall]hall_mod_init Done \n");
 
	return 0;
}

/******************************************************************************
 * hall_mod_exit
 * 
 * DESCRIPTION: 
 *   Free the device driver ! 
 * 
 * PARAMETERS: 
 *   None
 * 
 * RETURNS: 
 *   None
 * 
 * NOTES: 
 *   None
 * 
 ******************************************************************************/
 
static void __exit hall_mod_exit(void)
{
	//misc_deregister(&misc_hall);
	platform_device_unregister(&hall_device);
	printk("[Hall]hall_mod_exit Done \n");
}

module_init(hall_mod_init);
module_exit(hall_mod_exit);
MODULE_AUTHOR("zhouwl@lenovo.com");
MODULE_DESCRIPTION("Lenovo Hall Sensor Driver");
MODULE_LICENSE("GPL");
#endif
