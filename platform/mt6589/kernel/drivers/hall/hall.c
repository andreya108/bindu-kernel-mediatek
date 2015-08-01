/************************************************************
 * hall.c - Hall sensor driver
 * 
 * Copyright Lenovo MIDH
 * 
 * DESCRIPTION:
 *     This file provid the hall sensor drivers 
 *
 ***********************************************************/
#ifdef LENOVO_HALL_SENSOR_SUPPORT
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/switch.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>

//#ifdef MT6577 -david
#include <mtk_kpd.h>		/* custom file */
//#include <cust_kpd.h>
//#endif

#include <mach/irqs.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>

#define SET_HALL_STATUS _IOW('O', 10, unsigned int)
#define GET_HALL_STATUS _IOR('O', 11, unsigned int)

#define DEBUG
#ifdef DEBUG
#define DOCK_DEBUG printk
#else
#define DOCK_DEBUG
#endif

#define DOCK_ERR pr_err

extern struct input_dev *kpd_input_dev;
extern BOOL is_early_suspended;
extern BOOL is_uboot_refresh;
extern BOOL BL_turn_on;

static u8 hall_cover_closed_state = !CUST_EINT_MHALL_TYPE;
static BOOL has_hall_drv_file = false;
static BOOL driver_mode_legacy = false;
static struct switch_dev hall_data;

static void hall_cover_closed_eint_handler(void);

static struct work_struct hall_eint_work;
static struct workqueue_struct * hall_eint_workqueue = NULL;

static bool suspended = false;

/******************************************************************************
Device driver structure
*****************************************************************************/
static struct platform_device hall_device =
{
    .name = "hall-sensor",
    .id   = -1,
};


/******************************************************************************
Global Definations
******************************************************************************/
static void hall_suspend(struct early_suspend *h)
{
	suspended = true;
}

static void hall_resume(struct early_suspend *h)
{
	suspended = false;
}

static void hall_cover_closed_eint_handler(void)
{
    
    queue_work(hall_eint_workqueue, &hall_eint_work);
	
}

static void hall_eint_work_callback(struct work_struct *work)
{
	bool cover_closed;
	u8 old_state = hall_cover_closed_state;
	
	mt_eint_mask(CUST_EINT_MHALL_NUM);
//	DOCK_DEBUG("[hall%s] hall_cover_closed_state = %d\n", driver_mode_legacy?"-legacy":"",hall_cover_closed_state );
    hall_cover_closed_state = !mt_get_gpio_in(GPIO_MHALL_EINT_PIN);

	cover_closed = hall_cover_closed_state;
	printk("[hall%s]report cover_closed_status=%d (%s), next_pol=%d, suspended=%s\n", driver_mode_legacy?"-legacy":"", cover_closed, cover_closed ? "close" : "open", old_state, suspended ? "yes" : "no");

	if ( old_state ==  hall_cover_closed_state )
	{
		DOCK_ERR("[hall%s] no hall state changed!!!\n",driver_mode_legacy?"-legacy":"");
	} else {
		if (driver_mode_legacy) {
			if (cover_closed != suspended) 
			{
				printk("[hall%s] sending powerkey event!\n",driver_mode_legacy?"-legacy":"");
				input_report_key(kpd_input_dev, KPD_PWRKEY_MAP, 1);
				input_report_key(kpd_input_dev, KPD_PWRKEY_MAP, 0);
				input_sync(kpd_input_dev);
			} else {
				printk("[hall%s] skipping state change!\n",driver_mode_legacy?"-legacy":"");
			}

		}
	}

 	if (!driver_mode_legacy) 
 	{
		switch_set_state((struct switch_dev *)&hall_data, hall_cover_closed_state);
	}

	mt_eint_set_polarity(CUST_EINT_MHALL_NUM, old_state);
	mt_eint_unmask(CUST_EINT_MHALL_NUM);
}

static void hall_init_hw(void)
{
	
	mt_set_gpio_mode(GPIO_MHALL_EINT_PIN, GPIO_ACCDET_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_MHALL_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_MHALL_EINT_PIN, GPIO_PULL_DISABLE); 
	
	mt_eint_set_hw_debounce(CUST_EINT_MHALL_NUM, CUST_EINT_MHALL_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_MHALL_NUM, CUST_EINT_MHALL_TYPE, hall_cover_closed_eint_handler, 0);
	mt_eint_unmask(CUST_EINT_MHALL_NUM); 

}

static int hall_probe(struct platform_device *dev)	
{
	DOCK_DEBUG("[hall] %s  \n", __func__);
	#if 1
	hall_data.name = "hall";
        hall_data.index = 0;
        hall_data.state = hall_cover_closed_state;
        printk("hall_probe 1\n");
        switch_dev_register(&hall_data);
        printk("hall_probe 2\n");
	
	hall_eint_workqueue = create_singlethread_workqueue("hall_eint");
        printk("hall_probe 3\n");
        INIT_WORK(&hall_eint_work, hall_eint_work_callback);
       
        printk("hall_probe 4\n");
	hall_init_hw();
        printk("hall_probe\n");

	#endif
	return 0;
}
static int hall_remove(struct platform_device *dev)	
{
       #if 1
	destroy_workqueue(hall_eint_workqueue);
	switch_dev_unregister(&hall_data);
	#endif
	return 0;
}


static ssize_t hall_legacy_switch_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	size_t count = 0;

	count += sprintf(buf, "%d\n", driver_mode_legacy);

	return count;
}

static ssize_t hall_legacy_switch_dump(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	if (buf[0] >= '0' && buf[0] <= '2' && buf[1] == '\n')
        driver_mode_legacy = buf[0] == '1';

	return count;
}

static DEVICE_ATTR(legacy_mode, (S_IWUSR|S_IRUGO),
	hall_legacy_switch_show, hall_legacy_switch_dump);

static struct platform_driver hall_driver= {
        .probe      = hall_probe,
        .remove     = hall_remove,
       	.driver     = {
        	.name       = "hall-sensor",
        },
};

static struct early_suspend early_suspend_desc = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1,
	.suspend = hall_suspend,
	.resume = hall_resume,
};
struct kobject *hall_kobj;

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
        printk("hall_mod_init\n");
	ret = platform_driver_register(&hall_driver);
	if (ret != 0){
		DOCK_ERR("[Hall]Unable to register hall sensor device (%d)\n", ret);
        	return ret;
	}

	hall_kobj = kobject_create_and_add("hall", NULL);
	if (!hall_kobj)
		return -ENOMEM;

	ret = sysfs_create_file(hall_kobj, &dev_attr_legacy_mode.attr);
	if (ret) {
		pr_warn("%s: sysfs_create_file failed for hall.legacy_mode\n", __func__);
	}

	register_early_suspend(&early_suspend_desc);

	DOCK_DEBUG("[Hall]hall_mod_init Done \n");
 
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
	
	platform_driver_unregister(&hall_driver);
	sysfs_remove_group(hall_kobj, &dev_attr_legacy_mode);
	unregister_early_suspend(&early_suspend_desc);
	DOCK_DEBUG("[Hall]hall_mod_exit Done \n");
}

module_init(hall_mod_init);
module_exit(hall_mod_exit);
MODULE_AUTHOR("Lenovo");
MODULE_DESCRIPTION("Lenovo Hall Sensor Driver");
MODULE_LICENSE("GPL");
#endif
