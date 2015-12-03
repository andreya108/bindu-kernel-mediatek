#include <linux/kernel.h>
#include <linux/string.h>

#include <mach/mtk_rtc.h>
#include <mach/wd_api.h>
extern void wdt_arch_reset(char);
extern unsigned int pmic_config_interface (unsigned int RegNum, unsigned int val, unsigned int MASK, unsigned int SHIFT);/*lenovo-sw jixj 2013.5.15 add*/



void arch_reset(char mode, const char *cmd)
{
    char reboot = 0;
    int res=0;
    struct wd_api*wd_api = NULL;
    
    res = get_wd_api(&wd_api);
    printk("arch_reset: cmd = %s\n", cmd ? : "NULL");

    /*lenovo-sw jixj 2013.5.15 add begin*/
    #ifdef LENOVO_LONG_POWER_RESET
    pmic_config_interface(0x126, 0x0, 0x1, 0x2);
    #endif
    /*lenovo-sw jixj 2013.5.15 add end*/
    if (cmd && !strcmp(cmd, "charger")) {
        /* do nothing */
    } else if (cmd && !strcmp(cmd, "recovery")) {
        rtc_mark_recovery();
    } else if (cmd && !strcmp(cmd, "bootloader")){
    		rtc_mark_fast();	
    } 
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	else if (cmd && !strcmp(cmd, "kpoc")){
		rtc_mark_kpoc();
	}
#endif
    else {
    	reboot = 1;
    }

    if(res){
        printk("arch_reset, get wd api error %d\n",res);
    } else {
        wd_api->wd_sw_reset(reboot);
    }
}

