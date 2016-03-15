#ifndef __CUST_VIBRATOR_H__
#define __CUST_VIBRATOR_H__

#include <mach/mt_reg_base.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

/*----------------------------------------------------------------------------*/
struct vibrator_hw {
	int	vib_timer;
};
/*----------------------------------------------------------------------------*/
extern struct vibrator_hw *get_cust_vibrator_hw(void);
/*----------------------------------------------------------------------------*/
#endif
