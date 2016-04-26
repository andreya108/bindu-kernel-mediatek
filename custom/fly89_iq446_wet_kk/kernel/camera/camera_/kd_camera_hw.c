/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>
#include <linux/kernel.h>//for printk


#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    xlog_printk(ANDROID_LOG_INFO, PFX , fmt, ##arg)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         xlog_printk(ANDROID_LOG_ERR, PFX , fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...) \
                do {    \
                   xlog_printk(ANDROID_LOG_INFO, PFX , fmt, ##arg); \
                } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif


#define SENSOR_GC2035_YUV_PWR_ON() do {                                                                                   \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name)) {                                           \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_D2\n");                          \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                mdelay(10);                                                                                                 \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable analog power:CAMERA_POWER_VCAM_A\n");                            \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                mdelay(2);                                                                                                  \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1800,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_D\n");                           \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                mdelay(5);                                                                                                  \
            }while (0)
            
#define SENSOR_OV8825_MIPI_RAW_PWR_ON() do {	\
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name)) {                                           \
			PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_D2\n");                          \
			goto _kdCISModulePowerOn_exit_;                                                                         \
		}                                                                                                           \
		mdelay(8);                                                                                                  \ 
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name)) {                                            \
			PK_DBG("[CAMERA SENSOR] Fail to enable analog power:CAMERA_POWER_VCAM_A\n");                            \
			goto _kdCISModulePowerOn_exit_;                                                                         \
		}                                                                                                           \
		mdelay(5);                                                                                                  \
		/*if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name)) {                                            \
		PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_D\n");                           \
		goto _kdCISModulePowerOn_exit_;                                                                         \
		}    use internel LDO*/                                                                                                     \
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name)) {                                            \
			PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_A2\n");                           \
			goto _kdCISModulePowerOn_exit_;                                                                         \
		}                                                                                                           \
		mdelay(5);                                                                                                  \
		}while (0)
		
#define SENSOR_OV8825_MIPI_RAW_PWR_OFF() do {\
		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {\
			PK_DBG("[CAMERA SENSOR] Fail to disable CAMERA_POWER_VCAM_A power\n");\
		}\
		mdelay(1);\
		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2, mode_name)) {\
			PK_DBG("[CAMERA SENSOR] Fail to disable CAMERA_POWER_VCAM_D2 power\n");\
		}\
		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D,mode_name)){\
			PK_DBG("[CAMERA SENSOR] Fail to disable CAMERA_POWER_VCAM_D power\n");\
		}\
		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2, mode_name)) {\
			PK_DBG("[CAMERA SENSOR] Fail to disable CAMERA_POWER_VCAM_A2 power\n");\
		}\
		}while (0)

#define SENSOR_OV5647_RAW_PWR_ON() do {	\
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name)) {                                           \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_D2\n");                          \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable analog power:CAMERA_POWER_VCAM_A\n");                            \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_D\n");                           \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_A2\n");                           \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                mdelay(5);                                                                                                  \
            }while (0)

#define SENSOR_S5K5CAGX_YUV_PWR_ON() do {  \
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_D\n");                           \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                 \
                mdelay(1);                                                                                                 \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable analog power:CAMERA_POWER_VCAM_A\n");                            \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                mdelay(2);                                                                                                  \
                 if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name)) {                                           \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_D2\n");                          \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                } mdelay(5);                                                                                                  \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_A2\n");                           \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                mdelay(5);                                                                                                  \                
              }while (0)
#define SENSOR_A5142_RAW_PWR_ON() do {	\
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name)) {                                           \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_D2\n");                          \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable analog power:CAMERA_POWER_VCAM_A\n");                            \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1800,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_D\n");                           \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name)) {                                            \
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power:CAMERA_POWER_VCAM_A2\n");                           \
                    goto _kdCISModulePowerOn_exit_;                                                                         \
                }                                                                                                           \
                mdelay(5);                                                                                                  \
            }while (0)
#define SENSOR_PWR_OFF() do {\
		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {\
		PK_DBG("[CAMERA SENSOR] Fail to disable CAMERA_POWER_VCAM_A power\n");\
		}\
		mdelay(5);\
		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2, mode_name)) {\
		PK_DBG("[CAMERA SENSOR] Fail to disable CAMERA_POWER_VCAM_A2 power\n");\
		}\
		mdelay(5);\
		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D,mode_name)){\
		PK_DBG("[CAMERA SENSOR] Fail to disable CAMERA_POWER_VCAM_D power\n");\
		}\
		mdelay(5);\
		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name)){\
		PK_DBG("[CAMERA SENSOR] Fail to disable CAMERA_POWER_VCAM_A2 power\n");\
		}\
		}while (0)
