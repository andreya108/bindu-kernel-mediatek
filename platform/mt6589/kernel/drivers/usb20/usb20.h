#ifndef __USB20_H__
#define __USB20_H__

#define DEVICE_INTTERRUPT 1
#define EINT_CHR_DET_NUM 23

#ifdef CONFIG_USB_MTK_OTG
#ifdef ID_PIN_USE_EX_EINT
#define ID_PIN_EINT 1
#define ID_PIN_GPIO GPIO112
#define GPIO_ID_PIN_EINT_PIN_M_EINT GPIO_MODE_05
#else
#define ID_PIN_EINT 28
#define U2PHYDTM1  (USB_SIF_BASE+0x800 + 0x6c)
#define ID_PULL_UP 0x0101
#define ID_PHY_RESET 0x3d11
#endif
#endif

#if defined(MTK_FAN5405_SUPPORT) \
    || defined(MTK_BQ24158_SUPPORT) \
    || defined(MTK_NCP1851_SUPPORT) \
    || defined(MTK_BQ24196_SUPPORT)
#define SWITCH_CHARGER 1
#endif

#if defined (CONFIG_MTK_FPGA) \
    || defined(CONFIG_MT6589_FPGA) \
    || defined(CONFIG_MT6582_FPGA)
#define FPGA_PLATFORM 1
#endif

struct mt_usb_glue {
	struct device		*dev;
	struct platform_device	*musb;
};

/* Battery relative fucntion */
typedef enum {
    CHARGER_UNKNOWN = 0,
    STANDARD_HOST,          // USB : 450mA
    CHARGING_HOST,
    NONSTANDARD_CHARGER,    // AC : 450mA~1A
    STANDARD_CHARGER,       // AC : ~1A
} CHARGER_TYPE;

extern void wake_up_bat(void);
extern CHARGER_TYPE mt_charger_type_detection(void);
extern bool upmu_is_chr_det(void);
extern void BATTERY_SetUSBState(int usb_state);
extern void upmu_interrupt_chrdet_int_en(kal_uint32 val);

/* specific USB fuctnion */
typedef enum
{
    CABLE_MODE_CHRG_ONLY = 0,
    CABLE_MODE_NORMAL,
	CABLE_MODE_HOST_ONLY,
    CABLE_MODE_MAX
} CABLE_MODE;

/* switch charger API*/
#ifdef MTK_FAN5405_SUPPORT
extern void fan5405_set_opa_mode(kal_uint32 val);
extern void fan5405_set_otg_pl(kal_uint32 val);
extern void fan5405_set_otg_en(kal_uint32 val);
extern kal_uint32 fan5405_config_interface_liao (kal_uint8 RegNum, kal_uint8 val);

#elif defined(MTK_BQ24158_SUPPORT)
extern void bq24158_set_opa_mode(kal_uint32 val);
extern void bq24158_set_otg_pl(kal_uint32 val);
extern void bq24158_set_otg_en(kal_uint32 val);
extern kal_uint32 bq24158_config_interface_reg (kal_uint8 RegNum, kal_uint8 val);

#elif defined(MTK_NCP1851_SUPPORT) || defined(MTK_BQ24196_SUPPORT)
extern void tbl_charger_otg_vbus(kal_uint32 mode);
#endif

#endif
