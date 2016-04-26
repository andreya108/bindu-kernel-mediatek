/*
 * This file is part of the AP3216C sensor driver for MTK platform.
 * AP3216C is combined proximity, ambient light sensor and IRLED.
 *
 * Contact: YC Hou <yc.hou@liteonsemi.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 *
 * Filename: cust_alsps.c
 *
 * Summary:
 *	AP3216C hardware defines.
 *
 * Modification History:
 * Date     By       Summary
 * -------- -------- -------------------------------------------------------
 * 05/11/12 YC		 Original Creation (Test version:1.0)
 * 06/04/12 YC		 Modify ps threshold field to change to high/low threshold.
 */

#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
  .i2c_num    = 3,
  .polling_mode_ps =0,
  .polling_mode_als =1,
  .power_id   = MT65XX_POWER_LDO_VGP5,    /*LDO is not used*/
  .power_vol  = VOL_2800,          /*LDO is not used*/
  .i2c_addr   = {0x3C, 0x38, 0x3A, 0x00}, //
  .als_level  = { 0,  1,  1,   7,  15,  15,  100, 1000, 2000,  3000,  6000, 10000, 14000, 18000, 20000},
  .als_value  = {40, 40, 90,  90, 160, 160,  225,  320,  640,  1280,  1280,  2600,  2600, 2600,  10240, 10240},
  .ps_threshold_high = 600,
  .ps_threshold_low = 350,
};

struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}


int C_CUST_ALS_FACTOR_AP3216C=500; 


