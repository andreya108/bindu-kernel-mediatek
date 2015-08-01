/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
   BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
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

#ifndef BUILD_LK
#include <linux/string.h>

#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif
//begin jixu add for debug lcm
#ifdef BUILD_LK
#define ENHANCMENT_DEBUG_LCM 0
#else
#define ENHANCMENT_DEBUG_LCM 1
#endif

#if ENHANCMENT_DEBUG_LCM
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/ctype.h>

#endif
//end jixu add for debug lcm


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif



// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util_lenovo = {0};

#define SET_RESET_PIN(v)    	(lcm_util_lenovo.set_reset_pin((v)))

#define UDELAY(n) 		(lcm_util_lenovo.udelay(n))
#define MDELAY(n) 		(lcm_util_lenovo.mdelay(n))

extern LCM_PARAMS *lcm_params;
static struct proc_dir_entry *lenovo_lcm_debug_proc_entry;
#define LENOVO_LCM_DEBUG "lenovo_lcm_debug"
extern void DSI_PHY_TIMCONFIG(LCM_PARAMS *lcm_params);


struct lenovo_lcm_debug_init_info_t
{
    unsigned int  pdata[16];
    unsigned int  queue_size;
	unsigned int  delay_ms;
};


struct lenovo_lcm_debug_lcm_timming_t
{
	unsigned int HS_TRAIL;
	unsigned int HS_ZERO;
	unsigned int HS_PRPR;
	unsigned int LPX;
	
	unsigned int TA_SACK;
	unsigned int TA_GET;
	unsigned int TA_SURE;
	unsigned int TA_GO;
	
	unsigned int CLK_TRAIL;
	unsigned int CLK_ZERO;
	unsigned int LPX_WAIT;
	unsigned int CONT_DET;
	
	unsigned int CLK_HS_PRPR;
	unsigned int pll_div1; 	// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	unsigned int pll_div2; 		// div2=0~15: fout=fvo/(2*div2)

};

typedef enum
{
	LCM_DEBUG_SEGMENT_TYPE_REG = 0,
	LCM_DEBUG_SEGMENT_TYPE_TIMMING
}LCM_DEBUG_SEGMENT_TYPE_E;
#define LCM_INIT_DATA_ARRAY_MAX 70
static struct lenovo_lcm_debug_init_info_t lenovo_lcm_debug_reg_info[LCM_INIT_DATA_ARRAY_MAX];
static int lenovo_lcm_debug_reg_info_num ;
static struct lenovo_lcm_debug_lcm_timming_t lenovo_lcm_debug_timming;


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util_lenovo.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util_lenovo.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)						lcm_util_lenovo.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)			lcm_util_lenovo.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) 						lcm_util_lenovo.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   		lcm_util_lenovo.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
       
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------









 
//lenovo jixu add begin
// ---------------------------------------------------------------------------
//  Debug FileSystem Routines
// ---------------------------------------------------------------------------
#define LENOVO_DBGFS 0
#if LENOVO_DBGFS
struct dentry *mtkfb_dbgfs = NULL;


static ssize_t debug_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}


static char debug_buffer[2048];

static ssize_t debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(debug_buffer) - 1;
    int n = 0;

    n += scnprintf(debug_buffer + n, debug_bufmax - n, STR_HELP);
    debug_buffer[n++] = 0;

    return simple_read_from_buffer(ubuf, count, ppos, debug_buffer, n);
}


static ssize_t debug_write(struct file *file,
                           const char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(debug_buffer) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax) 
        count = debug_bufmax;

	if (copy_from_user(&debug_buffer, ubuf, count))
		return -EFAULT;

	debug_buffer[count] = 0;

    process_dbg_cmd(debug_buffer);

    return ret;
}


static struct file_operations debug_fops = {
	.read  = debug_read,
    .write = debug_write,
	.open  = debug_open,
};
#endif

/*** Cread by jixu@lenovo.com***/
/*******************************************************	
Function:
	Creat proc entry function.

Input:
	NULL
	
Output:
	Error Report.
*********************************************************/
static int lenovo_lcm_debug_creat_proc_file(void)
{
        int ret = 0;
        lenovo_lcm_debug_proc_entry = create_proc_entry(LENOVO_LCM_DEBUG, 0666, NULL);
        if (lenovo_lcm_debug_proc_entry == NULL){
            ret = -ENOMEM;
            printk("[JX] %s, can not creat %s proc entry\n",__func__,LENOVO_LCM_DEBUG);
        }else{
            //lenovo_lcm_debug_proc_entry->read_proc = cabc_read;
			//lenovo_lcm_debug_proc_entry->write_proc = cabc_write_debug;
            printk("[JX] %s, creat %s proc entry successful\n",__func__,LENOVO_LCM_DEBUG);
        }
        return ret;
}

