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
0x7a36091,   0xf0b93ede,   0x7a36091,   0x7a14c5a8,   0x0,     \
0x79b63e3,   0xf0c9383a,   0x79b63e3,   0x798ec622,   0x0,     \
0x776a522,   0xf112b5bc,   0x776a522,   0x7720c84b,   0x0,     \
0x74aedf3,   0xf16a241a,   0x74aedf3,   0x742ecad0,   0x0,     \
0x73bb426,   0xf18897b4,   0x73bb426,   0x7324cbad,   0x0,     \
0x6f66ffb,   0xf2132009,   0x6f66ffb,   0x6e52cf84,   0x0,     \
0x6a5addc,   0xf2b4a447,   0x6a5addc,   0x6882d3cd,   0x0,     \
0x689f6d7,   0xf2ec1251,   0x689f6d7,   0x6678d539,   0x0,     \
0x60e7e0f,   0xf3e303e2,   0x60e7e0f,   0x5d12db42,   0x0,     \
\
	0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0 
   

    /* Compensation Filter BPF coeffs: default all pass filter      */ 
    #define BES_LOUDNESS_BPF_COEFF \
0x3dd3cdf9,   0x33483206,   0xcee40000,     \
0x3da7d897,   0x32492768,   0xd00e0000,     \
0x3ce60a76,   0x2dddf589,   0xd53c0000,     \
0x3c0c3d74,   0x28e6c28b,   0xdb0c0000,     \
0x3bc34a76,   0x273cb589,   0xdcff0000,     \
0x0,   0x0,   0x0,     \ 
\
0x3e8cd66c,   0x39942993,   0xc7df0000,     \
0x3e6de2ee,   0x390c1d11,   0xc8860000,     \
0x3de11d6b,   0x36a4e294,   0xcb790000,     \
0x3d405602,   0x33dba9fd,   0xcee40000,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \ 
\
0x3f30ab0e,   0x38ef54f1,   0xc7df0000,     \
0x3f1fb195,   0x38594e6a,   0xc8860000,     \
0x3ed1d368,   0x35b42c97,   0xcb790000,     \
0x3e770000,   0x32a30000,   0xcee40000,     \
0x3e590f80,   0x3197f07f,   0xd00e0000,     \
0x3dd04b7e,   0x2cf2b481,   0xd53c0000,     \ 
\    
0x40808975,   0x3b9f768a,   0xc3df0000,     \
0x408b8acb,   0x3b3f7534,   0xc4340000,     \
0x40bd91fc,   0x39876e03,   0xc5ba0000,     \
0x40f99cb7,   0x377e6348,   0xc7870000,     \
0x410ea0fc,   0x36c95f03,   0xc8280000,     \
0x416cb790,   0x3393486f,   0xcaff0000,     \
\    
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\    
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\    
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
\
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0
    
    #define BES_LOUDNESS_LPF_COEFF \
0x1bc91bc9,   0x86c,   0x0,     \
0x1da81da8,   0x4ae,   0x0,     \
	0x265d265d,   0xf345,   0x0,     \ 
	0x32793279,   0xdb0c,   0x0,     \ 
	0x37cb37cb,   0xd069,   0x0,     \ 
	0x0,   0x0,   0x0 

    #define BES_LOUDNESS_WS_GAIN_MAX  0
           
    #define BES_LOUDNESS_WS_GAIN_MIN  0
           
    #define BES_LOUDNESS_FILTER_FIRST  0
           
    #define BES_LOUDNESS_GAIN_MAP_IN \
    0, 0, 0, 0,  0
   
    #define BES_LOUDNESS_GAIN_MAP_OUT \            
    0, 0, 0, 0, 0

	#define BES_LOUDNESS_ATT_TIME	164
	#define BES_LOUDNESS_REL_TIME	16400              

#endif
