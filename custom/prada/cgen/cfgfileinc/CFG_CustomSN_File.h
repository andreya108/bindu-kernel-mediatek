/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* 
* LENOVO SW jixu add sn 20120213
*
*
 */

#ifndef _CFG_CUSTOMSN2_FILE_H
#define _CFG_CUSTOMSN2_FILE_H

typedef struct
{
	unsigned char barcode[64];
}File_CustomSN_Struct;

#define CFG_FILE_CUSTOMSN_REC_SIZE    sizeof(File_CustomSN_Struct)
#define CFG_FILE_CUSTOMSN_REC_TOTAL   1

#endif