/*******************************************************	
Function:
	DeCreat proc entry function.

Input:
	NULL
	
Output:
	NULL
*********************************************************/
static void lenovo_lcm_debug_decreat_proc_file(void)
{
        remove_proc_entry(LENOVO_LCM_DEBUG, lenovo_lcm_debug_proc_entry);
		printk("[JX] %s, remove %s proc entry\n",__func__,LENOVO_LCM_DEBUG);
}

/*******************************************************	
Function:
	Open file function.

Input:
	path:the file path.
	old_fs_p:old fs point.
	
Output:
	File point.
*********************************************************/
static struct file * lenovo_lcm_debug_file_open(u8 * path, mm_segment_t * old_fs_p)
{
  s32 errno = -1;
  struct file *filp = NULL;
  *old_fs_p = get_fs();
  set_fs(KERNEL_DS);

  filp = filp_open(path, O_RDONLY, 0644);
  if(!filp || IS_ERR(filp))
  {
    if(!filp)
      errno = -ENOENT;
    else 
      errno = PTR_ERR(filp);
    printk("[JX] %s Open file error. errno is:%d\n",__func__,errno);
    return NULL;
  }

  filp->f_op->llseek(filp,0,0);
  return filp ;
}

/*******************************************************	
Function:
	Close file function.

Input:
	filp:the file point.
	old_fs_p:old fs point.
	
Output:
	None.
*********************************************************/
static void lenovo_lcm_debug_file_close(struct file * filp, mm_segment_t old_fs)
{
  set_fs(old_fs);
  if(filp)
    filp_close(filp, NULL);
}

/*******************************************************	
Function:
	Get file length function.

Input:
	path:the file path.
	
Output:
	File length.
*********************************************************/
static int lenovo_lcm_debug_get_file_length(char * path)
{
  struct file * file_ck = NULL;
  mm_segment_t old_fs;
  s32 length ;
	
  file_ck = lenovo_lcm_debug_file_open(path, &old_fs);
  if(file_ck == NULL)
    return 0;

  length = file_ck->f_op->llseek(file_ck, 0, SEEK_END);
  if(length < 0)
    length = 0;
  lenovo_lcm_debug_file_close(file_ck, old_fs);
  return length;	
}


/*******************************************************	
Function:
	lcm hard reset. Maybe need to modify this function.

Input:
	NULL
	
Output:
	NULL
*********************************************************/
static void lenovo_lcm_debug_reset(void)
{
	SET_RESET_PIN(0);
    MDELAY(5);
    SET_RESET_PIN(1);
    MDELAY(120);
}

static unsigned int lenovo_lcm_debug_read_reg(void)
{
#if 1//for read id
	unsigned int id = 0;
	unsigned char buffer[2];
	unsigned int array[16];
	
	printk("[jx] %s, hx8369_dsi\n", __func__);

	array[0]=0x00043902;
	array[1]=0x6983FFB9;
	dsi_set_cmdq(&array, 2, 1);
	MDELAY(2);

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; //we only need ID
	printk("[jx] %s, id1 = 0x%08x\n", __func__, id);
#else
	unsigned int ret = 0;
	ret = mt_get_gpio_in(GPIO99);
	#if defined(BUILD_UBOOT)
		printf("%s, GPIO99 = %d \n", __func__, ret);
	#endif	

	return (ret == 0)?1:0;

#endif
}

/*******************************************************	
Function:
	DeCreat proc entry function.

Input:
	NULL
	
Output:
	NULL
*********************************************************/
static u32 lenovo_lcm_debug_size_of_str(char * s)
{
	u32 temp=0;
	for (; *s != '\0'; ++s)
		temp++;
	return temp;

}


static int lenovo_lcm_debug_count_buffer_len(u8 *start, u8 *end)
{
	u8 *temp_s,*temp_e;
	int ret=0;
	temp_s=start;
	temp_e=end;
	if(temp_s>temp_e) return -1;	
	for(ret=1;temp_s<temp_e;ret++,temp_s++) {
	   // printk("[JX] %s temp_s:0x%x \n",__func__,temp_s);
	}
	return ret;
	

}

