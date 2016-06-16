#include <mach/mt_typedefs.h>

#define TEMPERATURE_T0                  110
#define TEMPERATURE_T1                  0
#define TEMPERATURE_T2                  25
#define TEMPERATURE_T3                  50
#define TEMPERATURE_T                   255 // This should be fixed, never change the value

#define FG_METER_RESISTANCE 	0

#define MAX_BOOTING_TIME_FGCURRENT	1*10 // 10s

#if (defined(CONFIG_POWER_EXT))
#define OCV_BOARD_COMPESATE	72 //mV 
#define R_FG_BOARD_BASE		1000
#define R_FG_BOARD_SLOPE	1000 //slope
#else
#define OCV_BOARD_COMPESATE	0 //mV 
#define R_FG_BOARD_BASE		1000
#define R_FG_BOARD_SLOPE	1000 //slope
#endif

#if defined(VREDNIIY_BAT_2)
#define Q_MAX_POS_50	2000
#define Q_MAX_POS_25	1900
#define Q_MAX_POS_0	1800
#define Q_MAX_NEG_10	1700

#define Q_MAX_POS_50_H_CURRENT	2100
#define Q_MAX_POS_25_H_CURRENT	2000
#define Q_MAX_POS_0_H_CURRENT	1850
#define Q_MAX_NEG_10_H_CURRENT	1700
#else
#define Q_MAX_POS_50	2750
#define Q_MAX_POS_25	2690
#define Q_MAX_POS_0	2550
#define Q_MAX_NEG_10	2400

#define Q_MAX_POS_50_H_CURRENT	2850
#define Q_MAX_POS_25_H_CURRENT	2640
#define Q_MAX_POS_0_H_CURRENT	2450
#define Q_MAX_NEG_10_H_CURRENT	2300
#endif

#define R_FG_VALUE 				20 // mOhm, base is 20
#define CURRENT_DETECT_R_FG	10  //1mA

#define OSR_SELECT_7			0

#define CAR_TUNE_VALUE			100 //1.00

/////////////////////////////////////////////////////////////////////
// <DOD, Battery_Voltage> Table
/////////////////////////////////////////////////////////////////////
typedef struct _BATTERY_PROFILE_STRUC
{
    kal_int32 percentage;
    kal_int32 voltage;
} BATTERY_PROFILE_STRUC, *BATTERY_PROFILE_STRUC_P;

typedef enum
{
    T1_0C,
    T2_25C,
    T3_50C
} PROFILE_TEMPERATURE;

#if (defined(HIGH_BATTERY_VOLTAGE_SUPPORT))
#include "high_voltage.h"
#else
#include "normal_voltage.h"
#endif

int fgauge_get_saddles(void);
BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature);

int fgauge_get_saddles_r_table(void);
R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature);
