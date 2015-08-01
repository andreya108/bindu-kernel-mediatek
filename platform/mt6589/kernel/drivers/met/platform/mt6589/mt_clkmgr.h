#ifndef _MT_CLKMGR_H
#define _MT_CLKMGR_H

#include <linux/list.h>
#include "mt_reg_base.h"
#include "mt_typedefs.h"

#define APMIXED_BASE    0xF0209000
#define AP_PLL_CON0     (APMIXED_BASE + 0x0000)
#define AP_PLL_CON1     (APMIXED_BASE + 0x0004)
#define AP_PLL_CON2     (APMIXED_BASE + 0x0008)
#define AP_PLL_CON3     (APMIXED_BASE + 0x000C)

#define PLL_HP_CON0     (APMIXED_BASE + 0x0014)

#define ARMPLL_CON0         (APMIXED_BASE + 0x0200)
#define ARMPLL_CON1         (APMIXED_BASE + 0x0204)
#define ARMPLL_CON2         (APMIXED_BASE + 0x0208)
#define ARMPLL_PWR_CON0     (APMIXED_BASE + 0x0218)

#define MAINPLL_CON0         (APMIXED_BASE + 0x021C)
#define MAINPLL_CON1         (APMIXED_BASE + 0x0220)
#define MAINPLL_CON2         (APMIXED_BASE + 0x0224)
#define MAINPLL_PWR_CON0     (APMIXED_BASE + 0x0234)

#define UNIVPLL_CON0        (APMIXED_BASE + 0x0238)
#define MMPLL_CON0        (APMIXED_BASE + 0x0240)
#define ISPPLL_CON0        (APMIXED_BASE + 0x0248)


#define MSDCPLL_CON0         (APMIXED_BASE + 0x0250)
#define MSDCPLL_CON1         (APMIXED_BASE + 0x0254)
#define MSDCPLL_CON2         (APMIXED_BASE + 0x0258)
#define MSDCPLL_PWR_CON0     (APMIXED_BASE + 0x0268)


#define TVDPLL_CON0         (APMIXED_BASE + 0x026C)
#define TVDPLL_CON1         (APMIXED_BASE + 0x0270)
#define TVDPLL_CON2         (APMIXED_BASE + 0x0274)
#define TVDPLL_CON3         (APMIXED_BASE + 0x0278)
#define TVDPLL_PWR_CON0     (APMIXED_BASE + 0x0284)

#define LVDSPLL_CON0         (APMIXED_BASE + 0x0288)
#define LVDSPLL_CON1         (APMIXED_BASE + 0x028C)
#define LVDSPLL_CON2         (APMIXED_BASE + 0x0290)
#define LVDSPLL_CON3         (APMIXED_BASE + 0x0294)
#define LVDSPLL_PWR_CON0     (APMIXED_BASE + 0x02A0)


#define TOPCK_PDN_SET     (TOPRGU_BASE + 0x0170)
#define TOPCK_PDN_CLR     (TOPRGU_BASE + 0x0174)
#define TOPCK_PDN_STA     (TOPRGU_BASE + 0x0178)

#define INFRA_PDN_SET   (INFRACFG_BASE + 0x0040)
#define INFRA_PDN_CLR   (INFRACFG_BASE + 0x0044)
#define INFRA_PDN_STA   (INFRACFG_BASE + 0x0048)

#define PERI_PDN0_SET   (PERICFG_BASE + 0x0008)
#define PERI_PDN0_CLR   (PERICFG_BASE + 0x0010)
#define PERI_PDN0_STA   (PERICFG_BASE + 0x0018)

#define PERI_PDN1_SET   (PERICFG_BASE + 0x000C)
#define PERI_PDN1_CLR   (PERICFG_BASE + 0x0014)
#define PERI_PDN1_STA   (PERICFG_BASE + 0x001C)

#define PERI_PDN_MD_MASK    (PERICFG_BASE + 0x0038)


#define AUDIO_TOP_CON0  (0xF2070000)

#define MFG_CG_CON      (0xF0206000)
#define MFG_CG_SET      (0xF0206004)
#define MFG_CG_CLR      (0xF0206008)

#define DISP_CG_CON0    (DISPSYS_BASE + 0x0100)
#define DISP_CG_SET0    (DISPSYS_BASE + 0x0104)
#define DISP_CG_CLR0    (DISPSYS_BASE + 0x0108)

#define DISP_CG_CON1    (DISPSYS_BASE + 0x0110)
#define DISP_CG_SET1    (DISPSYS_BASE + 0x0114)
#define DISP_CG_CLR1    (DISPSYS_BASE + 0x0118)

