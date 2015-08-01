#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include "tpd_custom_synaptics_rmi4.h"
#include "cust_gpio_usage.h"
#include "tpd.h"
#include "synaptics_dsx_rmi4_i2c.h"
#include "SynaImage.h"
#include "SynaImage_BYD.h"
#include "SynaImage_BM.h"
#include "SynaImage_JTOUCH.h"

#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

/* < DTS2012031404176  linghai 20120314 begin */
#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT]=TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4]=TPD_KEYS_DIM;
#endif
/* DTS2012031404176  linghai 20120314 end> */

//LINE<JIRA_ID><DATE20130107><ftm set key>zenghaihui
static const int TPD_KEYSFACTORY[TPD_KEY_COUNT] =  {KEY_F1, KEY_F2, KEY_F3};

//add by huxin
#ifdef HAVE_TOUCH_KEY
const u16 touch_key_array[] = { KEY_MENU, KEY_HOMEPAGE, KEY_BACK};		//add by lishengli 20130514
#define MAX_KEY_NUM ( sizeof( touch_key_array )/sizeof( touch_key_array[0] ) )
#endif


#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

static struct point {
	int x;
	int raw_x;
	int y;
	int raw_y;
	int z;
	int status;
};

struct function_descriptor {
	u16 query_base;
	u16 cmd_base;
	u16 ctrl_base;
	u16 data_base;
	u8 intSrc;
#define FUNCTION_VERSION(x) ((x >> 5) & 3)
#define INTERRUPT_SOURCE_COUNT(x) (x & 7)

	u8 functionNumber;
};

struct tpd_data {
	struct i2c_client *client;
	struct function_descriptor f01;
	struct function_descriptor f11;
	struct function_descriptor f1a;
	u8 fn11_mask;
	u8 fn1a_mask;
	struct point *cur_points;
	struct point *pre_points;
	struct mutex io_ctrl_mutex;
	struct work_struct work;
	int f11_max_x, f11_max_y;
	u8 points_supported;
	u8 data_length;
	u8 current_page;
};

struct tpd_debug {
	u8 button_0d_enabled;
};

static int lcd_x = 0;
static int lcd_y = 0;

extern struct tpd_device *tpd;
static struct tpd_data *ts = NULL;
static struct tpd_debug *td = NULL;
static struct workqueue_struct *mtk_tpd_wq;
static u8 boot_mode;

struct delayed_work det_work;
struct workqueue_struct *det_workqueue;

/* Function extern */
static void tpd_eint_handler(void);
extern void mt_eint_unmask(unsigned int line);
extern void mt_eint_mask(unsigned int line);
extern void mt_eint_set_hw_debounce(unsigned int eintno, unsigned int ms);
extern kal_uint32 mt_eint_set_sens(unsigned int eintno, unsigned int sens);
extern void mt_eint_registration(unsigned int eintno, unsigned int Dbounce_En, void (EINT_FUNC_PTR)(void), unsigned int auto_umask);
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect(struct i2c_client *client,  struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static void tpd_work_func(struct work_struct *work);
extern int tpd_i2c_read_data(struct i2c_client *client, unsigned short addr, unsigned char *data, unsigned short length);
extern int tpd_i2c_write_data(struct i2c_client *client, unsigned short addr, unsigned char *data, unsigned short length);
static void tpd_down(int x, int y, int p);
static void tpd_up(int x, int y);
static int tpd_sw_power(struct i2c_client *client, int on);
static int tpd_clear_interrupt(struct i2c_client *client);
extern int synaptics_fw_updater(unsigned char *fw_data);

//static u8 get_config_version(void);

//BEGIN <touch panel> <DATE2013821> <version info> zhangxiaofei
extern int synaptics_get_fw_version(void);
extern char * synaptics_get_vendor_info(void);
//END <touch panel> <DATE2013821> <version info> zhangxiaofei

static const struct i2c_device_id tpd_id[] = {{DRIVER_NAME,0},{}};
/* < DTS2012040603460 gkf61766 20120406 begin */
static unsigned short force[] = {0,0x70,I2C_CLIENT_END,I2C_CLIENT_END}; 
/* < DTS2012040603460 gkf61766 20120406 end */
static const unsigned short * const forces[] = { force, NULL };
//static struct i2c_client_address_data addr_data = { .forces = forces, };
static struct i2c_board_info __initdata i2c_tpd={ I2C_BOARD_INFO(DRIVER_NAME, (0x38))};		//add by lishengli 20130514


static struct i2c_driver tpd_i2c_driver = {
	.driver = {
		.name = DRIVER_NAME,
	},
	.probe = tpd_probe,
	.remove = __devexit_p(tpd_remove),
	.id_table = tpd_id,
	.detect = tpd_detect,
	//.address_list = (const unsigned short*) forces, 
	//.address_data = &addr_data,
};


#ifdef CONFIG_HAS_EARLYSUSPEND
static ssize_t synaptics_rmi4_full_pm_cycle_show(struct device *dev,
		struct device_attribute *attr, char *buf);

static ssize_t synaptics_rmi4_full_pm_cycle_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);

#endif

#if PROXIMITY
static ssize_t synaptics_rmi4_f51_enables_show(struct device *dev,
		struct device_attribute *attr, char *buf);

static ssize_t synaptics_rmi4_f51_enables_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
#endif

static ssize_t synaptics_rmi4_f01_reset_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);

static ssize_t synaptics_rmi4_f01_productinfo_show(struct device *dev,
		struct device_attribute *attr, char *buf);

static ssize_t synaptics_rmi4_f01_flashprog_show(struct device *dev,
		struct device_attribute *attr, char *buf);

static ssize_t synaptics_rmi4_0dbutton_show(struct device *dev,
		struct device_attribute *attr, char *buf);

static ssize_t synaptics_rmi4_0dbutton_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);

/*
struct kobject *attr_kobj;

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct kobj_attribute synaptics_pm_cycle_attr = {
    .attr = {
        .name = "full_pm_cycle",
        .mode = (S_IRUGO | S_IWUGO),
    },
    .show = &synaptics_rmi4_full_pm_cycle_show,
    .store = &synaptics_rmi4_full_pm_cycle_store,
};
#endif

#if PROXIMITY
static struct kobj_attribute synaptics_f51_enables_attr = {
    .attr = {
        .name = "proximity_enables",
        .mode = (S_IRUGO | S_IWUGO),
    },
    .show = &synaptics_rmi4_f51_enables_show,
    .store = &synaptics_rmi4_f51_enables_store,
};
#endif

static struct kobj_attribute synaptics_show_error_attr = {
    .attr = {
        .name = "reset",
        .mode = (S_IRUGO | S_IWUGO),
    },
    .show = &synaptics_rmi4_show_error,
    .store = &synaptics_rmi4_f01_reset_store,   					
};

static struct kobj_attribute synaptics_productinfo_attr = {
    .attr = {
        .name = "productinfo",
        .mode = (S_IRUGO | S_IWUGO),
    },
    .show = &synaptics_rmi4_f01_productinfo_show,
    .store = &synaptics_rmi4_store_error,
};

static struct kobj_attribute synaptics_flashprog_attr = {
    .attr = {
        .name = "prog",
        .mode = (S_IRUGO | S_IWUGO),
    },
    .show = &synaptics_rmi4_f01_flashprog_show,
    .store = &synaptics_rmi4_store_error,
};

static struct kobj_attribute synaptics_0dbutton_attr = {
    .attr = {
        .name = "0dbutton",
        .mode = (S_IRUGO | S_IWUGO),
    },
    .show = &synaptics_rmi4_0dbutton_show,
    .store = &synaptics_rmi4_0dbutton_store,			
};

static struct attribute *syna_attrs[] = {
#ifdef CONFIG_HAS_EARLYSUSPEND
    &synaptics_pm_cycle_attr.attr,
#endif

#if PROXIMITY 
	&synaptics_f51_enables_attr.attr,
#endif
	&synaptics_show_error_attr.attr,
	&synaptics_productinfo_attr.attr,
	&synaptics_0dbutton_attr.attr,
	&synaptics_flashprog_attr.attr,
    NULL
};

static struct attribute_group syna_attr_group = {
    .attrs = syna_attrs,	
};
*/

struct kobject *properties_kobj_synap;
struct kobject *properties_kobj_driver;


