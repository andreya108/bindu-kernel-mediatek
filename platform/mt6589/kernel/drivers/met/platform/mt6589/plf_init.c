#include <linux/kernel.h>
#include <linux/module.h>

#include "core/met_drv.h"

static const char strTopology[] = "LITTLE:0,1,2,3";

extern struct metdevice met_emi;
extern struct metdevice met_smi;
extern struct metdevice met_thermal;
extern struct metdevice met_mci;
extern struct metdevice met_dramc;

static int __init met_plf_init(void)
{
	met_register(&met_emi);
	met_register(&met_smi);
	met_register(&met_thermal);
	met_register(&met_mci);
	met_register(&met_dramc);
	met_set_platform("mt6589", 1);
	met_set_topology(strTopology, 1);

	return 0;
}

static void __exit met_plf_exit(void)
{
	met_deregister(&met_emi);
	met_deregister(&met_smi);
	met_deregister(&met_thermal);
	met_deregister(&met_mci);
	met_deregister(&met_dramc);
	met_set_platform(NULL, 0);
	met_set_topology(NULL, 0);
}

module_init(met_plf_init);
module_exit(met_plf_exit);
MODULE_AUTHOR("DT_DM5");
MODULE_DESCRIPTION("MET_MT6589");
MODULE_LICENSE("GPL");
