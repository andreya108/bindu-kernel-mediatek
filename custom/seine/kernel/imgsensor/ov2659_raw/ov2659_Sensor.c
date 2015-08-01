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
*  permission of MediaTek Inc. (C) 2005
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
 *   ov2659_Sensor.c
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/system.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "ov2659_Sensor.h"
#include "ov2659_Camera_Sensor_para.h"
#include "ov2659_CameraCustomized.h"

#define OV2659_DRIVER_TRACE
#define OV2659_DEBUG
#ifdef OV2659_DEBUG
#define SENSORDB(fmt, arg...) printk("%s: " fmt "\n", __FUNCTION__ ,##arg)
#else
#define SENSORDB(x,...)
#endif

typedef enum
{
    OV2659_SENSOR_MODE_INIT,
    OV2659_SENSOR_MODE_PREVIEW,  
    OV2659_SENSOR_MODE_CAPTURE
} OV2659_SENSOR_MODE;

/* SENSOR PRIVATE STRUCT */
typedef struct OV2659_sensor_STRUCT
{
    MSDK_SENSOR_CONFIG_STRUCT cfg_data;
    sensor_data_struct eng; /* engineer mode */
    MSDK_SENSOR_ENG_INFO_STRUCT eng_info;
    kal_uint8 mirror;

    OV2659_SENSOR_MODE ov2659_sensor_mode;
    
    kal_bool video_mode;
    kal_bool NightMode;
    kal_uint16 normal_fps; /* video normal mode max fps */
    kal_uint16 night_fps; /* video night mode max fps */
    kal_uint16 FixedFps;
    kal_uint16 shutter;
    kal_uint16 gain;
    kal_uint32 pclk;
    kal_uint16 frame_length;
    kal_uint16 line_length;

    kal_uint16 dummy_pixel;
    kal_uint16 dummy_line;
} OV2659_sensor_struct;


static MSDK_SCENARIO_ID_ENUM mCurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
static kal_bool OV2659AutoFlickerMode = KAL_FALSE;

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

UINT32 OV2659SetMaxFrameRate(UINT16 u2FrameRate);

static DEFINE_SPINLOCK(ov2659_drv_lock);


static OV2659_sensor_struct OV2659_sensor =
{
    .eng =
    {
        .reg = CAMERA_SENSOR_REG_DEFAULT_VALUE,
        .cct = CAMERA_SENSOR_CCT_DEFAULT_VALUE,
    },
    .eng_info =
    {
        .SensorId = 128,
        .SensorType = CMOS_SENSOR,
        .SensorOutputDataFormat = OV2659_COLOR_FORMAT,
    },
    .ov2659_sensor_mode = OV2659_SENSOR_MODE_INIT,
    .shutter = 0x450,  
    .gain = 0x100,
    .pclk = OV2659_PREVIEW_CLK,
    .frame_length = OV2659_PV_PERIOD_LINE_NUMS,
    .line_length = OV2659_PV_PERIOD_PIXEL_NUMS,
    .dummy_pixel = 0,
    .dummy_line = 0,
};


kal_uint16 OV2659_read_cmos_sensor(kal_uint32 addr)
{
    kal_uint16 get_byte=0;

    char puSendCmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };
    iReadRegI2C(puSendCmd, 2, (u8*)&get_byte, 1, OV2659_WRITE_ID);

    return get_byte;
}

kal_uint16 OV2659_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    char puSendCmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd, 3, OV2659_WRITE_ID);
}

static void OV2659_Write_Shutter(kal_uint16 iShutter)
{
    kal_uint16 extra_line = 0, frame_length;

    #ifdef OV2659_DRIVER_TRACE
        SENSORDB("iShutter =  %d", iShutter);
    #endif
    
    /* 0x3500, 0x3501, 0x3502 will increase VBLANK to get exposure larger than frame exposure */
    /* AE doesn't update sensor gain at capture mode, thus extra exposure lines must be updated here. */
    if (!iShutter) iShutter = 1;

    if(OV2659AutoFlickerMode){
        if(OV2659_sensor.video_mode == KAL_FALSE){
            if(mCurrentScenarioId == MSDK_SCENARIO_ID_CAMERA_ZSD)
            {
                //Change frame 14.7fps ~ 14.9fps to do auto flick
                OV2659SetMaxFrameRate(148);
            }
            else
            {
                //Change frame 29.5fps ~ 29.8fps to do auto flick
                OV2659SetMaxFrameRate(296);
            }
        }
    }

    // OV Recommend Solution
    // if shutter bigger than frame_length, should extend frame length first
#if 0

	if(iShutter > (OV2659_sensor.frame_length - 4))
		extra_line = iShutter - (OV2659_sensor.frame_length - 4);
	else
	    extra_line = 0;

	// Update Extra shutter
	OV2659_write_cmos_sensor(0x350c, (extra_line >> 8) & 0xFF);	
	OV2659_write_cmos_sensor(0x350d, (extra_line) & 0xFF); 
	
#endif

#if 1  

    if(iShutter > OV2659_sensor.frame_length - 4)
        frame_length = iShutter + 4;
    else
        frame_length = OV2659_sensor.frame_length;

    // Extend frame length
    OV2659_write_cmos_sensor(0x380f, frame_length & 0xFF);
    OV2659_write_cmos_sensor(0x380e, frame_length >> 8);
    
#endif

    // Update Shutter
    OV2659_write_cmos_sensor(0x3502, (iShutter << 4) & 0xFF);
    OV2659_write_cmos_sensor(0x3501, (iShutter >> 4) & 0xFF);     
    OV2659_write_cmos_sensor(0x3500, (iShutter >> 12) & 0x0F);
}   /*  OV2659_Write_Shutter  */

static void OV2659_Set_Dummy(const kal_uint16 iDummyPixels, const kal_uint16 iDummyLines)
{
    kal_uint16 hactive, vactive, line_length, frame_length;

    #ifdef OV2659_DRIVER_TRACE
        SENSORDB("iDummyPixels = %d, iDummyLines = %d ", iDummyPixels, iDummyLines);
    #endif

    if (OV2659_SENSOR_MODE_PREVIEW == OV2659_sensor.ov2659_sensor_mode){
        line_length = OV2659_PV_PERIOD_PIXEL_NUMS + iDummyPixels;
        frame_length = OV2659_PV_PERIOD_LINE_NUMS + iDummyLines;
    }else{
        line_length = OV2659_FULL_PERIOD_PIXEL_NUMS + iDummyPixels;
        frame_length = OV2659_FULL_PERIOD_LINE_NUMS + iDummyLines;
    }
    
    if ((line_length >= 0x1FFF)||(frame_length >= 0xFFF))
        return ;

    OV2659_sensor.dummy_pixel = iDummyPixels;
    OV2659_sensor.dummy_line = iDummyLines;
    OV2659_sensor.line_length = line_length;
    OV2659_sensor.frame_length = frame_length;
    
    //OV2659_write_cmos_sensor(0x380c, line_length >> 8);
    //OV2659_write_cmos_sensor(0x380d, line_length & 0xFF);
    //OV2659_write_cmos_sensor(0x380e, frame_length >> 8);
    //OV2659_write_cmos_sensor(0x380f, frame_length & 0xFF);
    
}   /*  OV2659_Set_Dummy  */


