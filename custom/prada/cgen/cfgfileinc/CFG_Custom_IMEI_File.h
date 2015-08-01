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
* LENOVO SW jixj add imei backup 2012.8.9
*
*
 */

#ifndef _CFG_CUSTOMSN_FILE_H
#define _CFG_CUSTOMSN_FILE_H

typedef struct
{
	unsigned int data1[4];
	unsigned int data2[4];
    char user[8];
    long long time;
}File_Data_Struct;

typedef struct
{
    File_Data_Struct custom1;
    File_Data_Struct custom2;
    File_Data_Struct custom3;
    int index;
}File_Custom_Data_Struct;

typedef struct
{
	unsigned int data1[4];
	unsigned int data2[4];
}File_Custom_Data_Info;


#define CFG_FILE_CUSTOM_IMEI_REC_SIZE    sizeof(File_Custom_Data_Struct)
#define CFG_FILE_CUSTOM_IMEI_REC_TOTAL   1

#define CFG_FILE_CUSTOM_DATA_INFO_REC_SIZE    sizeof(File_Custom_Data_Info)
#define CFG_FILE_CUSTOM_DATA_INFO_REC_TOTAL   1
#endif
