#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>

#include "core/met_drv.h"
#include "core/trace.h"

#include "mt_typedefs.h"
#include "mt_reg_base.h"
#include "mt_emi_bm.h"
#include "sync_write.h"
#include "plf_trace.h"
#include "mci.h"

extern struct metdevice met_mci;
static struct kobject *kobj_mci = NULL;
static ssize_t evt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t evt_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n);
static struct kobj_attribute evt_attr = __ATTR(evt, 0644, evt_show, evt_store);

static ssize_t evt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int i=0;
	int j;
	for (j=0; j<MCI_DESC_COUNT; j++) {
		i += snprintf(buf+i, PAGE_SIZE-i, "0x%x: %s\n", mci_desc[j].event, mci_desc[j].name);
	}
	i += snprintf(buf+i, PAGE_SIZE-i, "\nCurrent counter0=0x%x, counter1=0x%x\n\n", 
		MET_MCI_GetEvent(0), MET_MCI_GetEvent(1));

	return i;
}

static ssize_t evt_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	unsigned int evt0, evt1;
	if (sscanf(buf, "%x %x", &evt0, &evt1) != 2)
		return -EINVAL;

	MET_MCI_Event_Set(evt0, evt1);
	//MCI_Event_Read();
	return n;
}

static void mci_value_init(void)
{
	/*select MCI monitor event*/
	MET_MCI_Event_Set(0x0, 0x1); /*0x2: AC_R snoop transaction counts; 0x4: AC_R snoop hit*/
}

static void mci_init(void)
{
}

static void mci_start(void)
{
	MET_MCI_Mon_Enable();
}

static void mci_stop(void)
{
	MET_MCI_Mon_Disable();
}

static int do_mci(void)
{
	return met_mci.mode;
}

static unsigned int mci_polling(unsigned int *value)
{
	int j = -1;

	value[++j] = MET_MCI_GetEventCount(0);
	value[++j] = MET_MCI_GetEventCount(1);
	//MCI_Counter_Reset();
	//MCI_Event_Read();
	MET_MCI_Mon_Disable();
	MET_MCI_Mon_Enable();
	return j+1;
}

static void mci_uninit(void)
{
}

static int met_mci_create(struct kobject *parent)
{
	int ret = 0;

	kobj_mci = parent;

	ret = sysfs_create_file(kobj_mci, &evt_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create evt in sysfs\n");
		return ret;
	}
	mci_value_init();
    return ret;
}

static void met_mci_delete(void)
{
	sysfs_remove_file(kobj_mci, &evt_attr.attr);
	kobj_mci = NULL;
}

static void met_mci_start(void)
{
	if (do_mci()) {
		mci_init();
		mci_stop();
		mci_start();
	}
}

static void met_mci_stop(void)
{
	if (do_mci()) {
		mci_stop();
		mci_uninit();
	}

}

static void met_mci_polling(unsigned long long stamp, int cpu)
{
	unsigned char count;
	unsigned int mci_value[2];

	if (do_mci()) {
		count = mci_polling(mci_value);
		if (count) {
			ms_mci(stamp, count, mci_value);
		}
	}
}

static char header[] =
"met-info [000] 0.0: ms_ud_sys_header: ms_mci,0x%x(%s),0x%x(%s),x,x\n";
//"met-info [000] 0.0: ms_ud_sys_header: ms_mci,mci_cnt0,mci_cnt1,x,x\n";
static char help[] = "  --mci                             monitor MCI\n";

static int mci_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help);
}

static int mci_print_header(char *buf, int len)
{
	unsigned int evt0, evt1;
	evt0 = MET_MCI_GetEvent(0);
	evt1 = MET_MCI_GetEvent(1);
	return snprintf(buf, PAGE_SIZE, header, evt0, mci_desc[evt0].name, evt1, mci_desc[evt1].name);
}

struct metdevice met_mci = {
	.name = "mci",
	.owner = THIS_MODULE,
	.type = MET_TYPE_BUS,
	.create_subfs = met_mci_create,
	.delete_subfs = met_mci_delete,
	.cpu_related = 0,
	.start = met_mci_start,
	.stop = met_mci_stop,
	.polling_interval = 0,//ms
	.timed_polling = met_mci_polling,
	.tagged_polling = met_mci_polling,
	.print_help = mci_print_help,
	.print_header = mci_print_header,
};

