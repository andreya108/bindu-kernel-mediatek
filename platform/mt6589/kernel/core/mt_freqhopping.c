/*
 * Copyright (C) 2011 MediaTek, Inc.
 *
 * Author: Holmes Chiou <holmes.chiou@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <asm/sched_clock.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <board-custom.h>
#include <linux/seq_file.h>

#include "mach/mt_freqhopping.h"
#include "mach/mt_fhreg.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_gpio.h"
#include "mach/mt_gpufreq.h"
#include "mach/mt_cpufreq.h"
#include "mach/emi_bwl.h"
#include "mach/sync_write.h"
#include "mach/mt_sleep.h"

#include <mach/mt_freqhopping_drv.h>

//#define FH_MSG printk //TODO
//#define FH_BUG_ON(x) printk("BUGON %s:%d %s:%d",__FUNCTION__,__LINE__,current->comm,current->pid)//TODO
//#define FH_BUG_ON(...) //TODO

#define MT_FH_CLK_GEN 		0

#define USER_DEFINE_SETTING_ID 	1

static DEFINE_SPINLOCK(freqhopping_lock);


//current DRAMC@mempll 
static unsigned int 	g_curr_dramc=266; //default @266MHz ==> LPDDR2/DDR3 data rate 1066

#if MT_FH_CLK_GEN
static unsigned int 	g_curr_clkgen=MT658X_FH_PLL_TOTAL_NUM+1; //default clkgen ==> no clkgen output
#endif 

static unsigned char 	g_mempll_fh_table[8];

static unsigned int	g_initialize=0;

static unsigned int 	*g_fh_rank1_pa;
static unsigned int 	*g_fh_rank1_va;
static unsigned int	*g_fh_rank0_pa;
static unsigned int	*g_fh_rank0_va;
//static unsigned int	g_clk_en=0;

#ifndef PER_PROJECT_FH_SETTING	

#define MT_FH_DUMMY_READ	0

#define LOW_DRAMC_DDS		0x0010C000
#define LOW_DRAMC_INT		67 //233.5
#define LOW_DRAMC_FRACTION	0 //233.5
#define LOW_DRAMC		233 //233.5
#define LOW_DRAMC_FREQ		233500

#define LVDS_PLL_IS_ON		0

//TODO: fill in the default freq & corresponding setting_id
static  fh_pll_t g_fh_pll[MT658X_FH_PLL_TOTAL_NUM] = { //keep track the status of each PLL 
	{FH_FH_DISABLE,     FH_PLL_ENABLE   , 0, 0       , 0}, //ARMPLL   default SSC disable
	{FH_FH_DISABLE,     FH_PLL_ENABLE   , 0, 1612000 , 0}, //MAINPLL  default SSC disable
	{FH_FH_ENABLE_SSC,  FH_PLL_ENABLE   , 0, 266000  , 0}, //MEMPLL   default SSC enable
	{FH_FH_DISABLE,     FH_PLL_ENABLE   , 0, 1599000 , 0}, //MSDCPLL  default SSC enable
	{FH_FH_DISABLE,     FH_PLL_ENABLE   , 0, 1188000 , 0}, //TVDPLL   default SSC disable
	{FH_FH_DISABLE,     FH_PLL_DISABLE  , 0, 1200000 , 0}  //LVDSPLL  default SSC disable
};



//ARMPLL
static const struct freqhopping_ssc mt_ssc_armpll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{1209000 ,0 ,9 ,0, 0x770A, 0xBA000},// 1.209GHz , 0.27us, 0.023437500, 0 ~ -8%
	{1001000 ,0 ,9 ,0, 0x628F, 0x9A000},// 1.001GHz , 0.27us, 0.023437500, 0 ~ -8% 
	{ 715000 ,0 ,9 ,0, 0x8CCC, 0xDC000},//   715MHz , 0.27us, 0.023437500, 0 ~ -8%
	{ 419250 ,0 ,9 ,0, 0xA51E, 0x102000},//419.25MHz , 0.27us, 0.023437500, 0 ~ -8%
	{0,0,0,0,0,0} //EOF
};

static const struct freqhopping_ssc mt_ssc_mainpll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{1612000 ,0 ,9 ,0x4f5c, 0x4f5c, 0xF8000},// 1.209GHz , 0.27us, 0.023437500, +4% ~ -4%
	{0,0,0,0,0,0} //EOF
};

static const struct freqhopping_ssc mt_ssc_mempll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{266000 ,0 ,7 ,0, 0x61CD, 0x00131A2E},//266MHz , 0.27us, 0.007812500 , 0 ~ -4%
	//{266000 ,0 ,9 ,0, 0xC39B, 0x00131A2E},//266MHz , 0.27us, 0.023437500, 0 ~ -8%
	{233500 ,0 ,7 ,0, 0x55C2, 0x0010C000},//233.5MHz , 0.27us, 0.007812500, 0 ~ -4%  
	{208500 ,0 ,7 ,0, 0x4CA8, 0x000EF8F5},//208.5MHz , 0.27us, 0.007812500, 0 ~ -4%  
	{200000 ,0 ,7 ,0, 0x4989, 0x000E5CCC},//200MHz , 0.27us, 0.007812500, 0 ~ -4%  
	{0,0,0,0,0,0} //EOF
};

static const struct freqhopping_ssc mt_ssc_msdcpll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{1599000, 0, 9, 0, 0x9d70, 0xF6000}, // 1599MHz, ..., 61.5 ..0 ~ -8%
	{1352000, 0, 9, 0, 0x851e, 0xD0000}, // 1352MHz, ..., 52 ..0 ~ -8%
	{0,0,0,0,0,0} //EOF

};
static const struct freqhopping_ssc mt_ssc_tvdpll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{1188000 ,0 ,9 ,0, 0x74f8, 0xB6C4F},// 1188MHz ,45.69230769 0 ~ -8%
	{1728000, 0, 9, 0, 0xaa24, 0x109D89}, // 1728MHz, 66.46153846 0 ~ -8%
	{0,0,0,0,0,0} //EOF
};
static const struct freqhopping_ssc mt_ssc_lvdspll_setting[MT_SSC_NR_PREDEFINE_SETTING]= {
	{0,0,0,0,0,0},//Means disable
	{0,0xFF,0xFF,0xFF,0xFF,0xFF},//Means User-Define
	{1200000, 0, 9, 0, 0x9627, 0xB89D8}, // 1200MHz ,46.15384615
	{1400000, 0, 9, 0, 0x8dc8, 0xDD89D}, // 1400MHz, 55.38461538
	{0,0,0,0,0,0} //EOF
};

static struct freqhopping_ssc mt_ssc_fhpll_userdefined[MT_FHPLL_MAX]= {
	{0,1,1,2,2,0}, //ARMPLL
	{0,1,1,2,2,0}, //MAINPLL
	{0,1,1,2,2,0}, //MEMPLL
	{0,1,1,2,2,0}, //MSDCPLL
	{0,1,1,2,2,0}, //TVDPLL
	{0,1,1,2,2,0}  //LVDSPLL
};

#else //PER_PROJECT_FH_SETTING

PER_PROJECT_FH_SETTING

#endif	//PER_PROJECT_FH_SETTING

unsigned int mt_get_emi_freq(void);

#define PLL_STATUS_ENABLE 1
#define PLL_STATUS_DISABLE 0

static void mt_fh_hal_default_conf(void)
{
	FH_MSG("EN: %s",__func__);
	
	freqhopping_config(MT658X_FH_MEM_PLL, 266000, true); //Enable MEMPLL SSC
}


static void update_fhctl_status(const int pll_id, const int enable)
{
        int i = 0 ;
        int enabled_num = 0 ;
        static unsigned int pll_status[] = {
                PLL_STATUS_DISABLE, //ARMPLL
                PLL_STATUS_DISABLE, //MAINPLL
                PLL_STATUS_DISABLE, //MEMPLL
                PLL_STATUS_DISABLE, //MSDCPLL
                PLL_STATUS_DISABLE, //TVDPLL
                PLL_STATUS_DISABLE  //LVDSPLL
        } ;

        //FH_MSG("PLL#%d ori status is %d, you hope to change to %d\n", pll_id, pll_status[pll_id], enable) ;
        FH_MSG("PL%d:%d->%d", pll_id, pll_status[pll_id], enable) ;
        if(pll_status[pll_id] == enable) {
                FH_MSG("no ch") ;//no change
                return ;
        }

        pll_status[pll_id] = enable ;

        for(i = MT658X_FH_MINIMUMM_PLL ; i <= MT658X_FH_MAXIMUMM_PLL ; i++) {

                if(pll_status[i] == PLL_STATUS_ENABLE) {
                        //FH_MSG("PLL#%d is enabled", i) ;
                        enabled_num++ ;
                }
                /*else {
                        FH_MSG("PLL#%d is disabled", i) ;
                }*/
        }

        FH_MSG("PLen#=%d",enabled_num) ;