UINT32 OV2659SetMaxFrameRate(UINT16 u2FrameRate)
{
    kal_int16 dummy_line;
    kal_uint16 frame_length = OV2659_sensor.frame_length;
    unsigned long flags;

    #ifdef OV2659_DRIVER_TRACE
        SENSORDB("u2FrameRate = %d ", u2FrameRate);
    #endif

    frame_length= (10 * OV2659_sensor.pclk) / u2FrameRate / OV2659_sensor.line_length;

    spin_lock_irqsave(&ov2659_drv_lock, flags);
    OV2659_sensor.frame_length = frame_length;
    spin_unlock_irqrestore(&ov2659_drv_lock, flags);

    if(mCurrentScenarioId == MSDK_SCENARIO_ID_CAMERA_ZSD){
        if (frame_length > OV2659_FULL_PERIOD_LINE_NUMS)
            dummy_line = frame_length - OV2659_FULL_PERIOD_LINE_NUMS;
        else
            dummy_line = 0;
    }
    else {
        if (frame_length > OV2659_PV_PERIOD_LINE_NUMS)
            dummy_line = frame_length - OV2659_PV_PERIOD_LINE_NUMS;
        else
            dummy_line = 0;
    }

    OV2659_Set_Dummy(OV2659_sensor.dummy_pixel, dummy_line); /* modify dummy_pixel must gen AE table again */
}   /*  OV2659SetMaxFrameRate  */


/*************************************************************************
* FUNCTION
*   OV2659_SetShutter
*
* DESCRIPTION
*   This function set e-shutter of OV2659 to change exposure time.
*
* PARAMETERS
*   iShutter : exposured lines
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void set_OV2659_shutter(kal_uint16 iShutter)
{
    unsigned long flags;
    
    spin_lock_irqsave(&ov2659_drv_lock, flags);
    OV2659_sensor.shutter = iShutter;
    spin_unlock_irqrestore(&ov2659_drv_lock, flags);
    
    OV2659_Write_Shutter(iShutter);
}   /*  Set_OV2659_Shutter */


static kal_uint16 OV2659_Reg2Gain(const kal_uint8 iReg)
{
    kal_uint16 iGain ;
    /* Range: 1x to 32x */
    iGain = (iReg >> 4) * BASEGAIN + (iReg & 0xF) * BASEGAIN / 16; 
    return iGain ;
}


 kal_uint8 OV2659_Gain2Reg(const kal_uint16 iGain)
{
    kal_uint16 iReg = 0x0000;
    
    iReg = ((iGain / BASEGAIN) << 4) + ((iGain % BASEGAIN) * 16 / BASEGAIN);
    iReg = iReg & 0xFF;
    return (kal_uint8)iReg;
}


/*************************************************************************
* FUNCTION
*   OV2659_SetGain
*
* DESCRIPTION
*   This function is to set global gain to sensor.
*
* PARAMETERS
*   iGain : sensor global gain(base: 0x40)
*
* RETURNS
*   the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint16 OV2659_SetGain(kal_uint16 iGain)
{
    kal_uint8 iRegGain;

    OV2659_sensor.gain = iGain;

    /* 0x350A[0:1], 0x350B[0:7] AGC real gain */
    /* [0:3] = N meams N /16 X  */
    /* [4:9] = M meams M X       */
    /* Total gain = M + N /16 X   */

    //
    if(iGain >= BASEGAIN && iGain <= 32 * BASEGAIN){
    
        iRegGain = OV2659_Gain2Reg(iGain);

        #ifdef OV2659_DRIVER_TRACE
            SENSORDB("iGain = %d , iRegGain = 0x%x ", iGain, iRegGain);
        #endif

        if (iRegGain < 0x10) iRegGain = 0x10;
        OV2659_write_cmos_sensor(0x350b, iRegGain);
    } else {
        SENSORDB("Error gain setting");
    }
    
    return iGain;
}   /*  OV2659_SetGain  */


void OV2659_Set_Mirror_Flip(kal_uint8 image_mirror)
{
    SENSORDB("image_mirror = %d", image_mirror);

    /********************************************************
       *
       *   0x3820[2] ISP Vertical flip
       *   0x3820[1] Sensor Vertical flip
       *
       *   0x3821[2] ISP Horizontal mirror
       *   0x3821[1] Sensor Horizontal mirror
       *
       *   ISP and Sensor flip or mirror register bit should be the same!!
       *
       ********************************************************/
    
	switch (image_mirror)
	{
		case IMAGE_NORMAL:
		    OV2659_write_cmos_sensor(0x3820,((OV2659_read_cmos_sensor(0x3820) & !0x06) | 0x00));
		    OV2659_write_cmos_sensor(0x3821,((OV2659_read_cmos_sensor(0x3821) & !0x06) | 0x00));
		    break;
		case IMAGE_H_MIRROR:
		    OV2659_write_cmos_sensor(0x3820,((OV2659_read_cmos_sensor(0x3820) & !0x06) | 0x06));
		    OV2659_write_cmos_sensor(0x3821,((OV2659_read_cmos_sensor(0x3821) & !0x06) | 0x00));
		    break;
		case IMAGE_V_MIRROR:
		    OV2659_write_cmos_sensor(0x3820,((OV2659_read_cmos_sensor(0x3820) & !0x06) | 0x00));
		    OV2659_write_cmos_sensor(0x3821,((OV2659_read_cmos_sensor(0x3821) & !0x06) | 0x06));		
		    break;
		case IMAGE_HV_MIRROR:
		    OV2659_write_cmos_sensor(0x3820,((OV2659_read_cmos_sensor(0x3820) & !0x06) | 0x06));
		    OV2659_write_cmos_sensor(0x3821,((OV2659_read_cmos_sensor(0x3821) & !0x06) | 0x06));
		    break;
		default:
		    SENSORDB("Error image_mirror setting");
	}
}


/*************************************************************************
* FUNCTION
*   OV2659_NightMode
*
* DESCRIPTION
*   This function night mode of OV2659.
*
* PARAMETERS
*   bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void OV2659_night_mode(kal_bool enable)
{
/*No Need to implement this function*/ 
}   /*  OV2659_night_mode  */