static struct device_attribute attrs[] = {
#if defined CONFIG_HAS_EARLYSUSPEND
	__ATTR(full_pm_cycle, S_IRUGO,
			synaptics_rmi4_full_pm_cycle_show,
			synaptics_rmi4_full_pm_cycle_store),
#endif
#if PROXIMITY
	__ATTR(proximity_enables, S_IRUGO,
			synaptics_rmi4_f51_enables_show,
			synaptics_rmi4_f51_enables_store),
#endif
	__ATTR(reset, S_IRUGO,
			synaptics_rmi4_show_error,
			synaptics_rmi4_f01_reset_store),
	__ATTR(productinfo, S_IRUGO,
			synaptics_rmi4_f01_productinfo_show,
			synaptics_rmi4_store_error),
	__ATTR(flashprog, S_IRUGO,
			synaptics_rmi4_f01_flashprog_show,
			synaptics_rmi4_store_error),
	__ATTR(0dbutton, S_IRUGO,
			synaptics_rmi4_0dbutton_show,
			synaptics_rmi4_0dbutton_store),
};

static bool exp_fn_inited;
static struct mutex exp_fn_list_mutex;
static struct list_head exp_fn_list;

#if PROXIMITY
static struct synaptics_rmi4_f51_handle *f51;
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static ssize_t synaptics_rmi4_full_pm_cycle_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return 0;
	/*
	struct synaptics_rmi4_data *rmi4_data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%u\n",
			rmi4_data->full_pm_cycle);
			* */
			
}

static ssize_t synaptics_rmi4_full_pm_cycle_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	return 0;
	/*
	unsigned int input;
	struct synaptics_rmi4_data *rmi4_data = dev_get_drvdata(dev);

	if (sscanf(buf, "%u", &input) != 1)
		return -EINVAL;

	rmi4_data->full_pm_cycle = input > 0 ? 1 : 0;

	return count;
	* */
}
#endif

#if PROXIMITY
static ssize_t synaptics_rmi4_f51_enables_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int retval;
	unsigned char proximity_enables;

	if (!f51)
			return -ENODEV;

	retval = synaptics_rmi4_i2c_read(f51->rmi4_data,
			f51->proximity_enables_addr,
			&proximity_enables,
			sizeof(proximity_enables));
	if (retval < 0) {
		dev_err(dev,
				"%s: Failed to read proximity enables, error = %d\n",
				__func__, retval);
		return retval;
	}

	return snprintf(buf, PAGE_SIZE, "0x%02x\n",
			proximity_enables);
}

static ssize_t synaptics_rmi4_f51_enables_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int retval;
	unsigned int input;
	unsigned char proximity_enables;

	if (!f51)
			return -ENODEV;

	if (sscanf(buf, "%x", &input) != 1)
		return -EINVAL;

	proximity_enables = input;

	retval = synaptics_rmi4_i2c_write(f51->rmi4_data,
			f51->proximity_enables_addr,
			&proximity_enables,
			sizeof(proximity_enables));
	if (retval < 0) {
		dev_err(dev,
				"%s: Failed to write proximity enables, error = %d\n",
				__func__, retval);
		return retval;
	}

	return count;
}
#endif

static ssize_t synaptics_rmi4_f01_reset_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
/*	int retval;
	unsigned int reset;
	struct synaptics_rmi4_data *rmi4_data = dev_get_drvdata(dev);

	if (sscanf(buf, "%u", &reset) != 1)
		return -EINVAL;

	if (reset != 1)
		return -EINVAL;

	retval = synaptics_rmi4_reset_device(rmi4_data);
	if (retval < 0) {
		dev_err(dev,
				"%s: Failed to issue reset command, error = %d\n",
				__func__, retval);
		return retval;
	}

	return count;*/
	return 0;
}

static ssize_t synaptics_rmi4_f01_productinfo_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	/*struct synaptics_rmi4_data *rmi4_data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "0x%02x 0x%02x\n",
			(rmi4_data->rmi4_mod_info.product_info[0]),
			(rmi4_data->rmi4_mod_info.product_info[1]));*/

	return 0;
}

static ssize_t synaptics_rmi4_f01_flashprog_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	/*int retval;
	struct synaptics_rmi4_f01_device_status device_status;
	struct synaptics_rmi4_data *rmi4_data = dev_get_drvdata(dev);

	retval = synaptics_rmi4_i2c_read(rmi4_data,
			rmi4_data->f01_data_base_addr,
			device_status.data,
			sizeof(device_status.data));
	if (retval < 0) {
		dev_err(dev,
				"%s: Failed to read device status, error = %d\n",
				__func__, retval);
		return retval;
	}

	return snprintf(buf, PAGE_SIZE, "%u\n",
			device_status.flash_prog);*/
	return 0;
}

static ssize_t synaptics_rmi4_0dbutton_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
/*	struct synaptics_rmi4_data *rmi4_data = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n",
			rmi4_data->button_0d_enabled);*/
	return 0;
}

static ssize_t synaptics_rmi4_0dbutton_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
#if 0
	int retval;
	unsigned int input;
	unsigned char ii;
	unsigned char intr_enable;
	struct synaptics_rmi4_fn *fhandler;
	struct synaptics_rmi4_data *rmi4_data = dev_get_drvdata(dev);
	struct synaptics_rmi4_device_info *rmi;

	rmi = &(rmi4_data->rmi4_mod_info);

	if (sscanf(buf, "%u", &input) != 1)
		return -EINVAL;

	input = input > 0 ? 1 : 0;

	if (rmi4_data->button_0d_enabled == input)
		return count;

	list_for_each_entry(fhandler, &rmi->support_fn_list, link) {
		if (fhandler->fn_number == SYNAPTICS_RMI4_F1A) {
			ii = fhandler->intr_reg_num;

			retval = synaptics_rmi4_i2c_read(rmi4_data,
					rmi4_data->f01_ctrl_base_addr + 1 + ii,
					&intr_enable,
					sizeof(intr_enable));
			if (retval < 0)
				return retval;

			if (input == 1)
				intr_enable |= fhandler->intr_mask;
			else
				intr_enable &= ~fhandler->intr_mask;

			retval = synaptics_rmi4_i2c_write(rmi4_data,
					rmi4_data->f01_ctrl_base_addr + 1 + ii,
					&intr_enable,
					sizeof(intr_enable));
			if (retval < 0)
				return retval;
		}
	}

	rmi4_data->button_0d_enabled = input;
#endif
	return 0;
}


static int tpd_set_page(struct i2c_client *client,unsigned int address)
{
	int retval = 0;
	unsigned char retry;
	unsigned char buf[PAGE_SELECT_LEN];
	unsigned char page;

	page = ((address >> 8) & MASK_8BIT);
	if (page != ts->current_page) {
		buf[0] = MASK_8BIT;
		buf[1] = page;
		for (retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
			retval = i2c_master_send(client, buf, PAGE_SELECT_LEN);
			
			if (retval != PAGE_SELECT_LEN) {
				dev_err(&client->dev,
						"%s: I2C retry %d\n",
						__func__, retry + 1);
				msleep(20);
			} else {
				ts->current_page = page;
				break;
			}
		}
	} else {
		retval = PAGE_SELECT_LEN;
	}

	return retval;
}

int tpd_i2c_read_data(struct i2c_client *client,
		unsigned short addr, unsigned char *data, unsigned short length)
{
	int retval=0;
	u8 retry = 0;
	u8 *pData = data;
	int tmp_addr = addr;
	int left_len = length;
	
	mutex_lock(&(ts->io_ctrl_mutex));
	
	retval = tpd_set_page(client, addr);
	if (retval != PAGE_SELECT_LEN) {
		CTP_DBG("tpd_set_page fail, retval = %d\n", retval);
		retval = -EIO;
		goto exit;
	}
 #ifdef MT6577
	client->addr = client->addr & I2C_MASK_FLAG | I2C_WR_FLAG | I2C_RS_FLAG | I2C_ENEXT_FLAG;
 #endif
 //#ifdef MT6589
	u16 old_flag = client->ext_flag;
	client->addr = client->addr & I2C_MASK_FLAG ;
	client->ext_flag =client->ext_flag | I2C_WR_FLAG | I2C_RS_FLAG | I2C_ENEXT_FLAG;
 //#endif
	while (left_len > 0) {
		pData[0] = tmp_addr;
		
		for (retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
			if (left_len > 8) {
				retval = i2c_master_send(client, pData, (8 << 8 | 1));
			} else {
				retval = i2c_master_send(client, pData, (left_len << 8 | 1));
			}
			
			if (retval > 0) {
				break;
			} else {
				CTP_DBG("%s: I2C retry %d\n", __func__, retry + 1);
				msleep(20);
			}
		}
			
		left_len -= 8;
		pData += 8;
		tmp_addr += 8;
	}
 //#ifdef MT6589
	client->ext_flag = old_flag;
 //#endif
 #ifdef MT6577
	client->addr = client->addr & I2C_MASK_FLAG;
 #endif

exit:
	mutex_unlock(&(ts->io_ctrl_mutex));

	return retval;
}
EXPORT_SYMBOL(tpd_i2c_read_data);

