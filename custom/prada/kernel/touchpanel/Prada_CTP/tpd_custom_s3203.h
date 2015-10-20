#ifndef TOUCHPANEL_H__
#define TOUCHPANEL_H__

/* Pre-defined definition */

/* Register */
#define FD_ADDR_MAX    	0xE9
#define FD_ADDR_MIN    	0xDD
#define FD_BYTE_COUNT 	6

#define CUSTOM_MAX_WIDTH (720)
#define CUSTOM_MAX_HEIGHT (1280)

//#define TPD_UPDATE_FIRMWARE

#define TPD_HAVE_BUTTON
#define TPD_BUTTON_HEIGH        (100)
#define TPD_KEY_COUNT           3
#define TPD_KEYS                { KEY_MENU, KEY_HOMEPAGE, KEY_BACK}
#define TPD_KEYS_DIM		{{145,1330,120,TPD_BUTTON_HEIGH},\
							{360,1330,120,TPD_BUTTON_HEIGH},\
							{600,1330,120,TPD_BUTTON_HEIGH}}

#define TPD_POWER_SOURCE_CUSTOM	    MT65XX_POWER_LDO_VGP4
#define TPD_POWER_SOURCE_1800		MT65XX_POWER_LDO_VGP6


//#define TPD_HAVE_CALIBRATION
//#define TPD_CALIBRATION_MATRIX  {2680,0,0,0,2760,0,0,0};
//#define TPD_WARP_START
//#define TPD_WARP_END

//#define TPD_RESET_ISSUE_WORKAROUND
//#define TPD_MAX_RESET_COUNT 3
//#define TPD_WARP_Y(y) ( TPD_Y_RES - 1 - y )
//#define TPD_WARP_X(x) ( x )

#endif /* TOUCHPANEL_H__ */
