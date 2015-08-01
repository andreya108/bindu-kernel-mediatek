/******************************************************************************
 * pmic_wrapper.c - Linux pmic_wrapper Driver
 *
 *
 * DESCRIPTION:
 *     This file provid the other drivers PMIC wrapper relative functions
 *
 ******************************************************************************/


#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <mach/mt_typedefs.h>
#include <linux/timer.h>
#include <mach/mt_pmic_wrap.h>
#include <linux/io.h>
#include "pwrap_hal.h"

#define PMIC_WRAP_DEVICE "pmic_wrap"

static struct mt_pmic_wrap_driver *mt_wrp;

static spinlock_t	wrp_lock = __SPIN_LOCK_UNLOCKED(lock);

//----------interral API ------------------------
static S32 _pwrap_init_dio( U32 dio_en );
static S32 _pwrap_init_cipher( void );
static S32 _pwrap_init_sidly( void );
static S32 _pwrap_init_reg_clock( U32 regck_sel );
static BOOL _pwrap_timeout_ns (U64 start_time_ns, U64 timeout_time_ns);
static U64 _pwrap_get_current_time(void);
static U64 _pwrap_time2ns (U64 time_us);
static S32 pwrap_read_nochk( U32  adr, U32 *rdata );
static S32 pwrap_write_nochk( U32  adr, U32  wdata );
static void pwrap_trace_wacs2(void);
static S32 _pwrap_wacs2_nochk( U32 write, U32 adr, U32 wdata, U32 *rdata );
//******************************************************************************
//--add log for pmic access-------------------------------------------------
//******************************************************************************
typedef enum
{
    PWRAP_________READ,
    PWRAP_________WRITE,
    PWRAP_________MAX
} PWRAP_ACTION_ENUM;

typedef struct
{
    U64 wacs_time;
    PWRAP_ACTION_ENUM operation;
    U32 result;//result=0:success
    U32 addr;
    U32 wdata;
    U32 rdata;
} PWRAP_DEBUG_DATA_T;//adr, U32  wdata, U32 *rdata

#define PWRAP_DEBUG_COUNT 100
static volatile PWRAP_DEBUG_DATA_T pwrap_debug_data[PWRAP_DEBUG_COUNT] = {{0}};
static volatile U32 pwrap_debug_index = 0;
void pwrap_trace(U64 wacs_time,U64 result,U32  write, U32 addr, U32 wdata, U32 rdata)
{
    U32 index;
    pwrap_debug_index++;
    pwrap_debug_index %= PWRAP_DEBUG_COUNT;
    index =pwrap_debug_index;
    if(write==0)
      pwrap_debug_data[index].operation = PWRAP_________READ;//read
    else
      pwrap_debug_data[index].operation = PWRAP_________WRITE;
    pwrap_debug_data[index].wacs_time = wacs_time;
    pwrap_debug_data[index].result = result;
    pwrap_debug_data[index].addr = addr;
    pwrap_debug_data[index].wdata = wdata;
    pwrap_debug_data[index].rdata = rdata;
}

/*-pwrap debug--------------------------------------------------------------------------*/
static void pwrap_dump_all_register(void)
{
  U32 i=0;
  U32 reg_addr=0;
  U32 reg_value=0;
  PWRAPREG("dump pwrap register\n");
  for(i=0;i<=89;i++)
  {
    reg_addr=(PMIC_WRAP_BASE+i*4);
    reg_value=WRAP_RD32(reg_addr);
    PWRAPREG("0x%x=0x%x\n",reg_addr,reg_value);
  }
  PWRAPREG("dump peri_pwrap register\n");
  for(i=0;i<=24;i++)
  {
    reg_addr=(PERI_PWRAP_BRIDGE_BASE+i*4);
    reg_value=WRAP_RD32(reg_addr);
    PWRAPREG("0x%x=0x%x\n",reg_addr,reg_value);
  }
  PWRAPREG("dump dewrap register\n");
  for(i=0;i<=25;i++)
  {
    reg_addr=(DEW_BASE+i*4);
    reg_value=pwrap_read_nochk(reg_addr,&reg_value);
    PWRAPREG("0x%x=0x%x\n",reg_addr,reg_value);
  }
}
static void pwrap_dump_ap_register(void)
{
  U32 i=0;
  U32 reg_addr=0;
  U32 reg_value=0;
  PWRAPREG("dump pwrap register\n");
  for(i=0;i<=89;i++)
  {
    reg_addr=(PMIC_WRAP_BASE+i*4);
    reg_value=WRAP_RD32(reg_addr);
    PWRAPREG("0x%x=0x%x\n",reg_addr,reg_value);
  }
  PWRAPREG("dump peri_pwrap register\n");
  for(i=0;i<=24;i++)
  {
    reg_addr=(PERI_PWRAP_BRIDGE_BASE+i*4);
    reg_value=WRAP_RD32(reg_addr);
    PWRAPREG("0x%x=0x%x\n",reg_addr,reg_value);
  }
  //PWRAPREG("elapse_time=%llx(ns)\n",elapse_time);
}

/******************************************************************************
 wrapper timeout
******************************************************************************/
#define PWRAP_TIMEOUT
#ifdef PWRAP_TIMEOUT
static U64 _pwrap_get_current_time(void)
{
  return sched_clock();   ///TODO: fix me
}
//U64 elapse_time=0;

static BOOL _pwrap_timeout_ns (U64 start_time_ns, U64 timeout_time_ns)
{
  U64 cur_time=0;
  U64 elapse_time=0;

  // get current tick
  cur_time = sched_clock();//ns
  elapse_time=cur_time-start_time_ns;

  // check if timeout
  if (timeout_time_ns <= elapse_time)
  {
    // timeout
    return TRUE;
  }

  return FALSE;
}
static U64 _pwrap_time2ns (U64 time_us)
{
  return time_us*1000;
}

#else
static U64 _pwrap_get_current_time(void)
{
  return 0;
}
static BOOL _pwrap_timeout_ns (U64 start_time_ns, U64 elapse_time)//,U64 timeout_ns)
{
  return FALSE;
}
static U64 _pwrap_time2ns (U64 time_us)
{
  return 0;
}

#endif
//#####################################################################
//define macro and inline function (for do while loop)
//#####################################################################
typedef U32 (*loop_condition_fp)(U32);//define a function pointer