#define IMG_CG_CON      (IMGSYS_CONFG_BASE + 0x0000)
#define IMG_CG_SET      (IMGSYS_CONFG_BASE + 0x0004)
#define IMG_CG_CLR      (IMGSYS_CONFG_BASE + 0x0008)

#define VDEC_CKEN_SET   (VDEC_GCON_BASE + 0x0000)
#define VDEC_CKEN_CLR   (VDEC_GCON_BASE + 0x0004)

#define LARB_CKEN_SET   (VDEC_GCON_BASE + 0x0008)
#define LARB_CKEN_CLR   (VDEC_GCON_BASE + 0x000C)

#define VENCSYS_CG_CON      (VENC_TOP_BASE + 0x0000)
#define VENCSYS_CG_SET      (VENC_TOP_BASE + 0x0004)
#define VENCSYS_CG_CLR      (VENC_TOP_BASE + 0x0008)

enum {
    CG_PERI0 = 0,
    CG_PERI1 = 1,
    CG_INFRA = 2,
    CG_TOPCK = 3,
    CG_DISP0 = 4,
    CG_DISP1 = 5,
    CG_IMAGE = 6,
    CG_MFG   = 7,
    CG_AUDIO = 8,
    CG_VDEC0 = 9,
    CG_VDEC1 = 10,
    CG_VENC  = 11,
    NR_GRPS  = 12,
};

enum {
    MT_CG_PERI0_NFI = 0,
    MT_CG_PERI0_THERM = 1,
    MT_CG_PERI0_PWM1 = 2,
    MT_CG_PERI0_PWM2 = 3,
    MT_CG_PERI0_PWM3 = 4,
    MT_CG_PERI0_PWM4 = 5,
    MT_CG_PERI0_PWM5 = 6,
    MT_CG_PERI0_PWM6 = 7,
    MT_CG_PERI0_PWM7 = 8,
    MT_CG_PERI0_PWM = 9,
    MT_CG_PERI0_USB0 = 10,
    MT_CG_PERI0_USB1 = 11,
    MT_CG_PERI0_APDMA = 12,
    MT_CG_PERI0_MSDC0 = 13,
    MT_CG_PERI0_MSDC1 = 14,
    MT_CG_PERI0_MSDC2 = 15,
    MT_CG_PERI0_MSDC3 = 16,
    MT_CG_PERI0_MSDC4 = 17,
    MT_CG_PERI0_APHIF = 18,
    MT_CG_PERI0_MDHIF = 19,
    MT_CG_PERI0_NLI = 20,
    MT_CG_PERI0_IRDA = 21,
    MT_CG_PERI0_UART0 = 22,
    MT_CG_PERI0_UART1 = 23,
    MT_CG_PERI0_UART2 = 24,
    MT_CG_PERI0_UART3 = 25,
    MT_CG_PERI0_I2C0 = 26,
    MT_CG_PERI0_I2C1 = 27,
    MT_CG_PERI0_I2C2 = 28,
    MT_CG_PERI0_I2C3 = 29,
    MT_CG_PERI0_I2C4 = 30,
    MT_CG_PERI0_I2C5 = 31,

    MT_CG_PERI1_I2C6                = 32,
    MT_CG_PERI1_WRAP                = 33,
    MT_CG_PERI1_AUXADC              = 34,
    MT_CG_PERI1_SPI1                = 35,
    MT_CG_PERI1_PHCTL               = 36,

    MT_CG_INFRA_DBGCLK              = 64,
    MT_CG_INFRA_SMI                 = 65,
    MT_CG_INFRA_SPI0                = 66,
    MT_CG_INFRA_AUDIO               = 69,
    MT_CG_INFRA_CEC                 = 70,
    MT_CG_INFRA_MFGAXI              = 71,
    MT_CG_INFRA_M4U                 = 72,
    MT_CG_INFRA_MD1MCUAXI           = 73,
    MT_CG_INFRA_MD1HWMIXAXI         = 74,
    MT_CG_INFRA_MD1AHB              = 75,
    MT_CG_INFRA_MD2MCUAXI           = 76,
    MT_CG_INFRA_MD2HWMIXAXI         = 77,
    MT_CG_INFRA_MD2AHB              = 78,
    MT_CG_INFRA_CPUM                = 79,
    MT_CG_INFRA_KP                  = 80,
    MT_CG_INFRA_CCIF0               = 84,
    MT_CG_INFRA_CCIF1               = 85,
    MT_CG_INFRA_PMICSPI             = 86,
    MT_CG_INFRA_PMICWRAP            = 87,

