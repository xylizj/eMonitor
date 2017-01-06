/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2008, All Rights Reserved  *
*************************************************************************

************************************************************************
*                                                                      *
*             EEPROM Emulation Driver for S12XS/S12P/S12HY             *
*                                                                      *
* FILE NAME     :  SSD_SGF18.h                                         *
* DATE          :  02.23.2009                                          *
*                                                                      *
* AUTHOR        :  Infosys Team, Mysore                                *
* E-mail        :  r56611@freescale.com                                *
*                                                                      *
************************************************************************/

/******************************* CHANGES *******************************
 0.0.0a   10.09.2007      Sindhu R01       Initial Version
 0.1.0    04.28.2008      Sindhu R01       Added macros relating to DEAD
                                           sectors, Added prototypes of
                                           function FlashSetUserMargin()
 0.1.1    06.04.2008      Sindhu R01       Removed the SSD Configuration
                                           structure. CallBack and
                                           commandArray global variables are externed.
                                           Added EPAGE manipulation macros for
                                           reading D-Flash locations.
                                           Added the macros to select S12XS or
                                           the S12P family
 0.2.0    10.24.2008      Arvind Awasthi,  Added macros and function declaration
                          Vikas Kothari,   of FlashInterruptSet
                          Sindhu R01       to make the EED driver interrupt based.
 0.2.1    11.08.2008      Sindhu R01       Changed the prototype of the
                                           FlashInterruptSet()
 0.3.0    02.23.2009      Sindhu R01       Added the macro GLOBAL_ADDRESS_DFLASH
                                           to identify the 22:16 bits of global
                                           D-Flash address.
                                           Changed the values of D-Flash start and
                                           end addresses for S12XS.
                                           Changed the prototypes of the APIs
***********************************************************************/

#ifndef _SSD_SGF18_H_
#define _SSD_SGF18_H_

#define CODE_WARRIOR                      0x00
#define COSMIC_BANKED                     0x01

#define S12XS_SGF18                       0x00
#define S12P_SGF18                        0x01
#define S12HY_SGF18                       0x02


/*--------------- SUBJECT TO CHANGES ACCORDING TO COMPILER -------------*/
/*For selecting Codewarrior compiler define COMPILER_SELECT as CODE_WARRIOR   */
/*For selecting Cosmic compiler define COMPILER_SELECT as COSMIC_BANKED */
#define COMPILER_SELECT                   CODE_WARRIOR

/*For selecting S12XS family define SGF18_SELECT as S12XS_SGF18   */
/*For selecting S12P family define SGF18_SELECT as S12P_SGF18   */
/*For selecting S12HY family define SGF18_SELECT as S12HY_SGF18   */
#define SGF18_SELECT                      S12HY_SGF18

/*-------------------- Callback function enable ------------------*/
//#define CALLBACK_ENABLE                 TRUE
#define CALLBACK_ENABLE                 FALSE//modified by xyl


#if (CODE_WARRIOR == COMPILER_SELECT)
  /* For CodeWarrior, the keyword is near or __near */
  #define SSD_SGF18_NEAR                  __near

 /* For CodeWarrior, the keyword is far or __far */
  #define SSD_SGF18_FAR                  __far
  #define SSD_SGF18_FUNC_FAR             __far
#else
  /* For Cosmic compiler, the keyword is @near  */
  #define SSD_SGF18_NEAR                  @near

  /* For Cosmic compiler, the keyword is @far  */
  #define SSD_SGF18_FAR                  @far @gpage
  #define SSD_SGF18_FUNC_FAR             @far
#endif

/* Configuration MACROS to be used */
/* Base address of MCU register block */
#define SSD_MCU_REGISTER_BASE           0x00000000

/* Bus clock. The unit is 10KHz */
//#define SSD_BUS_CLOCK                   800
#define SSD_BUS_CLOCK                   1600  //modified by xyl

/* Oscillator clock. The unit is 10KHz */
#define SSD_OSCILLATOR_CLOCK            1600

/*------------ Return Code Definition for HCS12X/S12P/S12HY-SSD -------------*/
#define SGF_OK                          0x0000
#define SGF_ERR_CLKSET                  0x0001
#define SGF_ERR_SIZE                    0x0002
#define SGF_ERR_RANGE                   0x0004
#define SGF_ERR_ACCERR                  0x0008
#define SGF_ERR_PVIOL                   0x0010
#define SGF_ERR_MGSTAT0                 0x0020
#define SGF_ERR_MGSTAT1                 0x0040
#define SGF_ERR_EPVIOLIF                0x0080
#define SGF_ERR_PROTMODE                0x0100
#define SGF_ERR_PROTSIZE                0x0200
#define SGF_ERR_DATAMISMATCH            0x0400
#define SGF_ERR_INVALIDCLK              0x0800
#define SGF_ERR_ADDR                    0x1000
#define SGF_ERR_SFDIF                   0x2000
#define SGF_ERR_DFDIF                   0x4000


