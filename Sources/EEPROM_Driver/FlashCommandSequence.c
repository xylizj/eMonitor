/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2008, All Rights Reserved  *
*************************************************************************

*************************************************************************
*                                                                       *
*               EEPROM Emulation Driver for S12XS/S12P/S12HY            *
*                                                                       *
* FILE NAME     :  FlashCommandSequence.c                               *
* DATE          :  06.04.2008                                           *
*                                                                       *
* AUTHOR        :  Infosys Team, Mysore                                 *
* E-mail        :  r56611@freescale.com                                 *
*                                                                       *
*************************************************************************/

/******************************* CHANGES *******************************
 0.0.0a   10.09.2007      Sindhu R01      Initial Version
 0.1.0    06.04.2008      Sindhu R01      Removed the SSD Configuration
                                          structure. CallBack and
                                          commandArray are global variables.
***********************************************************************/

#include "SSD_Types.h"
#include "SSD_SGF18.h"
#include "SSD_SGF18_Internal.h"

/***********************************************************************
*
*  Function Name    : FlashCommandSequence
*  Description      : Perform command write sequence for flash operation
*
*  Arguments        : UINT8
*  Return Value     : UINT16
*
************************************************************************/

UINT16 FlashCommandSequence(UINT8 index)
{
    UINT8  counter;          /* for loop counter variable */
    UINT8  registerValue;    /* store data read from flash register */
    UINT16 returnCode;       /* return code variable */

    returnCode = SGF_OK;

    /* check CCIF bit */
    while(FALSE == (REG_BIT_TEST((SSD_MCU_REGISTER_BASE + \
                   FLASH_FSTAT_OFFSET), FLASH_FSTAT_CCIF)))
    {
        /* wait till CCIF bit is set */
    }

    /* clear ACCERR and FPVIOL bits */
    REG_WRITE((SSD_MCU_REGISTER_BASE + FLASH_FSTAT_OFFSET), \
              (FLASH_FSTAT_ACCERR|FLASH_FSTAT_FPVIOL));

    /* load FCCOBIX & FCCOB register */
    for(counter=0; counter<=index; counter++)
    {
        REG_WRITE(SSD_MCU_REGISTER_BASE + FLASH_FCCOBIX_OFFSET, counter);
        REG_WRITE16(SSD_MCU_REGISTER_BASE + FLASH_FCCOB_OFFSET, \
                    commandArray[counter]);
    }

    /* clear CCIF bit */
    REG_WRITE(SSD_MCU_REGISTER_BASE + FLASH_FSTAT_OFFSET, FLASH_FSTAT_CCIF);

    /* check CCIF bit */
    while(FALSE == (REG_BIT_TEST((SSD_MCU_REGISTER_BASE + \
                   FLASH_FSTAT_OFFSET), FLASH_FSTAT_CCIF)))
    {
        /* wait till CCIF bit is set */
        /* If CallBack is enabled, then serve callback function */
        #if(CALLBACK_ENABLE == TRUE)
            CallBack();
        #endif
    }

    /* Check error bits */
    /*Get flash status register value */
    registerValue = REG_READ(SSD_MCU_REGISTER_BASE + FLASH_FSTAT_OFFSET);

    /* checking access error */
    if(0 != (registerValue & FLASH_FSTAT_ACCERR))
    {
        returnCode = SGF_ERR_ACCERR;
    }
    /* checking protection error */
    else if(0 != (registerValue & FLASH_FSTAT_FPVIOL))
    {
        returnCode = SGF_ERR_PVIOL;
    }
    /* checking MGSTAT1 error */
    else if(0 != (registerValue & FLASH_FSTAT_MGSTAT1))
    {
        returnCode = SGF_ERR_MGSTAT1;
    }
    /* checking MGSTAT0 non-correctable error */
    else if(0 != (registerValue & FLASH_FSTAT_MGSTAT0))
    {
        returnCode = SGF_ERR_MGSTAT0;
    }
    else
    {
        /* do nothing*/
    }

    return(returnCode);
}