    MT_CG_TOPCK_PMICSPI             = 96,

    MT_CG_DISP0_LARB2_SMI           = 128,
    MT_CG_DISP0_ROT_ENGINE          = 129,
    MT_CG_DISP0_ROT_SMI             = 130,
    MT_CG_DISP0_SCL                 = 131,
    MT_CG_DISP0_OVL_ENGINE          = 132,
    MT_CG_DISP0_OVL_SMI             = 133,
    MT_CG_DISP0_COLOR               = 134,
    MT_CG_DISP0_2DSHP               = 135,
    MT_CG_DISP0_BLS                 = 136,
    MT_CG_DISP0_WDMA0_ENGINE        = 137,
    MT_CG_DISP0_WDMA0_SMI           = 138,
    MT_CG_DISP0_WDMA1_ENGINE        = 139,
    MT_CG_DISP0_WDMA1_SMI           = 140,
    MT_CG_DISP0_RDMA0_ENGINE        = 141,
    MT_CG_DISP0_RDMA0_SMI           = 142,
    MT_CG_DISP0_RDMA0_OUTPUT        = 143,
    MT_CG_DISP0_RDMA1_ENGINE        = 144,
    MT_CG_DISP0_RDMA1_SMI           = 145,
    MT_CG_DISP0_RDMA1_OUTPUT        = 146,
    MT_CG_DISP0_GAMMA_ENGINE        = 147,
    MT_CG_DISP0_GAMMA_PIXEL         = 148,
    MT_CG_DISP0_CMDQ_ENGINE         = 149,
    MT_CG_DISP0_CMDQ_SMI            = 150,
    MT_CG_DISP0_G2D_ENGINE          = 151,
    MT_CG_DISP0_G2D_SMI             = 152,

    MT_CG_DISP1_DBI_ENGINE          = 160,
    MT_CG_DISP1_DBI_SMI             = 161,
    MT_CG_DISP1_DBI_OUTPUT          = 162,
    MT_CG_DISP1_DSI_ENGINE          = 163,
    MT_CG_DISP1_DSI_DIGITAL         = 164,
    MT_CG_DISP1_DSI_DIGITAL_LANE    = 165,
    MT_CG_DISP1_DPI0                = 166,
    MT_CG_DISP1_DPI1                = 167,
    MT_CG_DISP1_LCD                 = 168,
    MT_CG_DISP1_SLCD                = 169,

    MT_CG_IMAGE_LARB3_SMI           = 192,
    MT_CG_IMAGE_LARB4_SMI           = 194,
    MT_CG_IMAGE_COMMN_SMI           = 196,
    MT_CG_IMAGE_CAM_SMI             = 197,
    MT_CG_IMAGE_CAM_CAM             = 198,
    MT_CG_IMAGE_SEN_TG              = 199,
    MT_CG_IMAGE_SEN_CAM             = 200,
    MT_CG_IMAGE_JPGD_SMI            = 201,
    MT_CG_IMAGE_JPGD_JPG            = 202,
    MT_CG_IMAGE_JPGE_SMI            = 203,
    MT_CG_IMAGE_JPGE_JPG            = 204,
    MT_CG_IMAGE_FPC                 = 205,

    MT_CG_MFG_AXI                   = 224,
    MT_CG_MFG_MEM                   = 225,
    MT_CG_MFG_G3D                   = 226,
    MT_CG_MFG_HYD                   = 227,

    MT_CG_AUDIO_AFE                 = 258,
    MT_CG_AUDIO_I2S                 = 262,

    MT_CG_VDEC0_VDE                 = 288,

    MT_CG_VDEC1_SMI                 = 320,

    MT_CG_VENC_VEN                  = 352,

    CG_PERI0_FROM                   = MT_CG_PERI0_NFI,
    CG_PERI0_TO                     = MT_CG_PERI0_I2C5,
    NR_PERI0_CLKS                   = 32,

    CG_PERI1_FROM                   = MT_CG_PERI1_I2C6,
    CG_PERI1_TO                     = MT_CG_PERI1_PHCTL,
    NR_PERI1_CLKS                   = 5,

    CG_INFRA_FROM                   = MT_CG_INFRA_DBGCLK,
    CG_INFRA_TO                     = MT_CG_INFRA_PMICWRAP,
    NR_INFRA_CLKS                   = 19,