/* Command Sequence Index Macros */
#define COMMAND_INDEX0                  0x00
#define COMMAND_INDEX1                  0x01
#define COMMAND_INDEX2                  0x02
#define COMMAND_INDEX3                  0x03
#define COMMAND_INDEX4                  0x04
#define COMMAND_INDEX5                  0x05


#if (SGF18_SELECT == S12XS_SGF18)/* If S12XS family */
  /* Address fetching mask */
  #define GLOBAL_ADDRESS_MASK             0x007F0000
  #define ADDRESS_OFFSET_MASK             0x0000FFFF

  /* Global address [22:16] to identify D-Flash block */
  #define GLOBAL_ADDRESS_DFLASH           0x0010

  /* D Flash data pertaining to S12XS device */

  /* Macros used for Reading D-Flash */
  #define DFLASH_LOWEST_START_PAGE        0x00
  #define DFLASH_START                    0x00100000
  #define DFLASH_PAGE_SIZE                0x0400
  #define DFLASH_PAGE_WINDOW_START        0x0800

  #define MMC2_EPAGE_OFFSET               0x0017

  /* D Flash block start address */
  #define DFLASH_BLOCK_START_ADDR         0x0000

  /* D Flash sector size */
  #define DFLASH_SECTOR_SIZE              0x0100  /* 256 Bytes size */

  /* D Flash maximum size */
  #define DFLASH_MAXIMUM_SIZE             0x2000  /* 8 KB */

  /* D Flash end address */
  #define DFLASH_BLOCK_END_ADDR           0x1FFF

  /* Read D-Flash Operation Macro(assumes the correct page value is loaded in EPAGE register */
  #define READ_DFLASH16(address)                 ((UINT16)(*(VUINT16 *SSD_SGF18_NEAR)(((address) & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START)))

#elif (SGF18_SELECT == S12P_SGF18) /* If S12P family */

  /* Address fetching mask */
  #define GLOBAL_ADDRESS_MASK             0x00030000
  #define ADDRESS_OFFSET_MASK             0x0000FFFF

  /* Global address [17:16] to identify D-Flash block */
  #define GLOBAL_ADDRESS_DFLASH           0x0000

  /* D Flash data pertaining to S12P device */

  /* D Flash block start address */
  #define DFLASH_BLOCK_START_ADDR         0x004400

  /* D Flash sector size */
  #define DFLASH_SECTOR_SIZE              0x0100  /* 256 Bytes size */

  /* D Flash maximum size */
  #define DFLASH_MAXIMUM_SIZE             0x00001000  /* 4 KB */

  /* D Flash end address */
  #define DFLASH_BLOCK_END_ADDR           0x0053FF

  #define DFLASH_PROGRAM_OFFSET           0x4000

  /* Read D-Flash Operation Macro */
  #define READ_DFLASH16(address)                 ((UINT16)(*(VUINT16 *SSD_SGF18_NEAR)(address-DFLASH_PROGRAM_OFFSET)))

#elif (SGF18_SELECT == S12HY_SGF18) /* If S12HY family */

  /* Address fetching mask */
  #define GLOBAL_ADDRESS_MASK             0x00030000
  #define ADDRESS_OFFSET_MASK             0x0000FFFF

  /* Global address [17:16] to identify D-Flash block */
  #define GLOBAL_ADDRESS_DFLASH           0x0000

  /* D Flash data pertaining to S12HY device */

  /* D Flash block start address */
  #define DFLASH_BLOCK_START_ADDR         0x004400

  /* D Flash sector size */
  #define DFLASH_SECTOR_SIZE              0x0100  /* 256 Bytes size */

  /* D Flash maximum size */
  #define DFLASH_MAXIMUM_SIZE             0x00001000  /* 4 KB */

  /* D Flash end address */
  #define DFLASH_BLOCK_END_ADDR           0x0053FF

  #define DFLASH_PROGRAM_OFFSET           0x4000

  /* Read D-Flash Operation Macro */
  #define READ_DFLASH16(address)                 ((UINT16)(*(VUINT16 *SSD_SGF18_NEAR)(address-DFLASH_PROGRAM_OFFSET)))

#endif

/* Maximum value for program & read once phrase index */
#define MAX_PHRASE_INDEX                0x07