#if 0  //TODO: FIX the enable clock mechanism
        if((g_clk_en == 0)&&(enabled_num >= 1)) {
		enable_clock_ext_locked(MT_CG_PERI1_FHCTL, "FREQHOP") ;
                //wait_for_sophieenable_clock(MT_CG_PERI1_FHCTL, "FREQHOP") ;
                g_clk_en = 1;                
        }
        else if((g_clk_en == 1)&&(enabled_num == 0)) {
                disable_clock_ext_locked(MT_CG_PERI1_FHCTL, "FREQHOP") ;
                //wait_for_sophie disable_clock(MT_CG_PERI1_FHCTL, "FREQHOP") ;
                g_clk_en = 0;
        }
#endif        
}



static int __mt_enable_freqhopping(unsigned int pll_id,const struct freqhopping_ssc* ssc_setting)
{
	unsigned int 	pll_hp=0;
	unsigned long 	flags=0;

	FH_MSG("EN: en_fh");
	FH_MSG("l: %x u: %x df: %d dt: %d dds:%x",ssc_setting->lowbnd
			   			      ,ssc_setting->upbnd
			   			      ,ssc_setting->df
			   			      ,ssc_setting->dt
			   			      ,ssc_setting->dds);
	
	update_fhctl_status(pll_id, PLL_STATUS_ENABLE) ;
        mb() ;


	//lock @ __freqhopping_ctrl_lock()
	//spin_lock_irqsave(&freqhopping_lock, flags);

	g_fh_pll[pll_id].fh_status = FH_FH_ENABLE_SSC;		


	//TODO: should we check the following here ??
	//if(unlikely(FH_PLL_STATUS_ENABLE == g_fh_pll[pll_id].fh_status)){
	//Do nothing due to this not allowable flow
	//We shall DISABLE and then re-ENABLE for the new setting or another round	
	//FH_MSG("ENABLE the same FH",pll_id);
	//WARN_ON(1);
	//spin_unlock_irqrestore(&freqhopping_lock, flags);
	//return 1;
	//}else {
	
	local_irq_save(flags);	

	//Set the relative parameter registers (dt/df/upbnd/downbnd)
	//Enable the fh bit
	fh_set_field(REG_FHCTL0_CFG+(0x10*pll_id),FH_FRDDSX_DYS,ssc_setting->df);
	fh_set_field(REG_FHCTL0_CFG+(0x10*pll_id),FH_FRDDSX_DTS,ssc_setting->dt);
	fh_set_field(REG_FHCTL0_CFG+(0x10*pll_id),FH_SFSTRX_DYS,ssc_setting->df);
	fh_set_field(REG_FHCTL0_CFG+(0x10*pll_id),FH_SFSTRX_DTS,ssc_setting->dt);

	fh_write32(REG_FHCTL0_UPDNLMT+(0x10*pll_id), (((ssc_setting->lowbnd) << 16) | (ssc_setting->upbnd)));
	
	fh_write32(REG_FHCTL0_DDS+(0x10*pll_id), (ssc_setting->dds)|(1U<<31));
	pll_hp = fh_read32(PLL_HP_CON0) | (1 << pll_id);
	fh_write32( PLL_HP_CON0,  pll_hp );

	mb();
	
	fh_set_field(REG_FHCTL0_CFG+(0x10*pll_id),FH_FRDDSX_EN,1);
	fh_set_field(REG_FHCTL0_CFG+(0x10*pll_id),FH_FHCTLX_EN,1);
	//lock @ __freqhopping_ctrl_lock()
	//spin_unlock_irqrestore(&freqhopping_lock, flags);

	local_irq_restore(flags);

	//FH_MSG("Exit");
	return 0;
}

static int __mt_disable_freqhopping(unsigned int pll_id,const struct freqhopping_ssc* ssc_setting)
{
	unsigned long 	flags=0;
	unsigned int 	pll_hp=0;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;


	FH_MSG("EN: _dis_fh");
	
	//lock @ __freqhopping_ctrl_lock()
	//spin_lock_irqsave(&freqhopping_lock, flags);

	local_irq_save(flags);
	//Set the relative registers	
	fh_set_field(REG_FHCTL0_CFG+(0x10*pll_id),FH_FRDDSX_EN,0);
	fh_set_field(REG_FHCTL0_CFG+(0x10*pll_id),FH_FHCTLX_EN,0);
	mb();
	local_irq_restore(flags);


	if(pll_id == 2){ //for mempll
		unsigned int i=0;
		
		pll_dds =  (DRV_Reg32(DDRPHY_BASE+0x624)) >> 11 ;
		fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
		
		FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
		
		while((pll_dds != fh_dds) && ( i < 100)){

			if(unlikely(i > 100)){
				FH_BUG_ON(i > 100);
				break;
			}
				
			udelay(10);
			fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			i++;
		}	
		
		FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
	}
	else{
		//TODO:  sync&wait DDS back to PLLCON1@other PLLs
		FH_MSG("n-mempll");
	}
	
		
	local_irq_save(flags);
	pll_hp = fh_read32(PLL_HP_CON0) & ~(1 << pll_id);
	fh_write32( (PLL_HP_CON0),  pll_hp );
	
	g_fh_pll[pll_id].fh_status = FH_FH_DISABLE;
	local_irq_restore(flags);
	
	//lock @ __freqhopping_ctrl_lock()
	//spin_unlock_irqrestore(&freqhopping_lock, flags);

	mb() ;
        update_fhctl_status(pll_id, PLL_STATUS_DISABLE) ;
	

	//FH_MSG("Exit");

	return 0;
}


//freq is in KHz, return at which number of entry in mt_ssc_xxx_setting[]
static noinline int __freq_to_index(enum FH_PLL_ID pll_id,int freq) 
{
	unsigned int retVal = 0;
	unsigned int i=2; //0 is disable, 1 is user defines, so start from 2
	
	//FH_MSG("EN: %s , pll_id: %d, freq: %d",__func__,pll_id,freq);
	FH_MSG("EN: , id: %d, f: %d",pll_id,freq);
	
	switch(pll_id) {

		//TODO: use Marco or something to make the code less redudant
		case MT658X_FH_ARM_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_armpll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;

		
		case MT658X_FH_MAIN_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_mainpll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;
		
		case MT658X_FH_MEM_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_mempll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;

		case MT658X_FH_MSDC_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_msdcpll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;

		case MT658X_FH_TVD_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_tvdpll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;

		case MT658X_FH_LVDS_PLL:
		while(i < MT_SSC_NR_PREDEFINE_SETTING){
			if(freq == mt_ssc_lvdspll_setting[i].freq){
				retVal = i;
				break;
			}
			i++;
		}
		break;
		
		case MT658X_FH_PLL_TOTAL_NUM:
		FH_MSG("Error MT658X_FH_PLL_TOTAL_NUM!");			
		break;
		

	};

	return retVal;	
}