static u8 * lenovo_lcm_debug_get_param(char * str,int char_end, u8 *pstart,unsigned int *data)
{
	u8 * pend=NULL;
	u8 * cp=NULL;
	u8 * param_ptr;
	int buffer_len;
	int str_len;
	cp = pstart;
	if(cp==NULL) return NULL;	
	//printk("[JX] %s str:%s pstart:%s\n",__func__,str,pstart);
	str_len = lenovo_lcm_debug_size_of_str(str);
	
	if (strncmp(cp, str, str_len) == 0) {
		cp+=str_len;
		if((pend = strchr(cp, char_end))!=NULL) {
			buffer_len = lenovo_lcm_debug_count_buffer_len(cp,pend);
			param_ptr = (u8*)vmalloc(buffer_len);
			if(!param_ptr) {
				vfree(param_ptr);
				printk("[JX] ERROR can not malloc buffer.\n");
				return NULL;
			}
			memset(param_ptr,0,buffer_len);
			memcpy(param_ptr,cp,buffer_len);
			*data = simple_strtoul(param_ptr,NULL,16);
			memset(param_ptr,0,buffer_len);
			vfree(param_ptr);
		}else{
			printk("[JX] ERROR can not found';'\n");
			return NULL;
		}
	}else {
		printk("[JX] ERROR not found %s\n",str);
		return NULL;
	}
	return pend+1;
}

static s32 lenovo_lcm_debug_parse_segment(u8 * data,LCM_DEBUG_SEGMENT_TYPE_E type)
{
	u8 * cp;
	int i;
	if(data==NULL) {
		printk("[JX] %s ERROR NULL of data",__func__);
		return -1;
	}
	
	cp = data;
	//printk("[JX] %s type:%d data:%s \n",__func__,type,cp);
	switch(type) {
		case LCM_DEBUG_SEGMENT_TYPE_REG:
			do {
				if (strncmp(cp, "size=", 5) == 0) {
					if(!(cp=lenovo_lcm_debug_get_param("size=",',',cp,&lenovo_lcm_debug_reg_info[lenovo_lcm_debug_reg_info_num].queue_size)))
					break;
				}
				if (strncmp(cp, "data=", 5) == 0) {
					if(lenovo_lcm_debug_reg_info[lenovo_lcm_debug_reg_info_num].queue_size!=0) {
						for(i=0;i<lenovo_lcm_debug_reg_info[lenovo_lcm_debug_reg_info_num].queue_size;i++) {
							if(!(cp=lenovo_lcm_debug_get_param("data=",',',cp,&lenovo_lcm_debug_reg_info[lenovo_lcm_debug_reg_info_num].pdata[i]))) {
								printk("[JX] %s ERROR reg%d data out of size\n",__func__,lenovo_lcm_debug_reg_info_num);
								return -1;
							}
						}
					}
				}
				if (strncmp(cp, "delay=", 6) == 0) {
					if(!(cp=lenovo_lcm_debug_get_param("delay=",';',cp,&lenovo_lcm_debug_reg_info[lenovo_lcm_debug_reg_info_num].delay_ms)))
					break;
				}
				
				cp++;
			}while(*cp != '\0');
			break;
			
		case LCM_DEBUG_SEGMENT_TYPE_TIMMING:
			do {
				if(!(cp=lenovo_lcm_debug_get_param("hs_trail=",',',cp,&lenovo_lcm_debug_timming.HS_TRAIL)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("hs_zero=",',',cp,&lenovo_lcm_debug_timming.HS_ZERO)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("hs_prpr=",',',cp,&lenovo_lcm_debug_timming.HS_PRPR)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("lpx=",',',cp,&lenovo_lcm_debug_timming.LPX)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("ta_sack=",',',cp,&lenovo_lcm_debug_timming.TA_SACK)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("ta_get=",',',cp,&lenovo_lcm_debug_timming.TA_GET)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("ta_sure=",',',cp,&lenovo_lcm_debug_timming.TA_SURE)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("ta_go=",',',cp,&lenovo_lcm_debug_timming.TA_GO)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("clk_trail=",',',cp,&lenovo_lcm_debug_timming.CLK_TRAIL)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("clk_zero=",',',cp,&lenovo_lcm_debug_timming.CLK_ZERO)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("lpx_wait=",',',cp,&lenovo_lcm_debug_timming.LPX_WAIT)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("cont_det=",',',cp,&lenovo_lcm_debug_timming.CONT_DET)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("clk_hs_prpr=",',',cp,&lenovo_lcm_debug_timming.CLK_HS_PRPR)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("pll_div1=",',',cp,&lenovo_lcm_debug_timming.pll_div1)))
					break;
				if(!(cp = lenovo_lcm_debug_get_param("pll_div2=",';',cp,&lenovo_lcm_debug_timming.pll_div2)))
					break;
				cp++;
			}while(*cp != '\0');
			
			break;
			
		default:
			break;

	}
}

