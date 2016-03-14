#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>

#include "tpd_custom_hx8526.h"
#include <linux/workqueue.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#include "cust_gpio_usage.h"
#include <linux/hwmsen_helper.h>

//dma
#include <linux/dma-mapping.h>

#define LCT_MTK_CTP_INFO_SUPPORT //luxiaotong add for LCM info 
#ifdef LCT_MTK_CTP_INFO_SUPPORT
#include <linux/proc_fs.h>
#define CTP_PROC_FILE "ctp_version"
static struct proc_dir_entry *g_ctp_proc = NULL;
#endif
#define TPD_CLOSE_POWER_IN_SLEEP
extern struct tpd_device *tpd;
 
static struct i2c_client *i2c_client = NULL;
static struct task_struct *thread = NULL;
 
static DECLARE_WAIT_QUEUE_HEAD(waiter);
static DEFINE_MUTEX(i2c_access);
 
static void tpd_eint_interrupt_handler(void);

extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);


 
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);
void himax_HW_reset(void);
static int himax_ts_poweron(void);

unsigned char IC_TYPE = 0;
struct work_struct *himax_wq;
#define HX_85XX_A_SERIES_PWON 1
#define HX_85XX_D_SERIES_PWON 2

static unsigned char * HIMAX_FW=NULL;
static int fw_size = -1;

enum wk_wdt_type {
	WK_WDT_LOC_TYPE,
	WK_WDT_EXT_TYPE,
	WK_WDT_LOC_TYPE_NOLOCK,
	WK_WDT_EXT_TYPE_NOLOCK,	
};

extern void mpcore_wdt_restart(enum wk_wdt_type type);
extern void mtk_wdt_restart(enum wk_wdt_type type);
//Himax: Function Define
#define Android4_0 1		//android4.0 = 1, android2.3 = 0
//#define HIMAX_BUTTON 0
#define HIMAX_DEBUG 0
#define HIMAX_UPGRADE 1
#ifdef LCT_PROJECT_ZNT_AW990_A01
#define HIMAX_UPGRADE_WITH_IFILE 0
#else
#define HIMAX_UPGRADE_WITH_IFILE 1
#endif

#define HIMAX_A01_AUTO_UPGRADE 0

#ifdef __MRS_AW990A_A01_DRV__
#define HIMAX_D32_AUTO_UPGRADE 1
#else
#define HIMAX_D32_AUTO_UPGRADE 1
#endif

#define HIMAX_A01_TP_FW_CUSTOMER_ID_PLATFORM            "longcheer"
#define HIMAX_A01_TP_FW_CUSTOMER_Doov      				"Doov"
#define HIMAX_A01_TP_FW_CUSTOMER_zhongnuo      			"zhongnuo"

#define HIMAX_D32_TP_FW_CUSTOMER_ID_PLATFORM            "NA"
#define HIMAX_D32_TP_FW_CUSTOMER_zhongnuo             "zhongnuo"

#ifdef __MRS_AW990A_A01_DRV__
#define HIMAX_SYS_NODE_RIGHT     0644
#else
#define HIMAX_SYS_NODE_RIGHT     0777
#endif




#define HIMAX_RAWDATA 0
#define HIMAX_CHECKSUM 1
#define HIMAX_5POINT_SUPPORT 1

#define ChangeIref1u 0
#define I2C_MASTER_CLOCK 400
//Himax: ESD
#define ENABLE_CHIP_RESET_MACHINE
#define ENABLE_CHIP_STATUS_MONITOR

#define HX_ESD_WORKAROUND 
//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------start
#ifdef HX_ESD_WORKAROUND
	static u8 ESD_RESET_ACTIVATE = 1;
	static u8 ESD_COUNTER = 0;
	static int ESD_COUNTER_SETTING = 3;
	static uint8_t ESD_UP_COUNTER = 0;
	
	static u8 NO_ESD_RESET = 0;

	void ESD_HW_REST(void);
#endif
//----[HX_ESD_WORKAROUND]-------------------------------------------------------------------------------end

//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start
#ifdef ENABLE_CHIP_STATUS_MONITOR
	struct delayed_work himax_chip_monitor;
	int running_status;
#endif
//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end

//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
#ifdef ENABLE_CHIP_RESET_MACHINE
	int init_success;
	int retry_time;
	struct delayed_work himax_chip_reset_work;
#endif
//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end

static int  himax_hang_shaking();
static u8	IC_STATUS_CHECK;



#if HIMAX_RAWDATA
#define MTK6589_DMA
#endif

#ifdef MTK6589_DMA
static uint8_t *gpDMABuf_va = NULL;
static uint32_t gpDMABuf_pa = NULL;
#endif



//Himax: Set Point Number and Resolution
#if HIMAX_5POINT_SUPPORT
const u8 PT_NUM_MAX = 5;
#else
const u8 PT_NUM_MAX = 4;
#endif
const u16 RESOLUTION_Y = 1372;
const u16 RESOLUTION_X = 720;
//Himax: Set Flash Pre-Patch
static char PrePatch = 0x06;

//Himax: Define Touch Number
static int tpd_flag = 0;
static int tpd_halt=0;
static int point_num = 0;
static int P_ID = 0;
static int p_soft_key = 0xFF;
static uint8_t diag_command = 0;

#define TMP_LCM_WIDTH  simple_strtoul(LCM_WIDTH, NULL, 0)
#define TMP_LCM_HEIGHT  simple_strtoul(LCM_HEIGHT, NULL, 0)

#define TPD_OK 0

//Himax: Set Button Number
//#define TPD_KEY_COUNT  2

//Himax: Set FW and CFG Flash Address 	//20130403
unsigned char FW_VER_MAJ_FLASH_ADDR;
unsigned char FW_VER_MAJ_FLASH_LENG;
unsigned char FW_VER_MIN_FLASH_ADDR;
unsigned char FW_VER_MIN_FLASH_LENG;	 
unsigned char CFG_VER_MAJ_FLASH_ADDR;
unsigned char CFG_VER_MAJ_FLASH_LENG;
unsigned char CFG_VER_MIN_FLASH_ADDR; 
unsigned char CFG_VER_MIN_FLASH_LENG;


#ifdef LCT_MTK_CTP_INFO_SUPPORT
//static unsigned char FW_VER_MAJ_FLASH_buff[FW_VER_MAJ_FLASH_LENG * 4];
//static unsigned char FW_VER_MIN_FLASH_buff[FW_VER_MIN_FLASH_LENG * 4];
static unsigned char CFG_VER_MAJ_FLASH_buff[3 * 4];
static unsigned char CFG_VER_MIN_FLASH_buff[3 * 4];
#endif

#define VELOCITY_CUSTOM_HX8526
#ifdef VELOCITY_CUSTOM_HX8526
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

// for magnify velocity********************************************

#ifndef TPD_VELOCITY_CUSTOM_X
#define TPD_VELOCITY_CUSTOM_X 10
#endif
#ifndef TPD_VELOCITY_CUSTOM_Y
#define TPD_VELOCITY_CUSTOM_Y 10
#endif

#define TOUCH_IOC_MAGIC 'A'

#define TPD_GET_VELOCITY_CUSTOM_X _IO(TOUCH_IOC_MAGIC,0)
#define TPD_GET_VELOCITY_CUSTOM_Y _IO(TOUCH_IOC_MAGIC,1)

int g_v_magnify_x =TPD_VELOCITY_CUSTOM_X;
int g_v_magnify_y =TPD_VELOCITY_CUSTOM_Y;

#ifdef SLT_DEVINFO_CTP

#define SLT_DEVINFO_CTP_DEBUG


#include  <linux/dev_info.h>
static int devinfo_first=0;
struct devinfo_struct *s_DEVINFO_ctp;   //suppose 10 max lcm device 
static char  temp_ver[16];
static int temp_pid;
//The followd code is for GTP style
static void devinfo_ctp_regchar(char *module,char * vendor,char *version,char *used)
{
 	
	s_DEVINFO_ctp =(struct devinfo_struct*) kmalloc(sizeof(struct devinfo_struct), GFP_KERNEL);	
	s_DEVINFO_ctp->device_type="CTP";
	s_DEVINFO_ctp->device_module=module;
	s_DEVINFO_ctp->device_vendor=vendor;
	s_DEVINFO_ctp->device_ic="HX8526";
	s_DEVINFO_ctp->device_info=DEVINFO_NULL;
	s_DEVINFO_ctp->device_version=version;
	s_DEVINFO_ctp->device_used=used;
#ifdef SLT_DEVINFO_CTP_DEBUG
		printk("[DEVINFO CTP]registe CTP device! type:<%s> module:<%s> vendor<%s> ic<%s> version<%s> info<%s> used<%s>\n",
				s_DEVINFO_ctp->device_type,s_DEVINFO_ctp->device_module,s_DEVINFO_ctp->device_vendor,
				s_DEVINFO_ctp->device_ic,s_DEVINFO_ctp->device_version,s_DEVINFO_ctp->device_info,s_DEVINFO_ctp->device_used);
#endif
DEVINFO_CHECK_DECLARE(s_DEVINFO_ctp->device_type,s_DEVINFO_ctp->device_module,s_DEVINFO_ctp->device_vendor,
				s_DEVINFO_ctp->device_ic,s_DEVINFO_ctp->device_version,s_DEVINFO_ctp->device_info,s_DEVINFO_ctp->device_used);

}


#endif

static int tpd_misc_open(struct inode *inode, struct file *file)
{
/*
	file->private_data = adxl345_i2c_client;

	if(file->private_data == NULL)
	{
		printk("tpd: null pointer!!\n");
		return -EINVAL;
	}
	*/
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int tpd_misc_release(struct inode *inode, struct file *file)
{
	//file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
//static int adxl345_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
//       unsigned long arg)
static long tpd_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	//struct i2c_client *client = (struct i2c_client*)file->private_data;
	//struct adxl345_i2c_data *obj = (struct adxl345_i2c_data*)i2c_get_clientdata(client);	
	//char strbuf[256];
	void __user *data;
	
	long err = 0;
	
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		TPD_DMESG("tpd: access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case TPD_GET_VELOCITY_CUSTOM_X:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &g_v_magnify_x, sizeof(g_v_magnify_x)))
			{
				err = -EFAULT;
				break;
			}				 
			break;

	   case TPD_GET_VELOCITY_CUSTOM_Y:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &g_v_magnify_y, sizeof(g_v_magnify_y)))
			{
				err = -EFAULT;
				break;
			}				 
			break;


		default:
			TPD_DMESG("tpd: unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;
			
	}

	return err;
}


static struct file_operations tpd_fops = {
//	.owner = THIS_MODULE,
	.open = tpd_misc_open,
	.release = tpd_misc_release,
	.unlocked_ioctl = tpd_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice tpd_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "touch",
	.fops = &tpd_fops,
};

//**********************************************
#endif /* VELOCITY_CUSTOM_HX8526 */

struct touch_info {
#if HIMAX_5POINT_SUPPORT
    int y[5];	// Y coordinate of touch point
    int x[5];	// X coordinate of touch point
    int p[5];	// event flag of touch point
    int id[5];	// touch id of touch point
#else
    int y[4];	
    int x[4];
    int p[4];
    int id[4];
#endif
    int count;	// touch counter
};

#ifdef TPD_HAVE_BUTTON
//Himax: Set KEY Type and KEY Area
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;

//int keyid = 0;
#endif

//Himax: Set I2C Slave Address
#if Android4_0
static const struct i2c_device_id tpd_i2c_id[] = {{"mtk-tpd",0},{}};
static struct i2c_board_info __initdata himax_i2c_tpd={ I2C_BOARD_INFO("mtk-tpd", (0x94>>1))};
#endif
#if Android2_3
static const struct i2c_device_id tpd_id[] = {{TPD_DEVICE,0},{}};
static  unsigned short force[] = {0,0x90,I2C_CLIENT_END,I2C_CLIENT_END}; 
static const unsigned short * const forces[] = { force, NULL }; 
static struct i2c_client_address_data addr_data = { .forces = forces, }; 
#endif

#if ((HIMAX_RAWDATA) | (HIMAX_UPGRADE))
static struct kobject *android_touch_kobj = NULL;
static uint8_t debug_log_level= 0;

static int himax_touch_sysfs_init(void);
static void himax_touch_sysfs_deinit(void);

static uint8_t getDebugLevel(void)
{
    return debug_log_level;
}
#endif

#ifdef MTK6589_DMA
static int hx8526_i2c_dma_recv_data(struct i2c_client *client, uint8_t command,uint8_t len,uint8_t *buf)
{
	int rc;
	unsigned short addr = 0;
	uint8_t *pReadData = 0;
	pReadData = gpDMABuf_va;
	addr = client->addr ;
	client->addr |= I2C_DMA_FLAG;
    // pa_addr   = gpDMABuf_pa;

	printk("[hx8526] hx8526_i2c_dma_recv_data len = %d\n",len);
    if(!pReadData){
		printk("[hx8526] dma_alloc_coherent failed!\n");
		return -1;
    }
	gpDMABuf_va[0] = command;
	rc = i2c_master_send(client, gpDMABuf_pa, 1);
	if (rc < 0) 
	{
		printk("[hx8526] hx8526_i2c_dma_recv_data sendcomand failed!\n");
	}
	rc = i2c_master_recv(client, gpDMABuf_pa, len);
	client->addr = addr;
	//copy_to_user(buf, pReadData, len);
	memcpy(buf, pReadData, len);
	printk("[hx8526] hx8526_i2c_dma_recv_data rc=%d!\n",rc);
	
	return rc;
}

static int hx8526_i2c_dma_send_data(struct i2c_client *client, uint8_t command,uint8_t len,uint8_t *buf)
{
	int rc;
	unsigned short addr = 0;
	uint8_t *pWriteData = gpDMABuf_va;
	addr = client->addr ;
	client->addr |= I2C_DMA_FLAG;
    //pa_addr    = gpDMABuf_pa;

	printk("[hx8526] hx8526_i2c_dma_send_data len = %d\n",len);
	
    if(!pWriteData){
        printk("[hx8526] dma_alloc_coherent failed!\n");
		return -1;
    }
	
	
    //copy_from_user(pWriteData+1, ((void*)buf), len);
	memcpy(pWriteData+1, ((void*)buf), len);
	
	pWriteData[0] = command;
	
	//rc = i2c_master_send(client, gpDMABuf_pa, 1);
	//printk("[hx8526] hx8526_i2c_dma_send_data send commandrc=%d!\n",rc);
	
	rc = i2c_master_send(client, gpDMABuf_pa, len+1);
	client->addr = addr;
	printk("[hx8526] hx8526_i2c_dma_send_data rc=%d!\n",rc);
	return rc;
}
#endif


#if HIMAX_RAWDATA
#define DEFAULT_X_CHANNEL            11   /* face the TS, x-axis */
#define DEFAULT_Y_CHANNEL            21   /* face the TS, y-axis */
#define DEFAULT_SELF_CHANNEL         32

static uint8_t himax_command = 0;
static uint8_t x_channel = DEFAULT_X_CHANNEL; /* x asix, when you face the top of TS */
static uint8_t y_channel = DEFAULT_Y_CHANNEL; /* y asix, when you face the top of TS  */
static uint8_t *diag_mutual = NULL;
static uint8_t diag_self[DEFAULT_X_CHANNEL+DEFAULT_Y_CHANNEL] = {0};

static uint8_t *getMutualBuffer(void)
{
    return diag_mutual;
}

static void setMutualBuffer(void)
{
    diag_mutual = kzalloc(x_channel * y_channel * sizeof(uint8_t), GFP_KERNEL);
}

static uint8_t *getSelfBuffer(void)
{
    return &diag_self[0];
}

static uint8_t getDiagCommand(void)
{
    return diag_command;
}

static uint8_t getXChannel(void)
{
    return x_channel;
}