static inline U32 wait_for_fsm_idle(U32 x)
{
  return (GET_WACS0_FSM( x ) != WACS_FSM_IDLE );
}
static inline U32 wait_for_fsm_vldclr(U32 x)
{
  return (GET_WACS0_FSM( x ) != WACS_FSM_WFVLDCLR);
}
static inline U32 wait_for_sync(U32 x)
{
  return (GET_SYNC_IDLE0(x) != WACS_SYNC_IDLE);
}
static inline U32 wait_for_idle_and_sync(U32 x)
{
  return ((GET_WACS0_FSM(x) != WACS_FSM_IDLE) || (GET_SYNC_IDLE0(x) != WACS_SYNC_IDLE)) ;
}
static inline U32 wait_for_wrap_idle(U32 x)
{
  return ((GET_WRAP_FSM(x) != 0x0) || (GET_WRAP_CH_DLE_RESTCNT(x) != 0x0));
}
static inline U32 wait_for_wrap_state_idle(U32 x)
{
  return ( GET_WRAP_AG_DLE_RESTCNT( x ) != 0 ) ;
}
static inline U32 wait_for_man_idle_and_noreq(U32 x)
{
  return ( (GET_MAN_REQ(x) != MAN_FSM_NO_REQ ) || (GET_MAN_FSM(x) != MAN_FSM_IDLE) );
}
static inline U32 wait_for_man_vldclr(U32 x)
{
  return  (GET_MAN_FSM( x ) != MAN_FSM_WFVLDCLR) ;
}
static inline U32 wait_for_cipher_ready(U32 x)
{
  return (x!=1) ;
}
static inline U32 wait_for_stdupd_idle(U32 x)
{
  return ( GET_STAUPD_FSM(x) != 0x0) ;
}

static inline U32 wait_for_state_ready_init(loop_condition_fp fp,U32 timeout_us,U32 wacs_register,U32 *read_reg)
{

  U64 start_time_ns=0, timeout_ns=0;
  U32 reg_rdata=0x0;
  //U32 rdata=0;
  start_time_ns = _pwrap_get_current_time();
  timeout_ns = _pwrap_time2ns(timeout_us);
  do
  {
    reg_rdata = WRAP_RD32(wacs_register);

    if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
    {
      PWRAPERR("timeout when waiting for idle\n");
      return E_PWR_TIMEOUT;
    }
  } while( fp(reg_rdata)); //IDLE State
  if(read_reg)
   *read_reg=reg_rdata;
  return 0;
}
static inline U32 wait_for_state_idle(loop_condition_fp fp,U32 timeout_us,U32 wacs_register,U32 wacs_vldclr_register,U32 *read_reg)
{

  U64 start_time_ns=0, timeout_ns=0;
  U32 reg_rdata;
  start_time_ns = _pwrap_get_current_time();
  timeout_ns = _pwrap_time2ns(timeout_us);
  do
  {
    reg_rdata = WRAP_RD32(wacs_register);
    if( GET_INIT_DONE0( reg_rdata ) != WACS_INIT_DONE)
    {
      PWRAPERR("initialization isn't finished \n");
      return E_PWR_NOT_INIT_DONE;
    }
    //if last read command timeout,clear vldclr bit
    //read command state machine:FSM_REQ-->wfdle-->WFVLDCLR;write:FSM_REQ-->idle
    switch ( GET_WACS0_FSM( reg_rdata ) )
    {
      case WACS_FSM_WFVLDCLR:
        WRAP_WR32(wacs_vldclr_register , 1);
        PWRAPERR("WACS_FSM = PMIC_WRAP_WACS_VLDCLR\n");
        break;
      case WACS_FSM_WFDLE:
        PWRAPERR("WACS_FSM = WACS_FSM_WFDLE\n");
        break;
      case WACS_FSM_REQ:
        PWRAPERR("WACS_FSM = WACS_FSM_REQ\n");
        break;
      default:
        break;
    }
    if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
    {
      PWRAPERR("timeout when waiting for idle\n");
      pwrap_dump_ap_register();
      pwrap_trace_wacs2();
      return E_PWR_WAIT_IDLE_TIMEOUT;
    }
  }while( fp(reg_rdata)); //IDLE State
  if(read_reg)
   *read_reg=reg_rdata;
  return 0;
}
static inline U32 wait_for_state_ready(loop_condition_fp fp,U32 timeout_us,U32 wacs_register,U32 *read_reg)
{

  U64 start_time_ns=0, timeout_ns=0;
  U32 reg_rdata;
  start_time_ns = _pwrap_get_current_time();
  timeout_ns = _pwrap_time2ns(timeout_us);
  do
  {
    reg_rdata = WRAP_RD32(wacs_register);

    if( GET_INIT_DONE0( reg_rdata ) != WACS_INIT_DONE)
    {
      PWRAPERR("initialization isn't finished \n");
      return E_PWR_NOT_INIT_DONE;
    }
    if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
    {
      PWRAPERR("timeout when waiting for idle\n");
      pwrap_dump_ap_register();
      pwrap_trace_wacs2();
      return E_PWR_WAIT_IDLE_TIMEOUT;
    }
  } while( fp(reg_rdata)); //IDLE State
  if(read_reg)
   *read_reg=reg_rdata;
  return 0;
}
/********************************************************************************************/
//extern API for PMIC driver, INT related control, this INT is for PMIC chip to AP(ROME)
/********************************************************************************************/
U32 mt_pmic_wrap_eint_status(void)
{
	return 0;
}

void mt_pmic_wrap_eint_clr(int offset)
{

}
//--------------------------------------------------------
//    Function : pwrap_wacs2()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
S32 pwrap_wacs2_hal( U32  write, U32  adr, U32  wdata, U32 *rdata )
{
  U64 wrap_access_time=0x0;
  U32 reg_rdata=0;
  U32 wacs_write=0;
  U32 wacs_adr=0;
  U32 wacs_cmd=0;
  U32 return_value=0;
  unsigned long flags=0;

  //PWRAPFUC();
  // Check argument validation
  if( (write & ~(0x1))    != 0)  return E_PWR_INVALID_RW;
  if( (adr   & ~(0xffff)) != 0)  return E_PWR_INVALID_ADDR;
  if( (wdata & ~(0xffff)) != 0)  return E_PWR_INVALID_WDAT;

  spin_lock_irqsave(&wrp_lock,flags);
  // Check IDLE & INIT_DONE in advance
  return_value=wait_for_state_idle(wait_for_fsm_idle,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,PMIC_WRAP_WACS2_VLDCLR,0);
  if(return_value!=0)
  {
    PWRAPERR("wait_for_fsm_idle fail,return_value=%d\n",return_value);
    goto FAIL;
  }
  wacs_write  = write << 31;
  wacs_adr    = (adr >> 1) << 16;
  wacs_cmd= wacs_write | wacs_adr | wdata;

  WRAP_WR32(PMIC_WRAP_WACS2_CMD,wacs_cmd);
  if( write == 0 )
  {
    if (NULL == rdata)
    {
      PWRAPERR("rdata is a NULL pointer\n");
      return_value= E_PWR_INVALID_ARG;
      goto FAIL;
    }
    return_value=wait_for_state_ready(wait_for_fsm_vldclr,TIMEOUT_READ,PMIC_WRAP_WACS2_RDATA,&reg_rdata);
    if(return_value!=0)
    {
      PWRAPERR("wait_for_fsm_vldclr fail,return_value=%d\n",return_value);
      return_value+=1;//E_PWR_NOT_INIT_DONE_READ or E_PWR_WAIT_IDLE_TIMEOUT_READ
      goto FAIL;
    }
    *rdata = GET_WACS0_RDATA( reg_rdata );
    WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR , 1);
  }