static u8 * lenovo_lcm_debug_get_segment(char * str, int char_end, u8 *pstart,LCM_DEBUG_SEGMENT_TYPE_E type)
{
	u8 * pend=NULL;
	u8 * cp=NULL;
	u8 * param_ptr;
	int buffer_len;
	int str_len;
	cp = pstart;
	if(cp==NULL) return NULL;	
	str_len = lenovo_lcm_debug_size_of_str(str);
	cp+=str_len;
	if((pend = strchr(cp, char_end))!=NULL) {
		buffer_len = lenovo_lcm_debug_count_buffer_len(cp,pend);
		param_ptr = (u8*)vmalloc(buffer_len);
		if(!param_ptr) {
			vfree(param_ptr);
			printk("[JX] ERROR can not malloc buffer.\n");
			return NULL;
		}
		memset(param_ptr,0,buffer_len);
		memcpy(param_ptr,cp,buffer_len);
		lenovo_lcm_debug_parse_segment(param_ptr,type);
		memset(param_ptr,0,buffer_len);
		vfree(param_ptr);
	}else{
		printk("[JX] ERROR can not found';'\n");
		return NULL;
	}
	return pend+1;
}

static s32 lenovo_lcm_debug_parse_cmd_2(u8 *data,u32 file_len )
{
	u8 *cp;
	int i,j;

	cp =  data;
	lenovo_lcm_debug_reg_info_num = 0;
	for(;cp<(data+file_len);cp++) {
		//printk("[JX] cp:0x%x str:%s\n",cp,cp);
		if(strncmp("reg:",cp,4)==0) {
			if(!(cp = lenovo_lcm_debug_get_segment("reg:",';',cp,LCM_DEBUG_SEGMENT_TYPE_REG))) {
				printk("[JX] ERROR NULL of cp");
				break;
			}
			lenovo_lcm_debug_reg_info_num++;
			if (lenovo_lcm_debug_reg_info_num>LCM_INIT_DATA_ARRAY_MAX) {
				printk("[JX] ERROR out of DATA_ARRAY_MAX %d\n",LCM_INIT_DATA_ARRAY_MAX);
				return -3;
				}
				
		}
		if(strncmp("timming:",cp,8)==0) {
			if(!(cp = lenovo_lcm_debug_get_segment("timming:",';',cp,LCM_DEBUG_SEGMENT_TYPE_TIMMING))) {
				printk("[JX] ERROR NULL of cp");
				break;
			}
		}
	}
#if 0
	
	//log out lenovo_lcm_debug_reg_info 
	for(i=0;i<lenovo_lcm_debug_reg_info_num;i++) 
	{
		printk("[JX] reg_%d=0x%x\n",i,lenovo_lcm_debug_reg_info[i].queue_size);
		for(j=0;j<lenovo_lcm_debug_reg_info[i].queue_size;j++)
		{
			printk("data:0x%x\n",lenovo_lcm_debug_reg_info[i].pdata[j]);
		}
		printk("delay:0x%x\n",lenovo_lcm_debug_reg_info[i].delay_ms);
	}

	//log out lenovo_lcm_debug_timming
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.HS_TRAIL);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.HS_ZERO);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.HS_PRPR);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.LPX);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.TA_SACK);	
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.TA_GET);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.TA_SURE);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.TA_GO);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.CLK_TRAIL);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.CLK_ZERO);	
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.LPX_WAIT);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.CONT_DET);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.CLK_HS_PRPR);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.pll_div1);
	printk("[JX] timming:0x%x\n",lenovo_lcm_debug_timming.pll_div2);

