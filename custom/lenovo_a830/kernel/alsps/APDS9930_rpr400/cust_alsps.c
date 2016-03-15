#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
//#include <mach/mt6577_pm_ldo.h>

static struct alsps_hw cust_alsps_hw_apds9930 = {
    .i2c_num    = 3,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x72, 0x48, 0x78, 0x00},
    /*Lenovo-sw chenlj2 add 2011-06-03,modify parameter below two lines*/
    //.als_level  = { 4, 40,  80,   120,   160, 250,  400, 800, 1200,  1600, 2000, 3000, 5000, 10000, 65535},
    //.als_value  = {10, 20,20,  120, 120, 280,  280,  280, 1600,  1600,  1600,  6000,  6000, 9000,  10240, 10240},
    //.als_level  = { 5, 10,  25,   50,  100, 150,  200, 400, 1000,  1500, 2000, 3000, 5000, 8000, 10000},
    //.als_value  = {10, 50,  100,  150, 200, 250,  280,  280, 1600,  1600,  1600,  6000,  6000, 9000,  10240, 10240},
    /* MTK: modified to support AAL */
    /* LENOVO.SW begin chenyb1 2013.2.27 add new value */
    .als_level  = {9,  33,   519,   1620, 5245, 8267, 9633},// no hardware
    .als_value  = {20, 90,350,  1100, 3500, 5600, 6700, 8700},
    /* LENOVO.SW end chenyb1 2013.2.27 add new value */
    .ps_threshold_high = 900,
    .ps_threshold_low = 750,
    .ps_threshold = 900,
};
static struct alsps_hw cust_alsps_hw_rpr400 = {
    .i2c_num    = 3,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x72, 0x48, 0x78, 0x00},
    /*Lenovo-sw chenlj2 add 2011-06-03,modify parameter below two lines*/
    /*lenovo-sw jixj 2011.12.22 modified being*/
    /*
    .als_level  = { 4, 40,  80,   120,   160, 250,  400, 800, 1200,  1600, 2000, 3000, 5000, 10000, 65535},
    .als_value  = {10, 20,20,  120, 120, 280,  280,  280, 1600,  1600,  1600,  6000,  6000, 9000,  10240, 10240},
    */
    //.als_level  = {1,  2,   4,   8, 12,  20, 37, 48,  62, 180, 352, 400, 800, 1300, 2900},
    //.als_value  = {10, 16,25,  40, 63, 101,  160,  254, 403,  640,  1016,  1613,  2560, 4064,  6451, 10240},
    /* LENOVO.SW begin chenyb1 2013.2.27 add new value */
    .als_level  = {9,  33,   519,   1620, 5245, 8267, 9633},
    .als_value  = {20, 90,350,  1100, 3500, 5600, 6700, 8700},
    /* LENOVO.SW end chenyb1 2013.2.27 add new value */
    /*lenovo-sw jixj 2011.12.22 modified end*/
    /*Lenovo-sw chenlj2 modified 2011-08-09*/
    .ps_threshold_high = 100,
    .ps_threshold_low = 50,
    .ps_threshold = 900,
};
int alsps_device_index = 0;// 0 unknow 1 avago 2 rohm
struct alsps_hw *get_cust_alsps_hw(void) {
    if(alsps_device_index ==1)
    return &cust_alsps_hw_apds9930;
    else if(alsps_device_index ==2)
    return &cust_alsps_hw_rpr400;
    else
    return &cust_alsps_hw_apds9930;
}
int APDS9930_CMM_PPCOUNT_VALUE = 0x06;
int APDS9930_CMM_CONTROL_VALUE = 0x68;
int ZOOM_TIME = 4;
int TMD2771_CMM_PPCOUNT_VALUE = 0x06;
int TMD2771_CMM_CONTROL_VALUE = 0xE0;
