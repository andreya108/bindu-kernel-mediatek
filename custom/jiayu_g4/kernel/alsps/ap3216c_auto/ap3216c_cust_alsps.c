#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
  .i2c_num    = 3,
  .polling_mode_ps =0,
  .polling_mode_als =1,
  .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
  .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
  .i2c_addr   = {0x3C, 0x38, 0x3A, 0x00}, //
  //.als_level  = { 0,  1,  5,   7,  15,  40,  100, 1000, 2000,  3000,  6000, 10000, 14000, 18000, 20000},
  //.als_value  = {25, 35, 45,  100, 160, 180,  225,  320,  640,  1280,  2000,  2600,  5000, 9000,  10240},
  .als_level  = { 2, 64,  128, 192, 256, 320,  384, 448, 512,  576, 640, 704, 768, 864, 1024},
  .als_value  = {40, 100, 200, 300, 400, 500,  600, 700, 800,  900, 1000, 1100, 1200, 1300,  1400, 1500},
  .ps_threshold_high = 600,
  .ps_threshold_low = 350,
};

struct alsps_hw *AP3216C_get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}