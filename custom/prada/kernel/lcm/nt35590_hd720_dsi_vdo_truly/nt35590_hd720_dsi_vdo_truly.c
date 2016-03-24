#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#include <asm/setup.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#if defined(MTK_WFD_SUPPORT)
#define   LCM_DSI_CMD_MODE							1
#else
#define   LCM_DSI_CMD_MODE							0
#endif

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID_NT35590 (0x90)

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

#define DEFAULT_NEWLCM 0
#define FBK_DIV_MAX 40
#define FBK_DIV_MIN 10
static unsigned int fbk_div = 0;
static unsigned int set_gamma = 1;

// ---------------------------------------------------------------------------
// //  Local Variables
// // ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#include "nt35590_hd720_dsi_vdo_truly_stocklcm.c.inc"
#include "nt35590_hd720_dsi_vdo_truly_newlcm.c.inc"

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util);

LCM_DRIVER nt35590_hd720_dsi_vdo_truly_lcm_drv =
{
    .name			= "nt35590_hd720_dsi_vdo_truly",
	.set_util_funcs = lcm_set_util_funcs,
#if DEFAULT_NEWLCM
	.get_params     = lcm_get_params_newlcm,
	.init           = lcm_init_newlcm,
	.suspend        = lcm_suspend_newlcm,
	.resume         = lcm_resume_newlcm,
	.compare_id     = lcm_compare_id_newlcm,
	.esd_check      = lcm_esd_check_newlcm,
	.esd_recover    = lcm_esd_recover_newlcm,
# if (LCM_DSI_CMD_MODE)
    .update         = lcm_update_newlcm,
# endif
#else
	.get_params     = lcm_get_params_stocklcm,
	.init           = lcm_init_stocklcm,
	.suspend        = lcm_suspend_stocklcm,
	.resume         = lcm_resume_stocklcm,
	.compare_id     = lcm_compare_id_stocklcm,
	.esd_check      = lcm_esd_check_stocklcm,
	.esd_recover    = lcm_esd_recover_stocklcm,
# if (LCM_DSI_CMD_MODE)
    .update         = lcm_update_stocklcm,
# endif
#endif
};

#ifndef BUILD_LK
static int __init fbk_div_setup(char *str)
{
    int tmp = fbk_div;
    printk("OPT %s: %s\n", __func__, str);
    get_option(&str, &tmp);
    printk("OPT %s: tmp=%d\n", __func__, tmp);
    if (tmp >= FBK_DIV_MIN && tmp <= FBK_DIV_MAX)
        fbk_div = tmp;
    return 0;
}
early_param("lcm.fbk_div",fbk_div_setup);

static int __init no_gamma_setup(char *str)
{
    printk("OPT %s: lcm.no_gamma\n", __func__);
    set_gamma = 0;
    return 0;
}
early_param("lcm.no_gamma",no_gamma_setup);

static int __init newlcm_setup(char *str)
{
    printk("OPT %s: newlcm\n", __func__);

	nt35590_hd720_dsi_vdo_truly_lcm_drv.get_params     = lcm_get_params_newlcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.init           = lcm_init_newlcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.suspend        = lcm_suspend_newlcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.resume         = lcm_resume_newlcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.compare_id     = lcm_compare_id_newlcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.esd_check      = lcm_esd_check_newlcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.esd_recover    = lcm_esd_recover_newlcm;
#if (LCM_DSI_CMD_MODE)
    nt35590_hd720_dsi_vdo_truly_lcm_drv.update         = lcm_update_newlcm;
#endif
    return 0;
}
early_param("newlcm",newlcm_setup);

static int __init stocklcm_setup(char *str)
{
    printk("OPT %s: stocklcm\n", __func__);

	nt35590_hd720_dsi_vdo_truly_lcm_drv.get_params     = lcm_get_params_stocklcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.init           = lcm_init_stocklcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.suspend        = lcm_suspend_stocklcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.resume         = lcm_resume_stocklcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.compare_id     = lcm_compare_id_stocklcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.esd_check      = lcm_esd_check_stocklcm;
	nt35590_hd720_dsi_vdo_truly_lcm_drv.esd_recover    = lcm_esd_recover_stocklcm;
#if (LCM_DSI_CMD_MODE)
    nt35590_hd720_dsi_vdo_truly_lcm_drv.update         = lcm_update_stocklcm;
#endif
    return 0;
}
early_param("stocklcm",stocklcm_setup);
#endif

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

