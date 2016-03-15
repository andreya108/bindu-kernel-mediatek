
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
 */   
/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.c
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 * Author:
 * -------
 *   PC Huang (MTK02204)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
//#include <windows.h>
//#include <memory.h>
//#include <nkintr.h>
//#include <ceddk.h>
//#include <ceddk_exp.h>

//#include "kal_release.h"
//#include "i2c_exp.h"
//#include "gpio_exp.h"
//#include "msdk_exp.h"
//#include "msdk_sensor_exp.h"
//#include "msdk_isp_exp.h"
//#include "base_regs.h"
//#include "Sensor.h"
//#include "camera_sensor_para.h"
//#include "CameraCustomized.h"

//s_porting add
//s_porting add
//s_porting add
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/xlog.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <asm/system.h>


#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"


#include "s5k8aayxmipi_Sensor.h"
#include "s5k8aayxmipi_Camera_Sensor_para.h"
#include "s5k8aayxmipi_CameraCustomized.h"

#define S5K8AAYX_MIPI_DEBUG
#ifdef S5K8AAYX_MIPI_DEBUG
//#define SENSORDB(fmt, arg...) xlog_printk(ANDROID_LOG_DEBUG, "[S5K8AAYXMIPI]", fmt, ##arg)
#define SENSORDB printk
  
#else
#define SENSORDB(x,...)
#endif

#define S5K8AAYX_TEST_PATTERN_CHECKSUM (0x7d732767)

typedef struct
{
  UINT16  iSensorVersion;
  UINT16  iNightMode;
  UINT16  iWB;
  UINT16  iEffect;
  UINT16  iEV;
  UINT16  iBanding;
  UINT16  iMirror;
  UINT16  iFrameRate;
} S5K8AAYX_MIPIStatus;
S5K8AAYX_MIPIStatus S5K8AAYX_MIPICurrentStatus;

static DEFINE_SPINLOCK(s5k8aayxmipi_drv_lock);

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

//static int sensor_id_fail = 0; 
static kal_uint32 zoom_factor = 0; 


kal_uint16 S5K8AAYX_MIPI_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
	char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
	iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,2,S5K8AAYX_MIPI_WRITE_ID);
	return (kal_uint16)(((get_byte<<8)&0xff00)|((get_byte>>8)&0x00ff));

}

inline void S5K8AAYX_MIPI_write_cmos_sensor(u16 addr, u32 para)
{
   char puSendCmd[4] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,(char)(para >> 8),(char)(para & 0xFF)};

   iWriteRegI2C(puSendCmd , 4,S5K8AAYX_MIPI_WRITE_ID);
}


/*******************************************************************************
* // Adapter for Winmo typedef 
********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT


/*******************************************************************************
* // End Adapter for Winmo typedef 
********************************************************************************/
/* Global Valuable */
kal_bool S5K8AAYX_MIPI_MPEG4_encode_mode = KAL_FALSE, S5K8AAYX_MIPI_MJPEG_encode_mode = KAL_FALSE;
static kal_bool S5K8AAYX_MIPI_VEDIO_encode_mode = KAL_FALSE; 
static kal_bool S5K8AAYX_MIPI_sensor_cap_state = KAL_FALSE; 
kal_uint32 S5K8AAYX_MIPI_PV_dummy_pixels=0,S5K8AAYX_MIPI_PV_dummy_lines=0,S5K8AAYX_MIPI_isp_master_clock=0;
static kal_uint32  S5K8AAYX_MIPI_sensor_pclk=920;
static kal_bool S5K8AAYX_MIPI_AE_ENABLE = KAL_TRUE; 

MSDK_SENSOR_CONFIG_STRUCT S5K8AAYX_MIPISensorConfigData;


/*************************************************************************
* FUNCTION
*	S5K8AAYX_MIPIInitialPara
*
* DESCRIPTION
*	This function initialize the global status of  MT9V114
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void S5K8AAYX_MIPIInitialPara(void)
{
  spin_lock(&s5k8aayxmipi_drv_lock);
  S5K8AAYX_MIPICurrentStatus.iNightMode = 0;
  S5K8AAYX_MIPICurrentStatus.iWB = AWB_MODE_AUTO;
  S5K8AAYX_MIPICurrentStatus.iEffect = MEFFECT_OFF;
  S5K8AAYX_MIPICurrentStatus.iBanding = AE_FLICKER_MODE_50HZ;
  S5K8AAYX_MIPICurrentStatus.iEV = AE_EV_COMP_00;
  S5K8AAYX_MIPICurrentStatus.iMirror = IMAGE_NORMAL;
  S5K8AAYX_MIPICurrentStatus.iFrameRate = 0;
  spin_unlock(&s5k8aayxmipi_drv_lock);
}


void S5K8AAYX_MIPI_set_mirror(kal_uint8 image_mirror)
{

		if(S5K8AAYX_MIPICurrentStatus.iMirror == image_mirror)
		  return;

		S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);
		S5K8AAYX_MIPI_write_cmos_sensor(0x002a, 0x01E8); 
	
		switch (image_mirror)  
		{
			case IMAGE_NORMAL:
				S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); 	// REG_0TC_PCFG_uPrevMirror
				S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); 	// REG_0TC_PCFG_uCaptureMirror
				break;
			case IMAGE_H_MIRROR:
				S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); 	// REG_0TC_PCFG_uPrevMirror
				S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); 	// REG_0TC_PCFG_uCaptureMirror
				break;
			case IMAGE_V_MIRROR:
				S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); 	// REG_0TC_PCFG_uPrevMirror
				S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); 	// REG_0TC_PCFG_uCaptureMirror
				break;
			case IMAGE_HV_MIRROR:
				S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); 	// REG_0TC_PCFG_uPrevMirror
				S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); 	// REG_0TC_PCFG_uCaptureMirror
				break;
				
			default:
				ASSERT(0);
				break;
		}
	
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01A8); 
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01AC);
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001); // #REG_TC_GP_PrevOpenAfterChange
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01A6); 
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);  // #REG_TC_GP_NewConfigSync // Update preview configuration
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01AA); 
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);  // #REG_TC_GP_PrevConfigChanged
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x019E); 
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);  // #REG_TC_GP_EnablePreview // Start preview
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);  // #REG_TC_GP_EnablePreviewChanged

		spin_lock(&s5k8aayxmipi_drv_lock);
		S5K8AAYX_MIPICurrentStatus.iMirror = image_mirror;
		spin_unlock(&s5k8aayxmipi_drv_lock);

}





/*****************************************************************************
 * FUNCTION
 *  S5K8AAYX_MIPI_set_dummy
 * DESCRIPTION
 *
 * PARAMETERS
 *  pixels      [IN]
 *  lines       [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void S5K8AAYX_MIPI_set_dummy(kal_uint16 dummy_pixels, kal_uint16 dummy_lines)
{
		/****************************************************
		  * Adjust the extra H-Blanking & V-Blanking.
		  *****************************************************/
		S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000); 
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x044C); 
		
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, dummy_pixels); 
		//S5K8AAYX_MIPI_write_cmos_sensor(0x0F1C, dummy_pixels); 	// Extra H-Blanking
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, dummy_lines); 	// Extra V-Blanking
}   /* S5K8AAYX_MIPI_set_dummy */

/*****************************************************************************
 * FUNCTION
 *  S5K8AAYX_MIPI_Initialize_Setting
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void S5K8AAYX_MIPI_Initialize_Setting(void)
{
/******
	Below two groups of setting is supply by Coasia and AVP, for MIPI EVT1 Version,
	they both can work on my module on MT6577+JB sw.

	we could try those two groups of setting on MT6588 .
*******/

#if 0  //Supply by Coasia ,verification ok on mt6577
	S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000);//ADD TEST


	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	// Reset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// Simmian bug workaround
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1030);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// Clear host interrupt so main will wait
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0014);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	// ARM go

#ifdef MIPI_INTERFACE
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x2470);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xB510);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF9ED);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF9E9);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6341);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF9E2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF9DE);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF9DA);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6448);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBC10);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBC08);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4718);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x27CC);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8EDD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2744);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8725);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x26E4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2638);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA6EF);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2604);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA0F1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x25D0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x058F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x24E4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x403E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F86);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DC);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x200A);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE28D);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0E3F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DB);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F86);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5DD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C3);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0027);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5DD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02E0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x12D4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10B8);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5C0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE28D);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3403);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE182);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC2A8);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2080);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE08C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE7B4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x039E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE80F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3E0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4624);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE00E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x47B4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE280);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC084);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE08C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x47B4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1DC);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0493);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4624);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE00E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x47B4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1CC);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC8B4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x039C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE003);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3623);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE00E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x38B4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE280);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE281);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFE7);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBAFF);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x403E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AB);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0248);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE310);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1234);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0DB2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0214);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE594);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE584);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4070);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0800);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0820);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4041);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE280);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x11B8);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x51B6);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE041);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0094);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1D11);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008D);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x11C0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x21A8);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3FB0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE353);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x31A4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5BB2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C3);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE085);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xCBB4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C3);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1DBC);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3EB4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2EB2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0193);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0092);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2811);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0194);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0092);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x11A1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0072);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1160);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02B4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4070);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2148);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x14B0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE311);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3110);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5C3);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D3);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3C1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x110C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x04B0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x41F0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC801);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC82C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1801);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1821);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4008);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x500C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3005);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x60A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D6);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B8);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x05B4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x70AC);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10F4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D6);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x26B0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D7);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0044);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x26B0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D7);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10F6);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D6);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D5);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C5);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x41F0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE594);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0040);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2068);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0054);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1005);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0032);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE584);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE594);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0030);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE584);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFF9);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEAFF);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x28E8);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3370);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1272);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1728);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x112C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x28EC);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x122C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF200);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2340);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0E2C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF400);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0CDC);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x20D4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x06D4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4778);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x46C0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC091);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0467);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2FA7);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xCB1F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x058F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA0F1);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF004);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE51F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD14C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B43);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8725);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6777);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8E49);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8EDD);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x96FF);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                             
	                                                                        
	//============================================================          
	// Set IO driving current                                               
	//============================================================          
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x04B4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0155); // d0~d4                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0155); // d5~d9                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1555); // gpio1~gpio3              
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0555); // HSYNC,VSYNC,PCLK,SCL,SDA 
	                                                                        
	//============================================================          
	// Analog Settings                                                      
	//============================================================          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E38);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0476);	//senHal_RegCompBiasNormSf //CDS bias
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0476);	//senHal_RegCompBiasYAv //CDS bias
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0AA0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	//setot_bUseDigitalHbin //1-Digital, 0-Analog
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E2C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	//senHal_bUseAnalogVerAv //2-Adding/averaging, 1-Y-Avg, 0-PLA
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E66);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	//senHal_RegBlstEnNorm      
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1250);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); 	//senHal_Bls_nSpExpLines  
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1202);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); 	//senHal_Dblr_VcoFreqMHZ  
	                                                                        
	//ADLC Filter                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1288);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F);	//gisp_dadlc_ResetFilterValue
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1C02);	//gisp_dadlc_SteadyFilterValue
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006);	//gisp_dadlc_NResetIIrFrames