FAIL:
  spin_unlock_irqrestore(&wrp_lock,flags);
  if(return_value!=0)
  {
    PWRAPERR("pwrap_wacs2 fail,return_value=%d\n",return_value);
    PWRAPERR("timeout:BUG_ON here\n");    
    //BUG_ON(1);      
  }
  wrap_access_time=sched_clock();
  pwrap_trace(wrap_access_time,return_value,write, adr, wdata,(U32)rdata);
  return return_value;
}
//******************************************************************************
//--internal API for pwrap_init-------------------------------------------------
//******************************************************************************
#if 0
static void pwrap_enable_clk(void)
{
  //enable_clock(MT65XX_PDN_MM_SPI, "pmic_wrap");
  return;
}

static void pwrap_disable_clk(void)
{
  //disable_clock(MT65XX_PDN_MM_SPI, "pmic_wrap");
  return;
}
#endif
//--------------------------------------------------------
//    Function : _pwrap_wacs2_nochk()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------

static S32 pwrap_read_nochk( U32  adr, U32 *rdata )
{
  return _pwrap_wacs2_nochk( 0, adr,  0, rdata );
}

static S32 pwrap_write_nochk( U32  adr, U32  wdata )
{
  return _pwrap_wacs2_nochk( 1, adr,wdata,0 );
}

static S32 _pwrap_wacs2_nochk( U32 write, U32 adr, U32 wdata, U32 *rdata )
{
  U32 reg_rdata=0x0;
  U32 wacs_write=0x0;
  U32 wacs_adr=0x0;
  U32 wacs_cmd=0x0;
  U32 return_value=0x0;
  //PWRAPFUC();
  // Check argument validation
  if( (write & ~(0x1))    != 0)  return E_PWR_INVALID_RW;
  if( (adr   & ~(0xffff)) != 0)  return E_PWR_INVALID_ADDR;
  if( (wdata & ~(0xffff)) != 0)  return E_PWR_INVALID_WDAT;

  // Check IDLE
  return_value=wait_for_state_ready_init(wait_for_fsm_idle,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
  if(return_value!=0)
  {
    PWRAPERR("_pwrap_wacs2_nochk write command fail,return_value=%x\n", return_value);
    return return_value;
  }

  wacs_write  = write << 31;
  wacs_adr    = (adr >> 1) << 16;
  wacs_cmd = wacs_write | wacs_adr | wdata;
  WRAP_WR32(PMIC_WRAP_WACS2_CMD,wacs_cmd);

  if( write == 0 )
  {
    if (NULL == rdata)
      return E_PWR_INVALID_ARG;
    // wait for read data ready
    return_value=wait_for_state_ready_init(wait_for_fsm_vldclr,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,&reg_rdata);
    if(return_value!=0)
    {
      PWRAPERR("_pwrap_wacs2_nochk read fail,return_value=%x\n", return_value);
      return return_value;
    }
    *rdata = GET_WACS0_RDATA( reg_rdata );
    WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR , 1);
  }
  return 0;
}
//--------------------------------------------------------
//    Function : _pwrap_init_dio()
// Description :call it in pwrap_init,mustn't check init done
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_init_dio( U32 dio_en )
{
  U32 arb_en_backup=0x0;
  U32 rdata=0x0;
  U32 return_value=0;

  //PWRAPFUC();
  arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 0x8); // only WACS2
  pwrap_write_nochk(DEW_DIO_EN, dio_en);

  // Check IDLE & INIT_DONE in advance
  return_value=wait_for_state_ready_init(wait_for_idle_and_sync,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
  if(return_value!=0)
  {
    PWRAPERR("_pwrap_init_dio fail,return_value=%x\n", return_value);
    return return_value;
  }
  WRAP_WR32(PMIC_WRAP_DIO_EN , dio_en);
  // Read Test
  pwrap_read_nochk(DEW_READ_TEST,&rdata);
  if( rdata != DEFAULT_VALUE_READ_TEST )
  {
    PWRAPERR("[Dio_mode][Read Test] fail,dio_en = %x, READ_TEST rdata=%x, exp=0x5aa5\n", dio_en, rdata);
    return E_PWR_READ_TEST_FAIL;
  }
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , arb_en_backup);
  return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_init_cipher()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_init_cipher( void )
{
  U32 arb_en_backup=0;
  U32 rdata=0;
  U32 return_value=0;
  U32 start_time_ns=0, timeout_ns=0;
  //PWRAPFUC();
  arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);

  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 0x8); // only WACS0

  WRAP_WR32(PMIC_WRAP_CIPHER_SWRST , 1);
  WRAP_WR32(PMIC_WRAP_CIPHER_SWRST , 0);
  WRAP_WR32(PMIC_WRAP_CIPHER_KEY_SEL , 1);
  WRAP_WR32(PMIC_WRAP_CIPHER_IV_SEL  , 2);
  WRAP_WR32(PMIC_WRAP_CIPHER_LOAD    , 1);
  WRAP_WR32(PMIC_WRAP_CIPHER_START   , 1);

  //Config CIPHER @ PMIC
  pwrap_write_nochk(DEW_CIPHER_SWRST,   0x1);
  pwrap_write_nochk(DEW_CIPHER_SWRST,   0x0);
  pwrap_write_nochk(DEW_CIPHER_KEY_SEL, 0x1);
  pwrap_write_nochk(DEW_CIPHER_IV_SEL,  0x2);
  pwrap_write_nochk(DEW_CIPHER_LOAD,    0x1);
  pwrap_write_nochk(DEW_CIPHER_START,   0x1);

  //wait for cipher data ready@AP
  return_value=wait_for_state_ready_init(wait_for_cipher_ready,TIMEOUT_WAIT_IDLE,PMIC_WRAP_CIPHER_RDY,0);
  if(return_value!=0)
  {
    PWRAPERR("wait for cipher data ready@AP fail,return_value=%x\n", return_value);
    return return_value;
  }

  //wait for cipher data ready@PMIC
  start_time_ns = _pwrap_get_current_time();
  timeout_ns = _pwrap_time2ns(TIMEOUT_WAIT_IDLE);
  do
  {
    pwrap_read_nochk(DEW_CIPHER_RDY,&rdata);
    if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
    {
      PWRAPERR("wait for cipher data ready:timeout when waiting for idle\n");
      return E_PWR_WAIT_IDLE_TIMEOUT;
    }
  } while( rdata != 0x1 ); //cipher_ready

  pwrap_write_nochk(DEW_CIPHER_MODE, 0x1);
  //wait for cipher mode idle
  return_value=wait_for_state_ready_init(wait_for_idle_and_sync,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
  if(return_value!=0)
  {
    PWRAPERR("wait for cipher mode idle fail,return_value=%x\n", return_value);
    return return_value;
  }
  WRAP_WR32(PMIC_WRAP_CIPHER_MODE , 1);

  // Read Test
  pwrap_read_nochk(DEW_READ_TEST,&rdata);
  if( rdata != DEFAULT_VALUE_READ_TEST )
  {
    PWRAPERR("_pwrap_init_cipher,read test error,error code=%x, rdata=%x\n", 1, rdata);
    return E_PWR_READ_TEST_FAIL;
  }

  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , arb_en_backup);
  return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_init_sidly()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_init_sidly( void )
{
  U32 arb_en_backup=0;
  U32 rdata=0;
  U32 ind=0;
  U32 result=0;
  U32 result_faulty=0;
  //PWRAPFUC();
  arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 0x8); // only WACS2

  // Scan all SIDLY by Read Test
  result = 0;
  for( ind=0 ; ind<4 ; ind++)
  {
    WRAP_WR32(PMIC_WRAP_SIDLY , ind);
    pwrap_read_nochk(DEW_READ_TEST,&rdata);
    if( rdata == DEFAULT_VALUE_READ_TEST )
    {
      PWRAPLOG("_pwrap_init_sidly [Read Test] pass,SIDLY=%x rdata=%x\n", ind,rdata);
      result |= (0x1 << ind);
    }
    else
      PWRAPLOG("_pwrap_init_sidly [Read Test] fail,SIDLY=%x,rdata=%x\n", ind,rdata);
  }

  // Config SIDLY according to result
  switch( result )
  {
    // Only 1 pass, choose it
    case 0x1:
      WRAP_WR32(PMIC_WRAP_SIDLY , 0);
      break;
    case 0x2:
      WRAP_WR32(PMIC_WRAP_SIDLY , 1);
      break;
    case 0x4:
      WRAP_WR32(PMIC_WRAP_SIDLY , 2);
      break;
    case 0x8:
      WRAP_WR32(PMIC_WRAP_SIDLY , 3);
      break;

    // two pass, choose the one on SIDLY boundary
    case 0x3:
      WRAP_WR32(PMIC_WRAP_SIDLY , 0);
      break;
    case 0x6:
      WRAP_WR32(PMIC_WRAP_SIDLY , 1); //no boundary, choose smaller one
      break;
    case 0xc:
      WRAP_WR32(PMIC_WRAP_SIDLY , 3);
      break;

    // three pass, choose the middle one
    case 0x7:
      WRAP_WR32(PMIC_WRAP_SIDLY , 1);
      break;
    case 0xe:
      WRAP_WR32(PMIC_WRAP_SIDLY , 2);
      break;
    // four pass, choose the smaller middle one
    case 0xf:
      WRAP_WR32(PMIC_WRAP_SIDLY , 1);
      break;

    // pass range not continuous, should not happen
    default:
      WRAP_WR32(PMIC_WRAP_SIDLY , 0);
      result_faulty = 0x1;
      break;
  }

  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , arb_en_backup);
  if( result_faulty == 0 )
    return 0;
  else
  {
    PWRAPERR("error,_pwrap_init_sidly fail,result=%x\n",result);
    return result_faulty;
  }
}