static int __freqhopping_ctrl(struct freqhopping_ioctl* fh_ctl,bool enable)
{
	const struct freqhopping_ssc* 	pSSC_setting=NULL;
	unsigned int  			ssc_setting_id=0;
	int				retVal=1;
	
	FH_MSG("EN: _fh_ctrl %d:%d",fh_ctl->pll_id,enable);
	//FH_MSG("%s fhpll_id: %d, enable: %d",(enable)?"enable":"disable",fh_ctl->pll_id,enable);

	//Check the out of range of frequency hopping PLL ID
	FH_BUG_ON(fh_ctl->pll_id>MT658X_FH_MAXIMUMM_PLL);
	FH_BUG_ON(fh_ctl->pll_id<MT658X_FH_MINIMUMM_PLL);

	if((enable == true) && (g_fh_pll[fh_ctl->pll_id].fh_status == FH_FH_ENABLE_SSC)  ){ 
		
		//The FH already enabled @ this PLL
	
		FH_MSG("re-en FH");
		
		//disable FH first, will be enable later
		__mt_disable_freqhopping(fh_ctl->pll_id,pSSC_setting);
	}
	else if((enable == false) && (g_fh_pll[fh_ctl->pll_id].fh_status == FH_FH_DISABLE)){
		
		//The FH already been disabled @ this PLL, do nothing & return
		
		FH_MSG("re-dis FH");	
		retVal = 0;
		goto Exit;		
	}
		
	
	//ccyeh fh_status set @ __mt_enable_freqhopping() __mt_disable_freqhopping()
	//g_fh_pll[fh_ctl->pll_id].fh_status = enable?FH_FH_ENABLE_SSC:FH_FH_DISABLE;
	
	if( enable == true) { //enable freq. hopping @ fh_ctl->pll_id

		if(g_fh_pll[fh_ctl->pll_id].pll_status == FH_PLL_DISABLE) {
			FH_MSG("pll is dis");
			
			//update the fh_status & don't really enable the SSC
			g_fh_pll[fh_ctl->pll_id].fh_status = FH_FH_ENABLE_SSC;
			retVal = 0;
			goto Exit;
		} 
		else {
			FH_MSG("pll is en");
			if(g_fh_pll[fh_ctl->pll_id].user_defined == true){
				FH_MSG("use u-def");

				pSSC_setting = &mt_ssc_fhpll_userdefined[fh_ctl->pll_id];
				g_fh_pll[fh_ctl->pll_id].setting_id = USER_DEFINE_SETTING_ID; 
			} 
			else {
				FH_MSG("n-user def");
				
				if( g_fh_pll[fh_ctl->pll_id].curr_freq != 0 ){
					ssc_setting_id = g_fh_pll[fh_ctl->pll_id].setting_id = __freq_to_index(fh_ctl->pll_id, g_fh_pll[fh_ctl->pll_id].curr_freq);
				}
				else{
					ssc_setting_id = 0;
				}
					
				
				FH_MSG("sid %d",ssc_setting_id);
				if(ssc_setting_id == 0){
					FH_MSG("!!! No corresponding setting found !!!");
					
					//just disable FH & exit
					__mt_disable_freqhopping(fh_ctl->pll_id,pSSC_setting);
					goto Exit;
				}
				
				switch(fh_ctl->pll_id) {
					case MT658X_FH_MAIN_PLL:
					FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_mainpll_setting)/sizeof(struct freqhopping_ssc)) );
					pSSC_setting = &mt_ssc_mainpll_setting[ssc_setting_id];
					break;
					case MT658X_FH_ARM_PLL:
					FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_armpll_setting)/sizeof(struct freqhopping_ssc)));
					pSSC_setting = &mt_ssc_armpll_setting[ssc_setting_id];
					break;
					case MT658X_FH_MSDC_PLL:
					FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_msdcpll_setting)/sizeof(struct freqhopping_ssc)));
					pSSC_setting = &mt_ssc_msdcpll_setting[ssc_setting_id];
					break;
					case MT658X_FH_TVD_PLL:
					FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_tvdpll_setting)/sizeof(struct freqhopping_ssc)));
					pSSC_setting = &mt_ssc_tvdpll_setting[ssc_setting_id];
					break;
					case MT658X_FH_LVDS_PLL:
					FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_lvdspll_setting)/sizeof(struct freqhopping_ssc)));
					pSSC_setting = &mt_ssc_lvdspll_setting[ssc_setting_id];
					break;
					case MT658X_FH_MEM_PLL:
					FH_BUG_ON(ssc_setting_id > (sizeof(mt_ssc_mempll_setting)/sizeof(struct freqhopping_ssc)));
					pSSC_setting = &mt_ssc_mempll_setting[ssc_setting_id];
					break;
				}
			}//user defined

			if(pSSC_setting == NULL){
				FH_MSG("!!! pSSC_setting is NULL !!!");
				//just disable FH & exit
				__mt_disable_freqhopping(fh_ctl->pll_id,pSSC_setting);
				goto Exit;
			}
			
			if( 0 == __mt_enable_freqhopping(fh_ctl->pll_id,pSSC_setting)) {
				retVal = 0;
				FH_MSG("en ok");
			}
			else{
				FH_MSG("__mt_enable_freqhopping fail.");
			}
		}
		
	}
	else{ //disable req. hopping @ fh_ctl->pll_id
		if( 0 == __mt_disable_freqhopping(fh_ctl->pll_id,pSSC_setting)) {
			retVal = 0;
			FH_MSG("dis ok");
		}
		else{
			FH_MSG("__mt_disable_freqhopping fail.");
		}
	}

Exit:
			
	//FH_MSG("Exit");
	return retVal;
}


#if MT_FH_DUMMY_READ

static noinline void mt_dummy_read(void)
{
	
	U64 		cur_time=0;
	unsigned int 	dummy=0;
	unsigned int 	i = 0;

	// get current tick
	
	FH_MSG("EN: DR() r0:%p r1:%p",g_fh_rank0_va,g_fh_rank1_va);

	cur_time = sched_clock();//ns
	if((DRV_Reg32(EMI_CONA) & 0x20000)){ /* check dual rank support */
		//dual rank
		while((sched_clock() - cur_time) < 800000)
		{
			i++;
			if(likely(g_fh_rank1_va != NULL)){
				dummy = g_fh_rank1_va[0];
			}
			
			if(likely(g_fh_rank0_va != NULL)){
				dummy = g_fh_rank0_va[0];
			}
			
			udelay(1);
			//dmac_flush_range(&g_fh_rank0_va , sizeof(g_fh_rank0_va));
			//dmac_flush_range(g_fh_rank1_va , g_fh_rank1_va + SZ_4K);

		}
	}
	else{ 
		//mono rank
		while((sched_clock() - cur_time) < 800000)
		{
			i++;
			if(likely(g_fh_rank0_va != NULL)){
				dummy = g_fh_rank0_va[0];
			}

			//dmac_flush_range(&g_fh_rank0_va , sizeof(g_fh_rank0_va));
			udelay(1);
		}
	}

	FH_MSG("i: %d",i);

	return;
}

static void mt_dummy_read_init(void)
{
	unsigned int 	fh_pfn;
	dma_addr_t 	dma_addr;
	unsigned long 	flags;
	unsigned long	i = 0;
	
	FH_MSG("EN: mt_dummy_read_init()");

	g_fh_rank0_va = dma_alloc_coherent( NULL, SZ_4K, &dma_addr, GFP_KERNEL);

	if (!g_fh_rank0_va){
		FH_BUG_ON(!g_fh_rank0_va);
		FH_MSG("Fail to dma_alloc_coherent for rank0!");
		g_fh_rank0_pa = NULL;
		g_fh_rank0_va = NULL;
	}

	FH_MSG("dma_addr: 0x%x",dma_addr);
	g_fh_rank0_pa = (unsigned int *)dma_addr;

	if((DRV_Reg32(EMI_CONA) & 0x20000)){ /* check dual rank support */
		
		do {
			if(i > 10){
				FH_BUG_ON(i>10);
				FH_MSG("Fail to allocate HIGHMEM @ rank1!");
				flags = 1;
				g_fh_rank1_pa = NULL;
				g_fh_rank1_va = NULL;
			}
			else{
				g_fh_rank1_va = __vmalloc(SZ_4K, __GFP_HIGHMEM, pgprot_noncached(PAGE_KERNEL));			
	
				if (g_fh_rank1_va == NULL){
					FH_MSG("vmalloc null");
				}
				else{
					fh_pfn = vmalloc_to_pfn(g_fh_rank1_va);
					g_fh_rank1_pa = (unsigned int *)__pfn_to_phys(fh_pfn);
		
					FH_MSG("g_fh_rank1_pa:%p\n",g_fh_rank1_pa);
				
					if (((unsigned int)g_fh_rank1_pa) < 0xa0000000){
						vfree(g_fh_rank1_va);
						flags = 0;
					}
					else{
						flags = 1;
					}
				}
			}

			i++;
		}while (flags == 0);
		
		FH_MSG("rank1: g_fh_rank0_pa:%p\n",g_fh_rank0_pa);
		FH_MSG("rank1: g_fh_rank0_va:%p\n",g_fh_rank0_va);
		FH_MSG("rank1: g_fh_rank1_pa:%p\n",g_fh_rank1_pa);
		FH_MSG("rank1: g_fh_rank1_va:%p\n",g_fh_rank1_va);
	}
}

