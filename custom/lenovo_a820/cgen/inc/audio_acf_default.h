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

/*******************************************************************************
 *
 * Filename:
 * ---------
 * audio_acf_default.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 * This file is the header of audio customization related parameters or definition.
 *
 * Author:
 * -------
 * Tina Tsai
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
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef AUDIO_ACF_DEFAULT_H
#define AUDIO_ACF_DEFAULT_H

    /* Compensation Filter HSF coeffs: default all pass filter       */
    /* BesLoudness also uses this coeffs    */ 
    #define BES_LOUDNESS_HSF_COEFF \
0x794ac34,   0xf0d6a797,   0x794ac34,   0x792fc69a,   0x0,     \
0x78b8fe9,   0xf0e8e02d,   0x78b8fe9,   0x7899c727,   0x0,     \
0x761eecd,   0xf13c2265,   0x761eecd,   0x75e3c9a6,   0x0,     \
0x7310072,   0xf19dff1c,   0x7310072,   0x72a9cc89,   0x0,     \
0x7201b95,   0xf1bfc8d5,   0x7201b95,   0x7189cd86,   0x0,     \
0x6d43117,   0xf2579dd2,   0x6d43117,   0x6c68d1e2,   0x0,     \
0x67d8fa1,   0xf304e0bd,   0x67d8fa1,   0x6668d6b6,   0x0,     \
0x660485d,   0xf33f6f46,   0x660485d,   0x6457d84e,   0x0,     \
0x5e08f2d,   0xf43ee1a6,   0x5e08f2d,   0x5b19df07,   0x0,     \
    \
0x7d12439,   0xf05db78d,   0x7d12439,   0x7cf6c2d1,   0x0,     \
0x7ccef76,   0xf0662113,   0x7ccef76,   0x7caec310,   0x0,     \
0x7b93dbe,   0xf08d8484,   0x7b93dbe,   0x7b56c42e,   0x0,     \
0x7a111c6,   0xf0bddc73,   0x7a111c6,   0x79a4c582,   0x0,     \
0x79875b7,   0xf0cf1491,   0x79875b7,   0x7907c5f8,   0x0,     \
0x770097f,   0xf11fed01,   0x770097f,   0x7612c811,   0x0,     \
0x73e4a10,   0xf1836be0,   0x73e4a10,   0x7248ca7f,   0x0,     \
0x72c907b,   0xf1a6df09,   0x72c907b,   0x70e5cb53,   0x0,     \
0x6d99123,   0xf24cddba,   0x6d99123,   0x6a2ccefa,   0x0

    /* Compensation Filter BPF coeffs: default all pass filter      */ 
    #define BES_LOUDNESS_BPF_COEFF \
0x3f8a86be,   0x3dfd7941,   0xc2770000,     \
0x3f8087bb,   0x3dd17844,   0xc2ad0000,     \
0x3f528d20,   0x3d0472df,   0xc3a90000,     \
0x3f1a9556,   0x3c0e6aa9,   0xc4d60000,     \
0x3f0698a6,   0x3bb96759,   0xc53f0000,     \
0x3eadaa87,   0x3a315578,   0xc7200000,     \
    \
0x3f8a8c05,   0x3dfd73fa,   0xc2770000,     \
0x3f808df4,   0x3dd1720b,   0xc2ad0000,     \
0x3f52989e,   0x3d046761,   0xc3a90000,     \
0x3f1aa8e9,   0x3c0e5716,   0xc4d60000,     \
0x3f06af6f,   0x3bb95090,   0xc53f0000,     \
0x3eadd1be,   0x3a312e41,   0xc7200000,     \
    \
0x3f8a84e0,   0x3dfd7b1f,   0xc2770000,     \
0x3f808587,   0x3dd17a78,   0xc2ad0000,     \
0x3f528904,   0x3d0476fb,   0xc3a90000,     \
0x3f1a8e36,   0x3c0e71c9,   0xc4d60000,     \
0x3f06904b,   0x3bb96fb4,   0xc53f0000,     \
0x3ead9b7f,   0x3a316480,   0xc7200000,     \
    \
0x3f8b82bc,   0x3dfd7d43,   0xc2770000,     \
0x3f818300,   0x3dd07cff,   0xc2ad0000,     \
0x3f528444,   0x3d037bbb,   0xc3a90000,     \
0x3f1b85e7,   0x3c0d7a18,   0xc4d60000,     \
0x3f088682,   0x3bb8797d,   0xc53f0000,     \
0x3eaf897a,   0x3a2f7685,   0xc7200000,     \
    \
0x3f8b836f,   0x3dfd7c90,   0xc2770000,     \
0x3f8183d3,   0x3dd07c2c,   0xc2ad0000,     \
0x3f5285d1,   0x3d037a2e,   0xc3a90000,     \
0x3f1b88a0,   0x3c0d775f,   0xc4d60000,     \
0x3f0889b8,   0x3bb87647,   0xc53f0000,     \
0x3eaf8f71,   0x3a2f708e,   0xc7200000,     \
    \
 	0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \         
\    
 	0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \    
\
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000, \ 
    0x40000000,0x00000000,0x00000000     
    
    #define BES_LOUDNESS_LPF_COEFF \
    0x265d265d,   0xf345,   0x0,     \
    0x294f294f,   0xed60,   0x0,     \
    0x3a433a43,   0xcb79,   0x0,     \
    0x0,   0x0,   0x0,     \
    0x0,   0x0,   0x0,     \
    0x0,   0x0,   0x0

    #define BES_LOUDNESS_WS_GAIN_MAX  0
           
    #define BES_LOUDNESS_WS_GAIN_MIN  0
           
    #define BES_LOUDNESS_FILTER_FIRST  0
           
    #define BES_LOUDNESS_GAIN_MAP_IN \
    0xd3, 0xda, 0xed, 0xee, 0x0
   
    #define BES_LOUDNESS_GAIN_MAP_OUT \            
    0xc, 0xc, 0xc, 0xc, 0x0

	#define BES_LOUDNESS_ATT_TIME	164
	#define BES_LOUDNESS_REL_TIME	16400              

#endif