/* write camera_para to sensor register */
static void OV2659_camera_para_to_sensor(void)
{
    kal_uint32 i;

    SENSORDB("OV2659_camera_para_to_sensor\n");

    for (i = 0; 0xFFFFFFFF != OV2659_sensor.eng.reg[i].Addr; i++)
    {
        OV2659_write_cmos_sensor(OV2659_sensor.eng.reg[i].Addr, OV2659_sensor.eng.reg[i].Para);
    }
    for (i = OV2659_FACTORY_START_ADDR; 0xFFFFFFFF != OV2659_sensor.eng.reg[i].Addr; i++)
    {
        OV2659_write_cmos_sensor(OV2659_sensor.eng.reg[i].Addr, OV2659_sensor.eng.reg[i].Para);
    }
    OV2659_SetGain(OV2659_sensor.gain); /* update gain */
}

/* update camera_para from sensor register */
static void OV2659_sensor_to_camera_para(void)
{
    kal_uint32 i,temp_data;

    SENSORDB("OV2659_sensor_to_camera_para\n");

    for (i = 0; 0xFFFFFFFF != OV2659_sensor.eng.reg[i].Addr; i++)
    {
        temp_data =OV2659_read_cmos_sensor(OV2659_sensor.eng.reg[i].Addr);
     
        spin_lock(&ov2659_drv_lock);
        OV2659_sensor.eng.reg[i].Para = temp_data;
        spin_unlock(&ov2659_drv_lock);
    }
    for (i = OV2659_FACTORY_START_ADDR; 0xFFFFFFFF != OV2659_sensor.eng.reg[i].Addr; i++)
    {
        temp_data =OV2659_read_cmos_sensor(OV2659_sensor.eng.reg[i].Addr);
    
        spin_lock(&ov2659_drv_lock);
        OV2659_sensor.eng.reg[i].Para = temp_data;
        spin_unlock(&ov2659_drv_lock);
    }
}

/* ------------------------ Engineer mode ------------------------ */
inline static void OV2659_get_sensor_group_count(kal_int32 *sensor_count_ptr)
{

    SENSORDB("OV2659_get_sensor_group_count\n");

    *sensor_count_ptr = OV2659_GROUP_TOTAL_NUMS;
}

inline static void OV2659_get_sensor_group_info(MSDK_SENSOR_GROUP_INFO_STRUCT *para)
{

    SENSORDB("OV2659_get_sensor_group_info\n");

    switch (para->GroupIdx)
    {
    case OV2659_PRE_GAIN:
        sprintf(para->GroupNamePtr, "CCT");
        para->ItemCount = 5;
        break;
    case OV2659_CMMCLK_CURRENT:
        sprintf(para->GroupNamePtr, "CMMCLK Current");
        para->ItemCount = 1;
        break;
    case OV2659_FRAME_RATE_LIMITATION:
        sprintf(para->GroupNamePtr, "Frame Rate Limitation");
        para->ItemCount = 2;
        break;
    case OV2659_REGISTER_EDITOR:
        sprintf(para->GroupNamePtr, "Register Editor");
        para->ItemCount = 2;
        break;
    default:
        ASSERT(0);
  }
}

inline static void OV2659_get_sensor_item_info(MSDK_SENSOR_ITEM_INFO_STRUCT *para)
{

    const static kal_char *cct_item_name[] = {"SENSOR_BASEGAIN", "Pregain-R", "Pregain-Gr", "Pregain-Gb", "Pregain-B"};
    const static kal_char *editer_item_name[] = {"REG addr", "REG value"};
  
    SENSORDB("OV2659_get_sensor_item_info");

    switch (para->GroupIdx)
    {
    case OV2659_PRE_GAIN:
        switch (para->ItemIdx)
        {
        case OV2659_SENSOR_BASEGAIN:
        case OV2659_PRE_GAIN_R_INDEX:
        case OV2659_PRE_GAIN_Gr_INDEX:
        case OV2659_PRE_GAIN_Gb_INDEX:
        case OV2659_PRE_GAIN_B_INDEX:
            break;
        default:
            ASSERT(0);
        }
        sprintf(para->ItemNamePtr, cct_item_name[para->ItemIdx - OV2659_SENSOR_BASEGAIN]);
        para->ItemValue = OV2659_sensor.eng.cct[para->ItemIdx].Para * 1000 / BASEGAIN;
        para->IsTrueFalse = para->IsReadOnly = para->IsNeedRestart = KAL_FALSE;
        para->Min = OV2659_MIN_ANALOG_GAIN * 1000;
        para->Max = OV2659_MAX_ANALOG_GAIN * 1000;
        break;
    case OV2659_CMMCLK_CURRENT:
        switch (para->ItemIdx)
        {
        case 0:
            sprintf(para->ItemNamePtr, "Drv Cur[2,4,6,8]mA");
            switch (OV2659_sensor.eng.reg[OV2659_CMMCLK_CURRENT_INDEX].Para)
            {
            case ISP_DRIVING_2MA:
                para->ItemValue = 2;
                break;
            case ISP_DRIVING_4MA:
                para->ItemValue = 4;
                break;
            case ISP_DRIVING_6MA:
                para->ItemValue = 6;
                break;
            case ISP_DRIVING_8MA:
                para->ItemValue = 8;
                break;
            default:
                ASSERT(0);
            }
            para->IsTrueFalse = para->IsReadOnly = KAL_FALSE;
            para->IsNeedRestart = KAL_TRUE;
            para->Min = 2;
            para->Max = 8;
            break;
        default:
            ASSERT(0);
        }
        break;
    case OV2659_FRAME_RATE_LIMITATION:
        switch (para->ItemIdx)
        {
        case 0:
            sprintf(para->ItemNamePtr, "Max Exposure Lines");
            para->ItemValue = 5998;
            break;
        case 1:
            sprintf(para->ItemNamePtr, "Min Frame Rate");
            para->ItemValue = 5;
            break;
        default:
            ASSERT(0);
        }
        para->IsTrueFalse = para->IsNeedRestart = KAL_FALSE;
        para->IsReadOnly = KAL_TRUE;
        para->Min = para->Max = 0;
        break;
    case OV2659_REGISTER_EDITOR:
        switch (para->ItemIdx)
        {
        case 0:
        case 1:
            sprintf(para->ItemNamePtr, editer_item_name[para->ItemIdx]);
            para->ItemValue = 0;
            para->IsTrueFalse = para->IsReadOnly = para->IsNeedRestart = KAL_FALSE;
            para->Min = 0;
            para->Max = (para->ItemIdx == 0 ? 0xFFFF : 0xFF);
            break;
        default:
            ASSERT(0);
        }
        break;
    default:
        ASSERT(0);
  }
}

