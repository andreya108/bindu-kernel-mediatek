#ifndef TOUCHPANEL_H__
#define TOUCHPANEL_H__

/* Pre-defined definition */
#define TPD_TYPE_CAPACITIVE
#define TPD_TYPE_RESISTIVE
#define TPD_POWER_SOURCE         
#define TPD_I2C_NUMBER           0
#define TPD_WAKEUP_TRIAL         60
#define TPD_WAKEUP_DELAY         100

//#define TPD_VELOCITY_CUSTOM_X 15
//#define TPD_VELOCITY_CUSTOM_Y 20

//#define TPD_POWER_SOURCE_CUSTOM         MT65XX_POWER_LDO_VGP2

#define TPD_DELAY                (2*HZ/100)
//#define TPD_RES_X                480
//#define TPD_RES_Y                800
#define TPD_CALIBRATION_MATRIX  {962,0,0,0,1600,0,0,0};

//#define TPD_HAVE_CALIBRATION
//#define TPD_HAVE_BUTTON
//#define TPD_HAVE_TREMBLE_ELIMINATION
#define TPD_HAVE_BUTTON
#define TPD_BUTTON_HEIGH        (64)
//#define TPD_BUTTON_WIDTH        (240)
#define TPD_KEY_COUNT           3
#define TPD_KEYS                { KEY_MENU, KEY_HOMEPAGE, KEY_BACK}
#define TPD_KEYS_DIM            {{95,1370,129,TPD_BUTTON_HEIGH},{360,1370,135,TPD_BUTTON_HEIGH},{627,1370,129,TPD_BUTTON_HEIGH}}

#endif /* TOUCHPANEL_H__ */
