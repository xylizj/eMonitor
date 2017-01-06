/*************************************************************************
 * (c) Copyright Freescale Semiconductor, Inc 2008, All Rights Reserved  *
 *************************************************************************

 ************************************************************************
 *                                                                      *
 *             EEPROM Emulation Driver for S12XS/S12P/S12HY             *                                                                      *
 * FILE NAME     :  SSD_SGF18_Internal.h                                *
 * DATE          :  08.18.2008                                          *
 *                                                                      *
 * AUTHOR        :  Infosys Team, Mysore                                *
 * E-mail        :  r56611@freescale.com                                *
 *                                                                      *
 ************************************************************************/

/******************************* CHANGES *******************************
 0.0.0a   10.09.2007      Sindhu R01      Initial Version
 0.0.1    07.16.2008      Sindhu R01      Removed some macros
 0.0.2    08.18.2008      Sindhu R01      Added macor FLASH_FCLKDIV_FDIVLCK
***********************************************************************/

#ifndef _SSD_SGF18_INTERNAL_H_
#define _SSD_SGF18_INTERNAL_H_

/*--------------- HCS12X Flash Module Memory Offset Map -----------------*/
/* Flash module base offset */
#define FLASH_REG_BASE                  0x0100

/* Flash clock divider register */
#define FLASH_FCLKDIV_OFFSET            (FLASH_REG_BASE + 0x0000)
#define FLASH_FCLKDIV_FDIVLD            0x80
/* For S12P part */
#if(SGF18_SELECT == S12P_SGF18)
  #define FLASH_FCLKDIV_FDIVLCK         0x40
#endif
/* For S12HY part */
#if(SGF18_SELECT == S12HY_SGF18)
  #define FLASH_FCLKDIV_FDIVLCK         0x40
#endif

/* Flash CCOB index register */
#define FLASH_FCCOBIX_OFFSET            (FLASH_REG_BASE + 0x0002)

/* Flash ECCR index register */
#define FLASH_FECCRIX_OFFSET            (FLASH_REG_BASE + 0x0003)

/* Flash configuration register */
#define FLASH_FCNFG_OFFSET              (FLASH_REG_BASE + 0x0004)
#define FLASH_FCNFG_CCIE                0x80
#define FLASH_FCNFG_IGNSF               0x10
#define FLASH_FCNFG_FDFD                0x02
#define FLASH_FCNFG_FSFD                0x01

/* Flash error configuration register */
#define FLASH_FERCNFG_OFFSET            (FLASH_REG_BASE + 0x0005)
#define FLASH_FERCNFG_DFDIE             0x02
#define FLASH_FERCNFG_SFDIE             0x01

/* Flash status register */
#define FLASH_FSTAT_OFFSET              (FLASH_REG_BASE + 0x0006)
#define FLASH_FSTAT_CCIF                0x80
#define FLASH_FSTAT_ACCERR              0x20
#define FLASH_FSTAT_FPVIOL              0x10
#define FLASH_FSTAT_MGSTAT1             0x02
#define FLASH_FSTAT_MGSTAT0             0x01

/* Flash error status register */
#define FLASH_FERSTAT_OFFSET            (FLASH_REG_BASE + 0x0007)
#define FLASH_FERSTAT_DFDIF             0x02
#define FLASH_FERSTAT_SFDIF             0x01

/* Flash common command object register */
#define FLASH_FCCOB_OFFSET              (FLASH_REG_BASE + 0x000A)

/* Flash common command object high register */
#define FLASH_FCCOBHI_OFFSET            (FLASH_REG_BASE + 0x000A)

/* Flash common command object low register */
#define FLASH_FCCOBLO_OFFSET            (FLASH_REG_BASE + 0x000B)

/************************************************************************/
               /* Macros releated to flash interrupts*/
/************************************************************************/
/* Offset of the S12X_INT module in the S12X register module */
#define S12X_INT_REGISTER_OFFSET        0x0120

/* If S12XS family */
#if (SGF18_SELECT == S12XS_SGF18)
  /* Offset of the Interrupt Vector Base Register(IVBR)in the S12X_INT module */
  #define INT_IVBR_OFFSET                 0x0001
#elif (SGF18_SELECT == S12P_SGF18) /* If S12P family */
  /* Offset of the Interrupt Vector Base Register(IVBR)in the S12X_INT module */
  #define INT_IVBR_OFFSET                 0x0000
#elif (SGF18_SELECT == S12HY_SGF18) /* If S12HY family */
  /* Offset of the Interrupt Vector Base Register(IVBR)in the S12X_INT module */
  #define INT_IVBR_OFFSET                 0x0000
#endif

#define FLASH_INT_CMD_COMPLETE          0x80
#define FLASH_INT_DOUBLE_BIT_FAULT      0x02
#define FLASH_INT_SINGLE_BIT_FAULT      0x01

#define FLASH_SECTOR_ERASE              0x01
#define FLASH_PROGRAM                   0x02

/*------------- Flash hardware algorithm operation commands -------------*/
#define FLASH_SET_USER_MARGIN           0x0D
#define DFLASH_ERASE_VERIFY             0x10
#define DFLASH_PROGRAM                  0x11
#define DFLASH_ERASE_SECTOR             0x12
#define FLASH_ERASE_VERIFY_BLOCK        0x02

/* Flash margin read settings */
#define FLASH_MARGIN_NORMAL             0x0000
#define FLASH_USER_MARGIN_LEVEL1        0x0001
#define FLASH_USER_MARGIN_LEVEL0        0x0002

#endif  /* _SSD_SGF18_INTERNAL_H_ */