#else //MT_FH_DUMMY_READ

static void mt_dummy_read_init(void)
{
	FH_MSG("EN: mt_dummy_read_init()");
}
static noinline void mt_dummy_read(void)
{
	FH_MSG("EN: DR() r0:%p r1:%p",g_fh_rank0_va,g_fh_rank1_va);
	udelay(800);
}

#endif //MT_FH_DUMMY_READ


//mempll 266->293MHz using FHCTL
static int mt_h2oc_mempll(void)
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;


	FH_MSG("EN: %s:%d",__func__,g_curr_dramc);

	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */

	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}

	if(g_curr_dramc != 266){
		FH_MSG("g_curr_dramc != 266)");
		return -1;	
	}

	FH_MSG("dds: %x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));

	if((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x131a2e){
		FH_BUG_ON((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x131a2e);
		FH_MSG("DDS != 0x131a2e");
		return -1;
	}
	
	//TODO: provelock issue spin_lock(&freqhopping_lock); 
	spin_lock_irqsave(&freqhopping_lock, flags);

	//disable SSC when OC
	__mt_disable_freqhopping(2, NULL);
	
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){//only when SSC is enable
		
		//Turn off MEMPLL hopping
		fh_write32(REG_FHCTL2_CFG, 0x70700000);
	
		//udelay(800);
		
		pll_dds =  (DRV_Reg32(DDRPHY_BASE+0x624)) >> 11 ;
		fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
		
		FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
		
		while((pll_dds != fh_dds) && ( i < 100)){
	
			if(unlikely(i > 100)){
				FH_BUG_ON(i > 100);
				break;
			}
				
			udelay(10);
			fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			i++;
		}	
		
		FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
	}

	FH_MSG("=> 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);

	//---------------------------------------------
	//Overwrite MEMPLL NCPO setting in hopping SRAM
	//---------------------------------------------
	//Turn on SRAM CE, point to SRAM address of MEMPLL NCPO value 0
	fh_write32(REG_FHSRAM_CON, (0x1 << 9 | 0x80) );

	//Target Freq. : NCPO INT : 84.165 (26MHz*84.165 = 2188.29 MHz, DRAMC = 293MHz)
	//INT : 84 => 0x54 << 14
	//FRACTION : 0.165 => 0x0A8F
	fh_write32(REG_FHSRAM_WR, ((0x54 << 14) | 0x0A8F) );
	
	//-------------------------------------------------
	// Turn on MEMPLL hopping and set slope < 2.5MHz/us
	//-------------------------------------------------
	//Only turn on 2G1 hopping
	fh_write32(REG_FHCTL_CFG, (1 << 8));

	//Use 2G1 hopping table to DFS
	//SW need fill 64・b1 in 2G1 DRAM address (channel number = 0)
	fh_write32(REG_FHDMA_2G1BASE,  __pa(g_mempll_fh_table));

	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));

        fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) | (1 << 2)) );
       
       	mb();

	//Configure hopping slope
	//sfstr_dts[3:0] = 4・d0(0.27us)
	//sfstr_dys[3:0] = 4・d9(0.023437500)
	//slope = 26MHz * 0.007812500 / 0.27us = 0.75MHz/us < 2.5MHz/us
	fh_write32(REG_FHCTL2_CFG, ( (0 << 24) | (7 << 28) | 5));
	
	//-------------------------------------------------
	// Trigger 2G1 Hopping on channel number 0
	//-------------------------------------------------
	fh_write32(REG_FHCTL_2G1_CH, 0);

	g_curr_dramc = 293;

	//TODO: provelock issue local_irq_restore(flags);

	mt_dummy_read();

	FH_MSG("-MON- 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);

	fh_write32( DDRPHY_BASE+0x624, (fh_read32(DDRPHY_BASE+0x624) & 0x7FF) ); //clear NCPO
	fh_write32( DDRPHY_BASE+0x624, ( fh_read32(DDRPHY_BASE+0x624) |(((0x54 << 14) | 0x0A8F ) << 11 )) ); //set NCPO
	
	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));
	
	//Turn off MEMPLL hopping
	fh_write32(REG_FHCTL2_CFG, 0x70700000);

	//TODO: provelock issue local_irq_restore(flags);

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){
		FH_MSG("to SSC");
		
	}
	else{
		//switch back to APMIXED
		FH_MSG("to APM");
        	fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) & ~(1 << 2)) );
        }

	FH_MSG("+MON+ 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	g_fh_pll[MT658X_FH_MEM_PLL].curr_freq 	= 293000;
	g_fh_pll[MT658X_FH_MEM_PLL].pll_status 	= FH_PLL_ENABLE;

	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);
	
	freqhopping_config(MT658X_FH_MEM_PLL, 293000, true); //update freq.

	return 0;
}

//mempll 293->266MHz using FHCTL
static int mt_oc2h_mempll(void)  
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;

	
	FH_MSG("EN: %s:%d",__func__,g_curr_dramc);
	
	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */

	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}
	
	if(g_curr_dramc != 293){
		FH_MSG("g_curr_dramc != 293)");
		return -1;	
	}
	
	FH_MSG("dds: %x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));
	
	if((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x150A8F){
		FH_BUG_ON((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x150A8F);
		FH_MSG("DDS != 0x150A8F");
		return 0;			
	}

	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);
	
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){//only when SSC is enable
		
		//Turn off MEMPLL hopping
		fh_write32(REG_FHCTL2_CFG, 0x70700000);
	
		//udelay(800);
		
		pll_dds =  (DRV_Reg32(DDRPHY_BASE+0x624)) >> 11 ;
		fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
		
		FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
		
		while((pll_dds != fh_dds) && ( i < 100)){
	
			if(unlikely(i > 100)){
				FH_BUG_ON(i > 100);
				break;
			}
				
			udelay(10);
			fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			i++;
		}	
		
		FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
	}


	FH_MSG("=> 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);
	
	//---------------------------------------------
	//Overwrite MEMPLL NCPO setting in hopping SRAM
	//---------------------------------------------
	//Turn on SRAM CE, point to SRAM address of MEMPLL NCPO value 0
	fh_write32(REG_FHSRAM_CON, (0x1 << 9 | 0x80) );

	//Target Freq. : NCPO INT : 76.409 (26MHz*76.409 = 1986.63MHz, DRAMC = 266MHz)
	//INT : 76 => 0x4C << 14
	//FRACTION : 0.409 => 0x1A2E
	fh_write32(REG_FHSRAM_WR, ((0x4C << 14) | 0x1A2E) );
	
	
	//-------------------------------------------------
	// Turn on MEMPLL hopping and set slope < 2.5MHz/us
	//-------------------------------------------------
	//Only turn on 2G1 hopping
	fh_write32(REG_FHCTL_CFG, (1 << 8));

	//Use 2G1 hopping table to DFS
	//SW need fill 64・b1 in 2G1 DRAM address (channel number = 0)
	fh_write32(REG_FHDMA_2G1BASE,  __pa(g_mempll_fh_table));

	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));
	
        fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) | (1 << 2)) );
	
	mb();

	//Configure hopping slope
	//sfstr_dts[3:0] = 4・d0(0.27us)
	//sfstr_dys[3:0] = 4・d9(0.023437500)
	//slope = 26MHz * 0.007812500 / 0.27us = 0.75MHz/us < 2.5MHz/us
	fh_write32(REG_FHCTL2_CFG, ( (0 << 24) | (7 << 28) | 5));
	
	//-------------------------------------------------
	// Trigger 2G1 Hopping on channel number 0
	//-------------------------------------------------
	fh_write32(REG_FHCTL_2G1_CH, 0);

	g_curr_dramc = 266;

	//TODO: provelock issue local_irq_restore(flags);
	
	mt_dummy_read();

	FH_MSG("-MON- 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);
	fh_write32( DDRPHY_BASE+0x624, (fh_read32(DDRPHY_BASE+0x624) & 0x7FF) ); //clear NCPO
	fh_write32( DDRPHY_BASE+0x624, ( fh_read32(DDRPHY_BASE+0x624) |(((0x4C << 14) | 0x1A2E ) << 11 )) ); //set NCPO
	
	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));

	//Turn off MEMPLL hopping
	fh_write32(REG_FHCTL2_CFG, 0x70700000);
	//TODO: provelock issue local_irq_restore(flags);
	
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){
		FH_MSG("to SSC");
		
	}
	else{
		//switch back to APMIXED
		FH_MSG("to APM");
        	fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) & ~(1 << 2)) );
	}

	FH_MSG("+MON+ 0x%x",DRV_Reg32(REG_FHCTL2_MON));
	g_fh_pll[MT658X_FH_MEM_PLL].curr_freq 	= 266000;
	g_fh_pll[MT658X_FH_MEM_PLL].pll_status 	= FH_PLL_ENABLE;

	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);
	
	freqhopping_config(MT658X_FH_MEM_PLL, 266000, true); //update freq.
	
	return 0;
}