#else
	// T&P part
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x2460);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xB510);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFB71);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFB6D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFB69);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFB65);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6241);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x490C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6341);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBC10);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBC08);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4718);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2998);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x27B7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2690);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x25DD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2650);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9DC7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x24C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x49CD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2A65);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2A8F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4070);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x200D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x20B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0B01);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x44F0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10FB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC4EC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1DC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0C02);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE311);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x200C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x20B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x24CC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE151);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10FA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x14B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE4B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x200C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x11B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x35B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1DE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE082);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE155);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x50B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE083);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3C05);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE283);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE082);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x50B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE082);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE152);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x200D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x35B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1DE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE082);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE155);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE0B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1D0F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE281);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x200E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE082);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE0B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x200E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE082);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE152);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10FC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0022);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10F9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE150);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x20F9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0018);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE28C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x13FC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x08B8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x08BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x13F0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0150);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4070);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0150);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4070);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4070);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x40FF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE200);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0CC1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0147);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE354);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1390);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0C01);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0388);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4FF8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xB36C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x80B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1CB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7340);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA0B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE355);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0137);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE355);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1344);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0344);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x18BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1340);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x19B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x133C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x933C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE155);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE045);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4C02);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE289);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6C01);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE289);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0019);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xCA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE355);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE355);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE355);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0034);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0C02);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE240);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE250);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xB2EC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE355);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02DC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x82B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x22D8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x20BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE380);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x08B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x08BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0040);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE380);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x08BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x87B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x87B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x83B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1CB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE355);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE155);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE355);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0DBA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0DBA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA9BC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA9BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x89BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x024C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA4B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4FF8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE58D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FFA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xB100);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xB0B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x082B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA2B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x08B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x09B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x87B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x87B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x85BC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x83B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA4B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x11E0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE155);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE281);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE155);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA6B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA6B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2D16);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x25BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x202A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE242);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x26B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2AB4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x21AC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2ABC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2F96);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE242);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2ABE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2DBA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2006);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3C2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2DBA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA9BC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA9BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x89BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE155);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE006);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE155);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0174);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE0B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE355);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F47);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3E0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE090);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0026);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FBE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1154);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x05B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0148);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1CBA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2CBE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3DB2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE042);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE08C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4C05);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE28C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4055);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE284);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4DB6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1EBA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2EBE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3FB2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1C02);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE28C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10AA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE281);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1FB6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1110);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0110);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1BBA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1BBC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0104);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x100C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x19BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1008);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1AB2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0C01);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE280);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xABBA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0C03);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE240);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x81BC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x82BC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF9A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEAFF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF97);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEAFF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF94);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEAFF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xCA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2F47);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3E0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE091);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0C02);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE580);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE580);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEAFF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1760);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1718);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x057C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A86);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0E2C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x20C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x187C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC100);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x232C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3600);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3333);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1200);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2107);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0116);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F88);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x042C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x031D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0834);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xB000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0203);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F2C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0555);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF500);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x102C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xB510);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF8AF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x482C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7F00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2801);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD00A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x482B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8940);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x07C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD006);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4828);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2180);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3820);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8B02);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4828);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF8A8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBC10);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBC08);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4718);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xB5F8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF8AA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4821);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7F00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2801);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD03C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4820);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8940);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x07C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD038);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x481D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2218);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3020);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2512);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5E82);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5F45);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1B51);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x17CB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F9B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1859);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x108B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x261E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5F86);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1AB1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x17CC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FA4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1861);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x108C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4917);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8849);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1949);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x18CD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4916);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x810D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4D16);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x886F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8AC5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x197F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x814F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4F15);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x887F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x197D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x18EB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x818B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4B13);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x885B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x189B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x81CB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4B12);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x885B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x189A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1912);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x820A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4A11);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8B80);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8852);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1812);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x824A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4A0F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8852);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1810);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1900);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8288);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x480E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8840);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1980);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x82C8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBCF8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBC08);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4718);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x112C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF402);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F94);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF4A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F98);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F9C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FA0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FA4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FA8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FAC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FB0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4778);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x46C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBF01);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2E89);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1DEF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5137);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E0F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2EA7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x25AD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x055F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x256B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2D27);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4778);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x46C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA5ED);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4778);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x46C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2EA7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4778);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x46C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2AF5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8E66);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x04B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0155); // d0~d4
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0155); // d5~d9
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1555); // gpio1~gpio3
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0555); // HSYNC,VSYNC,PCLK,SCL,SDA
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E8E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E92);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0693);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E96);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E9A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0693);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E9E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EA2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0693);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EA6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EAA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0690);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EAE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EB2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0690);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EB6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EBA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x025C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EBE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EC2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x025C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EC6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0ECA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0698);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0ECE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0ED2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0698);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0ED6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EDA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EDE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EE2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EE6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EEA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EEE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EF2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EF6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EFA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EFE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F02);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F06);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F0A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F0E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F12);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F16);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F1A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F22);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F26);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F2A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0205);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F2E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0268);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F32);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x068E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F36);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F3A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F3E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F42);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F46);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F4A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F4E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F52);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F56);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F5A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0055);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F5E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F62);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F66);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F6A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0208);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F6E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F72);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F76);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F7A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0205);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F7E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02C8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F82);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x068E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F86);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F8A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F8E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F92);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F96);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F9A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0142);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F9E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FA2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0208);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FA6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03BB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FAA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x04AB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FAE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x059B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FB2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0691);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FB6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FBA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0262);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FBE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FC2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FC6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FCA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FCE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FD2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FD6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0062);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FDA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0691);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FDE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FE2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FE6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x029E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FEA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0691);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FEE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FF2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FF6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FFA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0202);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FFE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02CA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x068B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1006);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x100A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x100E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1012);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1016);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x101A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x101E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02CA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1022);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x065C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1026);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x102A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x102E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1032);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1036);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x103A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0205);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x103E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1042);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1046);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02C8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x104A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x068E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x104E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1052);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1056);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x105A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x105E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0205);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1062);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02B8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1066);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x068E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x106A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x106E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1072);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0208);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1076);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0210);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x107A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x107E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1082);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1086);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x108A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x108E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0214);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1092);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1096);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x109A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x109E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10A2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10A6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10AA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10AE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0208);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x065F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0691);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10C2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10C6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10CA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10CE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10D6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10DA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10DE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10E2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10E6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10EA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10EE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10F2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10F6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10FA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10FE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1106);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x110A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x110E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1112);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x069E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1116);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0708);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x111A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x111E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1122);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1126);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x027D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x112A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02C8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x112E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0708);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1132);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E90);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E94);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0663);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E98);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E9C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0332);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EA0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0335);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EA4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0663);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EA8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EAC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EB0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x033D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EB4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0660);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EB8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EBC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0153);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EC0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0436);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EC4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0487);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EC8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0ECC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0334);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0ED0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x033B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0ED4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0668);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0ED8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0368);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EDC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0337);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EE0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0034);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EE4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EE8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03B5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EEC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x038D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EF0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0365);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EF4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x033A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EF8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0EFC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0059);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0031);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F04);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F08);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F0C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0390);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F10);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0084);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F14);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0056);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F18);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0369);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0336);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F20);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0035);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F24);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F28);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F2C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F30);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F34);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0430);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F38);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0493);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F3C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x065E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F40);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F44);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F48);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F4C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F50);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0340);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F54);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0386);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F58);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F5C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0055);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F60);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0340);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F64);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0389);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F68);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F6C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F70);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F74);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0433);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F78);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F7C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F80);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F84);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F88);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03B3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F8C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0430);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F90);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x04C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F94);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x065E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F98);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0F9C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FA0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x025B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FA4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FA8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03F1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FAC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0433);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FB0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x058F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FB4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0661);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FB8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0104);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FBC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0159);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FC0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0438);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FC4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x043E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FC8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FCC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FD0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FD4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FD8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0062);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FDC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FE0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0396);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FE4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0661);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FE8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0139);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FEC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FF0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x046D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FF4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0661);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FF8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0FFC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0327);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1008);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03B5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x100C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x042D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x04C3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1014);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x065B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1018);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0081);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x101C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1020);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1024);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02F8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1028);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03B5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x102C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1030);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x04C3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1034);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x062C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1038);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x103C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1040);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1044);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1048);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x104C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1050);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03B3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1054);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0430);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1058);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x04C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x105C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x065E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1060);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1064);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1068);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0430);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x106C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x04A1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1070);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x065E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1074);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1078);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x107C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1080);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x033D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1084);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0433);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1088);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0443);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x108C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0107);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1090);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0117);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1094);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0335);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1098);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0345);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x109C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x043B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x044B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10A4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10A8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10AC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02FB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10B8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10BC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0401);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0433);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x062F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10C8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0661);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10CC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10D8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10DC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10E0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10E4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10E8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10EC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10F0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10F4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10F8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x10FC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1100);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1104);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1108);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x110C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1110);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1114);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x066C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1118);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x06DF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x111C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1120);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1124);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1128);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x04AF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x112C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x04C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1130);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x06DF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1134);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1186);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0404);	// Fix ptr type for 74+75
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0C0C);	// Fix ptr type for 76+77
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// Fix ptr type for 78+79
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// Fix ptr type for 80+81
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x11F0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x026C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1136);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02FF);	// Single to double conversion threshold - need LSB of "1" to apply 4-sampling in new T&P
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x05FF);	// Single to double conversion threshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1224);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008);	// Single offset
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1230);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1234);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0090);	// Double offset factor
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x122A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1236);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x123A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0090);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E4C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1110);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E5C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F4);	// Clamp Level - temporary pending check of black sun
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E6C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F);	// All caps. Turn off limiter
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F);	// All caps. Turn off limiter
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E34);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013);	// Reduce Pixel boost current
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013);	// Reduce Pixel boost current
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0474);	// CDS bias
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0474);	// CDS bias
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E5E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	// Enable CMP_PD during Vblank
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1208);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0276);	// 630mV ADC_SAT
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E52);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8999);	// VPIX ~ 3.15V
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0779);	// 3 Charge pump capacitors
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1202);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008);	// Charge Pump frequency
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x11FE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// External calibration of DBLR clock
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1204);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// No DBLR clock offset, for low frequency
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0AA0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	// 1 - Digital, 0 - Analog
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E2C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	// 1 - Y-Avg, 0- PLA
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1358);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006C);	// Analog - Min DCLK 108MHz for optimal performance at 54MHz clk
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E66);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	// Enable anti-blooming shutter
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	// Enable anti-blooming shutter
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	// Enable anti-blooming shutter
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1250);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF);
#endif

	//ae
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0D46);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0440);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3CF0);	//lt_uMaxExp_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0444);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6590);	//lt_uMaxExp_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0448);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBB80);	//lt_uMaxExp_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x044C);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3880);	//lt_uMaxExp_3_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0450);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3CF0);	//lt_uCapMaxExp_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0454);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6590);	//lt_uCapMaxExp_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0458);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBB80);	//lt_uCapMaxExp_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x045C);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3880);	//lt_uCapMaxExp_3_   
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0460);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0190);	//lt_uMaxAnGain_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0280);	//lt_uMaxAnGain_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0540);	//lt_uMaxAnGain_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0B00); // max gain 0x0C00=> 12x  ;0x0BO0=>  11x     lt_uMaxAnGain_3_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //lt_uMaxDigGain
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3000); //lt_uMaxTotGain
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x042E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010E); //lt_uLimitHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F5); //lt_uLimitLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0DE0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);	//ae_Fade2BlackEnable  F2B off, F2W on
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0D40);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003E); //TVAR_ae_BrAve
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0D4E); //AE_Weight
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0303);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0303);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0403);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0304);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0403);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0304);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0403);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0304);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0303);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0303);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0202);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0202);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1326);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  //gisp_gos_Enable
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x063A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[0][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[0][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[0][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B4);  // #TVAR_ash_GASAlpha[0][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B4);  // #TVAR_ash_GASAlpha[1][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[1][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[1][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D2);  // #TVAR_ash_GASAlpha[1][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B4);  // #TVAR_ash_GASAlpha[2][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[2][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[2][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D2);  // #TVAR_ash_GASAlpha[2][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B4);  // #TVAR_ash_GASAlpha[3][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[3][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[3][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DC);  // #TVAR_ash_GASAlpha[3][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F5);  // #TVAR_ash_GASAlpha[4][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8);  // #TVAR_ash_GASAlpha[4][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8);  // #TVAR_ash_GASAlpha[4][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[4][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0109);  // #TVAR_ash_GASAlpha[5][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[5][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[5][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[5][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F0);  // #TVAR_ash_GASAlpha[6][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[6][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[6][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[6][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x067A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064);  // #TVAR_ash_GASBeta[0][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014);  // #TVAR_ash_GASBeta[0][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014);  // #TVAR_ash_GASBeta[0][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[0][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E);  // #TVAR_ash_GASBeta[1][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);  // #TVAR_ash_GASBeta[1][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);  // #TVAR_ash_GASBeta[1][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[1][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E);  // #TVAR_ash_GASBeta[2][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);  // #TVAR_ash_GASBeta[2][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);  // #TVAR_ash_GASBeta[2][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[2][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E);  // #TVAR_ash_GASBeta[3][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[3][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[3][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[3][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046);  // #TVAR_ash_GASBeta[4][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[4][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[4][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[4][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0032);  // #TVAR_ash_GASBeta[5][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[5][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[5][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[5][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0055);  // #TVAR_ash_GASBeta[6][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[6][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[6][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[6][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  //ash_bLumaMode
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0632);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F5); //TVAR_ash_CGrasAlphas_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8); //TVAR_ash_CGrasAlphas_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8); //TVAR_ash_CGrasAlphas_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //TVAR_ash_CGrasAlphas_3_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0672);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //TVAR_ash_GASOutdoorAlpha_0_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //TVAR_ash_GASOutdoorAlpha_1_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //TVAR_ash_GASOutdoorAlpha_2_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //TVAR_ash_GASOutdoorAlpha_3_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //ash_GASOutdoorBeta_0_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //ash_GASOutdoorBeta_1_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //ash_GASOutdoorBeta_2_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //ash_GASOutdoorBeta_3_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0624);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009a);  //TVAR_ash_AwbAshCord_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00d3);  //TVAR_ash_AwbAshCord_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00d4);  //TVAR_ash_AwbAshCord_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012c);  //TVAR_ash_AwbAshCord_3_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0162);  //TVAR_ash_AwbAshCord_4_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0190);  //TVAR_ash_AwbAshCord_5_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01a0);  //TVAR_ash_AwbAshCord_6_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06CC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0280);  //ash_uParabolicCenterX	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E0);  //ash_uParabolicCenterY	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000D);  //ash_uParabolicScalingA	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F);  //ash_uParabolicScalingB	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06C6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  //ash_bParabolicEstimation
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x347C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0254); //Tune_wbt_GAS_0_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B3); //Tune_wbt_GAS_1_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0163); //Tune_wbt_GAS_2_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0120); //Tune_wbt_GAS_3_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EE); //Tune_wbt_GAS_4_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CC); //Tune_wbt_GAS_5_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BE); //Tune_wbt_GAS_6_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C5); //Tune_wbt_GAS_7_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E1); //Tune_wbt_GAS_8_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0113); //Tune_wbt_GAS_9_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0151); //Tune_wbt_GAS_10_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A0); //Tune_wbt_GAS_11_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0203); //Tune_wbt_GAS_12_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01F6); //Tune_wbt_GAS_13_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018A); //Tune_wbt_GAS_14_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013C); //Tune_wbt_GAS_15_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F2); //Tune_wbt_GAS_16_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BD); //Tune_wbt_GAS_17_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009D); //Tune_wbt_GAS_18_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008D); //Tune_wbt_GAS_19_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0095); //Tune_wbt_GAS_20_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B3); //Tune_wbt_GAS_21_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E7); //Tune_wbt_GAS_22_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0136); //Tune_wbt_GAS_23_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018E); //Tune_wbt_GAS_24_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01EA); //Tune_wbt_GAS_25_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B1); //Tune_wbt_GAS_26_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014C); //Tune_wbt_GAS_27_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FA); //Tune_wbt_GAS_28_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AC); //Tune_wbt_GAS_29_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0078); //Tune_wbt_GAS_30_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0056); //Tune_wbt_GAS_31_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004B); //Tune_wbt_GAS_32_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0053); //Tune_wbt_GAS_33_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006C); //Tune_wbt_GAS_34_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A1); //Tune_wbt_GAS_35_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E7); //Tune_wbt_GAS_36_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014E); //Tune_wbt_GAS_37_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A9); //Tune_wbt_GAS_38_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x017F); //Tune_wbt_GAS_39_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0121); //Tune_wbt_GAS_40_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C7); //Tune_wbt_GAS_41_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); //Tune_wbt_GAS_42_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0049); //Tune_wbt_GAS_43_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_44_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0021); //Tune_wbt_GAS_45_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0027); //Tune_wbt_GAS_46_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003E); //Tune_wbt_GAS_47_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006E); //Tune_wbt_GAS_48_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B4); //Tune_wbt_GAS_49_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0116); //Tune_wbt_GAS_50_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0178); //Tune_wbt_GAS_51_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0168); //Tune_wbt_GAS_52_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0105); //Tune_wbt_GAS_53_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A9); //Tune_wbt_GAS_54_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005F); //Tune_wbt_GAS_55_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002D); //Tune_wbt_GAS_56_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0012); //Tune_wbt_GAS_57_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); //Tune_wbt_GAS_58_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E); //Tune_wbt_GAS_59_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); //Tune_wbt_GAS_60_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004F); //Tune_wbt_GAS_61_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0095); //Tune_wbt_GAS_62_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F6); //Tune_wbt_GAS_63_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015A); //Tune_wbt_GAS_64_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015C); //Tune_wbt_GAS_65_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F6); //Tune_wbt_GAS_66_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0099); //Tune_wbt_GAS_67_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0050); //Tune_wbt_GAS_68_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0022); //Tune_wbt_GAS_69_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009); //Tune_wbt_GAS_70_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Tune_wbt_GAS_71_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); //Tune_wbt_GAS_72_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0017); //Tune_wbt_GAS_73_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0043); //Tune_wbt_GAS_74_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0084); //Tune_wbt_GAS_75_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6); //Tune_wbt_GAS_76_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014F); //Tune_wbt_GAS_77_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015A); //Tune_wbt_GAS_78_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F7); //Tune_wbt_GAS_79_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009C); //Tune_wbt_GAS_80_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0054); //Tune_wbt_GAS_81_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024); //Tune_wbt_GAS_82_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009); //Tune_wbt_GAS_83_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Tune_wbt_GAS_84_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004); //Tune_wbt_GAS_85_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0018); //Tune_wbt_GAS_86_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0044); //Tune_wbt_GAS_87_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0086); //Tune_wbt_GAS_88_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6); //Tune_wbt_GAS_89_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014E); //Tune_wbt_GAS_90_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0162); //Tune_wbt_GAS_91_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0106); //Tune_wbt_GAS_92_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AA); //Tune_wbt_GAS_93_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0062); //Tune_wbt_GAS_94_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0030); //Tune_wbt_GAS_95_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); //Tune_wbt_GAS_96_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000B); //Tune_wbt_GAS_97_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); //Tune_wbt_GAS_98_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0025); //Tune_wbt_GAS_99_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0053); //Tune_wbt_GAS_100_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0095); //Tune_wbt_GAS_101_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F7); //Tune_wbt_GAS_102_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015C); //Tune_wbt_GAS_103_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x017C); //Tune_wbt_GAS_104_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0122); //Tune_wbt_GAS_105_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CB); //Tune_wbt_GAS_106_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); //Tune_wbt_GAS_107_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004C); //Tune_wbt_GAS_108_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0030); //Tune_wbt_GAS_109_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); //Tune_wbt_GAS_110_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C); //Tune_wbt_GAS_111_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0043); //Tune_wbt_GAS_112_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0074); //Tune_wbt_GAS_113_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B7); //Tune_wbt_GAS_114_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011B); //Tune_wbt_GAS_115_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x017A); //Tune_wbt_GAS_116_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A8); //Tune_wbt_GAS_117_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013C); //Tune_wbt_GAS_118_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F2); //Tune_wbt_GAS_119_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AA); //Tune_wbt_GAS_120_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0077); //Tune_wbt_GAS_121_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058); //Tune_wbt_GAS_122_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004E); //Tune_wbt_GAS_123_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0056); //Tune_wbt_GAS_124_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); //Tune_wbt_GAS_125_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A2); //Tune_wbt_GAS_126_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EB); //Tune_wbt_GAS_127_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0146); //Tune_wbt_GAS_128_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x019D); //Tune_wbt_GAS_129_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D9); //Tune_wbt_GAS_130_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016A); //Tune_wbt_GAS_131_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0121); //Tune_wbt_GAS_132_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E5); //Tune_wbt_GAS_133_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B5); //Tune_wbt_GAS_134_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0098); //Tune_wbt_GAS_135_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008C); //Tune_wbt_GAS_136_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0094); //Tune_wbt_GAS_137_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B1); //Tune_wbt_GAS_138_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E4); //Tune_wbt_GAS_139_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0126); //Tune_wbt_GAS_140_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x017D); //Tune_wbt_GAS_141_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01DE); //Tune_wbt_GAS_142_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A0); //Tune_wbt_GAS_143_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FA); //Tune_wbt_GAS_144_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BE); //Tune_wbt_GAS_145_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0095); //Tune_wbt_GAS_146_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007E); //Tune_wbt_GAS_147_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006F); //Tune_wbt_GAS_148_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006A); //Tune_wbt_GAS_149_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006C); //Tune_wbt_GAS_150_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007C); //Tune_wbt_GAS_151_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0094); //Tune_wbt_GAS_152_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B4); //Tune_wbt_GAS_153_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E7); //Tune_wbt_GAS_154_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014D); //Tune_wbt_GAS_155_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013A); //Tune_wbt_GAS_156_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D4); //Tune_wbt_GAS_157_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A7); //Tune_wbt_GAS_158_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0081); //Tune_wbt_GAS_159_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0068); //Tune_wbt_GAS_160_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005A); //Tune_wbt_GAS_161_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0053); //Tune_wbt_GAS_162_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0059); //Tune_wbt_GAS_163_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0068); //Tune_wbt_GAS_164_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0082); //Tune_wbt_GAS_165_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A7); //Tune_wbt_GAS_166_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D5); //Tune_wbt_GAS_167_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011F); //Tune_wbt_GAS_168_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); //Tune_wbt_GAS_169_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AB); //Tune_wbt_GAS_170_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0084); //Tune_wbt_GAS_171_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005D); //Tune_wbt_GAS_172_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0044); //Tune_wbt_GAS_173_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0034); //Tune_wbt_GAS_174_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0030); //Tune_wbt_GAS_175_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0035); //Tune_wbt_GAS_176_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0044); //Tune_wbt_GAS_177_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005F); //Tune_wbt_GAS_178_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0081); //Tune_wbt_GAS_179_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B3); //Tune_wbt_GAS_180_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F3); //Tune_wbt_GAS_181_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DC); //Tune_wbt_GAS_182_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0094); //Tune_wbt_GAS_183_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006B); //Tune_wbt_GAS_184_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0044); //Tune_wbt_GAS_185_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0029); //Tune_wbt_GAS_186_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); //Tune_wbt_GAS_187_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015); //Tune_wbt_GAS_188_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); //Tune_wbt_GAS_189_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); //Tune_wbt_GAS_190_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0042); //Tune_wbt_GAS_191_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); //Tune_wbt_GAS_192_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0094); //Tune_wbt_GAS_193_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D0); //Tune_wbt_GAS_194_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CD); //Tune_wbt_GAS_195_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0087); //Tune_wbt_GAS_196_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005B); //Tune_wbt_GAS_197_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0034); //Tune_wbt_GAS_198_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); //Tune_wbt_GAS_199_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); //Tune_wbt_GAS_200_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008); //Tune_wbt_GAS_201_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000B); //Tune_wbt_GAS_202_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0018); //Tune_wbt_GAS_203_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0031); //Tune_wbt_GAS_204_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0055); //Tune_wbt_GAS_205_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0085); //Tune_wbt_GAS_206_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BD); //Tune_wbt_GAS_207_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C6); //Tune_wbt_GAS_208_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007E); //Tune_wbt_GAS_209_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0053); //Tune_wbt_GAS_210_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C); //Tune_wbt_GAS_211_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013); //Tune_wbt_GAS_212_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006); //Tune_wbt_GAS_213_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); //Tune_wbt_GAS_214_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004); //Tune_wbt_GAS_215_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0011); //Tune_wbt_GAS_216_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_217_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004D); //Tune_wbt_GAS_218_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007C); //Tune_wbt_GAS_219_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B7); //Tune_wbt_GAS_220_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C6); //Tune_wbt_GAS_221_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); //Tune_wbt_GAS_222_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0054); //Tune_wbt_GAS_223_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002D); //Tune_wbt_GAS_224_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013); //Tune_wbt_GAS_225_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005); //Tune_wbt_GAS_226_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); //Tune_wbt_GAS_227_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004); //Tune_wbt_GAS_228_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); //Tune_wbt_GAS_229_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_230_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004C); //Tune_wbt_GAS_231_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007C); //Tune_wbt_GAS_232_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B7); //Tune_wbt_GAS_233_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CB); //Tune_wbt_GAS_234_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0083); //Tune_wbt_GAS_235_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005A); //Tune_wbt_GAS_236_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0034); //Tune_wbt_GAS_237_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); //Tune_wbt_GAS_238_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000B); //Tune_wbt_GAS_239_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007); //Tune_wbt_GAS_240_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); //Tune_wbt_GAS_241_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0017); //Tune_wbt_GAS_242_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0031); //Tune_wbt_GAS_243_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052); //Tune_wbt_GAS_244_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0082); //Tune_wbt_GAS_245_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BB); //Tune_wbt_GAS_246_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DF); //Tune_wbt_GAS_247_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0092); //Tune_wbt_GAS_248_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0069); //Tune_wbt_GAS_249_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0043); //Tune_wbt_GAS_250_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_251_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001B); //Tune_wbt_GAS_252_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015); //Tune_wbt_GAS_253_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); //Tune_wbt_GAS_254_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); //Tune_wbt_GAS_255_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0042); //Tune_wbt_GAS_256_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0063); //Tune_wbt_GAS_257_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0092); //Tune_wbt_GAS_258_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CE); //Tune_wbt_GAS_259_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //Tune_wbt_GAS_260_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A3); //Tune_wbt_GAS_261_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007B); //Tune_wbt_GAS_262_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0056); //Tune_wbt_GAS_263_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003D); //Tune_wbt_GAS_264_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0030); //Tune_wbt_GAS_265_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C); //Tune_wbt_GAS_266_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0030); //Tune_wbt_GAS_267_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003F); //Tune_wbt_GAS_268_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058); //Tune_wbt_GAS_269_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007A); //Tune_wbt_GAS_270_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A8); //Tune_wbt_GAS_271_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E5); //Tune_wbt_GAS_272_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0139); //Tune_wbt_GAS_273_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C6); //Tune_wbt_GAS_274_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0097); //Tune_wbt_GAS_275_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0077); //Tune_wbt_GAS_276_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0060); //Tune_wbt_GAS_277_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0053); //Tune_wbt_GAS_278_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0050); //Tune_wbt_GAS_279_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0053); //Tune_wbt_GAS_280_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); //Tune_wbt_GAS_281_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); //Tune_wbt_GAS_282_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009D); //Tune_wbt_GAS_283_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CD); //Tune_wbt_GAS_284_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011F); //Tune_wbt_GAS_285_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0192); //Tune_wbt_GAS_286_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F2); //Tune_wbt_GAS_287_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B6); //Tune_wbt_GAS_288_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008D); //Tune_wbt_GAS_289_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0075); //Tune_wbt_GAS_290_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0066); //Tune_wbt_GAS_291_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0060); //Tune_wbt_GAS_292_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); //Tune_wbt_GAS_293_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0072); //Tune_wbt_GAS_294_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008A); //Tune_wbt_GAS_295_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AB); //Tune_wbt_GAS_296_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DD); //Tune_wbt_GAS_297_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0145); //Tune_wbt_GAS_298_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0136); //Tune_wbt_GAS_299_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D0); //Tune_wbt_GAS_300_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A5); //Tune_wbt_GAS_301_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007E); //Tune_wbt_GAS_302_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); //Tune_wbt_GAS_303_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0055); //Tune_wbt_GAS_304_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004E); //Tune_wbt_GAS_305_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0053); //Tune_wbt_GAS_306_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0061); //Tune_wbt_GAS_307_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007C); //Tune_wbt_GAS_308_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A1); //Tune_wbt_GAS_309_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CE); //Tune_wbt_GAS_310_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011C); //Tune_wbt_GAS_311_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FC); //Tune_wbt_GAS_312_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AA); //Tune_wbt_GAS_313_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0082); //Tune_wbt_GAS_314_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005A); //Tune_wbt_GAS_315_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0041); //Tune_wbt_GAS_316_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0030); //Tune_wbt_GAS_317_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_318_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0030); //Tune_wbt_GAS_319_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003E); //Tune_wbt_GAS_320_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0059); //Tune_wbt_GAS_321_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007A); //Tune_wbt_GAS_322_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AC); //Tune_wbt_GAS_323_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EE); //Tune_wbt_GAS_324_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D8); //Tune_wbt_GAS_325_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0094); //Tune_wbt_GAS_326_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006A); //Tune_wbt_GAS_327_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0043); //Tune_wbt_GAS_328_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); //Tune_wbt_GAS_329_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016); //Tune_wbt_GAS_330_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0012); //Tune_wbt_GAS_331_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016); //Tune_wbt_GAS_332_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); //Tune_wbt_GAS_333_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003E); //Tune_wbt_GAS_334_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0061); //Tune_wbt_GAS_335_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0090); //Tune_wbt_GAS_336_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CE); //Tune_wbt_GAS_337_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CC); //Tune_wbt_GAS_338_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0087); //Tune_wbt_GAS_339_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005C); //Tune_wbt_GAS_340_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0034); //Tune_wbt_GAS_341_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0019); //Tune_wbt_GAS_342_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); //Tune_wbt_GAS_343_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005); //Tune_wbt_GAS_344_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009); //Tune_wbt_GAS_345_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015); //Tune_wbt_GAS_346_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002E); //Tune_wbt_GAS_347_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052); //Tune_wbt_GAS_348_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0082); //Tune_wbt_GAS_349_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BE); //Tune_wbt_GAS_350_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C5); //Tune_wbt_GAS_351_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007F); //Tune_wbt_GAS_352_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0054); //Tune_wbt_GAS_353_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002D); //Tune_wbt_GAS_354_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013); //Tune_wbt_GAS_355_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004); //Tune_wbt_GAS_356_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Tune_wbt_GAS_357_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); //Tune_wbt_GAS_358_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E); //Tune_wbt_GAS_359_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0027); //Tune_wbt_GAS_360_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_361_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007A); //Tune_wbt_GAS_362_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B5); //Tune_wbt_GAS_363_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C4); //Tune_wbt_GAS_364_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); //Tune_wbt_GAS_365_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0056); //Tune_wbt_GAS_366_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002E); //Tune_wbt_GAS_367_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); //Tune_wbt_GAS_368_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004); //Tune_wbt_GAS_369_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Tune_wbt_GAS_370_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); //Tune_wbt_GAS_371_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E); //Tune_wbt_GAS_372_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0027); //Tune_wbt_GAS_373_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_374_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0079); //Tune_wbt_GAS_375_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B6); //Tune_wbt_GAS_376_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CA); //Tune_wbt_GAS_377_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0086); //Tune_wbt_GAS_378_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005C); //Tune_wbt_GAS_379_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0035); //Tune_wbt_GAS_380_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001B); //Tune_wbt_GAS_381_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); //Tune_wbt_GAS_382_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005); //Tune_wbt_GAS_383_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008); //Tune_wbt_GAS_384_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015); //Tune_wbt_GAS_385_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002F); //Tune_wbt_GAS_386_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0050); //Tune_wbt_GAS_387_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007F); //Tune_wbt_GAS_388_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BA); //Tune_wbt_GAS_389_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DE); //Tune_wbt_GAS_390_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0094); //Tune_wbt_GAS_391_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006B); //Tune_wbt_GAS_392_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0045); //Tune_wbt_GAS_393_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_394_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); //Tune_wbt_GAS_395_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013); //Tune_wbt_GAS_396_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0018); //Tune_wbt_GAS_397_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0025); //Tune_wbt_GAS_398_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003F); //Tune_wbt_GAS_399_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005F); //Tune_wbt_GAS_400_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0090); //Tune_wbt_GAS_401_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CD); //Tune_wbt_GAS_402_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101); //Tune_wbt_GAS_403_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A6); //Tune_wbt_GAS_404_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); //Tune_wbt_GAS_405_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0057); //Tune_wbt_GAS_406_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003E); //Tune_wbt_GAS_407_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002F); //Tune_wbt_GAS_408_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_409_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002E); //Tune_wbt_GAS_410_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003B); //Tune_wbt_GAS_411_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0055); //Tune_wbt_GAS_412_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0077); //Tune_wbt_GAS_413_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A5); //Tune_wbt_GAS_414_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E3); //Tune_wbt_GAS_415_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0139); //Tune_wbt_GAS_416_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C6); //Tune_wbt_GAS_417_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0099); //Tune_wbt_GAS_418_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0078); //Tune_wbt_GAS_419_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0060); //Tune_wbt_GAS_420_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052); //Tune_wbt_GAS_421_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004D); //Tune_wbt_GAS_422_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0051); //Tune_wbt_GAS_423_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0060); //Tune_wbt_GAS_424_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0079); //Tune_wbt_GAS_425_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009A); //Tune_wbt_GAS_426_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CA); //Tune_wbt_GAS_427_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0120); //Tune_wbt_GAS_428_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016F); //Tune_wbt_GAS_429_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D5); //Tune_wbt_GAS_430_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009D); //Tune_wbt_GAS_431_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007A); //Tune_wbt_GAS_432_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0065); //Tune_wbt_GAS_433_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058); //Tune_wbt_GAS_434_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0054); //Tune_wbt_GAS_435_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0056); //Tune_wbt_GAS_436_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0063); //Tune_wbt_GAS_437_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007A); //Tune_wbt_GAS_438_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0095); //Tune_wbt_GAS_439_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C5); //Tune_wbt_GAS_440_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011E); //Tune_wbt_GAS_441_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010E); //Tune_wbt_GAS_442_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B5); //Tune_wbt_GAS_443_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008A); //Tune_wbt_GAS_444_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006B); //Tune_wbt_GAS_445_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0057); //Tune_wbt_GAS_446_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004B); //Tune_wbt_GAS_447_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046); //Tune_wbt_GAS_448_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_449_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058); //Tune_wbt_GAS_450_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006E); //Tune_wbt_GAS_451_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0090); //Tune_wbt_GAS_452_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B7); //Tune_wbt_GAS_453_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EE); //Tune_wbt_GAS_454_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D7); //Tune_wbt_GAS_455_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0091); //Tune_wbt_GAS_456_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006C); //Tune_wbt_GAS_457_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004C); //Tune_wbt_GAS_458_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0038); //Tune_wbt_GAS_459_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_460_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0026); //Tune_wbt_GAS_461_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_462_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0038); //Tune_wbt_GAS_463_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0050); //Tune_wbt_GAS_464_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006D); //Tune_wbt_GAS_465_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009C); //Tune_wbt_GAS_466_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C7); //Tune_wbt_GAS_467_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B5); //Tune_wbt_GAS_468_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007E); //Tune_wbt_GAS_469_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058); //Tune_wbt_GAS_470_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0038); //Tune_wbt_GAS_471_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0021); //Tune_wbt_GAS_472_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015); //Tune_wbt_GAS_473_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0011); //Tune_wbt_GAS_474_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); //Tune_wbt_GAS_475_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0022); //Tune_wbt_GAS_476_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003A); //Tune_wbt_GAS_477_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0057); //Tune_wbt_GAS_478_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0083); //Tune_wbt_GAS_479_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AF); //Tune_wbt_GAS_480_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A8); //Tune_wbt_GAS_481_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0071); //Tune_wbt_GAS_482_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004B); //Tune_wbt_GAS_483_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002B); //Tune_wbt_GAS_484_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); //Tune_wbt_GAS_485_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009); //Tune_wbt_GAS_486_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005); //Tune_wbt_GAS_487_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008); //Tune_wbt_GAS_488_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); //Tune_wbt_GAS_489_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002B); //Tune_wbt_GAS_490_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_491_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0076); //Tune_wbt_GAS_492_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009F); //Tune_wbt_GAS_493_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A1); //Tune_wbt_GAS_494_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006B); //Tune_wbt_GAS_495_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0045); //Tune_wbt_GAS_496_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024); //Tune_wbt_GAS_497_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F); //Tune_wbt_GAS_498_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); //Tune_wbt_GAS_499_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Tune_wbt_GAS_500_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); //Tune_wbt_GAS_501_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E); //Tune_wbt_GAS_502_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024); //Tune_wbt_GAS_503_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0042); //Tune_wbt_GAS_504_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006E); //Tune_wbt_GAS_505_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0098); //Tune_wbt_GAS_506_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A1); //Tune_wbt_GAS_507_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006C); //Tune_wbt_GAS_508_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046); //Tune_wbt_GAS_509_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0027); //Tune_wbt_GAS_510_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); //Tune_wbt_GAS_511_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004); //Tune_wbt_GAS_512_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Tune_wbt_GAS_513_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); //Tune_wbt_GAS_514_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E); //Tune_wbt_GAS_515_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024); //Tune_wbt_GAS_516_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0043); //Tune_wbt_GAS_517_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006E); //Tune_wbt_GAS_518_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0099); //Tune_wbt_GAS_519_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AA); //Tune_wbt_GAS_520_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); //Tune_wbt_GAS_521_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004D); //Tune_wbt_GAS_522_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002D); //Tune_wbt_GAS_523_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016); //Tune_wbt_GAS_524_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009); //Tune_wbt_GAS_525_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005); //Tune_wbt_GAS_526_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008); //Tune_wbt_GAS_527_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); //Tune_wbt_GAS_528_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C); //Tune_wbt_GAS_529_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0049); //Tune_wbt_GAS_530_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0076); //Tune_wbt_GAS_531_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009C); //Tune_wbt_GAS_532_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BD); //Tune_wbt_GAS_533_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007F); //Tune_wbt_GAS_534_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058); //Tune_wbt_GAS_535_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003A); //Tune_wbt_GAS_536_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024); //Tune_wbt_GAS_537_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0018); //Tune_wbt_GAS_538_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0012); //Tune_wbt_GAS_539_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016); //Tune_wbt_GAS_540_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); //Tune_wbt_GAS_541_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003B); //Tune_wbt_GAS_542_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058); //Tune_wbt_GAS_543_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0082); //Tune_wbt_GAS_544_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AB); //Tune_wbt_GAS_545_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DC); //Tune_wbt_GAS_546_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008F); //Tune_wbt_GAS_547_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006A); //Tune_wbt_GAS_548_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004C); //Tune_wbt_GAS_549_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0038); //Tune_wbt_GAS_550_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C); //Tune_wbt_GAS_551_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); //Tune_wbt_GAS_552_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C); //Tune_wbt_GAS_553_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0038); //Tune_wbt_GAS_554_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0050); //Tune_wbt_GAS_555_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006C); //Tune_wbt_GAS_556_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0096); //Tune_wbt_GAS_557_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C2); //Tune_wbt_GAS_558_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0117); //Tune_wbt_GAS_559_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AF); //Tune_wbt_GAS_560_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0083); //Tune_wbt_GAS_561_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0068); //Tune_wbt_GAS_562_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0054); //Tune_wbt_GAS_563_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_564_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046); //Tune_wbt_GAS_565_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_566_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058); //Tune_wbt_GAS_567_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006D); //Tune_wbt_GAS_568_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008A); //Tune_wbt_GAS_569_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B4); //Tune_wbt_GAS_570_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FB); //Tune_wbt_GAS_571_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1348);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);

	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B36);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);// awbb_IndoorGrZones_ZInfo_m_GridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B3A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00F3);// awbb_IndoorGrZones_ZInfo_m_BMin */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02CB);// awbb_IndoorGrZones_ZInfo_m_BMax */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B38);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0010);// awbb_IndoorGrZones_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0AE6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0385);// 0352 03E1 awbb_IndoorGrZones_m_BGrid_0__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03D8);// 038C 0413 awbb_IndoorGrZones_m_BGrid_0__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x032A);// 0321 039E awbb_IndoorGrZones_m_BGrid_1__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03C5);// 03A6 0416 awbb_IndoorGrZones_m_BGrid_1__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02F5);// 02EC 0367 awbb_IndoorGrZones_m_BGrid_2__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x039D);// 03A0 03F3 awbb_IndoorGrZones_m_BGrid_2__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02D3);// 02CA 032D awbb_IndoorGrZones_m_BGrid_3__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0372);// 038D 03C5 awbb_IndoorGrZones_m_BGrid_3__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02B1);// 02A8 02FD awbb_IndoorGrZones_m_BGrid_4__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x033E);// 036E 038F awbb_IndoorGrZones_m_BGrid_4__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x028A);// 0281 02D3 awbb_IndoorGrZones_m_BGrid_5__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0322);// 0344 0365 awbb_IndoorGrZones_m_BGrid_5__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0268);// 025F 02AA awbb_IndoorGrZones_m_BGrid_6__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02FD);// 0327 033E awbb_IndoorGrZones_m_BGrid_6__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0248);// 023F 028D awbb_IndoorGrZones_m_BGrid_7__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02EF);// 0302 0310 awbb_IndoorGrZones_m_BGrid_7__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x022F);// 0226 0271 awbb_IndoorGrZones_m_BGrid_8__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02D5);// 02DC 02F1 awbb_IndoorGrZones_m_BGrid_8__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0219);// 0210 025A awbb_IndoorGrZones_m_BGrid_9__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02C2);// 02B9 02D2 awbb_IndoorGrZones_m_BGrid_9__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0206);// 01FD 0249 awbb_IndoorGrZones_m_BGrid_10__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02A3);// 029A 02B9 awbb_IndoorGrZones_m_BGrid_10__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01F0);// 01E7 0238 awbb_IndoorGrZones_m_BGrid_11__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0286);// 027D 02A2 awbb_IndoorGrZones_m_BGrid_11__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01E3);// 01DA 021B awbb_IndoorGrZones_m_BGrid_12__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0268);// 025F 0289 awbb_IndoorGrZones_m_BGrid_12__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01D6);// 01CD 0200 awbb_IndoorGrZones_m_BGrid_13__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x024E);// 0245 026C awbb_IndoorGrZones_m_BGrid_13__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01DD);// 01D4 01FC awbb_IndoorGrZones_m_BGrid_14__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x022A);// 0221 024F awbb_IndoorGrZones_m_BGrid_14__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0210);// 0207 021E awbb_IndoorGrZones_m_BGrid_15__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01F2);// 01E9 022C awbb_IndoorGrZones_m_BGrid_15__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_16__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_16__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_17__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_17__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_18__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_18__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_19__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_19__m_right */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BAA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0006);// awbb_LowBrGrZones_ZInfo_m_GridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BAE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00CC);// 010E awbb_LowBrGrZones_ZInfo_m_BMin */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02F3);// 02E9 awbb_LowBrGrZones_ZInfo_m_BMax */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BAC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x000A);// 0009 awbb_LowBrGrZones_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B7A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x036C);// 0374 038C awbb_LowBrGrZones_m_BGrid_0__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03C6);// 03CE 03DA awbb_LowBrGrZones_m_BGrid_0__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02EE);// 02F6 030E awbb_LowBrGrZones_m_BGrid_1__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03F9);// 0401 03E9 awbb_LowBrGrZones_m_BGrid_1__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02BE);// 02C6 02A2 awbb_LowBrGrZones_m_BGrid_2__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03DF);// 03E7 03C2 awbb_LowBrGrZones_m_BGrid_2__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x027A);// 0282 0259 awbb_LowBrGrZones_m_BGrid_3__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03AE);// 03B6 038A awbb_LowBrGrZones_m_BGrid_3__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0234);// 023C 0218 awbb_LowBrGrZones_m_BGrid_4__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0376);// 037E 0352 awbb_LowBrGrZones_m_BGrid_4__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0204);// 020C 01F4 awbb_LowBrGrZones_m_BGrid_5__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x033E);// 0346 02E1 awbb_LowBrGrZones_m_BGrid_5__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01E0);// 01E8 01D7 awbb_LowBrGrZones_m_BGrid_6__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02CD);// 02D5 028E awbb_LowBrGrZones_m_BGrid_6__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01C3);// 01CB 01CB awbb_LowBrGrZones_m_BGrid_7__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x027A);// 0282 0258 awbb_LowBrGrZones_m_BGrid_7__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01B7);// 01BF 022B awbb_LowBrGrZones_m_BGrid_8__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0244);// 024C 01CC awbb_LowBrGrZones_m_BGrid_8__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01FE);// 01F8 0000 awbb_LowBrGrZones_m_BGrid_9__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01DD);// 0201 0000 awbb_LowBrGrZones_m_BGrid_9__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_LowBrGrZones_m_BGrid_10__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_LowBrGrZones_m_BGrid_10__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_LowBrGrZones_m_BGrid_11__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_LowBrGrZones_m_BGrid_11__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B70);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);// awbb_OutdoorGrZones_ZInfo_m_GridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B74);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01E3);// awbb_OutdoorGrZones_ZInfo_m_BMin */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0270);// awbb_OutdoorGrZones_ZInfo_m_BMax */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B72);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0006);// awbb_OutdoorGrZones_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B40);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x028A);// 029E awbb_OutdoorGrZones_m_BGrid_0__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02A1);// 02C8 awbb_OutdoorGrZones_m_BGrid_0__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0263);// 0281 awbb_OutdoorGrZones_m_BGrid_1__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02C0);// 02C8 awbb_OutdoorGrZones_m_BGrid_1__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x024C);// 0266 awbb_OutdoorGrZones_m_BGrid_2__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02BE);// 02AC awbb_OutdoorGrZones_m_BGrid_2__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x023D);// 0251 awbb_OutdoorGrZones_m_BGrid_3__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02A6);// 028E awbb_OutdoorGrZones_m_BGrid_3__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0243);// 023D awbb_OutdoorGrZones_m_BGrid_4__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0289);// 0275 awbb_OutdoorGrZones_m_BGrid_4__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x026F);// 0228 awbb_OutdoorGrZones_m_BGrid_5__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x025D);// 025D awbb_OutdoorGrZones_m_BGrid_5__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0228 awbb_OutdoorGrZones_m_BGrid_6__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0243 awbb_OutdoorGrZones_m_BGrid_6__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_7__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_7__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_8__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_8__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_9__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_9__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_10__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_10__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_11__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_11__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BC8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);// awbb_CWSkinZone_ZInfo_m_GridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BCC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x010F);// awbb_CWSkinZone_ZInfo_m_BMin */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x018F);// awbb_CWSkinZone_ZInfo_m_BMax */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BCA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);// awbb_CWSkinZone_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BB4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03E7);// awbb_CWSkinZone_m_BGrid_0__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03F8);// awbb_CWSkinZone_m_BGrid_0__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03A7);// awbb_CWSkinZone_m_BGrid_1__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03FC);// awbb_CWSkinZone_m_BGrid_1__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0352);// awbb_CWSkinZone_m_BGrid_2__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03D0);// awbb_CWSkinZone_m_BGrid_2__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0322);// awbb_CWSkinZone_m_BGrid_3__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x039E);// awbb_CWSkinZone_m_BGrid_3__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x032B);// awbb_CWSkinZone_m_BGrid_4__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x034D);// awbb_CWSkinZone_m_BGrid_4__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BE6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0006);// awbb_DLSkinZone_ZInfo_m_GridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BEA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x019E);// awbb_DLSkinZone_ZInfo_m_BMin */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0257);// awbb_DLSkinZone_ZInfo_m_BMax */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BE8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0004);// awbb_DLSkinZone_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BD2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x030B);// awbb_DLSkinZone_m_BGrid_0__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0323);// awbb_DLSkinZone_m_BGrid_0__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02C3);// awbb_DLSkinZone_m_BGrid_1__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x030F);// awbb_DLSkinZone_m_BGrid_1__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0288);// awbb_DLSkinZone_m_BGrid_2__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02E5);// awbb_DLSkinZone_m_BGrid_2__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x026A);// awbb_DLSkinZone_m_BGrid_3__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02A2);// awbb_DLSkinZone_m_BGrid_3__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_DLSkinZone_m_BGrid_4__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_DLSkinZone_m_BGrid_4__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C2C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0139);// awbb_IntcR */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0122);// awbb_IntcB */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BFC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0378);// 03AD awbb_IndoorWP_0__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x011E);// 013F awbb_IndoorWP_0__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02F0);// 0341 awbb_IndoorWP_1__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0184);// 017B awbb_IndoorWP_1__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0313);// 038D awbb_IndoorWP_2__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0158);// 014B awbb_IndoorWP_2__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02BA);// 02C3 awbb_IndoorWP_3__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01BA);// 01CC awbb_IndoorWP_3__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0231);// 0241 awbb_IndoorWP_4__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0252);// 027F awbb_IndoorWP_4__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0237);// 0241 awbb_IndoorWP_5__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x024C);// 027F awbb_IndoorWP_5__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x020F);// 0214 awbb_IndoorWP_6__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0279);// 02A8 awbb_IndoorWP_6__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0268);// 0270 255 awbb_OutdoorWP_r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x021A);// 0210 25B awbb_OutdoorWP_b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C4C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0450);// awbb_MvEq_RBthresh */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C58);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x059C);// awbb_MvEq_RBthresh */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BF8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01AE);// awbb_LowTSep_m_RminusB */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C28);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_SkinPreference */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0CAC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0050);// awbb_OutDMaxIncr */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C28);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_SkinPreference */ 
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x20BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0006);// Lowtemp bypass */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0D0E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00B8);// awbb_GridCoeff_R_2 */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00B2);// awbb_GridCoeff_B_2 */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0CFE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0FAB);// 0FAB awbb_GridConst_2_0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0FF5);// 0FF5 0FF5 awbb_GridConst_2_1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x10BB);// 10BB 10BB awbb_GridConst_2_2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x1117);// 1117 1123 1153 awbb_GridConst_2_3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x116D);// 116D 11C5 awbb_GridConst_2_4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x11D5);// 122A awbb_GridConst_2_5_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00A9);// awbb_GridCoeff_R_1 */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00C0);// awbb_GridCoeff_B_1 */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0CF8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02CC);// awbb_GridConst_1_0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x031E);// awbb_GridConst_1_1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0359);// awbb_GridConst_1_2_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0CB0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0030);// 0000 awbb_GridCorr_R_0__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 0000 awbb_GridCorr_R_0__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0060);// 0078 awbb_GridCorr_R_0__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 00AA awbb_GridCorr_R_0__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0008);// 0000 awbb_GridCorr_R_0__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0008);// 0000 awbb_GridCorr_R_0__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0030);// 0000 awbb_GridCorr_R_1__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 0096 awbb_GridCorr_R_1__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0060);// 0000 awbb_GridCorr_R_1__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 0000 awbb_GridCorr_R_1__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0008);// 0000 awbb_GridCorr_R_1__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0008);// 0000 awbb_GridCorr_R_1__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0030);// 00E6 awbb_GridCorr_R_2__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 0000 awbb_GridCorr_R_2__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0060);// 0000 awbb_GridCorr_R_2__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 0000 awbb_GridCorr_R_2__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0008);// 0000 awbb_GridCorr_R_2__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0008);// 0000 awbb_GridCorr_R_2__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0018);// 0000 awbb_GridCorr_B_0__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_0__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0064 awbb_GridCorr_B_0__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_0__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFF80);// 0000 awbb_GridCorr_B_0__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFEC0);// 0000 awbb_GridCorr_B_0__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0018);// 0000 awbb_GridCorr_B_1__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0032 awbb_GridCorr_B_1__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_1__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_1__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFF80);// FF38 awbb_GridCorr_B_1__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFEC0);// 0000 awbb_GridCorr_B_1__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0018);// 0000 awbb_GridCorr_B_2__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0032 awbb_GridCorr_B_2__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_2__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_2__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFF80);// 0000 awbb_GridCorr_B_2__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFEC0);// 0000 awbb_GridCorr_B_2__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0D30);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);// awbb_GridEnable */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x3372);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);// awbb_bUseOutdoorGrid */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_OutdoorGridCorr_R */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_OutdoorGridCorr_B */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C86);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);// awbb_OutdoorDetectionZone_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C70);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFF7B);// awbb_OutdoorDetectionZone_m_BGrid_0__m_left */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00CE);// awbb_OutdoorDetectionZone_m_BGrid_0__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFF23);// awbb_OutdoorDetectionZone_m_BGrid_1__m_left */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x010D);// awbb_OutdoorDetectionZone_m_BGrid_1__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFEF3);// awbb_OutdoorDetectionZone_m_BGrid_2__m_left */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x012C);// awbb_OutdoorDetectionZone_m_BGrid_2__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFED7);// awbb_OutdoorDetectionZone_m_BGrid_3__m_left */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x014E);// awbb_OutdoorDetectionZone_m_BGrid_3__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFEBB);// awbb_OutdoorDetectionZone_m_BGrid_4__m_left */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0162);// awbb_OutdoorDetectionZone_m_BGrid_4__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x1388);// awbb_OutdoorDetectionZone_ZInfo_m_AbsGridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C8A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x4ACB);// awbb_OutdoorDetectionZone_ZInfo_m_MaxNB */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C88);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0A7C);// awbb_OutdoorDetectionZone_ZInfo_m_NBoffs */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0CA0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0030); //awbb_GnCurPntImmunity
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0CA4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0030); //awbb_GnCurPntLongJump
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0180); //awbb_GainsMaxMove
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002); //awbb_GnMinMatchToJump


	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0538); //LutPreDemNoBin
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0035);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0095);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0121);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0139);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0150);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0177);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x019A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01DC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0219);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0251);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02B3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x035F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03B1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //LutPostDemNoBin
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0012);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0031);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0075);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0126);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0272);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0334);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x33A4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D0); //#TVAR_wbt_pBaseCcms[0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFA1); //#TVAR_wbt_pBaseCcms[1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFA); //#TVAR_wbt_pBaseCcms[2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6F); //#TVAR_wbt_pBaseCcms[3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0140); //#TVAR_wbt_pBaseCcms[4] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF49); //#TVAR_wbt_pBaseCcms[5] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC1); //#TVAR_wbt_pBaseCcms[6] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F); //#TVAR_wbt_pBaseCcms[7] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BD); //#TVAR_wbt_pBaseCcms[8] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013F); //#TVAR_wbt_pBaseCcms[9] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E1); //#TVAR_wbt_pBaseCcms[10]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF43); //#TVAR_wbt_pBaseCcms[11]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0191); //#TVAR_wbt_pBaseCcms[12]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC0); //#TVAR_wbt_pBaseCcms[13]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B7); //#TVAR_wbt_pBaseCcms[14]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF30); //#TVAR_wbt_pBaseCcms[15]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015F); //#TVAR_wbt_pBaseCcms[16]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0106); //#TVAR_wbt_pBaseCcms[17]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D0); //#TVAR_wbt_pBaseCcms[18]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFA1); //#TVAR_wbt_pBaseCcms[19]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFA); //#TVAR_wbt_pBaseCcms[20]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6F); //#TVAR_wbt_pBaseCcms[21]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0140); //#TVAR_wbt_pBaseCcms[22]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF49); //#TVAR_wbt_pBaseCcms[23]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC1); //#TVAR_wbt_pBaseCcms[24]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F); //#TVAR_wbt_pBaseCcms[25]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BD); //#TVAR_wbt_pBaseCcms[26]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013F); //#TVAR_wbt_pBaseCcms[27]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E1); //#TVAR_wbt_pBaseCcms[28]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF43); //#TVAR_wbt_pBaseCcms[29]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0191); //#TVAR_wbt_pBaseCcms[30]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC0); //#TVAR_wbt_pBaseCcms[31]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B7); //#TVAR_wbt_pBaseCcms[32]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF30); //#TVAR_wbt_pBaseCcms[33]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015F); //#TVAR_wbt_pBaseCcms[34]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0106); //#TVAR_wbt_pBaseCcms[35]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D0); //#TVAR_wbt_pBaseCcms[36]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFA1); //#TVAR_wbt_pBaseCcms[37]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFA); //#TVAR_wbt_pBaseCcms[38]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6F); //#TVAR_wbt_pBaseCcms[39]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0140); //#TVAR_wbt_pBaseCcms[40]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF49); //#TVAR_wbt_pBaseCcms[41]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC1); //#TVAR_wbt_pBaseCcms[42]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F); //#TVAR_wbt_pBaseCcms[43]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BD); //#TVAR_wbt_pBaseCcms[44]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013F); //#TVAR_wbt_pBaseCcms[45]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E1); //#TVAR_wbt_pBaseCcms[46]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF43); //#TVAR_wbt_pBaseCcms[47]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0191); //#TVAR_wbt_pBaseCcms[48]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC0); //#TVAR_wbt_pBaseCcms[49]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B7); //#TVAR_wbt_pBaseCcms[50]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF30); //#TVAR_wbt_pBaseCcms[51]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015F); //#TVAR_wbt_pBaseCcms[52]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0106); //#TVAR_wbt_pBaseCcms[53]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D0); //#TVAR_wbt_pBaseCcms[54]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFA1); //#TVAR_wbt_pBaseCcms[55]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFA); //#TVAR_wbt_pBaseCcms[56]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6F); //#TVAR_wbt_pBaseCcms[57]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0140); //#TVAR_wbt_pBaseCcms[58]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF49); //#TVAR_wbt_pBaseCcms[59]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC1); //#TVAR_wbt_pBaseCcms[60]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F); //#TVAR_wbt_pBaseCcms[61]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BD); //#TVAR_wbt_pBaseCcms[62]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013F); //#TVAR_wbt_pBaseCcms[63]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E1); //#TVAR_wbt_pBaseCcms[64]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF43); //#TVAR_wbt_pBaseCcms[65]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0191); //#TVAR_wbt_pBaseCcms[66]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC0); //#TVAR_wbt_pBaseCcms[67]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B7); //#TVAR_wbt_pBaseCcms[68]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF30); //#TVAR_wbt_pBaseCcms[69]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015F); //#TVAR_wbt_pBaseCcms[70]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0106); //#TVAR_wbt_pBaseCcms[71]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BF); //#TVAR_wbt_pBaseCcms[72]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFBF); //#TVAR_wbt_pBaseCcms[73]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFE); //#TVAR_wbt_pBaseCcms[74]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6D); //#TVAR_wbt_pBaseCcms[75]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B4); //#TVAR_wbt_pBaseCcms[76]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF66); //#TVAR_wbt_pBaseCcms[77]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFCA); //#TVAR_wbt_pBaseCcms[78]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFCE); //#TVAR_wbt_pBaseCcms[79]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x017B); //#TVAR_wbt_pBaseCcms[80]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0136); //#TVAR_wbt_pBaseCcms[81]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0132); //#TVAR_wbt_pBaseCcms[82]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF85); //#TVAR_wbt_pBaseCcms[83]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018B); //#TVAR_wbt_pBaseCcms[84]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF73); //#TVAR_wbt_pBaseCcms[85]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0191); //#TVAR_wbt_pBaseCcms[86]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF3F); //#TVAR_wbt_pBaseCcms[87]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015B); //#TVAR_wbt_pBaseCcms[88]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D0); //#TVAR_wbt_pBaseCcms[89]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BF); //#TVAR_wbt_pBaseCcms[90] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFBF); //#TVAR_wbt_pBaseCcms[91] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFE); //#TVAR_wbt_pBaseCcms[92] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6D); //#TVAR_wbt_pBaseCcms[93] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B4); //#TVAR_wbt_pBaseCcms[94] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF66); //#TVAR_wbt_pBaseCcms[95] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFCA); //#TVAR_wbt_pBaseCcms[96] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFCE); //#TVAR_wbt_pBaseCcms[97] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x017B); //#TVAR_wbt_pBaseCcms[98] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0136); //#TVAR_wbt_pBaseCcms[99] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0132); //#TVAR_wbt_pBaseCcms[100]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF85); //#TVAR_wbt_pBaseCcms[101]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018B); //#TVAR_wbt_pBaseCcms[102]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF73); //#TVAR_wbt_pBaseCcms[103]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0191); //#TVAR_wbt_pBaseCcms[104]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF3F); //#TVAR_wbt_pBaseCcms[105]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015B); //#TVAR_wbt_pBaseCcms[106]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D0); //#TVAR_wbt_pBaseCcms[107]
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x3380);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01C8); //#TVAR_wbt_pOutdoorCcm[0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFBF); //#TVAR_wbt_pOutdoorCcm[1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); //#TVAR_wbt_pOutdoorCcm[2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF0C); //#TVAR_wbt_pOutdoorCcm[3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0230); //#TVAR_wbt_pOutdoorCcm[4] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFEFA); //#TVAR_wbt_pOutdoorCcm[5] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006); //#TVAR_wbt_pOutdoorCcm[6] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFDE); //#TVAR_wbt_pOutdoorCcm[7] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0225); //#TVAR_wbt_pOutdoorCcm[8] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0124); //#TVAR_wbt_pOutdoorCcm[9] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010D); //#TVAR_wbt_pOutdoorCcm[10]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF21); //#TVAR_wbt_pOutdoorCcm[11]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D4); //#TVAR_wbt_pOutdoorCcm[12]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF40); //#TVAR_wbt_pOutdoorCcm[13]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0187); //#TVAR_wbt_pOutdoorCcm[14]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFEB3); //#TVAR_wbt_pOutdoorCcm[15]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014B); //#TVAR_wbt_pOutdoorCcm[16]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007B); //#TVAR_wbt_pOutdoorCcm[17]
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0612);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0128);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0166);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0193);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0498); //Indoor
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0021);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0060);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0127);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01FB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x021F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0260);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x029A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02F7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x034D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0395);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03CE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF);
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Outdoor
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0021);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0060);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0127);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01FB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x021F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0260);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x029A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02F7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x034D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0395);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03CE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF);

	// AFIT
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02DD);
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0734);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0040); // AFIT16_BRIGHTNESS
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFD0); // AFIT16_CONTRAST
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFE0); // AFIT16_SATURATION
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFE0); // AFIT16_SHARP_BLUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_GLAMOUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0078); // AFIT16_sddd8a_edge_high
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); // AFIT16_demsharpmix1_iLowBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iHighBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_demsharpmix1_iLowSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_demsharpmix1_iHighSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); // AFIT16_demsharpmix1_iLowThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_demsharpmix1_iHighThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E6); // AFIT16_demsharpmix1_iRGBOffset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_demsharpmix1_iDemClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); // AFIT16_demsharpmix1_iTune
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01FF); // AFIT16_YUV422_DENOISE_iUVLowThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0144); // AFIT16_YUV422_DENOISE_iUVHighThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F); // AFIT16_sddd8a_iClustThresh_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_Sharpening_iLowSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0087); // AFIT16_Sharpening_iHighSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT16_Sharpening_iHighSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046); // AFIT16_Sharpening_iHighSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B32); // AFIT8_sddd8a_edge_low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0601); // AFIT8_sddd8a_repl_force
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iHotThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iColdThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_AddNoisePower1
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF); // AFIT8_sddd8a_iSatSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x07FF); // AFIT8_sddd8a_iRadialLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); // AFIT8_sddd8a_iLowMaxSlopeAllowed
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iLowSlopeThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x050D); // AFIT8_Demosaicing_iDFD_ReduceCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E80); // AFIT8_Demosaicing_iCentGrad
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iGRDenoiseVal
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1408); // AFIT8_Demosaicing_iNearGrayDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0214); // AFIT8_Sharpening_iWShThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF01); // AFIT8_Sharpening_iReduceNegative
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x180F); // AFIT8_demsharpmix1_iBCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); // AFIT8_demsharpmix1_iFilterPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_demsharpmix1_iNarrMult
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A03); // AFIT8_YUV422_DENOISE_iUVSupport //SHADINGPOWER
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0094); // AFIT8_RGBGamma2_1LUT_sim_iLinearity
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0580); // AFIT8_ccm_oscar_sim_iSaturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0180); // AFIT8_YUV422_CONTROL_Y_mul
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0308); // AFIT8_sddd8a_iClustMulT_H 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3186); // AFIT8_sddd8a_DispTH_Low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x52FF); // AFIT8_sddd8a_iDenThreshLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A02); // AFIT8_Demosaicing_iDemSharpenLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080A); // AFIT8_Demosaicing_iDemSharpThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0500); // AFIT8_Demosaicing_iEdgeDesatThrLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D); // AFIT8_Demosaicing_iEdgeDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x324E); // AFIT8_Demosaicing_iDemShLowLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); // AFIT8_Sharpening_iHighSharpPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x022E); // AFIT8_Sharpening_iHighShDenoise
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x46FF); // AFIT8_sddd8a_iDenThreshLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0802); // AFIT8_Demosaicing_iDemSharpenLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0802); // AFIT8_Demosaicing_iDemSharpThresh_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3202); // AFIT8_Demosaicing_iDemShLowLimit_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x46FF); // AFIT8_sddd8a_iDenThreshLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0802); // AFIT8_Demosaicing_iDemSharpenLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0802); // AFIT8_Demosaicing_iDemSharpThresh_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3202); // AFIT8_Demosaicing_iDemShLowLimit_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_BRIGHTNESS
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_CONTRAST
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000D); // AFIT16_SATURATION
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFF0); // AFIT16_SHARP_BLUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_GLAMOUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006A); // AFIT16_sddd8a_edge_high
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); // AFIT16_demsharpmix1_iLowBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iHighBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_demsharpmix1_iLowSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_demsharpmix1_iHighSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); // AFIT16_demsharpmix1_iLowThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_demsharpmix1_iHighThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E6); // AFIT16_demsharpmix1_iRGBOffset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iDemClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); // AFIT16_demsharpmix1_iTune
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); // AFIT16_YUV422_DENOISE_iUVLowThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_YUV422_DENOISE_iUVHighThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_Sharpening_iLowSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0087); // AFIT16_Sharpening_iHighSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT16_Sharpening_iHighSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT16_Sharpening_iHighSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B32); // AFIT8_sddd8a_edge_low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0601); // AFIT8_sddd8a_repl_force
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iHotThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iColdThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_AddNoisePower1
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF); // AFIT8_sddd8a_iSatSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x07FF); // AFIT8_sddd8a_iRadialLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); // AFIT8_sddd8a_iLowMaxSlopeAllowed
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iLowSlopeThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x050D); // AFIT8_Demosaicing_iDFD_ReduceCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E80); // AFIT8_Demosaicing_iCentGrad
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iGRDenoiseVal
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1408); // AFIT8_Demosaicing_iNearGrayDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0214); // AFIT8_Sharpening_iWShThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF01); // AFIT8_Sharpening_iReduceNegative
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x180F); // AFIT8_demsharpmix1_iBCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); // AFIT8_demsharpmix1_iFilterPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_demsharpmix1_iNarrMult
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A03); // AFIT8_YUV422_DENOISE_iUVSupport //SHADINGPOWER
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_RGBGamma2_1LUT_sim_iLinearity
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_ccm_oscar_sim_iSaturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0180); // AFIT8_YUV422_CONTROL_Y_mul
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0308); // AFIT8_sddd8a_iClustMulT_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E65); // AFIT8_sddd8a_DispTH_Low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A1B); // AFIT8_sddd8a_iDenThreshLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A03); // AFIT8_Demosaicing_iDemSharpenLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080A); // AFIT8_Demosaicing_iDemSharpThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0500); // AFIT8_Demosaicing_iEdgeDesatThrLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D); // AFIT8_Demosaicing_iEdgeDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x144D); // AFIT8_Demosaicing_iDemShLowLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1805); // AFIT8_Sharpening_iHighSharpPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x021F); // AFIT8_Sharpening_iHighShDenoise
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2FFF); // AFIT8_sddd8a_iDenThreshLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0504); // AFIT8_Demosaicing_iDemSharpenLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x14FF); // AFIT8_sddd8a_iDenThreshLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0504); // AFIT8_Demosaicing_iDemSharpenLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_BRIGHTNESS
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); // AFIT16_CONTRAST
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0018); // AFIT16_SATURATION
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFF8); // AFIT16_SHARP_BLUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_GLAMOUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_sddd8a_edge_high
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); // AFIT16_demsharpmix1_iLowBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iHighBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_demsharpmix1_iLowSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_demsharpmix1_iHighSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); // AFIT16_demsharpmix1_iLowThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_demsharpmix1_iHighThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E6); // AFIT16_demsharpmix1_iRGBOffset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iDemClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); // AFIT16_demsharpmix1_iTune
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); // AFIT16_YUV422_DENOISE_iUVLowThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_YUV422_DENOISE_iUVHighThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_sddd8a_iClustThresh_C
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_Sharpening_iLowSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0087); // AFIT16_Sharpening_iHighSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0019); // AFIT16_sddd8a_iClustThresh_C_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT16_Sharpening_iHighSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0019); // AFIT16_sddd8a_iClustThresh_C_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT16_Sharpening_iHighSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B32); // AFIT8_sddd8a_edge_low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0601); // AFIT8_sddd8a_repl_force
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iHotThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iColdThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_AddNoisePower1
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF); // AFIT8_sddd8a_iSatSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x07FF); // AFIT8_sddd8a_iRadialLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); // AFIT8_sddd8a_iLowMaxSlopeAllowed
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iLowSlopeThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x050D); // AFIT8_Demosaicing_iDFD_ReduceCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E80); // AFIT8_Demosaicing_iCentGrad
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iGRDenoiseVal
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2008); // AFIT8_Demosaicing_iNearGrayDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iWShThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF01); // AFIT8_Sharpening_iReduceNegative
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x180F); // AFIT8_demsharpmix1_iBCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); // AFIT8_demsharpmix1_iFilterPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_demsharpmix1_iNarrMult
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A03); // AFIT8_YUV422_DENOISE_iUVSupport
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_RGBGamma2_1LUT_sim_iLinearity
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_ccm_oscar_sim_iSaturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0180); // AFIT8_YUV422_CONTROL_Y_mul
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0208); // AFIT8_sddd8a_iClustMulT_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E4B); // AFIT8_sddd8a_DispTH_Low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F0F); // AFIT8_sddd8a_iDenThreshLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A05); // AFIT8_Demosaicing_iDemSharpenLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080A); // AFIT8_Demosaicing_iDemSharpThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0500); // AFIT8_Demosaicing_iEdgeDesatThrLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D); // AFIT8_Demosaicing_iEdgeDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x324D); // AFIT8_Demosaicing_iDemShLowLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT8_Sharpening_iHighSharpPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iHighShDenoise
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1EFF); // AFIT8_sddd8a_iDenThreshLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0505); // AFIT8_Demosaicing_iDemSharpenLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1EFF); // AFIT8_sddd8a_iDenThreshLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0505); // AFIT8_Demosaicing_iDemSharpenLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_BRIGHTNESS
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); // AFIT16_CONTRAST
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0018); // AFIT16_SATURATION
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFF8); // AFIT16_SHARP_BLUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_GLAMOUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_sddd8a_edge_high
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); // AFIT16_demsharpmix1_iLowBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iHighBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_demsharpmix1_iLowSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_demsharpmix1_iHighSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); // AFIT16_demsharpmix1_iLowThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_demsharpmix1_iHighThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E6); // AFIT16_demsharpmix1_iRGBOffset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_demsharpmix1_iDemClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); // AFIT16_demsharpmix1_iTune
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); // AFIT16_YUV422_DENOISE_iUVLowThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_YUV422_DENOISE_iUVHighThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_sddd8a_iClustThresh_C
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_Sharpening_iLowSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009F); // AFIT16_Sharpening_iHighSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_C_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0037); // AFIT16_Sharpening_iHighSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_C_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0037); // AFIT16_Sharpening_iHighSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B32); // AFIT8_sddd8a_edge_low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0601); // AFIT8_sddd8a_repl_force
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iHotThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iColdThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_AddNoisePower1
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF); // AFIT8_sddd8a_iSatSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x07A0); // AFIT8_sddd8a_iRadialLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); // AFIT8_sddd8a_iLowMaxSlopeAllowed
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iLowSlopeThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x050D); // AFIT8_Demosaicing_iDFD_ReduceCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E80); // AFIT8_Demosaicing_iCentGrad
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iGRDenoiseVal
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2008); // AFIT8_Demosaicing_iNearGrayDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iWShThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF01); // AFIT8_Sharpening_iReduceNegative
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x180F); // AFIT8_demsharpmix1_iBCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); // AFIT8_demsharpmix1_iFilterPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_demsharpmix1_iNarrMult
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A03); // AFIT8_YUV422_DENOISE_iUVSupport
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_RGBGamma2_1LUT_sim_iLinearity
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_ccm_oscar_sim_iSaturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0180); // AFIT8_YUV422_CONTROL_Y_mul
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0108); // AFIT8_sddd8a_iClustMulT_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E32); // AFIT8_sddd8a_DispTH_Low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x090A); // AFIT8_sddd8a_iDenThreshLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A05); // AFIT8_Demosaicing_iDemSharpenLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080A); // AFIT8_Demosaicing_iDemSharpThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0328); // AFIT8_Demosaicing_iEdgeDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x324C); // AFIT8_Demosaicing_iDemShLowLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT8_Sharpening_iHighSharpPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iHighShDenoise
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FFF); // AFIT8_sddd8a_iDenThreshLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0307); // AFIT8_Demosaicing_iDemSharpenLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FFF); // AFIT8_sddd8a_iDenThreshLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0307); // AFIT8_Demosaicing_iDemSharpenLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_BRIGHTNESS
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); // AFIT16_CONTRAST
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0018); // AFIT16_SATURATION
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFF8); // AFIT16_SHARP_BLUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_GLAMOUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_edge_high
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); // AFIT16_demsharpmix1_iLowBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iHighBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_demsharpmix1_iLowSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_demsharpmix1_iHighSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); // AFIT16_demsharpmix1_iLowThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_demsharpmix1_iHighThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E6); // AFIT16_demsharpmix1_iRGBOffset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_demsharpmix1_iDemClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); // AFIT16_demsharpmix1_iTune
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0087); // AFIT16_YUV422_DENOISE_iUVLowThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_YUV422_DENOISE_iUVHighThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_sddd8a_iClustThresh_C
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_Sharpening_iLowSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B4); // AFIT16_Sharpening_iHighSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_C_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046); // AFIT16_Sharpening_iHighSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_C_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046); // AFIT16_Sharpening_iHighSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B23); // AFIT8_sddd8a_edge_low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0601); // AFIT8_sddd8a_repl_force
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iHotThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iColdThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_AddNoisePower1
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF); // AFIT8_sddd8a_iSatSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0B84); // AFIT8_sddd8a_iRadialLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); // AFIT8_sddd8a_iLowMaxSlopeAllowed
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iLowSlopeThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x050D); // AFIT8_Demosaicing_iDFD_ReduceCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E80); // AFIT8_Demosaicing_iCentGrad
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iGRDenoiseVal
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2008); // AFIT8_Demosaicing_iNearGrayDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iWShThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF01); // AFIT8_Sharpening_iReduceNegative
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x180F); // AFIT8_demsharpmix1_iBCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); // AFIT8_demsharpmix1_iFilterPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_demsharpmix1_iNarrMult
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A03); // AFIT8_YUV422_DENOISE_iUVSupport
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_RGBGamma2_1LUT_sim_iLinearity
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_ccm_oscar_sim_iSaturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0180); // AFIT8_YUV422_CONTROL_Y_mul
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0108); // AFIT8_sddd8a_iClustMulT_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E1E); // AFIT8_sddd8a_DispTH_Low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); // AFIT8_sddd8a_iDenThreshLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A0A); // AFIT8_Demosaicing_iDemSharpenLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0800); // AFIT8_Demosaicing_iDemSharpThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0328); // AFIT8_Demosaicing_iEdgeDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x324C); // AFIT8_Demosaicing_iDemShLowLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT8_Sharpening_iHighSharpPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iHighShDenoise
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6464); // AFIT8_sddd8a_DispTH_Low_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FFF); // AFIT8_sddd8a_iDenThreshLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0307); // AFIT8_Demosaicing_iDemSharpenLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6464); // AFIT8_sddd8a_DispTH_Low_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FFF); // AFIT8_sddd8a_iDenThreshLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0307); // AFIT8_Demosaicing_iDemSharpenLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7F5E);  //ConstAfitBaseVals_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFEEE);  //ConstAfitBaseVals_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD9B7);  //ConstAfitBaseVals_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0472);  //ConstAfitBaseVals_3_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  //ConstAfitBaseVals_4_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1278);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xAAF0);	//gisp_dadlc  Ladlc mode average
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0408);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x067F);	//REG_TC_DBG_AutoAlgEnBits all AA are on

	// User Control
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x018E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Brightness
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); //contrast
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); //Saturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //sharpness

	// Flicker
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0408);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x065F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x03F4); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);

