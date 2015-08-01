/*
 * (C) Copyright 2014
 * Lenovo <www.lenovo.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

#define PROC_DIR_NAME        "lenovo"
#define PROC_FILE_BOOSTER    "booster"

#define BOOSTER_OFF          0
#define BOOSTER_ON           1
// booster is not started by default
static int booster_status = BOOSTER_OFF;

static struct proc_dir_entry *proc_dir = NULL;
static struct proc_dir_entry *proc_file_booster = NULL;

static struct workqueue_struct *booster_workqueue = NULL;
static struct delayed_work boost_perf_work, restore_perf_work;

static int write_to_file(const void* data, int size, const char* path)
{
    struct file *fp = NULL;
    int ret = 0;
    mm_segment_t fs;

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(path, O_WRONLY | O_CREAT, 0);
    if (IS_ERR(fp)) {
        printk("booster: can not open(wr) file %s: %ld\n", path, PTR_ERR(fp));
        return -1;
    }

    if (fp->f_op->write(fp, data, size, &(fp->f_pos)) < 0) {
        ret = -1;
        printk("booster: can not write %s to %s\n", (char*)data, path);
    } else {
        printk("booster: write %s\n", (char*)data);
    }
    filp_close(fp, NULL);
    set_fs(fs);
    mdelay(20);

    return ret;
}

static int read_from_file(void* data, int size, const char* path)
{
    struct file *fp = NULL;
    int ret = 0;
    mm_segment_t fs;

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        printk("booster: can not open(rd) file %s: %ld\n", path, PTR_ERR(fp));
        return -1;
    }

    if (fp->f_op->read(fp, data, size, &(fp->f_pos)) < 0) {
        ret = -1;
        printk("booster: can not read %s to %s\n", (char*)data, path);
    } else {
        printk("booster: read %s\n", (char*)data);
    }
    filp_close(fp, NULL);
    set_fs(fs);

    return ret;
}

static int boost_perf_policy(void)
{
    // save old configuration before boosting
    //save_perf_policy();


    // change cpu policy to performance mode
    write_to_file("performance", 11, "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    write_to_file("0", 1, "/sys/module/mt_hotplug_mechanism/parameters/g_enable");
    write_to_file("1", 1, "/sys/devices/system/cpu/cpu1/online");
    write_to_file("1", 1, "/sys/devices/system/cpu/cpu2/online");
    write_to_file("1", 1, "/sys/devices/system/cpu/cpu3/online");

    // GPU set to 350Mhz, MUST ask MTK for more details if other product wants to tune GPU loading
    write_to_file("4", 1, "/proc/gpufreq/gpufreq_set_voltage_write");
    write_to_file("357500", 6, "/proc/gpufreq/gpufreq_set_frequency_write");

    return 0;
}

static void restore_perf_policy(void)
{
    write_to_file("hotplug", 7, "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    write_to_file("1", 1, "/sys/module/mt_hotplug_mechanism/parameters/g_enable");

    write_to_file("0", 1, "/proc/gpufreq/gpufreq_set_voltage_write");
    write_to_file("286000", 6, "/proc/gpufreq/gpufreq_set_frequency_write");
}

static int proc_read_booster(char* page, char** start, off_t offset, int count, int* eof, void* data)
{
    // we are always return only one byte data
    *page = '0';
    if (booster_status == BOOSTER_ON) {
        *page = '1';
    }

    return 1;
}

static int proc_write_booster(struct file* file, const char* buffer, unsigned long count, void* data)
{
    char buf[16] = {0};
    unsigned int ret;
    int len = 0;
    int i = 0;

    len = count;
    if (count > 15)
        len = 15;
    if (copy_from_user((void*)buf, buffer, len)) {
        printk(KERN_INFO "booster: copy from user error.\n");
        return -EFAULT;
    }

    for (i = 0; i < len; i++)
        printk("booster: %02x\n", (unsigned int)buf[i]);

    if (booster_status == BOOSTER_ON && buf[0] == '0') {
        queue_delayed_work(booster_workqueue, &restore_perf_work, 0);
    } else if (booster_status == BOOSTER_OFF && buf[0] == '1') {
        queue_delayed_work(booster_workqueue, &boost_perf_work, 0);
    } else {
        // this isn't the scenario we wanted
        printk(KERN_ERR "booster: invalid parameter\n");
    }

    return (int)count;
}

static void switch_to_performance(struct work_struct* work)
{
    printk(KERN_INFO "booster: boosting\n");
    boost_perf_policy();
    booster_status = BOOSTER_ON;
}

static void switch_to_hotplug(struct work_struct* work)
{
    printk(KERN_INFO "booster: restoring\n");
    restore_perf_policy();
    booster_status = BOOSTER_OFF;
}

/*
 *  booster_init
 */
static int __init booster_init(void)
{
    printk(KERN_INFO "booster init ...\n");
    booster_workqueue = create_workqueue("boosterwq");
    if (!booster_workqueue) {
        printk(KERN_ERR "booster: can not create workqueue.\n");
        return -ENOMEM;
    }

    INIT_DELAYED_WORK(&boost_perf_work, switch_to_performance);
    INIT_DELAYED_WORK(&restore_perf_work, switch_to_hotplug);

    proc_dir = proc_mkdir(PROC_DIR_NAME, NULL);
    if (proc_dir == NULL) {
        printk(KERN_ERR "booster: can not create proc dir\n");
        return -ENOMEM;
    }

    proc_file_booster = create_proc_entry(PROC_FILE_BOOSTER, S_IRUGO | S_IWUGO, proc_dir);
    if (proc_file_booster == NULL) {
        printk(KERN_ERR "booster: can not create proc file\n");
        remove_proc_entry(PROC_DIR_NAME, NULL);
        return -ENOMEM;
    }

    proc_file_booster->read_proc = proc_read_booster;
    proc_file_booster->write_proc = proc_write_booster;
    proc_file_booster->mode = S_IFREG | S_IRUGO | S_IWUGO;
    proc_file_booster->uid = 0;
    proc_file_booster->gid = 0;
    proc_file_booster->size = 1;

    printk(KERN_INFO "booster init done!\n");
    return 0;
}

static void __exit booster_exit(void)
{
    if (proc_file_booster) {
        remove_proc_entry(PROC_FILE_BOOSTER, NULL);
        remove_proc_entry(PROC_DIR_NAME, NULL);
        proc_file_booster = NULL;
    }
}

module_init(booster_init);
module_exit(booster_exit);
MODULE_AUTHOR("Lenovo");
MODULE_DESCRIPTION("Lenovo Booster");
MODULE_LICENSE("GPL");

