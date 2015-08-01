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
#define PK_DBG_FUNC(fmt, arg...)    printk(KERN_INFO PFX "%s: " fmt, __FUNCTION__ ,##arg)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         printk(KERN_ERR PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_XLOG_INFO(fmt, args...) \
                do {    \
                    xlog_printk(ANDROID_LOG_INFO, "kd_camera_hw", fmt, ##args); \
                } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif

kal_bool searchMainSensor = KAL_TRUE;
//#define CHECK_SENSOR_ID_FOR_PRADA  1
kal_bool ENABLE_DVDD_VOL_1500_8825 = KAL_FALSE;
kal_bool ENABLE_DVDD_VOL_1200_8865 = KAL_FALSE;
static kal_bool DISABLE_DVDD_ON_FIRST_CHECK = KAL_FALSE;
#define GPIO_CAMERA_MCLK (GPIO122|0x80000000)
extern void ISP_MCLK1_EN(bool En);

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
                        GPIO_OUT_ZERO,
                        GPIO_OUT_ONE,
                    }
                   };

  if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
      pinSetIdx = 0;
  }
  else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
      pinSetIdx = 1;
  }
  else if (DUAL_CAMERA_MAIN_2_SENSOR == SensorIdx) {
      //pinSetIdx = 2;
  }

  //power ON
  if (On) {
    //power ON-----------------------------------------------------------------------//
    #if 0  //TODO: depends on HW layout. Should be notified by SA.
    
      printk("Set CAMERA_POWER_PULL_PIN for power \n");
      if (mt_set_gpio_pull_enable(GPIO_CAMERA_LDO_EN_PIN, GPIO_PULL_DISABLE)) {PK_DBG("[[CAMERA SENSOR] Set CAMERA_POWER_PULL_PIN DISABLE ! \n"); }
      if(mt_set_gpio_mode(GPIO_CAMERA_LDO_EN_PIN, GPIO_CAMERA_LDO_EN_PIN_M_GPIO)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN mode failed!! \n");}
      if(mt_set_gpio_dir(GPIO_CAMERA_LDO_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN dir failed!! \n");}
      if(mt_set_gpio_out(GPIO_CAMERA_LDO_EN_PIN,GPIO_OUT_ONE)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN failed!! \n");}
    #endif
    if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
    if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
    if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
    if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
    if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
    if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
    //mdelay(5);
   //disable MCLK
	   mt_set_gpio_mode(GPIO_CAMERA_MCLK,GPIO_MODE_00);		 		
    mt_set_gpio_dir(GPIO_CAMERA_MCLK,GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CAMERA_MCLK,GPIO_OUT_ZERO);
    //ISP_MCLK1_EN(false);
    PK_DBG("[CAMERA SENSOR] kdCISModulePowerOn -on:currSensorName=%s,SensorIdx=%d pinSetIdx=%d\n",currSensorName,SensorIdx,pinSetIdx);
    PK_DBG("[CAMERA SENSOR] mytest kdCISModulePowerOn ENABLE_DVDD_VOL_1500_8825=%d ENABLE_DVDD_VOL_1200_8865=%d\n",ENABLE_DVDD_VOL_1500_8825,ENABLE_DVDD_VOL_1200_8865);
    //if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx)
    //if (currSensorName  &&  (0 == strcmp(SENSOR_DRVNAME_OV8825_MIPI_RAW,currSensorName)))
    if ((DUAL_CAMERA_MAIN_SENSOR == SensorIdx) && currSensorName  &&  (0 == strcmp(SENSOR_DRVNAME_OV8825_MIPI_RAW,currSensorName)))
    {
     //PK_DBG("SENSOR_DRVNAME_OV8825_MIPI_RAW power on sensorIdx:%d \n",SensorIdx);
      //dovdd
      if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }

      mdelay(5);
      //avdd
      if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
         //return -EIO;
         goto _kdCISModulePowerOn_exit_;
      }
      //dvdd
      mdelay(5);
      if(ENABLE_DVDD_VOL_1500_8825)
      {
        PK_DBG("[CAMERA SENSOR] kdCISModulePoweron CAMERA_POWER_VCAM_D:ENABLE_DVDD_VOL_1500_8825=%d\n",ENABLE_DVDD_VOL_1500_8825);

      if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name))
      {
         PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }
      }
      mdelay(5);

      if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      } 
      msleep(5);
      if(TRUE != hwPowerOn(MT65XX_POWER_LDO_VGP5, VOL_1800,mode_name))
      //PDN/STBY pin
      {
          PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
      }
      mdelay(2);

      if (GPIO_CAMERA_INVALID != pinSet[1-pinSetIdx][IDX_PS_CMRST]) {
              if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
              if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
              if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
              if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
              if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
              mdelay(3);
              if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
              if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
      }

      mdelay(10);
      if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
      if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
      if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
      mdelay(3);

      //clock
      mt_set_gpio_mode(GPIO_CAMERA_MCLK,GPIO_MODE_01);
      mdelay(3);
      ISP_MCLK1_EN(true);

      //RST pin
      if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
        if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
        if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
        //if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
        mdelay(3);
        if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
        mdelay(1);
      }

      mdelay(10);
      //PWDN pin is high
      //reset pin is low
    }
    //lenovo.sw wangsx3 20140515 add main backup sensor ov8865
    else if ((DUAL_CAMERA_MAIN_SENSOR == SensorIdx) && currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV8865_MIPI_RAW,currSensorName)))
    //else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV8865_MIPI_RAW,currSensorName)))    
    {
      PK_DBG("[CAMERA SENSOR] SENSOR_DRVNAME_OV8865_MIPI_RAW power on sensorIdx:%d \n",SensorIdx);
      if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
      if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
      if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}

      if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
      if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
      if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
      mdelay(2);
     //AVDD
      if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }
      mdelay(1);
     //DOVDD
      if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }
      mdelay(1);
      //PDW
      //PDN/STBY pin
      if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
     // printk("[CAMERA SENSOR]set pwrdwn pin hight\n");
      mdelay(1);
     //DVDD
      #if !defined(MAIN_CAM_DVDD_EXTERNAL_LDO)
      PK_DBG("[CAMERA SENSOR] main cam power up internal DVDD ENABLE_DVDD_VOL_1200_8865= %d\n",ENABLE_DVDD_VOL_1200_8865);
      if(ENABLE_DVDD_VOL_1200_8865)
      {
        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
        {
          PK_DBG("[CAMERA SENSOR][CAMERA SENSOR] Fail to enable analog power\n");
          //return -EIO;
          goto _kdCISModulePowerOn_exit_;
        }
       }
      #else
        PK_DBG("[CAMERA SENSOR]main cam power up external DVDD\n");
        mt_set_gpio_mode(GPIO_CAMERA_LDO_EN_PIN, 0);
        mt_set_gpio_dir(GPIO_CAMERA_LDO_EN_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CAMERA_LDO_EN_PIN, GPIO_OUT_ONE);
      #endif
     //AF VDD
      if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }

      //PWD
      //disable inactive sensor,sub sensor
      //PWDN pin is high
      if(mt_set_gpio_mode(GPIO_CAMERA_CMPDN1_PIN,GPIO_CAMERA_CMPDN1_PIN_M_GPIO)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}        
      if(mt_set_gpio_dir(GPIO_CAMERA_CMPDN1_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}       
      if(mt_set_gpio_out(GPIO_CAMERA_CMPDN1_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}      
      //PK_DBG("sub sensor powerdone pin is high\n"); 
     //RST
      //reset pin is low
      if(mt_set_gpio_mode(GPIO_CAMERA_CMRST1_PIN,GPIO_CAMERA_CMRST1_PIN_M_GPIO)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}        
      if(mt_set_gpio_dir(GPIO_CAMERA_CMRST1_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}     
      if(mt_set_gpio_out(GPIO_CAMERA_CMRST1_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}      
      //PK_DBG("sub sensor reset pin is low\n"); 

      mdelay(10); //wait sub sensor correctly reseted

      //Enable main camera
      //XSHUTDOWN pin
      if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
      mdelay(5);

      //clock
      mt_set_gpio_mode(GPIO_CAMERA_MCLK,GPIO_MODE_01);
      mdelay(3);
      ISP_MCLK1_EN(true);
      
    }
    //else if(DUAL_CAMERA_SUB_SENSOR == SensorIdx)
    //else if ((DUAL_CAMERA_SUB_SENSOR == SensorIdx) && currSensorName && (0 == strcmp(SENSOR_DRVNAME_MT9V113_MIPI_YUV,currSensorName)))
    else if ((DUAL_CAMERA_SUB_SENSOR == SensorIdx) &&currSensorName && (0 == strcmp(SENSOR_DRVNAME_MT9V113_MIPI_YUV,currSensorName)))
    {
      //PK_DBG("SENSOR_DRVNAME_MT9V113_MIPI_YUV power on sensorIdx:%d \n",SensorIdx);
      //PWDN pin is high
      if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
      if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
      if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
      mdelay(1);

      if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
      if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
      if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
      //PK_DBG("reset pin is high\n");

      //DOVDD
      //PK_DBG("IOVDD is 1.8v \n");
      //PK_DBG("[ON_general 1.8V]sensorIdx:%d \n",SensorIdx);
      if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
        //return -EIO;
        //goto _kdCISModulePowerOn_exit_;
      }
      mdelay(1);
       
      //AVDD
      //PK_DBG("AVDD is 2.8v \n");
      if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
        //return -EIO;
        //goto _kdCISModulePowerOn_exit_;
      }
      mdelay(1);
      mdelay(1);

      //DVDD
      //PK_DBG("DVDD is 1.5v \n");
      if(TRUE != hwPowerOn(MT65XX_POWER_LDO_VGP5, VOL_1800,mode_name))        
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
        //return -EIO;
        //goto _kdCISModulePowerOn_exit_;
      }
      mdelay(2);
      mdelay(1);

      mt_set_gpio_mode(GPIO_CAMERA_MCLK,GPIO_MODE_01);

      mdelay(3);
      ISP_MCLK1_EN(true);
       
      //PWDN pin is low
      if (GPIO_CAMERA_INVALID != pinSet[1-pinSetIdx][IDX_PS_CMRST]) {
        if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
        if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
        if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
        if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
        if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
        if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
      }
      //disable inactive sensor
      //reset pin is low
      if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
      if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
      if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
      //PK_DBG("main sensor reset pin is low\n");
      
      //PWDN pin is low
      if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
        if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
        if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
        if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
        mdelay(3);
        if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
        mdelay(1);
      }
      mdelay(10);
  }
  }
  else {//power OFF
    //8825 power off
      PK_DBG("kdCISModulePoweroff:currSensorName=%s,SensorIdx=%d\n",currSensorName,SensorIdx);
          mt_set_gpio_mode(GPIO_CAMERA_MCLK,GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_CAMERA_MCLK,GPIO_DIR_OUT);
      mt_set_gpio_out(GPIO_CAMERA_MCLK,GPIO_OUT_ZERO);
      //ISP_MCLK1_EN(false);

    //if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx)
    if((DUAL_CAMERA_MAIN_SENSOR == SensorIdx) && currSensorName  &&  (0 == strcmp(SENSOR_DRVNAME_OV8825_MIPI_RAW,currSensorName)))
    //if(currSensorName  &&  (0 == strcmp(SENSOR_DRVNAME_OV8825_MIPI_RAW,currSensorName)))      
    {//power OFF OV8825
      //PK_DBG("kdCISModulePoweroff:SENSOR_DRVNAME_OV8825_MIPI_RAW\n");
      //reset pull down
      if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
        if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
        if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
        if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
      }
      PK_DBG("kdCISModulePoweroff CAMERA_POWER_VCAM_D:ENABLE_DVDD_VOL_1200_8865=%d,ENABLE_DVDD_VOL_1500_8825=%d DISABLE_DVDD_ON_FIRST_CHECK=%d\n",ENABLE_DVDD_VOL_1200_8865,ENABLE_DVDD_VOL_1500_8825,DISABLE_DVDD_ON_FIRST_CHECK);
      //first time,ENABLE_DVDD_VOL_1500_8825=1,but CAMERA_POWER_VCAM_D is off
      if(ENABLE_DVDD_VOL_1500_8825 && DISABLE_DVDD_ON_FIRST_CHECK)
      {
        PK_DBG("kdCISModulePoweroff CAMERA_POWER_VCAM_D:ENABLE_DVDD_VOL_1200_8865=%d,ENABLE_DVDD_VOL_1500_8825=%d\n",ENABLE_DVDD_VOL_1200_8865,ENABLE_DVDD_VOL_1500_8825);
  
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
          PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
          //return -EIO;
          goto _kdCISModulePowerOn_exit_;
        }
      }
      else
        DISABLE_DVDD_ON_FIRST_CHECK=KAL_TRUE;

      if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
        PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }

      if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }   

      if(TRUE != hwPowerDown(MT65XX_POWER_LDO_VGP5,mode_name)) 
      {
        PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      } 

      //PDN pull down
      //reset pull down

      //AF
      if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }

      if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
        if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
        if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
        if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
        //interface
        //return -EIO;
      }
      //analog
      //return -EIO;
    }
    else if((DUAL_CAMERA_MAIN_SENSOR == SensorIdx) && currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV8865_MIPI_RAW,currSensorName)))
   //else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV8865_MIPI_RAW,currSensorName)))
    {//power OFF OV8865
      //msleep(3);

      //PK_DBG("kdCISModulePoweroff:SENSOR_DRVNAME_OV8865_MIPI_RAW\n");
      //PK_DBG("kdCISModulePoweroff:SENSOR_DRVNAME_OV8865_MIPI_RAW,SensorIdx=%d\n",SensorIdx);
      //reset pull down
      if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
      if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
      if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor

      //AF
      if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }

      #if !defined(MAIN_CAM_DVDD_EXTERNAL_LDO)
      PK_DBG("[CAMERA SENSOR] main cam power off internal DVDD ENABLE_DVDD_VOL_1200_8865= %d\n",ENABLE_DVDD_VOL_1200_8865);
      if(ENABLE_DVDD_VOL_1200_8865)
      {
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D,mode_name))
        {
          PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
          //return -EIO;
          goto _kdCISModulePowerOn_exit_;
        }
      }
      #else
      mt_set_gpio_out(GPIO_CAMERA_LDO_EN_PIN, GPIO_OUT_ZERO);
      #endif
      //analog
      if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
        PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }

      //interface
      if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }

      //PDN pull down
      if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
      
    }
    //else if(DUAL_CAMERA_SUB_SENSOR == SensorIdx)
    //else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_MT9V113_MIPI_YUV,currSensorName)))
    else if((DUAL_CAMERA_SUB_SENSOR == SensorIdx) && currSensorName && (0 == strcmp(SENSOR_DRVNAME_MT9V113_MIPI_YUV,currSensorName)))
    //else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_MT9V113_MIPI_YUV,currSensorName)))      
    {
      //PK_DBG("kdCISModulePoweroff:SENSOR_DRVNAME_MT9V113_MIPI_YUV SensorIdx=%d\n",SensorIdx);
      //PK_DBG("kdCISModulePoweroff:SENSOR_DRVNAME_MT9V113_MIPI_YUV\n");
      //PWDN pin is high
      if(mt_set_gpio_mode(GPIO_CAMERA_CMPDN1_PIN,GPIO_CAMERA_CMPDN1_PIN_M_GPIO)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
      if(mt_set_gpio_dir(GPIO_CAMERA_CMPDN1_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
      if(mt_set_gpio_out(GPIO_CAMERA_CMPDN1_PIN,GPIO_OUT_ONE)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
      //PK_DBG("powerdone pin is high\n");

      //reset pin is low
      if(mt_set_gpio_mode(GPIO_CAMERA_CMRST1_PIN,GPIO_CAMERA_CMRST1_PIN_M_GPIO)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}        
      if(mt_set_gpio_dir(GPIO_CAMERA_CMRST1_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}     
      if(mt_set_gpio_out(GPIO_CAMERA_CMRST1_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}      
     // PK_DBG("reset pin is low\n");

      if(TRUE != hwPowerDown(MT65XX_POWER_LDO_VGP5,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }
      if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }
      if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2, mode_name))
      {
        PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
        //return -EIO;
        goto _kdCISModulePowerOn_exit_;
      }
    }//
  }


  return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;
}

EXPORT_SYMBOL(kdCISModulePowerOn);
