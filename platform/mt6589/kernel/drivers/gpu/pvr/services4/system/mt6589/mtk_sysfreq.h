#ifndef MTK_SYSFREQ_H
#define MTK_SYSFREQ_H

#include "sgxdefs.h"
#include "services_headers.h"

#include "mach/mt_clkmgr.h"

#define TBLTYPE0        0x0
#define TBLTYPE1        0x1
#define TBLTYPE2        0x2
#define TBLTYPE3        0x3

#define TBLTYPE0_0      0x4
#define TBLTYPE0_1      0x5
#define TBLTYPE0_2      0x6
#define TBLTYPE0_3      0x7

typedef struct PVRSRV_BRIDGE_INPUT_TAG
{
    unsigned int freq;
    unsigned int type;
} PVRSRV_BRIDGE_INPUT;

typedef struct PVRSRV_BRIDGE_PWSRC_INPUT_TAG
{
    unsigned int in;
} PVRSRV_BRIDGE_PWSRC_INPUT;

typedef struct PVRSRV_BRIDGE_PWSRC_RETURN_TAG
{
    int powersrc;
} PVRSRV_BRIDGE_PWSRC_RETURN;

PVRSRV_ERROR MTKSetFreqInfo(unsigned int freq, unsigned int tbltype);


PVRSRV_ERROR MtkInitSetFreq(void);
void MtkInitSetFreqTbl(unsigned int tbltype);
void MtkSetKeepFreq(void);

extern unsigned int proton_gpu_frequency_get(void);
extern unsigned int proton_gpu_tbltype_get(void);
extern unsigned int proton_gpu_voltage_get(int num);
extern bool proton_gpu_dvfs;

#endif // MTK_SYSFREQ_H
