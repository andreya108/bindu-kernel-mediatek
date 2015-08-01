#ifndef _MT_GPIO_EXT_H_
#define _MT_GPIO_EXT_H_

#include <mach/mt_pmic_wrap.h>  
		
#define GPIOEXT_WR(addr, data)   pwrap_write((u32)addr, data)
#define GPIOEXT_RD(addr)         ({ \
		u32 ext_data; \
		(pwrap_read((u32)addr,&ext_data) != 0)?-1:ext_data;})
#define GPIOEXT_SET_BITS(BIT,REG)   (GPIOEXT_WR(REG, (u32)(BIT)))
#define GPIOEXT_CLR_BITS(BIT,REG)    ({ \
		u32 ext_data; \
		int ret; \
		ret = GPIOEXT_RD(REG);\
		ext_data = ret;\
		(ret < 0)?-1:(GPIOEXT_WR(REG,ext_data & ~((u32)(BIT))))})

#define MT_GPIO_EXT_START MT_GPIO_BASE_MAX
typedef enum GPIO_PIN_EXT
{    
	GPIO232 = MT_GPIO_EXT_START, 
	GPIO233, GPIO234, GPIO235, GPIO236, GPIO237, GPIO238, GPIO239,
	GPIO240, GPIO241, GPIO242, GPIO243, GPIO244, GPIO245, GPIO246, GPIO247,
	GPIO248, GPIO249, GPIO250, GPIO251, GPIO252, GPIO253, GPIO254, GPIO255,
	GPIO256, GPIO257, GPIO258, GPIO259, GPIO260, GPIO261, GPIO262, GPIO263,
	GPIO264, GPIO265, GPIO266, GPIO267, GPIO268, GPIO269, GPIO270, GPIO271,
	GPIO272, GPIO273, GPIO274, GPIO275, GPIO276, GPIO277, GPIO278, GPIO279,
	GPIO280,
	MT_GPIO_EXT_MAX
}GPIO_PIN_EXT;    

typedef enum GPIO_PIN_EXT1
{    
    GPIOEXT0 = MT_GPIO_EXT_START, 
	GPIOEXT1, GPIOEXT2, GPIOEXT3, GPIOEXT4, GPIOEXT5, GPIOEXT6, GPIOEXT7,	
    GPIOEXT8, GPIOEXT9, GPIOEXT10, GPIOEXT11, GPIOEXT12, GPIOEXT13, GPIOEXT14, GPIOEXT15,	
    GPIOEXT16, GPIOEXT17, GPIOEXT18, GPIOEXT19, GPIOEXT20, GPIOEXT21, GPIOEXT22, GPIOEXT23,	
    GPIOEXT24, GPIOEXT25, GPIOEXT26, GPIOEXT27, GPIOEXT28, GPIOEXT29, GPIOEXT30, GPIOEXT31,	
    GPIOEXT32, GPIOEXT33, GPIOEXT34, GPIOEXT35, GPIOEXT36, GPIOEXT37, GPIOEXT38, GPIOEXT39,	
    GPIOEXT40, GPIOEXT41, GPIOEXT42, GPIOEXT43, GPIOEXT44, GPIOEXT45, GPIOEXT46, GPIOEXT47,	
    GPIOEXT48,
	MT_GPIO_EXT1_MAX = MT_GPIO_EXT_START
}GPIO_PIN_EXT1;    
#define GPIO_EXTEND_START GPIOEXT0
#define MT_GPIO_MAX_PIN MT_GPIO_EXT_MAX

/*----------------------------------------------------------------------------*/
typedef struct {
    unsigned short val;        
    unsigned short set;
    unsigned short rst;
    unsigned short _align;
} EXT_VAL_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
    EXT_VAL_REGS    dir[4];            /*0x0000 ~ 0x001F: 32 bytes*/
    EXT_VAL_REGS    pullen[4];         /*0x0020 ~ 0x003F: 32 bytes*/
    EXT_VAL_REGS    pullsel[4];        /*0x0040 ~ 0x005F: 32 bytes*/
    EXT_VAL_REGS    dinv[4];           /*0x0060 ~ 0x007F: 32 bytes*/    
    EXT_VAL_REGS    dout[4];           /*0x0080 ~ 0x009F: 32 bytes*/
    EXT_VAL_REGS    din[4];            /*0x00A0 ~ 0x00BF: 32 bytes*/
    EXT_VAL_REGS    mode[10];          /*0x00C0 ~ 0x010F: 80 bytes*/ 
} GPIOEXT_REGS;

#endif //_MT_GPIO_EXT_H_