#else	
	// then write to lcm

	if(lenovo_lcm_debug_timming.HS_TRAIL!=0) {
		lcm_params->dsi.HS_TRAIL = lenovo_lcm_debug_timming.HS_TRAIL;
		lcm_params->dsi.HS_ZERO = lenovo_lcm_debug_timming.HS_ZERO;
		lcm_params->dsi.HS_PRPR = lenovo_lcm_debug_timming.HS_PRPR;
		lcm_params->dsi.LPX = lenovo_lcm_debug_timming.LPX;
		lcm_params->dsi.TA_SACK = lenovo_lcm_debug_timming.TA_SACK;		
		lcm_params->dsi.TA_GET = lenovo_lcm_debug_timming.TA_GET;
		lcm_params->dsi.TA_SURE = lenovo_lcm_debug_timming.TA_SURE;
		lcm_params->dsi.TA_GO = lenovo_lcm_debug_timming.TA_GO;
		lcm_params->dsi.CLK_TRAIL = lenovo_lcm_debug_timming.CLK_TRAIL;
		lcm_params->dsi.CLK_ZERO = lenovo_lcm_debug_timming.CLK_ZERO;		
		lcm_params->dsi.LPX_WAIT = lenovo_lcm_debug_timming.LPX_WAIT;
		lcm_params->dsi.CONT_DET = lenovo_lcm_debug_timming.CONT_DET;
		lcm_params->dsi.CLK_HS_PRPR = lenovo_lcm_debug_timming.CLK_HS_PRPR;
		lcm_params->dsi.pll_div1 = lenovo_lcm_debug_timming.pll_div1;
		lcm_params->dsi.pll_div2 = lenovo_lcm_debug_timming.pll_div2;
		
		DSI_PHY_TIMCONFIG(lcm_params);
	}
	//lenovo_lcm_debug_reset();
	//lenovo_lcm_debug_read_reg();
	for(i=0;i<lenovo_lcm_debug_reg_info_num;i++) {
		dsi_set_cmdq(&(lenovo_lcm_debug_reg_info[i].pdata), lenovo_lcm_debug_reg_info[i].queue_size, 1);
		MDELAY(lenovo_lcm_debug_reg_info[i].delay_ms);
	}
#endif
	printk("[JX] %s, send cmd done. \n",__func__);
	return 0;

}


/*******************************************************	
Function:
	main of the lem debug function.

Input:
	NULL	
Output:
	Error Report.
*********************************************************/
int lenovo_lcm_debug_main(const LCM_UTIL_FUNCS *util)
{
	struct file *file_data = NULL;
    mm_segment_t old_fs;
	static s8* fw_path = "/data/lcm_init";
    u32 file_len = 0;
    u8 *file_ptr = NULL;
    s32 ret = -1;

	printk("[JX] %s.\n",__func__);
memcpy(&lcm_util_lenovo, util, sizeof(LCM_UTIL_FUNCS));
	if(NULL == lcm_util_lenovo.set_reset_pin)
	{
		printk("[JX] Cannot get util funcs,Exit update.\n");
		goto Err;
	}		
	
	file_data = lenovo_lcm_debug_file_open(fw_path, &old_fs);
	if(file_data == NULL)
	{
		printk("[JX] Cannot open update file,Exit update.\n");
		goto Done;
	}

	file_len = lenovo_lcm_debug_get_file_length(fw_path);
	printk("[JX] Update file length:%d.\n", file_len);

	file_ptr = (u8*)vmalloc(file_len);
	if(file_ptr==NULL)
	{
		printk("[JX] Cannot malloc memory,Exit update.\n");
		goto FreeDone;
	}	

	ret = file_data->f_op->read(file_data, file_ptr, file_len, &file_data->f_pos);
	if(ret <= 0)
	{
		printk("[JX] Read file data failed,Exit update.\n");
		goto FreeDone;
	}
	lenovo_lcm_debug_file_close(file_data, old_fs);
//	ret = lenovo_lcm_debug_parse_cmd(file_ptr, file_len);
	ret = lenovo_lcm_debug_parse_cmd_2(file_ptr, file_len);
	if(ret < 0)
	{
		printk("[JX] Update failed!Exit.\n");
		goto Free;
	}
	vfree(file_ptr);
	return 0;
Err:
	return 0;
Done:
	lenovo_lcm_debug_file_close(file_data, old_fs);
	return 0;
FreeDone:
	vfree(file_ptr);
	lenovo_lcm_debug_file_close(file_data, old_fs);
	return 0;
Free:
	vfree(file_ptr);
	return 0;

}

#endif
