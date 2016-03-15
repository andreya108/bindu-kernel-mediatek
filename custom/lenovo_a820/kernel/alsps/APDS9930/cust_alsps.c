#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 3,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x72, 0x48, 0x78, 0x00},
    /* MTK: modified to support AAL */
    //.als_level  = { 6, 	9, 	 17,  38,  56,   74,   116,  342,  778,  1082, 1386, 1914,  3000,  5000,  8000 },
    //.als_value  = {136, 218, 312, 730, 1065, 1400, 2250, 4286, 5745, 7390, 9034, 11000, 10240, 10240, 10240, 10240},
    //.ps_threshold_high = 300,
    //.ps_threshold_low = 400,
    //.ps_threshold = 600,
    
    /* I.nfraR.ed: 02.08.2015 Extract values from stock S150 kernel */
    .als_level  = { 0, 	0, 	1,  5, 10,  16,  45,  62,  76, 103,  135,  168,  192,  840, 1603 },
    .als_value  = {10, 16, 25, 40, 63, 101, 160, 254, 403, 640, 1016, 1613, 2560, 4064, 6451, 10240},
    .ps_threshold_high = 900,
    .ps_threshold_low = 750,
    .ps_threshold = 900,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}
//int APDS9930_CMM_PPCOUNT_VALUE = 0x08;
//int APDS9930_CMM_CONTROL_VALUE = 0xE4;
/* I.nfraR.ed: 02.08.2015 Extract values from stock S150 kernel */
int APDS9930_CMM_PPCOUNT_VALUE = 0x06;
int APDS9930_CMM_CONTROL_VALUE = 0x68;
int ZOOM_TIME = 4;
