/*******************************************************************************
* FILE NAME: ifi_default.h
*
* DESCRIPTION: 
*  This file contains important data definitions.              
*
*  DO NOT EDIT THIS FILE!
*******************************************************************************/

#ifndef __ifi_default_h_
#define __ifi_default_h_

/* TODO Replace this with a relative path. */
#include <vex_pic/ifi/picdefs.h>

#ifdef UNCHANGEABLE_DEFINITION_AREA

/*******************************************************************************
                             ALIAS DEFINITIONS
*******************************************************************************/
                           /* DO NOT CHANGE! */

#define   DATA_SIZE          30
#define   SPI_TRAILER_SIZE   2
#define   SPI_XFER_SIZE      DATA_SIZE + SPI_TRAILER_SIZE

#define   FALSE             0
#define   TRUE              !FALSE

#ifdef    _SIMULATOR
#define   RESET_VECTOR      0x000
#define   HIGH_INT_VECTOR   0x008
#define   LOW_INT_VECTOR    0x018
#else
#define   RESET_VECTOR      0x800
#define   HIGH_INT_VECTOR   0x808
#define   LOW_INT_VECTOR    0x818
#endif


/*******************************************************************************
                            VARIABLE DECLARATIONS
*******************************************************************************/
                            /* DO NOT CHANGE! */
/* This structure contains important system status information. */
typedef struct
{
  unsigned int  :6;
  unsigned int  autonomous:1;    /* Autonomous enabled = 1, disabled = 0 */
  unsigned int  disabled:1;      /* Robot disabled = 1, enabled = 0 */
} modebits;

/******************************************************************************/
                            /* DO NOT CHANGE! */
/* This structure allows you to address specific bits of a byte.*/
typedef struct
{
  unsigned int  bit0:1;
  unsigned int  bit1:1;
  unsigned int  bit2:1;
  unsigned int  bit3:1;
  unsigned int  bit4:1;
  unsigned int  bit5:1;
  unsigned int  bit6:1;
  unsigned int  bit7:1;
} bitid;

/******************************************************************************/
                            /* DO NOT CHANGE! */
/* This structure defines the contents of the data received from the Master
 * microprocessor. */
typedef struct  {     /* begin rx_data_record structure */
  unsigned char packet_num;
  union
  { 
    bitid bitselect;
    modebits mode;            /*rxdata.rc_mode_byte.mode.(autonomous|disabled)*/
    unsigned char allbits;    /*rxdata.rc_mode_byte.allbits*/
  } rc_mode_byte;
  union
  {
    bitid bitselect;          /*rxdata.rc_receiver_status_byte.bitselect.bit0*/
    unsigned char allbits;    /*rxdata.rc_receiver_status_byte.allbits*/
  } rc_receiver_status_byte;  
  unsigned char spare01,spare02,spare03; /*reserved - for future use*/ 

  unsigned char oi_analog01, oi_analog02, oi_analog03, oi_analog04;  /*rxdata.oi_analog01*/
  unsigned char oi_analog05, oi_analog06, oi_analog07, oi_analog08;         
  unsigned char oi_analog09, oi_analog10, oi_analog11, oi_analog12;
  unsigned char oi_analog13, oi_analog14, oi_analog15, oi_analog16;
  unsigned char reserve[9];  /*reserved - for future use*/
  unsigned char master_version; 
} rx_data_record;

typedef rx_data_record *rx_data_ptr;

/******************************************************************************/
                            /* DO NOT CHANGE! */
/* This structure defines the contents of the data transmitted to the master  
 * microprocessor. */
typedef struct  {     /*begin tx_data_record structure*/
  unsigned char spare01,spare02,spare03,spare04;  /*reserved - for future use*/ 

  unsigned char rc_pwm01, rc_pwm02, rc_pwm03, rc_pwm04;   /*txdata.rc_pwm01*/
  unsigned char rc_pwm05, rc_pwm06, rc_pwm07, rc_pwm08;
  unsigned char rc_pwm09, rc_pwm10, rc_pwm11, rc_pwm12;
  unsigned char rc_pwm13, rc_pwm14, rc_pwm15, rc_pwm16;

  unsigned char user_cmd;     /*reserved - for future use*/
  unsigned char cmd_byte1;    /*reserved - Don't use*/
  unsigned char pwm_mask;     /*<EDU> make sure you know how this works before changing*/
  unsigned char warning_code; /*reserved - Don't use*/
  unsigned char reserve[4];   /*reserved - for future use*/
  unsigned char error_code;   /*reserved - Don't use*/
  unsigned char packetnum;    /*reserved - Don't use*/
  unsigned char current_mode; /*reserved - Don't use*/
  unsigned char control;      /*reserved - Don't use*/
} tx_data_record;

