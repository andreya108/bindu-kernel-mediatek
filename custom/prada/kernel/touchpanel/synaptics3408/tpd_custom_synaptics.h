#ifndef TOUCHPANEL_H__
#define TOUCHPANEL_H__

/* Pre-defined definition */
#define TPD_TYPE_CAPACITIVE
//#define TPD_POWER_SOURCE	MT6323_POWER_LDO_VGP1         
#define TPD_POWER_SOURCE_CUSTOM         MT65XX_POWER_LDO_VGP4
#define TPD_POWER_SOURCE_1800           MT65XX_POWER_LDO_VGP6

#define TPD_I2C_NUMBER		0
#define TPD_I2C_ADDR		0x38
#define TPD_WAKEUP_TRIAL	60
#define TPD_WAKEUP_DELAY	100

//#define SEINE_HACK

#define TPD_DRIVER_NAME "syn-tpd"
//#define TPD_HAVE_TREMBLE_ELIMINATION

/* Define the virtual button mapping */
#define TPD_HAVE_BUTTON
#define TPD_BUTTON_HEIGH        (100)
#define TPD_KEY_COUNT           3
#define TPD_KEYS                { KEY_MENU, KEY_HOMEPAGE, KEY_BACK}
#define TPD_KEYS_DIM            {{145,1330,120,TPD_BUTTON_HEIGH},\
                                                        {360,1330,120,TPD_BUTTON_HEIGH},\
                                                        {600,1330,120,TPD_BUTTON_HEIGH}}

/* Define the touch dimension */
#ifdef TPD_HAVE_BUTTON
#define TPD_TOUCH_HEIGH_RATIO	1380
#define TPD_DISPLAY_HEIGH_RATIO	1280
#endif

#endif /* TOUCHPANEL_H__ */