//mempll 200->266MHz using FHCTL
static int mt_fh_hal_l2h_mempll(void)  //mempll low to high (200->266MHz)
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;
	
	FH_MSG("EN: %s:%d",__func__,g_curr_dramc);
	
	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */

	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}
	
	if(g_curr_dramc == 266){
		return -1;	
	}
	
	FH_MSG("dds: %x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));
	
	if((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != LOW_DRAMC_DDS){
		FH_BUG_ON((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != LOW_DRAMC_DDS);
		FH_MSG("DDS != 0x%X",LOW_DRAMC_DDS);
		return 0;			
	}

	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);

	
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){//only when SSC is enable
		
		//Turn off MEMPLL hopping
		fh_write32(REG_FHCTL2_CFG, 0x70700000);
	
		//udelay(800);
		
		pll_dds =  (DRV_Reg32(DDRPHY_BASE+0x624)) >> 11 ;
		fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
		
		FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
		
		while((pll_dds != fh_dds) && ( i < 100)){
	
			if(unlikely(i > 100)){
				FH_BUG_ON(i > 100);
				break;
			}
				
			udelay(10);
			fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			i++;
		}	
		
		FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
	}


	FH_MSG("=> 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);
	
	//---------------------------------------------
	//Overwrite MEMPLL NCPO setting in hopping SRAM
	//---------------------------------------------
	//Turn on SRAM CE, point to SRAM address of MEMPLL NCPO value 0
	fh_write32(REG_FHSRAM_CON, (0x1 << 9 | 0x80) );

	//Target Freq. : NCPO INT : 76.409 (26MHz*76.409 = 1986.63MHz, DRAMC = 266MHz)
	//INT : 76 => 0x4C << 14
	//FRACTION : 0.409 => 0x1A2E
	fh_write32(REG_FHSRAM_WR, ((0x4C << 14) | 0x1A2E) );
	
	
	//-------------------------------------------------
	// Turn on MEMPLL hopping and set slope < 2.5MHz/us
	//-------------------------------------------------
	//Only turn on 2G1 hopping
	fh_write32(REG_FHCTL_CFG, (1 << 8));

	//Use 2G1 hopping table to DFS
	//SW need fill 64・b1 in 2G1 DRAM address (channel number = 0)
	fh_write32(REG_FHDMA_2G1BASE,  __pa(g_mempll_fh_table));

	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));
	
        fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) | (1 << 2)) );
	
	mb();

	//Configure hopping slope
	//sfstr_dts[3:0] = 4・d0(0.27us)
	//sfstr_dys[3:0] = 4・d9(0.023437500)
	//slope = 26MHz * 0.007812500 / 0.27us = 0.75MHz/us < 2.5MHz/us
	fh_write32(REG_FHCTL2_CFG, ( (0 << 24) | (7 << 28) | 5));
	
	//-------------------------------------------------
	// Trigger 2G1 Hopping on channel number 0
	//-------------------------------------------------
	fh_write32(REG_FHCTL_2G1_CH, 0);

	g_curr_dramc = 266;

	//TODO: provelock issue local_irq_restore(flags);
	
	mt_dummy_read();

	FH_MSG("-MON- 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);
	fh_write32( DDRPHY_BASE+0x624, (fh_read32(DDRPHY_BASE+0x624) & 0x7FF) ); //clear NCPO
	fh_write32( DDRPHY_BASE+0x624, ( fh_read32(DDRPHY_BASE+0x624) |(((0x4C << 14) | 0x1A2E ) << 11 )) ); //set NCPO
	
	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));

	//Turn off MEMPLL hopping
	fh_write32(REG_FHCTL2_CFG, 0x70700000);
	//TODO: provelock issue local_irq_restore(flags);
	
	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){
		FH_MSG("to SSC");
		
	}
	else{
		//switch back to APMIXED
		FH_MSG("to APM");
        	fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) & ~(1 << 2)) );
	}

	FH_MSG("+MON+ 0x%x",DRV_Reg32(REG_FHCTL2_MON));
	g_fh_pll[MT658X_FH_MEM_PLL].curr_freq 	= 266000;
	g_fh_pll[MT658X_FH_MEM_PLL].pll_status 	= FH_PLL_ENABLE;

	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);

	freqhopping_config(MT658X_FH_MEM_PLL, 266000, true); //update freq.
	
	return 0;
}

//mempll 266->200MHz using FHCTL
static int mt_fh_hal_h2l_mempll(void)  //mempll low to high (200->266MHz)
{
	unsigned long 	flags;
	unsigned int	fh_dds=0;
	unsigned int	pll_dds=0;
	unsigned int	i=0;
	
	FH_MSG("EN: %s:%d",__func__,g_curr_dramc);
	
	/*Please note that the DFS can・t be applied in systems with DDR3 DRAM!
	  It・s only for the mobile DRAM (LPDDR*)! */

	if( (get_ddr_type() != LPDDR2) && (get_ddr_type() != LPDDR3) ) {
		FH_MSG("Not LPDDR*");
		return -1;
	}

	if(g_curr_dramc == LOW_DRAMC){
		return -1;	
	}

	FH_MSG("dds: %x",(DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ));

	if((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x131a2e){
		FH_BUG_ON((DRV_Reg32(DDRPHY_BASE+0x624) >> 11 ) != 0x131a2e);
		FH_MSG("DDS != 0x131a2e");
		return 0;			
	}
	
	//TODO: provelock issue spin_lock(&freqhopping_lock);
	spin_lock_irqsave(&freqhopping_lock, flags);
	

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){//only when SSC is enable
		
		//Turn off MEMPLL hopping
		fh_write32(REG_FHCTL2_CFG, 0x70700000);
	
		//udelay(800);
		
		pll_dds =  (DRV_Reg32(DDRPHY_BASE+0x624)) >> 11 ;
		fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
		
		FH_MSG(">p:f< %x:%x",pll_dds,fh_dds);
		
		while((pll_dds != fh_dds) && ( i < 100)){
	
			if(unlikely(i > 100)){
				FH_BUG_ON(i > 100);
				break;
			}
				
			udelay(10);
			fh_dds  =  (DRV_Reg32(REG_FHCTL2_MON)) & 0x1FFFFF;
			i++;
		}	
		
		FH_MSG("<p:f:i> %x:%x:%d",pll_dds,fh_dds,i);
	}

	FH_MSG("=> 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);

	//---------------------------------------------
	//Overwrite MEMPLL NCPO setting in hopping SRAM
	//---------------------------------------------
	//Turn on SRAM CE, point to SRAM address of MEMPLL NCPO value 0
	fh_write32(REG_FHSRAM_CON, (0x1 << 9 | 0x80) );

	//>>>Example<<<
	//Target Freq. : NCPO INT : 59.89 (26MHz*59.89 = 1557.14 MHz, DRAMC = 208.5MHz)
	//INT : 59 => 0x3B << 14
	//FRACTION : 0.89 => 0x38F5
	fh_write32(REG_FHSRAM_WR, LOW_DRAMC_DDS);
	
	
	//-------------------------------------------------
	// Turn on MEMPLL hopping and set slope < 2.5MHz/us
	//-------------------------------------------------
	//Only turn on 2G1 hopping
	fh_write32(REG_FHCTL_CFG, (1 << 8));

	//Use 2G1 hopping table to DFS
	//SW need fill 64・b1 in 2G1 DRAM address (channel number = 0)
	fh_write32(REG_FHDMA_2G1BASE,  __pa(g_mempll_fh_table));

	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));

        fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) | (1 << 2)) );
        
       	mb();

	//Configure hopping slope
	//sfstr_dts[3:0] = 4・d0(0.27us)
	//sfstr_dys[3:0] = 4・d9(0.023437500)
	//slope = 26MHz * 0.007812500 / 0.27us = 0.75MHz/us < 2.5MHz/us
	fh_write32(REG_FHCTL2_CFG, ( (0 << 24) | (7 << 28) | 5));
	
	//-------------------------------------------------
	// Trigger 2G1 Hopping on channel number 0
	//-------------------------------------------------
	fh_write32(REG_FHCTL_2G1_CH, 0);

	g_curr_dramc = LOW_DRAMC;

	//TODO: provelock issue local_irq_restore(flags);

	mt_dummy_read();

	FH_MSG("-MON- 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	//TODO: provelock issue local_irq_save(flags);

	fh_write32( DDRPHY_BASE+0x624, (fh_read32(DDRPHY_BASE+0x624) & 0x7FF) ); //clear NCPO
	fh_write32( DDRPHY_BASE+0x624, ( fh_read32(DDRPHY_BASE+0x624) |(((LOW_DRAMC_INT << 14) | LOW_DRAMC_FRACTION ) << 11 )) ); //set NCPO
	
	//sync NCPO value
	fh_write32(REG_FHCTL2_DDS, (DRV_Reg32(DDRPHY_BASE+0x624) >> 11 )|(1U<<31));
	
	//Turn off MEMPLL hopping
	fh_write32(REG_FHCTL2_CFG, 0x70700000);

	//TODO: provelock issue local_irq_restore(flags);

	if(g_fh_pll[MT658X_FH_MEM_PLL].fh_status == FH_FH_ENABLE_SSC){
		FH_MSG("to SSC");
		
	}
	else{
		//switch back to APMIXED
		FH_MSG("to APM");
        	fh_write32( PLL_HP_CON0,  (fh_read32(PLL_HP_CON0) & ~(1 << 2)) );
        }

	FH_MSG("+MON+ 0x%x",DRV_Reg32(REG_FHCTL2_MON));

	g_fh_pll[MT658X_FH_MEM_PLL].curr_freq 	= LOW_DRAMC_FREQ;
	g_fh_pll[MT658X_FH_MEM_PLL].pll_status 	= FH_PLL_ENABLE;

	//TODO: provelock issue spin_unlock(&freqhopping_lock);
	spin_unlock_irqrestore(&freqhopping_lock, flags);

	freqhopping_config(MT658X_FH_MEM_PLL, LOW_DRAMC_FREQ, true); //update freq.

	return 0;
}