int tpd_i2c_write_data(struct i2c_client *client,
		unsigned short addr, unsigned char *data, unsigned short length)

{
	int retval=0;
	u8 retry = 0;
	u8 *pData = data;
	u8 buf[5] = {0};
	int tmp_addr = addr;
	int left_len = length;
	
	mutex_lock(&(ts->io_ctrl_mutex));
	
	retval = tpd_set_page(client, addr);
	if (retval != PAGE_SELECT_LEN) {
		CTP_DBG("tpd_set_page fail, retval = %d\n", retval);
		retval = -EIO;
		goto exit;
	}

	while (left_len > 0) {	
		buf[0] = tmp_addr;
		for (retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
			if (left_len > 4) {	
				memcpy(buf+1, pData, 4);
				retval = i2c_master_send(client, buf, 5);
			} else {
				memcpy(buf+1, pData, left_len);
				retval = i2c_master_send(client, buf, left_len + 1);
			}
			
			if (retval > 0) {
				break;
			} else {
				CTP_DBG("%s: I2C retry %d\n", __func__, retry + 1);
				msleep(20);
			}
		}
			
		left_len -= 4;
		pData += 4;
		tmp_addr += 4;
	}


exit:
	mutex_unlock(&(ts->io_ctrl_mutex));

	return retval;
}

/*{
	int retval;
	unsigned char retry;
	unsigned char buf[length + 1];
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = length + 1,
			.buf = buf,
			.ext_flag = 0,
		},
	};

	mutex_lock(&(ts->io_ctrl_mutex));

	retval = tpd_set_page(client, addr);
	if (retval != PAGE_SELECT_LEN)
		goto exit;

	buf[0] = addr & MASK_8BIT;
	memcpy(&buf[1], &data[0], length);

	for (retry = 0; retry < SYN_I2C_RETRY_TIMES; retry++) {
		if (i2c_transfer(client->adapter, msg, 1) == 1) {
			retval = length;
			break;
		}
		dev_err(&client->dev,
				"%s: I2C retry %d\n",
				__func__, retry + 1);
		msleep(20);
	}

	if (retry == SYN_I2C_RETRY_TIMES) {
		dev_err(&client->dev,
				"%s: I2C write over retry limit\n",
				__func__);
		retval = -EIO;
	}

exit:
	mutex_unlock(&(ts->io_ctrl_mutex));

	return retval;
}*/
EXPORT_SYMBOL(tpd_i2c_write_data);


#if 0
 /**
 * synaptics_rmi4_f12_abs_report()
 *
 * Called by synaptics_rmi4_report_touch() when valid Function $12
 * finger data has been detected.
 *
 * This function reads the Function $12 data registers, determines the
 * status of each finger supported by the Function, processes any
 * necessary coordinate manipulation, reports the finger data to
 * the input subsystem, and returns the number of fingers detected.
 */
static int synaptics_rmi4_f12_abs_report(struct synaptics_rmi4_data *rmi4_data,
		struct synaptics_rmi4_fn *fhandler)
{
	int retval;
	unsigned char touch_count = 0; /* number of touch points */
	unsigned char finger;
	unsigned char fingers_supported;
	unsigned char finger_status;
	unsigned short data_addr;
	int x;
	int y;
	int wx;
	int wy;
	struct synaptics_rmi4_f12_finger_data *data;
	struct synaptics_rmi4_f12_finger_data *finger_data;

	fingers_supported = fhandler->num_of_data_points;
	data_addr = fhandler->full_addr.data_base;

	retval = synaptics_rmi4_i2c_read(rmi4_data,
			data_addr + fhandler->data1_offset,
			(unsigned char *)fhandler->data,
			fhandler->data_size);
	if (retval < 0)
		return 0;

	data = (struct synaptics_rmi4_f12_finger_data *)fhandler->data;

	for (finger = 0; finger < fingers_supported; finger++) {
		finger_data = data + finger;
		finger_status = finger_data->object_type_and_status & MASK_2BIT;

		/*
		 * Each 2-bit finger status field represents the following:
		 * 00 = finger not present
		 * 01 = finger present and data accurate
		 * 10 = finger present but data may be inaccurate
		 * 11 = reserved
		 */
		if ((finger_status == 0x01) || (finger_status == 0x02)) {
			x = (finger_data->x_msb << 8) | (finger_data->x_lsb);
			y = (finger_data->y_msb << 8) | (finger_data->y_lsb);
			wx = finger_data->wx;
			wy = finger_data->wy;

			if (rmi4_data->board->x_flip)
				x = rmi4_data->sensor_max_x - x;
			if (rmi4_data->board->y_flip)
				y = rmi4_data->sensor_max_y - y;

			dev_dbg(&rmi4_data->i2c_client->dev,
					"%s: Finger %d:\n"
					"status = 0x%02x\n"
					"x = %d\n"
					"y = %d\n"
					"wx = %d\n"
					"wy = %d\n",
					__func__, finger,
					finger_status,
					x, y, wx, wy);

			input_report_abs(rmi4_data->input_dev,
					ABS_MT_POSITION_X, x);
			input_report_abs(rmi4_data->input_dev,
					ABS_MT_POSITION_Y, y);
			input_report_abs(rmi4_data->input_dev,
					ABS_MT_TOUCH_MAJOR, max(wx, wy));
			input_report_abs(rmi4_data->input_dev,
					ABS_MT_TOUCH_MINOR, min(wx, wy));
			input_mt_sync(rmi4_data->input_dev);

			touch_count++;
		}
	}

	if (!touch_count)
		input_mt_sync(tpd->dev);

	input_sync(tpd->dev);

	return touch_count;
}
#endif


#if PROXIMITY
static int synaptics_rmi4_f51_report(struct synaptics_rmi4_data *rmi4_data,
		struct synaptics_rmi4_fn *fhandler)
{
	int retval;
	unsigned char touch_count = 0; /* number of touch points */
	unsigned short data_base_addr;
	int x;
	int y;
	int z;
	struct synaptics_rmi4_f51_data *data_reg;

	data_base_addr = fhandler->full_addr.data_base;
	data_reg = (struct synaptics_rmi4_f51_data *)fhandler->data;

	retval =tpd_i2c_read(rmi4_data,
			data_base_addr,
			data_reg->data,
			sizeof(data_reg->data));
	if (retval < 0)
		return 0;

	if (data_reg->data[0] == 0x00)
		return 0;

/**/
#if sdfsdfadf
	if (data_reg->finger_hover_det) {
		if (data_reg->hover_finger_z > 0) {
			x = (data_reg->hover_finger_x_4__11 << 4) |
					(data_reg->hover_finger_xy_0__3 & 0x0f);
			y = (data_reg->hover_finger_y_4__11 << 4) |
					(data_reg->hover_finger_xy_0__3 >> 4);
			z = HOVER_Z_MAX - data_reg->hover_finger_z;

			dev_dbg(&rmi4_data->i2c_client->dev,
					"%s: Hover finger:\n"
					"x = %d\n"
					"y = %d\n"
					"z = %d\n",
					__func__, x, y, z);

			input_report_abs(tpd->dev,
					ABS_MT_POSITION_X, x);
			input_report_abs(tpd->dev,
					ABS_MT_POSITION_Y, y);
#ifdef INPUT_MULTITOUCH
			input_report_abs(tpd->dev,
					ABS_MT_DISTANCE, z);
#endif
			input_mt_sync(tpd->dev);

			touch_count++;
		}
	}

	if (data_reg->air_swipe_det) {
		dev_dbg(&rmi4_data->i2c_client->dev,
				"%s: Swipe direction 0 = %d\n",
				__func__, data_reg->air_swipe_dir_0);
		dev_dbg(&rmi4_data->i2c_client->dev,
				"%s: Swipe direction 1 = %d\n",
				__func__, data_reg->air_swipe_dir_1);
	}

