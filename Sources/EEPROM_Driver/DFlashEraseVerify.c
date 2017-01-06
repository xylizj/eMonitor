/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2008, All Rights Reserved  *
*************************************************************************

*************************************************************************
*                                                                       *
*               EEPROM Emulation Driver for S12XS/S12P/S12HY            *
*                                                                       *
* FILE NAME     :  DFlashEraseVerify.c                                  *
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
                                          Pointer to FlashCommandSequence is
                                          removed.
 0.1.1    02.23.2009      Sindhu R01      Removed the calculation of global
                                          address.
***********************************************************************/

#include "SSD_Types.h"
#include "SSD_SGF18.h"
#include "SSD_SGF18_Internal.h"

/****************************************************************************
*
*  Function Name    : DFlashEraseVerify
*  Description      : Perform erase verify operation on D Flash
*
*  Arguments        : UINT16, UINT16
*  Return Value     : UINT16
*
*****************************************************************************/

UINT16 DFlashEraseVerify(UINT16 destination, UINT16 size)
{
    UINT8  index;           /* command index */
    UINT16 commandValue;    /* store command value */
    UINT16 returnCode;      /* return code variable */
    UINT16 wordCount;       /* no. of words */


    returnCode = SGF_OK;

    /* checking size alignment (section 2 Bytes) and correctness of size */
    if((0 != (size % HCS12X_WORD_SIZE)) || (size > DFLASH_MAXIMUM_SIZE))
    {
        returnCode = SGF_ERR_SIZE;
    }
    /* checking size zero */
    else if(0 == size)
    {
        returnCode = SGF_OK;
    }
    else
    {
        /* checking for block erase verify */
        if((DFLASH_BLOCK_START_ADDR == destination) && \
           ((DFLASH_MAXIMUM_SIZE) == size))
        {
            /* preparing passing parameter for block erase verify */
            index = COMMAND_INDEX0;
            commandValue = (((UINT16)(FLASH_ERASE_VERIFY_BLOCK)) << 8);
        }
        else
        {
            /* preparing passing parameter for section erase verify */
            index = COMMAND_INDEX2;
            wordCount = (UINT16)(size / HCS12X_WORD_SIZE);

            commandValue = (((UINT16)(DFLASH_ERASE_VERIFY)) << 8);
            commandArray[1] = destination;
            commandArray[2] = (UINT16)wordCount;
        }

        commandArray[0] = (commandValue|GLOBAL_ADDRESS_DFLASH);

        /* calling flash command sequence function to execute the command */
        returnCode = FlashCommandSequence(index);

    }

    return(returnCode);
}