//pll
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x012E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x5DC0);	// input clock=24MHz
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0146); //
	
#ifdef MIPI_INTERFACE   
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_TC_IPRM_UseNPviClocks
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001); //REG_TC_IPRM_UseNMipiClocks

	//92M for 30 fps
	 S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014C);
	 S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x2CEC); 
	 S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0152);
	 S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x59D8);
	 S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014E);
	 S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x59D8); 
#else
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001); //REG_TC_IPRM_UseNPviClocks
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_TC_IPRM_UseNMipiClocks

	//72M Pclk
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x2328);	//0x2EE0 //REG_TC_IPRM_sysClocks_0 54MHz 34BC
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0152);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x4650);	//72M //REG_TC_IPRM_MinOutRate4KHz_0 6977
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x4650); // 72M//REG_TC_IPRM_MaxOutRate4KHz_0 6978
#endif

    //72M Pclk
	//S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014C);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x2328);	//0x2EE0 //REG_TC_IPRM_sysClocks_0 54MHz 34BC
	//S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0152);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x4650);	//72M //REG_TC_IPRM_MinOutRate4KHz_0 6977
	//S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014E);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x4650); // 72M//REG_TC_IPRM_MaxOutRate4KHz_0 6978

    //58M
	//S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014C);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x38A4);	
	//S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0152);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x38A4);	
	//S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014E);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x38A4); 

    //88M 29 fps
	//S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014C);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x2AF8); 
	//S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0152);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x55F0); 
	//S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014E);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x55F0);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00FA); 


	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0164); //update PLL
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001); //REG_TC_IPRM_InitParamsUpdated	 
	
	//============================================================
	// Preview configuration 0
	//============================================================
					
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0500);	// 1280
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03C0);	// 960
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);	// YUV422 7:raw10
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01C4); 
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0042); //REG_0TC_PCFG_PVIMask 0052
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040); //negative pclk
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01C8); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_PCFG_uClockInd
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D2); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);	//REG_0TC_PCFG_usFrTimeType
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_PCFG_FrRateQualityType
	
     S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_PCFG_usMinFrTimeMsecMult10
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0D05);	//REG_0TC_PCFG_usMaxFrTimeMsecMult10
     S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01E8);
     S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_PCFG_uPrevMirror

	//============================================================
	// active preview configure
	//============================================================
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x01A8); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #REG_TC_GP_ActivePrevConfig
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x01AC); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  // #REG_TC_GP_PrevOpenAfterChange
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x01A6); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  // #REG_TC_GP_NewConfigSync
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x01AA); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  // #REG_TC_GP_PrevConfigChanged
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x019E); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  // #REG_TC_GP_EnablePreview
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  // #REG_TC_GP_EnablePreviewChanged
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x1000, 0x0001);

	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// Set host interrupt