	if (data_reg->large_obj_det) {
		dev_dbg(&rmi4_data->i2c_client->dev,
				"%s: Large object activity = %d\n",
				__func__, data_reg->large_obj_act);
	}

	if (data_reg->hover_pinch_det) {
		dev_dbg(&rmi4_data->i2c_client->dev,
				"%s: Hover pinch direction = %d\n",
				__func__, data_reg->hover_pinch_dir);
	}
#endif

	if (!touch_count)
		input_mt_sync(tpd->dev);

	input_sync(tpd->dev);

	return touch_count;
}
#endif

#if 0 
 /**
 * synaptics_rmi4_f12_init()
 *
 * Called by synaptics_rmi4_query_device().
 *
 * This funtion parses information from the Function 12 registers and
 * determines the number of fingers supported, offset to the data1
 * register, x and y data ranges, offset to the associated interrupt
 * status register, interrupt bit mask, and allocates memory resources
 * for finger data acquisition.
 */
static int synaptics_rmi4_f12_init(struct synaptics_rmi4_data *rmi4_data,
		struct synaptics_rmi4_fn *fhandler,
		struct synaptics_rmi4_fn_desc *fd,
		unsigned int intr_count)
{
	int retval;
	unsigned char ii;
	unsigned char intr_offset;
	unsigned char ctrl_8_offset;
	unsigned char ctrl_23_offset;
	struct synaptics_rmi4_f12_query_5 query_5;
	struct synaptics_rmi4_f12_query_8 query_8;
	struct synaptics_rmi4_f12_ctrl_8 ctrl_8;
	struct synaptics_rmi4_f12_ctrl_23 ctrl_23;
	struct synaptics_rmi4_f12_finger_data *finger_data_list;

	fhandler->fn_number = fd->fn_number;
	fhandler->num_of_data_sources = fd->intr_src_count;

	retval = synaptics_rmi4_i2c_read(rmi4_data,
			fhandler->full_addr.query_base + 5,
			query_5.data,
			sizeof(query_5.data));
	if (retval < 0)
		return retval;

	ctrl_8_offset = query_5.ctrl0_is_present +
			query_5.ctrl1_is_present +
			query_5.ctrl2_is_present +
			query_5.ctrl3_is_present +
			query_5.ctrl4_is_present +
			query_5.ctrl5_is_present +
			query_5.ctrl6_is_present +
			query_5.ctrl7_is_present;

	ctrl_23_offset = ctrl_8_offset +
			query_5.ctrl8_is_present +
			query_5.ctrl9_is_present +
			query_5.ctrl10_is_present +
			query_5.ctrl11_is_present +
			query_5.ctrl12_is_present +
			query_5.ctrl13_is_present +
			query_5.ctrl14_is_present +
			query_5.ctrl15_is_present +
			query_5.ctrl16_is_present +
			query_5.ctrl17_is_present +
			query_5.ctrl18_is_present +
			query_5.ctrl19_is_present +
			query_5.ctrl20_is_present +
			query_5.ctrl21_is_present +
			query_5.ctrl22_is_present;

	retval = synaptics_rmi4_i2c_read(rmi4_data,
			fhandler->full_addr.ctrl_base + ctrl_23_offset,
			ctrl_23.data,
			sizeof(ctrl_23.data));
	if (retval < 0)
		return retval;

	/* Maximum number of fingers supported */
	fhandler->num_of_data_points = ctrl_23.max_reported_objects;

	retval = synaptics_rmi4_i2c_read(rmi4_data,
			fhandler->full_addr.query_base + 8,
			query_8.data,
			sizeof(query_8.data));
	if (retval < 0)
		return retval;

	/* Determine the presence of the Data0 register */
	fhandler->data1_offset = query_8.data0_is_present;

	retval = synaptics_rmi4_i2c_read(rmi4_data,
			fhandler->full_addr.ctrl_base + ctrl_8_offset,
			ctrl_8.data,
			sizeof(ctrl_8.data));
	if (retval < 0)
		return retval;

	/* Maximum x and y */
	rmi4_data->sensor_max_x =
			((unsigned short)ctrl_8.max_x_coord_lsb << 0) |
			((unsigned short)ctrl_8.max_x_coord_msb << 8);
	rmi4_data->sensor_max_y =
			((unsigned short)ctrl_8.max_y_coord_lsb << 0) |
			((unsigned short)ctrl_8.max_y_coord_msb << 8);
	dev_dbg(&rmi4_data->i2c_client->dev,
			"%s: Function %02x max x = %d max y = %d\n",
			__func__, fhandler->fn_number,
			rmi4_data->sensor_max_x,
			rmi4_data->sensor_max_y);

	rmi4_data->num_of_rx = ctrl_8.num_of_rx;
	rmi4_data->num_of_tx = ctrl_8.num_of_tx;

	fhandler->intr_reg_num = (intr_count + 7) / 8;
	if (fhandler->intr_reg_num != 0)
		fhandler->intr_reg_num -= 1;

	/* Set an enable bit for each data source */
	intr_offset = intr_count % 8;
	fhandler->intr_mask = 0;
	for (ii = intr_offset;
			ii < ((fd->intr_src_count & MASK_3BIT) +
			intr_offset);
			ii++)
		fhandler->intr_mask |= 1 << ii;

	/* Allocate memory for finger data storage space */
	fhandler->data_size = fhandler->num_of_data_points *
			sizeof(struct synaptics_rmi4_f12_finger_data);
	finger_data_list = kmalloc(fhandler->data_size, GFP_KERNEL);
	fhandler->data = (void *)finger_data_list;

	return retval;
}
#endif


#if PROXIMITY
static int synaptics_rmi4_f51_init(struct synaptics_rmi4_data *rmi4_data,
		struct synaptics_rmi4_fn *fhandler,
		struct synaptics_rmi4_fn_desc *fd,
		unsigned int intr_count)
{
	int retval;
	unsigned char ii;
	unsigned short intr_offset;
	unsigned char proximity_enable_mask = PROXIMITY_ENABLE;
	struct synaptics_rmi4_f51_query query_register;
	struct synaptics_rmi4_f51_data *data_register;

	fhandler->fn_number = fd->fn_number;
	fhandler->num_of_data_sources = fd->intr_src_count;

	fhandler->intr_reg_num = (intr_count + 7) / 8;
	if (fhandler->intr_reg_num != 0)
		fhandler->intr_reg_num -= 1;

	/* Set an enable bit for each data source */
	intr_offset = intr_count % 8;
	fhandler->intr_mask = 0;
	for (ii = intr_offset;
			ii < ((fd->intr_src_count & MASK_3BIT) +
			intr_offset);
			ii++)
		fhandler->intr_mask |= 1 << ii;

	retval = synaptics_rmi4_i2c_read(rmi4_data,
			fhandler->full_addr.query_base,
			query_register.data,
			sizeof(query_register.data));
	if (retval < 0)
		return retval;

	fhandler->data_size = sizeof(data_register->data);
	data_register = kmalloc(fhandler->data_size, GFP_KERNEL);
	fhandler->data = (void *)data_register;

	retval = synaptics_rmi4_i2c_write(rmi4_data,
			fhandler->full_addr.ctrl_base +
			query_register.control_register_count - 1,
			&proximity_enable_mask,
			sizeof(proximity_enable_mask));
	if (retval < 0)
		return retval;

	f51 = kmalloc(sizeof(*f51), GFP_KERNEL);
	f51->rmi4_data = rmi4_data;
	f51->proximity_enables_addr = fhandler->full_addr.ctrl_base +
			query_register.control_register_count - 1;

	return 0;
}

int synaptics_rmi4_proximity_enables(unsigned char enables)
{
	int retval;
	unsigned char proximity_enables = enables;

	if (!f51)
		return -ENODEV;

	retval = synaptics_rmi4_i2c_write(f51->rmi4_data,
			f51->proximity_enables_addr,
			&proximity_enables,
			sizeof(proximity_enables));
	if (retval < 0)
		return retval;

	return 0;
}
EXPORT_SYMBOL(synaptics_rmi4_proximity_enables);
#endif

