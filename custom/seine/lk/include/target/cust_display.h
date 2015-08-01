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

/*

+// new animation parameters example:WVGA (480*800)
// A , start point of first number rectangle
// B , left_top point of battery_capacity fill_in rectangle
// c , left_bottom point of battery_capacity fill_in rectangle

// battery capacity rectangle
#define CAPACITY_LEFT                (172) // CAPACITY_LEFT = B.x = 172
#define CAPACITY_TOP                 (330) // CAPACITY_TOP = B.y = 330
#define CAPACITY_RIGHT               (307) // CAPACITY_RIGHT = B.x + fill_line.w = 172 + 135
#define CAPACITY_BOTTOM              (546) // CAPACITY_BOTTOM  = C.y = 546

// first number rectangle
#define NUMBER_LEFT                  (178) // NUMBER_LEFT = A.x
#define NUMBER_TOP                   (190) // NUMBER_TOP  = A.y
#define NUMBER_RIGHT                 (216) // NUMBER_RIGHT = A.x + num.w = 178 + 38
#define NUMBER_BOTTOM                (244) // NUMBER_BOTTOM = A.y + num.h = 190 + 54

// %  rectangle
#define PERCENT_LEFT                 (254) // PERCENT_LEFT = A.x + 2*num.w = 178 + 2*38
#define PERCENT_TOP                  (190) // PERCENT_TOP  = A.y
#define PERCENT_RIGHT                (302) // PERCENT_LEFT = A.x + 2*num.w +(%).w 
#define PERCENT_BOTTOM               (244) // PERCENT_BOTTOM = A.y + (%).h = 190 + 54

// top animation part
#define TOP_ANIMATION_LEFT           (172) // TOP_ANIMATION_LEFT = B.x
#define TOP_ANIMATION_TOP            (100) // 100 
#define TOP_ANIMATION_RIGHT          (307) // TOP_ANIMATION_LEFT = B.x + fill_line.w = 172 + 135
#define TOP_ANIMATION_BOTTOM         (124) // TOP_ANIMATION_BOTTOM = TOP_ANIMATION_TOP + fill_line.h = 100 + 24

*/


#ifndef __CUST_DISPLAY_H__
#define __CUST_DISPLAY_H__

// color
#define BAR_OCCUPIED_COLOR  (0x07E0)    // Green
#define BAR_EMPTY_COLOR     (0xFFFF)    // White
#define BAR_BG_COLOR        (0x0000)    // Black

// LOGO number
#define ANIM_V0_LOGO_NUM   5            // version 0: show 4 recatangle growing animation without battery number
#define ANIM_V1_LOGO_NUM   39           // version 1: show wave animation with  battery number 
#define ANIM_V2_LOGO_NUM   68           // version 2: show wireless charging animation      

// Common LOGO index
#define BOOT_LOGO_INDEX   0 
#define KERNEL_LOGO_INDEX   38 

#define ANIM_V0_BACKGROUND_INDEX   1 
#define ANIM_V1_BACKGROUND_INDEX   35
 
 
#define LOW_BATTERY_INDEX   2 
#define CHARGER_OV_INDEX   3 
#define FULL_BATTERY_INDEX   37 

// version 1: show wave animation with  battery number 

// NUMBER LOGO INDEX
#define NUMBER_PIC_START_0   4 
#define NUMBER_PIC_PERCENT   14 

// DYNAMIC ANIMATION LOGO INDEX
#define BAT_ANIM_START_0   15 

// LOW BATTERY(0~10%) ANIMATION LOGO
#define LOW_BAT_ANIM_START_0    25 

#define ANIM_LINE_INDEX   36 


// version 2: show wireless charging animation logo index

#define V2_NUM_START_0_INDEX  39  
#define V2_NUM_PERCENT_INDEX  49 
 
#define V2_BAT_0_10_START_INDEX     50  
#define V2_BAT_10_40_START_INDEX    54 
#define V2_BAT_40_80_START_INDEX    58 
#define V2_BAT_80_100_START_NDEX   62

#define V2_BAT_0_INDEX   66
#define V2_BAT_100_INDEX   67

// hd720 720*1280
/* lenovo_sw liaohj add for lenovo poweroff charging ui 2013-11-01 ---begin*/
//lenovo charging ui
#ifdef LENOVO_POWEROFF_CHARGING_UI