static uint8_t getYChannel(void)
{
    return y_channel;
}

static void setXChannel(uint8_t x)
{
    x_channel = x;
}

static void setYChannel(uint8_t y)
{
    y_channel = y;
}

/* kernel sysfs interface for reading/writing HIMAX register == START */
static ssize_t himax_register_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;
	uint8_t data[96] = { 0 }, loop_i;

	printk(KERN_ERR "[Himax]himax_register_show himax_command=%x\n",himax_command);

	TPD_DMESG(KERN_INFO "[TP]%x\n", himax_command);
	#ifdef MTK6589_DMA
	if(hx8526_i2c_dma_recv_data(i2c_client,himax_command,96,&data[0]) < 0)
	{
		TPD_DMESG(KERN_WARNING "%s: dma read fail\n", __func__);
		return ret;
	}
	#else
	if (i2c_smbus_read_i2c_block_data(i2c_client, himax_command, 96, &data[0]) < 0) {
		TPD_DMESG(KERN_WARNING "%s: read fail\n", __func__);
		return ret;
	}
	#endif

	ret += sprintf(buf, "command: %x\n", himax_command);
	for (loop_i = 0; loop_i < 96; loop_i++) {
		ret += sprintf(buf + ret, "0x%2.2X ", data[loop_i]);
		if ((loop_i % 16) == 15)
			ret += sprintf(buf + ret, "\n");
	}
	ret += sprintf(buf + ret, "\n");
	return ret;
}

static ssize_t himax_register_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	char buf_tmp[6], length = 0;
	uint8_t veriLen = 0;
	uint8_t write_da[100];
	unsigned long result = 0;

	printk(KERN_ERR "[Himax]himax_register_store IC_TYPE=%d\n",IC_TYPE);

	memset(buf_tmp, 0x0, sizeof(buf_tmp));
	memset(write_da, 0x0, sizeof(write_da));

	if ((buf[0] == 'r' || buf[0] == 'w') && buf[1] == ':') {
		if (buf[2] == 'x') {
			uint8_t loop_i;
			uint16_t base = 5;
			memcpy(buf_tmp, buf + 3, 2);
			if (!strict_strtoul(buf_tmp, 16, &result))
				himax_command = result;
			for (loop_i = 0; loop_i < 100; loop_i++) {
				if (buf[base] == '\n') {
					if (buf[0] == 'w')
					#ifdef MTK6589_DMA
						hx8526_i2c_dma_send_data(i2c_client, himax_command, length, &write_da[0]);
					#else
                        i2c_smbus_write_i2c_block_data(i2c_client, himax_command, length, &write_da[0]);
					#endif
					TPD_DMESG(KERN_INFO "CMD: %x, %x, %d\n", himax_command,
						write_da[0], length);
					for (veriLen = 0; veriLen < length; veriLen++)
						TPD_DMESG(KERN_INFO "%x ", *((&write_da[0])+veriLen));

					TPD_DMESG(KERN_INFO "\n");
					return count;
				}
				if (buf[base + 1] == 'x') {
					buf_tmp[4] = '\n';
					buf_tmp[5] = '\0';
					memcpy(buf_tmp, buf + base + 2, 2);
					if (!strict_strtoul(buf_tmp, 16, &result))
						write_da[loop_i] = result;
					length++;
				}
				base += 4;
			}
		}
	}
	return count;
}

static DEVICE_ATTR(register, HIMAX_SYS_NODE_RIGHT,
	himax_register_show, himax_register_store);
/* kernel sysfs interface for reading/writing HIMAX register == END */

#if 0
/* kernel sysfs interface for showing firmware version info == START */
static ssize_t touch_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%s_%s\n", FW_VER_MAJ_FLASH_buff, FW_VER_MIN_FLASH_buff);
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(vendor, 0444, touch_vendor_show, NULL);
/* kernel sysfs interface for showing firmware version info == END */
#endif

/* kernel sysfs interface for executing HIMAX special command == START*/
static ssize_t himax_diag_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	size_t count = 0;
	uint32_t loop_i;
	uint16_t mutual_num, self_num, width;

	printk(KERN_ERR "[Himax]himax_diag_show IC_TYPE=%d\n",IC_TYPE);
	mutual_num = x_channel * y_channel;
	self_num = x_channel + y_channel;
	width = x_channel;
	count += sprintf(buf + count, "Channel: %4d, %4d\n\n", x_channel, y_channel);

	if (diag_command >= 1 && diag_command <= 6) {
		if (diag_command <=3) {
			for (loop_i = 0; loop_i < mutual_num; loop_i++) {
				count += sprintf(buf + count, "%4d", diag_mutual[loop_i]);
				if ((loop_i % width) == (width - 1)) {
					count += sprintf(buf + count, " %3d\n", diag_self[width + loop_i/width]);
				}
			}
			count += sprintf(buf + count, "\n");
			for (loop_i = 0; loop_i < width; loop_i++) {
				count += sprintf(buf + count, "%4d", diag_self[loop_i]);
				if (((loop_i) % width) == (width - 1))
					count += sprintf(buf + count, "\n");
			}
		} else if (diag_command > 4) {
			for (loop_i = 0; loop_i < self_num; loop_i++) {
				count += sprintf(buf + count, "%4d", diag_self[loop_i]);
				if (((loop_i - mutual_num) % width) == (width - 1))
					count += sprintf(buf + count, "\n");
			}
		} else {
			for (loop_i = 0; loop_i < mutual_num; loop_i++) {
				count += sprintf(buf + count, "%4d", diag_mutual[loop_i]);
				if ((loop_i % width) == (width - 1))
					count += sprintf(buf + count, "\n");
			}
		}
	}

	return count;
}

static ssize_t himax_diag_dump(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	uint8_t command_ec_128_raw_flag = 0x01;
    uint8_t command_ec_24_normal_flag = 0x00;
	uint8_t command_ec_128_raw_baseline_flag = 0x00;
	uint8_t command_ec_128_raw_bank_flag;

	printk(KERN_ERR "[Himax]himax_diag_dump IC_TYPE=%d\n",IC_TYPE);
	if (IC_TYPE == HX_85XX_A_SERIES_PWON)     
	{    
		  command_ec_128_raw_baseline_flag = 0x02 | command_ec_128_raw_flag;
	}
	else if (IC_TYPE == HX_85XX_D_SERIES_PWON)
	{
    	command_ec_128_raw_baseline_flag = 0x02;
    	command_ec_128_raw_bank_flag = 0x03;
	} 
	else
	{
		command_ec_128_raw_baseline_flag = 0x02;
	}
    	uint8_t command_91h[2] = {0x91, 0x00};
    	uint8_t command_82h[1] = {0x82};
    	uint8_t command_F3h[2] = {0xF3, 0x00};
   		uint8_t command_83h[1] = {0x83};
    	uint8_t receive[1];

    if (buf[0] == '1')
    {
        command_91h[1] = command_ec_128_raw_baseline_flag;
		
        i2c_smbus_write_i2c_block_data(i2c_client, command_91h[0], 1, &command_91h[1]);
		
        diag_command = buf[0] - '0';
        printk(KERN_ERR "[Himax]111 diag_command=0x%x\n",diag_command);
    }
    else if (buf[0] == '2')
    {
        command_91h[1] = command_ec_128_raw_flag;
		
        i2c_smbus_write_i2c_block_data(i2c_client, command_91h[0], 1, &command_91h[1]);
		
        diag_command = buf[0] - '0';
        printk(KERN_ERR "[Himax]222 diag_command=0x%x\n",diag_command);
    }
    else if (buf[0] == '3')
	{
		if (IC_TYPE == HX_85XX_A_SERIES_PWON) 	    
		{    
			
			i2c_smbus_write_i2c_block_data(i2c_client, command_82h[0], 1, &command_82h[0]);
			
  			msleep(50);
  			
  			i2c_smbus_read_i2c_block_data(i2c_client, command_F3h[0], 1, &receive[0]);
			
  			command_F3h[1] = (receive[0] | 0x80);
			
  			i2c_smbus_write_i2c_block_data(i2c_client, command_F3h[0], 2, &command_F3h[1]);    
			
 			command_91h[1] = command_ec_128_raw_flag;
			
  			i2c_smbus_write_i2c_block_data(i2c_client, command_91h[0], 1, &command_91h[1]);      
  			i2c_smbus_write_i2c_block_data(i2c_client, command_83h[0], 2, &command_83h[0]);
			
  			msleep(50);
		}
		else if (IC_TYPE == HX_85XX_D_SERIES_PWON)
		{
  			command_91h[1] = command_ec_128_raw_bank_flag;
			
  			i2c_smbus_write_i2c_block_data(i2c_client, command_91h[0], 1, &command_91h[1]);
			
		}    	        
  		
		diag_command = buf[0] - '0';
  		printk(KERN_ERR "[Himax]333 diag_command=0x%x\n",diag_command);
	}
	else	
	{
		if (IC_TYPE == HX_85XX_A_SERIES_PWON)         
  		{  
  			
  			i2c_smbus_write_i2c_block_data(i2c_client, command_82h[0], 1, &command_82h[0]);
			
    		msleep(50);
        
    		command_91h[1] = command_ec_24_normal_flag;
			
    		i2c_smbus_write_i2c_block_data(i2c_client, command_91h[0], 1, &command_91h[1]);
        
   			i2c_smbus_read_i2c_block_data(i2c_client, command_F3h[0], 1, &receive[0]);
			
    		command_F3h[1] = (receive[0] & 0x7F);
			
    		i2c_smbus_write_i2c_block_data(i2c_client, command_F3h[0], 2, &command_F3h[1]);
    		i2c_smbus_write_i2c_block_data(i2c_client, command_83h[0], 2, &command_83h[0]);
			
		}
		else if(IC_TYPE == HX_85XX_D_SERIES_PWON)
		{    
			command_91h[1] = command_ec_24_normal_flag;
			
			i2c_smbus_write_i2c_block_data(i2c_client, command_91h[0], 1, &command_91h[1]);
			
		} 	
 		 diag_command = 0;
  		printk(KERN_ERR "[Himax] 444 diag_command=0x%x\n",diag_command);  
	}

	return count;
}


static DEVICE_ATTR(diag, HIMAX_SYS_NODE_RIGHT,
	himax_diag_show, himax_diag_dump);
/* kernel sysfs interface for executing HIMAX special command == END*/
#endif /* HIMAX_RAWDATA */


#if HIMAX_UPGRADE
#if HIMAX_UPGRADE_WITH_IFILE


// for D32
static unsigned char HIMAX_FW_D32[]=   //20130104zjq
{
	#include "LongCheerAW990_D3V08_2013-12-31_1258.i" //Paul Check
};
static unsigned char HIMAX_FW_D32_zhongnuo[]=   
{
	#include "LongCheerAW990_zhongnuo_D32V101_2013-05-28_1618.i" //Paul Check
};

//for  A01
static unsigned char HIMAX_FW_A01[]=   //20130104zjq
{
	#include "longcheeraw890v10_2013-01-08_1621.i" //Paul Check
};

static unsigned char HIMAX_FW_A01_Doov[]=   //20130104zjq
{
	#include "Himax_FW_Doov_v109_2013-01-12_1226.i" //Paul Check
};

static unsigned char HIMAX_FW_A01_zhongnuo[]=   //20130104zjq
{
	#include "Himax_FW_zhongnuo_v002_2013-04-10_1628.i" //Paul Check
};

static int himax_i_file_select_sub(void)
{
	if (IC_TYPE == HX_85XX_D_SERIES_PWON)     
	{    
		#if HIMAX_D32_AUTO_UPGRADE
		if(strncmp(CFG_VER_MAJ_FLASH_buff,HIMAX_D32_TP_FW_CUSTOMER_ID_PLATFORM,strlen(HIMAX_D32_TP_FW_CUSTOMER_ID_PLATFORM)) == 0)
		{
			#ifdef LCT_PROJECT_ZNT_AW990_A01
			HIMAX_FW=HIMAX_FW_D32_zhongnuo;
			fw_size = sizeof(HIMAX_FW_D32_zhongnuo);
			#else
			HIMAX_FW=HIMAX_FW_D32;
			fw_size = sizeof(HIMAX_FW_D32);
			#endif
		}
		else if(strncmp(CFG_VER_MAJ_FLASH_buff,HIMAX_D32_TP_FW_CUSTOMER_zhongnuo,strlen(HIMAX_D32_TP_FW_CUSTOMER_zhongnuo)) == 0)
		{
			HIMAX_FW=HIMAX_FW_D32_zhongnuo;
			fw_size = sizeof(HIMAX_FW_D32_zhongnuo);
		}
		else
		{
			return 0;
		}
		#else
		 return 0;
		#endif
	}
	else if (IC_TYPE == HX_85XX_A_SERIES_PWON)
	{
		#if HIMAX_A01_AUTO_UPGRADE
		if(strncmp(CFG_VER_MAJ_FLASH_buff,HIMAX_A01_TP_FW_CUSTOMER_ID_PLATFORM,strlen(HIMAX_A01_TP_FW_CUSTOMER_ID_PLATFORM)) == 0)
		{
			HIMAX_FW=HIMAX_FW_A01;
			fw_size = sizeof(HIMAX_FW_A01);
		}
		else if(strncmp(CFG_VER_MAJ_FLASH_buff,HIMAX_A01_TP_FW_CUSTOMER_Doov,strlen(HIMAX_A01_TP_FW_CUSTOMER_Doov)) == 0)
		{
			HIMAX_FW=HIMAX_FW_A01_Doov;
			fw_size = sizeof(HIMAX_FW_A01_Doov);
		}
		else if(strncmp(CFG_VER_MAJ_FLASH_buff,HIMAX_A01_TP_FW_CUSTOMER_zhongnuo,strlen(HIMAX_A01_TP_FW_CUSTOMER_zhongnuo)) == 0)
		{
			HIMAX_FW=HIMAX_FW_A01_zhongnuo;
			fw_size = sizeof(HIMAX_FW_A01_zhongnuo);
		}
		else
		{
			return 0;
		}
		#else
		 return 0;
		#endif
	}
	else
	{
		printk("i file load fail!\n"); 
		return 0;
	}
	return 1;
}

#endif

#if 0
/*Hiamx add FW Update FLow*/
unsigned char SFR_3u_1[16][2] = {{0x18,0x06},{0x18,0x16},{0x18,0x26},{0x18,0x36},{0x18,0x46},
								 {0x18,0x56},{0x18,0x66},{0x18,0x76},{0x18,0x86},{0x18,0x96},
								 {0x18,0xA6},{0x18,0xB6},{0x18,0xC6},{0x18,0xD6},{0x18,0xE6},
								 {0x18,0xF6}};

unsigned char SFR_6u_1[16][2] = {{0x98,0x04},{0x98,0x14},{0x98,0x24},{0x98,0x34},{0x98,0x44},
								 {0x98,0x54},{0x98,0x64},{0x98,0x74},{0x98,0x84},{0x98,0x94},
								 {0x98,0xA4},{0x98,0xB4},{0x98,0xC4},{0x98,0xD4},{0x98,0xE4},
								 {0x98,0xF4}};
#endif
								 
static unsigned char upgrade_fw[32*1024];

void himax_ManualMode(int enter)
{
	unsigned char cmd[2];
	cmd[0] = enter;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x42, 1, &cmd[0]);
}

void himax_FlashMode(int enter)
{
	unsigned char cmd[2];
	cmd[0] = enter;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 1, &cmd[0]);
}

