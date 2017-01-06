/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2008, All Rights Reserved  *
*************************************************************************

************************************************************************
*                                                                      *
*         EEPROM Emulation Flash Driver For S12P/S12XS/S12HY           *
*                                                                      *
* FILE NAME     :  FlashSetInterruptEnable.c                           *
* DATE          :  11.08.2008                                          *
*                                                                      *
* AUTHOR        :  Infosys Team, Mysore                                *
* E-mail        :  r56611@freescale.com                                *
*                                                                      *
************************************************************************/
/************************** CHANGES ***********************************
 0.0.0a   10.09.2008     Arvind Awasthi      Initial Version
 0.0.1    11.08.2008     Sindhu R01          Removed the return statement
***********************************************************************/

/* include the header files */
#include "SSD_Types.h"
#include "SSD_SGF18.h"
#include "SSD_SGF18_Internal.h"

/*********************************************************************
*
*  Function Name    : FlashSetInterruptEnable
*  Description      : Set the flash module interrupt enable state
*  Arguments        : UINT8
*  Return Value     : UINT16
*
**********************************************************************/

void FlashInterruptSet(UINT8 interruptState)
{
    /* store data read from flash register */
    UINT8  registerValue;

    /*Get flash configration register value */
    registerValue = REG_READ(SSD_MCU_REGISTER_BASE + FLASH_FCNFG_OFFSET);

    /* Set register value with desired interrupt state */
    registerValue &= ~(FLASH_FCNFG_CCIE);
    registerValue |= (interruptState & FLASH_FCNFG_CCIE);
    REG_WRITE(SSD_MCU_REGISTER_BASE + FLASH_FCNFG_OFFSET, registerValue);

    return;
}


/* end of file */