//--------------------------------------------------------
//    Function : _pwrap_reset_spislv()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------

static S32 _pwrap_reset_spislv( void )
{
  U32 ret=0;
  U32 return_value=0;
  //PWRAPFUC();
  // This driver does not using _pwrap_switch_mux
  // because the remaining requests are expected to fail anyway

  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 0);
  WRAP_WR32(PMIC_WRAP_WRAP_EN , 0);
  WRAP_WR32(PMIC_WRAP_MUX_SEL , 1);
  WRAP_WR32(PMIC_WRAP_MAN_EN ,1);
  WRAP_WR32(PMIC_WRAP_DIO_EN , 0);

  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_CSL  << 8));
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8)); //to reset counter
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_CSH  << 8));
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));
  WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));

  return_value=wait_for_state_ready_init(wait_for_sync,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
  if(return_value!=0)
  {
    PWRAPERR("_pwrap_reset_spislv fail,return_value=%x\n", return_value);
    ret=E_PWR_TIMEOUT;
    goto timeout;
  }

  WRAP_WR32(PMIC_WRAP_MAN_EN , 0);
  WRAP_WR32(PMIC_WRAP_MUX_SEL , 0);

timeout:
  WRAP_WR32(PMIC_WRAP_MAN_EN , 0);
  WRAP_WR32(PMIC_WRAP_MUX_SEL , 0);
  return ret;
}

static S32 _pwrap_init_reg_clock( U32 regck_sel )
{
  U32 wdata=0;
  U32 rdata=0;

  // Set reg clk freq
  pwrap_read_nochk(PMIC_TOP_CKCON2,&rdata);

  if( regck_sel == 1 )
    wdata = (rdata & (~(0x3<<10))) | (0x1<<10);
  else
    wdata = rdata & (~(0x3<<10));

  pwrap_write_nochk(PMIC_TOP_CKCON2, wdata);
  pwrap_read_nochk(PMIC_TOP_CKCON2, &rdata);
  if( rdata != wdata ) {
    PWRAPERR("_pwrap_init_reg_clock,PMIC_TOP_CKCON2 Write [15]=1 Fail, rdata=%x\n",rdata);
    return E_PWR_WRITE_TEST_FAIL;
  }

  // Config SPI Waveform according to reg clk
  if( regck_sel == 1 ) { //18MHz
    WRAP_WR32(PMIC_WRAP_CSHEXT, 0xc);
    WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE   , 0x4);
    WRAP_WR32(PMIC_WRAP_CSHEXT_READ    , 0xc);
    WRAP_WR32(PMIC_WRAP_CSLEXT_START   , 0x0);
    WRAP_WR32(PMIC_WRAP_CSLEXT_END     , 0x0);
  } else if( regck_sel == 2 ){ //36MHz
    WRAP_WR32(PMIC_WRAP_CSHEXT         , 0x4);
    WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE   , 0x0);
    WRAP_WR32(PMIC_WRAP_CSHEXT_READ    , 0x4);
    WRAP_WR32(PMIC_WRAP_CSLEXT_START   , 0x0);
    WRAP_WR32(PMIC_WRAP_CSLEXT_END     , 0x0);
  } else { //Safe mode
    WRAP_WR32(PMIC_WRAP_CSHEXT         , 0xf);
    WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE   , 0xf);
    WRAP_WR32(PMIC_WRAP_CSHEXT_READ    , 0xf);
    WRAP_WR32(PMIC_WRAP_CSLEXT_START   , 0xf);
    WRAP_WR32(PMIC_WRAP_CSLEXT_END     , 0xf);
  }

  return 0;
}