static int tpd_rmi4_read_pdt(struct tpd_data *ts)
{
	int retval;
	unsigned char ii;
	unsigned char offset=0;
	unsigned char page_number;
	unsigned char intr_count = 0;
	unsigned char data_sources = 0;
	unsigned char f01_query[F01_STD_QUERY_LEN];
	unsigned char f11_query[F11_STD_QUERY_LEN];
	U32 f11_max_xy;
	u8  point_length;
	unsigned short pdt_entry_addr;
	unsigned short intr_addr;
	static u8 intsrc = 1;
	//struct synaptics_rmi4_f01_device_status status;
	struct synaptics_rmi4_fn_desc rmi_fd;

	/* Scan the page description tables of the pages to service */
	for (page_number = 0; page_number < PAGES_TO_SERVICE; page_number++) {
		for (pdt_entry_addr = PDT_START; pdt_entry_addr > PDT_END;
				pdt_entry_addr -= PDT_ENTRY_SIZE) {
			pdt_entry_addr |= (page_number << 8);
			retval = tpd_i2c_read_data(ts->client,
					pdt_entry_addr,
					(unsigned char *)&rmi_fd,
					sizeof(rmi_fd));
			if (retval < 0)
				return retval;

			if (rmi_fd.fn_number == 0) {
				dev_dbg(&ts->client->dev, "%s: Reached end of PDT\n", __func__);
				break;
			}

			dev_dbg(&ts->client->dev,
					"%s: F%02x found (page %d)\n",
					__func__, rmi_fd.fn_number,
					page_number);
						

			switch (rmi_fd.fn_number) {
			case SYNAPTICS_RMI4_F01:
				ts->f01.query_base = rmi_fd.query_base_addr;
				ts->f01.ctrl_base = rmi_fd.ctrl_base_addr;
				ts->f01.cmd_base = rmi_fd.cmd_base_addr;
				ts->f01.data_base =page_number | rmi_fd.data_base_addr;
				ts->f01.intSrc = intsrc++;
				ts->f01.functionNumber = rmi_fd.fn_number;

				break;
				
			case SYNAPTICS_RMI4_F11:
				if (rmi_fd.intr_src_count == 0)
					break;
				ts->f11.query_base = rmi_fd.query_base_addr;
				ts->f11.ctrl_base = rmi_fd.ctrl_base_addr;
				ts->f11.cmd_base = rmi_fd.cmd_base_addr;
				ts->f11.data_base = page_number |rmi_fd.data_base_addr;
				ts->f11.intSrc = intsrc++;
				ts->f11.functionNumber = rmi_fd.fn_number;
				ts->fn11_mask = 0;
				offset = intr_count%8;
				for(ii=offset;ii<(rmi_fd.intr_src_count+offset);ii++)
					ts->fn11_mask |= 1 <<ii;
				retval = tpd_i2c_read_data(ts->client,ts->f11.query_base,f11_query,sizeof(f11_query));
				if (retval < 0)
					return retval;
				CTP_DBG("f11 query base=%d\n",ts->f11.query_base);
				/* Maximum number of fingers supported */
				if ((f11_query[1] & MASK_3BIT) <= 4){
					ts->points_supported = (f11_query[1] & MASK_3BIT) + 1;
					CTP_DBG("points_supported=%d\n",ts->points_supported);
					}
				else if ((f11_query[1] & MASK_3BIT) == 5){
					ts->points_supported = 10;
					CTP_DBG("points_supported=%d\n",ts->points_supported);
					}
				retval = tpd_i2c_read_data(ts->client,ts->f11.ctrl_base+6, (char *)(&f11_max_xy), sizeof(f11_max_xy));
				if (retval < 0)
					return retval;

				/* Maximum x and y */
				ts->f11_max_x = f11_max_xy & 0xFFF;
				ts->f11_max_y = (f11_max_xy >> 16) & 0xFFF;


				ts->pre_points = kzalloc(ts->points_supported * sizeof(struct point), GFP_KERNEL);
				if (ts->pre_points == NULL) {
			        CTP_DBG("Error zalloc failed!\n");
					retval = -ENOMEM;
					return retval;
				}

				ts->cur_points = kzalloc(ts->points_supported * sizeof(struct point), GFP_KERNEL);
				if (ts->cur_points == NULL) {
			        CTP_DBG("Error zalloc failed!\n");
					retval = -ENOMEM;
					return retval;
				}

				ts->data_length = 3 + (2 * ((f11_query[5] & MASK_2BIT) == 0 ? 1 : 0));
				break;
				
			case SYNAPTICS_RMI4_F12:
				/*if (rmi_fd.intr_src_count == 0)
					break;

				retval = synaptics_rmi4_alloc_fh(&fhandler,
						&rmi_fd, page_number);
				if (retval < 0) {
					dev_err(&rmi4_data->i2c_client->dev,
							"%s: Failed to alloc for F%d\n",
							__func__,
							rmi_fd.fn_number);
					return retval;
				}

				retval = synaptics_rmi4_f12_init(rmi4_data,
						fhandler, &rmi_fd, intr_count);
				if (retval < 0)
					return retval;*/
				break;
			case SYNAPTICS_RMI4_F1A:
				if (rmi_fd.intr_src_count == 0)
					break;

				ts->f1a.query_base = rmi_fd.query_base_addr;
				ts->f1a.ctrl_base = rmi_fd.ctrl_base_addr;
				ts->f1a.cmd_base = rmi_fd.cmd_base_addr;
				ts->f1a.data_base = page_number | rmi_fd.data_base_addr;
				ts->f01.intSrc = intsrc++;
				ts->f01.functionNumber = rmi_fd.fn_number;

				td->button_0d_enabled = 1;

				ts->fn1a_mask = 0;
				offset = intr_count%8;
				for(ii=offset;ii<(rmi_fd.intr_src_count+offset);ii++)
					ts->fn1a_mask |= 1 <<ii;
				
				break;
				
#if PROXIMITY
			case SYNAPTICS_RMI4_F51:
				if (rmi_fd.intr_src_count == 0)
					break;

				/*retval = synaptics_rmi4_alloc_fh(&fhandler,
						&rmi_fd, page_number);
				if (retval < 0) {
					dev_err(&rmi4_data->i2c_client->dev,
							"%s: Failed to alloc for F%d\n",
							__func__,
							rmi_fd.fn_number);
					return retval;
				}

				retval = synaptics_rmi4_f51_init(rmi4_data,
						fhandler, &rmi_fd, intr_count);
				if (retval < 0)
					return retval;*/
				break;
#endif
			}
			if(rmi_fd.intr_src_count&0x03){
				intr_count += rmi_fd.intr_src_count&0x03;
			}
			
		}
	}

#if 0
flash_prog_mode:
	rmi4_data->num_of_intr_regs = (intr_count + 7) / 8;
	dev_dbg(&rmi4_data->i2c_client->dev,
			"%s: Number of interrupt registers = %d\n",
			__func__, rmi4_data->num_of_intr_regs);

	retval = synaptics_rmi4_i2c_read(rmi4_data,
			rmi4_data->f01_query_base_addr,
			f01_query,
			sizeof(f01_query));
	if (retval < 0)
		return retval;

	/* RMI Version 4.0 currently supported */
	rmi->version_major = 4;
	rmi->version_minor = 0;

	rmi->manufacturer_id = f01_query[0];
	rmi->product_props = f01_query[1];
	rmi->product_info[0] = f01_query[2] & MASK_7BIT;
	rmi->product_info[1] = f01_query[3] & MASK_7BIT;
	rmi->date_code[0] = f01_query[4] & MASK_5BIT;
	rmi->date_code[1] = f01_query[5] & MASK_4BIT;
	rmi->date_code[2] = f01_query[6] & MASK_5BIT;
	rmi->tester_id = ((f01_query[7] & MASK_7BIT) << 8) |
			(f01_query[8] & MASK_7BIT);
	rmi->serial_number = ((f01_query[9] & MASK_7BIT) << 8) |
			(f01_query[10] & MASK_7BIT);
	memcpy(rmi->product_id_string, &f01_query[11], 10);

	if (rmi->manufacturer_id != 1) {
		dev_err(&rmi4_data->i2c_client->dev,
				"%s: Non-Synaptics device found, manufacturer ID = %d\n",
				__func__, rmi->manufacturer_id);
	}

	memset(rmi4_data->intr_mask, 0x00, sizeof(rmi4_data->intr_mask));

	/*
	 * Map out the interrupt bit masks for the interrupt sources
	 * from the registered function handlers.
	 */
	list_for_each_entry(fhandler, &rmi->support_fn_list, link)
		data_sources += fhandler->num_of_data_sources;
	if (data_sources) {
		list_for_each_entry(fhandler, &rmi->support_fn_list, link) {
			if (fhandler->num_of_data_sources) {
				rmi4_data->intr_mask[fhandler->intr_reg_num] |=
						fhandler->intr_mask;
			}
		}
	}

	/* Enable the interrupt sources */
	for (ii = 0; ii < rmi4_data->num_of_intr_regs; ii++) {
		if (rmi4_data->intr_mask[ii] != 0x00) {
			dev_dbg(&rmi4_data->i2c_client->dev,
					"%s: Interrupt enable mask %d = 0x%02x\n",
					__func__, ii, rmi4_data->intr_mask[ii]);
			intr_addr = rmi4_data->f01_ctrl_base_addr + 1 + ii;
			retval = synaptics_rmi4_i2c_write(rmi4_data,
					intr_addr,
					&(rmi4_data->intr_mask[ii]),
					sizeof(rmi4_data->intr_mask[ii]));
			if (retval < 0)
				return retval;
		}
	}
#endif
	return 0;
}


