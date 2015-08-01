#ifndef __DDP_DEBUG_H__
#define __DDP_DEBUG_H__

#include <linux/kernel.h>
#include <linux/mmprofile.h>
#include <linux/aee.h>


extern struct DDP_MMP_Events_t
{
    MMP_Event DDP;
    MMP_Event MutexParent;
    MMP_Event Mutex[6];
    MMP_Event BackupReg;
    MMP_Event DDP_IRQ;
    MMP_Event SCL_IRQ;
    MMP_Event ROT_IRQ;
    MMP_Event OVL_IRQ;
    MMP_Event WDMA0_IRQ;
    MMP_Event WDMA1_IRQ;
    MMP_Event RDMA0_IRQ;
    MMP_Event RDMA1_IRQ;
    MMP_Event COLOR_IRQ;
    MMP_Event BLS_IRQ;
    MMP_Event TDSHP_IRQ;
    MMP_Event CMDQ_IRQ;
    MMP_Event Mutex_IRQ;
    MMP_Event WAIT_INTR;
    MMP_Event Debug;
} DDP_MMP_Events;
//#define DISP_MSG(...)   xlog_printk(ANDROID_LOG_DEBUG, "xlog/disp", __VA_ARGS__)
//#define DISP_DBG(...)   xlog_printk(ANDROID_LOG_WARN,  "xlog/disp", __VA_ARGS__)
//#define DISP_ERR(...)   xlog_printk(ANDROID_LOG_ERROR, "xlog/disp", __VA_ARGS__)

extern unsigned int dbg_log;
extern unsigned int irq_log;

#define DISP_IRQ(string, args...) if(irq_log) printk("[DDP]"string,##args)  // default off
#define DISP_DBG(string, args...) if(dbg_log) printk("[DDP]"string,##args)  // default off, use "adb shell "echo dbg_log:1 > sys/kernel/debug/dispsys" to enable
#define DISP_MSG(string, args...) printk("[DDP]"string,##args)  // default on, important msg, not err
#define DISP_ERR(string, args...) printk("[DDP]error:"string,##args)  //default on, err msg

#define ddp_aee_mmp_dump(string, args...) do{\
    MMProfileLogEx(DDP_MMP_Events.Mutex[5], MMProfileFlagPulse, 1, 1); \
    DISP_MSG("DDP dump MMP start at file=%s, line=%d, time=%s", __FILE__, __LINE__, __TIME__); \
    aee_kernel_warning_api(__FILE__, __LINE__, DB_OPT_MMPROFILE_BUFFER, "DDP", "[DDP]"string, ##args);  \
}while(0)

void ddp_debug_init(void);
void ddp_debug_exit(void);
int ddp_mem_test(void);
int ddp_mem_test2(void);


#endif /* __DDP_DEBUG_H__ */