/*Interrupt handler function*/
static irqreturn_t mt_pmic_wrap_interrupt(int irqno, void *dev_id)
{
  U32 i=0;
  unsigned long flags=0;

  PWRAPFUC();
  PWRAPREG("dump pwrap register\n");
  spin_lock_irqsave(&wrp_lock,flags);
  //*-----------------------------------------------------------------------
  pwrap_dump_all_register();
  //*-----------------------------------------------------------------------
  //print the latest access of pmic
  PWRAPREG("the latest 20 access of pmic is following.\n");
  if(pwrap_debug_index>=20)
  {
    for(i=pwrap_debug_index-20;i<=pwrap_debug_index;i++)
      PWRAPREG("index=%d,time=%llx,operation=%x,addr=%x,wdata=%x,rdata=%x\n",i,
        pwrap_debug_data[i].wacs_time,pwrap_debug_data[i].operation,pwrap_debug_data[i].addr,
      pwrap_debug_data[i].wdata,pwrap_debug_data[i].rdata);
  }
  else //PWRAP_DEBUG_COUNT=100
  {
    for(i=PWRAP_DEBUG_COUNT+pwrap_debug_index-20;i<PWRAP_DEBUG_COUNT;i++)
      PWRAPREG("index=%d,time=%llx,operation=%x,addr=%x,wdata=%x,rdata=%x\n",i,
        pwrap_debug_data[i].wacs_time,pwrap_debug_data[i].operation,pwrap_debug_data[i].addr,
        pwrap_debug_data[i].wdata,pwrap_debug_data[i].rdata);
    for(i=0;i<=pwrap_debug_index;i++)
      PWRAPREG("index=%d,time=%llx,option=%x,addr=%x,wdata=%x,rdata=%x\n",i,
        pwrap_debug_data[i].wacs_time,pwrap_debug_data[i].operation,pwrap_debug_data[i].addr,
        pwrap_debug_data[i].wdata,pwrap_debug_data[i].rdata);
  }
  //raise the priority of WACS2 for AP
  WRAP_WR32(PMIC_WRAP_HARB_HPRIO,1<<3);

  //*-----------------------------------------------------------------------
  //clear interrupt flag
  WRAP_WR32(PMIC_WRAP_INT_CLR, 0xffffffff);
  BUG_ON(1);
  spin_unlock_irqrestore(&wrp_lock,flags);
  return IRQ_HANDLED;
}
#if 0
//--------------------------------------------------------
//    Function : _pwrap_AlignCRC()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static U32 _pwrap_AlignCRC( void )
{
  U32 arb_en_backup=0;
  U32 staupd_prd_backup=0;
  U32 return_value=0;
  //Backup Configuration & Set New Ones
  arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , 0x8); // only WACS2
  staupd_prd_backup = WRAP_RD32(PMIC_WRAP_STAUPD_PRD);
  WRAP_WR32(PMIC_WRAP_STAUPD_PRD , 0); //disable STAUPD

  // reset CRC
  return_value=pwrap_write(DEW_CRC_EN, 0);
  //PWRAPLOG("reset CR return_value=%d\n",return_value);
  WRAP_WR32(PMIC_WRAP_CRC_EN , 0);

  //Wait for FSM to be IDLE
  return_value=wait_for_state_ready_init(wait_for_wrap_state_idle,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WRAP_STA,0);
  if(return_value!=0)
  {
    PWRAPERR("AlignCRC wait for DLE fail=%d\n",return_value);
    return return_value;
  }

  // Enable CRC
  return_value=pwrap_write(DEW_CRC_EN, 1);
  //PWRAPLOG("Enable CRC return_value=%d\n",return_value);
  WRAP_WR32(PMIC_WRAP_CRC_EN , 1);

  //restore Configuration
  WRAP_WR32(PMIC_WRAP_STAUPD_PRD , staupd_prd_backup);
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , arb_en_backup);
  return 0;
}
#endif
/*
*pmic_wrap init,init wrap interface
*
*/
S32 pwrap_init ( void )
{
  S32 sub_return=0;
  S32 sub_return1=0;
  U32 rdata=0x0;
  //U32 timeout=0;
  PWRAPFUC();
  //###############################
  // Reset related modules
  // PMIC_WRAP, PERI_PWRAP_BRIDGE, PWRAP_SPICTL
  // Subject to project
  //###############################
  WRAP_SET_BIT(0x80,INFRA_GLOBALCON_RST0);
  WRAP_SET_BIT(0x04,PERI_GLOBALCON_RST1);
  rdata=WRAP_RD32(WDT_SWSYSRST);
  WRAP_WR32(WDT_SWSYSRST,(0x88000000 | ((rdata | (0x1 << 11)) & 0x00ffffff )));

  WRAP_CLR_BIT(0x80,INFRA_GLOBALCON_RST0);
  WRAP_CLR_BIT(0x04,PERI_GLOBALCON_RST1);
  rdata=WRAP_RD32(WDT_SWSYSRST);
  WRAP_WR32(WDT_SWSYSRST,(0x88000000 | ((rdata & (~(0x1 << 11))) & 0x00ffffff )));

  //###############################
  //TBD: Set SPI_CK freq = 66MHz
  //###############################
  //WRAP_WR32(CLK_CFG_8, 5);
  rdata = WRAP_RD32(CLK_CFG_8);
  WRAP_WR32(CLK_CFG_8, (rdata & ~0x7) | 0x5);

  //###############################
  //Enable DCM
  //###############################
   WRAP_WR32(PMIC_WRAP_DCM_EN , 1);
   WRAP_WR32(PMIC_WRAP_DCM_DBC_PRD ,0);

  //###############################
  //Reset SPISLV
  //###############################
  sub_return=_pwrap_reset_spislv();
  if( sub_return != 0 )
  {
    PWRAPERR("error,_pwrap_reset_spislv fail,sub_return=%x\n",sub_return);
    return E_PWR_INIT_RESET_SPI;
  }
  //###############################
  // Enable WACS2
  //###############################
  WRAP_WR32(PMIC_WRAP_WRAP_EN,1);//enable wrap
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,8); //Only WACS2
  WRAP_WR32(PMIC_WRAP_WACS2_EN,1);

  //###############################
  //TBD: Set SPI_CK freq = 66MHz
  //###############################
  //WRAP_WR32(CLK_CFG_8, 5);
  rdata = WRAP_RD32(CLK_CFG_8);
  WRAP_WR32(CLK_CFG_8, (rdata & ~0x7) | 0x5);

  //###############################
  // SIDLY setting
  //###############################
  sub_return = _pwrap_init_sidly();
  //sub_return = 0; //TBD
  if( sub_return != 0 )
  {
    PWRAPERR("error,_pwrap_init_sidly fail,sub_return=%x\n",sub_return);
    return E_PWR_INIT_SIDLY;
  }
  //###############################
  // SPI Waveform Configuration
  //###############################
  sub_return = _pwrap_init_reg_clock(2); //0:safe mode, 1:18MHz, 2:36MHz //2
  if( sub_return != 0)  {
    PWRAPERR("error,_pwrap_init_reg_clock fail,sub_return=%x\n",sub_return);
    return E_PWR_INIT_REG_CLOCK;
  }

  //###############################
  // Enable PMIC
  // (May not be necessary, depending on S/W partition)
  //###############################
  sub_return= pwrap_write_nochk(PMIC_WRP_CKPDN,   0);//set dewrap clock bit
  sub_return1=pwrap_write_nochk(PMIC_WRP_RST_CON, 0);//clear dewrap reset bit
  if(( sub_return != 0 )||( sub_return1 != 0 ))
  {
    PWRAPERR("Enable PMIC fail, sub_return=%x sub_return1=%x\n", sub_return,sub_return1);
    return E_PWR_INIT_ENABLE_PMIC;
  }
  //###############################
  // Enable DIO mode
  //###############################
  sub_return = _pwrap_init_dio(1);
  if( sub_return != 0 )
  {
    PWRAPERR("_pwrap_init_dio test error,error code=%x, sub_return=%x\n", 0x11, sub_return);
    return E_PWR_INIT_DIO;
  }

  //###############################
  // Enable Encryption
  //###############################
  sub_return = _pwrap_init_cipher();
  if( sub_return != 0 )
  {
    PWRAPERR("Enable Encryption fail, return=%x\n", sub_return);
    return E_PWR_INIT_CIPHER;
  }

  //###############################
  // Write test using WACS2
  //###############################
  sub_return = pwrap_write_nochk(DEW_WRITE_TEST, WRITE_TEST_VALUE);
  sub_return1 = pwrap_read_nochk(DEW_WRITE_TEST,&rdata);
  if(( rdata != WRITE_TEST_VALUE )||( sub_return != 0 )||( sub_return1 != 0 ))
  {
    PWRAPERR("write test error,rdata=0x%x,exp=0xa55a,sub_return=0x%x,sub_return1=0x%x\n", rdata,sub_return,sub_return1);
    return E_PWR_INIT_WRITE_TEST;
  }

  //###############################
  // Signature Checking - Using Write Test Register
  // should be the last to modify WRITE_TEST
  //###############################
  #if 0 //old mode
  _pwrap_wacs2_nochk(1, DEW_WRITE_TEST, 0x5678, &rdata);
  WRAP_WR32(PMIC_WRAP_SIG_ADR,DEW_WRITE_TEST);
  WRAP_WR32(PMIC_WRAP_SIG_VALUE,0x5678);
  WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x1);
  #endif

  //###############################
  // Signature Checking - Using CRC
  // should be the last to modify WRITE_TEST
  //###############################
  sub_return=pwrap_write_nochk(DEW_CRC_EN, 0x1);
  if( sub_return != 0 )
  {
    PWRAPERR("enable CRC fail,sub_return=%x\n", sub_return);
    return E_PWR_INIT_ENABLE_CRC;
  }
  WRAP_WR32(PMIC_WRAP_CRC_EN ,0x1);
  WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x0);
  WRAP_WR32(PMIC_WRAP_SIG_ADR , DEW_CRC_VAL);


  //###############################
  // PMIC_WRAP enables
  //###############################
  WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,0x1ff);
  WRAP_WR32(PMIC_WRAP_RRARB_EN ,0x7);
  WRAP_WR32(PMIC_WRAP_WACS0_EN,0x1);
  WRAP_WR32(PMIC_WRAP_WACS1_EN,0x1);
  WRAP_WR32(PMIC_WRAP_WACS2_EN,0x1);//already enabled
  WRAP_WR32(PMIC_WRAP_EVENT_IN_EN,0x1);
  WRAP_WR32(PMIC_WRAP_EVENT_DST_EN,0xffff);
  WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0x5);  //0x1:20us,for concurrence test,MP:0x5;  //100us
  WRAP_WR32(PMIC_WRAP_STAUPD_GRPEN,0xff);
  WRAP_WR32(PMIC_WRAP_WDT_UNIT,0xf);
  WRAP_WR32(PMIC_WRAP_WDT_SRC_EN,0xffffffff);
  WRAP_WR32(PMIC_WRAP_TIMER_EN,0x1);
  WRAP_WR32(PMIC_WRAP_INT_EN,0x7ffffffd); //except for [31] debug_int

  //###############################
  // PERI_PWRAP_BRIDGE enables
  //###############################
  WRAP_WR32(PERI_PWRAP_BRIDGE_IORD_ARB_EN, 0x7f);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WACS3_EN , 0x1);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WACS4_EN ,0x1);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WDT_UNIT , 0xf);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WDT_SRC_EN , 0xffff);
  WRAP_WR32(PERI_PWRAP_BRIDGE_TIMER_EN  , 0x1);
  WRAP_WR32(PERI_PWRAP_BRIDGE_INT_EN    , 0x7ff);

  //###############################
  // PMIC_DEWRAP enables
  //###############################
  sub_return  = pwrap_write_nochk(DEW_EVENT_OUT_EN, 0x1);
  sub_return1 = pwrap_write_nochk(DEW_EVENT_SRC_EN, 0xffff);
  if(( sub_return != 0 )||( sub_return1 != 0 ))
  {
    PWRAPERR("enable dewrap fail,sub_return=%d,sub_return1=%d\n", sub_return,sub_return1);
    return E_PWR_INIT_ENABLE_DEWRAP;
  }
  //###############################
  // Initialization Done
  //###############################
  WRAP_WR32(PMIC_WRAP_INIT_DONE2 , 0x1);

  //###############################
  //TBD: Should be configured by MD MCU
  //###############################
  #if 1  //def CONFIG_MTK_LDVT_PMIC_WRAP
    WRAP_WR32(PMIC_WRAP_INIT_DONE0 ,1);
    WRAP_WR32(PMIC_WRAP_INIT_DONE1 , 1);
    WRAP_WR32(PERI_PWRAP_BRIDGE_INIT_DONE3 , 1);
    WRAP_WR32(PERI_PWRAP_BRIDGE_INIT_DONE4 , 1);
  #endif
  return 0;
}
static void pwrap_trace_wacs2(void)
{
  U32 i=0;
  //print the latest access of pmic
  PWRAPREG("the latest 20 access of pmic is following.\n");
  if(pwrap_debug_index>=20)
  {
    for(i=pwrap_debug_index-20;i<=pwrap_debug_index;i++)
      PWRAPREG("index=%d,time=%llx,operation=%x,result=%d,addr=%x,wdata=%x,rdata=%x\n",i,
        pwrap_debug_data[i].wacs_time,pwrap_debug_data[i].operation,pwrap_debug_data[i].result,pwrap_debug_data[i].addr,
      pwrap_debug_data[i].wdata,pwrap_debug_data[i].rdata);
  }
  else //PWRAP_DEBUG_COUNT=100
  {
    for(i=PWRAP_DEBUG_COUNT+pwrap_debug_index-20;i<PWRAP_DEBUG_COUNT;i++)
      PWRAPREG("index=%d,time=%llx,operation=%x,result=%d,addr=%x,wdata=%x,rdata=%x\n",i,
        pwrap_debug_data[i].wacs_time,pwrap_debug_data[i].operation,pwrap_debug_data[i].result,pwrap_debug_data[i].addr,
        pwrap_debug_data[i].wdata,pwrap_debug_data[i].rdata);
    for(i=0;i<=pwrap_debug_index;i++)
      PWRAPREG("index=%d,time=%llx,operation=%x,result=%d,addr=%x,wdata=%x,rdata=%x\n",i,
        pwrap_debug_data[i].wacs_time,pwrap_debug_data[i].operation,pwrap_debug_data[i].result,pwrap_debug_data[i].addr,
        pwrap_debug_data[i].wdata,pwrap_debug_data[i].rdata);
  }
}
static void pwrap_read_reg_on_ap(U32 reg_addr)
{
  U32 reg_value=0;
  reg_value=WRAP_RD32(reg_addr);
  PWRAPREG("0x%x=0x%x\n",reg_addr,reg_value);
}