#endif

#if 1////Supply by AVP,for Trule module 
	S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000);//ADD TEST
	

	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	// Reset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// Simmian bug workaround
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1030);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// Clear host interrupt so main will wait
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0014);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	// ARM go

	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x2470);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xB510);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4910);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4810);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFA41);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4910);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4810);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFA3D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4910);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4810);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6341);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4910);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4811);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFA36);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4910);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4811);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFA32);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4910);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4811);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFA2E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4810);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4911);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6448);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4911);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4811);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFA27);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBC10);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBC08);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4718);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2870);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8EDD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x27E8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8725);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2788);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x26DC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA6EF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x26A8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA0F1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2674);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x058F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2568);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x24F4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xAC79);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4070);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x23BC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10BC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x33A8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x14BC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE151);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE041);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x11B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0091);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1034);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE593);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0520);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0091);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1384);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1008);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE591);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE080);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE585);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4070);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x401F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F86);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE28D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0E3F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F86);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5DD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5DD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5DD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0011);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0029);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5DD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0011);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0026);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02E8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0022);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x12E4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE352);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE28D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3403);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE182);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC2B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2080);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE08C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE7B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x039E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE80F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3E0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4624);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE00E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x47B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE280);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC084);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE08C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x47B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1DC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0493);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4624);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE00E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x47B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1CC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC8B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x039C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3623);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE00E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x38B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE280);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE281);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFE7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBAFF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x401F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0250);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE310);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x123C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0DB2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x021C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE594);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE584);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4070);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0800);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0820);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4041);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE280);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x11B8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x51B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE041);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0094);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1D11);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x11C8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x21B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3FB0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE353);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x31AC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5BB2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE085);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xCBB4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE351);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1DBC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3EB4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2EB2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0193);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0092);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2811);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0194);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0092);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x11A1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1168);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4070);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0072);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2150);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x14B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE311);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0144);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEA00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3118);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5C3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE5D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1114);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x04B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x41F0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC801);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC82C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1801);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1821);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4008);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x500C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x609C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x05B4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7080);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10F4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x26B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0048);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x26B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x10F6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0043);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1C5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x41F0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE92D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE594);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE350);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0030);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE1A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2068);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE590);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE3A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0036);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE584);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4010);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE8BD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE594);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0034);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEB00);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE584);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFF9);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xEAFF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3360);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x20D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x16C8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x299C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1272);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1728);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x112C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x29A0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x122C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF200);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2340);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0E2C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF400);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0CDC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x06D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4778);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x46C0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC091);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xF004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE51F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD14C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xAC79);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0467);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2FA7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xCB1F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x058F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA0F1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B43);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8725);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6777);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8E49);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xC000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE59F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF1C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xE12F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x8EDD);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xA4B6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);                            
	                                                                        
	                                                                        
	                                                                        
	                                                                        
	                                                                        
	//============================================================          
	// Set IO driving current                                               
	//============================================================          
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x04B4);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0155); // d0~d4                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0155); // d5~d9                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1555); // gpio1~gpio3              
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0555); // HSYNC,VSYNC,PCLK,SCL,SDA 
	                                                                        
	//============================================================          
	// Analog Settings                                                      
	//============================================================          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E38);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0476);	//senHal_RegCompBiasNormSf //CDS bias
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0476);	//senHal_RegCompBiasYAv //CDS bias
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0AA0);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	//setot_bUseDigitalHbin //1-Digital, 0-Analog
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E2C);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	//senHal_bUseAnalogVerAv //2-Adding/averaging, 1-Y-Avg, 0-PLA
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0E66);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	//senHal_RegBlstEnNorm      
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1250);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); 	//senHal_Bls_nSpExpLines  
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1202);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); 	//senHal_Dblr_VcoFreqMHZ  
	                                                                        
	//ADLC Filter                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1288);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F);	//gisp_dadlc_ResetFilterValue
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1C02);	//gisp_dadlc_SteadyFilterValue
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006);	//gisp_dadlc_NResetIIrFrames

	// 05.OTP Control
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x3368);                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// Tune_TP_bReMultGainsByNvm
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0001);	// Tune_TP_bUseNvmMultGain            2 7000336A SHORT
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// Tune_TP_bCorrectBlueChMismatch     2 7000336C SHORT
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// Tune_TP_BlueGainOfs88              2 7000336E SHORT
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);	// Tune_TP_BlueGainFactor88           2 70003370 SHORT


	//ae
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0D46);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0440);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3CF0);	//lt_uMaxExp_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0444);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6590);	//lt_uMaxExp_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0448);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBB80);	//lt_uMaxExp_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x044C);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3880);	//lt_uMaxExp_3_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0450);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3CF0);	//lt_uCapMaxExp_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0454);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6590);	//lt_uCapMaxExp_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0458);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xBB80);	//lt_uCapMaxExp_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x045C);                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3880);	//lt_uCapMaxExp_3_   
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);	
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0460);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0190);	//lt_uMaxAnGain_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0280);	//lt_uMaxAnGain_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0540);	//lt_uMaxAnGain_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0B00); //max gain 0x0C00=> 12x  ;0x0BO0=>  11x     lt_uMaxAnGain_3_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //lt_uMaxDigGain
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3000); //lt_uMaxTotGain
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x042E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010E); //lt_uLimitHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F5); //lt_uLimitLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0DE0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);	//ae_Fade2BlackEnable  F2B off, F2W on
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0D40);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0043); //TVAR_ae_BrAve
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0D4E); //AE_Weight
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0303);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0303);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0403);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0304);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0403);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0304);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0403);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0304);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0303);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0303);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0201);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0202);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0202);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1326);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  //gisp_gos_Enable
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x063A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);  // #TVAR_ash_GASAlpha[0][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[0][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[0][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0104);  // #TVAR_ash_GASAlpha[0][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0104);  // #TVAR_ash_GASAlpha[1][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[1][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[1][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0132);  // #TVAR_ash_GASAlpha[1][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0104);  // #TVAR_ash_GASAlpha[2][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[2][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[2][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0132);  // #TVAR_ash_GASAlpha[2][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B4);  // #TVAR_ash_GASAlpha[3][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[3][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);  // #TVAR_ash_GASAlpha[3][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DC);  // #TVAR_ash_GASAlpha[3][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0);  // #TVAR_ash_GASAlpha[4][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8);  // #TVAR_ash_GASAlpha[4][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8);  // #TVAR_ash_GASAlpha[4][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[4][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0109);  // #TVAR_ash_GASAlpha[5][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[5][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[5][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[5][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D0);  // #TVAR_ash_GASAlpha[6][0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[6][1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[6][2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100);  // #TVAR_ash_GASAlpha[6][3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x067A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064);  // #TVAR_ash_GASBeta[0][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014);  // #TVAR_ash_GASBeta[0][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014);  // #TVAR_ash_GASBeta[0][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[0][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E);  // #TVAR_ash_GASBeta[1][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);  // #TVAR_ash_GASBeta[1][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);  // #TVAR_ash_GASBeta[1][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[1][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E);  // #TVAR_ash_GASBeta[2][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);  // #TVAR_ash_GASBeta[2][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);  // #TVAR_ash_GASBeta[2][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[2][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E);  // #TVAR_ash_GASBeta[3][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[3][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[3][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[3][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046);  // #TVAR_ash_GASBeta[4][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[4][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[4][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[4][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0032);  // #TVAR_ash_GASBeta[5][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[5][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[5][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[5][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0055);  // #TVAR_ash_GASBeta[6][0]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[6][1]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[6][2]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);  // #TVAR_ash_GASBeta[6][3]
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  //ash_bLumaMode
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0632);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F5); //TVAR_ash_CGrasAlphas_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8); //TVAR_ash_CGrasAlphas_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F8); //TVAR_ash_CGrasAlphas_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //TVAR_ash_CGrasAlphas_3_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0672);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0120); //TVAR_ash_GASOutdoorAlpha_0_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //TVAR_ash_GASOutdoorAlpha_1_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //TVAR_ash_GASOutdoorAlpha_2_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //TVAR_ash_GASOutdoorAlpha_3_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06B2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //ash_GASOutdoorBeta_0_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //ash_GASOutdoorBeta_1_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //ash_GASOutdoorBeta_2_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //ash_GASOutdoorBeta_3_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0624);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009a);  //TVAR_ash_AwbAshCord_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00d3);  //TVAR_ash_AwbAshCord_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00d4);  //TVAR_ash_AwbAshCord_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012c);  //TVAR_ash_AwbAshCord_3_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0162);  //TVAR_ash_AwbAshCord_4_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0190);  //TVAR_ash_AwbAshCord_5_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01a0);  //TVAR_ash_AwbAshCord_6_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06CC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0280);  //ash_uParabolicCenterX	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E0);  //ash_uParabolicCenterY	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06D0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000D);  //ash_uParabolicScalingA	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F);  //ash_uParabolicScalingB	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06C6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  //ash_bParabolicEstimation
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x347C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0397); //Tune_wbt_GAS_0_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x028A); //Tune_wbt_GAS_1_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E7); //Tune_wbt_GAS_2_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0179); //Tune_wbt_GAS_3_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012A); //Tune_wbt_GAS_4_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F6); //Tune_wbt_GAS_5_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6); //Tune_wbt_GAS_6_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F3); //Tune_wbt_GAS_7_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011D); //Tune_wbt_GAS_8_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0166); //Tune_wbt_GAS_9_		
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01C6); //Tune_wbt_GAS_10_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0279); //Tune_wbt_GAS_11_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0348); //Tune_wbt_GAS_12_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030B); //Tune_wbt_GAS_13_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0209); //Tune_wbt_GAS_14_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016C); //Tune_wbt_GAS_15_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010D); //Tune_wbt_GAS_16_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BF); //Tune_wbt_GAS_17_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008D); //Tune_wbt_GAS_18_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007A); //Tune_wbt_GAS_19_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0083); //Tune_wbt_GAS_20_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AC); //Tune_wbt_GAS_21_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F9); //Tune_wbt_GAS_22_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015C); //Tune_wbt_GAS_23_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E4); //Tune_wbt_GAS_24_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x028F); //Tune_wbt_GAS_25_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0295); //Tune_wbt_GAS_26_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BC); //Tune_wbt_GAS_27_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0136); //Tune_wbt_GAS_28_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C0); //Tune_wbt_GAS_29_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0074); //Tune_wbt_GAS_30_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0047); //Tune_wbt_GAS_31_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0037); //Tune_wbt_GAS_32_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0041); //Tune_wbt_GAS_33_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0062); //Tune_wbt_GAS_34_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A9); //Tune_wbt_GAS_35_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0112); //Tune_wbt_GAS_36_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0196); //Tune_wbt_GAS_37_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0227); //Tune_wbt_GAS_38_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0246); //Tune_wbt_GAS_39_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0187); //Tune_wbt_GAS_40_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FD); //Tune_wbt_GAS_41_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008F); //Tune_wbt_GAS_42_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0047); //Tune_wbt_GAS_43_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001D); //Tune_wbt_GAS_44_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013); //Tune_wbt_GAS_45_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001C); //Tune_wbt_GAS_46_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0037); //Tune_wbt_GAS_47_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0075); //Tune_wbt_GAS_48_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DD); //Tune_wbt_GAS_49_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0158); //Tune_wbt_GAS_50_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E4); //Tune_wbt_GAS_51_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0214); //Tune_wbt_GAS_52_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016A); //Tune_wbt_GAS_53_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DB); //Tune_wbt_GAS_54_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0072); //Tune_wbt_GAS_55_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002F); //Tune_wbt_GAS_56_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); //Tune_wbt_GAS_57_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Tune_wbt_GAS_58_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008); //Tune_wbt_GAS_59_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0020); //Tune_wbt_GAS_60_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0056); //Tune_wbt_GAS_61_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BB); //Tune_wbt_GAS_62_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0138); //Tune_wbt_GAS_63_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B6); //Tune_wbt_GAS_64_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0225); //Tune_wbt_GAS_65_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015F); //Tune_wbt_GAS_66_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DB); //Tune_wbt_GAS_67_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006C); //Tune_wbt_GAS_68_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C); //Tune_wbt_GAS_69_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008); //Tune_wbt_GAS_70_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); //Tune_wbt_GAS_71_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008); //Tune_wbt_GAS_72_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0017); //Tune_wbt_GAS_73_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052); //Tune_wbt_GAS_74_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2); //Tune_wbt_GAS_75_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013B); //Tune_wbt_GAS_76_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01AD); //Tune_wbt_GAS_77_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0226); //Tune_wbt_GAS_78_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x017F); //Tune_wbt_GAS_79_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EB); //Tune_wbt_GAS_80_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007E); //Tune_wbt_GAS_81_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0039); //Tune_wbt_GAS_82_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015); //Tune_wbt_GAS_83_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); //Tune_wbt_GAS_84_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0011); //Tune_wbt_GAS_85_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0029); //Tune_wbt_GAS_86_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0063); //Tune_wbt_GAS_87_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C9); //Tune_wbt_GAS_88_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0149); //Tune_wbt_GAS_89_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01C1); //Tune_wbt_GAS_90_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0261); //Tune_wbt_GAS_91_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0198); //Tune_wbt_GAS_92_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0110); //Tune_wbt_GAS_93_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A3); //Tune_wbt_GAS_94_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005B); //Tune_wbt_GAS_95_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002D); //Tune_wbt_GAS_96_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0022); //Tune_wbt_GAS_97_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002C); //Tune_wbt_GAS_98_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_99_	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008B); //Tune_wbt_GAS_100_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F0); //Tune_wbt_GAS_101_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016A); //Tune_wbt_GAS_102_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01F1); //Tune_wbt_GAS_103_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02B2); //Tune_wbt_GAS_104_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D6); //Tune_wbt_GAS_105_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014C); //Tune_wbt_GAS_106_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E2); //Tune_wbt_GAS_107_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0095); //Tune_wbt_GAS_108_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0063); //Tune_wbt_GAS_109_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0055); //Tune_wbt_GAS_110_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005E); //Tune_wbt_GAS_111_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0085); //Tune_wbt_GAS_112_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C9); //Tune_wbt_GAS_113_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); //Tune_wbt_GAS_114_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A1); //Tune_wbt_GAS_115_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0244); //Tune_wbt_GAS_116_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0340); //Tune_wbt_GAS_117_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0248); //Tune_wbt_GAS_118_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A9); //Tune_wbt_GAS_119_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013B); //Tune_wbt_GAS_120_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EF); //Tune_wbt_GAS_121_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B8); //Tune_wbt_GAS_122_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AD); //Tune_wbt_GAS_123_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B6); //Tune_wbt_GAS_124_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DF); //Tune_wbt_GAS_125_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0126); //Tune_wbt_GAS_126_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x017E); //Tune_wbt_GAS_127_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0215); //Tune_wbt_GAS_128_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02BA); //Tune_wbt_GAS_129_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03EE); //Tune_wbt_GAS_130_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02D8); //Tune_wbt_GAS_131_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0215); //Tune_wbt_GAS_132_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A1); //Tune_wbt_GAS_133_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0150); //Tune_wbt_GAS_134_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0125); //Tune_wbt_GAS_135_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0114); //Tune_wbt_GAS_136_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0122); //Tune_wbt_GAS_137_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0144); //Tune_wbt_GAS_138_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0183); //Tune_wbt_GAS_139_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E8); //Tune_wbt_GAS_140_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x029A); //Tune_wbt_GAS_141_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x035D); //Tune_wbt_GAS_142_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0235); //Tune_wbt_GAS_143_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0197); //Tune_wbt_GAS_144_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012B); //Tune_wbt_GAS_145_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F1); //Tune_wbt_GAS_146_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C7); //Tune_wbt_GAS_147_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A9); //Tune_wbt_GAS_148_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009E); //Tune_wbt_GAS_149_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A6); //Tune_wbt_GAS_150_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BF); //Tune_wbt_GAS_151_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E8); //Tune_wbt_GAS_152_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011A); //Tune_wbt_GAS_153_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0184); //Tune_wbt_GAS_154_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0227); //Tune_wbt_GAS_155_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01DA); //Tune_wbt_GAS_156_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0138); //Tune_wbt_GAS_157_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E7); //Tune_wbt_GAS_158_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2); //Tune_wbt_GAS_159_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0081); //Tune_wbt_GAS_160_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005F); //Tune_wbt_GAS_161_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0053); //Tune_wbt_GAS_162_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0059); //Tune_wbt_GAS_163_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); //Tune_wbt_GAS_164_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A2); //Tune_wbt_GAS_165_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D6); //Tune_wbt_GAS_166_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011D); //Tune_wbt_GAS_167_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A2); //Tune_wbt_GAS_168_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0198); //Tune_wbt_GAS_169_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0110); //Tune_wbt_GAS_170_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C6); //Tune_wbt_GAS_171_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0084); //Tune_wbt_GAS_172_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052); //Tune_wbt_GAS_173_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002D); //Tune_wbt_GAS_174_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); //Tune_wbt_GAS_175_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002B); //Tune_wbt_GAS_176_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0041); //Tune_wbt_GAS_177_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006E); //Tune_wbt_GAS_178_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AD); //Tune_wbt_GAS_179_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F2); //Tune_wbt_GAS_180_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0159); //Tune_wbt_GAS_181_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0168); //Tune_wbt_GAS_182_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F4); //Tune_wbt_GAS_183_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A7); //Tune_wbt_GAS_184_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0061); //Tune_wbt_GAS_185_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0032); //Tune_wbt_GAS_186_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013); //Tune_wbt_GAS_187_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000D); //Tune_wbt_GAS_188_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0012); //Tune_wbt_GAS_189_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024); //Tune_wbt_GAS_190_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_191_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008B); //Tune_wbt_GAS_192_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CD); //Tune_wbt_GAS_193_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0131); //Tune_wbt_GAS_194_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0147); //Tune_wbt_GAS_195_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E2); //Tune_wbt_GAS_196_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0090); //Tune_wbt_GAS_197_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004D); //Tune_wbt_GAS_198_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0020); //Tune_wbt_GAS_199_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006); //Tune_wbt_GAS_200_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Tune_wbt_GAS_201_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005); //Tune_wbt_GAS_202_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); //Tune_wbt_GAS_203_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0037); //Tune_wbt_GAS_204_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0078); //Tune_wbt_GAS_205_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BC); //Tune_wbt_GAS_206_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0112); //Tune_wbt_GAS_207_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0156); //Tune_wbt_GAS_208_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DB); //Tune_wbt_GAS_209_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0092); //Tune_wbt_GAS_210_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0049); //Tune_wbt_GAS_211_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F); //Tune_wbt_GAS_212_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); //Tune_wbt_GAS_213_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); //Tune_wbt_GAS_214_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007); //Tune_wbt_GAS_215_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E); //Tune_wbt_GAS_216_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0035); //Tune_wbt_GAS_217_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006F); //Tune_wbt_GAS_218_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BD); //Tune_wbt_GAS_219_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); //Tune_wbt_GAS_220_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0150); //Tune_wbt_GAS_221_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EF); //Tune_wbt_GAS_222_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0099); //Tune_wbt_GAS_223_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0056); //Tune_wbt_GAS_224_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); //Tune_wbt_GAS_225_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); //Tune_wbt_GAS_226_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0008); //Tune_wbt_GAS_227_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); //Tune_wbt_GAS_228_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); //Tune_wbt_GAS_229_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0040); //Tune_wbt_GAS_230_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); //Tune_wbt_GAS_231_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C5); //Tune_wbt_GAS_232_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0115); //Tune_wbt_GAS_233_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0174); //Tune_wbt_GAS_234_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F9); //Tune_wbt_GAS_235_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AF); //Tune_wbt_GAS_236_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006E); //Tune_wbt_GAS_237_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003D); //Tune_wbt_GAS_238_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); //Tune_wbt_GAS_239_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0012); //Tune_wbt_GAS_240_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0019); //Tune_wbt_GAS_241_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002D); //Tune_wbt_GAS_242_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0057); //Tune_wbt_GAS_243_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0095); //Tune_wbt_GAS_244_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D6); //Tune_wbt_GAS_245_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0133); //Tune_wbt_GAS_246_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A7); //Tune_wbt_GAS_247_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011C); //Tune_wbt_GAS_248_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D0); //Tune_wbt_GAS_249_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0093); //Tune_wbt_GAS_250_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0062); //Tune_wbt_GAS_251_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003D); //Tune_wbt_GAS_252_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0033); //Tune_wbt_GAS_253_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003A); //Tune_wbt_GAS_254_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0054); //Tune_wbt_GAS_255_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007A); //Tune_wbt_GAS_256_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B5); //Tune_wbt_GAS_257_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00ED); //Tune_wbt_GAS_258_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0164); //Tune_wbt_GAS_259_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01FC); //Tune_wbt_GAS_260_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015D); //Tune_wbt_GAS_261_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0102); //Tune_wbt_GAS_262_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C7); //Tune_wbt_GAS_263_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0098); //Tune_wbt_GAS_264_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0072); //Tune_wbt_GAS_265_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006C); //Tune_wbt_GAS_266_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0071); //Tune_wbt_GAS_267_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008A); //Tune_wbt_GAS_268_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B1); //Tune_wbt_GAS_269_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DE); //Tune_wbt_GAS_270_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012D); //Tune_wbt_GAS_271_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B0); //Tune_wbt_GAS_272_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0274); //Tune_wbt_GAS_273_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BD); //Tune_wbt_GAS_274_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014B); //Tune_wbt_GAS_275_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0107); //Tune_wbt_GAS_276_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D6); //Tune_wbt_GAS_277_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B8); //Tune_wbt_GAS_278_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AF); //Tune_wbt_GAS_279_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B5); //Tune_wbt_GAS_280_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CA); //Tune_wbt_GAS_281_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E9); //Tune_wbt_GAS_282_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0120); //Tune_wbt_GAS_283_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0189); //Tune_wbt_GAS_284_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0222); //Tune_wbt_GAS_285_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0236); //Tune_wbt_GAS_286_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0197); //Tune_wbt_GAS_287_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012D); //Tune_wbt_GAS_288_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F1); //Tune_wbt_GAS_289_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C5); //Tune_wbt_GAS_290_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A5); //Tune_wbt_GAS_291_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0098); //Tune_wbt_GAS_292_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A0); //Tune_wbt_GAS_293_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B9); //Tune_wbt_GAS_294_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E2); //Tune_wbt_GAS_295_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0114); //Tune_wbt_GAS_296_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x017E); //Tune_wbt_GAS_297_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x021B); //Tune_wbt_GAS_298_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E5); //Tune_wbt_GAS_299_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013E); //Tune_wbt_GAS_300_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00ED); //Tune_wbt_GAS_301_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B5); //Tune_wbt_GAS_302_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0083); //Tune_wbt_GAS_303_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005F); //Tune_wbt_GAS_304_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0052); //Tune_wbt_GAS_305_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0056); //Tune_wbt_GAS_306_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0071); //Tune_wbt_GAS_307_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009E); //Tune_wbt_GAS_308_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D5); //Tune_wbt_GAS_309_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011B); //Tune_wbt_GAS_310_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A2); //Tune_wbt_GAS_311_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A6); //Tune_wbt_GAS_312_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011A); //Tune_wbt_GAS_313_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CD); //Tune_wbt_GAS_314_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0088); //Tune_wbt_GAS_315_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0055); //Tune_wbt_GAS_316_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002E); //Tune_wbt_GAS_317_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024); //Tune_wbt_GAS_318_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_319_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0040); //Tune_wbt_GAS_320_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006D); //Tune_wbt_GAS_321_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AD); //Tune_wbt_GAS_322_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F1); //Tune_wbt_GAS_323_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0159); //Tune_wbt_GAS_324_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0176); //Tune_wbt_GAS_325_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //Tune_wbt_GAS_326_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0); //Tune_wbt_GAS_327_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0067); //Tune_wbt_GAS_328_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0036); //Tune_wbt_GAS_329_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015); //Tune_wbt_GAS_330_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E); //Tune_wbt_GAS_331_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013); //Tune_wbt_GAS_332_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024); //Tune_wbt_GAS_333_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_334_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008C); //Tune_wbt_GAS_335_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CE); //Tune_wbt_GAS_336_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0133); //Tune_wbt_GAS_337_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0158); //Tune_wbt_GAS_338_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EF); //Tune_wbt_GAS_339_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009A); //Tune_wbt_GAS_340_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0053); //Tune_wbt_GAS_341_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024); //Tune_wbt_GAS_342_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009); //Tune_wbt_GAS_343_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); //Tune_wbt_GAS_344_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007); //Tune_wbt_GAS_345_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0015); //Tune_wbt_GAS_346_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0038); //Tune_wbt_GAS_347_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0079); //Tune_wbt_GAS_348_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BD); //Tune_wbt_GAS_349_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0113); //Tune_wbt_GAS_350_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0167); //Tune_wbt_GAS_351_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E9); //Tune_wbt_GAS_352_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009D); //Tune_wbt_GAS_353_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0051); //Tune_wbt_GAS_354_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024); //Tune_wbt_GAS_355_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006); //Tune_wbt_GAS_356_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006); //Tune_wbt_GAS_357_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0009); //Tune_wbt_GAS_358_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F); //Tune_wbt_GAS_359_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0037); //Tune_wbt_GAS_360_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0072); //Tune_wbt_GAS_361_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BE); //Tune_wbt_GAS_362_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010E); //Tune_wbt_GAS_363_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0165); //Tune_wbt_GAS_364_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FE); //Tune_wbt_GAS_365_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A6); //Tune_wbt_GAS_366_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005F); //Tune_wbt_GAS_367_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002E); //Tune_wbt_GAS_368_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F); //Tune_wbt_GAS_369_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); //Tune_wbt_GAS_370_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E); //Tune_wbt_GAS_371_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001B); //Tune_wbt_GAS_372_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0043); //Tune_wbt_GAS_373_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0084); //Tune_wbt_GAS_374_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C8); //Tune_wbt_GAS_375_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0119); //Tune_wbt_GAS_376_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0189); //Tune_wbt_GAS_377_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0109); //Tune_wbt_GAS_378_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BC); //Tune_wbt_GAS_379_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0078); //Tune_wbt_GAS_380_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0044); //Tune_wbt_GAS_381_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F); //Tune_wbt_GAS_382_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016); //Tune_wbt_GAS_383_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001B); //Tune_wbt_GAS_384_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002F); //Tune_wbt_GAS_385_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0059); //Tune_wbt_GAS_386_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0097); //Tune_wbt_GAS_387_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D8); //Tune_wbt_GAS_388_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0137); //Tune_wbt_GAS_389_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BE); //Tune_wbt_GAS_390_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012D); //Tune_wbt_GAS_391_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DD); //Tune_wbt_GAS_392_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009E); //Tune_wbt_GAS_393_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006A); //Tune_wbt_GAS_394_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0043); //Tune_wbt_GAS_395_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0038); //Tune_wbt_GAS_396_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003D); //Tune_wbt_GAS_397_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0056); //Tune_wbt_GAS_398_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); //Tune_wbt_GAS_399_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B8); //Tune_wbt_GAS_400_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F0); //Tune_wbt_GAS_401_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0168); //Tune_wbt_GAS_402_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0212); //Tune_wbt_GAS_403_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016E); //Tune_wbt_GAS_404_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010F); //Tune_wbt_GAS_405_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D1); //Tune_wbt_GAS_406_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A0); //Tune_wbt_GAS_407_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0078); //Tune_wbt_GAS_408_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006F); //Tune_wbt_GAS_409_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); //Tune_wbt_GAS_410_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008C); //Tune_wbt_GAS_411_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B3); //Tune_wbt_GAS_412_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DF); //Tune_wbt_GAS_413_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012E); //Tune_wbt_GAS_414_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B3); //Tune_wbt_GAS_415_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x028E); //Tune_wbt_GAS_416_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01CE); //Tune_wbt_GAS_417_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015B); //Tune_wbt_GAS_418_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0112); //Tune_wbt_GAS_419_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E0); //Tune_wbt_GAS_420_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BE); //Tune_wbt_GAS_421_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B3); //Tune_wbt_GAS_422_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B9); //Tune_wbt_GAS_423_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CB); //Tune_wbt_GAS_424_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EB); //Tune_wbt_GAS_425_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0123); //Tune_wbt_GAS_426_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018B); //Tune_wbt_GAS_427_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0224); //Tune_wbt_GAS_428_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01F4); //Tune_wbt_GAS_429_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016A); //Tune_wbt_GAS_430_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0114); //Tune_wbt_GAS_431_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DE); //Tune_wbt_GAS_432_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B9); //Tune_wbt_GAS_433_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009D); //Tune_wbt_GAS_434_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0092); //Tune_wbt_GAS_435_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009B); //Tune_wbt_GAS_436_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2); //Tune_wbt_GAS_437_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D9); //Tune_wbt_GAS_438_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); //Tune_wbt_GAS_439_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015E); //Tune_wbt_GAS_440_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E0); //Tune_wbt_GAS_441_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BB); //Tune_wbt_GAS_442_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0120); //Tune_wbt_GAS_443_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D5); //Tune_wbt_GAS_444_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A7); //Tune_wbt_GAS_445_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007A); //Tune_wbt_GAS_446_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005B); //Tune_wbt_GAS_447_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0051); //Tune_wbt_GAS_448_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0055); //Tune_wbt_GAS_449_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); //Tune_wbt_GAS_450_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009B); //Tune_wbt_GAS_451_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CC); //Tune_wbt_GAS_452_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); //Tune_wbt_GAS_453_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016F); //Tune_wbt_GAS_454_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x017C); //Tune_wbt_GAS_455_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); //Tune_wbt_GAS_456_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B9); //Tune_wbt_GAS_457_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0079); //Tune_wbt_GAS_458_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004C); //Tune_wbt_GAS_459_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002B); //Tune_wbt_GAS_460_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); //Tune_wbt_GAS_461_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0029); //Tune_wbt_GAS_462_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003F); //Tune_wbt_GAS_463_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006C); //Tune_wbt_GAS_464_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A7); //Tune_wbt_GAS_465_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6); //Tune_wbt_GAS_466_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0133); //Tune_wbt_GAS_467_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0152); //Tune_wbt_GAS_468_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E7); //Tune_wbt_GAS_469_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A1); //Tune_wbt_GAS_470_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005C); //Tune_wbt_GAS_471_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002E); //Tune_wbt_GAS_472_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0011); //Tune_wbt_GAS_473_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); //Tune_wbt_GAS_474_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0012); //Tune_wbt_GAS_475_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); //Tune_wbt_GAS_476_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_477_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0088); //Tune_wbt_GAS_478_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C6); //Tune_wbt_GAS_479_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0111); //Tune_wbt_GAS_480_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0135); //Tune_wbt_GAS_481_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DC); //Tune_wbt_GAS_482_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008C); //Tune_wbt_GAS_483_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004D); //Tune_wbt_GAS_484_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0020); //Tune_wbt_GAS_485_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006); //Tune_wbt_GAS_486_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Tune_wbt_GAS_487_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006); //Tune_wbt_GAS_488_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); //Tune_wbt_GAS_489_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0036); //Tune_wbt_GAS_490_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0074); //Tune_wbt_GAS_491_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B5); //Tune_wbt_GAS_492_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FA); //Tune_wbt_GAS_493_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014A); //Tune_wbt_GAS_494_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D5); //Tune_wbt_GAS_495_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0091); //Tune_wbt_GAS_496_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004A); //Tune_wbt_GAS_497_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0020); //Tune_wbt_GAS_498_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005); //Tune_wbt_GAS_499_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004); //Tune_wbt_GAS_500_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0007); //Tune_wbt_GAS_501_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F); //Tune_wbt_GAS_502_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0035); //Tune_wbt_GAS_503_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006F); //Tune_wbt_GAS_504_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B7); //Tune_wbt_GAS_505_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F4); //Tune_wbt_GAS_506_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0143); //Tune_wbt_GAS_507_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00EE); //Tune_wbt_GAS_508_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0098); //Tune_wbt_GAS_509_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058); //Tune_wbt_GAS_510_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x002A); //Tune_wbt_GAS_511_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F); //Tune_wbt_GAS_512_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000B); //Tune_wbt_GAS_513_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000E); //Tune_wbt_GAS_514_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001C); //Tune_wbt_GAS_515_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0042); //Tune_wbt_GAS_516_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007F); //Tune_wbt_GAS_517_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00BF); //Tune_wbt_GAS_518_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FB); //Tune_wbt_GAS_519_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016B); //Tune_wbt_GAS_520_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F5); //Tune_wbt_GAS_521_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B0); //Tune_wbt_GAS_522_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); //Tune_wbt_GAS_523_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0040); //Tune_wbt_GAS_524_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F); //Tune_wbt_GAS_525_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0017); //Tune_wbt_GAS_526_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001D); //Tune_wbt_GAS_527_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0030); //Tune_wbt_GAS_528_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0058); //Tune_wbt_GAS_529_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0093); //Tune_wbt_GAS_530_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CF); //Tune_wbt_GAS_531_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011A); //Tune_wbt_GAS_532_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x019A); //Tune_wbt_GAS_533_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x011B); //Tune_wbt_GAS_534_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D0); //Tune_wbt_GAS_535_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0095); //Tune_wbt_GAS_536_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0065); //Tune_wbt_GAS_537_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0042); //Tune_wbt_GAS_538_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0039); //Tune_wbt_GAS_539_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003E); //Tune_wbt_GAS_540_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0057); //Tune_wbt_GAS_541_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); //Tune_wbt_GAS_542_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B2); //Tune_wbt_GAS_543_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6); //Tune_wbt_GAS_544_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0148); //Tune_wbt_GAS_545_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01F3); //Tune_wbt_GAS_546_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015C); //Tune_wbt_GAS_547_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0101); //Tune_wbt_GAS_548_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C8); //Tune_wbt_GAS_549_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009B); //Tune_wbt_GAS_550_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0077); //Tune_wbt_GAS_551_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0072); //Tune_wbt_GAS_552_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0075); //Tune_wbt_GAS_553_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x008E); //Tune_wbt_GAS_554_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B3); //Tune_wbt_GAS_555_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00DB); //Tune_wbt_GAS_556_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0124); //Tune_wbt_GAS_557_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0190); //Tune_wbt_GAS_558_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0262); //Tune_wbt_GAS_559_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01AF); //Tune_wbt_GAS_560_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0142); //Tune_wbt_GAS_561_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FC); //Tune_wbt_GAS_562_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D0); //Tune_wbt_GAS_563_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B5); //Tune_wbt_GAS_564_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00AB); //Tune_wbt_GAS_565_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B1); //Tune_wbt_GAS_566_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00C4); //Tune_wbt_GAS_567_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E2); //Tune_wbt_GAS_568_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0112); //Tune_wbt_GAS_569_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0174); //Tune_wbt_GAS_570_ 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01ED); //Tune_wbt_GAS_571_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1348);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);

	//awb
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B36);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);// awbb_IndoorGrZones_ZInfo_m_GridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B3A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00F3);// awbb_IndoorGrZones_ZInfo_m_BMin */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02CB);// awbb_IndoorGrZones_ZInfo_m_BMax */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B38);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0010);// awbb_IndoorGrZones_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0AE6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0385);// 0352 03E1 awbb_IndoorGrZones_m_BGrid_0__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03D8);// 038C 0413 awbb_IndoorGrZones_m_BGrid_0__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x032A);// 0321 039E awbb_IndoorGrZones_m_BGrid_1__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03C5);// 03A6 0416 awbb_IndoorGrZones_m_BGrid_1__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02F5);// 02EC 0367 awbb_IndoorGrZones_m_BGrid_2__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x039D);// 03A0 03F3 awbb_IndoorGrZones_m_BGrid_2__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02D3);// 02CA 032D awbb_IndoorGrZones_m_BGrid_3__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0372);// 038D 03C5 awbb_IndoorGrZones_m_BGrid_3__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02B1);// 02A8 02FD awbb_IndoorGrZones_m_BGrid_4__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x033E);// 036E 038F awbb_IndoorGrZones_m_BGrid_4__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x028A);// 0281 02D3 awbb_IndoorGrZones_m_BGrid_5__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0322);// 0344 0365 awbb_IndoorGrZones_m_BGrid_5__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0268);// 025F 02AA awbb_IndoorGrZones_m_BGrid_6__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02FD);// 0327 033E awbb_IndoorGrZones_m_BGrid_6__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0248);// 023F 028D awbb_IndoorGrZones_m_BGrid_7__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02EF);// 0302 0310 awbb_IndoorGrZones_m_BGrid_7__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x022F);// 0226 0271 awbb_IndoorGrZones_m_BGrid_8__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02D5);// 02DC 02F1 awbb_IndoorGrZones_m_BGrid_8__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0219);// 0210 025A awbb_IndoorGrZones_m_BGrid_9__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02C2);// 02B9 02D2 awbb_IndoorGrZones_m_BGrid_9__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0206);// 01FD 0249 awbb_IndoorGrZones_m_BGrid_10__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02A3);// 029A 02B9 awbb_IndoorGrZones_m_BGrid_10__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01F0);// 01E7 0238 awbb_IndoorGrZones_m_BGrid_11__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0286);// 027D 02A2 awbb_IndoorGrZones_m_BGrid_11__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01E3);// 01DA 021B awbb_IndoorGrZones_m_BGrid_12__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0268);// 025F 0289 awbb_IndoorGrZones_m_BGrid_12__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01D6);// 01CD 0200 awbb_IndoorGrZones_m_BGrid_13__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x024E);// 0245 026C awbb_IndoorGrZones_m_BGrid_13__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01DD);// 01D4 01FC awbb_IndoorGrZones_m_BGrid_14__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x022A);// 0221 024F awbb_IndoorGrZones_m_BGrid_14__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0210);// 0207 021E awbb_IndoorGrZones_m_BGrid_15__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01F2);// 01E9 022C awbb_IndoorGrZones_m_BGrid_15__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_16__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_16__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_17__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_17__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_18__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_18__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_19__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_IndoorGrZones_m_BGrid_19__m_right */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BAA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0006);// awbb_LowBrGrZones_ZInfo_m_GridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BAE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00CC);// 010E awbb_LowBrGrZones_ZInfo_m_BMin */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02F3);// 02E9 awbb_LowBrGrZones_ZInfo_m_BMax */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BAC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x000A);// 0009 awbb_LowBrGrZones_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B7A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x036C);// 0374 038C awbb_LowBrGrZones_m_BGrid_0__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03C6);// 03CE 03DA awbb_LowBrGrZones_m_BGrid_0__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02EE);// 02F6 030E awbb_LowBrGrZones_m_BGrid_1__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03F9);// 0401 03E9 awbb_LowBrGrZones_m_BGrid_1__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02BE);// 02C6 02A2 awbb_LowBrGrZones_m_BGrid_2__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03DF);// 03E7 03C2 awbb_LowBrGrZones_m_BGrid_2__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x027A);// 0282 0259 awbb_LowBrGrZones_m_BGrid_3__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03AE);// 03B6 038A awbb_LowBrGrZones_m_BGrid_3__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0234);// 023C 0218 awbb_LowBrGrZones_m_BGrid_4__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0376);// 037E 0352 awbb_LowBrGrZones_m_BGrid_4__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0204);// 020C 01F4 awbb_LowBrGrZones_m_BGrid_5__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x033E);// 0346 02E1 awbb_LowBrGrZones_m_BGrid_5__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01E0);// 01E8 01D7 awbb_LowBrGrZones_m_BGrid_6__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02CD);// 02D5 028E awbb_LowBrGrZones_m_BGrid_6__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01C3);// 01CB 01CB awbb_LowBrGrZones_m_BGrid_7__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x027A);// 0282 0258 awbb_LowBrGrZones_m_BGrid_7__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01B7);// 01BF 022B awbb_LowBrGrZones_m_BGrid_8__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0244);// 024C 01CC awbb_LowBrGrZones_m_BGrid_8__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01FE);// 01F8 0000 awbb_LowBrGrZones_m_BGrid_9__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01DD);// 0201 0000 awbb_LowBrGrZones_m_BGrid_9__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_LowBrGrZones_m_BGrid_10__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_LowBrGrZones_m_BGrid_10__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_LowBrGrZones_m_BGrid_11__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 0000 awbb_LowBrGrZones_m_BGrid_11__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B70);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);// awbb_OutdoorGrZones_ZInfo_m_GridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B74);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01E3);// awbb_OutdoorGrZones_ZInfo_m_BMin */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0270);// awbb_OutdoorGrZones_ZInfo_m_BMax */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B72);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0006);// awbb_OutdoorGrZones_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0B40);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x028A);// 029E awbb_OutdoorGrZones_m_BGrid_0__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02A1);// 02C8 awbb_OutdoorGrZones_m_BGrid_0__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0263);// 0281 awbb_OutdoorGrZones_m_BGrid_1__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02C0);// 02C8 awbb_OutdoorGrZones_m_BGrid_1__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x024C);// 0266 awbb_OutdoorGrZones_m_BGrid_2__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02BE);// 02AC awbb_OutdoorGrZones_m_BGrid_2__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x023D);// 0251 awbb_OutdoorGrZones_m_BGrid_3__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02A6);// 028E awbb_OutdoorGrZones_m_BGrid_3__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0243);// 023D awbb_OutdoorGrZones_m_BGrid_4__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0289);// 0275 awbb_OutdoorGrZones_m_BGrid_4__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x026F);// 0228 awbb_OutdoorGrZones_m_BGrid_5__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x025D);// 025D awbb_OutdoorGrZones_m_BGrid_5__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0228 awbb_OutdoorGrZones_m_BGrid_6__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0243 awbb_OutdoorGrZones_m_BGrid_6__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_7__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_7__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_8__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_8__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_9__m_left   */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_9__m_right  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_10__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_10__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_11__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_OutdoorGrZones_m_BGrid_11__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BC8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);// awbb_CWSkinZone_ZInfo_m_GridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BCC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x010F);// awbb_CWSkinZone_ZInfo_m_BMin */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x018F);// awbb_CWSkinZone_ZInfo_m_BMax */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BCA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);// awbb_CWSkinZone_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BB4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03E7);// awbb_CWSkinZone_m_BGrid_0__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03F8);// awbb_CWSkinZone_m_BGrid_0__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03A7);// awbb_CWSkinZone_m_BGrid_1__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03FC);// awbb_CWSkinZone_m_BGrid_1__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0352);// awbb_CWSkinZone_m_BGrid_2__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03D0);// awbb_CWSkinZone_m_BGrid_2__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0322);// awbb_CWSkinZone_m_BGrid_3__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x039E);// awbb_CWSkinZone_m_BGrid_3__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x032B);// awbb_CWSkinZone_m_BGrid_4__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x034D);// awbb_CWSkinZone_m_BGrid_4__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BE6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0006);// awbb_DLSkinZone_ZInfo_m_GridStep */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BEA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x019E);// awbb_DLSkinZone_ZInfo_m_BMin */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0257);// awbb_DLSkinZone_ZInfo_m_BMax */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BE8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0004);// awbb_DLSkinZone_ZInfo_m_GridSz */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BD2);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x030B);// awbb_DLSkinZone_m_BGrid_0__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0323);// awbb_DLSkinZone_m_BGrid_0__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02C3);// awbb_DLSkinZone_m_BGrid_1__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x030F);// awbb_DLSkinZone_m_BGrid_1__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0288);// awbb_DLSkinZone_m_BGrid_2__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02E5);// awbb_DLSkinZone_m_BGrid_2__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x026A);// awbb_DLSkinZone_m_BGrid_3__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02A2);// awbb_DLSkinZone_m_BGrid_3__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_DLSkinZone_m_BGrid_4__m_left  */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_DLSkinZone_m_BGrid_4__m_right */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C2C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0139);// awbb_IntcR */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0122);// awbb_IntcB */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BFC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0378);// 03AD awbb_IndoorWP_0__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x011E);// 013F awbb_IndoorWP_0__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02F0);// 0341 awbb_IndoorWP_1__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0184);// 017B awbb_IndoorWP_1__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0313);// 038D awbb_IndoorWP_2__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0158);// 014B awbb_IndoorWP_2__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02BA);// 02C3 awbb_IndoorWP_3__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01BA);// 01CC awbb_IndoorWP_3__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0231);// 0241 awbb_IndoorWP_4__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0252);// 027F awbb_IndoorWP_4__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0237);// 0241 awbb_IndoorWP_5__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x024C);// 027F awbb_IndoorWP_5__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x020F);// 0214 awbb_IndoorWP_6__r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0279);// 02A8 awbb_IndoorWP_6__b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0268);// 0270 255 awbb_OutdoorWP_r */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x021A);// 0210 25B awbb_OutdoorWP_b */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C4C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0450);// awbb_MvEq_RBthresh */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C58);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x059C);// awbb_MvEq_RBthresh */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0BF8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x01AE);// awbb_LowTSep_m_RminusB */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C28);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_SkinPreference */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0CAC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0050);// awbb_OutDMaxIncr */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0C28);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_SkinPreference */ 
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x20BA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0006);// Lowtemp bypass */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0D0E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00B8);// awbb_GridCoeff_R_2 */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00B2);// awbb_GridCoeff_B_2 */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0CFE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0FAB);// 0FAB awbb_GridConst_2_0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0FF5);// 0FF5 0FF5 awbb_GridConst_2_1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x10BB);// 10BB 10BB awbb_GridConst_2_2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x1117);// 1117 1123 1153 awbb_GridConst_2_3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x116D);// 116D 11C5 awbb_GridConst_2_4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x11D5);// 122A awbb_GridConst_2_5_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00A9);// awbb_GridCoeff_R_1 */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x00C0);// awbb_GridCoeff_B_1 */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0CF8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x02CC);// awbb_GridConst_1_0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x031E);// awbb_GridConst_1_1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0359);// awbb_GridConst_1_2_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0CB0);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0030);// 0000 awbb_GridCorr_R_0__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 0000 awbb_GridCorr_R_0__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0060);// 0078 awbb_GridCorr_R_0__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 00AA awbb_GridCorr_R_0__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0008);// 0000 awbb_GridCorr_R_0__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_R_0__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0030);// 0000 awbb_GridCorr_R_1__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 0096 awbb_GridCorr_R_1__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0060);// 0000 awbb_GridCorr_R_1__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 0000 awbb_GridCorr_R_1__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0008);// 0000 awbb_GridCorr_R_1__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_R_1__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0030);// 00E6 awbb_GridCorr_R_2__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 0000 awbb_GridCorr_R_2__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0060);// 0000 awbb_GridCorr_R_2__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0040);// 0000 awbb_GridCorr_R_2__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0008);// 0000 awbb_GridCorr_R_2__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_R_2__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0018);// 0000 awbb_GridCorr_B_0__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_0__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0064 awbb_GridCorr_B_0__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_0__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFF80);// 0000 awbb_GridCorr_B_0__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_0__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0018);// 0000 awbb_GridCorr_B_1__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0032 awbb_GridCorr_B_1__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_1__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_1__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFF80);// FF38 awbb_GridCorr_B_1__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_1__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0018);// 0000 awbb_GridCorr_B_2__0_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0032 awbb_GridCorr_B_2__1_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_2__2_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_2__3_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0xFF80);// 0000 awbb_GridCorr_B_2__4_ */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// 0000 awbb_GridCorr_B_2__5_ */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0D30);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);// awbb_GridEnable */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x3362);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);// awbb_bUseOutdoorGrid */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_OutdoorGridCorr_R */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// awbb_OutdoorGridCorr_B */
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0C86);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0005);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0C70);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF7B);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00CE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF23);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFEF3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFED7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFEBB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0162);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1388);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0C8A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x4ACB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0C88);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A7C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0538); //LutPreDemNoBin
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0035);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0095);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E6);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0121);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0139);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0150);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0177);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x019A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01DC);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0219);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0251);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02B3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x035F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03B1);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //LutPostDemNoBin
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0012);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0016);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0024);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0031);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x003E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x004E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0075);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00A8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0126);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0272);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0334);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x33A4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D0); //#TVAR_wbt_pBaseCcms[0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFA1); //#TVAR_wbt_pBaseCcms[1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFA); //#TVAR_wbt_pBaseCcms[2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6F); //#TVAR_wbt_pBaseCcms[3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0140); //#TVAR_wbt_pBaseCcms[4] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF49); //#TVAR_wbt_pBaseCcms[5] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC1); //#TVAR_wbt_pBaseCcms[6] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F); //#TVAR_wbt_pBaseCcms[7] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BD); //#TVAR_wbt_pBaseCcms[8] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013F); //#TVAR_wbt_pBaseCcms[9] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E1); //#TVAR_wbt_pBaseCcms[10]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF43); //#TVAR_wbt_pBaseCcms[11]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0191); //#TVAR_wbt_pBaseCcms[12]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC0); //#TVAR_wbt_pBaseCcms[13]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B7); //#TVAR_wbt_pBaseCcms[14]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF30); //#TVAR_wbt_pBaseCcms[15]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015F); //#TVAR_wbt_pBaseCcms[16]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0106); //#TVAR_wbt_pBaseCcms[17]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D0); //#TVAR_wbt_pBaseCcms[18]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFA1); //#TVAR_wbt_pBaseCcms[19]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFA); //#TVAR_wbt_pBaseCcms[20]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6F); //#TVAR_wbt_pBaseCcms[21]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0140); //#TVAR_wbt_pBaseCcms[22]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF49); //#TVAR_wbt_pBaseCcms[23]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC1); //#TVAR_wbt_pBaseCcms[24]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F); //#TVAR_wbt_pBaseCcms[25]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BD); //#TVAR_wbt_pBaseCcms[26]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013F); //#TVAR_wbt_pBaseCcms[27]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E1); //#TVAR_wbt_pBaseCcms[28]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF43); //#TVAR_wbt_pBaseCcms[29]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0191); //#TVAR_wbt_pBaseCcms[30]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC0); //#TVAR_wbt_pBaseCcms[31]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B7); //#TVAR_wbt_pBaseCcms[32]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF30); //#TVAR_wbt_pBaseCcms[33]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015F); //#TVAR_wbt_pBaseCcms[34]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0106); //#TVAR_wbt_pBaseCcms[35]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D0); //#TVAR_wbt_pBaseCcms[36]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFA1); //#TVAR_wbt_pBaseCcms[37]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFA); //#TVAR_wbt_pBaseCcms[38]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6F); //#TVAR_wbt_pBaseCcms[39]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0140); //#TVAR_wbt_pBaseCcms[40]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF49); //#TVAR_wbt_pBaseCcms[41]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC1); //#TVAR_wbt_pBaseCcms[42]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001F); //#TVAR_wbt_pBaseCcms[43]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01BD); //#TVAR_wbt_pBaseCcms[44]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x013F); //#TVAR_wbt_pBaseCcms[45]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00E1); //#TVAR_wbt_pBaseCcms[46]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF43); //#TVAR_wbt_pBaseCcms[47]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0191); //#TVAR_wbt_pBaseCcms[48]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC0); //#TVAR_wbt_pBaseCcms[49]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01B7); //#TVAR_wbt_pBaseCcms[50]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF30); //#TVAR_wbt_pBaseCcms[51]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015F); //#TVAR_wbt_pBaseCcms[52]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0106); //#TVAR_wbt_pBaseCcms[53]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0165); //#TVAR_wbt_pBaseCcms[54]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFF7); //#TVAR_wbt_pBaseCcms[55]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0027); //#TVAR_wbt_pBaseCcms[56]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6D); //#TVAR_wbt_pBaseCcms[57]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016A); //#TVAR_wbt_pBaseCcms[58]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF52); //#TVAR_wbt_pBaseCcms[59]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC4); //#TVAR_wbt_pBaseCcms[60]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //#TVAR_wbt_pBaseCcms[61]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A3); //#TVAR_wbt_pBaseCcms[62]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010F); //#TVAR_wbt_pBaseCcms[63]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D8); //#TVAR_wbt_pBaseCcms[64]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFAA); //#TVAR_wbt_pBaseCcms[65]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018E); //#TVAR_wbt_pBaseCcms[66]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFA2); //#TVAR_wbt_pBaseCcms[67]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A8); //#TVAR_wbt_pBaseCcms[68]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF34); //#TVAR_wbt_pBaseCcms[69]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015C); //#TVAR_wbt_pBaseCcms[70]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F1); //#TVAR_wbt_pBaseCcms[71]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0165); //#TVAR_wbt_pBaseCcms[72]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFF7); //#TVAR_wbt_pBaseCcms[73]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0027); //#TVAR_wbt_pBaseCcms[74]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6D); //#TVAR_wbt_pBaseCcms[75]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016A); //#TVAR_wbt_pBaseCcms[76]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF52); //#TVAR_wbt_pBaseCcms[77]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC4); //#TVAR_wbt_pBaseCcms[78]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //#TVAR_wbt_pBaseCcms[79]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A3); //#TVAR_wbt_pBaseCcms[80]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010F); //#TVAR_wbt_pBaseCcms[81]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D8); //#TVAR_wbt_pBaseCcms[82]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFAA); //#TVAR_wbt_pBaseCcms[83]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018E); //#TVAR_wbt_pBaseCcms[84]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFA2); //#TVAR_wbt_pBaseCcms[85]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A8); //#TVAR_wbt_pBaseCcms[86]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF34); //#TVAR_wbt_pBaseCcms[87]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015C); //#TVAR_wbt_pBaseCcms[88]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00F1); //#TVAR_wbt_pBaseCcms[89]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01F0); //#TVAR_wbt_pBaseCcms[90] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFBA); //#TVAR_wbt_pBaseCcms[91] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFB); //#TVAR_wbt_pBaseCcms[92] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF6D); //#TVAR_wbt_pBaseCcms[93] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A7); //#TVAR_wbt_pBaseCcms[94] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF62); //#TVAR_wbt_pBaseCcms[95] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFC9); //#TVAR_wbt_pBaseCcms[96] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFD6); //#TVAR_wbt_pBaseCcms[97] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0181); //#TVAR_wbt_pBaseCcms[98] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015A); //#TVAR_wbt_pBaseCcms[99] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0120); //#TVAR_wbt_pBaseCcms[100]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF60); //#TVAR_wbt_pBaseCcms[101]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x018B); //#TVAR_wbt_pBaseCcms[102]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF7A); //#TVAR_wbt_pBaseCcms[103]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0194); //#TVAR_wbt_pBaseCcms[104]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF3D); //#TVAR_wbt_pBaseCcms[105]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x015B); //#TVAR_wbt_pBaseCcms[106]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D5); //#TVAR_wbt_pBaseCcms[107]
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x3380);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01C8); //#TVAR_wbt_pOutdoorCcm[0] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFBF); //#TVAR_wbt_pOutdoorCcm[1] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); //#TVAR_wbt_pOutdoorCcm[2] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF0C); //#TVAR_wbt_pOutdoorCcm[3] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0230); //#TVAR_wbt_pOutdoorCcm[4] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFEFA); //#TVAR_wbt_pOutdoorCcm[5] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0006); //#TVAR_wbt_pOutdoorCcm[6] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFDE); //#TVAR_wbt_pOutdoorCcm[7] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0225); //#TVAR_wbt_pOutdoorCcm[8] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0124); //#TVAR_wbt_pOutdoorCcm[9] 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010D); //#TVAR_wbt_pOutdoorCcm[10]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF21); //#TVAR_wbt_pOutdoorCcm[11]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D4); //#TVAR_wbt_pOutdoorCcm[12]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF40); //#TVAR_wbt_pOutdoorCcm[13]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0187); //#TVAR_wbt_pOutdoorCcm[14]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFEB3); //#TVAR_wbt_pOutdoorCcm[15]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014B); //#TVAR_wbt_pOutdoorCcm[16]
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007B); //#TVAR_wbt_pOutdoorCcm[17]
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0612);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0128);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0166);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0193);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0498); //Indoor
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0021);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0060);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0127);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01FB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x021F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0260);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x029A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02F7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x034D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0395);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03CE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF);
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Outdoor
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0021);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0060);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0127);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x014C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01A5);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01D3);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01FB);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x021F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0260);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x029A);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02F7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x034D);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0395);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03CE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF);

	// AFIT
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x06D4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0013);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x005C);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B7);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x016E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x02DD);
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0734);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0040); // AFIT16_BRIGHTNESS
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFD0); // AFIT16_CONTRAST
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_SATURATION
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_SHARP_BLUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_GLAMOUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0078); // AFIT16_sddd8a_edge_high
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); // AFIT16_demsharpmix1_iLowBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iHighBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_demsharpmix1_iLowSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_demsharpmix1_iHighSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); // AFIT16_demsharpmix1_iLowThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_demsharpmix1_iHighThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E6); // AFIT16_demsharpmix1_iRGBOffset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_demsharpmix1_iDemClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); // AFIT16_demsharpmix1_iTune
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01FF); // AFIT16_YUV422_DENOISE_iUVLowThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0144); // AFIT16_YUV422_DENOISE_iUVHighThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000F); // AFIT16_sddd8a_iClustThresh_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_Sharpening_iLowSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0087); // AFIT16_Sharpening_iHighSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT16_Sharpening_iHighSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046); // AFIT16_Sharpening_iHighSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B32); // AFIT8_sddd8a_edge_low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0601); // AFIT8_sddd8a_repl_force
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iHotThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iColdThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_AddNoisePower1
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF); // AFIT8_sddd8a_iSatSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x07FF); // AFIT8_sddd8a_iRadialLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); // AFIT8_sddd8a_iLowMaxSlopeAllowed
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iLowSlopeThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x050D); // AFIT8_Demosaicing_iDFD_ReduceCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E80); // AFIT8_Demosaicing_iCentGrad
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iGRDenoiseVal
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1408); // AFIT8_Demosaicing_iNearGrayDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0214); // AFIT8_Sharpening_iWShThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF01); // AFIT8_Sharpening_iReduceNegative
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x180F); // AFIT8_demsharpmix1_iBCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); // AFIT8_demsharpmix1_iFilterPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_demsharpmix1_iNarrMult
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A03); // AFIT8_YUV422_DENOISE_iUVSupport //SHADINGPOWER
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0094); // AFIT8_RGBGamma2_1LUT_sim_iLinearity
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0580); // AFIT8_ccm_oscar_sim_iSaturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0180); // AFIT8_YUV422_CONTROL_Y_mul
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0308); // AFIT8_sddd8a_iClustMulT_H 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3186); // AFIT8_sddd8a_DispTH_Low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xAFFF); // AFIT8_sddd8a_iDenThreshLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A02); // AFIT8_Demosaicing_iDemSharpenLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080A); // AFIT8_Demosaicing_iDemSharpThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0500); // AFIT8_Demosaicing_iEdgeDesatThrLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D); // AFIT8_Demosaicing_iEdgeDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x324E); // AFIT8_Demosaicing_iDemShLowLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); // AFIT8_Sharpening_iHighSharpPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x022E); // AFIT8_Sharpening_iHighShDenoise
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x46FF); // AFIT8_sddd8a_iDenThreshLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0802); // AFIT8_Demosaicing_iDemSharpenLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0802); // AFIT8_Demosaicing_iDemSharpThresh_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3202); // AFIT8_Demosaicing_iDemShLowLimit_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x46FF); // AFIT8_sddd8a_iDenThreshLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0802); // AFIT8_Demosaicing_iDemSharpenLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0802); // AFIT8_Demosaicing_iDemSharpThresh_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3202); // AFIT8_Demosaicing_iDemShLowLimit_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_BRIGHTNESS
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_CONTRAST
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_SATURATION
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_SHARP_BLUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_GLAMOUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x006A); // AFIT16_sddd8a_edge_high
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); // AFIT16_demsharpmix1_iLowBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iHighBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_demsharpmix1_iLowSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_demsharpmix1_iHighSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); // AFIT16_demsharpmix1_iLowThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_demsharpmix1_iHighThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E6); // AFIT16_demsharpmix1_iRGBOffset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iDemClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); // AFIT16_demsharpmix1_iTune
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); // AFIT16_YUV422_DENOISE_iUVLowThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_YUV422_DENOISE_iUVHighThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_Sharpening_iLowSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0087); // AFIT16_Sharpening_iHighSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT16_Sharpening_iHighSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000A); // AFIT16_sddd8a_iClustThresh_C_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT16_Sharpening_iHighSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B32); // AFIT8_sddd8a_edge_low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0601); // AFIT8_sddd8a_repl_force
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iHotThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iColdThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_AddNoisePower1
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF); // AFIT8_sddd8a_iSatSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x07FF); // AFIT8_sddd8a_iRadialLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); // AFIT8_sddd8a_iLowMaxSlopeAllowed
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iLowSlopeThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x050D); // AFIT8_Demosaicing_iDFD_ReduceCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E80); // AFIT8_Demosaicing_iCentGrad
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iGRDenoiseVal
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1408); // AFIT8_Demosaicing_iNearGrayDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0214); // AFIT8_Sharpening_iWShThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF01); // AFIT8_Sharpening_iReduceNegative
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x180F); // AFIT8_demsharpmix1_iBCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); // AFIT8_demsharpmix1_iFilterPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_demsharpmix1_iNarrMult
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A03); // AFIT8_YUV422_DENOISE_iUVSupport //SHADINGPOWER
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_RGBGamma2_1LUT_sim_iLinearity
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_ccm_oscar_sim_iSaturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0180); // AFIT8_YUV422_CONTROL_Y_mul
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0308); // AFIT8_sddd8a_iClustMulT_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E65); // AFIT8_sddd8a_DispTH_Low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7795); // AFIT8_sddd8a_iDenThreshLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A03); // AFIT8_Demosaicing_iDemSharpenLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080A); // AFIT8_Demosaicing_iDemSharpThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0500); // AFIT8_Demosaicing_iEdgeDesatThrLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D); // AFIT8_Demosaicing_iEdgeDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x144D); // AFIT8_Demosaicing_iDemShLowLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1805); // AFIT8_Sharpening_iHighSharpPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x021F); // AFIT8_Sharpening_iHighShDenoise
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2FFF); // AFIT8_sddd8a_iDenThreshLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0504); // AFIT8_Demosaicing_iDemSharpenLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x14FF); // AFIT8_sddd8a_iDenThreshLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0504); // AFIT8_Demosaicing_iDemSharpenLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_BRIGHTNESS
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); // AFIT16_CONTRAST
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_SATURATION
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_SHARP_BLUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_GLAMOUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_sddd8a_edge_high
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); // AFIT16_demsharpmix1_iLowBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iHighBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_demsharpmix1_iLowSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_demsharpmix1_iHighSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); // AFIT16_demsharpmix1_iLowThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_demsharpmix1_iHighThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E6); // AFIT16_demsharpmix1_iRGBOffset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iDemClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); // AFIT16_demsharpmix1_iTune
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); // AFIT16_YUV422_DENOISE_iUVLowThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_YUV422_DENOISE_iUVHighThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_sddd8a_iClustThresh_C
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_Sharpening_iLowSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0087); // AFIT16_Sharpening_iHighSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0019); // AFIT16_sddd8a_iClustThresh_C_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT16_Sharpening_iHighSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_sddd8a_iClustThresh_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0019); // AFIT16_sddd8a_iClustThresh_C_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT16_Sharpening_iHighSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B32); // AFIT8_sddd8a_edge_low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0601); // AFIT8_sddd8a_repl_force
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iHotThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iColdThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_AddNoisePower1
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF); // AFIT8_sddd8a_iSatSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x07FF); // AFIT8_sddd8a_iRadialLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); // AFIT8_sddd8a_iLowMaxSlopeAllowed
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iLowSlopeThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x050D); // AFIT8_Demosaicing_iDFD_ReduceCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E80); // AFIT8_Demosaicing_iCentGrad
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iGRDenoiseVal
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2008); // AFIT8_Demosaicing_iNearGrayDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iWShThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF01); // AFIT8_Sharpening_iReduceNegative
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x180F); // AFIT8_demsharpmix1_iBCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0002); // AFIT8_demsharpmix1_iFilterPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_demsharpmix1_iNarrMult
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A03); // AFIT8_YUV422_DENOISE_iUVSupport
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_RGBGamma2_1LUT_sim_iLinearity
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_ccm_oscar_sim_iSaturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0180); // AFIT8_YUV422_CONTROL_Y_mul
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0208); // AFIT8_sddd8a_iClustMulT_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E4B); // AFIT8_sddd8a_DispTH_Low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6C86); // AFIT8_sddd8a_iDenThreshLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A05); // AFIT8_Demosaicing_iDemSharpenLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080A); // AFIT8_Demosaicing_iDemSharpThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0500); // AFIT8_Demosaicing_iEdgeDesatThrLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x032D); // AFIT8_Demosaicing_iEdgeDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x324D); // AFIT8_Demosaicing_iDemShLowLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT8_Sharpening_iHighSharpPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iHighShDenoise
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1EFF); // AFIT8_sddd8a_iDenThreshLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0505); // AFIT8_Demosaicing_iDemSharpenLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1EFF); // AFIT8_sddd8a_iDenThreshLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0505); // AFIT8_Demosaicing_iDemSharpenLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_BRIGHTNESS
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); // AFIT16_CONTRAST
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_SATURATION
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_SHARP_BLUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_GLAMOUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_sddd8a_edge_high
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); // AFIT16_demsharpmix1_iLowBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iHighBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_demsharpmix1_iLowSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_demsharpmix1_iHighSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); // AFIT16_demsharpmix1_iLowThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_demsharpmix1_iHighThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E6); // AFIT16_demsharpmix1_iRGBOffset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_demsharpmix1_iDemClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); // AFIT16_demsharpmix1_iTune
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x007D); // AFIT16_YUV422_DENOISE_iUVLowThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_YUV422_DENOISE_iUVHighThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_sddd8a_iClustThresh_C
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_Sharpening_iLowSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x009F); // AFIT16_Sharpening_iHighSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_C_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0037); // AFIT16_Sharpening_iHighSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_C_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0037); // AFIT16_Sharpening_iHighSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B32); // AFIT8_sddd8a_edge_low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0601); // AFIT8_sddd8a_repl_force
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iHotThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iColdThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_AddNoisePower1
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF); // AFIT8_sddd8a_iSatSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x07A0); // AFIT8_sddd8a_iRadialLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); // AFIT8_sddd8a_iLowMaxSlopeAllowed
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iLowSlopeThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x050D); // AFIT8_Demosaicing_iDFD_ReduceCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E80); // AFIT8_Demosaicing_iCentGrad
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iGRDenoiseVal
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2008); // AFIT8_Demosaicing_iNearGrayDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iWShThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF01); // AFIT8_Sharpening_iReduceNegative
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x180F); // AFIT8_demsharpmix1_iBCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); // AFIT8_demsharpmix1_iFilterPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_demsharpmix1_iNarrMult
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A03); // AFIT8_YUV422_DENOISE_iUVSupport
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_RGBGamma2_1LUT_sim_iLinearity
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_ccm_oscar_sim_iSaturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0180); // AFIT8_YUV422_CONTROL_Y_mul
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0108); // AFIT8_sddd8a_iClustMulT_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E32); // AFIT8_sddd8a_DispTH_Low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x665E); // AFIT8_sddd8a_iDenThreshLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A05); // AFIT8_Demosaicing_iDemSharpenLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080A); // AFIT8_Demosaicing_iDemSharpThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0328); // AFIT8_Demosaicing_iEdgeDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x324C); // AFIT8_Demosaicing_iDemShLowLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT8_Sharpening_iHighSharpPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iHighShDenoise
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FFF); // AFIT8_sddd8a_iDenThreshLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0307); // AFIT8_Demosaicing_iDemSharpenLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x9696); // AFIT8_sddd8a_DispTH_Low_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FFF); // AFIT8_sddd8a_iDenThreshLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0307); // AFIT8_Demosaicing_iDemSharpenLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_BRIGHTNESS
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001A); // AFIT16_CONTRAST
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_SATURATION
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_SHARP_BLUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_GLAMOUR
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_edge_high
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x012C); // AFIT16_demsharpmix1_iLowBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x03FF); // AFIT16_demsharpmix1_iHighBright
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0014); // AFIT16_demsharpmix1_iLowSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0064); // AFIT16_demsharpmix1_iHighSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x000C); // AFIT16_demsharpmix1_iLowThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); // AFIT16_demsharpmix1_iHighThreshold
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x01E6); // AFIT16_demsharpmix1_iRGBOffset
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT16_demsharpmix1_iDemClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0070); // AFIT16_demsharpmix1_iTune
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0087); // AFIT16_YUV422_DENOISE_iUVLowThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_YUV422_DENOISE_iUVHighThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_sddd8a_iClustThresh_C
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0073); // AFIT16_Sharpening_iLowSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00B4); // AFIT16_Sharpening_iHighSharpClamp
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_C_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046); // AFIT16_Sharpening_iHighSharpClamp_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0028); // AFIT16_sddd8a_iClustThresh_C_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0023); // AFIT16_Sharpening_iLowSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0046); // AFIT16_Sharpening_iHighSharpClamp_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2B23); // AFIT8_sddd8a_edge_low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0601); // AFIT8_sddd8a_repl_force
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iHotThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iColdThreshHigh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_AddNoisePower1
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x00FF); // AFIT8_sddd8a_iSatSat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0B84); // AFIT8_sddd8a_iRadialLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFFFF); // AFIT8_sddd8a_iLowMaxSlopeAllowed
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_sddd8a_iLowSlopeThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x050D); // AFIT8_Demosaicing_iDFD_ReduceCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E80); // AFIT8_Demosaicing_iCentGrad
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iGRDenoiseVal
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x2008); // AFIT8_Demosaicing_iNearGrayDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iWShThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFF01); // AFIT8_Sharpening_iReduceNegative
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x180F); // AFIT8_demsharpmix1_iBCoeff
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); // AFIT8_demsharpmix1_iFilterPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_demsharpmix1_iNarrMult
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3A03); // AFIT8_YUV422_DENOISE_iUVSupport
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_RGBGamma2_1LUT_sim_iLinearity
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); // AFIT8_ccm_oscar_sim_iSaturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0180); // AFIT8_YUV422_CONTROL_Y_mul
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0108); // AFIT8_sddd8a_iClustMulT_H
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x1E1E); // AFIT8_sddd8a_DispTH_Low
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x5D52); // AFIT8_sddd8a_iDenThreshLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0A0A); // AFIT8_Demosaicing_iDemSharpenLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0800); // AFIT8_Demosaicing_iDemSharpThresh
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0328); // AFIT8_Demosaicing_iEdgeDesat
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x324C); // AFIT8_Demosaicing_iDemShLowLimit
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x001E); // AFIT8_Sharpening_iHighSharpPower
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200); // AFIT8_Sharpening_iHighShDenoise
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6464); // AFIT8_sddd8a_DispTH_Low_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FFF); // AFIT8_sddd8a_iDenThreshLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0307); // AFIT8_Demosaicing_iDemSharpenLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0103); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_1_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x010C); // AFIT8_sddd8a_iClustMulT_H_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x6464); // AFIT8_sddd8a_DispTH_Low_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0FFF); // AFIT8_sddd8a_iDenThreshLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0307); // AFIT8_Demosaicing_iDemSharpenLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x080F); // AFIT8_Demosaicing_iDemSharpThresh_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); // AFIT8_Demosaicing_iEdgeDesatThrLow_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x030F); // AFIT8_Demosaicing_iEdgeDesat_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x3208); // AFIT8_Demosaicing_iDemShLowLimit_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0F1E); // AFIT8_Sharpening_iHighSharpPower_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x020F); // AFIT8_Sharpening_iHighShDenoise_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0003); // AFIT8_demsharpmix1_iNarrFiltReduce_Bin_2_mode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x7F5E);  //ConstAfitBaseVals_0_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xFEEE);  //ConstAfitBaseVals_1_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xD9B7);  //ConstAfitBaseVals_2_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0472);  //ConstAfitBaseVals_3_
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  //ConstAfitBaseVals_4_
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x1278);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0xAAF0);	//gisp_dadlc  Ladlc mode average
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0408);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x067F);	//REG_TC_DBG_AutoAlgEnBits all AA are on

	// User Control
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x018E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000); //Brightness
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); //contrast
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0010); //Saturation
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0004); //sharpness

	// Flicker
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0408);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x065F);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x03F4); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
	

	//==================================================================================                                                                      
	// Clock Setting                                                                                                                                       
	//==================================================================================                                                                      
	// Input Clock 	S5K8AAYX_MIPI_write_cmos_sensor(Mclk)                                                                                                                                     
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x012E);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x5DC0);	//input clock                                                                                             
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0146);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002); //REG_TC_IPRM_UseNPviClocks                                                                               
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_TC_IPRM_UseNMipiClocks                                                                              
	                                                                                                                                                      
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014C);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x2CEC);//2328 //REG_TC_IPRM_sysClocks_0                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0152);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x59D8);//4650	//REG_TC_IPRM_MinOutRate4KHz_0                                                                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x014E);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x59D8);//4650 //REG_TC_IPRM_MaxOutRate4KHz_0                                                                      
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0154);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x2981); //29FE //REG_TC_IPRM_sysClocks_1                                                                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x015A);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x5208); //5302	//REG_TC_IPRM_MinOutRate4KHz_1                                                                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0156);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x53FC); //54F6 //REG_TC_IPRM_MaxOutRate4KHz_1                                                                     
                                                                                                      
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0164); //update PLL                                                                                              
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);       
	//*********************************************************************************                                                                       
	// 20.Preview & Capture Configration Setting                                                                                                              
	//*********************************************************************************                                                                       
	// Preview config[0] 1280X960  3~30fps //                                                                                                                 
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01BE);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0500); //REG_0TC_PCFG_usWidth                                                                                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03C0); //REG_0TC_PCFG_usHeight                                                                                   
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005); //REG_0TC_PCFG_Format                                                                                     
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0062); //REG_0TC_PCFG_PVIMask                                                                                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0100); //REG_0TC_PCFG_OIFMask                                                                                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_PCFG_uClockInd                                                                                  
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D2);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);	//REG_0TC_PCFG_usFrTimeType                                                                               
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_PCFG_FrRateQualityType                                                                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);//014D//0000 //REG_0TC_PCFG_usMinFrTimeMsecMult10                                                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x029A);//03e8 MTK revise to 0D05	//REG_0TC_PCFG_usMaxFrTimeMsecMult10                                      
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01E8);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_PCFG_uPrevMirror                                                                                
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_PCFG_uPCaptureMirror                                                                            
	                                                                                                                                                      
	// Capture config[0] 1280x960  7.5-30fps                                                                                                                  
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x02AE);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_CCFG_uCaptureMode                                                                               
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0500);	//REG_0TC_CCFG_usWidth                                                                                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03C0);	//REG_0TC_CCFG_usHeight                                                                                   
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);	//REG_0TC_CCFG_Format                                                                                     
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0062); //REG_0TC_CCFG_PVIMask                                                                                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0100); //REG_0TC_CCFG_OIFMask                                                                                    
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_CCFG_uClockInd                                                                                  
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x02C4);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);	//REG_0TC_CCFG_usFrTimeType                                                                               
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_CCFG_FrRateQualityType                                                                          
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);//014D //REG_0TC_CCFG_usMinFrTimeMsecMult10                                                                
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0D05);//0535	//REG_0TC_CCFG_usMaxFrTimeMsecMult10                                                              
	                                                                                                                                                      
	//==================================================================================                                                                      
	// 21.Select Cofigration Display                                                                                                                          
	//==================================================================================                                                                      
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01A8);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// REG_TC_GP_ActivePreviewConfig */                                                                        
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01AA);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);// REG_TC_GP_PreviewConfigChanged */                                                                       
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x019E);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);// REG_TC_GP_EnablePreview */                                                                              
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);// REG_TC_GP_EnablePreviewChanged */                                                                       
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028,0xD000);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x1000);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// Set host interrupt                                                                                     
	                                                                                                                                                      
	//s002A01B0                                                                                                                                               
	//s0F120000	//REG_TC_GP_ActiveCapConfig                                                                                                               
	//s002A01A6                                                                                                                                               
	//s0F120001	//REG_TC_GP_NewConfigSync                                                                                                                 
	//s002A01B2                                                                                                                                               
	//s0F120001	//REG_TC_GP_CapConfigChanged                                                                                                              
	//s002A01A2                                                                                                                                               
	//s0F120001	//REG_TC_GP_EnableCapture                                                                                                                 
	//s0F120001	//REG_TC_GP_EnableCaptureChanged                                                                                                          
	                                                                                                                                                      
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028,0xD000);                                                                                                           
	S5K8AAYX_MIPI_write_cmos_sensor(0x1000,0x0001);                                                                                                           
	mdelay(200);                                                                                                                                              
	                                                                                                                                                      
	//===================================================================================                                                                     
	// 22. ESD Check                                                                                                                                          
	//===================================================================================                                                                     
	//===================================================================================                                                                     
	// 23.ISSUE                                                                                                                                               
	//===================================================================================                                                                     