void himax_lock_flash(void)
{
	unsigned char cmd[5];
	
	/* lock sequence start */
	cmd[0] = 0x01;cmd[1] = 0x00;cmd[2] = 0x06;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]);

	cmd[0] = 0x03;cmd[1] = 0x00;cmd[2] = 0x00;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x44, 3, &cmd[0]);

	cmd[0] = 0x00;cmd[1] = 0x00;cmd[2] = 0x7D;cmd[3] = 0x03;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x45, 4, &cmd[0]);

	i2c_smbus_write_i2c_block_data(i2c_client, 0x4A, 0, &cmd[0]);
	mdelay(50);
	/* lock sequence stop */
}

void himax_unlock_flash(void)
{
	unsigned char cmd[5];
	
	/* unlock sequence start */
	cmd[0] = 0x01;cmd[1] = 0x00;cmd[2] = 0x06;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]);

	cmd[0] = 0x03;cmd[1] = 0x00;cmd[2] = 0x00;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x44, 3, &cmd[0]);

	cmd[0] = 0x00;cmd[1] = 0x00;cmd[2] = 0x3D;cmd[3] = 0x03;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x45, 4, &cmd[0]);

	i2c_smbus_write_i2c_block_data(i2c_client, 0x4A, 0, &cmd[0]);
	mdelay(50);
	/* unlock sequence stop */
}

#if 0
int himax_modifyIref(void)
{
	unsigned char i;
	unsigned char cmd[5];
	unsigned char Iref[2] = {0x00,0x00};
	
	cmd[0] = 0x01;cmd[1] = 0x00;cmd[2] = 0x08;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		return 0;
	
	cmd[0] = 0x00;cmd[1] = 0x00;cmd[2] = 0x00;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x44, 3, &cmd[0]))< 0)
		return 0;
	
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x46, 0, &cmd[0]))< 0)
		return 0;
	
	if((i2c_smbus_read_i2c_block_data(i2c_client, 0x59, 4, &cmd[0]))< 0)
			return 0;
			 
	mdelay(5);
	for(i=0;i<16;i++)
	{
		if(cmd[1]==SFR_3u_1[i][0]&&cmd[2]==SFR_3u_1[i][1])
		{
			Iref[0]= SFR_6u_1[i][0];
			Iref[1]= SFR_6u_1[i][1];
		} 
	}
	
	cmd[0] = 0x01;cmd[1] = 0x00;cmd[2] = 0x06;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		return 0;
	
	cmd[0] = 0x00;cmd[1] = 0x00;cmd[2] = 0x00;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x44, 3, &cmd[0]))< 0)
		return 0;
	
	cmd[0] = Iref[0];cmd[1] = Iref[1];cmd[2] = 0x27;cmd[3] = 0x27;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x45, 4, &cmd[0]))< 0)
		return 0;
	
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x4A, 0, &cmd[0]))< 0)
		return 0;
		
	return 1;	
}
#endif

static uint8_t himax_calculateChecksum(char *ImageBuffer, int fullLength)//, int address, int RST)
{
	u16 checksum = 0;
	unsigned char cmd[5], last_byte;
	int FileLength, i, readLen, k, lastLength;

	FileLength = fullLength - 2;
	memset(cmd, 0x00, sizeof(cmd));

	//if(himax_modifyIref() == 0)
		//return 0;
	
	himax_FlashMode(1);

	FileLength = (FileLength + 3) / 4;
	for (i = 0; i < FileLength; i++) 
	{
		last_byte = 0;
		readLen = 0;

		cmd[0] = i & 0x1F;
		if (cmd[0] == 0x1F || i == FileLength - 1)
			last_byte = 1;
		cmd[1] = (i >> 5) & 0x1F;cmd[2] = (i >> 10) & 0x1F;
		if((i2c_smbus_write_i2c_block_data(i2c_client, 0x44, 3, &cmd[0]))< 0)
			return 0;

		if((i2c_smbus_write_i2c_block_data(i2c_client, 0x46, 0, &cmd[0]))< 0)
			return 0;

		if((i2c_smbus_read_i2c_block_data(i2c_client, 0x59, 4, &cmd[0]))< 0)
			return 0; 

		if (i < (FileLength - 1))
		{
			checksum += cmd[0] + cmd[1] + cmd[2] + cmd[3];
			if (i == 0)
				TPD_DMESG("Himax_marked cmd 0 to 3 (first): %d, %d, %d, %d\n", cmd[0], cmd[1], cmd[2], cmd[3]);
		}
		else 
		{
			TPD_DMESG("Himax_marked cmd 0 to 3 (last): %d, %d, %d, %d\n", cmd[0], cmd[1], cmd[2], cmd[3]);
			TPD_DMESG("Himax_marked, checksum (not last): %d\n", checksum);
			lastLength = (((fullLength - 2) % 4) > 0)?((fullLength - 2) % 4):4;
			
			for (k = 0; k < lastLength; k++) 
				checksum += cmd[k];
			TPD_DMESG("Himax_marked, checksum (final): %d\n", checksum);
			
			//Check Success
			if (ImageBuffer[fullLength - 1] == (u8)(0xFF & (checksum >> 8)) && ImageBuffer[fullLength - 2] == (u8)(0xFF & checksum)) 
			{
				himax_FlashMode(0);
				return 1;
			} 
			else //Check Fail
			{
				himax_FlashMode(0);
				return 0;
			}
		}
	}
}

#if HIMAX_UPGRADE_WITH_IFILE

void hx8526_kickdog(void *info)
{
	mtk_wdt_restart(WK_WDT_EXT_TYPE);
}

int himax_fw_upgrade_with_i_file(void)  //20130104
{
	//Get the Firmware.bin and Length
  unsigned char* ImageBuffer = HIMAX_FW;
  int fullFileLength = fw_size;//sizeof(HIMAX_FW);
  static char data[2];
  int i, j;
	unsigned char cmd[5], last_byte, prePage;
	int FileLength;
	uint8_t checksumResult = 0;

	//Himax: Retry 3 Times
	for (j = 0; j < 3; j++) 
	{
		FileLength = fullFileLength - 2;

		himax_HW_reset();
		if (IC_TYPE == HX_85XX_A_SERIES_PWON)
		{
		  	data[0] =0x02;
				if((i2c_smbus_write_i2c_block_data(i2c_client, 0x42, 1, &data[0]))< 0)
					return 0;
				mdelay(1);
		}
   if((i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &cmd[0]))< 0)
		   return 0; 
		mdelay(120);

		himax_unlock_flash();  //ok

		cmd[0] = 0x05;cmd[1] = 0x00;cmd[2] = 0x02;
    if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		   return 0; 

    if((i2c_smbus_write_i2c_block_data(i2c_client, 0x4F, 0, &cmd[0]))< 0)
		   return 0; 	
		mdelay(50);

		himax_ManualMode(1);
		himax_FlashMode(1);

		FileLength = (FileLength + 3) / 4;
		for (i = 0, prePage = 0; i < FileLength; i++) 
		{
			last_byte = 0;

			cmd[0] = i & 0x1F;
			if (cmd[0] == 0x1F || i == FileLength - 1)
				last_byte = 1;
			cmd[1] = (i >> 5) & 0x1F;
			cmd[2] = (i >> 10) & 0x1F;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x44, 3, &cmd[0]))< 0)
		   return 0;

			if (prePage != cmd[1] || i == 0) 
			{
				prePage = cmd[1];

				cmd[0] = 0x01;cmd[1] = 0x09;cmd[2] = 0x02;
				if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		   		return 0;
		   		
				cmd[0] = 0x01;cmd[1] = 0x0D;cmd[2] = 0x02;
				if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		   		return 0;

				cmd[0] = 0x01;cmd[1] = 0x09;cmd[2] = 0x02;
				if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		   		return 0;
			}

			memcpy(&cmd[0], &ImageBuffer[4*i], 4);//Paul
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x45, 4, &cmd[0]))< 0)
		   		return 0;

			cmd[0] = 0x01;cmd[1] = 0x0D;cmd[2] = 0x02;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		   		return 0;
		   		
			cmd[0] = 0x01;cmd[1] = 0x09;cmd[2] = 0x02;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		   		return 0;

			if (last_byte == 1) 
			{
				cmd[0] = 0x01;cmd[1] = 0x01;cmd[2] = 0x02;
				if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		   		return 0;
		   		
				cmd[0] = 0x01;cmd[1] = 0x05;cmd[2] = 0x02;
				if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		   		return 0;

				cmd[0] = 0x01;cmd[1] = 0x01;cmd[2] = 0x02;
				if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		   		return 0;
		   		
				cmd[0] = 0x01;cmd[1] = 0x00;cmd[2] = 0x02;
				if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		   			return 0;

				mdelay(10);
				if (i == (FileLength - 1)) 
				{
					himax_FlashMode(0);
					himax_ManualMode(0);
										
					/*Android4_1 kick dog
					mtk_wdt_restart(WK_WDT_EXT_TYPE);//kick external WDT 
					mtk_wdt_restart(WK_WDT_LOC_TYPE);//kick local WDT, CPU0/CPU1 kick 
					*/
				  					
					
					//on_each_cpu((smp_ca'll_func_t)mpcore_wdt_restart,0,0);

					on_each_cpu((smp_call_func_t)hx8526_kickdog,NULL,0);
					
					#if Android2_3
				    		mtk_wdt_restart();
				  	#endif
					checksumResult = himax_calculateChecksum(ImageBuffer, fullFileLength);
					
					himax_lock_flash();
					if (checksumResult) //Success
					{
						himax_HW_reset();
						return 1;
					} 
					else if (!checksumResult) //Fail
					{
						himax_HW_reset();
					} 
					else //Retry
					{
						himax_FlashMode(0);
						himax_ManualMode(0);
					}
				}
			}
		}
	}
	return 0;
}
#endif

int fts_ctpm_fw_upgrade_with_sys_fs(unsigned char *fw, int len)
{
    unsigned char* ImageBuffer = fw;//CTPM_FW;
    int fullFileLength = len;//sizeof(CTPM_FW); //Paul Check
    int i, j;
    unsigned char cmd[5], last_byte, prePage;
    int FileLength;
    uint8_t checksumResult = 0;

    //Try 3 Times
    for (j = 0; j < 3; j++) 
    {
        FileLength = fullFileLength - 2;

        himax_HW_reset();

        if((i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &cmd[0]))< 0){
            TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
            return 0;
        }
       
        mdelay(120);

        himax_unlock_flash();  //ok

        cmd[0] = 0x05;cmd[1] = 0x00;cmd[2] = 0x02;
        if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)     //ok
        {
            TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
            return 0;
        }

        if((i2c_smbus_write_i2c_block_data(i2c_client, 0x4F, 0, &cmd[0]))< 0)     //ok
        {
            TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
            return 0;
        }     
        mdelay(50);

        himax_ManualMode(1); //ok
        himax_FlashMode(1);  //ok

        FileLength = (FileLength + 3) / 4;
        for (i = 0, prePage = 0; i < FileLength; i++) 
        {
            last_byte = 0;
            cmd[0] = i & 0x1F;
            if (cmd[0] == 0x1F || i == FileLength - 1)
                last_byte = 1;
            cmd[1] = (i >> 5) & 0x1F;
            cmd[2] = (i >> 10) & 0x1F;
            if((i2c_smbus_write_i2c_block_data(i2c_client, 0x44, 3, &cmd[0]))< 0){
                TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                return 0;
            }

            if (prePage != cmd[1] || i == 0) 
            {
                prePage = cmd[1];
                cmd[0] = 0x01;cmd[1] = 0x09;cmd[2] = 0x02;
                if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0){
                    TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                    return 0;
                }
                   
                cmd[0] = 0x01;cmd[1] = 0x0D;cmd[2] = 0x02;
                if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0){
                    TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                    return 0;
                }

                cmd[0] = 0x01;cmd[1] = 0x09;cmd[2] = 0x02;
                if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0){
                    TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                    return 0;
                }
            }

            memcpy(&cmd[0], &ImageBuffer[4*i], 4);//Paul
            if((i2c_smbus_write_i2c_block_data(i2c_client, 0x45, 4, &cmd[0]))< 0){
                TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                return 0;
            }

            cmd[0] = 0x01;cmd[1] = 0x0D;cmd[2] = 0x02;
            if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0){
                TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                return 0;
            }
                   
            cmd[0] = 0x01;cmd[1] = 0x09;cmd[2] = 0x02;
            if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0){
                TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                return 0;
            }

            if (last_byte == 1) 
            {
                cmd[0] = 0x01;cmd[1] = 0x01;cmd[2] = 0x02;
                if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0){
                    TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                    return 0;
                }
                   
                cmd[0] = 0x01;cmd[1] = 0x05;cmd[2] = 0x02;
                if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0){
                    TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                    return 0;
                }

                cmd[0] = 0x01;cmd[1] = 0x01;cmd[2] = 0x02;
                if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0){
                    TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                    return 0;
                }
                   
                cmd[0] = 0x01;cmd[1] = 0x00;cmd[2] = 0x02;
                if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0){
                    TPD_DMESG(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
                    return 0;
                }

                mdelay(10);
                if (i == (FileLength - 1)) 
                {
                    himax_FlashMode(0);
                    himax_ManualMode(0);
                    checksumResult = himax_calculateChecksum(ImageBuffer, fullFileLength);//, address, RST);
                    //himax_ManualMode(0);
                    himax_lock_flash();
                    
                    if (checksumResult) //Success
                    {
                        himax_HW_reset();
                        return 1;
                    } 
                    else if (/*j == 4 && */!checksumResult) //Fail
                    {
                        himax_HW_reset();
                       // return 0;
                    } 
                    else //Retry
                    {
                        himax_FlashMode(0);
                        himax_ManualMode(0);
                    }
                }
            }
        }
    }    
    return 0;
}


/* kernel sysfs interface for HIMAX firmware upgrade == START*/
static ssize_t himax_debug_level_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	size_t count = 0;

	count += sprintf(buf, "%d\n", debug_log_level);

	return count;
}

static ssize_t himax_debug_level_dump(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	char data[3];
	if (buf[0] >= '0' && buf[0] <= '9' && buf[1] == '\n')
		debug_log_level = buf[0] - '0';

    struct file* filp = NULL;
    mm_segment_t oldfs;
    int result = 0;
    char fileName[128];
    
    if(buf[0] == 't')
    {
        memset(fileName, 0, 128);
        snprintf(fileName, count-2, "%s", &buf[2]);
        TPD_DMESG(KERN_INFO "[TP] %s: upgrade from file(%s) start!\n", __func__, fileName);
        filp = filp_open(fileName, O_RDONLY, 0);
        if(IS_ERR(filp)) {
            TPD_DMESG(KERN_ERR "[TP] %s: open firmware file failed\n", __func__);
            return count;
        }
        oldfs = get_fs();
        set_fs(get_ds());

        result=filp->f_op->read(filp,upgrade_fw,sizeof(upgrade_fw), &filp->f_pos);
        if(result < 0) {
            TPD_DMESG(KERN_ERR "[TP] %s: read firmware file failed\n", __func__);
            return count;
        }

        set_fs(oldfs);
        filp_close(filp, NULL);

        TPD_DMESG(KERN_INFO "[TP] %s: upgrade start,len %d: %02X, %02X, %02X, %02X\n", __func__, result, upgrade_fw[0], upgrade_fw[1], upgrade_fw[2], upgrade_fw[3]);

        if(result > 0)
        {
            	//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start	
							#ifdef ENABLE_CHIP_STATUS_MONITOR
							running_status = 1;
							cancel_delayed_work_sync(&himax_chip_monitor);
							#endif
        		NO_ESD_RESET = 1;	
            if(fts_ctpm_fw_upgrade_with_sys_fs(upgrade_fw, result) == 0)
                TPD_DMESG(KERN_INFO "[TP] %s: TP upgrade error, line: %d\n", __func__, __LINE__);
            else
                TPD_DMESG(KERN_INFO "[TP] %s: TP upgrade OK, line: %d\n", __func__, __LINE__);
                NO_ESD_RESET = 0; //john add
			himax_ts_poweron();

							//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------start
							#ifdef ENABLE_CHIP_STATUS_MONITOR
							running_status = 0;
							queue_delayed_work(himax_wq, &himax_chip_monitor, 5*HZ);
							#endif
            
            return count;                    
        }
    }

HimaxErr:
	TPD_DMESG("Himax TP: I2C transfer error, line: %d\n", __LINE__);

	return count;
}