/**
* synaptics_rmi4_detection_work()
*
* Called by the kernel at the scheduled time.
*
* This function is a self-rearming work thread that checks for the
* insertion and removal of other expansion Function modules such as
* rmi_dev and calls their initialization and removal callback functions
* accordingly.
*/
static void synaptics_rmi4_detection_work(struct work_struct *work)
{
	struct synaptics_rmi4_exp_fn *exp_fhandler, *next_list_entry;

	//queue_delayed_work(det_workqueue,&det_work,msecs_to_jiffies(EXP_FN_DET_INTERVAL));

	mutex_lock(&exp_fn_list_mutex);
	if (!list_empty(&exp_fn_list)) {
		list_for_each_entry_safe(exp_fhandler,
				next_list_entry,
				&exp_fn_list,
				link) {
			if ((exp_fhandler->func_init != NULL) &&
					(exp_fhandler->inserted == false)) {
				exp_fhandler->func_init(ts->client);
				exp_fhandler->inserted = true;
			} else if ((exp_fhandler->func_init == NULL) &&
					(exp_fhandler->inserted == true)) {
				exp_fhandler->func_remove(ts->client);
				list_del(&exp_fhandler->link);
				kfree(exp_fhandler);
			}
		}
	}
	mutex_unlock(&exp_fn_list_mutex);

	return;
}

/**
* synaptics_rmi4_new_function()
*
* Called by other expansion Function modules in their module init and
* module exit functions.
*
* This function is used by other expansion Function modules such as
* rmi_dev to register themselves with the driver by providing their
* initialization and removal callback function pointers so that they
* can be inserted or removed dynamically at module init and exit times,
* respectively.
*/
void synaptics_rmi4_new_function(enum exp_fn fn_type, bool insert,
		int (*func_init)(struct i2c_client *client),
		void (*func_remove)(struct i2c_client *client),
		void (*func_attn)(struct i2c_client *client,
		unsigned char intr_mask))
{
	struct synaptics_rmi4_exp_fn *exp_fhandler;

	if (!exp_fn_inited) {
		mutex_init(&exp_fn_list_mutex);
		INIT_LIST_HEAD(&exp_fn_list);
		exp_fn_inited = 1;
	}

	mutex_lock(&exp_fn_list_mutex);
	if (insert) {
		exp_fhandler = kzalloc(sizeof(*exp_fhandler), GFP_KERNEL);
		if (!exp_fhandler) {
			pr_err("%s: Failed to alloc mem for expansion function\n",
					__func__);
			goto exit;
		}
		exp_fhandler->fn_type = fn_type;
		exp_fhandler->func_init = func_init;
		exp_fhandler->func_attn = func_attn;
		exp_fhandler->func_remove = func_remove;
		exp_fhandler->inserted = false;
		list_add_tail(&exp_fhandler->link, &exp_fn_list);
	} else {
		list_for_each_entry(exp_fhandler, &exp_fn_list, link) {
			if (exp_fhandler->func_init == func_init) {
				exp_fhandler->inserted = false;
				exp_fhandler->func_init = NULL;
				exp_fhandler->func_attn = NULL;
				goto exit;
			}
		}
	}

exit:
	mutex_unlock(&exp_fn_list_mutex);

	return;
}
EXPORT_SYMBOL(synaptics_rmi4_new_function);

static void tpd_down(int x, int y, int p)
{
	input_report_abs(tpd->dev, ABS_PRESSURE, p);
	input_report_key(tpd->dev, BTN_TOUCH, 1);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(tpd->dev);

	#ifdef TPD_HAVE_BUTTON
	/*BEGIN PN: DTS2012051505359 ,modified by s00179437 , 2012-05-31*/
	if (FACTORY_BOOT == boot_mode || RECOVERY_BOOT == boot_mode)
	/*END PN: DTS2012051505359 ,modified by s00179437 , 2012-05-31*/
	{   
		tpd_button(x, y, 1);  
	}	
	#endif
	/* < DTS2012042803609 gkf61766 20120428 begin */
	CTP_DBG("=================>D---[%4d %4d %4d]\n", x, y, p);
	/* DTS2012042803609 gkf61766 20120428 end > */
	TPD_DOWN_DEBUG_TRACK(x,y);
}
 
static void tpd_up(int x, int y)
{
	//input_report_abs(tpd->dev, ABS_PRESSURE, 0);
	input_report_key(tpd->dev, BTN_TOUCH, 0);
	//input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
	//input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	//input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(tpd->dev);

	#ifdef TPD_HAVE_BUTTON
	/*BEGIN PN: DTS2012051505359 ,modified by s00179437 , 2012-05-31*/
	if (FACTORY_BOOT == boot_mode || RECOVERY_BOOT == boot_mode)
	/*END PN: DTS2012051505359 ,modified by s00179437 , 2012-05-31*/
	{   
		tpd_button(x, y, 0); 
	}   
	#endif
	/* < DTS2012042803609 gkf61766 20120428 begin */
	CTP_DBG("==================>U---[%4d %4d %4d]\n", x, y, 0);
	/* DTS2012042803609 gkf61766 20120428 end > */
	TPD_UP_DEBUG_TRACK(x,y);
}