#endif

}

/*****************************************************************************
 * FUNCTION
 *  S5K8AAYX_MIPI_PV_Mode
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void S5K8AAYX_MIPI_PV_Mode(void)
{	SENSORDB("PV set st\n");
	// Preview config[0] 1280X960  3~30fps //
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01BE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0500); //REG_0TC_PCFG_usWidth
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03C0); //REG_0TC_PCFG_usHeight
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005); //REG_0TC_PCFG_Format 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0062); //REG_0TC_PCFG_PVIMask
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0100); //REG_0TC_PCFG_OIFMask 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_PCFG_uClockInd
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D2); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);	//REG_0TC_PCFG_usFrTimeType 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_PCFG_FrRateQualityType
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);//014D//0000 //REG_0TC_PCFG_usMinFrTimeMsecMult10
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x029A);//03e8 MTK revise to 0D05	//REG_0TC_PCFG_usMaxFrTimeMsecMult10
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01E8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_PCFG_uPrevMirror
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_PCFG_uPCaptureMirror

	// Capture config[0] 1280x960  7.5-30fps 
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x02AE);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_CCFG_uCaptureMode
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0500);	//REG_0TC_CCFG_usWidth
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x03C0);	//REG_0TC_CCFG_usHeight
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005);	//REG_0TC_CCFG_Format
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0062); //REG_0TC_CCFG_PVIMask
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0100); //REG_0TC_CCFG_OIFMask
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); //REG_0TC_CCFG_uClockInd
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x02C4);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);	//REG_0TC_CCFG_usFrTimeType
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_CCFG_FrRateQualityType
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);//014D //REG_0TC_CCFG_usMinFrTimeMsecMult10
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0D05);//0535	//REG_0TC_CCFG_usMaxFrTimeMsecMult10

	//==================================================================================
	// 21.Select Cofigration Display
	//==================================================================================
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01A8);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);// REG_TC_GP_ActivePreviewConfig */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01AA);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);// REG_TC_GP_PreviewConfigChanged */
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x019E);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);// REG_TC_GP_EnablePreview */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);// REG_TC_GP_EnablePreviewChanged */
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028,0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x1000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// Set host interrupt


	S5K8AAYX_MIPI_write_cmos_sensor(0x0028,0xD000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x1000,0x0001);
	mdelay(200);

	//S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);
	//S5K8AAYX_MIPI_write_cmos_sensor(0x002a, 0x019A);	
	//S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //EV 0
	SENSORDB("PV set ed\n");
}



