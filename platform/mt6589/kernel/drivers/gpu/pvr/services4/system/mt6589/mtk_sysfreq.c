
#include "mtk_sysfreq.h"
#include "mach/mt_gpufreq.h"
#include "mach/mt_clkmgr.h"
#include <mach/sync_write.h>

static bool mt_keep_freq_non_od_set = false;

//#define MTK_GPU_DVFS 0

void MtkInitSetFreqTbl(unsigned int tbltype)
{
    int volt_max = proton_gpu_voltage_get(0);
    int volt_mid = proton_gpu_voltage_get(1);
    int volt_min = proton_gpu_voltage_get(2);

    bool dvfs_on = proton_gpu_dvfs;

    if(dvfs_on) {
        // 238 - 156MHz
        struct mt_gpufreq_info freqs_special0_0_vrf18_2[] = {
            {GPU_DVFS_F7, 40, 100, volt_max, 100},
            {GPU_DVFS_F8, 0,  40, volt_min,  80},
        };
        // 268 - 238MHz
        struct mt_gpufreq_info freqs_special0_1_vrf18_2[] = {
            {GPU_DVFS_F6, 40, 100, volt_max, 100},
            {GPU_DVFS_F7, 0,  40, volt_min,  80},
        };
        // 286 - 268MHz
        struct mt_gpufreq_info freqs_special0_2_vrf18_2[] = {
            {GPU_DVFS_F5, 40, 100, volt_max, 100},
            {GPU_DVFS_F6, 0,  40, volt_min,  80},
        };
        // 312 - 286MHz
        struct mt_gpufreq_info freqs_special0_3_vrf18_2[] = {
            {GPU_DVFS_F5, 40, 100, volt_max, 100},
            {GPU_DVFS_F6, 0,  40, volt_min,  80},
        };
        // 357 - 286MHz
        struct mt_gpufreq_info freqs_special_vrf18_2[] = {
            {GPU_DVFS_F3, 40, 100, volt_max, 100},
            {GPU_DVFS_F5, 0,  40, volt_min,  80},
        };
        // 403 - 357 - 286MHz
        struct mt_gpufreq_info freqs_special2_vrf18_2[] = {
            {GPU_DVFS_F2, 60, 100, volt_max, 100},
            {GPU_DVFS_F3, 30,  60, volt_mid,  90},
            {GPU_DVFS_F5, 0,  30, volt_min,  75},
        };
        // 476 - 357 - 286MHz
        struct mt_gpufreq_info freqs_special3_vrf18_2[] = {
            {GPU_DVFS_F1, 60, 100, volt_max, 100},
            {GPU_DVFS_F3, 30,  60, volt_mid, 90},
            {GPU_DVFS_F5, 0,  30, volt_min,  75},
        };

        switch (tbltype)
        {
        case TBLTYPE0_0:
            mt_gpufreq_register(freqs_special0_0_vrf18_2, 2);
            break;
        case TBLTYPE0_1:
            mt_gpufreq_register(freqs_special0_1_vrf18_2, 2);
            break;
        case TBLTYPE0_2:
            mt_gpufreq_register(freqs_special0_2_vrf18_2, 2);
            break;
        case TBLTYPE0_3:
            mt_gpufreq_register(freqs_special0_3_vrf18_2, 2);
            break;
        case TBLTYPE1:
            mt_gpufreq_register(freqs_special_vrf18_2, 2);
            break;
        case TBLTYPE2:
            mt_gpufreq_register(freqs_special2_vrf18_2, 3);
            break;
        case TBLTYPE3:
            mt_gpufreq_register(freqs_special3_vrf18_2, 3);
            break;
        default:
        case TBLTYPE0:
            mt_gpufreq_non_register();
            break;
        }

    } else {
        mt_gpufreq_non_register();
    }
}


PVRSRV_ERROR MTKSetFreqInfo(unsigned int freq, unsigned int tbltype)
{
    unsigned int voltage;
    unsigned int pll;

    bool dvfs_on = proton_gpu_dvfs;

    freq = proton_gpu_frequency_get();
    tbltype = proton_gpu_tbltype_get();
    voltage = proton_gpu_voltage_get(0);

//#if defined(MTK_FREQ_OD_INIT)
//    if (freq > GPU_DVFS_F8)
//    {
        mt_gpufreq_set_initial(freq, voltage);
        //mt_gpufreq_set_initial(freq, GPU_POWER_VRF18_1_05V);
        mt65xx_reg_sync_writel((readl(CLK_CFG_8)&0xffcffff)|0x30000, CLK_CFG_8);

        if (dvfs_on) {
            mt_gpufreq_keep_frequency_non_OD_init(GPU_KEEP_FREQ_NON_OD_BYPASS, GPU_KEEP_VOLT_NON_OD_BYPASS);
        } else {
            switch (freq)
            {
                case GPU_DVFS_F1: // 476MHz
                    pll = GPU_MMPLL_D3;
                    break;
                case GPU_DVFS_F2: // 403MHz
                    pll = GPU_SYSPLL_D2;
                    break;
                case GPU_DVFS_F3: // 357MHz
                    pll = GPU_MMPLL_D4;
                    break;
                case GPU_DVFS_F4: // 312MHz
                    pll = GPU_UNIVPLL1_D2;
                    break;
                default: // set in the default parameter initialization anyway
                case GPU_DVFS_F5: // 286MHz
                    pll = GPU_MMPLL_D5;
                    break;
                case GPU_DVFS_F6: // 268MHz
                    pll = GPU_SYSPLL_D3;
                    break;
                case GPU_DVFS_F7: // 238MHz
                    pll = GPU_MMPLL_D6;
                    break;
                case GPU_DVFS_F8: // 156MHz
                    pll = GPU_UNIVPLL1_D4;
                    break;
            }
            mt_gpufreq_keep_frequency_non_OD_init(pll, voltage);
        }

//    }
//    else
//#endif
//    {
//        mt_gpufreq_set_initial(freq, GPU_POWER_VRF18_1_05V);
//        mt_gpufreq_keep_frequency_non_OD_init(GPU_KEEP_FREQ_NON_OD_BYPASS, GPU_KEEP_VOLT_NON_OD_BYPASS);
//    }

    MtkInitSetFreqTbl(tbltype);


    return PVRSRV_OK;
}

void MtkSetKeepFreq(void)
{
    if (mt_gpufreq_keep_frequency_non_OD_get())
    {
        if (mt_keep_freq_non_od_set==false)
        {
            mt_gpufreq_keep_frequency_non_OD_set(1);
            mt_keep_freq_non_od_set=true;
        }
    }
    else
    {
        if (mt_keep_freq_non_od_set==true)
        {
            mt_gpufreq_keep_frequency_non_OD_set(0);
            mt_keep_freq_non_od_set=false;
        }
    }
}