/*-------------- Read/Write/Set/Clear Operation Macros ------------------*/
  #define REG_BIT_SET(address, mask)      (*(VUINT8 *SSD_SGF18_NEAR)(address) |= (mask))
  #define REG_BIT_CLEAR(address, mask)    (*(VUINT8 *SSD_SGF18_NEAR)(address) &= ~(mask))
  #define REG_BIT_TEST(address, mask)     (*(VUINT8 *SSD_SGF18_NEAR)(address) & (mask))
  #define REG_WRITE(address, value)       (*(VUINT8 *SSD_SGF18_NEAR)(address) = (value))
  #define REG_READ(address)               ((UINT8)(*(VUINT8 *SSD_SGF18_NEAR)(address)))
  #define REG_WRITE16(address, value)     (*(VUINT16 *SSD_SGF18_NEAR)(address) = (value))
  #define REG_READ16(address)             ((UINT16)(*(VUINT16 *SSD_SGF18_NEAR)(address)))

  #define WRITE8(address, value)          (*(VUINT8 *SSD_SGF18_NEAR)(address) = (value))
  #define READ8(address)                  ((UINT8)(*(VUINT8 *SSD_SGF18_NEAR)(address)))
  #define SET8(address, value)            (*(VUINT8 *SSD_SGF18_NEAR)(address) |= (value))
  #define CLEAR8(address, value)          (*(VUINT8 *SSD_SGF18_NEAR)(address) &= ~(value))
  #define TEST8(address, value)           (*(VUINT8 *SSD_SGF18_NEAR)(address) & (value))

  #define WRITE16(address, value)         (*(VUINT16 *SSD_SGF18_NEAR)(address) = (value))
  #define READ16(address)                 ((UINT16)(*(VUINT16 *SSD_SGF18_NEAR)(address)))
  #define SET16(address, value)           (*(VUINT16 *SSD_SGF18_NEAR)(address) |= (value))
  #define CLEAR16(address, value)         (*(VUINT16 *SSD_SGF18_NEAR)(address) &= ~(value))
  #define TEST16(address, value)          (*(VUINT16 *SSD_SGF18_NEAR)(address) & (value))

  #define WRITE32(address, value)         (*(VUINT32 *SSD_SGF18_NEAR)(address) = (value))
  #define READ32(address)                 ((UINT32)(*(VUINT32 *SSD_SGF18_NEAR)(address)))
  #define SET32(address, value)           (*(VUINT32 *SSD_SGF18_NEAR)(address) |= (value))
  #define CLEAR32(address, value)         (*(VUINT32 *SSD_SGF18_NEAR)(address) &= ~(value))
  #define TEST32(address, value)          (*(VUINT32 *SSD_SGF18_NEAR)(address) & (value))

/*-------------------- Macro definition for HCS12X ----------------------*/
#define HCS12X_WORD_SIZE                0x0002

/*--------------------- CallBack function period ------------------------*/
#define FLASH_CALLBACK               20   /* Used in Program Verify and Search Record */

/*-------------------- Callback function prototype ---------------------*/
/* If CallBack is enabled, then declare a callback function pointer type */
#if(CALLBACK_ENABLE == TRUE)
 typedef void (* PCALLBACK)(void);
#endif

/*--------------------Null Callback function defination ------------------*/
#define NULL_CALLBACK                   ((PCALLBACK)0xFFFFFFFF)

/* Phrase size */
#define FLASH_PHRASE_SIZE               0x0008  /* 8 bytes */

/* Setting CCIE interrupt enabling bits */
#define CCIE_INTERRUPT_ENABLE           0x80

/* Clearing CCIE interrupt enabling bits */
#define CCIE_INTERRUPT_DISABLE          0x00


/* enum to monitor states */
typedef enum
{
  IDLE =0,
  BUSY = 1,
  DONE = 2,
  FAIL = 3
}EE_StateType;

/* Global variables */
/* Flag to keep track of Erase State*/
extern volatile EE_StateType eraseStatusFlag;

extern UINT8 gJobToMain;

/* If CallBack is enabled, then declare a callback function pointer type */
#if(CALLBACK_ENABLE == TRUE)
  /* A function pointer to the CallBack function */
  extern PCALLBACK      CallBack; /* pointer to CallBack function */
#endif

/* A command array which contains the data to be written to the FCCOB register. */
extern UINT16 commandArray[6]; /* command sequence array */

/*---------------- Function Prototypes for Flash SSD --------------------*/

/* Flash initialization */
extern UINT16 FlashInit(void);

/* D flash program */
extern UINT16 DFlashProgram(UINT16 dest, UINT16 size, UINT16 *SSD_SGF18_NEAR source);

/* D flash erase */
extern UINT16 DFlashErase(UINT16 destination);

/* D flash erase verify */
extern UINT16 DFlashEraseVerify(UINT16 destination, UINT16 size);

/* Flash program verify */
extern UINT16 FlashProgramVerify(UINT16 destination, UINT16 size, UINT16 *SSD_SGF18_NEAR source);

/* Flash set user margin */
extern UINT16 FlashSetUserMargin(UINT16 marginValue);

/* Flash set interrupt */
extern void FlashInterruptSet(UINT8 interruptState);

/* D flash read */
/* If S12XS family */
#if (SGF18_SELECT == S12XS_SGF18)
  extern UINT16 FSL_ReadDFlash (UINT16 destination);
#endif


/* Internal function. Called by driver APIs only */
/* Flash command sequence */

extern UINT16 FlashCommandSequence(UINT8 index);

#endif /* _SSD_SGF18_H_ */