static void pwrap_write_reg_on_ap(U32 reg_addr,U32 reg_value)
{
  PWRAPREG("write 0x%x to register 0x%x\n",reg_value,reg_addr);
  WRAP_WR32(reg_addr,reg_value);
  reg_value=WRAP_RD32(reg_addr);
  PWRAPREG("the result:0x%x=0x%x\n",reg_addr,reg_value);
}

static void pwrap_read_reg_on_pmic(U32 reg_addr)
{
  U32 reg_value=0;
  U32 return_value=0;
  //PWRAPFUC();
  return_value=pwrap_read(reg_addr, &reg_value);
  PWRAPREG("0x%x=0x%x,return_value=%x\n",reg_addr,reg_value,return_value);
}

static void pwrap_write_reg_on_pmic(U32 reg_addr,U32 reg_value)
{
  U32 return_value=0;
  PWRAPREG("write 0x%x to register 0x%x\n",reg_value,reg_addr);
  return_value=pwrap_write(reg_addr, reg_value);
  return_value=pwrap_read(reg_addr, &reg_value);
  //PWRAPFUC();
  PWRAPREG("the result:0x%x=0x%x,return_value=%x\n",reg_addr,reg_value,return_value);
}
static U32 pwrap_read_test(void)
{
  U32 rdata=0;
  U32 return_value=0;
  // Read Test
  return_value=pwrap_read(DEW_READ_TEST,&rdata);
  if( rdata != DEFAULT_VALUE_READ_TEST )
  {
    PWRAPREG("Read Test fail,rdata=0x%x, exp=0x5aa5,return_value=0x%x\n", rdata,return_value);
    return E_PWR_READ_TEST_FAIL;
  }
  else
  {
    PWRAPREG("Read Test pass,return_value=%d\n",return_value);
    return 0;
}
}
static U32 pwrap_write_test(void)
{
  U32 rdata=0;
  U32 sub_return=0;
  U32 sub_return1=0;
  //###############################
  // Write test using WACS2
  //###############################
  sub_return = pwrap_write(DEW_WRITE_TEST, WRITE_TEST_VALUE);
  PWRAPREG("after pwrap_write_nochk\n");
  sub_return1 = pwrap_read(DEW_WRITE_TEST,&rdata);
  if(( rdata != WRITE_TEST_VALUE )||( sub_return != 0 )||( sub_return1 != 0 ))
  {
    PWRAPREG("write test error,rdata=0x%x,exp=0xa55a,sub_return=0x%x,sub_return1=0x%x\n", rdata,sub_return,sub_return1);
    return E_PWR_INIT_WRITE_TEST;
  }
  else
  {
    PWRAPREG("write Test pass\n");
    return 0;
  }
}
#define WRAP_ACCESS_TEST_REG DEW_CIPHER_IV1
static void pwrap_wacs2_para_test(void)
{
  U32 return_value=0;
  U32 result=0;
  U32 rdata=0;
  //test 1st parameter--------------------------------------------
  return_value=pwrap_wacs2(3, WRAP_ACCESS_TEST_REG, 0x1234, &rdata);
  if( return_value != 0 )
  {
    PWRAPREG("pwrap_wacs2_para_test 1st para,return_value=%x\n", return_value);
    result+=1;
  }
  //test 2nd parameter--------------------------------------------
  return_value=pwrap_wacs2(0, 0xffff+0x10, 0x1234, &rdata);
  if( return_value != 0 )
  {
    PWRAPREG("pwrap_wacs2_para_test 2nd para,return_value=%x\n", return_value);
    result+=1;
  }
  //test 3rd parameter--------------------------------------------
  return_value=pwrap_wacs2(0, WRAP_ACCESS_TEST_REG, 0xffff+0x10, &rdata);
  if( return_value != 0 )
  {
    PWRAPREG("pwrap_wacs2_para_test 3rd para,return_value=%x\n", return_value);
    result+=1;
  }
  //test 4th parameter--------------------------------------------
  return_value=pwrap_wacs2(0, WRAP_ACCESS_TEST_REG, 0x1234, 0);
  if( return_value != 0 )
  {
    PWRAPREG("pwrap_wacs2_para_test 4th para,return_value=%x\n", return_value);
    result+=1;
  }
  if(result==4)
    PWRAPREG("pwrap_wacs2_para_test pass\n");
  else
    PWRAPREG("pwrap_wacs2_para_test fail\n");
}
static void pwrap_ut(U32 ut_test)
{
  switch(ut_test)
  {
  case 1:
    pwrap_wacs2_para_test();
    break;
  case 2:
    //pwrap_wacs2_para_test();
    break;

  default:
    PWRAPREG ( "default test.\n" );
    break;
  }
}