inline static kal_bool OV2659_set_sensor_item_info(MSDK_SENSOR_ITEM_INFO_STRUCT *para)
{
    kal_uint16 temp_para;

    SENSORDB("OV2659_set_sensor_item_info\n");

    switch (para->GroupIdx)
    {
    case OV2659_PRE_GAIN:
        switch (para->ItemIdx)
        {
        case OV2659_SENSOR_BASEGAIN:
        case OV2659_PRE_GAIN_R_INDEX:
        case OV2659_PRE_GAIN_Gr_INDEX:
        case OV2659_PRE_GAIN_Gb_INDEX:
        case OV2659_PRE_GAIN_B_INDEX:
            spin_lock(&ov2659_drv_lock);
            OV2659_sensor.eng.cct[para->ItemIdx].Para = para->ItemValue * BASEGAIN / 1000;
            spin_unlock(&ov2659_drv_lock);
            OV2659_SetGain(OV2659_sensor.gain); /* update gain */
            break;
        default:
            ASSERT(0);
        }
        break;
    case OV2659_CMMCLK_CURRENT:
        switch (para->ItemIdx)
        {
        case 0:
            switch (para->ItemValue)
            {
            case 2:
                temp_para = ISP_DRIVING_2MA;
                break;
            case 3:
            case 4:
                temp_para = ISP_DRIVING_4MA;
                break;
            case 5:
            case 6:
                temp_para = ISP_DRIVING_6MA;
                break;
            default:
                temp_para = ISP_DRIVING_8MA;
                break;
            }
            spin_lock(&ov2659_drv_lock);
            //OV2659_set_isp_driving_current(temp_para);
            OV2659_sensor.eng.reg[OV2659_CMMCLK_CURRENT_INDEX].Para = temp_para;
            spin_unlock(&ov2659_drv_lock);
            break;
        default:
            ASSERT(0);
        }
        break;
    case OV2659_FRAME_RATE_LIMITATION:
        ASSERT(0);
        break;
    case OV2659_REGISTER_EDITOR:
        switch (para->ItemIdx)
        {
        static kal_uint32 fac_sensor_reg;
        case 0:
            if (para->ItemValue < 0 || para->ItemValue > 0xFFFF) return KAL_FALSE;
            fac_sensor_reg = para->ItemValue;
            break;
        case 1:
            if (para->ItemValue < 0 || para->ItemValue > 0xFF) return KAL_FALSE;
            OV2659_write_cmos_sensor(fac_sensor_reg, para->ItemValue);
            break;
        default:
            ASSERT(0);
        }
        break;
    default:
        ASSERT(0);
    }

    return KAL_TRUE;
}

