/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2008, All Rights Reserved  *
*************************************************************************

*************************************************************************
*                                                                       *
*               EEPROM Emulation Driver for S12XS/S12P/S12HY            *
*                                                                       *
* FILE NAME     :  DFlashErase.c                                        *
* DATE          :  02.24.2009                                           *
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
                                          Pointer to FlashCommandSequence is
                                          removed.
 0.2.0    10.20.2008     Arvind Awasthi,  Updated the function to launch the
                         Sindhu R01       erase operation and return to the
                                          calling function. Changed the
                                          prototype
 0.2.1    11.08.2008      Sindhu R01      Updated to calling of the
                                          FlashInterruptSet
 0.2.2    02.24.2009      Sindhu R01      Removed the calculation of global
                                          address.
***********************************************************************/

#include "SSD_Types.h"
#include "SSD_SGF18.h"
#include "SSD_SGF18_Internal.h"
#include "MCU.h"

/*************************************************************************
*
*  Function Name    : DFlashErase
*  Description      : Perform erase operation on D Flash
*
*  Arguments        : UINT16
*  Return Value     : UINT16
*
**************************************************************************/
UINT16 DFlashErase(UINT16 destination)
{
    UINT16 commandValue;     /* store command value */
    UINT16 returnCode;       /* return code variable */
    UINT8  counter;          /* for loop counter variable */

    returnCode = SGF_OK;

    /* range checking */
    if((destination < DFLASH_BLOCK_START_ADDR) || \
       (destination >= DFLASH_BLOCK_END_ADDR) || \
       ((destination + DFLASH_SECTOR_SIZE) > DFLASH_BLOCK_END_ADDR))
    {
        returnCode = SGF_ERR_RANGE;
    }
    else
    {
        commandValue = (((UINT16)(DFLASH_ERASE_SECTOR)) << 8);

        /* preparing passing parameter for 256 Bytes sector erase */
        commandArray[0] = (commandValue|GLOBAL_ADDRESS_DFLASH);
        commandArray[1] = destination;

        /* clear ACCERR and FPVIOL bits */
        REG_WRITE((SSD_MCU_REGISTER_BASE + FLASH_FSTAT_OFFSET), \
          (FLASH_FSTAT_ACCERR|FLASH_FSTAT_FPVIOL));

        /* load FCCOBIX & FCCOB register */
        for(counter=0; counter<=COMMAND_INDEX1; counter++)
        {
           REG_WRITE(SSD_MCU_REGISTER_BASE + FLASH_FCCOBIX_OFFSET, counter);
           REG_WRITE16(SSD_MCU_REGISTER_BASE + FLASH_FCCOB_OFFSET, \
                commandArray[counter]);
        }

        /*Make the eraseStatusFlag as ERASE_STARTED*/
        eraseStatusFlag = BUSY;
        /* clear CCIF bit */
        REG_WRITE(SSD_MCU_REGISTER_BASE + FLASH_FSTAT_OFFSET, FLASH_FSTAT_CCIF);
        RTI_INT_DISABLE();//added by xyl 2014.10.18
        /* Enable Flash interrupts*/
        FlashInterruptSet(CCIE_INTERRUPT_ENABLE);
    }
    return(returnCode);
}