/*****************************************************************************
 * FUNCTION
 *  S5K8AAYX_MIPI_CAP_Mode
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/

void S5K8AAYX_MIPI_CAP_Mode(void)
{
//
}

static void S5K8AAYX_MIPI_set_AE_mode(kal_bool AE_enable)
{
     if(AE_enable==KAL_TRUE)
     {
               S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000); // Set page 
               S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000); // Set address  

               S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x214A); 
               S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
     }
     else
     {
               S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000); // Set page 
               S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000); // Set address  

               S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x214A); 
               S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0000);
     }

}


/*************************************************************************
* FUNCTION
*	S5K8AAYX_MIPI_night_mode
*
* DESCRIPTION
*	This function night mode of S5K8AAYX_MIPI.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void S5K8AAYX_MIPI_night_mode(kal_bool enable)
{
    //if(S5K8AAYX_MIPICurrentStatus.iNightMode == enable)
    //    return;

	if (S5K8AAYX_MIPI_sensor_cap_state == KAL_TRUE)
		return ;	

	//S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000); // Set page 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000); // Set address	
	if (enable)
	{
		if (S5K8AAYX_MIPI_MPEG4_encode_mode == KAL_TRUE)
		{
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D2); 
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	//REG_0TC_PCFG_usFrTimeType 
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_PCFG_FrRateQualityType
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D6);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,S5K8AAYX_MIPI_VID_NIT_FIX_FR_TIME); //REG_0TC_PCFG_usMinFrTimeMsecMult10
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,S5K8AAYX_MIPI_VID_NIT_FIX_FR_TIME); //REG_0TC_PCFG_usMaxFrTimeMsecMult10 fps
		}
		else
		{
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D2); 
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);	//REG_0TC_PCFG_usFrTimeType
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_PCFG_FrRateQualityType
			
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D6);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,S5K8AAYX_MIPI_CAM_NIT_MIN_FR_TIME); //REG_0TC_PCFG_usMinFrTimeMsecMult10
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,S5K8AAYX_MIPI_CAM_NIT_MAX_FR_TIME);	//REG_0TC_PCFG_usMaxFrTimeMsecMult10
		}	
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0468); 
        S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0140);  //lt_uMaxDigGain
	}
	else
	{
		if (S5K8AAYX_MIPI_MPEG4_encode_mode == KAL_TRUE)
		{
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D2); 
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	//REG_0TC_PCFG_usFrTimeType 
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_PCFG_FrRateQualityType
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D6);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, S5K8AAYX_MIPI_VID_NOM_FIX_FR_TIME); //#REG_0TC_PCFG_usMaxFrTimeMsecMult10
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, S5K8AAYX_MIPI_VID_NOM_FIX_FR_TIME); //#REG_0TC_PCFG_usMinFrTimeMsecMult10
		}
		else
		{
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D2); 
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);	//REG_0TC_PCFG_usFrTimeType
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_PCFG_FrRateQualityType
			
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D6);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,S5K8AAYX_MIPI_CAM_NOM_MIN_FR_TIME); //REG_0TC_PCFG_usMinFrTimeMsecMult10
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,S5K8AAYX_MIPI_CAM_NOM_MAX_FR_TIME);	//REG_0TC_PCFG_usMaxFrTimeMsecMult10
		}	

		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x0468); 
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0100);	//lt_uMaxDigGain
	}
	
	// active preview configure
	//============================================================
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01A8); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);	// #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01AC); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// #REG_TC_GP_PrevOpenAfterChange
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01A6); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// #REG_TC_GP_NewConfigSync // Update preview configuration
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01AA); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// #REG_TC_GP_PrevConfigChanged
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x019E); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// #REG_TC_GP_EnablePreview // Start preview
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// #REG_TC_GP_EnablePreviewChanged

	spin_lock(&s5k8aayxmipi_drv_lock);
    S5K8AAYX_MIPICurrentStatus.iNightMode = enable;
	spin_unlock(&s5k8aayxmipi_drv_lock);
}	/* S5K8AAYX_MIPI_night_mode */

