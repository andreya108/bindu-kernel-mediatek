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
#define HAVE_TOUCH_KEY
#define TPD_HAVE_BUTTON
#define TPD_BUTTON_HEIGH        (100)
#define TPD_KEY_COUNT           3
#define TPD_KEYS                {KEY_MENU, KEY_HOMEPAGE, KEY_BACK}
#define TPD_KEYS_DIM            {{130,1340,180,60},{360,1340,180,60},{600,1340,180,60}}

//#define TPD_POWER_SOURCE_1800         MT65XX_POWER_LDO_VGP5
#define TPD_POWER_SOURCE_CUSTOM         MT65XX_POWER_LDO_VGP1

#define LCD_X           720
#define LCD_Y           1280

//#define TPD_HAVE_CALIBRATION
//#define TPD_CALIBRATION_MATRIX  {2465,0,0,0,2525,0,0,0};
//#define TPD_WARP_START
//#define TPD_WARP_END

#endif /* TOUCHPANEL_H__ */