    CG_TOPCK_FROM                   = MT_CG_TOPCK_PMICSPI,
    CG_TOPCK_TO                     = MT_CG_TOPCK_PMICSPI,
    NR_TOPCK_CLKS                   = 1,

    CG_DISP0_FROM                   = MT_CG_DISP0_LARB2_SMI,
    CG_DISP0_TO                     = MT_CG_DISP0_G2D_SMI,
    NR_DISP0_CLKS                   = 25,

    CG_DISP1_FROM                   = MT_CG_DISP1_DBI_ENGINE,
    CG_DISP1_TO                     = MT_CG_DISP1_SLCD,
    NR_DISP1_CLKS                   = 10,

    CG_IMAGE_FROM                   = MT_CG_IMAGE_LARB3_SMI,
    CG_IMAGE_TO                     = MT_CG_IMAGE_FPC,
    NR_IMAGE_CLKS                   = 12,

    CG_MFG_FROM                     = MT_CG_MFG_AXI,
    CG_MFG_TO                       = MT_CG_MFG_HYD,
    NR_MFG_CLKS                     = 4,

    CG_AUDIO_FROM                   = MT_CG_AUDIO_AFE,
    CG_AUDIO_TO                     = MT_CG_AUDIO_I2S,
    NR_AUDIO_CLKS                   = 2,

    CG_VDEC0_FROM                   = MT_CG_VDEC0_VDE,
    CG_VDEC0_TO                     = MT_CG_VDEC0_VDE,
    NR_VDEC0_CLKS                   = 1,

    CG_VDEC1_FROM                   = MT_CG_VDEC1_SMI,
    CG_VDEC1_TO                     = MT_CG_VDEC1_SMI,
    NR_VDEC1_CLKS                   = 1,

    CG_VENC_FROM                    = MT_CG_VENC_VEN,
    CG_VENC_TO                      = MT_CG_VENC_VEN,
    NR_VENC_CLKS                    = 1,

    NR_CLKS                         = 353,
};


enum {
    ARMPLL  = 0,
    MAINPLL = 1,
    MSDCPLL = 2,
    TVDPLL  = 3,
    LVDSPLL = 4,
    UNIVPLL = 5,
    MMPLL   = 6,
    ISPPLL  = 7,
    NR_PLLS = 8,
};

enum {
    SYS_MD1 = 0,
    SYS_MD2 = 1,
    SYS_DPY = 2,
    SYS_DIS = 3,
    SYS_MFG = 4,
    SYS_ISP = 5,
    SYS_IFR = 6,
    SYS_VEN = 7,
    SYS_VDE = 8,
    NR_SYSS = 9,
};

extern int disable_clock(int id, char *mod_name);
extern int enable_clock(int id, char *mod_name);
extern int enable_pll(int id, char *mod_name);
extern int disable_pll(int id, char *mod_name);
extern int enable_subsys(int id, char *mod_name);
extern int disable_subsys(int id, char *mod_name);

extern int pll_fsel(int id, unsigned int value);
extern int pll_is_on(int id);

extern int subsys_is_on(int id);
extern int md_power_on(int id);
extern int md_power_off(int id, unsigned int timeout);

#if 0   //add by mftsai
#define CLK_REG_WIDTH                   32
enum mt65xx_clock_category {
    MT65XX_CLOCK_PERI_PDN0              = 0,
    MT65XX_CLOCK_PERI_PDN1              = 1,
    MT65XX_CLOCK_MMSYS1_PDN0            = 2,
    MT65XX_CLOCK_MMSYS1_PDN1            = 3,
    MT65XX_CLOCK_MMSYS1_PDN2            = 4,
    MT65XX_CLOCK_MMSYS2_PDN             = 5,
    MT65XX_CLOCK_AUDIO_PDN              = 6,
    MT65XX_CLOCK_CATEGORY_COUNT         = 7,
};

