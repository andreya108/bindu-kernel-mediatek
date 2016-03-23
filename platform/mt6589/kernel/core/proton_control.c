/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/*********************
* GPU Frequency List
**********************/
#define GPU_DVFS_F1     (476666)   // KHz
#define GPU_DVFS_F2     (403000)   // KHz
#define GPU_DVFS_F3     (357500)   // KHz
#define GPU_DVFS_F4     (312000)   // KHz
#define GPU_DVFS_F5     (286000)   // KHz
#define GPU_DVFS_F6     (268666)   // KHz
#define GPU_DVFS_F7     (238333)   // KHz
#define GPU_DVFS_F8     (156000)   // KHz

#define TBLTYPE0        0x0
#define TBLTYPE1        0x1
#define TBLTYPE2        0x2
#define TBLTYPE3        0x3

#define TBLTYPE0_0      0x4
#define TBLTYPE0_1      0x5
#define TBLTYPE0_2      0x6
#define TBLTYPE0_3      0x7

/******************************
* MFG Power Voltage Selection
*******************************/
#define GPU_POWER_VCORE_1_05V   (64)
#define GPU_POWER_VRF18_1_05V   (0)
#define GPU_POWER_VRF18_1_075V  (1)
#define GPU_POWER_VRF18_1_10V   (2)
#define GPU_POWER_VRF18_1_125V  (3)
#define GPU_POWER_VRF18_1_15V   (4)
#define GPU_POWER_VRF18_1_175V  (5)
#define GPU_POWER_VRF18_1_20V   (6)


/*********************************************************************
 * VARIABLES
 ********************************************************************/

// GPU frequency control in MHz
int proton_gpu_frequency = 286;
module_param(proton_gpu_frequency, int, 0664);
MODULE_PARM_DESC(proton_gpu_frequency, "Sets the GPU frequency");
EXPORT_SYMBOL(proton_gpu_frequency);

// GPU voltage control in mV
int proton_gpu_voltage[3] = {1050, 1050, 1050};
module_param_array(proton_gpu_voltage, int, NULL, 0664);
MODULE_PARM_DESC(proton_gpu_voltage, "Sets the GPU voltage");
EXPORT_SYMBOL(proton_gpu_voltage);

// GPU DVFS switch (1: enable, 0: disable)
bool proton_gpu_dvfs = 0;
module_param(proton_gpu_dvfs, bool, 0664);
MODULE_PARM_DESC(proton_gpu_dvfs, "Enables or disabled GPU DVFS (Dynamic voltage and frequency)");
EXPORT_SYMBOL(proton_gpu_dvfs);

/*********************************************************************
 * FUNCTION DEFINITIONS
 ********************************************************************/
 
unsigned int proton_gpu_frequency_get(void) {
	
	unsigned int freq;
	
	/* 
	 * Select GPU frequency (in MHz)
	 * Values defined in mt_gpufreq.h
	 */
	switch (proton_gpu_frequency)
	{
		case 476:
			freq = GPU_DVFS_F1;
	    	break;
	    case 403:
			freq = GPU_DVFS_F2;
	    	break;
	    case 357:
			freq = GPU_DVFS_F3;
	    	break;
	    case 312:
			freq = GPU_DVFS_F4;
	    	break;
	    default:
	    case 286:
			freq = GPU_DVFS_F5;
	    	break;
	    case 268:
			freq = GPU_DVFS_F6;
			break;
		case 238:
			freq = GPU_DVFS_F7;
			break;
		case 156:
			freq = GPU_DVFS_F8;
			break;
	}
	
	return freq;
}
EXPORT_SYMBOL_GPL(proton_gpu_frequency_get);

unsigned int proton_gpu_tbltype_get(void) {
	
	unsigned int tbltype;
	
	/* 
	 * Select GPU tbltype
	 * Values defined in mtk_sysfreq.h
	 */
	switch (proton_gpu_frequency)
	{
		case 476:
		tbltype = TBLTYPE3;
			break;
		case 403:
		tbltype = TBLTYPE2;
			break;
		case 357:
		tbltype = TBLTYPE1;
			break;
		case 312:
		tbltype = TBLTYPE0_3;
			break;
		case 286:
		tbltype = TBLTYPE0_2;
			break;
		case 268:
		tbltype = TBLTYPE0_1;
			break;
		case 238:
		tbltype = TBLTYPE0_0;
			break;
		default:
		tbltype = TBLTYPE0;
			break;
	}
	
	return tbltype;
}
EXPORT_SYMBOL_GPL(proton_gpu_tbltype_get);

unsigned int proton_gpu_voltage_get(int num) {

	unsigned int voltage;

	// Select GPU voltage (in mV)
	switch (proton_gpu_voltage[num])
	{
		case 1200:
			voltage = GPU_POWER_VRF18_1_20V;
			break;
		case 1175:
			voltage = GPU_POWER_VRF18_1_175V;
			break;
		case 1150:
			voltage = GPU_POWER_VRF18_1_15V;
			break;
		case 1125:
			voltage = GPU_POWER_VRF18_1_125V;
			break;
		case 1100:
			voltage = GPU_POWER_VRF18_1_10V;
			break;
		case 1075:
			voltage = GPU_POWER_VRF18_1_075V;
			break;
		default:
		case 1050:
			voltage = GPU_POWER_VRF18_1_05V;
			break;
	}
	
	return voltage;
}
EXPORT_SYMBOL_GPL(proton_gpu_voltage_get);

static int __init proton_control_init(void)
{
	return 0;
}

arch_initcall(proton_control_init);

MODULE_AUTHOR("Ivan Rusanov <ivan.b.rusanov@gmail.com");
MODULE_DESCRIPTION("PROTON GPU Init Driver");
MODULE_LICENSE("GPL");