/*---------------------------------------------------------------------------*/
static ssize_t mt_pwrap_show_hal(char *buf)
{
    PWRAPFUC();
    return snprintf(buf, PAGE_SIZE, "%s\n","no implement" );
}
/*---------------------------------------------------------------------------*/
static ssize_t mt_pwrap_store_hal(const char *buf, size_t count)
{
  U32 reg_value=0;
  U32 reg_addr=0;
  U32 return_value=0;
  U32 ut_test=0;
  if(!strncmp(buf, "-h", 2))
  {
    PWRAPREG("PWRAP debug: [-dump_reg][-trace_wacs2][-init][-rdap][-wrap][-rdpmic][-wrpmic][-readtest][-writetest]\n");
    PWRAPREG("PWRAP UT: [1][2]\n");
  }
  //--------------------------------------pwrap debug-------------------------------------------------------------
  else if(!strncmp(buf, "-dump_reg", 9))
  {
    pwrap_dump_all_register();
  }
  else if(!strncmp(buf, "-trace_wacs2", 12))
  {
    pwrap_trace_wacs2();
  }
  else if(!strncmp(buf, "-init", 5))
  {
    return_value=pwrap_init();
    if(return_value==0)
      PWRAPREG("pwrap_init pass,return_value=%d\n",return_value);
    else
      PWRAPREG("pwrap_init fail,return_value=%d\n",return_value);
  }
  else if (!strncmp(buf, "-rdap", 5) && (1 == sscanf(buf+5, "%x", &reg_addr)))
  {
    pwrap_read_reg_on_ap(reg_addr);
  }
  else if (!strncmp(buf, "-wrap", 5) && (2 == sscanf(buf+5, "%x %x", &reg_addr,&reg_value)))
  {
    pwrap_write_reg_on_ap(reg_addr,reg_value);
  }
  else if (!strncmp(buf, "-rdpmic", 7) && (1 == sscanf(buf+7, "%x", &reg_addr)))
  {
    pwrap_read_reg_on_pmic(reg_addr);
  }
  else if (!strncmp(buf, "-wrpmic", 7) && (2 == sscanf(buf+7, "%x %x", &reg_addr,&reg_value)))
  {
    pwrap_write_reg_on_pmic(reg_addr,reg_value);
  }
  else if(!strncmp(buf, "-readtest", 9))
  {
    pwrap_read_test();
  }
  else if(!strncmp(buf, "-writetest", 10))
  {
    pwrap_write_test();
  }
  //--------------------------------------pwrap UT-------------------------------------------------------------
  else if (!strncmp(buf, "-ut", 3) && (1 == sscanf(buf+3, "%d", &ut_test)))
  {
    pwrap_ut(ut_test);
  }else{
	PWRAPREG("wrong parameter\n");
  }
    return count;
}
/*-----suspend/resume for pmic_wrap-------------------------------------------*/

