#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/io.h>
#include <mach/mt_reg_base.h>
#include <mach/etm.h>

static struct etm_driver_data etm_driver_data[4] =
{
	{
		.etm_regs = IOMEM(DEBUGTOP_BASE + 0x7C000),
		.is_ptm = 0,
	},
	{
		.etm_regs = IOMEM(DEBUGTOP_BASE + 0x7D000),
		.is_ptm = 0,
	},
	{
		.etm_regs = IOMEM(DEBUGTOP_BASE + 0x7E000),
		.is_ptm = 0,
	},
	{
		.etm_regs = IOMEM(DEBUGTOP_BASE + 0x7F000),
		.is_ptm = 0,
	},
};

static struct platform_device etm_device[4] = 
{
	{
		.name = "etm",
		.id = 0,
		.dev = 
		{
			.platform_data = &(etm_driver_data[0]),
		},
	},
	{
		.name = "etm",
		.id = 1,
		.dev = 
		{
			.platform_data = &(etm_driver_data[1]),
		},
	},
	{
		.name = "etm",
		.id = 2,
		.dev = 
		{
			.platform_data = &(etm_driver_data[2]),
		},
	},
	{
		.name = "etm",
		.id = 3,
		.dev = 
		{
			.platform_data = &(etm_driver_data[3]),
		},
	},
};

static struct etb_driver_data etb_driver_data =
{
	.etb_regs = IOMEM(DEBUGTOP_BASE + 0x13000),
	.funnel_regs = IOMEM(DEBUGTOP_BASE + 0x14000),
	.tpiu_regs = IOMEM(DEBUGTOP_BASE + 0x13000),
	.dem_regs = IOMEM(DEBUGTOP_BASE + 0x1A000),
	.etr_virt = INTER_SRAM + 0xF800,
	.etr_phys = 0x00100000 + 0xF800,
	.etr_len = 0x200,
	.use_etr = 1,
};

static struct platform_device etb_device =
{
	.name = "etb",
	.id = -1,
	.dev =
	{
		.platform_data = &etb_driver_data,
	},
};

void trace_start_dormant(void)
{
	trace_start_by_cpus(cpu_all_mask, 1);
}

void trace_stop_dormant(void)
{
}

void trace_start_unlock(void)
{
    trace_start_by_cpus(cpu_all_mask, 1);
}

void trace_stop_unlock(void)
{
}


static int __init pftracer_init(void)
{
	int err;

	err = platform_device_register(&(etm_device[0]));
	if (err) {
		pr_err("Fail to register etm_device 0");
		return err;
	}
	err = platform_device_register(&(etm_device[1]));
	if (err) {
		pr_err("Fail to register etm_device 1");
		return err;
	}
	err = platform_device_register(&(etm_device[2]));
	if (err) {
		pr_err("Fail to register etm_device 2");
		return err;
	}
	err = platform_device_register(&(etm_device[3]));
	if (err) {
		pr_err("Fail to register etm_device 3");
		return err;
	}

	err = platform_device_register(&etb_device);
	if (err) {
		pr_err("Fail to register etb_device");
		return err;
	}

	return 0;
}

arch_initcall(pftracer_init);