static DEVICE_ATTR(debug_level, HIMAX_SYS_NODE_RIGHT,
	himax_debug_level_show, himax_debug_level_dump);
/* kernel sysfs interface for HIMAX firmware upgrade == END*/
#endif /* HIMAX_UPGRADE */


#if ChangeIref1u

#define TarIref 1

		unsigned char SFR_1u_1[16][2] = {{0x18,0x07},{0x18,0x17},{0x18,0x27},{0x18,0x37},{0x18,0x47},
			{0x18,0x57},{0x18,0x67},{0x18,0x77},{0x18,0x87},{0x18,0x97},
			{0x18,0xA7},{0x18,0xB7},{0x18,0xC7},{0x18,0xD7},{0x18,0xE7},
			{0x18,0xF7}};

		unsigned char SFR_2u_1[16][2] = {{0x98,0x06},{0x98,0x16},{0x98,0x26},{0x98,0x36},{0x98,0x46},
			{0x98,0x56},{0x98,0x66},{0x98,0x76},{0x98,0x86},{0x98,0x96},
			{0x98,0xA6},{0x98,0xB6},{0x98,0xC6},{0x98,0xD6},{0x98,0xE6},
			{0x98,0xF6}};

		unsigned char SFR_3u_1[16][2] = {{0x18,0x06},{0x18,0x16},{0x18,0x26},{0x18,0x36},{0x18,0x46},
			{0x18,0x56},{0x18,0x66},{0x18,0x76},{0x18,0x86},{0x18,0x96},
			{0x18,0xA6},{0x18,0xB6},{0x18,0xC6},{0x18,0xD6},{0x18,0xE6},
			{0x18,0xF6}};

		unsigned char SFR_4u_1[16][2] = {{0x98,0x05},{0x98,0x15},{0x98,0x25},{0x98,0x35},{0x98,0x45},
			{0x98,0x55},{0x98,0x65},{0x98,0x75},{0x98,0x85},{0x98,0x95},
			{0x98,0xA5},{0x98,0xB5},{0x98,0xC5},{0x98,0xD5},{0x98,0xE5},
			{0x98,0xF5}};

		unsigned char SFR_5u_1[16][2] = {{0x18,0x05},{0x18,0x15},{0x18,0x25},{0x18,0x35},{0x18,0x45},
			{0x18,0x55},{0x18,0x65},{0x18,0x75},{0x18,0x85},{0x18,0x95},
			{0x18,0xA5},{0x18,0xB5},{0x18,0xC5},{0x18,0xD5},{0x18,0xE5},
			{0x18,0xF5}};

		unsigned char SFR_6u_1[16][2] = {{0x98,0x04},{0x98,0x14},{0x98,0x24},{0x98,0x34},{0x98,0x44},
			{0x98,0x54},{0x98,0x64},{0x98,0x74},{0x98,0x84},{0x98,0x94},
			{0x98,0xA4},{0x98,0xB4},{0x98,0xC4},{0x98,0xD4},{0x98,0xE4},
			{0x98,0xF4}};

		unsigned char SFR_7u_1[16][2] = {{0x18,0x04},{0x18,0x14},{0x18,0x24},{0x18,0x34},{0x18,0x44},
			{0x18,0x54},{0x18,0x64},{0x18,0x74},{0x18,0x84},{0x18,0x94},
			{0x18,0xA4},{0x18,0xB4},{0x18,0xC4},{0x18,0xD4},{0x18,0xE4},
			{0x18,0xF4}};

int ChangeIrefSPP(void)
{
	//struct i2c_client * i2c_client = ts->client ;

#if 1
	unsigned char i;
	//int readLen;
	unsigned char cmd[5];
	//unsigned char Iref[2] = {0x00,0x00};

	unsigned char spp_source[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,		//SPP
						  	        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};		//SPP

	unsigned char spp_target[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,		//SPP
 						  	        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};		//SPP
	unsigned char retry;
	unsigned char spp_ok;

	printk("entern Himax ChangeIrefSPP by zhuxinglong\n");

	//--------------------------------------------------------------------------
	//Inital
	//--------------------------------------------------------------------------
	cmd[0] = 0x42;
	cmd[1] = 0x02;
	if(i2c_master_send(i2c_client, cmd, 2) < 0)
	{
		printk("entern Himax ChangeIrefSPP by zhuxinglong1\n");
		return 0;
	}
	udelay(10);

	cmd[0] = 0x81;
	if(i2c_master_send(i2c_client, cmd, 1) < 0)
	{
		printk("entern Himax ChangeIrefSPP by zhuxinglong2\n");
		return 0;
	}
	mdelay(160);

	//--------------------------------------------------------------------------
	//read 16-byte SPP to spp_source
	//--------------------------------------------------------------------------
	cmd[0] = 0x43;
	cmd[1] = 0x01;
	cmd[2] = 0x00;
	cmd[3] = 0x1A;
	if(i2c_master_send(i2c_client, cmd, 4) < 0)
	{
		printk("entern Himax ChangeIrefSPP by zhuxinglong3\n");
		return 0;
	}
	udelay(10);

	//4 words
	for (i = 0; i < 4; i++)
	{
		cmd[0] = 0x44;
		cmd[1] = i;			//word
		cmd[2] = 0x00;		//page
		cmd[3] = 0x00;		//sector
		if(i2c_master_send(i2c_client, cmd, 4) < 0)
		{
			printk("entern Himax ChangeIrefSPP by zhuxinglong4\n");
			return 0;
		}
		udelay(10);

		cmd[0] = 0x46;
		if(i2c_master_send(i2c_client, cmd, 1) < 0)
		{
			printk("entern Himax ChangeIrefSPP by zhuxinglong5\n");
			return 0;
		}
		udelay(10);

		cmd[0] = 0x59;
		if(i2c_master_send(i2c_client, cmd, 1) < 0)
		{
			printk("entern Himax ChangeIrefSPP by zhuxinglong6\n");
			return 0;
		}
		mdelay(5);				//check this

		if(i2c_master_recv(i2c_client, cmd, 4) < 0)
			return 0;
		udelay(10);

		//save data
		spp_source[4*i + 0] = cmd[0];
		spp_source[4*i + 1] = cmd[1];
		spp_source[4*i + 2] = cmd[2];
		spp_source[4*i + 3] = cmd[3];

		printk("entern Himax ChangeIrefSPP by zhuxinglong7 cmd0 = 0x%x\n",cmd[0]);
		printk("entern Himax ChangeIrefSPP by zhuxinglong7 cmd1 = 0x%x\n",cmd[1]);
		printk("entern Himax ChangeIrefSPP by zhuxinglong7 cmd2 = 0x%x\n",cmd[2]);
		printk("entern Himax ChangeIrefSPP by zhuxinglong7 cmd3 = 0x%x\n",cmd[3]);
		//printk("entern Himax ChangeIrefSPP by zhuxinglong7 cmd4 = 0x%x\n",cmd[4]);
		printk("entern Himax ChangeIrefSPP by zhuxinglong7\n");
	}

	//--------------------------------------------------------------------------
	//Search 3u Iref
	//--------------------------------------------------------------------------
	for (i = 0; i < 16; i++)
	{
		if(spp_source[0]==SFR_1u_1[i][0] && spp_source[1]==SFR_1u_1[i][1])
		{
			//found in 1uA
			return (1);				//OK
		}
	}

	spp_ok = 0;
	for (i = 0; i < 16; i++)
	{
		if(spp_source[0]==SFR_3u_1[i][0] && spp_source[1]==SFR_3u_1[i][1])
		{
			//found in 3uA
			spp_ok = 1;

			spp_source[0]= SFR_1u_1[i][0];
			spp_source[1]= SFR_1u_1[i][1];
			break;
		}
	}

	if (spp_ok == 0)
	{
		//no matched pair in SFR_1u_1 or SFR_3u_1
		return 0;
	}

	//--------------------------------------------------------------------------
	//write SPP (retry for 3 times if errors occur)
	//--------------------------------------------------------------------------
	for (retry = 0; retry < 3; retry++)
	{
		himax_unlock_flash();

		//write 16-byte SPP
		cmd[0] = 0x43;
		cmd[1] = 0x01;
		cmd[2] = 0x00;
		cmd[3] = 0x1A;
		if(i2c_master_send(i2c_client, cmd, 4) < 0)
			return 0;
		udelay(10);

		cmd[0] = 0x4A;
		if(i2c_master_send(i2c_client, cmd, 1) < 0)
			return 0;
		udelay(10);

		for (i = 0; i < 4; i++)
		{
			cmd[0] = 0x44;
			cmd[1] = i;			//word
			cmd[2] = 0x00;		//page
			cmd[3] = 0x00;		//sector
			if(i2c_master_send(i2c_client, cmd, 4) < 0)
				return 0;
			udelay(10);

			cmd[0] = 0x45;
			cmd[1] = spp_source[4 * i + 0];
			cmd[2] = spp_source[4 * i + 1];
			cmd[3] = spp_source[4 * i + 2];
			cmd[4] = spp_source[4 * i + 3];
			if(i2c_master_send(i2c_client, cmd, 5) < 0)
				return 0;
			udelay(10);

			cmd[0] = 0x4B;		//write SPP
			if(i2c_master_send(i2c_client, cmd, 1) < 0)
				return 0;
			udelay(10);
		}

		cmd[0] = 0x4C;
		if(i2c_master_send(i2c_client, cmd, 1) < 0)
			return 0;
		mdelay(10);

		//read 16-byte SPP to spp_target
		cmd[0] = 0x43;
		cmd[1] = 0x01;
		cmd[2] = 0x00;
		cmd[3] = 0x1A;
		if(i2c_master_send(i2c_client, cmd, 4) < 0)
		{
			printk("entern Himax ChangeIrefSPP by zhuxinglong3\n");
			return 0;
		}
		udelay(10);

		for (i = 0; i < 4; i++)
		{
			cmd[0] = 0x44;
			cmd[1] = i;			//word
			cmd[2] = 0x00;		//page
			cmd[3] = 0x00;		//sector
			if(i2c_master_send(i2c_client, cmd, 4) < 0)
			{
				printk("entern Himax ChangeIrefSPP by zhuxinglong4\n");
				return 0;
			}
			udelay(10);

			cmd[0] = 0x46;
			if(i2c_master_send(i2c_client, cmd, 1) < 0)
			{
				printk("entern Himax ChangeIrefSPP by zhuxinglong5\n");
				return 0;
			}
			udelay(10);

			cmd[0] = 0x59;
			if(i2c_master_send(i2c_client, cmd, 1) < 0)
			{
				printk("entern Himax ChangeIrefSPP by zhuxinglong6\n");
				return 0;
			}
			mdelay(5);		//check this

			if(i2c_master_recv(i2c_client, cmd, 4) < 0)
				return 0;
			udelay(10);

			spp_target[4*i + 0] = cmd[0];
			spp_target[4*i + 1] = cmd[1];
			spp_target[4*i + 2] = cmd[2];
			spp_target[4*i + 3] = cmd[3];

			printk("entern Himax ChangeIrefSPP by zhuxinglong9 cmd0 = 0x%x\n",cmd[0]);
			printk("entern Himax ChangeIrefSPP by zhuxinglong9 cmd1 = 0x%x\n",cmd[1]);
			printk("entern Himax ChangeIrefSPP by zhuxinglong9 cmd2 = 0x%x\n",cmd[2]);
			printk("entern Himax ChangeIrefSPP by zhuxinglong9 cmd3 = 0x%x\n",cmd[3]);
			//printk("entern Himax ChangeIrefSPP by zhuxinglong9 cmd4 = 0x%x\n",cmd[4]);
			printk("entern Himax ChangeIrefSPP by zhuxinglong9\n");
		}

		//compare source and target
		spp_ok = 1;
		for (i = 0; i < 16; i++)
		{
			if (spp_target[i] != spp_source[i])
			{
				spp_ok = 0;
			}
		}

		if (spp_ok == 1)
		{
			printk("entern Himax ChangeIrefSPP by zhuxinglong10\n");
			return 1;	//Modify Success
		}

		//error --> reset SFR
		cmd[0] = 0x43;
		cmd[1] = 0x01;
		cmd[2] = 0x00;
		cmd[3] = 0x06;
		if(i2c_master_send(i2c_client, cmd, 4) < 0)
			return 0;
		udelay(10);

		//write 16-byte SFR
		for (i = 0; i < 4; i++)
		{
			cmd[0] = 0x44;
			cmd[1] = i;			//word
			cmd[2] = 0x00;		//page
			cmd[3] = 0x00;		//sector
			if(i2c_master_send(i2c_client, cmd, 4) < 0)
				return 0;
			udelay(10);

			cmd[0] = 0x45;
			cmd[1] = spp_source[4 * i + 0];
			cmd[2] = spp_source[4 * i + 1];
			cmd[3] = spp_source[4 * i + 2];
			cmd[4] = spp_source[4 * i + 3];
			if(i2c_master_send(i2c_client, cmd, 5) < 0)
				return 0;
			udelay(10);

			cmd[0] = 0x4A;		//write SFR
			if(i2c_master_send(i2c_client, cmd, 1) < 0)
				return 0;
			udelay(10);
		}
	}	//retry

	printk("entern Himax ChangeIrefSPP by zhuxinglong8\n");
	return 0;			//No 3u Iref setting

	#else
	ModifyIref(1);
	return 0;
	#endif
}
#endif


#if ((HIMAX_UPGRADE) | (HIMAX_RAWDATA))
static int himax_touch_sysfs_init(void)
{
	int ret;
	android_touch_kobj = kobject_create_and_add("android_touch", NULL);
	if (android_touch_kobj == NULL) {
		TPD_DMESG(KERN_ERR "[TP]TOUCH_ERR: subsystem_register failed\n");
		ret = -ENOMEM;
		return ret;
	}
	#if HIMAX_UPGRADE
	ret = sysfs_create_file(android_touch_kobj, &dev_attr_debug_level.attr);
	if (ret) {
		TPD_DMESG(KERN_ERR "[TP]TOUCH_ERR: create_file debug_level failed\n");
		return ret;
	}
	#endif
	
	#if HIMAX_RAWDATA
	himax_command = 0;
	ret = sysfs_create_file(android_touch_kobj, &dev_attr_register.attr);
	if (ret) {
		TPD_DMESG(KERN_ERR "[TP]TOUCH_ERR: create_file register failed\n");
		return ret;
	}

	//ret = sysfs_create_file(android_touch_kobj, &dev_attr_vendor.attr);
	//if (ret) {
	//	TPD_DMESG(KERN_ERR "[TP]TOUCH_ERR: sysfs_create_file failed\n");
	//	return ret;
	//}
	ret = sysfs_create_file(android_touch_kobj, &dev_attr_diag.attr);
	if (ret) {
		TPD_DMESG(KERN_ERR "[TP]TOUCH_ERR: sysfs_create_file failed\n");
		return ret;
	}
	#endif
	
	return 0 ;
}

static void himax_touch_sysfs_deinit(void)
{
	#if HIMAX_UPGRADE
	sysfs_remove_file(android_touch_kobj, &dev_attr_debug_level.attr);
	#endif

	#if HIMAX_RAWDATA	
	sysfs_remove_file(android_touch_kobj, &dev_attr_diag.attr);
	sysfs_remove_file(android_touch_kobj, &dev_attr_register.attr);
	//sysfs_remove_file(android_touch_kobj, &dev_attr_vendor.attr);
	#endif
	
	kobject_del(android_touch_kobj);
}
#endif /* ((HIMAX_UPGRADE) | (HIMAX_RAWDATA)) */


