/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2008, All Rights Reserved  *
*************************************************************************

*************************************************************************
*                                                                       *
*               EEPROM Emulation Driver for S12XS/S12P/S12HY                  *
*                                                                       *
* FILE NAME     :  FlashSetUserMargin.c                                 *
* DATE          :  02.25.2009                                           *
*                                                                       *
* AUTHOR        :  Infosys Team, Mysore                                 *
* E-mail        :  r56611@freescale.com                                 *
*                                                                       *
************************************************************************/


/******************************* CHANGES ********************************
  0.0.0a   05.05.2008      Sindhu R01      Initial Version
  0.1.0    06.04.2008      Sindhu R01      Removed the SSD Configuration
                                           structure. CallBack and
                                           commandArray are global variables.
                                           Pointer to FlashCommandSequence is
                                           removed.
  0.2.0    02.25.2009      Sindhu R01      Removed the block address parameter
************************************************************************/

#include "SSD_Types.h"
#include "SSD_SGF18.h"
#include "SSD_SGF18_Internal.h"

/************************************************************************
*
*  Function Name    : FlashSetUserMargin.c
*  Description      : Set the user margin level for a given block
*
*  Arguments        : UINT16
*  Return Value     : UINT16
*
*************************************************************************/

UINT16 FlashSetUserMargin(UINT16 marginValue)
{
    UINT16 commandValue;    /* store command value */
    UINT16 returnCode;      /* return code variable */

    returnCode = SGF_OK;

    /* preparing passing parameters to set user margin level for given block */
    commandValue = (((UINT16)(FLASH_SET_USER_MARGIN)) << 8);
    commandArray[0] = (commandValue|GLOBAL_ADDRESS_DFLASH);
    commandArray[1] = marginValue;

    /* calling flash command sequence function to execute the command */
    returnCode = FlashCommandSequence(COMMAND_INDEX1);

    return(returnCode);
}