int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{
u32 pinSetIdx = 0;//default main sensor

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4

#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3   
u32 pinSet[3][8] = {
                    //for main sensor
                    {GPIO_CAMERA_CMRST_PIN,
                        GPIO_CAMERA_CMRST_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,                   /* ON state */
                        GPIO_OUT_ZERO,                  /* OFF state */
                     GPIO_CAMERA_CMPDN_PIN,
                        GPIO_CAMERA_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                    },
                    //for sub sensor
                    {GPIO_CAMERA_CMRST1_PIN,
                     GPIO_CAMERA_CMRST1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                     GPIO_CAMERA_CMPDN1_PIN,
                        GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
                        GPIO_OUT_ZERO,
                        GPIO_OUT_ONE,
                    },
                    //for main_2 sensor
                    {GPIO_CAMERA_2_CMRST_PIN,
                        GPIO_CAMERA_2_CMRST_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,                   /* ON state */
                        GPIO_OUT_ZERO,                  /* OFF state */
                     GPIO_CAMERA_2_CMPDN_PIN,
                        GPIO_CAMERA_2_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                    }
                   };




    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    }
    else if (DUAL_CAMERA_MAIN_2_SENSOR == SensorIdx) {
       // pinSetIdx = 2;
    }
    if((pinSetIdx ==0) && (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5647MIPI_RAW,currSensorName))))
    {
       pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON] = GPIO_OUT_ZERO;
	pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF] = GPIO_OUT_ONE;
	pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON] = GPIO_OUT_ZERO;
	pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF] = GPIO_OUT_ONE;
    }
	if((pinSetIdx ==0) && (currSensorName && (0 == strcmp(SENSOR_DRVNAME_A5142_MIPI_RAW,currSensorName))))
    {
       pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON] = GPIO_OUT_ONE;
	pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF] = GPIO_OUT_ZERO;
	pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON] = GPIO_OUT_ZERO;
	pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF] = GPIO_OUT_ONE;
    }
    if ((pinSetIdx ==1) && (currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K5CAGX_YUV,currSensorName))))
    {
       pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON] = GPIO_OUT_ZERO;
	pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF] = GPIO_OUT_ONE;
	pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON] = GPIO_OUT_ZERO;
	pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF] = GPIO_OUT_ONE;
    }
    //power ON
    if (On) {


#if 0  //TODO: depends on HW layout. Should be notified by SA.

        PK_DBG("Set CAMERA_POWER_PULL_PIN for power \n");
        if (mt_set_gpio_pull_enable(GPIO_CAMERA_LDO_EN_PIN, GPIO_PULL_DISABLE)) {PK_DBG("[[CAMERA SENSOR] Set CAMERA_POWER_PULL_PIN DISABLE ! \n"); }
        if(mt_set_gpio_mode(GPIO_CAMERA_LDO_EN_PIN, GPIO_CAMERA_LDO_EN_PIN_M_GPIO)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN mode failed!! \n");}
        if(mt_set_gpio_dir(GPIO_CAMERA_LDO_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN dir failed!! \n");}
        if(mt_set_gpio_out(GPIO_CAMERA_LDO_EN_PIN,GPIO_OUT_ONE)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN failed!! \n");}
#endif

		PK_DBG("kdCISModulePowerOn -on:currSensorName=%s\n",currSensorName);
		PK_DBG("kdCISModulePowerOn -on:pinSetIdx=%d\n",pinSetIdx);

		if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV8825_MIPI_RAW,currSensorName) ||(0 == strcmp(SENSOR_DRVNAME_OV8830_RAW,currSensorName))))
		{
			PK_DBG("kdCISModulePowerOn get in---  SENSOR_DRVNAME_OV8825_MIPI_RAW \n");
			PK_DBG("[ON_general 2.8V]sensorIdx:%d \n",SensorIdx);
			
			SENSOR_OV8825_MIPI_RAW_PWR_ON();

			//PDN/STBY pin
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
			{
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
				msleep(10);
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
				msleep(1);

				//RST pin
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(10);
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(1);
			}

        //disable inactive sensor
        if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
	        if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
	            if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	        }
        }
		else {
	        if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
	            if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	        }
	        if (GPIO_CAMERA_INVALID != pinSet[2][IDX_PS_CMRST]) {
               /*
	            if(mt_set_gpio_mode(pinSet[2][IDX_PS_CMRST],pinSet[2][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[2][IDX_PS_CMPDN],pinSet[2][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[2][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[2][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[2][IDX_PS_CMRST],pinSet[2][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
	            if(mt_set_gpio_out(pinSet[2][IDX_PS_CMPDN],pinSet[2][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
               */
	        }
		}

	}
	else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2035_YUV,currSensorName))) 
	{
		printk("SENSOR_Gc2035_YUV_PWR_ON ");
		SENSOR_GC2035_YUV_PWR_ON();  
		//disable inactive sensor
		if (GPIO_CAMERA_INVALID != pinSet[1-pinSetIdx][IDX_PS_CMRST]) {
			if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			mdelay(5);
			if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
			mdelay(5);
		}        

		//enable active sensor
		//RST pin
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
			//PDN pin
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
			mdelay(10);

			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
			mdelay(10);
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
			mdelay(5);
		}   
	}
	else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5647MIPI_RAW,currSensorName)))
	{      

		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
			mdelay(5);
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
			mdelay(5);
		}
	       SENSOR_OV5647_RAW_PWR_ON();
		mdelay(2); 

		//RST pin
		if (GPIO_CAMERA_INVALID != pinSet[1-pinSetIdx][IDX_PS_CMRST]) {
			if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			mdelay(5);
			if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
			mdelay(5);
		}  

		 //enable active sensor
	        //RST pin
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
			//PDN pin
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
			mdelay(30);
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
		}	
		mdelay(10); 
		printk("SENSOR_OV5647_RAW_PWR_ON'");
	}
        else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_A5142_MIPI_RAW,currSensorName)))
	{      

		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
			mdelay(5);
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
			mdelay(5);
		}
	       SENSOR_A5142_RAW_PWR_ON();
		mdelay(10);   
		//RST pin
		if (GPIO_CAMERA_INVALID != pinSet[1-pinSetIdx][IDX_PS_CMRST]) {
			if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			mdelay(5);
			if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
			mdelay(5);
		}  

		 //enable active sensor
	        //RST pin
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
			//PDN pin
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
			mdelay(30);
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
		}	
		mdelay(10); 
		printk("SENSOR_A5142_RAW_PWR_ON'");
	}
	else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K5CAGX_YUV,currSensorName)))
	{
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
			mdelay(5);
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
			mdelay(5);
		}
		printk("SENSOR_S5K5CAGX_YUV_PWR_ON'");
		SENSOR_S5K5CAGX_YUV_PWR_ON();

			//disable inactive sensor
		if (GPIO_CAMERA_INVALID != pinSet[1-pinSetIdx][IDX_PS_CMRST]) {
			if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			mdelay(5);
			if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
			mdelay(5);
		}        

		//enable active sensor
	        //RST pin
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
			//PDN pin
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
			mdelay(30);
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
		}	
		mdelay(10); 

	}
	else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K8AAYX_MIPI_YUV,currSensorName)))
	{

	}
	else
	{

	}

  }
  else {//power OFF

#if 0 //TODO: depends on HW layout. Should be notified by SA.
        PK_DBG("Set GPIO 94 for power OFF\n");
        if (mt_set_gpio_pull_enable(GPIO_CAMERA_LDO_EN_PIN, GPIO_PULL_DISABLE)) {PK_DBG("[CAMERA SENSOR] Set GPIO94 PULL DISABLE ! \n"); }
        if(mt_set_gpio_mode(GPIO_CAMERA_LDO_EN_PIN, GPIO_CAMERA_LDO_EN_PIN_M_GPIO)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
        if(mt_set_gpio_dir(GPIO_CAMERA_LDO_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
        if(mt_set_gpio_out(GPIO_CAMERA_LDO_EN_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
#endif
		PK_DBG("kdCISModulePowerOn -off:currSensorName=%s\n",currSensorName);

		if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV8825_MIPI_RAW,currSensorName) ||(0 == strcmp(SENSOR_DRVNAME_OV8830_RAW,currSensorName))))
		{
			PK_DBG("kdCISModulePower--off get in---SENSOR_DRVNAME_OV8825_MIPI_RAW \n");

			//reset pull down
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			}

			SENSOR_OV8825_MIPI_RAW_PWR_OFF();
			
			//PDN pull down
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
			}
		}
		else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2035_YUV,currSensorName))) 
		{
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
				mdelay(2);
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
				mdelay(2);
			}
			PK_DBG("kdCISModulePower--off get in---SENSOR_DRVNAME_GC2035_YUV \n");

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
			PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
			PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
			}

			if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
			{
			PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
			}
		}
		else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5647MIPI_RAW,currSensorName)))
		{
	  		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	  			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	  			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	  			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	  			mdelay(5);
	  			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	  			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	  			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	  			mdelay(5);
			PK_DBG("kdCISModulePower--off get in---SENSOR_DRVNAME_OV5647MIPI_RAW \n");
			SENSOR_PWR_OFF();
			}
		}
		else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K5CAGX_YUV,currSensorName)))
		{
			  if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	  			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	  			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	  			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	  			mdelay(5);
	  			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	  			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	  			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	  			mdelay(5);
			PK_DBG("kdCISModulePower--off get in---SENSOR_DRVNAME_S5K5CAGX_YUV \n");
			SENSOR_PWR_OFF();

			}
		}
		else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K8AAYX_MIPI_YUV,currSensorName)))
		{

		}
		else
		{
			SENSOR_PWR_OFF();
		}
		
  }
	return 0;

_kdCISModulePowerOn_exit_:
	SENSOR_PWR_OFF();
	return -EIO;
}

EXPORT_SYMBOL(kdCISModulePowerOn);


//!--
//