enum mt65xx_clock_id {
    /* PERI_GLOBALCON_PDN0 */
    MT65XX_PDN_PERI_NFI                 = 0,
    MT65XX_PDN_PERI_THERM               = 1,
    MT65XX_PDN_PERI_PWM1                = 2,
    MT65XX_PDN_PERI_PWM2                = 3,
    MT65XX_PDN_PERI_PWM3                = 4,
    MT65XX_PDN_PERI_PWM456              = 5,
    MT65XX_PDN_PERI_PWM7                = 6,
    MT65XX_PDN_PERI_SIMIF0              = 7,
    MT65XX_PDN_PERI_SIMIF1              = 8,
    MT65XX_PDN_PERI_USB1                = 9,
    MT65XX_PDN_PERI_USB2                = 10,
    MT65XX_PDN_PERI_CCIF                = 11,
    MT65XX_PDN_PERI_APDMA               = 12,
    MT65XX_PDN_PERI_APXGPT              = 13,
    MT65XX_PDN_PERI_MSDC0               = 15,
    MT65XX_PDN_PERI_MSDC1               = 16,
    MT65XX_PDN_PERI_MSDC2               = 17,
    MT65XX_PDN_PERI_MSDC3               = 18,
    MT65XX_PDN_PERI_APHIF               = 19,
    MT65XX_PDN_PERI_MDHIF               = 20,
    MT65XX_PDN_PERI_NLI_ARB             = 21,
    MT65XX_PDN_PERI_ACCDET              = 22,
    MT65XX_PDN_PERI_IRDA                = 23,
    MT65XX_PDN_PERI_UART0               = 24,
    MT65XX_PDN_PERI_UART1               = 25,
    MT65XX_PDN_PERI_UART2               = 26,
    MT65XX_PDN_PERI_UART3               = 27,
    MT65XX_PDN_PERI_I2C0                = 28,
    MT65XX_PDN_PERI_I2C1                = 29,
    MT65XX_PDN_PERI_I2C_DUAL            = 30,
    MT65XX_PDN_PERI_AUXADC              = 31,

    /* PERI_GLOBALCON_PDN1 */
    MT65XX_PDN_PERI_HACC                 = 32,
    MT65XX_PDN_PERI_TRNG                = 33,

    /* MMSYS1 Clock Gating #0 */
    MT65XX_PDN_MM_VBUF                  = 64,
    MT65XX_PDN_MM_VDEC                  = 65,
    MT65XX_PDN_MM_VENC                  = 66,
    MT65XX_PDN_MM_SMI_LARB2_ACP_BUS     = 67,
    MT65XX_PDN_MM_SMI_LARB2_260MHZ      = 68,
    MT65XX_PDN_MM_SMI_LARB2_EMI         = 69,
    MT65XX_PDN_MM_SMI_LARB2_ACP_BUS_EMI = 70,
    MT65XX_PDN_MM_SMI_LARB1_EMI         = 71,
    MT65XX_PDN_MM_SMI_LARB0_EMI         = 72,

    /* MMSYS1 Clock Gating #1 */
    MT65XX_PDN_MM_VRZ1                  = 96,
    MT65XX_PDN_MM_CSI2                  = 97,
    MT65XX_PDN_MM_FD            		= 98,
    MT65XX_PDN_MM_RESZ_LB       		= 99,
    MT65XX_PDN_MM_TV_ROT        		= 100,
    MT65XX_PDN_MM_LCD           		= 101,
    MT65XX_PDN_MM_RGB_ROT2      		= 102,
    MT65XX_PDN_MM_PRZ1          		= 103,
    MT65XX_PDN_MM_R_DMA1        		= 104,
    MT65XX_PDN_MM_VDO_ROT1      		= 105,
    MT65XX_PDN_MM_RGB_ROT1      		= 106,
    MT65XX_PDN_MM_VRZ           		= 107,
    MT65XX_PDN_MM_RGB_ROT0      		= 108,
    MT65XX_PDN_MM_PRZ0_MOUT     		= 109,
    MT65XX_PDN_MM_PRZ0          		= 110,
    MT65XX_PDN_MM_VDO_ROT0      		= 111,
    MT65XX_PDN_MM_MOUT          		= 112,
    MT65XX_PDN_MM_OVL_DMA_MIMO  		= 113,
    MT65XX_PDN_MM_OVL_DMA_BPS   		= 114,
    MT65XX_PDN_MM_OVL_DMA       		= 115,
    MT65XX_PDN_MM_IPP_MOUT      		= 116,
    MT65XX_PDN_MM_IPP           		= 117,
    MT65XX_PDN_MM_EIS           		= 118,
    MT65XX_PDN_MM_CRZ           		= 119,
    MT65XX_PDN_MM_JPEG_DMA      		= 120,
    MT65XX_PDN_MM_BRZ_MOUT      		= 121,
    MT65XX_PDN_MM_BRZ           		= 122,
    MT65XX_PDN_MM_JPEG_DEC      		= 123,
    MT65XX_PDN_MM_JPEG_ENC      		= 124,
    MT65XX_PDN_MM_R_DMA0_MOUT   		= 125,
    MT65XX_PDN_MM_R_DMA0        		= 126,
    MT65XX_PDN_MM_CAM                   = 127,