static void OV2659_Sensor_Init(void)
{
    SENSORDB("Enter!");

    /***********************************************************************  
        * Raw Full 1600x1200 15fps
        * MCLK 24M  PCLK 36M
        */

    OV2659_write_cmos_sensor(0x0103, 0x01); // Software Reset
   
    OV2659_write_cmos_sensor(0x3000, 0x0f); // DVP D[9:8]
    OV2659_write_cmos_sensor(0x3001, 0xff); // DVP D[7:0]
    OV2659_write_cmos_sensor(0x3002, 0xff); // VSYNC HSYNC PCLK output enable

    //OV2659_write_cmos_sensor(0x3003, 0x00); // PLL
    //OV2659_write_cmos_sensor(0x3004, 0x10); // PLL
    //OV2659_write_cmos_sensor(0x3005, 0x24); // PLL
    //OV2659_write_cmos_sensor(0x3006, 0x0D); // PLL
    
    OV2659_write_cmos_sensor(0x0100, 0x01); // wake up from software sleep
    
    OV2659_write_cmos_sensor(0x3633, 0x3d);
    OV2659_write_cmos_sensor(0x3620, 0x02);
    OV2659_write_cmos_sensor(0x3631, 0x11);
    OV2659_write_cmos_sensor(0x3612, 0x04);
    OV2659_write_cmos_sensor(0x3630, 0x20);
    OV2659_write_cmos_sensor(0x4702, 0x02);
    OV2659_write_cmos_sensor(0x370c, 0x34);
    OV2659_write_cmos_sensor(0x3004, 0x10);
    OV2659_write_cmos_sensor(0x3005, 0x24);
    
    OV2659_write_cmos_sensor(0x3800, 0x00); // xstart = 0
    OV2659_write_cmos_sensor(0x3801, 0x00); // xstart
    OV2659_write_cmos_sensor(0x3802, 0x00); // ystart = 0
    OV2659_write_cmos_sensor(0x3803, 0x00); // ystart
    OV2659_write_cmos_sensor(0x3804, 0x06); // xend = 1631
    OV2659_write_cmos_sensor(0x3805, 0x5f); // yend
    OV2659_write_cmos_sensor(0x3806, 0x04); // yend = 1211
    OV2659_write_cmos_sensor(0x3807, 0xbb); // yend
    
    OV2659_write_cmos_sensor(0x3808, 0x06); // x output size = 1600
    OV2659_write_cmos_sensor(0x3809, 0x40); // x output size
    OV2659_write_cmos_sensor(0x380a, 0x04); // y output size = 1200
    OV2659_write_cmos_sensor(0x380b, 0xb0); // y output size
    
    OV2659_write_cmos_sensor(0x380c, 0x07); // hts = 1951
    OV2659_write_cmos_sensor(0x380d, 0x9f); // hts
    OV2659_write_cmos_sensor(0x380e, 0x04); // vts = 1232
    OV2659_write_cmos_sensor(0x380f, 0xd0); // vts

    OV2659_write_cmos_sensor(0x3811, 0x10); // isp x win 16
    OV2659_write_cmos_sensor(0x3813, 0x06); // isp y win 6
    
    OV2659_write_cmos_sensor(0x3814, 0x11); // x inc
    OV2659_write_cmos_sensor(0x3815, 0x11); // y inc
    
    OV2659_write_cmos_sensor(0x3a02, 0x04);
    OV2659_write_cmos_sensor(0x3a03, 0xd0);
    OV2659_write_cmos_sensor(0x3a08, 0x00);
    OV2659_write_cmos_sensor(0x3a09, 0xb8);
    OV2659_write_cmos_sensor(0x3a0a, 0x00);
    OV2659_write_cmos_sensor(0x3a0b, 0x9a);
    OV2659_write_cmos_sensor(0x3a0d, 0x08);
    OV2659_write_cmos_sensor(0x3a0e, 0x06);
    OV2659_write_cmos_sensor(0x3a14, 0x04);
    OV2659_write_cmos_sensor(0x3a15, 0x50);
    OV2659_write_cmos_sensor(0x3623, 0x00);
    OV2659_write_cmos_sensor(0x3634, 0x44);
    OV2659_write_cmos_sensor(0x3701, 0x44);
    OV2659_write_cmos_sensor(0x3702, 0x30);
    OV2659_write_cmos_sensor(0x3703, 0x48);
    OV2659_write_cmos_sensor(0x3704, 0x48);
    OV2659_write_cmos_sensor(0x3705, 0x18);
    
    OV2659_write_cmos_sensor(0x3820, 0x80); // flip off, v bin off
    OV2659_write_cmos_sensor(0x3821, 0x00); // mirror on, v bin off
    
    OV2659_write_cmos_sensor(0x370a, 0x12);
    OV2659_write_cmos_sensor(0x4608, 0x00);
    OV2659_write_cmos_sensor(0x4609, 0x80);
    OV2659_write_cmos_sensor(0x4300, 0xf8);
    OV2659_write_cmos_sensor(0x5086, 0x00);
    OV2659_write_cmos_sensor(0x5000, 0x83);
    OV2659_write_cmos_sensor(0x5001, 0x00);
    OV2659_write_cmos_sensor(0x5002, 0x00);
    OV2659_write_cmos_sensor(0x5025, 0x0e);
    OV2659_write_cmos_sensor(0x5026, 0x18);
    OV2659_write_cmos_sensor(0x5027, 0x34);
    OV2659_write_cmos_sensor(0x5028, 0x4c);
    OV2659_write_cmos_sensor(0x5029, 0x62);
    OV2659_write_cmos_sensor(0x502a, 0x74);
    OV2659_write_cmos_sensor(0x502b, 0x85);
    OV2659_write_cmos_sensor(0x502c, 0x92);
    OV2659_write_cmos_sensor(0x502d, 0x9e);
    OV2659_write_cmos_sensor(0x502e, 0xb2);
    OV2659_write_cmos_sensor(0x502f, 0xc0);
    OV2659_write_cmos_sensor(0x5030, 0xcc);
    OV2659_write_cmos_sensor(0x5031, 0xe0);
    OV2659_write_cmos_sensor(0x5032, 0xee);
    OV2659_write_cmos_sensor(0x5033, 0xf6);
    OV2659_write_cmos_sensor(0x5034, 0x11);
    OV2659_write_cmos_sensor(0x5070, 0x1c);
    OV2659_write_cmos_sensor(0x5071, 0x5b);
    OV2659_write_cmos_sensor(0x5072, 0x05);
    OV2659_write_cmos_sensor(0x5073, 0x20);
    OV2659_write_cmos_sensor(0x5074, 0x94);
    OV2659_write_cmos_sensor(0x5075, 0xb4);
    OV2659_write_cmos_sensor(0x5076, 0xb4);
    OV2659_write_cmos_sensor(0x5077, 0xaf);
    OV2659_write_cmos_sensor(0x5078, 0x05);
    OV2659_write_cmos_sensor(0x5079, 0x98);
    OV2659_write_cmos_sensor(0x507a, 0x21);
    OV2659_write_cmos_sensor(0x5035, 0x6a);
    OV2659_write_cmos_sensor(0x5036, 0x11);
    OV2659_write_cmos_sensor(0x5037, 0x92);
    OV2659_write_cmos_sensor(0x5038, 0x21);
    OV2659_write_cmos_sensor(0x5039, 0xe1);
    OV2659_write_cmos_sensor(0x503a, 0x01);
    OV2659_write_cmos_sensor(0x503c, 0x05);
    OV2659_write_cmos_sensor(0x503d, 0x08);
    OV2659_write_cmos_sensor(0x503e, 0x08);
    OV2659_write_cmos_sensor(0x503f, 0x64);
    OV2659_write_cmos_sensor(0x5040, 0x58);
    OV2659_write_cmos_sensor(0x5041, 0x2a);
    OV2659_write_cmos_sensor(0x5042, 0xc5);
    OV2659_write_cmos_sensor(0x5043, 0x2e);
    OV2659_write_cmos_sensor(0x5044, 0x3a);
    OV2659_write_cmos_sensor(0x5045, 0x3c);
    OV2659_write_cmos_sensor(0x5046, 0x44);
    OV2659_write_cmos_sensor(0x5047, 0xf8);
    OV2659_write_cmos_sensor(0x5048, 0x08);
    OV2659_write_cmos_sensor(0x5049, 0x70);
    OV2659_write_cmos_sensor(0x504a, 0xf0);
    OV2659_write_cmos_sensor(0x504b, 0xf0);
    OV2659_write_cmos_sensor(0x500c, 0x03);
    OV2659_write_cmos_sensor(0x500d, 0x20);
    OV2659_write_cmos_sensor(0x500e, 0x02);
    OV2659_write_cmos_sensor(0x500f, 0x5c);
    OV2659_write_cmos_sensor(0x5010, 0x48);
    OV2659_write_cmos_sensor(0x5011, 0x00);
    OV2659_write_cmos_sensor(0x5012, 0x66);
    OV2659_write_cmos_sensor(0x5013, 0x03);
    OV2659_write_cmos_sensor(0x5014, 0x30);
    OV2659_write_cmos_sensor(0x5015, 0x02);
    OV2659_write_cmos_sensor(0x5016, 0x7c);
    OV2659_write_cmos_sensor(0x5017, 0x40);
    OV2659_write_cmos_sensor(0x5018, 0x00);
    OV2659_write_cmos_sensor(0x5019, 0x66);
    OV2659_write_cmos_sensor(0x501a, 0x03);
    OV2659_write_cmos_sensor(0x501b, 0x10);
    OV2659_write_cmos_sensor(0x501c, 0x02);
    OV2659_write_cmos_sensor(0x501d, 0x7c);
    OV2659_write_cmos_sensor(0x501e, 0x3a);
    OV2659_write_cmos_sensor(0x501f, 0x00);
    OV2659_write_cmos_sensor(0x5020, 0x66);
    OV2659_write_cmos_sensor(0x506e, 0x44);
    OV2659_write_cmos_sensor(0x5064, 0x08);
    OV2659_write_cmos_sensor(0x5065, 0x10);
    OV2659_write_cmos_sensor(0x5066, 0x12);
    OV2659_write_cmos_sensor(0x5067, 0x02);
    OV2659_write_cmos_sensor(0x506c, 0x08);
    OV2659_write_cmos_sensor(0x506d, 0x10);
    OV2659_write_cmos_sensor(0x506f, 0xa6);
    OV2659_write_cmos_sensor(0x5068, 0x08);
    OV2659_write_cmos_sensor(0x5069, 0x10);
    OV2659_write_cmos_sensor(0x506a, 0x04);
    OV2659_write_cmos_sensor(0x506b, 0x12);
    OV2659_write_cmos_sensor(0x507e, 0x40);
    OV2659_write_cmos_sensor(0x507f, 0x20);
    OV2659_write_cmos_sensor(0x507b, 0x02);
    OV2659_write_cmos_sensor(0x507a, 0x01);
    OV2659_write_cmos_sensor(0x5084, 0x0c);
    OV2659_write_cmos_sensor(0x5085, 0x3e);
    OV2659_write_cmos_sensor(0x5005, 0x80);
    OV2659_write_cmos_sensor(0x3a0f, 0x30);
    OV2659_write_cmos_sensor(0x3a10, 0x28);
    OV2659_write_cmos_sensor(0x3a1b, 0x32);
    OV2659_write_cmos_sensor(0x3a1e, 0x26);
    OV2659_write_cmos_sensor(0x3a11, 0x60);
    OV2659_write_cmos_sensor(0x3a1f, 0x14);
    
    OV2659_write_cmos_sensor(0x5060, 0x69); //Average Control Reg
    OV2659_write_cmos_sensor(0x5061, 0x7d); //Average Control Reg
    OV2659_write_cmos_sensor(0x5062, 0x7d); //Average Control Reg
    OV2659_write_cmos_sensor(0x5063, 0x69); //Average Control Reg

    OV2659_write_cmos_sensor(0x3503, 0x07); //close AE

    // exposure time (shutter)
    OV2659_write_cmos_sensor(0x3500, 0x00); //1/15s
    OV2659_write_cmos_sensor(0x3501, 0x45); //1/15s
    OV2659_write_cmos_sensor(0x3502, 0x00);

    // gain
    OV2659_write_cmos_sensor(0x350a, 0x00);
    OV2659_write_cmos_sensor(0x350b, 0x40); //4x gain
}   /*  OV2659_Sensor_Init  */