typedef tx_data_record *tx_data_ptr;


/******************************************************************************/
                            /* DO NOT CHANGE! */
/* This structure defines some flags which are used by the system. */

typedef struct
{
  unsigned int  NEW_SPI_DATA:1;
  unsigned int  TX_UPDATED:1;
  unsigned int  FIRST_TIME:1;
  unsigned int  TX_BUFFSELECT:1;
  unsigned int  RX_BUFFSELECT:1;
  unsigned int  SPI_SEMAPHORE:1;
  unsigned int  :2;
} packed_struct;

/******************************************************************************/

extern tx_data_record txdata; 
extern rx_data_record rxdata; 
extern packed_struct statusflag;

#else
#error  *** Error - Invalid Default File!
#endif

/*******************************************************************************
                           FUNCTION PROTOTYPES
*******************************************************************************/

/* These routines reside in ifi_library.lib */
/*******************************************************************************
* FUNCTION NAME: InterruptVectorHigh
* PURPOSE:       This vector jumps to the appropriate high pri. interrupt handler.
* CALLED FROM:   High priority interrupt vector
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void InterruptHandlerHigh (void);

void Initialize_Registers (void);
/*******************************************************************************
* FUNCTION NAME: IFI_Initialization
* PURPOSE:       Configure registers and initializes the SPI RX/TX buffers.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void IFI_Initialization (void);

/*******************************************************************************
* FUNCTION NAME: User_Proc_Is_Ready
* PURPOSE:       This routine informs the master processor that all user 
*                initalization has been complete.  The call to this routine must 
*                always be the last line of User_Initialization.  
* CALLED FROM:   user_routines.c
* ARGUMENTS:     none
* RETURNS:       void 
*******************************************************************************/
void User_Proc_Is_Ready (void);

/*******************************************************************************
* FUNCTION NAME: Putdata
* PURPOSE:       Fill the transmit buffer with data to send out to the master 
*                microprocessor. This routine takes 23 us to complete.
* CALLED FROM:   user_routines(_fast).c
* ARGUMENTS:     
*     Argument       Type           IO   Description
*     --------       -----------    --   -----------
*          ptr       tx_data_ptr    I    pointer to the transmit buffer
* RETURNS:       void
*******************************************************************************/
void Putdata(tx_data_ptr ptr);

/*******************************************************************************
* FUNCTION NAME: Getdata
* PURPOSE:       Retrieve data from the SPI receive buffer sent by the master 
*                microprocessor.  This routine takes 14.8 us to complete.
* CALLED FROM:   user_routines(_fast).c
* ARGUMENTS:     
*     Argument       Type           IO   Description
*     --------       -----------    --   -----------
*          ptr       rx_data_ptr    I    pointer to the receive buffer
* RETURNS:       void
*******************************************************************************/
void Getdata(rx_data_ptr ptr);

/*******************************************************************************
* FUNCTION NAME: Setup_PWM_Output_Type
* PURPOSE:       
* CALLED FROM:   user_routines.c
* ARGUMENTS:     
*     Argument       Type           IO   Description
*     --------       -----------    --   -----------
*     pwmSpec1       int            I    pwm13 type definition (IFI_PWM or USER_CCP)
*     pwmSpec2       int            I    pwm14 type definition (IFI_PWM or USER_CCP)
*     pwmSpec3       int            I    pwm15 type definition (IFI_PWM or USER_CCP)
*     pwmSpec4       int            I    pwm16 type definition (IFI_PWM or USER_CCP)
* RETURNS:       void
*******************************************************************************/
void Setup_PWM_Output_Type(int pwmSpec1,int pwmSpec2,int pwmSpec3,int pwmSpec4);

#endif

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
