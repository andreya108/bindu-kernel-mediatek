#ifndef _CUST_BAT_H_
#define _CUST_BAT_H_

typedef enum
{
	Cust_CC_1600MA = 0x0,
	Cust_CC_1500MA = 0x1,
	Cust_CC_1400MA = 0x2,
	Cust_CC_1300MA = 0x3,
	Cust_CC_1200MA = 0x4,
	Cust_CC_1100MA = 0x5,
	Cust_CC_1000MA = 0x6,
	Cust_CC_900MA  = 0x7,
	Cust_CC_800MA  = 0x8,
	Cust_CC_700MA  = 0x9,
	Cust_CC_650MA  = 0xA,
	Cust_CC_550MA  = 0xB,
	Cust_CC_450MA  = 0xC,
	Cust_CC_400MA  = 0xD,
	Cust_CC_200MA  = 0xE,
	Cust_CC_70MA   = 0xF,
	Cust_CC_0MA	   = 0xDD
}cust_charging_current_enum;

typedef struct{
	unsigned int BattVolt;
	unsigned int BattPercent;
}VBAT_TO_PERCENT;

#define LENOVO_PROJECT_PRADA

/* Battery Temperature Protection */
/*lenovo-sw weiweij added for high temp warning*/
//lenovo_sw liaohj add smartt_rom 2013-10-01
#if defined(LENOVO_PROJECT_SNOOPY)|| defined(LENOVO_PROJECT_SNOOPY_CU) || defined(LENOVO_PROJECT_SMARTT)|| defined(LENOVO_PROJECT_SNOOPYTD)
#define MAX_CHARGE_TEMPERATURE  58
#else
#define MAX_CHARGE_TEMPERATURE  50
#endif
/*lenovo-sw weiweij added for high temp warning*/
#define MIN_CHARGE_TEMPERATURE  0
#define ERR_CHARGE_TEMPERATURE  0xFF

/* Recharging Battery Voltage */
#ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
#define RECHARGING_VOLTAGE      4300
#else
#define RECHARGING_VOLTAGE      4110
#endif
/* Charging Current Setting */
#define CONFIG_USB_IF 						0
#define USB_CHARGER_CURRENT_SUSPEND			Cust_CC_0MA		// def CONFIG_USB_IF
#define USB_CHARGER_CURRENT_UNCONFIGURED	Cust_CC_70MA	// def CONFIG_USB_IF
#define USB_CHARGER_CURRENT_CONFIGURED		Cust_CC_450MA	// def CONFIG_USB_IF
#define USB_CHARGER_CURRENT					Cust_CC_450MA
#define AC_CHARGER_CURRENT					Cust_CC_650MA

/* Battery Meter Solution */
#define CONFIG_ADC_SOLUTION 	1

/* Battery Voltage and Percentage Mapping Table */
VBAT_TO_PERCENT Batt_VoltToPercent_Table[] = {
	/*BattVolt,BattPercent*/
/*    {3404,0}, // mod1
    {3450,1},
    {3520,2},
    {3536,4},
    {3552,6},
    {3568,8},
    {3584,10},
    {3600,12},
    {3616,14},
    {3632,16},
    {3648,18},
    {3664,20},
    {3680,22},
    {3696,24},
    {3712,26},
    {3728,28},
    {3744,30},
    {3760,32},
    {3776,34},
    {3792,36},
    {3808,38},
    {3824,40},
    {3840,42},
    {3856,44},
    {3872,46},
    {3888,48},
    {3904,50},
    {3920,52},
    {3936,54},
    {3952,56},
    {3968,58},
    {3984,60},
    {4000,62},
    {4016,64},
    {4032,66},
    {4048,68},
    {4064,70},
    {4080,72},
    {4096,74},
    {4112,76},
    {4128,78},
    {4144,80},
    {4160,82},
    {4176,84},
    {4192,86},
    {4208,88},
    {4224,90},
    {4240,92},
    {4256,94},
    {4272,96},
    {4288,98},
    {4304,99},
    {4335,100},*/
/*    {3400,0}, // original
	{3641,10},
	{3708,20},
	{3741,30},
	{3765,40},
	{3793,50},
	{3836,60},
	{3891,70},
	{3960,80},
	{4044,90},
	{4183,100},*/
    {3400,  0}, // mod2
    {3600,  5},
	{3660, 10},
    {3710, 15},
    {3730, 20},
    {3750, 25},
    {3770, 30},
    {3790, 35},
    {3805, 40},
    {3820, 45},
    {3840, 50},
    {3855, 55},
    {3870, 60},
    {3910, 65},
    {3950, 70},
    {3980, 75},
    {4020, 80},
    {4080, 85},
    {4110, 90},
    {4150, 95},
    {4250, 99},
    {4350,100},

};

/* Precise Tunning */
#define BATTERY_AVERAGE_SIZE 	10
//#define BATTERY_AVERAGE_SIZE   30

/* Common setting */
#define R_CURRENT_SENSE 2				// 0.2 Ohm
#define R_BAT_SENSE 4					// times of voltage
#define R_I_SENSE 4						// times of voltage
#define R_CHARGER_1 330
#define R_CHARGER_2 39
#define R_CHARGER_SENSE ((R_CHARGER_1+R_CHARGER_2)/R_CHARGER_2)	// times of voltage
#define V_CHARGER_MAX 6500				// 6.5 V
#define V_CHARGER_MIN 4400				// 4.4 V
#define V_CHARGER_ENABLE 0				// 1:ON , 0:OFF

/* Teperature related setting */
#define RBAT_PULL_UP_R             39000
#define RBAT_PULL_UP_VOLT          1800
//#define TBAT_OVER_CRITICAL_LOW     68237
//#define TBAT_OVER_CRITICAL_LOW     483954
#define TBAT_OVER_CRITICAL_LOW     67790
#define BAT_TEMP_PROTECT_ENABLE    1
#define BAT_NTC_10 0
#define BAT_NTC_47 0
#define BAT_NTC_CG103JF103F

/* Battery Notify */
#define BATTERY_NOTIFY_CASE_0001
#define BATTERY_NOTIFY_CASE_0002
//#define BATTERY_NOTIFY_CASE_0003
//#define BATTERY_NOTIFY_CASE_0004
//lenovo_sw liaohj open chrtimer detect 2013-10-01
#define BATTERY_NOTIFY_CASE_0005

//#define CONFIG_POWER_VERIFY

#endif /* _CUST_BAT_H_ */