static void OV2659_Preview_Setting(void)
{
    SENSORDB("Enter!");

}   /*  OV2659_Preview_Setting  */


static void OV2659_Capture_Setting(void)
{
    SENSORDB("Enter!");


}   /*  OV2659_Capture_Setting  */


/*************************************************************************
* FUNCTION
*   OV2659Open
*
* DESCRIPTION
*   This function initialize the registers of CMOS sensor
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV2659Open(void)
{
    kal_uint16 sensor_id = 0; 

    // check if sensor ID correct
    sensor_id=((OV2659_read_cmos_sensor(0x300A) << 8) | OV2659_read_cmos_sensor(0x300B));   
    SENSORDB("sensor_id = 0x%x ", sensor_id);
    
    if (sensor_id != OV2659_SENSOR_ID){
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    
    /* initail sequence write in  */
    OV2659_Sensor_Init();

    spin_lock(&ov2659_drv_lock);
    OV2659AutoFlickerMode = KAL_FALSE;
    OV2659_sensor.ov2659_sensor_mode = OV2659_SENSOR_MODE_INIT;
    OV2659_sensor.shutter = 0x450;
    OV2659_sensor.gain = 0x100;
    OV2659_sensor.pclk = OV2659_PREVIEW_CLK;
    OV2659_sensor.frame_length = OV2659_PV_PERIOD_LINE_NUMS;
    OV2659_sensor.line_length = OV2659_PV_PERIOD_PIXEL_NUMS;
    OV2659_sensor.dummy_pixel = 0;
    OV2659_sensor.dummy_line = 0;
    spin_unlock(&ov2659_drv_lock);

    return ERROR_NONE;
}   /*  OV2659Open  */


/*************************************************************************
* FUNCTION
*   OV2659GetSensorID
*
* DESCRIPTION
*   This function get the sensor ID 
*
* PARAMETERS
*   *sensorID : return the sensor ID 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV2659GetSensorID(UINT32 *sensorID) 
{
    volatile signed char i;
    kal_uint32 sensor_id=0;

    OV2659_write_cmos_sensor(0x0103, 0x01);// Reset sensor
    mDELAY(10);

    //  Read sensor ID to adjust I2C is OK?
    for(i=0; i<3; i++)
    {
        sensor_id = (OV2659_read_cmos_sensor(0x300A) << 8) | OV2659_read_cmos_sensor(0x300B);
        
        SENSORDB("Sensor ID: 0x%x ", *sensorID);
        
        if(sensor_id != OV2659_SENSOR_ID)
        {   
            *sensorID = 0xFFFFFFFF;
            return ERROR_SENSOR_CONNECT_FAIL;
        }
    }

    *sensorID = sensor_id;
	
    return ERROR_NONE; 
}

/*************************************************************************
* FUNCTION
*   OV2659Close
*
* DESCRIPTION
*   
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV2659Close(void)
{

    /*No Need to implement this function*/ 
    
    return ERROR_NONE;
}   /*  OV2659Close  */


/*************************************************************************
* FUNCTION
* OV2659Preview
*
* DESCRIPTION
*   This function start the sensor preview.
*
* PARAMETERS
*   *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV2659Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 dummy_pixel, dummy_line;

    SENSORDB("Enter!");
    
    OV2659_Preview_Setting();

    spin_lock(&ov2659_drv_lock);
    OV2659_sensor.ov2659_sensor_mode = OV2659_SENSOR_MODE_PREVIEW;
    OV2659_sensor.pclk = OV2659_PREVIEW_CLK;
    OV2659_sensor.video_mode = KAL_FALSE;
    spin_unlock(&ov2659_drv_lock);

    //OV2659_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);

    dummy_pixel = 0;
    dummy_line = 0;
    OV2659_Set_Dummy(dummy_pixel, dummy_line); /* modify dummy_pixel must gen AE table again */

    SENSORDB("Exit!");
    
    return ERROR_NONE;
}   /*  OV2659Preview   */

UINT32 OV2659ZSDPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 dummy_pixel, dummy_line;

    SENSORDB("Enter!");

    OV2659_Capture_Setting();

    spin_lock(&ov2659_drv_lock);
    OV2659_sensor.ov2659_sensor_mode = OV2659_SENSOR_MODE_CAPTURE;
    OV2659_sensor.pclk = OV2659_CAPTURE_CLK;
    OV2659_sensor.video_mode = KAL_FALSE;
    spin_unlock(&ov2659_drv_lock);

    //OV2659_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);

    dummy_pixel = 0;
    dummy_line = 0;   
    OV2659_Set_Dummy(dummy_pixel, dummy_line); /* modify dummy_pixel must gen AE table again */

    SENSORDB("Exit!");

    return ERROR_NONE;
   
}   /*  OV2659ZSDPreview   */

/*************************************************************************
* FUNCTION
*   OV2659Capture
*
* DESCRIPTION
*   This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV2659Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                          MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 dummy_pixel, dummy_line, cap_shutter, cap_gain;
    kal_uint16 pre_shutter = OV2659_sensor.shutter;
    kal_uint16 pre_gain = OV2659_sensor.gain;

    SENSORDB("Enter!");

    if(OV2659_SENSOR_MODE_PREVIEW == OV2659_sensor.ov2659_sensor_mode)
    {
        OV2659_Capture_Setting();
    }
        
    spin_lock(&ov2659_drv_lock);
    OV2659_sensor.ov2659_sensor_mode = OV2659_SENSOR_MODE_CAPTURE;
    OV2659_sensor.pclk = OV2659_CAPTURE_CLK;
    OV2659_sensor.video_mode = KAL_FALSE;
    OV2659AutoFlickerMode = KAL_FALSE;    
    spin_unlock(&ov2659_drv_lock);

    //OV2659_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);

    dummy_pixel = 0;
    dummy_line = 0;               
    OV2659_Set_Dummy(dummy_pixel, dummy_line);

    SENSORDB("Exit!");
    
    return ERROR_NONE;
}   /* OV2659_Capture() */