/*************************************************************************
* FUNCTION
*	S5K8AAYX_MIPI_GetSensorID
*
* DESCRIPTION
*	This function get the sensor ID
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static u8 s5k3l2_rc = 0;
static u8 s5k3l2_used = 0;
#ifdef SLT_DEVINFO_CMM
#include <linux/dev_info.h>
#endif

static kal_uint32 S5K8AAYX_MIPI_GetSensorID(kal_uint32 *sensorID)
{
   // volatile signed char i;
	kal_uint16 sensor_id=0;//,sensor_id_2=0;

	s5k3l2_rc++;
	
	#ifdef SLT_DEVINFO_CMM
    struct devinfo_struct *dev;
	#endif

     // check if sensor ID correct
	 S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0x0000);
	 sensor_id=S5K8AAYX_MIPI_read_cmos_sensor(0x0040);

	 SENSORDB("Sensor Read S5K8AAYX_MIPI ID_use_FCFC %x \r\n",sensor_id);
	 
	 *sensorID=sensor_id;

	 if (sensor_id != S5K8AAYX_MIPI_SENSOR_ID)
	{
     #ifdef SLT_DEVINFO_CMM
       // S5K3L2X2DB("zjz: To ensure this segment will run. LensID = 0x%04x VcmID = 0x%04x\n", LensID, VcmID);
		dev=(struct devinfo_struct*)kmalloc(sizeof(struct devinfo_struct), GFP_KERNEL);
		dev->device_type = "CCM";
		dev->device_module = "O-film";
		dev->device_vendor	 ="O-film";		  
		dev->device_ic	 = "S5K8AAYX";
		dev->device_version = "CDGM+unknown";
		dev->device_info = "30W";
		dev->device_used	=	DEVINFO_USED;
	
        s5k3l2_used = 1; 
		devinfo_check_add_device(dev);		
 	 #endif            
	}
	if (sensor_id != S5K8AAYX_MIPI_SENSOR_ID)
	{
	 #ifdef SLT_DEVINFO_CMM
	  if(s5k3l2_rc == 2 && s5k3l2_used == 0 )
	  {
         dev=(struct devinfo_struct*)kmalloc(sizeof(struct devinfo_struct), GFP_KERNEL);
         dev->device_type = "CMM";
         dev->device_module = "O-film";
         dev->device_vendor	 ="O-film";
         dev->device_ic	 = "S5K8AAYX";
		 dev->device_version = "OPTO+ALPS";
         dev->device_info = "300W";
         dev->device_used	=	DEVINFO_UNUSED;
         devinfo_check_add_device(dev);
	  }  
 	#endif
	
	    *sensorID=0xFFFFFFFF;
	    SENSORDB("Sensor Read ByeBye \r\n");
		return ERROR_SENSOR_CONNECT_FAIL;
	}
    return ERROR_NONE;    
}  


/*****************************************************************************/
/* Windows Mobile Sensor Interface */
/*****************************************************************************/
/*************************************************************************
* FUNCTION
*	S5K8AAYX_MIPIOpen
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 S5K8AAYX_MIPIOpen(void)
{
	//volatile signed char i;
	kal_uint16 sensor_id=0;

	 S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0x0000);
	 sensor_id=S5K8AAYX_MIPI_read_cmos_sensor(0x0040);
	 SENSORDB("Sensor Read S5K8AAYX_MIPI ID %x \r\n",sensor_id);

	if (sensor_id != S5K8AAYX_MIPI_SENSOR_ID)
	{
	    SENSORDB("Sensor Read ByeBye \r\n");
		return ERROR_SENSOR_CONNECT_FAIL;
	}

    S5K8AAYX_MIPIInitialPara(); 
	S5K8AAYX_MIPI_Initialize_Setting();
	return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*	S5K8AAYX_MIPIClose
*
* DESCRIPTION
*	This function is to turn off sensor module power.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 S5K8AAYX_MIPIClose(void)
{
	return ERROR_NONE;
}	/* S5K8AAYX_MIPIClose() */

/*************************************************************************
* FUNCTION
*	S5K8AAYX_MIPIPreview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 S5K8AAYX_MIPIPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	spin_lock(&s5k8aayxmipi_drv_lock);
	S5K8AAYX_MIPI_sensor_cap_state = KAL_FALSE;
	S5K8AAYX_MIPI_PV_dummy_lines = 0;    
	
	if(sensor_config_data->SensorOperationMode==MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
	{
		S5K8AAYX_MIPI_MPEG4_encode_mode = KAL_TRUE;		
		S5K8AAYX_MIPI_MJPEG_encode_mode = KAL_FALSE;
	}
	else
	{
		S5K8AAYX_MIPI_MPEG4_encode_mode = KAL_FALSE;		
		S5K8AAYX_MIPI_MJPEG_encode_mode = KAL_FALSE;
		
	}
	spin_unlock(&s5k8aayxmipi_drv_lock);
	
	S5K8AAYX_MIPI_PV_Mode();
	S5K8AAYX_MIPI_night_mode(S5K8AAYX_MIPICurrentStatus.iNightMode);
	S5K8AAYX_MIPI_set_mirror(sensor_config_data->SensorImageMirror);
 
    image_window->ExposureWindowWidth = S5K8AAYX_MIPI_IMAGE_SENSOR_PV_WIDTH;
    image_window->ExposureWindowHeight = S5K8AAYX_MIPI_IMAGE_SENSOR_PV_HEIGHT;
	
	// copy sensor_config_data
	memcpy(&S5K8AAYX_MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
  	return ERROR_NONE;
}	/* S5K8AAYX_MIPIPreview() */

UINT32 S5K8AAYX_MIPICapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                 MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
     //kal_uint32 pv_integration_time = 0;       // Uinit - us
     //kal_uint32 cap_integration_time = 0;
     //kal_uint16 PV_line_len = 0;
     //kal_uint16 CAP_line_len = 0;

	spin_lock(&s5k8aayxmipi_drv_lock);
	S5K8AAYX_MIPI_sensor_cap_state = KAL_TRUE;
	spin_unlock(&s5k8aayxmipi_drv_lock);
               
    S5K8AAYX_MIPI_CAP_Mode();         

    image_window->GrabStartX = S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_INSERTED_PIXELS;
    image_window->GrabStartY = S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_INSERTED_LINES;
    image_window->ExposureWindowWidth= S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_WIDTH;
    image_window->ExposureWindowHeight = S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_HEIGHT;

     // copy sensor_config_data
     memcpy(&S5K8AAYX_MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
     return ERROR_NONE;
}        /* S5K8AAYX_MIPICapture() */

UINT32 S5K8AAYX_MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	pSensorResolution->SensorFullWidth=S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_WIDTH;  //modify by yanxu
	pSensorResolution->SensorFullHeight=S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_HEIGHT;
	pSensorResolution->SensorPreviewWidth=S5K8AAYX_MIPI_IMAGE_SENSOR_PV_WIDTH; 
	pSensorResolution->SensorPreviewHeight=S5K8AAYX_MIPI_IMAGE_SENSOR_PV_HEIGHT;
	
	pSensorResolution->SensorVideoWidth=S5K8AAYX_MIPI_IMAGE_SENSOR_PV_WIDTH; 
	pSensorResolution->SensorVideoHeight=S5K8AAYX_MIPI_IMAGE_SENSOR_PV_HEIGHT;

	return ERROR_NONE;
}	/* S5K8AAYX_MIPIGetResolution() */

UINT32 S5K8AAYX_MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	
    switch(ScenarioId)
    {
    	case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 pSensorInfo->SensorPreviewResolutionX=S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_WIDTH;
	         pSensorInfo->SensorPreviewResolutionY=S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_HEIGHT;
			 pSensorInfo->SensorCameraPreviewFrameRate=15;
			 break;
		
		default:
			 pSensorInfo->SensorPreviewResolutionX=S5K8AAYX_MIPI_IMAGE_SENSOR_PV_WIDTH;
	         pSensorInfo->SensorPreviewResolutionY=S5K8AAYX_MIPI_IMAGE_SENSOR_PV_HEIGHT;
			 pSensorInfo->SensorCameraPreviewFrameRate=30;
    }
	pSensorInfo->SensorFullResolutionX=S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_WIDTH;
	pSensorInfo->SensorFullResolutionY=S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_HEIGHT;
	pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=10;
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE;
	pSensorInfo->SensorResetDelayCount=1;
	
	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
	
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;

	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_HIGH;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorInterruptDelayLines = 1;
	
	#ifdef MIPI_INTERFACE
   		pSensorInfo->SensroInterfaceType        = SENSOR_INTERFACE_TYPE_MIPI;
   	#else
   		pSensorInfo->SensroInterfaceType		= SENSOR_INTERFACE_TYPE_PARALLEL;
   	#endif
	
	pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;//ISP_DRIVING_4MA;   	
	pSensorInfo->CaptureDelayFrame = 3; 
	pSensorInfo->PreviewDelayFrame = 3; 
	pSensorInfo->VideoDelayFrame = 4; 

	pSensorInfo->YUVAwbDelayFrame = 3; 
	pSensorInfo->YUVEffectDelayFrame = 2; 
	
	switch (ScenarioId) 
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pSensorInfo->SensorClockFreq=24;
			
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			
			pSensorInfo->SensorDataLatchCount= 2;
			
			pSensorInfo->SensorGrabStartX = 2; 
			pSensorInfo->SensorGrabStartY = 2; 
#ifdef MIPI_INTERFACE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;			
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 4; 
	        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
#endif
		break;

		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
			pSensorInfo->SensorGrabStartX = 2; 
			pSensorInfo->SensorGrabStartY = 2; 					
			#ifdef MIPI_INTERFACE
	            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;			
	            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
		        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 4; 
		        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
	            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
	            pSensorInfo->SensorPacketECCOrder = 1;
	        	#endif			
		break;
				
		default:
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount=3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
			pSensorInfo->SensorGrabStartX = 2; 
			pSensorInfo->SensorGrabStartY = 2; 					
		break;
	}
	memcpy(pSensorConfigData, &S5K8AAYX_MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	return ERROR_NONE;
}	/* S5K8AAYX_MIPIGetInfo() */


UINT32 S5K8AAYX_MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			S5K8AAYX_MIPIPreview(pImageWindow, pSensorConfigData);
		break;
		
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			S5K8AAYX_MIPICapture(pImageWindow, pSensorConfigData);
		break;
		
		//#if defined(MT6575)
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			   S5K8AAYX_MIPICapture(pImageWindow, pSensorConfigData);
			break;
		//#endif
		
        default:
            return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
}	/* S5K8AAYX_MIPIControl() */

/* [TC] YUV sensor */	
#if WINMO_USE
void S5K8AAYX_MIPIQuery(PMSDK_FEATURE_INFO_STRUCT pSensorFeatureInfo)
{
	MSDK_FEATURE_TYPE_RANGE_STRUCT *pFeatureRange;
	MSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT *pFeatureMultiSelection;
	switch (pSensorFeatureInfo->FeatureId)
	{
		case ISP_FEATURE_DSC_MODE:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_MULTI_SELECTION;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_CAMERA_SUPPORTED);
			pFeatureMultiSelection = (PMSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureMultiSelection);
			pFeatureMultiSelection->TotalSelection = CAM_NO_OF_SCENE_MODE_MAX-2;
			pFeatureMultiSelection->DefaultSelection = CAM_AUTO_DSC_MODE;
			pFeatureMultiSelection->SupportedSelection = 
				(CAMERA_FEATURE_SUPPORT(CAM_AUTO_DSC_MODE)|
				CAMERA_FEATURE_SUPPORT(CAM_NIGHTSCENE_MODE));			
		break;
		case ISP_FEATURE_WHITEBALANCE:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_MULTI_SELECTION;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_CAMERA_VIDEO_SUPPORTED);
			pFeatureMultiSelection = (PMSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureMultiSelection);
			pFeatureMultiSelection->TotalSelection = CAM_NO_OF_WB;
			pFeatureMultiSelection->DefaultSelection = CAM_WB_AUTO;
			pFeatureMultiSelection->SupportedSelection = 
				(CAMERA_FEATURE_SUPPORT(CAM_WB_AUTO)|
				CAMERA_FEATURE_SUPPORT(CAM_WB_CLOUD)|
				CAMERA_FEATURE_SUPPORT(CAM_WB_DAYLIGHT)|
				CAMERA_FEATURE_SUPPORT(CAM_WB_INCANDESCENCE)|
				CAMERA_FEATURE_SUPPORT(CAM_WB_FLUORESCENT));
		break;
		case ISP_FEATURE_IMAGE_EFFECT:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_MULTI_SELECTION;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_CAMERA_VIDEO_SUPPORTED);
			pFeatureMultiSelection = (PMSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureMultiSelection);
			pFeatureMultiSelection->TotalSelection = CAM_NO_OF_EFFECT_ENC;
			pFeatureMultiSelection->DefaultSelection = CAM_EFFECT_ENC_NORMAL;
			pFeatureMultiSelection->SupportedSelection = 
				(CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_NORMAL)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_GRAYSCALE)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_COLORINV)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_GRAYINV)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_SEPIABLUE)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_SKETCH)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_EMBOSSMENT)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_SEPIA));	
		break;
		case ISP_FEATURE_AE_METERING_MODE:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;
		break;
		case ISP_FEATURE_BRIGHTNESS:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_RANGE;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_CAMERA_VIDEO_SUPPORTED);
			pFeatureRange = (PMSDK_FEATURE_TYPE_RANGE_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureRange);
			pFeatureRange->MinValue = CAM_EV_NEG_4_3;
			pFeatureRange->MaxValue = CAM_EV_POS_4_3;
			pFeatureRange->StepValue = CAMERA_FEATURE_ID_EV_STEP;
			pFeatureRange->DefaultValue = CAM_EV_ZERO;
		break;
		case ISP_FEATURE_BANDING_FREQ:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_MULTI_SELECTION;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_CAMERA_VIDEO_SUPPORTED);
			pFeatureMultiSelection = (PMSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureMultiSelection);
			pFeatureMultiSelection->TotalSelection = CAM_NO_OF_BANDING;
			pFeatureMultiSelection->DefaultSelection = CAM_BANDING_50HZ;
			pFeatureMultiSelection->SupportedSelection = 
				(CAMERA_FEATURE_SUPPORT(CAM_BANDING_50HZ)|
				CAMERA_FEATURE_SUPPORT(CAM_BANDING_60HZ));
		break;
		case ISP_FEATURE_AF_OPERATION:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;
		break;
		case ISP_FEATURE_AF_RANGE_CONTROL:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;
		break;
		case ISP_FEATURE_FLASH:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;			
		break;
		case ISP_FEATURE_VIDEO_SCENE_MODE:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_MULTI_SELECTION;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_VIDEO_SUPPORTED);
			pFeatureMultiSelection = (PMSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureMultiSelection);
			pFeatureMultiSelection->TotalSelection = 2;
			pFeatureMultiSelection->DefaultSelection = CAM_VIDEO_AUTO_MODE;
			pFeatureMultiSelection->SupportedSelection = 
				(CAMERA_FEATURE_SUPPORT(CAM_VIDEO_AUTO_MODE)|
				CAMERA_FEATURE_SUPPORT(CAM_VIDEO_NIGHT_MODE));
		break;
		case ISP_FEATURE_ISO:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;			
		break;
		default:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;			
		break;
	}
}
#endif 