#ifdef LCT_MTK_CTP_INFO_SUPPORT
//Himax: Read Falsh Function
static int hx8526_read_flash(unsigned char *buf, unsigned int addr, unsigned int length) //OK
{
	//unsigned char index_byte,index_page,index_sector;
	unsigned char buf0[7];	
	unsigned int i;
	unsigned int j = 0;
	
	//index_byte = (addr & 0x001F);
	//index_page = ((addr & 0x03E0) >> 5);
	//index_sector = ((addr & 0x1C00) >> 10);

	for (i = addr; i < addr + length; i++)
	{
		buf0[0] = i&0x1F;
		buf0[1] = (i>>5)&0x1F;
		buf0[2] = (i>>10)&0x1F;

        if((i2c_smbus_write_i2c_block_data(i2c_client, 0x44, 3, &buf0[0]))< 0){
            printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
            return -1;
        }
        udelay(10);
        
        if((i2c_smbus_write_i2c_block_data(i2c_client, 0x46, 0, &buf0[0]))< 0){
            printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
            return -1;
        }
        udelay(10);
        if((i2c_smbus_read_i2c_block_data(i2c_client, 0x59, 4, &buf[0+j]))< 0){
            printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
            return -1;
        }	
        udelay(10);
        j=j+4;
	}

	return 1;
}

#if HIMAX_UPGRADE_WITH_IFILE
int himax_read_FW_checksum(void)  //20130124z
{
	int fullFileLength = fw_size;//sizeof(HIMAX_FW); //Paul Check
	//u16 rem;
	u16 FLASH_VER_START_ADDR = 1030;
	u16 FW_VER_END_ADDR = 4120;
	u16 i, j, k;
	unsigned char cmd[4];
	int ret;
	u8 fail_count = 0;
		
	printk("Himax  TP version check start");
	//if((i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &cmd[0]))< 0)	{ret = -1;	goto ret_proc;}	
	//mdelay(120);
	
	//rem = fullFileLength % 4;
	FLASH_VER_START_ADDR = (fullFileLength / 4) - 3;
	//if (rem == 0 && FLASH_VER_START_ADDR > 1)
	//{	
	//	FLASH_VER_START_ADDR--;
	//	rem = 4;
	//}
	FW_VER_END_ADDR = FLASH_VER_START_ADDR * 4;

	himax_FlashMode(1);	

	printk("Himax TP version check for loop start");
	for (k = 0; k < 3; k++)
	{
		ret = 1;
		j = FW_VER_END_ADDR;
		
		cmd[0] = FLASH_VER_START_ADDR & 0x1F;
		cmd[1] = (FLASH_VER_START_ADDR >> 5) & 0x1F;
		cmd[2] = (FLASH_VER_START_ADDR >> 10) & 0x1F;
		
		if((i2c_smbus_write_i2c_block_data(i2c_client, 0x44, 3, &cmd[0]))< 0)	{ret = -1;	goto ret_proc;}
		if((i2c_smbus_write_i2c_block_data(i2c_client, 0x46, 0, &cmd[0]))< 0)	{ret = -1;	goto ret_proc;}
		if((i2c_smbus_read_i2c_block_data(i2c_client, 0x59, 4, &cmd[0]))< 0)	{ret = -1; 	goto ret_proc;}
		
		//for (i = 0; i < rem; i++)
		for (i = 0; i < 4; i++)
		{
			printk("Ghong_zguoqing_marked TP version check, CTPW[%x]:%x, cmd[0]:%x\n", j, HIMAX_FW[j], cmd[i]);
			if (HIMAX_FW[j] != cmd[i])
			{
				ret = 0;
				break;
			}
			j++;
		}

		if (ret == 0)	fail_count++;
		//if (ret == 1)	break;
	}
	
ret_proc:
	himax_FlashMode(0);	
	
	printk("Himax TP version check loop count[%d], fail count[%d]\n", k, fail_count);
	printk("Himax TP version check for loop end, return:%d", ret);

	return ret;
}
#endif


u8 himax_read_FW_ver(void) //OK
{
	unsigned int i;
	unsigned char cmd[5];
	unsigned char buffer[20];

	//Himax: Power On Flow
	cmd[0] =0x02;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x42, 1, &cmd[0]))< 0)
		goto HimaxErr;
	msleep(1);
	
	cmd[0] =0x02;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x35, 1, &cmd[0]))< 0)
		goto HimaxErr;
	msleep(1);
		
	cmd[0] =0x0F;
	cmd[1] =0x53;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x36, 2, &cmd[0]))< 0)
		goto HimaxErr;
	msleep(1);
	
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &cmd[0]))< 0)
		goto HimaxErr;
	msleep(120);
	//Himax: Power On Flow End 

	//Himax: Flash Read Enable	
	cmd[0] = 0x01;
	cmd[1] = 0x00;
	cmd[2] = 0x02;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		goto HimaxErr;
	msleep(1);

	//Himax: Read FW Major Version
	//if (hx8526_read_flash(FW_VER_MAJ_FLASH_buff, FW_VER_MAJ_FLASH_ADDR, FW_VER_MAJ_FLASH_LENG) < 0)	goto HimaxErr;
	//Himax: Read FW Minor Version
	//if (hx8526_read_flash(FW_VER_MIN_FLASH_buff, FW_VER_MIN_FLASH_ADDR, FW_VER_MIN_FLASH_LENG) < 0)	goto HimaxErr;
	//Himax: Read CFG Major Version
	if (hx8526_read_flash(CFG_VER_MAJ_FLASH_buff, CFG_VER_MAJ_FLASH_ADDR, CFG_VER_MAJ_FLASH_LENG) < 0)	goto HimaxErr;
	//Himax: Read CFG Minor Version
	if (hx8526_read_flash(CFG_VER_MIN_FLASH_buff, CFG_VER_MIN_FLASH_ADDR, CFG_VER_MIN_FLASH_LENG) < 0)	goto HimaxErr;

	//{
	//Himax: Check FW Major Version
	//sprintf(buffer, "%s\n", FW_VER_MAJ_FLASH_buff);
	//TPD_DMESG("Himax TP: FW_VER_MAJ_FLASH_buff = %x,%x,%x,%x\n", FW_VER_MAJ_FLASH_buff[0],FW_VER_MAJ_FLASH_buff[1],FW_VER_MAJ_FLASH_buff[2],FW_VER_MAJ_FLASH_buff[3]);
	
	//Himax: Read FW Minor Version
	//sprintf(buffer, "%s\n", FW_VER_MIN_FLASH_buff);
	//TPD_DMESG("Himax TP: FW_VER_MIN_FLASH_buff = %x,%x,%x,%x\n", FW_VER_MIN_FLASH_buff[0],FW_VER_MIN_FLASH_buff[1],FW_VER_MIN_FLASH_buff[2],FW_VER_MIN_FLASH_buff[3]);
	
	//Himax: Read CFG Major Version
	//sprintf(buffer, "%s\n", CFG_VER_MAJ_FLASH_buff);
	//TPD_DMESG("Himax TP: CFG_VER_MAJ_FLASH_buff = %s\n", buffer);
	
	//Himax: Read CFG Minor Version
	//sprintf(buffer, "%s\n", CFG_VER_MIN_FLASH_buff);
	//TPD_DMESG("Himax TP: CFG_VER_MIN_FLASH_buff = %s\n", buffer);
	//}
	
	//Himax: Flash Read Disable
	cmd[0] = 0x00;
	cmd[1] = 0x00;
	cmd[2] = 0x02;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
		goto HimaxErr;
	msleep(1);
	return 1;

HimaxErr:
	return 0;
}
#endif

//Himax: Define I2C Data Structure    
#if Android4_0			//Himax: Define I2C Data Structure for 4.0
static struct i2c_driver tpd_i2c_driver = {
 .driver = {
	.name = "mtk-tpd",
 },
  .probe = tpd_probe,
  .remove = __devexit_p(tpd_remove),
 .id_table = tpd_i2c_id,
  .detect = tpd_detect,
};
#endif

#if Android2_3			//Himax: Define I2C Data Structure for 2.3
static struct i2c_driver tpd_i2c_driver = {  
  .driver = {
	 .name = TPD_DEVICE,
	 .owner = THIS_MODULE,
  },
  .probe = tpd_probe,
  .remove = __devexit_p(tpd_remove),
  .id_table = tpd_id,
  .detect = tpd_detect,
  .address_data = &addr_data,
 };
#endif

 
extern struct input_dev *kpd_input_dev;

static int himax_ts_poweron(void)
{    

	char data[3];
	int retval = TPD_OK;
		
		if (IC_TYPE == HX_85XX_A_SERIES_PWON)
		{    //Himax Touch Controller power-on sequence
			data[0] =0x02;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x42, 1, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);
			data[0] =0x02;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x35, 1, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);
						
			data[0] =0x0f;
			data[1] =0x53;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x36, 2, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);	
			data[0] =PrePatch;
			data[1] =0x02;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0xdd, 2, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);
			
			data[0] =0x01;
			data[1] =0x2D;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x76, 2, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);
			
			data[0] =0x01;
			data[1] =0xF5;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0xcb, 2, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);
			
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x83, 0, &data[0]))< 0)
			{
				return -1;
			}
			msleep(120);
			
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &data[0]))< 0)
			{
				return -1;
			}
			msleep(120);
		}			
		else if (IC_TYPE == HX_85XX_D_SERIES_PWON)
		{    //Himax Touch Controller power-on sequence
			data[0] =0x02;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x42, 1, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);
			
			data[0] =0x0f;
			data[1] =0x53;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x36, 2, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);	
			data[0] =0x04;
			data[1] =0x03;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0xdd, 2, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);
			
			data[0] =0x01;
			data[1] =0x36;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0xb9, 2, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);
			
			data[0] =0x01;
			data[1] =0xF5;
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0xcb, 2, &data[0]))< 0)
			{
				return -1;
			}
			msleep(1);
			
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x83, 0, &data[0]))< 0)
			{
				return -1;
			}
			msleep(40);
			
			if((i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &data[0]))< 0)
			{
				return -1;
			}
			msleep(40);
			
			
			#if HIMAX_DEBUG
				printk("hx8526 power on success\n");
		  	#endif
			
		
		}//end HX_85XX_D_SERIES_PWON
		else 
		{
		 	printk("hx8526 power on fail!!\n");
			
		}	
    return retval;   
}

//Himax: Touch Down for Coor.
static  void tpd_down(int x, int y, int p) 
{
	input_report_key(tpd->dev, BTN_TOUCH, 1);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 20);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
#if HIMAX_DEBUG
	TPD_DMESG("tpd_down[%4d %4d %4d]\n ", x, y, p);
#endif
	/* track id Start 0 */
	//input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, p); 	//20130322 paco 
	input_mt_sync(tpd->dev);
	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
	{   
		tpd_button(x, y, 1);  
	}
}

//Himax: Touch Up for Coor. and Key  
static  int tpd_up(int x, int y, int *count) 
{	
	char data[3];
#if HIMAX_DEBUG
	TPD_DMESG("tpd_up[%4d %4d]\n ", x, y);
#endif
	input_report_key(tpd->dev, BTN_TOUCH, 0);
	input_mt_sync(tpd->dev);
	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
	{   
		tpd_button(x, y, 0); 
	}

}

kal_bool keyflag= false;

int i2c_read_bytes(unsigned char addr,int len,unsigned char* buf)
{
    struct i2c_msg msg[2];
    int ret;
 //printk("hx8526 : %s enter\n",__FUNCTION__);
    msg[0].addr = i2c_client->addr;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = &addr;
    msg[0].timing = I2C_MASTER_CLOCK;
    
    msg[1].addr = i2c_client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = len;
    msg[1].buf =buf;
    msg[1].timing = I2C_MASTER_CLOCK;

    ret = i2c_transfer(i2c_client->adapter, msg, 2);
    //printk("hx8526 : %s ret = %d\n",__FUNCTION__,ret);    
    return ret;
}


#ifdef HX_ESD_WORKAROUND
void ESD_HW_REST(void)
{
	int retval = TPD_OK;
	
	if (NO_ESD_RESET == 0)
  {
  		ESD_RESET_ACTIVATE = 1;
			ESD_COUNTER = 0;
	
		//	printk(KERN_ERR "[Himax TP] %s: ESD_HW_RESET  - INN_Reset  start %d\n ",__func__,__LINE__);		
			
			mutex_lock(&i2c_access);
			
			mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
			msleep(20);
			mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
			msleep(20);
		
			if( himax_ts_poweron() < 0)
			{
				//goto ESD_HW_RESET_IIC_FAIL;
	//			printk(KERN_ERR "[Himax TP] %s: ESD_HW_RESET  - INN_powre on fail %d \n",__func__,__LINE__);
				return -1;
		
			}
			mutex_unlock(&i2c_access);
			
			mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
			
		//	printk(KERN_ERR "[Himax TP] %s: ESD_HW_RESET - INN_Reset  end %d\n",__func__,__LINE__);
  }
	
	return retval;
}
#endif

int himax_hang_shaking(void)    //0:Running, 1:Stop, 2:I2C Fail
{
	int ret, result;
	uint8_t hw_reset_check[1];
	uint8_t hw_reset_check_2[1];
	uint8_t buf0[2];

	//Mutexlock Protect Start
	mutex_lock(&i2c_access);
	//Mutexlock Protect End	

//	printk(KERN_ERR "[Himax TP]: ESD - hang_shaking  start\n",__func__,__LINE__);


	//Write 0x92
	buf0[0] = 0x92;
	if(IC_STATUS_CHECK == 0xAA)
	{
		buf0[1] = 0xAA;
		IC_STATUS_CHECK = 0x55;
	}
	else
	{
		buf0[1] = 0x55;
		IC_STATUS_CHECK = 0xAA;
	}
	//i2c_smbus_write_i2c_block_data(i2c_client, command_91h[0], 1, &command_91h[1]);
	//i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &data[0]);
	//ret = i2c_himax_master_write(private_ts->client, buf0, 2, DEFAULT_RETRY_CNT);
	ret = i2c_smbus_write_i2c_block_data(i2c_client,buf0[0], 1,&buf0[1]);
	if(ret < 0) 
	{
		printk(KERN_ERR "[Himax TP]:write 0x92 failed line: %d \n",__func__,__LINE__);
		goto work_func_send_i2c_msg_fail;
	}
	mdelay(30); //Must more than 1 frame

	buf0[0] = 0x92;
	buf0[1] = 0x00;
	//			ret = i2c_himax_master_write(private_ts->client, buf0, 2, DEFAULT_RETRY_CNT);
	ret = i2c_smbus_write_i2c_block_data(i2c_client,buf0[0], 1,&buf0[1]);
	if(ret < 0) 
	{
		printk(KERN_ERR "[Himax TP]:write 0x92 failed line: %d \n",__func__,__LINE__);
		goto work_func_send_i2c_msg_fail;
	}
	mdelay(2);

	//		ret = i2c_himax_read(private_ts->client, 0xDA, hw_reset_check, 1, DEFAULT_RETRY_CNT);
	ret = i2c_smbus_read_i2c_block_data(i2c_client,0xDA, 1,&hw_reset_check[0]);
	if(ret < 0)
	{
		printk(KERN_ERR "[Himax TP]:i2c_himax_read 0xDA failed line: %d \n",__func__,__LINE__);
		goto work_func_send_i2c_msg_fail;
	}
	//printk("[Himax]: ESD 0xDA - 0x%x.\n", hw_reset_check[0]); 

	if((IC_STATUS_CHECK != hw_reset_check[0]))
	{
		mdelay(2);
		//	ret = i2c_himax_read(private_ts->client, 0xDA, hw_reset_check_2, 1, DEFAULT_RETRY_CNT);
		ret = i2c_smbus_read_i2c_block_data(i2c_client,0xDA, 1,&hw_reset_check_2[0]);
		if(ret < 0)
		{
			printk(KERN_ERR "[Himax TP]:i2c_himax_read 0xDA failed line: %d \n",__func__,__LINE__);
			goto work_func_send_i2c_msg_fail;
		}
		//printk("[Himax]: ESD check 2 0xDA - 0x%x.\n", hw_reset_check_2[0]);

		if(hw_reset_check[0] == hw_reset_check_2[0])
		{
			result = 1; //MCU Stop
		//	printk(KERN_ERR "[Himax TP]: ESD - hang_shaking result =1 MCU stop\n",__func__,__LINE__);

		}
		else
		{
			result = 0; //MCU Running
		//	printk(KERN_ERR "[Himax TP]: ESD - hang_shaking result =0  MCU running\n",__func__,__LINE__);

		}
	}
	else
	{
		result = 0; //MCU Running
	}

	//Mutexlock Protect Start
	mutex_unlock(&i2c_access);
	//Mutexlock Protect End

//	printk(KERN_ERR "[Himax TP]: ESD - hang_shaking  end\n",__func__,__LINE__);


	return result;

work_func_send_i2c_msg_fail:
	//Mutexlock Protect Start
	mutex_unlock(&i2c_access);
	//Mutexlock Protect End
	return 2;

//	printk(KERN_ERR "[Himax TP]: ESD - hang_shaking  end\n",__func__,__LINE__);

}