static void tpd_work_func(struct work_struct *work)
{	
	u16 temp=0;
	u8 i = 0 ;
	u8 status = 0;
	u8 button = 0; 
	int retval = 0;
	u8 finger_status = 0;
	u8 finger_status_reg[3];
	u8 data[F11_STD_DATA_LEN];
	u8 num_of_finger_status_regs = 0;
	struct point *ppt = NULL;
	struct point ppt_v={0};	
      CTP_DBG("tpd_work_function read data to clear interrupt!!\n");

	//clear interrupt bit
	retval = tpd_i2c_read_data(ts->client, ts->f01.data_base + 1, &status, 1);
	if (retval < 0)
		return;


#if 0
	//LINE <touchpanel> <DATE2013826> <hardware button> zhangxiaofei
	if (status & 0x20) { /* key */
	//if (status & ts->fn1a_mask) { /* key */
		u8 button = 0; 
		static u8 id = 0;
		//tpd_i2c_write(ts->client, )
		//retval = tpd_i2c_read_data(ts->client, ts->f1a.data_base, &button, 1);
		retval = tpd_i2c_read_data(ts->client, 0x200, &button, 1);
		if (button)
		{
			/*for (i = 0; i < MAX_KEY_NUM; i++)
			{
				input_report_key(tpd->dev, touch_key_array[i], button & (0x01 << i));
				TPD_DEBUG("======>hx report button[%4d]\n",button & (0x01 << i));
			}*/
			for (i = 0; i < MAX_KEY_NUM; i++)
			{
			  if((button & (0x01 << i))==1){
			  	input_report_key(tpd->dev, touch_key_array[i], button & (0x01 << i));
			  	//tpd_down(100,850,50);
			  	id = i;
			  }
			  if((button & (0x01 << i))==2){
			  	input_report_key(tpd->dev, touch_key_array[i], button & (0x01 << i));
			  	//tpd_down(250,850,50);
			  	id = i;
			  }
			  if((button & (0x01 << i))==4){
			  	input_report_key(tpd->dev, touch_key_array[i], button & (0x01 << i));
			  	//tpd_down(400,850,50);
			  	id = i;
			  }
			}
		}else {
		    	input_report_key(tpd->dev, touch_key_array[id], 0);
		          //tpd_up(0,0);
			}
		input_sync(tpd->dev);

		return;
	}
	#endif
#ifdef HAVE_TOUCH_KEY
	if (status & ts->fn1a_mask) 
	{
		retval = tpd_i2c_read_data(ts->client, 0x200, &button, 1);
	}
	
	if (button)
	{
		for (i = 0; i < MAX_KEY_NUM; i++)
		{
			if (button & (0x01 << i))
			{
				ppt_v.x = tpd_keys_dim_local[i][0];
				ppt_v.y = tpd_keys_dim_local[i][1];
    			CTP_DBG("key down: %d!!\n",i);
				tpd_down(ppt_v.x, ppt_v.y, 20);
			}
		}
	} else {
		tpd_up(0,0);		
    	CTP_DBG("key up\n");
	}
#endif	

    CTP_DBG("status:%d!!\n",status);
//	if (status & 0x04) { /* point */
	if (status & ts->fn11_mask) {	
		tpd_i2c_read_data(ts->client, ts->f11.data_base, finger_status_reg, (ts->points_supported + 3) / 4);
		num_of_finger_status_regs = (ts->points_supported + 3) / 4;

		CTP_DBG("finger regs num=:%d!!\n",num_of_finger_status_regs);
		for (i = 0; i < ts->points_supported; i++) {
		//	if (!(i % 4))
				finger_status = finger_status_reg[i / 4];
		//	else
		//		finger_status = finger_status_reg[i / 4];

			
			finger_status = (finger_status >> ((i % 4) * 2)) & 3;

			ppt = &ts->cur_points[i];
			ppt->status = finger_status;

			if (0x01==finger_status || 0x02 == finger_status) {

				
				temp=ts->f11.data_base + num_of_finger_status_regs + i * ts->data_length;
					
				CTP_DBG("finger addr=:%d!!\n",temp);
				tpd_i2c_read_data(ts->client, ts->f11.data_base + num_of_finger_status_regs + i * ts->data_length, 
							data, ts->data_length);
				
				ppt->raw_x = ppt->x = (((u16)(data[0]) << 4) | (data[2] & 0x0F));
				ppt->raw_y = ppt->y = (((u16)(data[1]) << 4) | ((data[2] >> 4) & 0x0F));
				ppt->z = data[4];

				
				#ifdef TPD_HAVE_CALIBRATION
				//tpd_calibrate(&ppt->x, &ppt->y);
				#endif
				///ppt->x = ppt->raw_x * lcd_x /ts_x_max;
				//ppt->y = ppt->raw_y* lcd_y / 1970;
				
				tpd_down(ppt->x, ppt->y, ppt->z);
				CTP_DBG("finger index=:%d!!\n",(i+1));
				TPD_EM_PRINT(ppt->raw_x, ppt->raw_y, ppt->x, ppt->y, ppt->z, 1);
				
			} else {
				ppt = &ts->pre_points[i];
				if (ppt->status) {
					tpd_up(ppt->x, ppt->y);
					TPD_EM_PRINT(ppt->raw_x, ppt->raw_y, ppt->x, ppt->y, ppt->z, 0);
				}
			}
		}

		input_sync(tpd->dev);

		ppt = ts->pre_points;
		ts->pre_points = ts->cur_points;
		ts->cur_points = ppt;
	}

}

/*
static void tpd_work_func(struct work_struct *work)
{
	synaptics_rmi4_sensor_report(g_rmi4_data);
}
*/
static void tpd_eint_handler(void)
{
	TPD_DEBUG("TPD interrupt has been triggered\n");
	queue_work(mtk_tpd_wq, &ts->work);
}

static int tpd_sw_power(struct i2c_client *client, int on)
{
	int retval = 0;
	u8 device_ctrl = 0;

	retval = tpd_i2c_read_data(client,
				ts->f01.ctrl_base,
				&device_ctrl,
				sizeof(device_ctrl));
	if (retval < 0) {
		CTP_DBG("Error sensor can not wake up\n");
		goto out;
	}

	if (on)
	{
		device_ctrl = (device_ctrl & ~MASK_3BIT);
		device_ctrl = (device_ctrl | NO_SLEEP_OFF | NORMAL_OPERATION);
	
		retval = tpd_i2c_write_data(client,
				ts->f01.ctrl_base,
				&device_ctrl,
				sizeof(device_ctrl));
		if (retval < 0) {
			CTP_DBG("Error touch can not leave very-low power state\n");
			goto out;
		}
	
	} else {
	
		device_ctrl = (device_ctrl & ~MASK_3BIT);
		device_ctrl = (device_ctrl | NO_SLEEP_OFF | SENSOR_SLEEP);

		retval = tpd_i2c_write_data(client,
				ts->f01.ctrl_base,
				&device_ctrl,
				sizeof(device_ctrl));
		if (retval < 0) {
			CTP_DBG("Error touch can not enter very-low power state\n");
			goto out;
		}      
	}

out:
	return retval;	
}

static int tpd_detect (struct i2c_client *client,  struct i2c_board_info *info) 
{
	strcpy(info->type, TPD_DEVICE);    
	return 0;
}

static int __devexit tpd_remove(struct i2c_client *client)
{
	TPD_DEBUG("TPD removed\n");
	return 0;
}

static int tpd_clear_interrupt(struct i2c_client *client)
{
	int retval = 0;
	u8 status = 0;
	retval = tpd_i2c_read_data(client, ts->f01.data_base + 1, &status, 1);
	if (retval < 0){
		dev_err(&client->dev,
				"%s: Failed to enable attention interrupt\n",
				__func__);
	}
	return retval;
}


static struct task_struct * synaptics_update_thread;
static  int update_firmware_thread(void *priv)
{
	char *product_id = NULL;
	TPD_DEBUG("[synaptics] enter update_firmware_thread\n");
	synaptics_rmi4_detection_work(NULL);
    product_id = synaptics_get_vendor_info();
    CTP_DBG("tpd product id: %s\n",product_id);
    #if 1
    if(product_id)
    {
        if( 0 == memcmp(product_id, "BYD", 3))
        {
            CTP_DBG("BYD TP ready to upgrade \n");
            synaptics_fw_updater(synaImage_BYD);
        }
        else if( 0 == memcmp(product_id, "BM", 2))
        {
            //default BM TP
            CTP_DBG("BM TP ready to upgrade \n");
            synaptics_fw_updater(synaImage_BM);
        }
        else if( 0 == memcmp(product_id, "JTOUCH", 6))
        {
            CTP_DBG("JTOUCH TP ready to upgrade \n" );
            synaptics_fw_updater(synaImage_JTOUCH);
        }
        else
        {
            CTP_DBG("no match TP: \n", product_id);
        }
    }
    else
    {
        CTP_DBG("UNKNOWN TP \n");
    }
    #endif
    kthread_should_stop();
	return NULL;
	
}

int synaptics_auto_upgrade(void)
{
	int err;
	//if(get_match_firmware() == 1)//fts_ctpm_get_upg_ver() != panel_version)
	//{
		synaptics_update_thread = kthread_run(update_firmware_thread, 0, TPD_DEVICE);
		if (IS_ERR(synaptics_update_thread)) {
		        err = PTR_ERR(synaptics_update_thread);
		        TPD_DEBUG(TPD_DEVICE " failed to create update_firmware_thread thread: %d\n", err);
		    }
	//}
	//else 
		//TPD_DEBUG("Do not excute auto upgrade ..\n");
	return err;
}


static void tpd_poweron()
{
	CTP_DBG("Power On\n");
	hwPowerOn(TPD_POWER_SOURCE_CUSTOM , VOL_2800, "TP");
	hwPowerOn(TPD_POWER_SOURCE_1800,  VOL_1800, "TP");
}

static void tpd_poweroff()
{
	CTP_DBG("Power Off\n");
       hwPowerDown(TPD_POWER_SOURCE_CUSTOM , "TP");
       hwPowerDown(TPD_POWER_SOURCE_1800,  "TP");
}

static int tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	u8 ii;
	u8 attr_count,buf[2];
	u8 status = 0;
	int retval;
	int i;
	u16 TP_Max_X =0;
	u16 TP_Max_Y =0;
	u16 tp_x_for_lcd=0;
	u16 tp_y_for_lcd=0;
//	struct synaptics_rmi4_f1a_handle *f1a;
	struct synaptics_rmi4_fn *fhandler;
	struct synaptics_rmi4_data *rmi4_data;
	struct synaptics_rmi4_device_info *rmi;
    //char *product_id = NULL;

 //   mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
 //   mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
 //   mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
	//LINE <touchpanel> <DATE2013821> <project custom power on> zhangxiaofei