//Time
#define LENOVO_TIME_NUMBER_LEFT                  (170) 
#define LENOVO_TIME_NUMBER_RIGHT                 (254)
#define LENOVO_TIME_NUMBER_TOP                   (126)
#define LENOVO_TIME_NUMBER_BOTTOM                (266)
// : colon
#define LENOVO_TIME_COLON_LEFT                 (338) 
#define LENOVO_TIME_COLON_RIGHT                (380)
#define LENOVO_TIME_COLON_TOP                  (126)
#define LENOVO_TIME_COLON_BOTTOM               (266)

//Date
#define LENOVO_DATE_NUMBER_LEFT                  (260) 
#define LENOVO_DATE_NUMBER_RIGHT                 (280)
#define LENOVO_DATE_NUMBER_TOP                   (294)
#define LENOVO_DATE_NUMBER_BOTTOM                (330)
// : horiz
#define LENOVO_DATE_HORIZ_LEFT                 (340) 
#define LENOVO_DATE_HORIZ_RIGHT                (360)
#define LENOVO_DATE_HORIZ_TOP                  (294)
#define LENOVO_DATE_HORIZ_BOTTOM               (330)

// Battery
#define LENOVO_CAPACITY_LEFT                (291) // battery capacity center
#define LENOVO_CAPACITY_RIGHT               (429)
#define LENOVO_CAPACITY_TOP                 (721)
#define LENOVO_CAPACITY_BOTTOM              (901)
// top animation part
#define LENOVO_TOP_ANIMATION_LEFT           (291) // top animation
#define LENOVO_TOP_ANIMATION_RIGHT          (429)
#define LENOVO_TOP_ANIMATION_TOP            (100)
#define LENOVO_TOP_ANIMATION_BOTTOM         (120)

// above 90
#define LENOVO_CAPACITY_ABOVE_90_LEFT           (291) // top animation
#define LENOVO_CAPACITY_ABOVE_90_RIGHT          (429)
#define LENOVO_CAPACITY_ABOVE_90_TOP            (741)
#define LENOVO_CAPACITY_ABOVE_90_BOTTOM         (743)


//must modify debug liao
//Capacity num
#define LENOVO_CAPACITY_NUMBER_LEFT                  (340) 
#define LENOVO_CAPACITY_NUMBER_RIGHT                 (360)
#define LENOVO_CAPACITY_NUMBER_TOP                   (949)
#define LENOVO_CAPACITY_NUMBER_BOTTOM                (985)
// : logo
#define LENOVO_CAPACITY_LOGO_LEFT                 (300) 
#define LENOVO_CAPACITY_LOGO_RIGHT                (340)
#define LENOVO_CAPACITY_LOGO_TOP                  (949)
#define LENOVO_CAPACITY_LOGO_BOTTOM               (985)

// %  rectangle
#define LENOVO_CAPACITY_PERCENT_LEFT                 (380) // percent number_left + 2*number_width
#define LENOVO_CAPACITY_PERCENT_RIGHT                  (400)
#define LENOVO_CAPACITY_PERCENT_TOP                (949)
#define LENOVO_CAPACITY_PERCENT_BOTTOM               (985)

//Charing animation
#define LENOVO_CHARGING_ANI_LEFT                 (330) // percent number_left + 2*number_width
#define LENOVO_CHARGING_ANI_RIGHT                (390)
#define LENOVO_CHARGING_ANI_TOP                  (1000)
#define LENOVO_CHARGING_ANI_BOTTOM               (1185)

#endif
/* lenovo_sw liaohj add for lenovo poweroff charging ui 2013-11-01 ---end*/

// old MTK
// battery capacity rectangle
#define CAPACITY_LEFT                (291) // battery capacity center
#define CAPACITY_TOP                 (429)
#define CAPACITY_RIGHT               (721)
#define CAPACITY_BOTTOM              (901)

// first number rectangle
#define NUMBER_LEFT                  (290) // number
#define NUMBER_TOP                   (386)
#define NUMBER_RIGHT                 (335)
#define NUMBER_BOTTOM                (450)

// %  rectangle
#define PERCENT_LEFT                 (380) // percent number_left + 2*number_width
#define PERCENT_TOP                  (386)
#define PERCENT_RIGHT                (437)
#define PERCENT_BOTTOM               (450)

// top animation part
#define TOP_ANIMATION_LEFT           (278) // top animation
#define TOP_ANIMATION_TOP            (100)
#define TOP_ANIMATION_RIGHT          (441)
#define TOP_ANIMATION_BOTTOM         (129)

// for old animation
#define BAR_LEFT            (313)
#define BAR_TOP             (238)
#define BAR_RIGHT           (406)
#define BAR_BOTTOM          (453)


/* The option of new charging animation */
#define ANIMATION_NEW

#endif // __CUST_DISPLAY_H__