//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
#ifdef ENABLE_CHIP_RESET_MACHINE
static void himax_chip_reset_function(struct work_struct *dat)
{
	printk("[Himax]:himax_chip_reset_function ++ \n");

	if(retry_time <= 10)
	{
		#ifdef HX_ESD_WORKAROUND
		ESD_RESET_ACTIVATE = 1;
		#endif

		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
		msleep(20);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
		msleep(20);

		himax_ts_poweron();
		
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
	}
	retry_time ++;
	printk("[Himax]:himax_chip_reset_function retry_time =%d --\n",retry_time);
}
#endif
//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end


//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start
#ifdef ENABLE_CHIP_STATUS_MONITOR
static int himax_chip_monitor_function(struct work_struct *dat) //for ESD solution
{
	int ret;
	
	if(running_status == 0)//&& himax_chip->suspend_state == 0)
	{
		#ifdef HX_RST_PIN_FUNC
			if(mt_get_gpio_in(GPIO_CTP_RST_PIN) == 0)
			{
				printk("[Himax]%s: IRQ = 0, Enable IRQ\n", __func__);
				mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
			}
		#endif

		ret = himax_hang_shaking(); //0:Running, 1:Stop, 2:I2C Fail
		if(ret == 2)
		{
			#ifdef ENABLE_CHIP_RESET_MACHINE
			queue_delayed_work(himax_wq, &himax_chip_reset_work, 0);
			#endif
			printk(KERN_INFO "[Himax] %s: I2C Fail \n", __func__);
		}
		if(ret == 1)
		{
			printk(KERN_INFO "[Himax] %s: MCU Stop \n", __func__);
			
			#ifdef ENABLE_CHIP_RESET_MACHINE
			retry_time = 0;
			#endif
			
			#ifdef HX_ESD_WORKAROUND
			ESD_HW_REST();
			#endif
		}
		
		queue_delayed_work(himax_wq, &himax_chip_monitor, 5*HZ);
	}
	return 0;
}
#endif

static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)
{
#if ((HIMAX_RAWDATA) | (HIMAX_UPGRADE))
    //uint8_t *mutual_data;
    //uint8_t *self_data;
    //int mul_num, self_num;
    //int index = 0;
	int read_len = 0;
	int ret = 0;
	//Bizzy added for common RawData
    int raw_cnt_max = PT_NUM_MAX/4;
    int raw_cnt_rmd = PT_NUM_MAX%4;
    int hx_touch_info_size, RawDataLen;
    if(raw_cnt_rmd != 0x00)
    {
    	if (IC_TYPE == HX_85XX_D_SERIES_PWON)
		{
			RawDataLen = 128 - ((PT_NUM_MAX+raw_cnt_max+3)*4) - 1;
     	}
		else if (IC_TYPE == HX_85XX_A_SERIES_PWON) 
		{
			RawDataLen = 128 - ((PT_NUM_MAX+raw_cnt_max+3)*4);
		}
		
		hx_touch_info_size = (PT_NUM_MAX+raw_cnt_max+2)*4;
	}
	else
	{
		if (IC_TYPE == HX_85XX_D_SERIES_PWON)
		{
			RawDataLen = 128 - ((PT_NUM_MAX+raw_cnt_max+2)*4) - 1;
		}
		else if  (IC_TYPE == HX_85XX_A_SERIES_PWON) 
		{	
			RawDataLen = 128 - ((PT_NUM_MAX+raw_cnt_max+2)*4);
		}
		hx_touch_info_size = (PT_NUM_MAX+raw_cnt_max+1)*4;
	}
#endif

	int i = 0;
	char data[128] = {0};
	u8 check_sum_cal = 0;
	u16 high_byte,low_byte;
#if HIMAX_CHECKSUM
	//const u8 CHECK_SUM_LENGTH = 4*(PT_NUM_MAX+2) ;		//If PT_NUM_MAX <= 4
	const u8 CHECK_SUM_LENGTH = 4*(PT_NUM_MAX+3) ;   	//If PT_NUM_MAX >= 5 && <8 
	//const u8 CHECK_SUM_LENGTH = 4*(PT_NUM_MAX+4) ;   	//If PT_NUM_MAX >= 8 && <=10
#endif
	int err[4] = {0};
	
	//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start	
	#ifdef ENABLE_CHIP_STATUS_MONITOR
	running_status = 1;
	cancel_delayed_work_sync(&himax_chip_monitor);
	#endif
	//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end
	
#if HIMAX_UPGRADE
	#if HIMAX_RAWDATA
    if(diag_command) 
        read_len = 128;
    else
	#endif
        read_len = hx_touch_info_size;
	
	//Himax: Receive raw data about Coordinates and
	mutex_lock(&i2c_access);
	if (tpd_halt)
	{
		mutex_unlock(&i2c_access);
		#if HIMAX_DEBUG
		TPD_DMESG("Himax TP: tpd_touchinfo return ..\n");
		#endif
		return false;
	}
	#ifdef MTK6589_DMA
	 hx8526_i2c_dma_recv_data(i2c_client, 0x86, read_len,&(data[0]));
	#else
	for(i=0;i<read_len;i=i+8)
		hwmsen_read_block(i2c_client, 0x86, &(data[i]), 8);
	#endif
	mutex_unlock(&i2c_access);
#else
	mutex_lock(&i2c_access);
	if (tpd_halt)
	{
		mutex_unlock(&i2c_access);
		#if HIMAX_DEBUG
		TPD_DMESG("Himax TP: tpd_touchinfo return ..\n");
		#endif
		return false;
	}
	
	//Himax: Receive raw data about Coordinates and 
	err[0] = hwmsen_read_block(i2c_client, 0x86, &(data[0x00]), 8);	// 1st/2nd layer
	err[1] = hwmsen_read_block(i2c_client, 0x86, &(data[0x08]), 8);	// 3rd/4th layer
	err[2] = hwmsen_read_block(i2c_client, 0x86, &(data[0x10]), 8);	// 5th/6th layer
	#if HIMAX_5POINT_SUPPORT
	err[3] = hwmsen_read_block(i2c_client, 0x86, &(data[0x18]), 8);	// 7th/8th layer  // Modified 20121101
	#endif
	mutex_unlock(&i2c_access);
#endif
	
#if HIMAX_DEBUG
	TPD_DMESG("received raw data from touch panel as following:\r\n");
	TPD_DMESG("x1:%x,y1:%x\r\n", (u16)((data[0]<<8)|data[1]),(u16)((data[2]<<8)|data[3]));
	TPD_DMESG("x2:%x,y2:%x\r\n", (u16)((data[4]<<8)|data[5]),(u16)((data[6]<<8)|data[7]));
	TPD_DMESG("x3:%x,y3:%x\r\n", (u16)((data[8]<<8)|data[9]),(u16)((data[10]<<8)|data[11]));
	TPD_DMESG("x4:%x,y4:%x\r\n", (u16)((data[12]<<8)|data[13]),(u16)((data[14]<<8)|data[15]));
	TPD_DMESG("x5:%x,y5:%x\r\n", (u16)((data[16]<<8)|data[17]),(u16)((data[18]<<8)|data[19]));
	TPD_DMESG("area1:%x,area2:%x,area3:%x,area4:%x\r\n", data[20],data[21],data[22],data[23]);
	TPD_DMESG("area5:%x,area6:%x,area7:%x,area8:%x\r\n", data[24],data[25],data[26],data[27]);    			// modified 20121101
	TPD_DMESG("point num:%x,ID info1:%x,ID info2:%x,check sum:%x\r\n", data[28],data[29],data[30],data[31]);	// modified 20121101
#endif

#if HIMAX_RAWDATA
	if (diag_command >= 1 && diag_command <= 6) 
	{
		int mul_num, self_num;
		int index = 0;
		/* Header: %x, %x, %x, %x\n", buf[24], buf[25], buf[26], buf[27] */
		mul_num = x_channel * y_channel;
		self_num = x_channel + y_channel;

		if (data[hx_touch_info_size] == data[hx_touch_info_size+1] && data[hx_touch_info_size+1] == data[hx_touch_info_size+2] 
			&& data[hx_touch_info_size+2] == data[hx_touch_info_size+3] && data[hx_touch_info_size] > 0) 
		{
			index = (data[hx_touch_info_size] - 1) * RawDataLen;

			if (IC_TYPE == HX_85XX_D_SERIES_PWON)
			{
				for (i = 0; i < RawDataLen; i++) 			
				{		    	
					if ((index+i) < mul_num) 		    	
					{		    			
						diag_mutual[index + i] = data[i + hx_touch_info_size+4];		    		
					}				
					else					
					{						
						if ((i+index) >= (self_num+mul_num))							
							break;						
						diag_self[i+index-mul_num] = data[i + hx_touch_info_size+4];											
					}											
				}
			}
			else
			{
				for (i = 0; i < RawDataLen; i++) 
				{
					if (index < mul_num) 
					{ 
						diag_mutual[index + i] = data[i + hx_touch_info_size+4];	//4: RawData Header
					} 
					else 
					{//self
						if (i >= self_num)
							break;
						diag_self[i] = data[i + hx_touch_info_size+4];	//4: RawData Header
					}
				}
			}
		}
	}
#endif

#ifdef HX_ESD_WORKAROUND
		for(i = 0; i < CHECK_SUM_LENGTH; i++)
		{
				if(data[i] == 0x00)
				{
					check_sum_cal = 1;
				}		
				else if(data[i]==0xED)
				{
				    check_sum_cal =2;				
				}
				else
				{
						check_sum_cal = 0;
						i = CHECK_SUM_LENGTH;
				}
		}
		
		
     if(diag_command == 0)
	 {
		if((data[31] == 0x1f) && data[28] == 0xff)
			{
				ESD_UP_COUNTER++;
				if(ESD_UP_COUNTER > 4)
				{
				//	printk("[HIMAX TP MSG]: ESD event checked - ALL FF.\n");
					ESD_UP_COUNTER = 0;
					ESD_HW_REST();
					
					
				    #ifdef ENABLE_CHIP_STATUS_MONITOR
				    running_status = 0;
			 	    queue_delayed_work(himax_wq, &himax_chip_monitor, 5*HZ);
			    	#endif
					
					return;
				}
			}
			else
			{
				ESD_UP_COUNTER = 0;
			}
		
		}
		
		
		#if HIMAX_RAWDATA
		    #if ESD_WORKAROUND
			if((check_sum_cal != 0) &&( ESD_RESET_ACTIVATE == 0)&& (diag_command == 0))	//ESD Check
		    #else
			if(check_sum_cal != 0 && diag_command == 0)
			#endif
		#else
		    #if ESD_WORKAROUND
			if(check_sum_cal != 0 && ESD_RESET_ACTIVATE == 0 )  //ESD Check
			#else
			if(check_sum_cal !=0)
			#endif
		#endif	
		{
			#if HIMAX_DEBUG
				printk(KERN_ERR "[Himax TP]: ESD event check\n",__func__,__LINE__);
			#endif

				ret = himax_hang_shaking(); //0:Running, 1:Stop, 2:I2C Fail
			
			if(ret == 2)
				{
					printk(KERN_ERR "[Himax TP]:iic transfer fail \n",__func__,__LINE__);
					goto work_func_send_i2c_msg_fail;

				}
				
				if((ret == 1) && (check_sum_cal == 1))
				{   
					//printk("[Himax TP]: ret=1 check_sum_cal=1 . inn \n",__func__,__LINE__);
				//	printk("[Himax TP]: ESD event checked - ALL Zero. inn\n",__func__,__LINE__);
					ESD_HW_REST();
				}
				else if(check_sum_cal == 2)
				{
				//	printk("[Himax TP]: check_sum_cal=2 .inn \n",__func__,__LINE__);
				//	printk("[Himax TP]: ESD event checked - ALL 0xED.inn \n");
					ESD_HW_REST();
				}
				//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------start
				#ifdef ENABLE_CHIP_STATUS_MONITOR
				running_status = 0;
				queue_delayed_work(himax_wq, &himax_chip_monitor, 5*HZ);
				#endif
				//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------end

				return;
		}
		else if(ESD_RESET_ACTIVATE) 
		{
			ESD_RESET_ACTIVATE = 0;
			#if HIMAX_DEBUG
			printk(KERN_ERR "[Himax TP]: Back from ESD reset, ready to serve.\n",__func__);
			#endif
			
			mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);		

			//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------start
			#ifdef ENABLE_CHIP_STATUS_MONITOR
			running_status = 0;
			queue_delayed_work(himax_wq, &himax_chip_monitor, 5*HZ);
			#endif
			//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------end
			//return ;
		}
#endif 

#if HIMAX_CHECKSUM
	for(i = 0; i < CHECK_SUM_LENGTH; i++)
		check_sum_cal += data[i];

	#if HIMAX_5POINT_SUPPORT
	if (check_sum_cal != 0x00 || data[28] & 0xF0 != 0xF0)
	#else
	if (check_sum_cal != 0x00 || data[20] & 0xF0 != 0xF0)	
	#endif
	{
#if HIMAX_DEBUG 
		TPD_DMESG("Himax TP: Coor. Checksum Error\n");
		#endif
		//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------start
			#ifdef HX_ESD_WORKAROUND
			ESD_COUNTER++;
	//		printk("[HIMAX TP MSG]: ESD event checked - check_sum_cal, ESD_COUNTER = %d.\n", ESD_COUNTER);
			if(ESD_COUNTER > ESD_COUNTER_SETTING)
			{
				ESD_HW_REST();
			}
			#endif
		//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------end
		
		//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------start
			#ifdef ENABLE_CHIP_STATUS_MONITOR
			running_status = 0;
			queue_delayed_work(himax_wq, &himax_chip_monitor, 5*HZ);
			#endif
		//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------end
		
		return false;
	}
#endif

#if HIMAX_BUTTON	
	//Himax: Get Soft Key number
	keyid  = data[22];
#endif
	
	//Himax: Get the number of the touch points
	#if HIMAX_5POINT_SUPPORT
	if (data[28] == 0xFF)
	{
		point_num = 0;
	}
	else
	{
		point_num = data[28] & 0x07;
	}
	#else
	if (data[20] == 0xFF)
	{
		point_num = 0;
	}
	else
	{
		point_num = data[20] & 0x07;
	}
	#endif /* HIMAX_5POINT_SUPPORT */