//	hwPowerOn(MT65XX_POWER_LDO_VGP5, VOL_2800, "TP");
	tpd_poweron();
	msleep(10);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
       mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
       mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(50);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
       mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
       mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(50);
    
       if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA)) {
		CTP_DBG("SMBus byte data not supported\n");
		return -EIO;
	}
	ts = kzalloc(sizeof(struct tpd_data), GFP_KERNEL);
	if (!ts) {
		CTP_DBG("Failed to alloc mem for tpd_data\n");
		return -ENOMEM;
	}
	td = kzalloc(sizeof(struct tpd_debug), GFP_KERNEL);
	if (!td) {
		CTP_DBG("Failed to alloc mem for tpd_debug\n");
	}
	ts->client = client;
	mutex_init(&(ts->io_ctrl_mutex));

	retval = tpd_rmi4_read_pdt(ts);
	if (retval < 0) {
		CTP_DBG("Failed to query device\n");
		goto err_query_device;
	}
	if (!exp_fn_inited) {
		mutex_init(&exp_fn_list_mutex);
		INIT_LIST_HEAD(&exp_fn_list);
		exp_fn_inited = 1;
	}
    
	INIT_WORK(&ts->work, tpd_work_func);
	mtk_tpd_wq = create_singlethread_workqueue("mtk_tpd_wq");
	if (!mtk_tpd_wq)
	{
		CTP_DBG("Error Could not create work queue mtk_tpd_wq: no memory");
		retval = -ENOMEM;
		goto error_wq_creat_failed; 
	}
	tpd_clear_interrupt(client);

	properties_kobj_synap = kobject_create_and_add("synapics", NULL);
	
/*	//480
       buf[0]=0xe0;
       buf[1]=0x01;
	tpd_i2c_write_data(client, ts->f11.ctrl_base+6, &buf[0],1);
	tpd_i2c_write_data(client, ts->f11.ctrl_base+7, &buf[1],1);
	
	//870
	buf[0]=0x61;
       buf[1]=0x03;
	tpd_i2c_write_data(client, ts->f11.ctrl_base+8,&buf[0],1);
	tpd_i2c_write_data(client, ts->f11.ctrl_base+9,&buf[1],1);
*/	

#ifdef HAVE_TOUCH_KEY
	CTP_DBG("HAVE_TOUCH_KEY\n");

	 set_bit(EV_KEY, tpd->dev->evbit);
	for(i=0;i<MAX_KEY_NUM;i++)
	   __set_bit(touch_key_array[i], tpd->dev->keybit);
#endif
	
#ifdef TPD_UPDATE_FIRMWARE
CTP_DBG("TPD_UPDATE_FIRMWARE\n");
//	det_workqueue =	create_singlethread_workqueue("rmi_det_workqueue");
//	INIT_DELAYED_WORK(&det_work,synaptics_rmi4_detection_work);
//	queue_delayed_work(det_workqueue,&det_work,msecs_to_jiffies(EXP_FN_DET_INTERVAL));
	//BEGIN <synaptics touch panel> <date20131029><tp auto update> zhangxiaofei
    synaptics_auto_upgrade();
	//END <synaptics touch panel> <date20131029><tp auto update> zhangxiaofei
#endif	

	properties_kobj_driver = kobject_create_and_add("driver", properties_kobj_synap);
	/*
	for (attr_count = 0; attr_count < ARRAY_SIZE(attrs); attr_count++) {
		//retval = sysfs_create_file(&tpd->dev->dev.kobj, &attrs[attr_count].attr);
		retval = sysfs_create_file(properties_kobj_driver, &attrs[attr_count].attr);
		   
		if (retval < 0) {
			dev_err(&client->dev, "%s: Failed to create sysfs attributes\n", __func__);
			goto err_sysfs;
		}
	}
	* */
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
      //BEGIN <touch panel> <DATE2013821> <removed> zhangxiaofei
	//mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	//mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_handler, 1); 
       //END <touch panel> <DATE2013821> <removed> zhangxiaofei
	mt_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_TYPE, tpd_eint_handler, 1); 
	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	tpd_load_status = 1;
	CTP_DBG("%s: TouchPanel Device Probe %s\n", __func__, (retval < 0) ? "FAIL" : "PASS");
    return 0;
	
err_sysfs:
	//for (attr_count--; attr_count >= 0; attr_count--) {
	//	sysfs_remove_file(properties_kobj_driver, &attrs[attr_count].attr);
	//}
	
error_wq_creat_failed:
err_query_device:
	kfree(td);

err_tpd_data:

    kfree(ts);
//    hwPowerDown(MT65XX_POWER_LDO_VGP5, "TP");
    tpd_poweroff();
    return retval;
}


static int tpd_local_init(void)
{
       int i;
	if(i2c_add_driver(&tpd_i2c_driver)!=0)
	{
		CTP_DBG("Error unable to add i2c driver.\n");
		return -1;
	}
    
#ifdef TPD_HAVE_BUTTON
CTP_DBG("TPD_HAVE_BUTTON\n");
         if (FACTORY_BOOT == get_boot_mode())
         {
             for (i = 0; i < TPD_KEY_COUNT ; i++)
                 tpd_keys_local[i] = TPD_KEYSFACTORY[i];
         }
	tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);
#endif

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
CTP_DBG("TPD_DO_WARP\n");
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif 

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
CTP_DBG("TPD_HAVE_CALIBRATION\n");
    memcpy(tpd_calmat, tpd_def_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
#endif  

	boot_mode = get_boot_mode();
	if (boot_mode == 3) boot_mode = NORMAL_BOOT;

	CTP_DBG("end %s, %d\n", __FUNCTION__, __LINE__);  
	tpd_type_cap = 1;
    return 0; 
 }

static void tpd_resume(struct early_suspend *h)
{
	CTP_DBG("TPD wake up\n");
	TPD_DEBUG("TPD wake up\n");

/*
#ifdef TPD_CLOSE_POWER_IN_SLEEP	
	hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP"); 
#else
#ifdef MT6573
	mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif	
	msleep(100);

	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(1);  
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
#endif
*/
/*   	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(50);  
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);*/
	
	tpd_sw_power(ts->client, 1);
	tpd_clear_interrupt(ts->client);
	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
	
	return;
}
 
static void tpd_suspend(struct early_suspend *h)
{
	TPD_DEBUG("TPD enter sleep\n");
	CTP_DBG("TPD enter sleep\n");
	mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	
 /*   mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);*/
   
	tpd_sw_power(ts->client, 0);

/*	 
#ifdef TPD_CLOSE_POWER_IN_SLEEP	
	hwPowerDown(TPD_POWER_SOURCE,"TP");
#else
i2c_smbus_write_i2c_block_data(i2c_client, 0xA5, 1, &data);  //TP enter sleep mode
#ifdef MT6573
mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif

#endif
*/
	 return;
 } 



//LINE<JIRA_ID><DATE20130422><add multi tp>zenghaihui
int S3202_tpd_get_fw_version( void )
{
    return synaptics_get_fw_version();
}

void S3202_tpd_get_fw_vendor_name(char * fw_vendor_name)
{
    char *product_id = synaptics_get_vendor_info();

    if(product_id)
    {
        sprintf(fw_vendor_name, "%s", product_id);
    }
    else
    {
        sprintf(fw_vendor_name, "%s", "UNKNOWN");
    }
}

 
static struct tpd_driver_t tpd_device_driver = {
	.tpd_device_name = DRIVER_NAME,
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
	.tpd_have_button = 1,
#else
	.tpd_have_button = 0,
#endif		
    //LINE<JIRA_ID><DATE20130422><add multi tp>zenghaihui
    .tpd_get_fw_version = S3202_tpd_get_fw_version,
    .tpd_get_fw_vendor_name = S3202_tpd_get_fw_vendor_name,
};

static int __init tpd_driver_init(void)
{
	CTP_DBG("Synaptics touch panel driver init.\n");
	i2c_register_board_info(0, &i2c_tpd, 1);
	if(tpd_driver_add(&tpd_device_driver) < 0)
		CTP_DBG("Error Add synaptics driver failed\n");
	return 0;
}

static void __exit tpd_driver_exit(void)
{
	CTP_DBG("Synaptics touch panel driver exit\n");
	tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

MODULE_DESCRIPTION("Mediatek Synaptics Driver");