BOOL S5K8AAYX_MIPI_set_param_wb(UINT16 para)
{

	if(S5K8AAYX_MIPICurrentStatus.iWB == para)
		return TRUE;
	SENSORDB("[Enter]S5K8AAYX_MIPI set_param_wb func:para = %d\n",para);

	S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000); // Set page 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000); // Set address	
	
	switch (para)
	{
		case AWB_MODE_AUTO:
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0408); //bit[3]:AWB Auto:1 menual:0
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x067F);
			break;

		case AWB_MODE_CLOUDY_DAYLIGHT:	
			//======================================================================
			//	MWB : Cloudy_D65										 
			//======================================================================
			S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0408);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0677);							 
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x03DA);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0740);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0400);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0460);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); 
			break;

		case AWB_MODE_DAYLIGHT:
			//==============================================
			//	MWB : sun&daylight_D50						
			//==============================================
			S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0408);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0677);							 
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x03DA);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x05E0);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0400);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0530);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);  
			break;

		case AWB_MODE_INCANDESCENT:
			//==============================================									   
			//	MWB : Incand_Tungsten						
			//==============================================
			S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0408); 
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0677);							
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x03DA); 
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x05C0); //Reg_sf_user_Rgain
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); //Reg_sf_user_RgainChanged update
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0400); //Reg_sf_user_Ggain
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); //Reg_sf_user_GgainChanged
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x08B0); //Reg_sf_user_Bgain
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); //Reg_sf_user_BgainChanged		
			break;

		case AWB_MODE_FLUORESCENT:
			//==================================================================
			//	MWB : Florescent_TL84							  
			//==================================================================
			S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0408);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0677);
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x03DA);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0575);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0400);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0800);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); 
			break;

		case AWB_MODE_TUNGSTEN:	
			S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x0408);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0677);
								 
			S5K8AAYX_MIPI_write_cmos_sensor(0x002A, 0x03DA);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0400);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0400);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0940);
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0001); 
			break;
		default:
			return KAL_FALSE;	

		}
		spin_lock(&s5k8aayxmipi_drv_lock);
	    S5K8AAYX_MIPICurrentStatus.iWB = para;
		spin_unlock(&s5k8aayxmipi_drv_lock);
return TRUE;
}

BOOL S5K8AAYX_MIPI_set_param_effect(UINT16 para)
{
	/*----------------------------------------------------------------*/
	   /* Local Variables												 */
	   /*----------------------------------------------------------------*/
	   kal_uint32 ret = KAL_TRUE;
	
	   /*----------------------------------------------------------------*/
	   /* Code Body 													 */
	   /*----------------------------------------------------------------*/

	   if(S5K8AAYX_MIPICurrentStatus.iEffect == para)
		  return TRUE;
	   SENSORDB("[Enter]s5k8aayxmipi set_param_effect func:para = %d\n",para);

	   S5K8AAYX_MIPI_write_cmos_sensor(0x0028,0x7000);
	   S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x019C);
	   switch (para)
	   {
		   case MEFFECT_OFF:
			   S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000); // Normal, 
			   break;
		   case MEFFECT_MONO:
			   S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001); // Monochrome (Black & White)
			   break;
		   case MEFFECT_SEPIA:
			   S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0004); // Sepia
			   break;
		   case MEFFECT_SEPIABLUE:
			   S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0005); // Aqua (Blue)
			   break;
		   case MEFFECT_NEGATIVE:
			   S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0003); // Negative color
		   default:
			   ret = KAL_FALSE;
	   }
	   
	   spin_lock(&s5k8aayxmipi_drv_lock);
	   S5K8AAYX_MIPICurrentStatus.iEffect = para;
	   spin_unlock(&s5k8aayxmipi_drv_lock);
	   return ret;
} 

void S5K8AAYX_MIPIGetAEAWBLock(UINT32 *pAElockRet32,UINT32 *pAWBlockRet32)
{
    *pAElockRet32 = 1;
	*pAWBlockRet32 = 1;
    SENSORDB("S5K8AAYX_MIPIGetAEAWBLock,AE=%d ,AWB=%d\n,",*pAElockRet32,*pAWBlockRet32);
}


BOOL S5K8AAYX_MIPI_set_param_banding(UINT16 para)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

#if (defined(S5K8AAYX_MIPI_MANUAL_ANTI_FLICKER))

	if(S5K8AAYX_MIPICurrentStatus.iBanding == para)
      return TRUE;

    switch (para)
	{
		case AE_FLICKER_MODE_50HZ:
				S5K8AAYX_MIPI_write_cmos_sensor(0x0028,0x7000);
				S5K8AAYX_MIPI_write_cmos_sensor(0x002a,0x0408);
				S5K8AAYX_MIPI_write_cmos_sensor(0x0f12,0x065F);
				S5K8AAYX_MIPI_write_cmos_sensor(0x002a,0x03F4);
				S5K8AAYX_MIPI_write_cmos_sensor(0x0f12,0x0001); //REG_SF_USER_FlickerQuant 1:50hz  2:60hz
				S5K8AAYX_MIPI_write_cmos_sensor(0x0f12,0x0001); //REG_SF_USER_FlickerQuantChanged active flicker
			break;
		case AE_FLICKER_MODE_60HZ:
				S5K8AAYX_MIPI_write_cmos_sensor(0x0028,0x7000);
				S5K8AAYX_MIPI_write_cmos_sensor(0x002a,0x0408);
				S5K8AAYX_MIPI_write_cmos_sensor(0x0f12,0x065F);
				S5K8AAYX_MIPI_write_cmos_sensor(0x002a,0x03F4);
				S5K8AAYX_MIPI_write_cmos_sensor(0x0f12,0x0002); //REG_SF_USER_FlickerQuant 1:50hz  2:60hz
				S5K8AAYX_MIPI_write_cmos_sensor(0x0f12,0x0001); //REG_SF_USER_FlickerQuantChanged active flicker
			break;
		default:
			return KAL_FALSE;
	}
	spin_lock(&s5k8aayxmipi_drv_lock);
    S5K8AAYX_MIPICurrentStatus.iBanding = para;
	spin_unlock(&s5k8aayxmipi_drv_lock);
	return TRUE;
	
#else
	/* Auto anti-flicker method is enabled, then nothing need to do in this function.  */

#endif	/* #if (defined(S5K8AAYX_MIPI_MANUAL_ANTI_FLICKER)) */
	return KAL_TRUE;
} /* S5K8AAYX_MIPI_set_param_banding */

BOOL S5K8AAYX_MIPI_set_param_exposure(UINT16 para)
{

	//if(S5K8AAYX_MIPICurrentStatus.iEV == para)
	//	return TRUE;
	
	SENSORDB("[Enter]s5k8aayxmipi set_param_exposure func:para = %d\n",para);
	  
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000);
	S5K8AAYX_MIPI_write_cmos_sensor(0x002a, 0x019A);		
    switch (para)
	{	
		case AE_EV_COMP_n10:
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0080); //EV -1
			break;
		case AE_EV_COMP_00:				   
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0100); //EV 0
			break;
		case AE_EV_COMP_10:					
			S5K8AAYX_MIPI_write_cmos_sensor(0x0F12, 0x0200);  //EV +1
			break;
			
		case AE_EV_COMP_n13:					
		case AE_EV_COMP_n07:				   
		case AE_EV_COMP_n03:				   
		case AE_EV_COMP_03: 			
		case AE_EV_COMP_07: 
		case AE_EV_COMP_13:
			break;
			
		default:			
			return FALSE;
	}
	spin_lock(&s5k8aayxmipi_drv_lock);
	S5K8AAYX_MIPICurrentStatus.iEV = para;
	spin_unlock(&s5k8aayxmipi_drv_lock);
	return TRUE;

}/* S5K8AAYX_MIPI_set_param_exposure */


UINT32 S5K8AAYX_MIPIYUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
	switch (iCmd) {
	case FID_SCENE_MODE:
	    if (iPara == SCENE_MODE_OFF)
	    {
	        S5K8AAYX_MIPI_night_mode(0); 
	    }

         else if (iPara == SCENE_MODE_NIGHTSCENE)			
	    {
            S5K8AAYX_MIPI_night_mode(1); 
	    }	    
	    break; 	    
	case FID_AWB_MODE:
         S5K8AAYX_MIPI_set_param_wb(iPara);
	break;
	case FID_COLOR_EFFECT:
    	    
         S5K8AAYX_MIPI_set_param_effect(iPara);
	break;
	case FID_AE_EV:  	    
         S5K8AAYX_MIPI_set_param_exposure(iPara);
	break;
	case FID_AE_FLICKER:
	    	    	    
         S5K8AAYX_MIPI_set_param_banding(iPara);
	break;
    case FID_AE_SCENE_MODE:  
		spin_lock(&s5k8aayxmipi_drv_lock);
      if (iPara == AE_MODE_OFF) {
          S5K8AAYX_MIPI_AE_ENABLE = KAL_FALSE; 
      }
      else {
          S5K8AAYX_MIPI_AE_ENABLE = KAL_TRUE; 
      }
	  spin_unlock(&s5k8aayxmipi_drv_lock);
      S5K8AAYX_MIPI_set_AE_mode(S5K8AAYX_MIPI_AE_ENABLE);
    break; 
	case FID_ZOOM_FACTOR:
		spin_lock(&s5k8aayxmipi_drv_lock);
	    zoom_factor = iPara; 
		spin_unlock(&s5k8aayxmipi_drv_lock);
	break; 
	default:
	break;
	}
	return ERROR_NONE;
}   /* S5K8AAYX_MIPIYUVSensorSetting */


UINT32 S5K8AAYX_MIPIYUVSetVideoMode(UINT16 u2FrameRate)
{

    if(S5K8AAYX_MIPICurrentStatus.iFrameRate == u2FrameRate)
      return ERROR_NONE;

	spin_lock(&s5k8aayxmipi_drv_lock);
    S5K8AAYX_MIPI_VEDIO_encode_mode = KAL_TRUE; 
	S5K8AAYX_MIPI_MPEG4_encode_mode = KAL_TRUE;
	spin_unlock(&s5k8aayxmipi_drv_lock);
	
	//S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC, 0xD000); // Set page 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0028, 0x7000); // Set address	

    if(20<=u2FrameRate && u2FrameRate<=30) //fix 30fps
    {		
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D2); 
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	//REG_0TC_PCFG_usFrTimeType 
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_PCFG_FrRateQualityType
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D6);
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,S5K8AAYX_MIPI_VID_NOM_FIX_FR_TIME); //REG_0TC_PCFG_usMinFrTimeMsecMult10
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,S5K8AAYX_MIPI_VID_NOM_FIX_FR_TIME); //REG_0TC_PCFG_usMaxFrTimeMsecMult10 fps
    }
    else if(5<=u2FrameRate && u2FrameRate<20 )// fix 15fps
    {
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D2); 
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	//REG_0TC_PCFG_usFrTimeType 
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0002);	//REG_0TC_PCFG_FrRateQualityType
		S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01D6);
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,S5K8AAYX_MIPI_VID_NIT_FIX_FR_TIME); //REG_0TC_PCFG_usMinFrTimeMsecMult10
		S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,S5K8AAYX_MIPI_VID_NIT_FIX_FR_TIME); //REG_0TC_PCFG_usMaxFrTimeMsecMult10 fps
    }
    else 
    {
        printk("Wrong Frame Rate \n"); 
    }
	
	// active preview configure
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01A8); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0000);	// #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01AC); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// #REG_TC_GP_PrevOpenAfterChange
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01A6); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// #REG_TC_GP_NewConfigSync // Update preview configuration
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x01AA); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// #REG_TC_GP_PrevConfigChanged
	S5K8AAYX_MIPI_write_cmos_sensor(0x002A,0x019E); 
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// #REG_TC_GP_EnablePreview // Start preview
	S5K8AAYX_MIPI_write_cmos_sensor(0x0F12,0x0001);	// #REG_TC_GP_EnablePreviewChanged
    return ERROR_NONE;
}


kal_uint16 S5K8AAYX_MIPIReadShutter(void)
{
   kal_uint16 temp_msb=0x0000,temp_lsb=0x0000;
   kal_uint32 temp=0x00000000;
   
   
   S5K8AAYX_MIPI_write_cmos_sensor(0x002c,0x7000);
   S5K8AAYX_MIPI_write_cmos_sensor(0x002e,0x16E2);//16bit

   temp_msb=S5K8AAYX_MIPI_read_cmos_sensor(0x0f12);
   temp_msb = (temp_msb<<16)&0xffff0000;

   S5K8AAYX_MIPI_write_cmos_sensor(0x002c,0x7000);
   S5K8AAYX_MIPI_write_cmos_sensor(0x002e,0x16E0);
   temp_lsb=S5K8AAYX_MIPI_read_cmos_sensor(0x0f12);
   temp_lsb = temp_lsb&0x0000ffff;

   temp = (temp_msb|temp_lsb)/400;

   temp = temp*72000/(S5K8AAYX_MIPI_IMAGE_SENSOR_PV_WIDTH*2);
   return temp;
   
}
kal_uint16 S5K8AAYX_MIPIReadGain(void)
{
    kal_uint16 temp=0x0000,Base_gain=64;
	S5K8AAYX_MIPI_write_cmos_sensor(0x002c,0x7000);
    S5K8AAYX_MIPI_write_cmos_sensor(0x002e,0x20D0);//AGain
    
	temp=S5K8AAYX_MIPI_read_cmos_sensor(0x0f12);
	temp = Base_gain*temp/256;

	return temp;
}
kal_uint16 S5K8AAYX_MIPIReadAwbRGain(void)
{
    kal_uint16 temp=0x0000,Base_gain=64;
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002c,0x7000);
    S5K8AAYX_MIPI_write_cmos_sensor(0x002e,0x20DC);

	temp=S5K8AAYX_MIPI_read_cmos_sensor(0x0f12);
	temp = Base_gain*temp/1024;
	return temp;
}
kal_uint16 S5K8AAYX_MIPIReadAwbBGain(void)
{
    kal_uint16 temp=0x0000,Base_gain=64;
	
	S5K8AAYX_MIPI_write_cmos_sensor(0x002c,0x7000);
    S5K8AAYX_MIPI_write_cmos_sensor(0x002e,0x20E0);

	temp=S5K8AAYX_MIPI_read_cmos_sensor(0x0f12);
	temp = Base_gain*temp/1024;

	return temp;
}

//#if defined(MT6575)

/*************************************************************************
* FUNCTION
*    S5K8AAYX_MIPIGetEvAwbRef
*
* DESCRIPTION
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void S5K8AAYX_MIPIGetEvAwbRef(UINT32 pSensorAEAWBRefStruct/*PSENSOR_AE_AWB_REF_STRUCT Ref*/)
{
    PSENSOR_AE_AWB_REF_STRUCT Ref = (PSENSOR_AE_AWB_REF_STRUCT)pSensorAEAWBRefStruct;
    SENSORDB("S5K8AAYX_MIPIGetEvAwbRef  \n" );
    	
	Ref->SensorAERef.AeRefLV05Shutter = 5422;
    Ref->SensorAERef.AeRefLV05Gain = 478; /* 128 base */
    Ref->SensorAERef.AeRefLV13Shutter = 80;
    Ref->SensorAERef.AeRefLV13Gain = 128; /*  128 base */
    Ref->SensorAwbGainRef.AwbRefD65Rgain = 186; /* 128 base */
    Ref->SensorAwbGainRef.AwbRefD65Bgain = 158; /* 128 base */
    Ref->SensorAwbGainRef.AwbRefCWFRgain = 196; /* 1.25x, 128 base */
    Ref->SensorAwbGainRef.AwbRefCWFBgain = 278; /* 1.28125x, 128 base */
}
/*************************************************************************
* FUNCTION
*    S5K8AAYX_MIPIGetCurAeAwbInfo
*
* DESCRIPTION
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void S5K8AAYX_MIPIGetCurAeAwbInfo(UINT32 pSensorAEAWBCurStruct/*PSENSOR_AE_AWB_CUR_STRUCT Info*/)
{
    PSENSOR_AE_AWB_CUR_STRUCT Info = (PSENSOR_AE_AWB_CUR_STRUCT)pSensorAEAWBCurStruct;
    SENSORDB("S5K8AAYX_MIPIGetCurAeAwbInfo  \n" );

    Info->SensorAECur.AeCurShutter = S5K8AAYX_MIPIReadShutter();
    Info->SensorAECur.AeCurGain = S5K8AAYX_MIPIReadGain() * 2; /* 128 base */
    
    Info->SensorAwbGainCur.AwbCurRgain = S5K8AAYX_MIPIReadAwbRGain()<< 1; /* 128 base */
    
    Info->SensorAwbGainCur.AwbCurBgain = S5K8AAYX_MIPIReadAwbBGain()<< 1; /* 128 base */
}

//#endif

void S5K8AAYX_MIPIGetAFMaxNumFocusAreas(UINT32 *pFeatureReturnPara32)
{	
    *pFeatureReturnPara32 = 0;    
    SENSORDB("S5K8AAYX_MIPIGetAFMaxNumFocusAreas, *pFeatureReturnPara32 = %d\n",*pFeatureReturnPara32);

}

void S5K8AAYX_MIPIGetAFMaxNumMeteringAreas(UINT32 *pFeatureReturnPara32)
{	
    *pFeatureReturnPara32 = 0;    
    SENSORDB("S5K8AAYX_MIPIGetAFMaxNumMeteringAreas,*pFeatureReturnPara32 = %d\n",*pFeatureReturnPara32);

}

void S5K8AAYX_MIPIGetExifInfo(UINT32 exifAddr)
{
    SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;
    pExifInfo->FNumber = 28;
    pExifInfo->AEISOSpeed = AE_ISO_100;
    pExifInfo->AWBMode = S5K8AAYX_MIPICurrentStatus.iWB;
    pExifInfo->CapExposureTime = 0;
    pExifInfo->FlashLightTimeus = 0;
    pExifInfo->RealISOValue = AE_ISO_100;
}

void S5K8AAYX_MIPIGetDelayInfo(UINT32 delayAddr)
{
    SENSOR_DELAY_INFO_STRUCT* pDelayInfo = (SENSOR_DELAY_INFO_STRUCT*)delayAddr;
    pDelayInfo->InitDelay = 3;
    pDelayInfo->EffectDelay = 0;
    pDelayInfo->AwbDelay = 4;
}


UINT32 S5K8AAYXMIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) {
	kal_uint32 pclk;
	kal_int16 dummyLine;
	kal_uint16 lineLength,frameHeight;
		
	SENSORDB("S5K8AAYXMIPISetMaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pclk = S5K8AAYX_MIPI_sensor_pclk/10;
			lineLength = S5K8AAYX_MIPI_SXGA_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - S5K8AAYX_MIPI_SXGA_PERIOD_LINE_NUMS;
			SENSORDB("S5K8AAYXMIPISetMaxFramerateByScenario MSDK_SCENARIO_ID_CAMERA_PREVIEW: lineLength = %d, dummy=%d\n",lineLength,dummyLine);
			S5K8AAYX_MIPI_set_dummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pclk = S5K8AAYX_MIPI_sensor_pclk/10;
			lineLength = S5K8AAYX_MIPI_SXGA_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - S5K8AAYX_MIPI_SXGA_PERIOD_LINE_NUMS;
			SENSORDB("S5K8AAYXMIPISetMaxFramerateByScenario MSDK_SCENARIO_ID_VIDEO_PREVIEW: lineLength = %d, dummy=%d\n",lineLength,dummyLine);			
			S5K8AAYX_MIPI_set_dummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:			
			pclk = S5K8AAYX_MIPI_sensor_pclk/10;
			lineLength = S5K8AAYX_MIPI_SXGA_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - S5K8AAYX_MIPI_SXGA_PERIOD_LINE_NUMS;
			SENSORDB("S5K8AAYXMIPISetMaxFramerateByScenario MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG: lineLength = %d, dummy=%d\n",lineLength,dummyLine);			
			S5K8AAYX_MIPI_set_dummy(0, dummyLine);			
			break;		
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
            break;
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
			break;
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			break;		
		default:
			break;
	}	
	return ERROR_NONE;
}


UINT32 S5K8AAYXMIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 *pframeRate = 300;
			break;		
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			 *pframeRate = 300;
			break;		
		default:
			break;
	}

	return ERROR_NONE;
}

UINT32 S5K8AAYXSetTestPatternMode(kal_bool bEnable)
{
    SENSORDB("[S5K8AAYXSetTestPatternMode] Test pattern enable:%d\n", bEnable);

	if(bEnable) 
	{
        //fix AE/AWB output setting
        S5K8AAYX_MIPI_write_cmos_sensor(0xFCFC,0xD000);
        S5K8AAYX_MIPI_write_cmos_sensor(0x3700,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x3E00,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x4300,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x4400,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x4500,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x4600,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x4700,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x6000,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x6100,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x6330,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x6400,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x6500,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x6700,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x6800,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x6A00,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x6B00,0x0001);
        S5K8AAYX_MIPI_write_cmos_sensor(0x4100,0x0001);
        
        //Output test pattern mode setting
        //0x0002 - solid color
        S5K8AAYX_MIPI_write_cmos_sensor(0x3602,0x1F40);
        S5K8AAYX_MIPI_write_cmos_sensor(0x3604,0x1A40);
        S5K8AAYX_MIPI_write_cmos_sensor(0x3606,0x1A40);
        S5K8AAYX_MIPI_write_cmos_sensor(0x3608,0x1040);
        //0x0004 - gradient
        S5K8AAYX_MIPI_write_cmos_sensor(0x360a,0x0383);
        //Address: D0003600
        //0x0000 -- bypass
        //0x0002 - solid color
        //0x0004 - gradient
        //0x0006 - address dependent noise
        //0x0008 - random
        //0x000A - gradient + address dependent noise
        //0x000C - gradient + random
        //0x000E - out pixel attributes
        S5K8AAYX_MIPI_write_cmos_sensor(0x3600,0x0004);

        mdelay(100);

	}
	else        
	{
		S5K8AAYX_MIPI_write_cmos_sensor(0x3600,0x0000);	
	}
    return ERROR_NONE;
}

UINT32 S5K8AAYX_MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
	//PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
	//MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
	//MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
	//MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;
#if WINMO_USE	
	PMSDK_FEATURE_INFO_STRUCT pSensorFeatureInfo=(PMSDK_FEATURE_INFO_STRUCT) pFeaturePara;
#endif 


	switch (FeatureId)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_WIDTH;
			*pFeatureReturnPara16=S5K8AAYX_MIPI_IMAGE_SENSOR_FULL_HEIGHT;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PERIOD:
			//*pFeatureReturnPara16++=S5K8AAYX_MIPI_PV_PERIOD_PIXEL_NUMS+S5K8AAYX_MIPI_PV_dummy_pixels;
			//*pFeatureReturnPara16=S5K8AAYX_MIPI_PV_PERIOD_LINE_NUMS+S5K8AAYX_MIPI_PV_dummy_lines;
			//*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			*pFeatureReturnPara32 = S5K8AAYX_MIPI_sensor_pclk/10;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_ESHUTTER:
		break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			S5K8AAYX_MIPI_night_mode((BOOL) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_GAIN:
		case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			S5K8AAYX_MIPI_isp_master_clock=*pFeatureData32;
		break;
		case SENSOR_FEATURE_SET_REGISTER:
			S5K8AAYX_MIPI_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
		break;
		case SENSOR_FEATURE_GET_REGISTER:
			pSensorRegData->RegData = S5K8AAYX_MIPI_read_cmos_sensor(pSensorRegData->RegAddr);
		break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			memcpy(pSensorConfigData, &S5K8AAYX_MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
			*pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
		break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
		case SENSOR_FEATURE_GET_CCT_REGISTER:
		case SENSOR_FEATURE_SET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:

		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
		
		case SENSOR_FEATURE_GET_GROUP_INFO:
		case SENSOR_FEATURE_GET_ITEM_INFO:
		case SENSOR_FEATURE_SET_ITEM_INFO:
		case SENSOR_FEATURE_GET_ENG_INFO:
					break;
		case SENSOR_FEATURE_GET_GROUP_COUNT:
                        *pFeatureReturnPara32++=0;
                        *pFeatureParaLen=4;	    

		break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_YUV_CMD:
			//S5K8AAYX_MIPIYUVSensorSetting((MSDK_ISP_FEATURE_ENUM)*pFeatureData16, *(pFeatureData16+1));
			S5K8AAYX_MIPIYUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
		break;

#if WINMO_USE		
		case SENSOR_FEATURE_QUERY:
			S5K8AAYX_MIPIQuery(pSensorFeatureInfo);
			*pFeatureParaLen = sizeof(MSDK_FEATURE_INFO_STRUCT);
		break;		
		case SENSOR_FEATURE_SET_YUV_CAPTURE_RAW_SUPPORT:
			/* update yuv capture raw support flag by *pFeatureData16 */
		break;
#endif 				
		case SENSOR_FEATURE_SET_VIDEO_MODE:
		    S5K8AAYX_MIPIYUVSetVideoMode(*pFeatureData16);
		    break; 
		//#if defined(MT6575)
		case SENSOR_FEATURE_GET_EV_AWB_REF:
			 S5K8AAYX_MIPIGetEvAwbRef(*pFeatureData32);
				break;
  		case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
			   S5K8AAYX_MIPIGetCurAeAwbInfo(*pFeatureData32);	
			break;
		//#endif
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
            S5K8AAYX_MIPI_GetSensorID(pFeatureData32); 
            break; 	

		case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:
			S5K8AAYX_MIPIGetAFMaxNumFocusAreas(pFeatureReturnPara32); 		   
			*pFeatureParaLen=4;
			break;	   
		case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
			S5K8AAYX_MIPIGetAFMaxNumMeteringAreas(pFeatureReturnPara32);			  
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_GET_EXIF_INFO:
			SENSORDB("SENSOR_FEATURE_GET_EXIF_INFO\n");
			SENSORDB("EXIF addr = 0x%x\n",*pFeatureData32); 		 
			S5K8AAYX_MIPIGetExifInfo(*pFeatureData32);
			break;
		case SENSOR_FEATURE_GET_DELAY_INFO:
			SENSORDB("SENSOR_FEATURE_GET_DELAY_INFO\n");
		    S5K8AAYX_MIPIGetDelayInfo(*pFeatureData32);
			break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			S5K8AAYXMIPISetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			S5K8AAYXMIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
			break;
		case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
			S5K8AAYX_MIPIGetAEAWBLock((*pFeatureData32),*(pFeatureData32+1));
			break;
        case SENSOR_FEATURE_SET_TEST_PATTERN://for factory mode auto testing    
            S5K8AAYXSetTestPatternMode((BOOL)*pFeatureData16);
            break;
        case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE://for factory mode auto testing             
            *pFeatureReturnPara32= S5K8AAYX_TEST_PATTERN_CHECKSUM;           
            *pFeatureParaLen=4;                             
            break;

		default:
			break;			
	}
	return ERROR_NONE;
}

SENSOR_FUNCTION_STRUCT	SensorFuncS5K8AAYX_MIPI=
{
	S5K8AAYX_MIPIOpen,
	S5K8AAYX_MIPIGetInfo,
	S5K8AAYX_MIPIGetResolution,
	S5K8AAYX_MIPIFeatureControl,
	S5K8AAYX_MIPIControl,
	S5K8AAYX_MIPIClose
};

UINT32 S5K8AAYX_MIPI_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncS5K8AAYX_MIPI;
	return ERROR_NONE;
}	/* SensorInit() */