#ifdef LCT_PROJECT_LCT_W9600A_A01	// add by gpg for W9660 vk
	do{
		int tpd_key = 0;
		static int tpd_key_down = 0;
		
		tpd_key = data[30]>> 4 & 0x0f;
		if( tpd_key != 0x0f )
		{
			switch(tpd_key)
			{
				case 1:
				case 2:
				case 3:	
					tpd_key--;
					cinfo->x[0] = tpd_keys_dim_local[tpd_key][0];
					cinfo->y[0] = tpd_keys_dim_local[tpd_key][1];
					cinfo->id[0] = 0;
					tpd_key_down = 1;
					cinfo->count = 1;

					cinfo->x[1] = 0xffff;
					cinfo->y[1] = 0xffff;	
					cinfo->x[2] = 0xffff;
					cinfo->y[2] = 0xffff;
					cinfo->x[3] = 0xffff;
					cinfo->y[3] = 0xffff;
					cinfo->x[4] = 0xffff;
					cinfo->y[4] = 0xffff;
					
			//		printk("GPG Hx8526 %d , %d, %d, %d\n",point_num, tpd_key, cinfo->x[0], cinfo->y[0]);
					point_num = 1;
					return true;
				default:
					break;
			}
		}
		else
		{
			// release
		}
	}while(0);
#endif  //end by gpg

	//Himax: Check Coor.
	for (i = 0; i < PT_NUM_MAX; i++)
	{ 
		if (data[4*i] != 0xFF)
		{
			/*get the X coordinate, 2 bytes*/
			high_byte = data[4*i];
			high_byte <<= 8;
			high_byte &= 0x0f00;
			low_byte = data[4*i + 1];
			high_byte = high_byte |low_byte;
			
			//Himax: Check X Resolution
			if (high_byte <= RESOLUTION_X)
			{
				cinfo->x[i] = high_byte;
			}
			else
			{
				cinfo->x[i] = 0xFFFF;
				cinfo->y[i] = 0xFFFF;
				#if HIMAX_DEBUG
				TPD_DMESG("Himax TP: X Coor. Error\n");
				#endif
				continue;
			}
				
			/*get the Y coordinate, 2 bytes*/
			high_byte = data[4*i+2];
			high_byte <<= 8;
			high_byte &= 0x0f00;
			low_byte = data[4*i+3];
			high_byte = high_byte |low_byte;
      	
			//Himax: Check Y Resolution
			if (high_byte <= RESOLUTION_Y)
			{
				cinfo->y[i] = high_byte;
			}
			else
			{
				cinfo->x[i] = 0xFFFF;
				cinfo->y[i] = 0xFFFF;
				#if HIMAX_DEBUG
				TPD_DMESG("Himax TP: Y Coor. Error\n");
				#endif
				continue;
			}

			/*get the point index of touch point*/
			if ((data[29]&0x1F)>>i)
			{
				cinfo->id[i] = i;
			}
			cinfo->count++;
			
			//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------start
			#ifdef HX_ESD_WORKAROUND
			ESD_COUNTER = 0;
#endif
		}
		else
		{
			cinfo->x[i] = 0xFFFF;
			cinfo->y[i] = 0xFFFF;
			cinfo->id[i] = 0xFFFF;
			
			//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------start
			#ifdef HX_ESD_WORKAROUND
			ESD_COUNTER = 0;
			#endif
			//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------end
		}
	}
		  
	//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start
	#ifdef ENABLE_CHIP_STATUS_MONITOR
	running_status = 0;
	queue_delayed_work(himax_wq, &himax_chip_monitor,5*HZ);
	#endif
	//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end
	
	return true;
	
	work_func_send_i2c_msg_fail:

		printk(KERN_ERR "[HIMAX TP ERROR]:work_func_send_i2c_msg_fail: %d \n",__LINE__);

		//----[ENABLE_CHIP_RESET_MACHINE]-------------------------------------------------------------------start
		#ifdef ENABLE_CHIP_RESET_MACHINE
		if(init_success)
		{
			queue_delayed_work(himax_wq, &himax_chip_reset_work, 0);
		}
		#endif
		//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------end

		//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------start
		#ifdef ENABLE_CHIP_STATUS_MONITOR
		running_status = 0;
		queue_delayed_work(himax_wq, &himax_chip_monitor, 5*HZ);
		#endif
		//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------end
};

static char touch_screen_is_down=0;

static int touch_event_handler(void *unused)
 {
	struct touch_info cinfo, pinfo;
	const u8 PT_LEAVE = 1;
	u8 i;

	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	sched_setscheduler(current, SCHED_RR, &param);

	do
	{
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
		set_current_state(TASK_INTERRUPTIBLE); 
		wait_event_interruptible(waiter,tpd_flag!=0);
						 
		tpd_flag = 0;

		set_current_state(TASK_RUNNING);

		if (tpd_touchinfo(&cinfo, &pinfo)) 
		{
			//TPD_DEBUG("point_num = %d\n",point_num);
			TPD_DEBUG_SET_TIME;

			for(i = 0; i < PT_NUM_MAX; i++)
			{
				if (cinfo.x[i] != 0xFFFF)
				{
					//if (cinfo.p[i] != PT_LEAVE)
						tpd_down(cinfo.x[i], cinfo.y[i], cinfo.id[i]);
					//else
						//tpd_up(cinfo.x[i], cinfo.y[i],i+1);
				}
			}
			if (point_num == 0)
				tpd_up(cinfo.x[0], cinfo.y[0], i + 1);
			input_sync(tpd->dev);
			/*
			if(point_num > 0) 
			{
				tpd_down(cinfo.x[0], cinfo.y[0], 1);
				if(point_num>1)
				{
					tpd_down(cinfo.x[1], cinfo.y[1], 2);
					if(point_num >2) tpd_down(cinfo.x[2], cinfo.y[2], 3);
					if(point_num >3) tpd_down(cinfo.x[3], cinfo.y[3], 4);
				}
				input_sync(tpd->dev);
				//TPD_DEBUG("press --->\n");

			} 
			else  
			{
				tpd_up(cinfo.x[0], cinfo.y[0],1);
				//tpd_up(cinfo.x[0], cinfo.y[0], 0);
				//TPD_DEBUG("release --->\n"); 
				//input_mt_sync(tpd->dev);
				input_sync(tpd->dev);
			}
			*/
		}
	}while(!kthread_should_stop());

	return 0;
 }

static int tpd_detect (struct i2c_client *client, int kind, struct i2c_board_info *info) 
{
	strcpy(info->type, TPD_DEVICE);
	return 0;
}
 
//static int Check_FW_Version(void);

static void tpd_eint_interrupt_handler(void)
{
	tpd_flag = 1;
	wake_up_interruptible(&waiter);
}

//Himax: HW_RESET
void himax_HW_reset(void)
{
	#ifdef HX_ESD_WORKAROUND
	ESD_RESET_ACTIVATE = 1;
	#endif

	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(20);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(20);
}

#if HIMAX_UPGRADE_WITH_IFILE
//Himax: Check FW Version
static int Check_FW_Version(void)
{
    int tp_ver, i_file_ver;
	unsigned char FW_VER_MAJ_FLASH_buff[FW_VER_MAJ_FLASH_LENG * 4];
	unsigned char FW_VER_MIN_FLASH_buff[FW_VER_MIN_FLASH_LENG * 4];
	unsigned char CFG_VER_MAJ_FLASH_buff[CFG_VER_MAJ_FLASH_LENG * 4];
	unsigned char CFG_VER_MIN_FLASH_buff[CFG_VER_MIN_FLASH_ADDR * 4];
	unsigned int i;
	unsigned char cmd[5];

  /*
	cmd[0] =0x02;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x42, 1, &cmd[0]))< 0)
	{
		return -1;
	}
	msleep(1);
	*/
	
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &cmd[0]))< 0)
	{
		return -1;
	}
	msleep(120);

	//if((i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &cmd[0]))< 0)	return 0;	msleep(100);

	//Himax: Flash Read Enable
	cmd[0] = 0x01;	cmd[1] = 0x00;	cmd[2] = 0x02;	
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)	return 0;	udelay(10);	

	//Himax: Read FW Major Version
	if (hx8526_read_flash(FW_VER_MAJ_FLASH_buff, FW_VER_MAJ_FLASH_ADDR, FW_VER_MAJ_FLASH_LENG) < 0)	goto err;
	//Himax: Read FW Minor Version
	if (hx8526_read_flash(FW_VER_MIN_FLASH_buff, FW_VER_MIN_FLASH_ADDR, FW_VER_MIN_FLASH_LENG) < 0)	goto err;
	//Himax: Read CFG Major Version
	if (hx8526_read_flash(CFG_VER_MAJ_FLASH_buff, CFG_VER_MAJ_FLASH_ADDR, CFG_VER_MAJ_FLASH_LENG) < 0)	goto err;
	//Himax: Read CFG Minor Version
	if (hx8526_read_flash(CFG_VER_MIN_FLASH_buff, CFG_VER_MIN_FLASH_ADDR, CFG_VER_MIN_FLASH_LENG) < 0)	goto err;
	#if 0
	//Himax: Check FW Major Version
	for (i = 0; i < FW_VER_MAJ_FLASH_LENG * 4; i++)	
	{
		if (FW_VER_MAJ_FLASH_buff[i] != *(HIMAX_FW + (FW_VER_MAJ_FLASH_ADDR * 4) + i))	
			return 1;	
	}
	
	//Himax: Read FW Minor Version
	for (i = 0; i < FW_VER_MIN_FLASH_LENG * 4; i++)	
	{
		if (FW_VER_MIN_FLASH_buff[i] != *(HIMAX_FW + (FW_VER_MIN_FLASH_ADDR * 4) + i))	
			return 1;	
	}
	
	//Himax: Read CFG Major Version
	for (i = 0; i < CFG_VER_MAJ_FLASH_LENG * 4; i++)	
	{
		if (CFG_VER_MAJ_FLASH_buff[i] != *(HIMAX_FW + (CFG_VER_MAJ_FLASH_ADDR * 4) + i))	
			return 1;	
	}
	for (i = 0; i < CFG_VER_MIN_FLASH_LENG * 4; i++)	
	{
		if (CFG_VER_MIN_FLASH_buff[i] != *(HIMAX_FW + (CFG_VER_MIN_FLASH_ADDR * 4) + i))	
			return 1;	
	}
	#else
	//Himax: Flash Read Disable
	cmd[0] = 0x00;	cmd[1] = 0x00;	cmd[2] = 0x02;	
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x43, 3, &cmd[0]))< 0)
	{
		udelay(10);
		return -1;
	}

	//Himax: Read CFG Minor Version
	if(sscanf(&(CFG_VER_MIN_FLASH_buff[1]), "%x", &tp_ver) == 0)
        {   
            printk("++++tp_fw=0x%x\n", tp_ver);
        return -1;
       }
	if(sscanf(HIMAX_FW + (CFG_VER_MIN_FLASH_ADDR * 4) + 1, "%x", &i_file_ver) == 0)
        {   
            printk("++++i_file_ver=0x%x\n", tp_ver);
            return -1;
       }
	printk("++++tp_fw=0x%x, i_file_ver=0x%x\n", tp_ver, i_file_ver);
	if ( tp_ver < i_file_ver)	// case-1: CTP firmware with lower version number detected		
		return 1;
	else if (tp_ver > i_file_ver)	// disable firmware upgrade procedure with i-file
		return -1;
	else
		return 0;					// firmware upgrade procedure depends on checksume data
	#endif

err:
	printk("Himax TP: FW update error exit\n");
	return -1;
}
#endif

#ifdef LCT_MTK_CTP_INFO_SUPPORT
static int ctp_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int cnt= 0;
	char ic_type[15] = {0};
	
	NO_ESD_RESET = 1;
	
	himax_HW_reset();	//Hardware Reset
	himax_read_FW_ver(); //OK
	himax_HW_reset();	//Hardware Reset
	//Himax: Power On Flow
	himax_ts_poweron();
	if (IC_TYPE == HX_85XX_A_SERIES_PWON)
	{
		strcpy(ic_type,"hx8526A01");
	}
	else
	{
		strcpy(ic_type,"hx8526D32");
	}
	cnt = sprintf(page, "vid:truly,firmware:%s,ic:%s\n",CFG_VER_MIN_FLASH_buff,ic_type);
	
	NO_ESD_RESET = 0;
	return cnt;
}
#endif /* LCT_MTK_CTP_INFO_SUPPORT */

void Himax_ic_package_check(void)  //20130403 Sunny added
{    
	unsigned char cmd[3];
	unsigned char data[3];
	
	i2c_smbus_read_i2c_block_data(i2c_client, 0xD1, 3, &(cmd[0]));		
		 
	i2c_smbus_read_i2c_block_data(i2c_client, 0x31, 3, &(data[0]));

	if((data[0] == 0x85 && data[1] == 0x28) || (cmd[0] == 0x04 && cmd[1] == 0x85 && (cmd[2] == 0x26 || cmd[2] == 0x27 || cmd[2] == 0x28)))
	{
		IC_TYPE = HX_85XX_D_SERIES_PWON;
		//Himax: Set FW and CFG Flash Address                                          
		FW_VER_MAJ_FLASH_ADDR	= 33;	//0x0085 0x0086                              
		FW_VER_MAJ_FLASH_LENG	= 1;;                                                
		FW_VER_MIN_FLASH_ADDR	= 34;  //0x0087                                     
		FW_VER_MIN_FLASH_LENG	= 1;                                                
		CFG_VER_MAJ_FLASH_ADDR = 40;	//0x009F              173	//0x02B4         
		CFG_VER_MAJ_FLASH_LENG = 3; 	//20130312                                 
		CFG_VER_MIN_FLASH_ADDR = 43;	//0x00AC              172	//0x02C0 20130312
		CFG_VER_MIN_FLASH_LENG = 3;   
		
		printk("Himax IC package 8528\n");
	}
	else if ((data[0] == 0x85 && data[1] == 0x26) || ((cmd[0] == 0x01 || cmd[0] == 0x02) && cmd[1] == 0x85 && cmd[2] == 0x26))
	{
		IC_TYPE = HX_85XX_A_SERIES_PWON;
		//Himax: Set FW and CFG Flash Address                                                  
		FW_VER_MAJ_FLASH_ADDR	= 33;	//0x0085  
		FW_VER_MAJ_FLASH_LENG	= 1;             
		FW_VER_MIN_FLASH_ADDR	= 182;	//0x02D8  
		FW_VER_MIN_FLASH_LENG	= 1;             
		CFG_VER_MAJ_FLASH_ADDR = 173;	//0x02B4
		CFG_VER_MAJ_FLASH_LENG = 3;           
		CFG_VER_MIN_FLASH_ADDR = 176;	//0x02C0
		CFG_VER_MIN_FLASH_LENG = 3; 
		
		printk("Himax IC package 8526\n");
	}
	else
	{ 
		printk("Himax IC package incorrect!!\n");
	}
}

