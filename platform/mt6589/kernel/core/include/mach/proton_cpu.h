#ifndef _PROTON_CPU_H
#define _PROTON_CPU_H

/**
 * Chipset Undervolt
 */

#if defined(PROTON_UNDERVOLT)
	#define DVFS_V0 (1075)  // mV
	#define DVFS_V1 (1050)  // mV
	#define DVFS_V2 ( 950)  // mV
	#define DVFS_V3 ( 900)  // mV
	#define DVFS_V4 ( 800)  // mV
	
	/* offset values */
	#define DVFS_PKV0 0x3C // 1.075V
	#define DVFS_PKV1 0x38 // 1.050V
	#define DVFS_PKV2 0x28 // 0.950V
	#define DVFS_PKV3 0x24 // 0.925V
	#define DVFS_PKV4 0x10 // 0.800V
	
	#define DVFS_PKV5 0x30 // 1.000V
	#define DVFS_PKV6 0x28 // 0.950V
	#define DVFS_PKV7 0x10 // 0.800V
	
#else //default
	#define DVFS_V0     (1200)  // mV
	#define DVFS_V1     (1150)  // mV
	#define DVFS_V2     (1050)  // mV
	#define DVFS_V3     ( 950)  // mV
	#define DVFS_V4     ( 850)  // mV
	
	/* offset values */
	#define DVFS_PKV0 0x50 // 1.20V
	#define DVFS_PKV1 0x48 // 1.15V
	#define DVFS_PKV2 0x38 // 1.05V
	#define DVFS_PKV3 0x28 // 0.95V
	#define DVFS_PKV4 0x18 // 0.85V

	#define DVFS_PKV5 0x38 // 1.050V
	#define DVFS_PKV6 0x28 // 0.950V
	#define DVFS_PKV7 0x18 // 0.850V
#endif

	#define PK_VMAX 0x5D // 1.28125V
	#define PK_VMIN DVFS_PKV4 // 0.800V
	#define PK_VBOOT DVFS_PKV1 // 1.050V

#endif