UINT32 OV2659GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
#ifdef OV2659_DRIVER_TRACE
        SENSORDB("Enter");
#endif

    pSensorResolution->SensorFullWidth = OV2659_IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight = OV2659_IMAGE_SENSOR_FULL_HEIGHT;
    
    pSensorResolution->SensorPreviewWidth = OV2659_IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight = OV2659_IMAGE_SENSOR_PV_HEIGHT;

    pSensorResolution->SensorVideoWidth = OV2659_IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorVideoHeight = OV2659_IMAGE_SENSOR_PV_HEIGHT;		
    
    return ERROR_NONE;
}   /*  OV2659GetResolution  */

UINT32 OV2659GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                      MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                      MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
#ifdef OV2659_DRIVER_TRACE
    SENSORDB("ScenarioId = %d", ScenarioId);
#endif

    switch(ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorPreviewResolutionX = OV2659_IMAGE_SENSOR_FULL_WIDTH; /* not use */
            pSensorInfo->SensorPreviewResolutionY = OV2659_IMAGE_SENSOR_FULL_HEIGHT; /* not use */
            pSensorInfo->SensorCameraPreviewFrameRate = 15; /* not use */
            break;

        default:
            pSensorInfo->SensorPreviewResolutionX = OV2659_IMAGE_SENSOR_PV_WIDTH; /* not use */
            pSensorInfo->SensorPreviewResolutionY = OV2659_IMAGE_SENSOR_PV_HEIGHT; /* not use */
            pSensorInfo->SensorCameraPreviewFrameRate = 15; /* not use */
            break;
    }

    pSensorInfo->SensorFullResolutionX = OV2659_IMAGE_SENSOR_FULL_WIDTH; /* not use */
    pSensorInfo->SensorFullResolutionY = OV2659_IMAGE_SENSOR_FULL_HEIGHT; /* not use */

    pSensorInfo->SensorVideoFrameRate = 15; /* not use */
	pSensorInfo->SensorStillCaptureFrameRate= 15; /* not use */
	pSensorInfo->SensorWebCamCaptureFrameRate= 15; /* not use */

    pSensorInfo->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW; /* not use */
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 4; /* not use */
    pSensorInfo->SensorResetActiveHigh = FALSE; /* not use */
    pSensorInfo->SensorResetDelayCount = 5; /* not use */

    pSensorInfo->SensroInterfaceType = SENSOR_INTERFACE_TYPE_PARALLEL;

    pSensorInfo->SensorOutputDataFormat = OV2659_COLOR_FORMAT;

    pSensorInfo->CaptureDelayFrame = 2; 
    pSensorInfo->PreviewDelayFrame = 2; 
    pSensorInfo->VideoDelayFrame = 2; 

    pSensorInfo->SensorMasterClockSwitch = 0; /* not use */
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;
    
    pSensorInfo->AEShutDelayFrame = 0;          /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 0;    /* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 2;   

    switch (ScenarioId)
    {
	    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
	    case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorClockFreq = 24;
			pSensorInfo->SensorClockDividCount = 3; /* not use */
			pSensorInfo->SensorClockRisingCount = 0;
			pSensorInfo->SensorClockFallingCount = 2; /* not use */
			pSensorInfo->SensorPixelClockCount = 3; /* not use */
			pSensorInfo->SensorDataLatchCount = 2; /* not use */
	        pSensorInfo->SensorGrabStartX = OV2659_FULL_START_X; 
	        pSensorInfo->SensorGrabStartY = OV2659_FULL_START_Y;
	        break;
	    default:
	        pSensorInfo->SensorClockFreq = 24;
			pSensorInfo->SensorClockDividCount = 3; /* not use */
			pSensorInfo->SensorClockRisingCount = 0;
			pSensorInfo->SensorClockFallingCount = 2; /* not use */
			pSensorInfo->SensorPixelClockCount = 3; /* not use */
			pSensorInfo->SensorDataLatchCount = 2; /* not use */
	        pSensorInfo->SensorGrabStartX = OV2659_PV_START_X; 
	        pSensorInfo->SensorGrabStartY = OV2659_PV_START_Y;
	        break;
    }
	
	return ERROR_NONE;
}   /*  OV2659GetInfo  */


UINT32 OV2659Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                      MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    SENSORDB("ScenarioId = %d", ScenarioId);

    mCurrentScenarioId =ScenarioId;
    
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            OV2659Preview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            OV2659Capture(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            OV2659ZSDPreview(pImageWindow, pSensorConfigData);
            break;      
        default:
            SENSORDB("Error ScenarioId setting");
            return ERROR_INVALID_SCENARIO_ID;
    }

    return ERROR_NONE;
}   /* OV2659Control() */



UINT32 OV2659SetVideoMode(UINT16 u2FrameRate)
{
    SENSORDB("u2FrameRate = %d ", u2FrameRate);

    // SetVideoMode Function should fix framerate
    if(u2FrameRate < 5){
        // Dynamic frame rate
        return ERROR_NONE;
    }

    spin_lock(&ov2659_drv_lock);
    OV2659_sensor.video_mode = KAL_TRUE;
    spin_unlock(&ov2659_drv_lock);

    if(u2FrameRate == 30){
        spin_lock(&ov2659_drv_lock);
        OV2659_sensor.NightMode = KAL_FALSE;
        spin_unlock(&ov2659_drv_lock);
    }else if(u2FrameRate == 15){
        spin_lock(&ov2659_drv_lock);
        OV2659_sensor.NightMode = KAL_TRUE;
        spin_unlock(&ov2659_drv_lock);
    }else{
        // fixed other frame rate
    }

    spin_lock(&ov2659_drv_lock);
    OV2659_sensor.FixedFps = u2FrameRate;
    spin_unlock(&ov2659_drv_lock);

    if((u2FrameRate == 30)&&(OV2659AutoFlickerMode == KAL_TRUE))
        u2FrameRate = 296;
    else if ((u2FrameRate == 15)&&(OV2659AutoFlickerMode == KAL_TRUE))
        u2FrameRate = 146;
    else
        u2FrameRate = 10 * u2FrameRate;
    
    OV2659SetMaxFrameRate(u2FrameRate);
    OV2659_Write_Shutter(OV2659_sensor.shutter);//From Meimei Video issue

    return ERROR_NONE;
}

UINT32 OV2659SetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
    SENSORDB("bEnable = %d, u2FrameRate = %d ", bEnable, u2FrameRate);

    if(bEnable){
        spin_lock(&ov2659_drv_lock);
        OV2659AutoFlickerMode = KAL_TRUE;
        spin_unlock(&ov2659_drv_lock);

        /*Change frame rate 29.5fps to 29.8fps to do Auto flick*/
        if((OV2659_sensor.FixedFps == 30)&&(OV2659_sensor.video_mode==KAL_TRUE))
            OV2659SetMaxFrameRate(296);
        else if((OV2659_sensor.FixedFps == 15)&&(OV2659_sensor.video_mode==KAL_TRUE))
            OV2659SetMaxFrameRate(148);
        
    }else{ //Cancel Auto flick
        spin_lock(&ov2659_drv_lock);
        OV2659AutoFlickerMode = KAL_FALSE;
        spin_unlock(&ov2659_drv_lock);
        
        if((OV2659_sensor.FixedFps == 30)&&(OV2659_sensor.video_mode==KAL_TRUE))
            OV2659SetMaxFrameRate(300);
        else if((OV2659_sensor.FixedFps == 15)&&(OV2659_sensor.video_mode==KAL_TRUE))
            OV2659SetMaxFrameRate(150);            
    }

    return ERROR_NONE;
}