static int mt_fh_hal_dram_overclock(int clk)
{
	FH_MSG("EN: %s clk:%d",__func__,clk);
	
	if( clk == LOW_DRAMC){ //target freq: 208.5MHz
		if( g_curr_dramc != 266 ){ //266 -> 208.5 only
			FH_BUG_ON(1);
			return -1;
		}
		else{ //let's move from 266 to 208.5
			return(mt_fh_hal_h2l_mempll());			
		}
	}
	
	if( clk == 293){ //target freq: 293MHz
		if( g_curr_dramc != 266 ){ //266 -> 293 only
			FH_BUG_ON(1);
			return -1;
		}
		else{ //let's move from 266 to 293
			return(mt_h2oc_mempll());			
		}
	}
	
	if( clk == 266){ ////target freq: 293MHz
		if( g_curr_dramc == 266 ){ //cannot be 266 -> 266
			FH_BUG_ON(1);
			return -1;
		}
		else if( g_curr_dramc == LOW_DRAMC ){ //208 -> 266
			return(mt_fh_hal_l2h_mempll());			
		}
		else if( g_curr_dramc == 293 ){ //293 -> 266
			return(mt_oc2h_mempll());			
		}
	}
	
	
	FH_BUG_ON(1);
	return(-1);
}

static int mt_fh_hal_get_dramc(void)
{
	return(g_curr_dramc);
}

static void mt_fh_hal_popod_save(void)
{
	FH_MSG("EN: %s",__func__);
}

static void mt_fh_hal_popod_restore(void)
{
	FH_MSG("EN: %s",__func__);
}

//static int freqhopping_dramc_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
static int freqhopping_dramc_proc_read(struct seq_file* m, void* v)
{
	FH_MSG("EN: %s",__func__);
	
	seq_printf(m, "DRAMC: %dMHz\r\n",g_curr_dramc);
	seq_printf(m, "mt_get_emi_freq(): %dHz\r\n",mt_get_emi_freq());	
	seq_printf(m, "get_ddr_type(): %d\r\n",get_ddr_type());	
	seq_printf(m, "rank: 0x%x\r\n",(DRV_Reg32(EMI_CONA) & 0x20000));	
	seq_printf(m, "infra: 0x%x\r\n",(slp_will_infra_pdn()));	
	seq_printf(m, "g_fh_rank0_pa: %p\r\n",g_fh_rank0_pa);	
	seq_printf(m, "g_fh_rank0_va: %p\r\n",g_fh_rank0_va);	
	seq_printf(m, "g_fh_rank1_pa: %p\r\n",g_fh_rank1_pa);	
	seq_printf(m, "g_fh_rank1_va: %p\r\n",g_fh_rank1_va);	
  return 0;		
}


static int freqhopping_dramc_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len = 0, freq = 0;
	char dramc[32];

	FH_MSG("EN: proc");

	len = (count < (sizeof(dramc) - 1)) ? count : (sizeof(dramc) - 1);

	if (copy_from_user(dramc, buffer, len))
	{
		FH_MSG("copy_from_user fail!");
		return 1;
	}
	
	dramc[len] = '\0';
   
	if (sscanf(dramc, "%d", &freq) == 1)
	{
		if( (freq == 266) || (freq == 200)){
			FH_MSG("dramc:%d ", freq);
			(freq==266) ? mt_fh_hal_l2h_mempll() : mt_fh_hal_h2l_mempll();
		}
		else if(freq == 293){
			mt_fh_hal_dram_overclock(293);
		}
		else{
			FH_MSG("must be 200/266/293!");
		}

#if 0 
		if(freq == 266){
			FH_MSG("==> %d",mt_fh_hal_dram_overclock(266));
		}
		else if(freq == 293){
			FH_MSG("==> %d",mt_fh_hal_dram_overclock(293));
		}
		else if(freq == LOW_DRAMC){
			FH_MSG("==> %d",mt_fh_hal_dram_overclock(208));
		}
#endif

		return count;
	}
	else
	{
		FH_MSG("  bad argument!!");
	}

	return -EINVAL;
}


