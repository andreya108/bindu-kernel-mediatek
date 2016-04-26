#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
#include <mach/upmu_common.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 3,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    //.i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
    .als_level  = { 6, 9, 14, 20, 40, 70, 100, 250, 650, 800,  1100, 1500, 2500, 4000, 6000},
    .als_value  = {40, 40, 90,  90, 160, 160,  225,  320,  640,  1280,  1280,  2600,  2600, 2600,  10240, 10240},
    .ps_threshold_high = 120,
    .ps_threshold_low = 100,
};
struct alsps_hw *get_cust_alsps_hw_apds9930(void) {
    return &cust_alsps_hw;
}

int APDS9930_CMM_PPCOUNT_VALUE = 0x08;
int APDS9930_CMM_CONTROL_VALUE = 0xE4;
int ZOOM_TIME = 4;

int pmic_ldo_suspend_enable(int enable)
{
	//0 for disable suspend, 1 for enable suspend
	//upmu_set_vio18_lp_sel(enable);
	//upmu_set_vio28_lp_sel(enable);
	//return 0;
}
