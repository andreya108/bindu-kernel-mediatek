#ifndef __CUST_KEY_H__
#define __CUST_KEY_H__

#include<cust_kpd.h>

#define MT65XX_META_KEY		42	/* KEY_2 */
/*lenovo-sw jixj 2013.11.1 remove begin*/
//#define MT65XX_PMIC_RST_KEY	1000	/* Invalid Key */
/*lenovo-sw jixj 2013.11.1 remove  begin*/
#define MT_CAMERA_KEY 		10

#define MT65XX_BOOT_MENU_KEY       1   /* KEY_VOLUMEUP */
#define MT65XX_MENU_SELECT_KEY     MT65XX_BOOT_MENU_KEY   
#define MT65XX_MENU_OK_KEY         0    /* KEY_VOLUMEDOWN */

/*lenovo-sw jixj 2013.11.1 add begin*/
#ifdef LENOVO_PMIC_HOME_KEY
#define MT65XX_PMIC_RST_KEY	2
#endif
/*lenovo-sw jixj 2013.11.1 add end*/

/*lenovo-sw jixj 2013.9.18 add begin*/
#define LENOVO_META_KEY MT65XX_FACTORY_KEY
#define LENOVO_FACTORY_KEY MT65XX_RECOVERY_KEY
/*lenovo-sw jixj 2013.9.18 add end*/
#endif /* __CUST_KEY_H__ */