extern int lct_tp_early_suspend ;
u8 isTP_Updated = 0;
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{	 
	int retval = TPD_OK;
	char data[5];
	#if HIMAX_UPGRADE_WITH_IFILE
	unsigned char cmd[5];
	#endif

	#ifdef VELOCITY_CUSTOM_HX8526
	int err=0;
	#endif
	i2c_client = client;
#if 1//def LCT_PROJECT_LCT_W9600A_A01
	i2c_client->timing = 400;
	
	#endif
	himax_wq = create_singlethread_workqueue("himax_wq");
	if (!himax_wq) 
	{
		printk(KERN_ERR "[HIMAX TP ERROR] %s: create workqueue failed\n", __func__);
	}

	//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
	#ifdef ENABLE_CHIP_RESET_MACHINE
	INIT_DELAYED_WORK(&himax_chip_reset_work, himax_chip_reset_function);
	init_success = 0;
	retry_time = 0;
	#endif
	//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end

	//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------start
	#ifdef HX_ESD_WORKAROUND   
	ESD_RESET_ACTIVATE = 0;
	//printk("[HIMAX TP MSG]: ESD in probe ESD_RESET_ACTIVATE = 0; .\n",__LINE__);
	#endif
	//----[HX_ESD_WORKAROUND]-------------------------------------------------------------------------------end
	
#if 0//def TPD_CLOSE_POWER_IN_SLEEP	 
	hwPowerDown(TPD_POWER_SOURCE,"TP");
	hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP");
	msleep(100);
#else
	#if 0
	//Himax: SET Power GPIO
	mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
	msleep(100);

	//Himax: SET HW_RESET GPIO
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(100);
	
	//Himax: HW_RESET
	himax_HW_reset();
	#endif
        mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);

        //#ifndef SLT_DRV_AW890_CONFIG
        hwPowerOn(MT65XX_POWER_LDO_VGP4, VOL_2800, "TP");
        hwPowerOn(MT65XX_POWER_LDO_VGP5, VOL_1800, "TP_EINT");
        //hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_1800, "TP");      
        msleep(100);
        //#endif

        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);//lllllllllllllllll
        msleep(100);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);//hhhhhhhhhhhhhhhhhhhhhhh
        msleep(100);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);//hhhhhhhhhhhhhhhhhhhhhhh
        msleep(100);

	
	#ifdef HX_ESD_WORKAROUND
	ESD_RESET_ACTIVATE = 1;
//	printk("[HIMAX TP MSG]: ESD in probe ESD_RESET_ACTIVATE = 1; .\n",__LINE__);
	#endif
	
	//Bizzy added
	Himax_ic_package_check();

  if (IC_TYPE == HX_85XX_A_SERIES_PWON)
	{
	#if ChangeIref1u
	retval = ChangeIrefSPP();
	TPD_DMESG("return value = {%d} ChangeIrefSPP by zhuxinglong\n",retval);
	msleep(20);
	//if(ret > 0)
		himax_HW_reset();	//Hardware Reset
    msleep(100);
	#endif
	}

  	#ifdef LCT_MTK_CTP_INFO_SUPPORT
	if(himax_read_FW_ver() == 1)
		TPD_DMESG("Himax TP: FW Read Pass\n");
	#ifdef SLT_DEVINFO_CTP
		sprintf(temp_ver, "%s\n",CFG_VER_MIN_FLASH_buff);
		devinfo_ctp_regchar("unkown","truly",temp_ver,DEVINFO_USED);
	#endif

		
	himax_HW_reset();
	msleep(100);
	#endif


	#if HIMAX_UPGRADE_WITH_IFILE
	if (himax_i_file_select_sub( )==1)
	{
		if (isTP_Updated == 0)
	 	{
	 	
	 	//--------------------------------------------------------------------------
			//Inital
			//--------------------------------------------------------------------------
			cmd[0] = 0x42;
			cmd[1] = 0x02;
			if(i2c_master_send(i2c_client, cmd, 2) < 0)
			{
				printk("entern Himax ChangeIrefSPP by zhuxinglong1\n");
				return 0;
			}
			udelay(10);

	//    data[0] =0x00;data[1] =0x04;data[2]=0x0A;data[3]=0x0A;data[4]=0x02;
	//    i2c_smbus_write_i2c_block_data(i2c_client, 0x7D, 5, &data[0);



			cmd[0] = 0x81;
			if(i2c_master_send(i2c_client, cmd, 1) < 0)
			{
				printk("entern Himax ChangeIrefSPP by zhuxinglong2\n");
				return 0;
			}
						mdelay(120);
						
				 	
	 	
		if (// Case-1: upgrade firmware with higher version number
			Check_FW_Version() > 0
			// Case-2: upgrade firmware with the same version number when checksum error founded
			|| (Check_FW_Version() == 0 && himax_read_FW_checksum() == 0))
			{
				if (himax_fw_upgrade_with_i_file() == 0)
				{
					isTP_Updated = 0;
					TPD_DMESG("Himax TP: Upgrade Error, line:%d\n", __LINE__);
				}
				else
				{
					isTP_Updated = 1;
					TPD_DMESG("Himax TP: Upgrade OK, line:%d\n", __LINE__);
				}
				
				// Himax: HW_RESET
				himax_HW_reset();
				msleep(100);
			}
			
		}
	}
	#endif


#endif
  
  //Himax: SET Interrupt GPIO, no setting PULL LOW or PULL HIGH  
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_DISABLE);
	//mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);

#if Android4_0
	//Himax: Probe Interrupt Function (Trigger Type detemine by CUST_EINT_TOUCH_PANEL_SENSITIVE = 0 Level Trigger; 1 Edge Trigger)
	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);  
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);

	//Himax: Edge Trigger Type  detemine by CUST_EINT_TOUCH_PANEL_POLARITY, Setting = 0 Falling Edge ; 1 Rising Edge) 
	//mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler,0);	
	mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler,1); 
#endif
/*
#if Android2_3
	//Himax: Probe Interrupt Function (Trigger Type detemine by CUST_EINT_TOUCH_PANEL_SENSITIVE = 1 Level Trigger; 0 Edge Trigger)
	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, 1);  
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);

	//Himax: Edge Trigger Type  detemine by CUST_EINT_TOUCH_PANEL_POLARITY, Setting = 0 Falling Edge ; 1 Rising Edge) 
	mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler,0);	
#endif
*/
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	msleep(100);

	printk(KERN_ERR "Himax TP: testonly1\n");
		

	
	//Himax: Power On Flow
	if( himax_ts_poweron() < 0)
    {
        return -1;
    }

	if(IC_TYPE == HX_85XX_D_SERIES_PWON)
	{
		lct_tp_early_suspend = 0;
	}
	
	tpd_load_status = 1;	

	//Himax: Power On Flow End

#if HIMAX_RAWDATA
    // Himax: register sysfs node for reading raw data
    setXChannel(DEFAULT_X_CHANNEL); // X channel
    setYChannel(DEFAULT_Y_CHANNEL); // Y channel

    setMutualBuffer();
    if (getMutualBuffer() == NULL) {
       TPD_DMESG("Himax TP: mutual buffer allocate fail failed\n");
       return -1; 
    }
#endif

#if ((HIMAX_UPGRADE) | (HIMAX_RAWDATA))
    himax_touch_sysfs_init();
#endif

	#ifdef LCT_MTK_CTP_INFO_SUPPORT
	g_ctp_proc = create_proc_entry(CTP_PROC_FILE, 0444, NULL);
	if (g_ctp_proc == NULL) {
		TPD_DMESG("create_proc_entry failed\n");
	} else {
		g_ctp_proc->read_proc = ctp_proc_read;
		g_ctp_proc->write_proc = NULL;
		//g_ctp_proc->owner = THIS_MODULE;
		TPD_DMESG("create_proc_entry success\n");
	}
	#endif
	
	#ifdef VELOCITY_CUSTOM_HX8526
	if((err = misc_register(&tpd_misc_device)))
	{
		TPD_DMESG("Himax TP: tpd_misc_device register failed\n");
		
	}
	#endif

	#ifdef MTK6589_DMA    
    gpDMABuf_va = (u8 *)dma_alloc_coherent(NULL, 4096, &gpDMABuf_pa, GFP_KERNEL);
    if(!gpDMABuf_va){
		printk(KERN_INFO "[elan] Allocate DMA I2C Buffer failed\n");
    }
	#endif

	
	//Himax: Start Touch INT
	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	if (IS_ERR(thread))
	{ 
		retval = PTR_ERR(thread);
		TPD_DMESG(TPD_DEVICE "Himax TP: Failed to create kernel thread: %d\n", retval);
	}
	
	
	//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start	
	#ifdef ENABLE_CHIP_STATUS_MONITOR
	INIT_DELAYED_WORK(&himax_chip_monitor, himax_chip_monitor_function); //for ESD solution
	running_status = 0;
	#endif
	//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end
	
	//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
	#ifdef ENABLE_CHIP_RESET_MACHINE
	init_success = 1;
	retry_time = 0;
	#endif
	//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end

	//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start
	#ifdef ENABLE_CHIP_STATUS_MONITOR
	queue_delayed_work(himax_wq, &himax_chip_monitor, 60*HZ);   //for ESD solution
	#endif
	//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end


	TPD_DMESG("Himax TP: Touch Panel Device Probe %s\n", (retval < TPD_OK) ? "FAIL" : "PASS");
	//return 0;
	
HimaxErr:
	
	//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
	#ifdef ENABLE_CHIP_RESET_MACHINE
	cancel_delayed_work(&himax_chip_reset_work);
	#endif
	//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end

	//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start
	#ifdef ENABLE_CHIP_STATUS_MONITOR
	cancel_delayed_work(&himax_chip_monitor);
	#endif
	//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end
	
	TPD_DMESG("Himax TP: I2C transfer error, line: %d\n", __LINE__);
	return -1;
}

static int __devexit tpd_remove(struct i2c_client *client)
{   
#if ((HIMAX_UPGRADE) | (HIMAX_RAWDATA))
	himax_touch_sysfs_deinit();
#endif

#ifdef LCT_MTK_CTP_INFO_SUPPORT
	remove_proc_entry(CTP_PROC_FILE, NULL);
#endif
#ifdef MTK6589_DMA    
if(gpDMABuf_va){
    dma_free_coherent(NULL, 4096, gpDMABuf_va, gpDMABuf_pa);
    gpDMABuf_va = NULL;
    gpDMABuf_pa = NULL;
}
#endif

	TPD_DMESG("Himax TP: TPD removed\n");
	return 0;
}
 
 
static int tpd_local_init(void)
{ 
	TPD_DMESG("HIMAX_TS I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);

	if(i2c_add_driver(&tpd_i2c_driver)!=0)
	{
		TPD_DMESG("unable to add i2c driver.\n");
		return -1;
	}
	
#ifdef TPD_HAVE_BUTTON
	tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif   
  
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif 

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_def_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
#endif  
		TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
		tpd_type_cap = 1;
    return 0; 
 }

//Himax: Resume Function
/*20130305
static int tpd_resume(struct i2c_client *client)
{
	int retval = TPD_OK;
	char data[5];
	
 	printk(KERN_ERR "Himax TP: Resume\n");

	mutex_lock(&i2c_access);
 	//Himax: Deep Sleep Out 
	data[0] =0x00;
  	i2c_smbus_write_i2c_block_data(i2c_client, 0xD7, 1, &data[0]);
  	msleep(5);

	//Himax: Reload Disable
	data[0] =0x02;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x42, 1, &data[0]);
	msleep(1);

	//Himax: MCU On
	data[0] =0x02;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x35, 1, &data[0]);
	msleep(1);

	//Himax: ROM Ready
	data[0] =0x0F;
	data[1] =0x53;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x36, 2, &data[0]);
	msleep(1);
	
	//Himax: Check Flash Pre-Patch
	data[0] =PrePatch; //Himax: Modify by case
	data[1] =0x02;
	i2c_smbus_write_i2c_block_data(i2c_client, 0xDD, 2, &data[0]);
	msleep(1);
	
	//Himax: Normal Mode
	data[0] =0x00;
	i2c_smbus_write_i2c_block_data(i2c_client, 0xE9, 1, &data[0]);
	msleep(1);
	
	//Himax: Sense On
	data[0] =0x00;data[1] =0x04;data[2]=0x0A;data[3]=0x0A;data[4]=0x02;
	i2c_smbus_write_i2c_block_data(i2c_client, 0x7D, 5, &data[0]);
	msleep(1);
	
	i2c_smbus_write_i2c_block_data(i2c_client, 0x83, 0, &data[0]);
	msleep(100);
	
	//Himax: Sleep Out
	i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &data[0]);
	msleep(100);
	mutex_unlock(&i2c_access);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	tpd_halt = 0;
	return retval;
}
*/
 static int tpd_resume(struct i2c_client *client)
 {
  int retval = TPD_OK;
  #if Android4_0
 	static char data[3];
 	#endif
 	#if Android2_3
 	char data[3];
 	#endif

    hwPowerOn(MT65XX_POWER_LDO_VGP4, VOL_2800, "TP");
    hwPowerOn(MT65XX_POWER_LDO_VGP5, VOL_1800, "TP_EINT");
    //msleep(50);
    himax_HW_reset();
	
 

	data[0] =0x00;
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0xD7, 1, &data[0]))< 0)
	{
		printk("tpd_resume send comand D7 failed\n");
	    return -1;
	}        
    msleep(1);

	
    if( himax_ts_poweron() < 0)
    {
    	printk("tpd_resumehimax_ts_poweron failed\n");
        return -1;
    }

	//i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &data);  //TP exit sleep mode
	//if (IC_TYPE == HX_85XX_A_SERIES_PWON)
   		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		tpd_halt = 0;
		
		#ifdef HX_ESD_WORKAROUND
		ESD_COUNTER = 0;
		#endif
		
		
		#ifdef ENABLE_CHIP_STATUS_MONITOR
		running_status = 0;
		queue_delayed_work(himax_wq, &himax_chip_monitor, 5*HZ);
		#endif
	
	return retval;
}


//Himax: Suspend Function 
static int tpd_suspend(struct i2c_client *client, pm_message_t message)
{
	int retval = TPD_OK;
	static char data[2];

		#ifdef ENABLE_CHIP_STATUS_MONITOR
	running_status = 1;
	cancel_delayed_work_sync(&himax_chip_monitor);
	#endif

 	tpd_halt = 1;
	TPD_DMESG("Himax TP: Suspend\n");
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	mutex_lock(&i2c_access);
#ifdef TPD_CLOSE_POWER_IN_SLEEP
       //hwPowerDown(TPD_POWER_SOURCE,"TP");

	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	hwPowerDown(MT65XX_POWER_LDO_VGP4,  "TP");
	hwPowerDown(MT65XX_POWER_LDO_VGP5, "TP_EINT");
#else
	//Himax: Sense Off
	i2c_smbus_write_i2c_block_data(i2c_client, 0x82, 0, &data[0]);
	msleep(120);

	//Himax: Sleep In
	i2c_smbus_write_i2c_block_data(i2c_client, 0x80, 0, &data[0]);
	msleep(120);

	//Himax: Deep Sleep In
	data[0] =0x01;
	i2c_smbus_write_i2c_block_data(i2c_client, 0xD7, 1, &data[0]);	
	msleep(100);
#endif
	mutex_unlock(&i2c_access);

/* #if ESD_WORKAROUND	
	first_pressed = 0;
#endi */
	
	return retval;
} 

//Himax: Touch Driver Structure
static struct tpd_driver_t tpd_device_driver = {
		 .tpd_device_name = "HIMAX_TS",
		 .tpd_local_init = tpd_local_init,
		 .suspend = tpd_suspend,
		 .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		 .tpd_have_button = 1,
#else
		 .tpd_have_button = 0,
#endif		
};

//Himax: Called when loaded into kernel
static int __init tpd_driver_init(void) 
{
	i2c_register_board_info(0, &himax_i2c_tpd, 1);
	TPD_DMESG("MediaTek HIMAX_TS touch panel driver init\n");
	
	if(tpd_driver_add(&tpd_device_driver) < 0)
		TPD_DMESG("add HIMAX_TS driver failed\n");
		
	return 0;
}
 
/* should never be called */
static void __exit tpd_driver_exit(void) 
{
	TPD_DMESG("MediaTek HIMAX_TS touch panel driver exit\n");
	//input_unregister_device(tpd->dev);
	tpd_driver_remove(&tpd_device_driver);
}
 
 module_init(tpd_driver_init);
 module_exit(tpd_driver_exit);