UINT32 OV2659SetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) 
{
    kal_uint32 pclk;
	kal_int16 dummyLine;
	kal_uint16 lineLength,frameHeight;
  
	SENSORDB("scenarioId = %d, frame rate = %d\n", scenarioId, frameRate);

    switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pclk = OV2659_PREVIEW_CLK;
			lineLength = OV2659_PV_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - OV2659_PV_PERIOD_LINE_NUMS;
			OV2659_Set_Dummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pclk = OV2659_VIDEO_CLK;
			lineLength = OV2659_VIDEO_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - OV2659_VIDEO_PERIOD_LINE_NUMS;
			OV2659_Set_Dummy(0, dummyLine);			
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:			
			pclk = OV2659_CAPTURE_CLK;
			lineLength = OV2659_FULL_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - OV2659_FULL_PERIOD_LINE_NUMS;
			OV2659_Set_Dummy(0, dummyLine);			
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


UINT32 OV2659GetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{
	SENSORDB("scenarioId = %d \n", scenarioId);

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 *pframeRate = 150;
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


UINT32 OV2659FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
                             UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 OV2659SensorRegNumber;
    UINT32 i;
    PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ENG_INFO_STRUCT *pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

    #ifdef OV2659_DRIVER_TRACE
        //SENSORDB("FeatureId = %d", FeatureId);
    #endif

    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++=OV2659_IMAGE_SENSOR_FULL_WIDTH;
            *pFeatureReturnPara16=OV2659_IMAGE_SENSOR_FULL_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
            switch(mCurrentScenarioId)
            {
                case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
                case MSDK_SCENARIO_ID_CAMERA_ZSD:
                    *pFeatureReturnPara16++ = OV2659_FULL_PERIOD_PIXEL_NUMS;
                    *pFeatureReturnPara16 = OV2659_sensor.frame_length;
                    *pFeatureParaLen=4;
                    break;
                default:
                    *pFeatureReturnPara16++ = OV2659_PV_PERIOD_PIXEL_NUMS;
                    *pFeatureReturnPara16 = OV2659_sensor.frame_length;
                    *pFeatureParaLen=4;
                    break;
            }
            break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            switch(mCurrentScenarioId)
            {
                case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
                case MSDK_SCENARIO_ID_CAMERA_ZSD: 
                    *pFeatureReturnPara32 = OV2659_CAPTURE_CLK;
                    *pFeatureParaLen=4;
                    break;
                default:
                    *pFeatureReturnPara32 = OV2659_PREVIEW_CLK;
                    *pFeatureParaLen=4;
                    break;
            }
            break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            set_OV2659_shutter(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            OV2659_night_mode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:       
            OV2659_SetGain((UINT16) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            OV2659_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = OV2659_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            memcpy(&OV2659_sensor.eng.cct, pFeaturePara, sizeof(OV2659_sensor.eng.cct));
            break;
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
            if (*pFeatureParaLen >= sizeof(OV2659_sensor.eng.cct) + sizeof(kal_uint32))
            {
              *((kal_uint32 *)pFeaturePara++) = sizeof(OV2659_sensor.eng.cct);
              memcpy(pFeaturePara, &OV2659_sensor.eng.cct, sizeof(OV2659_sensor.eng.cct));
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            memcpy(&OV2659_sensor.eng.reg, pFeaturePara, sizeof(OV2659_sensor.eng.reg));
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
            if (*pFeatureParaLen >= sizeof(OV2659_sensor.eng.reg) + sizeof(kal_uint32))
            {
              *((kal_uint32 *)pFeaturePara++) = sizeof(OV2659_sensor.eng.reg);
              memcpy(pFeaturePara, &OV2659_sensor.eng.reg, sizeof(OV2659_sensor.eng.reg));
            }
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            ((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->Version = NVRAM_CAMERA_SENSOR_FILE_VERSION;
            ((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorId = OV2659_SENSOR_ID;
            memcpy(((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorEngReg, &OV2659_sensor.eng.reg, sizeof(OV2659_sensor.eng.reg));
            memcpy(((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorCCTReg, &OV2659_sensor.eng.cct, sizeof(OV2659_sensor.eng.cct));
            *pFeatureParaLen = sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pFeaturePara, &OV2659_sensor.cfg_data, sizeof(OV2659_sensor.cfg_data));
            *pFeatureParaLen = sizeof(OV2659_sensor.cfg_data);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
             OV2659_camera_para_to_sensor();
            break;
        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            OV2659_sensor_to_camera_para();
            break;                          
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            OV2659_get_sensor_group_count((kal_uint32 *)pFeaturePara);
            *pFeatureParaLen = 4;
            break;    
        case SENSOR_FEATURE_GET_GROUP_INFO:
            OV2659_get_sensor_group_info((MSDK_SENSOR_GROUP_INFO_STRUCT *)pFeaturePara);
            *pFeatureParaLen = sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            OV2659_get_sensor_item_info((MSDK_SENSOR_ITEM_INFO_STRUCT *)pFeaturePara);
            *pFeatureParaLen = sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_SET_ITEM_INFO:
            OV2659_set_sensor_item_info((MSDK_SENSOR_ITEM_INFO_STRUCT *)pFeaturePara);
            *pFeatureParaLen = sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ENG_INFO:
            memcpy(pFeaturePara, &OV2659_sensor.eng_info, sizeof(OV2659_sensor.eng_info));
            *pFeatureParaLen = sizeof(OV2659_sensor.eng_info);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            OV2659SetVideoMode(*pFeatureData16);
            break; 
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            OV2659GetSensorID(pFeatureReturnPara32); 
            break; 
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            OV2659SetAutoFlickerMode((BOOL)*pFeatureData16,*(pFeatureData16+1));
            break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			OV2659SetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			OV2659GetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
			break;
        default:
            break;
    }
  
    return ERROR_NONE;
}   /*  OV2659FeatureControl()  */
SENSOR_FUNCTION_STRUCT  SensorFuncOV2659=
{
    OV2659Open,
    OV2659GetInfo,
    OV2659GetResolution,
    OV2659FeatureControl,
    OV2659Control,
    OV2659Close
};

UINT32 OV2659SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncOV2659;

    return ERROR_NONE;
}   /*  OV2659SensorInit  */