//static int freqhopping_dumpregs_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
static int freqhopping_dumpregs_proc_read(struct seq_file* m, void* v)
{
	int 	i=0;

	FH_MSG("EN: %s",__func__);

	seq_printf(m, "FHDMA_CFG:\r\n");

	seq_printf(m, "0x%08x 0x%08x 0x%08x 0x%08x\r\n",
		DRV_Reg32(REG_FHDMA_CFG+(i*0x10)),
		DRV_Reg32(REG_FHDMA_2G1BASE+(i*0x10)),
		DRV_Reg32(REG_FHDMA_2G2BASE+(i*0x10)),
		DRV_Reg32(REG_FHDMA_INTMDBASE+(i*0x10)));

	seq_printf(m, "0x%08x 0x%08x 0x%08x 0x%08x\r\n",
		DRV_Reg32(REG_FHDMA_EXTMDBASE+(i*0x10)),
		DRV_Reg32(REG_FHDMA_BTBASE+(i*0x10)),
		DRV_Reg32(REG_FHDMA_WFBASE+(i*0x10)),
		DRV_Reg32(REG_FHDMA_FMBASE+(i*0x10)));

	seq_printf(m, "0x%08x 0x%08x 0x%08x 0x%08x\r\n",
		DRV_Reg32(REG_FHSRAM_CON+(i*0x10)),
		DRV_Reg32(REG_FHSRAM_WR+(i*0x10)),
		DRV_Reg32(REG_FHSRAM_RD+(i*0x10)),
		DRV_Reg32(REG_FHCTL_CFG+(i*0x10)));

	seq_printf(m, "0x%08x 0x%08x 0x%08x 0x%08x\r\n",
		DRV_Reg32(REG_FHCTL_2G1_CH+(i*0x10)),
		DRV_Reg32(REG_FHCTL_2G2_CH+(i*0x10)),
		DRV_Reg32(REG_FHCTL_INTMD_CH+(i*0x10)),
		DRV_Reg32(REG_FHCTL_EXTMD_CH+(i*0x10)));

	seq_printf(m, "0x%08x 0x%08x 0x%08x \r\n\r\n",
		DRV_Reg32(REG_FHCTL_BT_CH+(i*0x10)),
		DRV_Reg32(REG_FHCTL_WF_CH+(i*0x10)),
		DRV_Reg32(REG_FHCTL_FM_CH+(i*0x10)));


	seq_printf(m, "FHCTL0_CFG:\r\n");

	for(i=0;i<MT_FHPLL_MAX;i++) {
		seq_printf(m, "0x%08x 0x%08x 0x%08x 0x%08x\r\n",
			DRV_Reg32(REG_FHCTL0_CFG+(i*0x10)),
			DRV_Reg32(REG_FHCTL0_UPDNLMT+(i*0x10)),
			DRV_Reg32(REG_FHCTL0_DDS+(i*0x10)),
			DRV_Reg32(REG_FHCTL0_MON+(i*0x10)));
	}

	
	seq_printf(m, "\r\nPLL_HP_CON0:\r\n0x%08x\r\n",
		DRV_Reg32(PLL_HP_CON0));
		
		
	seq_printf(m, "\r\nPLL_CON0 :\r\nARM:0x%08x MAIN:0x%08x MSDC:0x%08x TV:0x%08x LVDS:0x%08x UNIV:0x%08x\r\n",
			DRV_Reg32(ARMPLL_CON0),	
			DRV_Reg32(MAINPLL_CON0),	
			DRV_Reg32(MSDCPLL_CON0),	
			DRV_Reg32(TVDPLL_CON0),
			DRV_Reg32(LVDSPLL_CON0),
			DRV_Reg32(UNIVPLL_CON0));

	seq_printf(m, "\r\nPLL_CON1 :\r\nARM:0x%08x MAIN:0x%08x MSDC:0x%08x TV:0x%08x LVDS:0x%08x UNIV:0x%08x\r\n",
			DRV_Reg32(ARMPLL_CON1),	
			DRV_Reg32(MAINPLL_CON1),	
			DRV_Reg32(MSDCPLL_CON1),	
			DRV_Reg32(TVDPLL_CON1),
			DRV_Reg32(LVDSPLL_CON1),
			DRV_Reg32(UNIVPLL_CON0 + 0x4));
		
	seq_printf(m, "\r\nPLL_CON2 :\r\nARM:0x%08x MAIN:0x%08x MSDC:0x%08x TV:0x%08x LVDS:0x%08x UNIV:0x%08x\r\n",
			DRV_Reg32(ARMPLL_CON2),	
			DRV_Reg32(MAINPLL_CON2),	
			DRV_Reg32(MSDCPLL_CON2),	
			DRV_Reg32(TVDPLL_CON2),
			DRV_Reg32(LVDSPLL_CON2),
			DRV_Reg32(UNIVPLL_CON0 + 0x8));


	seq_printf(m, "\r\nMEMPLL :\r\nMEMPLL9: 0x%08x MEMPLL10: 0x%08x MEMPLL11: 0x%08x MEMPLL12: 0x%08x\r\n",
			DRV_Reg32(DDRPHY_BASE+0x624),
			DRV_Reg32(DDRPHY_BASE+0x628),
			DRV_Reg32(DDRPHY_BASE+0x62C),
			DRV_Reg32(DDRPHY_BASE+0x630)); //TODO: Hard code for now...
		
	seq_printf(m, "\r\nCLK26CALI: 0x%08x\r\n",
			DRV_Reg32(CLK26CALI)); 
  return 0;		
}


#if MT_FH_CLK_GEN

//static int freqhopping_clkgen_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
static int freqhopping_clkgen_proc_read(struct seq_file* m, void* v)
{

	FH_MSG("EN: %s",__func__);
	
	if(g_curr_clkgen > MT658X_FH_PLL_TOTAL_NUM ){
		seq_printf(m, "no clkgen output.\r\n");
	}
	else{
		seq_printf(m, "clkgen:%d\r\n",g_curr_clkgen);
	}

	seq_printf(m, "\r\nMBIST :\r\nMBIST_CFG_2: 0x%08x MBIST_CFG_6: 0x%08x MBIST_CFG_7: 0x%08x\r\n",
			DRV_Reg32(MBIST_CFG_2),
			DRV_Reg32(MBIST_CFG_6),
			DRV_Reg32(MBIST_CFG_7));
			
	seq_printf(m, "\r\nCLK_CFG_3: 0x%08x\r\n",
			DRV_Reg32(CLK_CFG_3));
			
	seq_printf(m, "\r\nTOP_CKMUXSEL: 0x%08x\r\n",
			DRV_Reg32(TOP_CKMUXSEL));

	seq_printf(m, "\r\nGPIO: 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
			DRV_Reg32(GPIO_BASE+0xC60),
			DRV_Reg32(GPIO_BASE+0xC70),
			DRV_Reg32(GPIO_BASE+0xCD0),
			DRV_Reg32(GPIO_BASE+0xD90));
	
	
	seq_printf(m, "\r\nDDRPHY_BASE :\r\n0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\r\n",
			DRV_Reg32(DDRPHY_BASE+0x600),
			DRV_Reg32(DDRPHY_BASE+0x604),
			DRV_Reg32(DDRPHY_BASE+0x608),
			DRV_Reg32(DDRPHY_BASE+0x60C),
			DRV_Reg32(DDRPHY_BASE+0x614),
			DRV_Reg32(DDRPHY_BASE+0x61C));
  return 0;		
}