//infra power down while suspend,pmic_wrap will gate clock after suspend.
//so,need to init PPB when resume.
//only affect PWM and I2C

static int pwrap_suspend_hal(void)
{
  //PWRAPLOG("pwrap_suspend\n");
  return 0;
}
static void pwrap_resume_hal(void)
{
  //PWRAPLOG("pwrap_resume\n");
  //###############################
  // Reset related modules
  // PERI_PWRAP_BRIDGE
  // Subject to project
  //###############################
  WRAP_SET_BIT(0x04,PERI_GLOBALCON_RST1);
  WRAP_CLR_BIT(0x04,PERI_GLOBALCON_RST1);

  //###############################
  // PERI_PWRAP_BRIDGE enables
  //###############################
  WRAP_WR32(PERI_PWRAP_BRIDGE_IORD_ARB_EN , 0x7f);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WACS3_EN  , 0x1);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WACS4_EN  , 0x1);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WDT_UNIT  , 0xf);
  WRAP_WR32(PERI_PWRAP_BRIDGE_WDT_SRC_EN  , 0xffff);
  WRAP_WR32(PERI_PWRAP_BRIDGE_TIMER_EN  , 0x1);
  WRAP_WR32(PERI_PWRAP_BRIDGE_INT_EN    , 0x7ff);
}

/*---------------------------------------------------------------------------*/
#define VERSION     "$Revision$"
static int is_pwrap_init_done(void)
{
	int ret=0;
	int rdata = 0;
  //###############################
  //TBD: Set SPI_CK freq = 66MHz
  //###############################
  //WRAP_WR32(CLK_CFG_8, 5);
  	rdata = WRAP_RD32(CLK_CFG_8);
  	WRAP_WR32(CLK_CFG_8, (rdata & ~0x7) | 0x5);
  
	ret = WRAP_RD32(PMIC_WRAP_INIT_DONE2);
	if(ret!=0)
		return 0;

	ret = pwrap_init();
	if(ret!=0){
		PWRAPERR("init error (%d)\n", ret);
		pwrap_dump_all_register();
		return ret;
	}
	PWRAPLOG("init successfully done (%d)\n\n", ret);
	return ret;
}
static int __init mt_pwrap_init(void)
{
  U32 ret = 0;
  U32 rdata=0;
  PWRAPLOG("MT6589 PWRAP: version %s\n", VERSION);

  mt_wrp = get_mt_pmic_wrap_drv();	
  mt_wrp->store_hal = mt_pwrap_store_hal;
  mt_wrp->show_hal = mt_pwrap_show_hal;
  mt_wrp->wacs2_hal = pwrap_wacs2_hal;
  mt_wrp->suspend = pwrap_suspend_hal;
  mt_wrp->resume = pwrap_resume_hal;
  
	if(is_pwrap_init_done()==0){
		ret = request_irq(MT6589_PMIC_WRAP_IRQ_ID, mt_pmic_wrap_interrupt, IRQF_TRIGGER_HIGH, PMIC_WRAP_DEVICE, NULL);
		if (ret) {
			PWRAPERR("register IRQ failed (%d)\n", ret);
			return ret;
		}
	}else{
		PWRAPERR("not init (%d)\n", ret);
	}

	//PWRAPERR("not init (%x)\n", is_pwrap_init_done);
  //gating eint/i2c/pwm/kpd clock@PMIC
  pwrap_read_nochk(PMIC_WRP_CKPDN,&rdata);
  //disable clock,except kpd(bit[4] kpd and bit[6] 32k);
  ret= pwrap_write_nochk(PMIC_WRP_CKPDN,  rdata | 0x2F);//set dewrap clock bit
  return ret;

}
postcore_initcall(mt_pwrap_init);