    /*MMSYS1 Clock Gating #2 */
    MT65XX_PDN_MM_SCAM                  = 128,
    MT65XX_PDN_MM_SPI       			= 129,
    MT65XX_PDN_MM_TVC       			= 130,
    MT65XX_PDN_MM_TVE       			= 131,
    MT65XX_PDN_MM_DPI       			= 132,
    MT65XX_PDN_MM_DSI       			= 133,
    MT65XX_PDN_MM_SMI_LARB2 			= 134,
    MT65XX_PDN_MM_SMI_LARB1 			= 135,
    MT65XX_PDN_MM_SMI_LARB0 			= 136,
    MT65XX_PDN_MM_LCD_EMI   			= 136,

    /* MMSYS2 Clock Gating */
    MT65XX_PDN_MM_SMI_LARB3_FULL    	= 160,
    MT65XX_PDN_MM_GDC_SHARE_MACRO   	= 162,
    MT65XX_PDN_MM_G2D               	= 163,
    MT65XX_PDN_MM_MFG               	= 164,
    MT65XX_PDN_MM_G3D               	= 174,
    MT65XX_PDN_MM_F26M              	= 175,
    MT65XX_PDN_MM_GDC_SHARE_MACRO_HALF  = 176,
    MT65XX_PDN_MM_SMI_LARB3_HALF    	= 177,
    MT65XX_PDN_MM_AUDIO_CORE            = 178,
    MT65XX_PDN_MM_MFG_HALF          	= 179,

    /* MMSYS2 Clock Gating */
    MT65XX_PDN_AUDIO_AFE                = 194,
    MT65XX_PDN_AUDIO_ADC                = 197,
    MT65XX_PDN_AUDIO_I2S                = 198,

    /* Alias */
    MT65XX_CLOCK_COUNT_BEGIN            = MT65XX_PDN_PERI_NFI,

    MT65XX_PERI_PDN0_BEGIN              = MT65XX_PDN_PERI_NFI,
    MT65XX_PERI_PDN0_END                = MT65XX_PDN_PERI_AUXADC,

    MT65XX_PERI_PDN1_BEGIN              = MT65XX_PDN_PERI_HACC,
    MT65XX_PERI_PDN1_END                = MT65XX_PDN_PERI_TRNG,

    MT65XX_MM1_PDN0_BEGIN               = MT65XX_PDN_MM_VBUF,
    MT65XX_MM1_PDN0_END                 = MT65XX_PDN_MM_SMI_LARB0_EMI,

    MT65XX_MM1_PDN1_BEGIN               = MT65XX_PDN_MM_VRZ1,
    MT65XX_MM1_PDN1_END                 = MT65XX_PDN_MM_CAM,

    MT65XX_MM1_PDN2_BEGIN               = MT65XX_PDN_MM_SCAM,
    MT65XX_MM1_PDN2_END                 = MT65XX_PDN_MM_LCD_EMI,

    MT65XX_MM2_PDN_BEGIN                = MT65XX_PDN_MM_SMI_LARB3_FULL,
    MT65XX_MM2_PDN_END                  = MT65XX_PDN_MM_MFG_HALF,

    MT65XX_AUDIO_PDN_BEGIN              = CLK_REG_WIDTH * MT65XX_CLOCK_AUDIO_PDN, /* 32*6 = 192*/
    MT65XX_AUDIO_PDN_END                = MT65XX_PDN_AUDIO_I2S,

    MT65XX_CLOCK_COUNT,
};

enum mt65xx_pll_id {
    MT65XX_ARMPLL                       = 0,
    MT65XX_MAINPLL                      = 1,
    MT65XX_IPLL                         = 2,
    MT65XX_UPLL                         = 3,
    MT65XX_MDPLL                        = 4,
    MT65XX_TVDDS                        = 5,
    MT65XX_WPLL                         = 6,
    MT65XX_AUDPLL                       = 7,
    MT65XX_MEMPLL                       = 8,
    MT65XX_PLL_COUNT                    = 9,
};

#else
enum mt65xx_clock_id {
    MT65XX_PDN_PERI_USB1                = 9,
    MT65XX_PDN_PERI_AUXADC              = 31,
    MT65XX_PDN_MM_SPI       			= 129,
};

enum mt65xx_pll_id {
    MT65XX_UPLL                         = 3,
    MT65XX_TVDDS                        = 5,
    MT65XX_AUDPLL                       = 7,
};
#endif

#endif