static int freqhopping_clkgen_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len = 0, pll_id = 0;
	char clkgen[32];

	FH_MSG("EN: %s",__func__);

	len = (count < (sizeof(clkgen) - 1)) ? count : (sizeof(clkgen) - 1);

	//mt_set_gpio_mode(GPIO_CAMERA_CMPDN1_PIN, GPIO_CAMERA_CMPDN1_PIN_M_CLK);
	mt_set_gpio_mode(GPIO34, GPIO_MODE_03);

	if (copy_from_user(clkgen, buffer, len))
	{
		FH_MSG("copy_from_user fail!");
		return 1;
	}
	
	clkgen[len] = '\0';
   
	if (sscanf(clkgen, "%d", &pll_id) == 1)
	{
		if(pll_id == MT658X_FH_ARM_PLL){
			fh_write32(MBIST_CFG_2, 0x00000009); //divide by 9+1
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000001); //pll_pre_clk [don't care @ ARMPLL]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000001); //pll_clk_sel [don't care @ ARMPLL]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000401); //abist_clk_sel [0100: armpll_occ_mon]
			udelay(1000);

			fh_write32(CLK26CALI, 0x00000001); 
		}
		else if(pll_id == MT658X_FH_MAIN_PLL){
			fh_write32(MBIST_CFG_2, 0x00000009); //divide by 9+1
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000F0001); //pll_pre_clk [1111: AD_MAIN_H230P3M]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000F0001); //pll_clk_sel [0000: pll_pre_clk]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000F0F01); //abist_clk_sel [1111: pll_clk_out]
			udelay(1000);

			fh_write32(CLK26CALI, 0x00000001); 
		}
		else if(pll_id == MT658X_FH_MEM_PLL){
						
			fh_write32(DDRPHY_BASE+0x600, ( (DRV_Reg32(DDRPHY_BASE+0x600)) | 0x1<<5));
			
			
			fh_write32(DDRPHY_BASE+0x60C, ( (DRV_Reg32(DDRPHY_BASE+0x60C)) | 0x1<<21));
			fh_write32(DDRPHY_BASE+0x614, ( (DRV_Reg32(DDRPHY_BASE+0x614)) | 0x1<<21));
			fh_write32(DDRPHY_BASE+0x61C, ( (DRV_Reg32(DDRPHY_BASE+0x61C)) | 0x1<<21));

			fh_write32(DDRPHY_BASE+0x60C, ( (DRV_Reg32(DDRPHY_BASE+0x60C)) & ~0x7));
			fh_write32(DDRPHY_BASE+0x60C, ( (DRV_Reg32(DDRPHY_BASE+0x60C)) | 0x2 ));
			fh_write32(DDRPHY_BASE+0x614, ( (DRV_Reg32(DDRPHY_BASE+0x614)) & ~0x7));
			fh_write32(DDRPHY_BASE+0x614, ( (DRV_Reg32(DDRPHY_BASE+0x614)) | 0x2 ));
			fh_write32(DDRPHY_BASE+0x61C, ( (DRV_Reg32(DDRPHY_BASE+0x61C)) & ~0x7));
			fh_write32(DDRPHY_BASE+0x61C, ( (DRV_Reg32(DDRPHY_BASE+0x61C)) | 0x2));

			fh_write32(DDRPHY_BASE+0x604, ( (DRV_Reg32(DDRPHY_BASE+0x604)) | 0x1<<3));
			fh_write32(DDRPHY_BASE+0x604, ( (DRV_Reg32(DDRPHY_BASE+0x604)) | 0x1<<7));
			fh_write32(DDRPHY_BASE+0x604, ( (DRV_Reg32(DDRPHY_BASE+0x604)) | 0x1<<4));
			fh_write32(DDRPHY_BASE+0x604, ( (DRV_Reg32(DDRPHY_BASE+0x604)) | 0x1<<9));

#if 0			
			fh_write32(DDRPHY_BASE+0x608, ( (DRV_Reg32(DDRPHY_BASE+0x608)) & ~0x000E0000 ));
#endif 
			fh_write32(DDRPHY_BASE+0x608, ( (DRV_Reg32(DDRPHY_BASE+0x608)) | 0x00040000 ));

			fh_write32(DDRPHY_BASE+0x608, ( (DRV_Reg32(DDRPHY_BASE+0x608)) & ~0xF0000000 ));
			fh_write32(DDRPHY_BASE+0x608, ( (DRV_Reg32(DDRPHY_BASE+0x608)) | 0x80000000 ));

			//fh_write32(MBIST_CFG_2, 0x00000001); //divide by 1+1
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000001); //pll_pre_clk [don't care @ ARMPLL]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000001); //pll_clk_sel [don't care @ ARMPLL]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00000501); //abist_clk_sel [0101: AD_MEMPLL_MONCLK]
			udelay(1000);

			fh_write32(CLK26CALI, 0x00000000); 
		}
		else if(pll_id == MT658X_FH_MSDC_PLL){

			fh_write32(MBIST_CFG_2, 0x00000009); //divide by 9+1
			
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00080001); //pll_pre_clk [1000: AD_MSDCPLL_H208M]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00080001); //pll_clk_sel [0000: pll_pre_clk]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00080F01); //abist_clk_sel [1111: pll_clk_out]
			udelay(1000);
			
			fh_write32(CLK26CALI, 0x00000001); 
			
		}
		else if(pll_id == MT658X_FH_TVD_PLL){
			fh_write32(MBIST_CFG_2, 0x00000009); //divide by 9+1
			
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00090001); //pll_pre_clk [1001: AD_TVHDMI_H_CK]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00090001); //pll_clk_sel [0000: pll_pre_clk]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x00090F01); //abist_clk_sel [1111: pll_clk_out]
			udelay(1000);

			fh_write32(CLK26CALI, 0x00000001); 
		}
		else if(pll_id == MT658X_FH_LVDS_PLL){
			fh_write32(MBIST_CFG_2, 0x00000009); //divide by 9+1
			
			fh_write32(CLK_CFG_3, 0x00000001); //enable it
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000A0001); //pll_pre_clk [1010: AD_LVDS_H180M_CK]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000A0001); //pll_clk_sel [0000: pll_pre_clk]
			udelay(1000);
			fh_write32(CLK_CFG_3, 0x000A0F01); //abist_clk_sel [1111: pll_clk_out]
			udelay(1000);
			
			fh_write32(CLK26CALI, 0x00000001); 
		}
	}
	else
	{
		FH_MSG("  bad argument!!");
	}
	
	g_curr_clkgen = pll_id;

	return count;

	//return -EINVAL;
}

#endif //MT_FH_CLK_GEN

//TODO: __init void mt_freqhopping_init(void)
static void mt_fh_hal_init(void)
{
	int 		i;
	unsigned long 	flags;
	
	FH_MSG("EN: %s",__func__);
	
	if(g_initialize == 1){
		FH_MSG("already init!");
		return;
	}
		
	mt_dummy_read_init();
	
	//init hopping table for mempll 200<->266
	memset(g_mempll_fh_table, 0, sizeof(g_mempll_fh_table));

	
	for(i=0;i<MT_FHPLL_MAX;i++) {
		
		//TODO: use the g_fh_pll[] to init the FHCTL
		spin_lock_irqsave(&freqhopping_lock, flags);
				
		g_fh_pll[i].setting_id = 0;

		fh_write32(REG_FHCTL0_CFG+(i*0x10), 0x00000000); //No SSC and FH enabled
		fh_write32(REG_FHCTL0_UPDNLMT+(i*0x10), 0x00000000); //clear all the settings
		fh_write32(REG_FHCTL0_DDS+(i*0x10), 0x00000000); //clear all the settings
		
		//TODO: do we need this
		//fh_write32(REG_FHCTL0_MON+(i*0x10), 0x00000000); //clear all the settings
		
		spin_unlock_irqrestore(&freqhopping_lock, flags);
	}
	
	//TODO: update each PLL status (go through g_fh_pll)
	//TODO: wait for sophie's table & query the EMI clock
	//TODO: ask sophie to call this init function during her init call (mt_clkmgr_init() ??)
	//TODO: call __freqhopping_ctrl() to init each pll
	

	g_initialize = 1;
	
	enable_clock(MT_CG_PERI1_FHCTL, "FREQHOP") ;
}

static void mt_fh_hal_lock(unsigned long *flags)
{
	spin_lock_irqsave(&freqhopping_lock, *flags);	
}

static void mt_fh_hal_unlock(unsigned long *flags)
{
	spin_unlock_irqrestore(&freqhopping_lock, *flags);
}

static int mt_fh_hal_get_init(void)
{
	return(g_initialize);
}

//TODO: module_init(mt_freqhopping_init);
//TODO: module_exit(cpufreq_exit);

static struct mt_fh_hal_driver g_fh_hal_drv;

struct mt_fh_hal_driver *mt_get_fh_hal_drv(void)
{
	memset(&g_fh_hal_drv, 0, sizeof(g_fh_hal_drv));
	
	g_fh_hal_drv.fh_pll			= g_fh_pll;
	g_fh_hal_drv.fh_usrdef			= mt_ssc_fhpll_userdefined;
	g_fh_hal_drv.pll_cnt 			= MT658X_FH_PLL_TOTAL_NUM;
	g_fh_hal_drv.mempll 			= MT658X_FH_MEM_PLL;
	g_fh_hal_drv.lvdspll 			= MT658X_FH_LVDS_PLL;

	
	g_fh_hal_drv.mt_fh_hal_init 		=  mt_fh_hal_init;

#if MT_FH_CLK_GEN	
	g_fh_hal_drv.proc.clk_gen_read 		=  freqhopping_clkgen_proc_read;
	g_fh_hal_drv.proc.clk_gen_write 	=  freqhopping_clkgen_proc_write;
#endif 
	
	g_fh_hal_drv.proc.dramc_read 		=  freqhopping_dramc_proc_read;
	g_fh_hal_drv.proc.dramc_write 		=  freqhopping_dramc_proc_write;	
	g_fh_hal_drv.proc.dumpregs_read 	=  freqhopping_dumpregs_proc_read;


	g_fh_hal_drv.mt_fh_hal_ctrl		= __freqhopping_ctrl;
	g_fh_hal_drv.mt_fh_lock			= mt_fh_hal_lock;
	g_fh_hal_drv.mt_fh_unlock		= mt_fh_hal_unlock;
	g_fh_hal_drv.mt_fh_get_init		= mt_fh_hal_get_init;

	g_fh_hal_drv.mt_fh_popod_restore 	= mt_fh_hal_popod_restore;
	g_fh_hal_drv.mt_fh_popod_save		= mt_fh_hal_popod_save;

	g_fh_hal_drv.mt_l2h_mempll		= mt_fh_hal_l2h_mempll;
	g_fh_hal_drv.mt_h2l_mempll		= mt_fh_hal_h2l_mempll;
	g_fh_hal_drv.mt_dram_overclock		= mt_fh_hal_dram_overclock;
	g_fh_hal_drv.mt_get_dramc		= mt_fh_hal_get_dramc;
	g_fh_hal_drv.mt_fh_default_conf		= mt_fh_hal_default_conf;

	return (&g_fh_hal_drv);
}
