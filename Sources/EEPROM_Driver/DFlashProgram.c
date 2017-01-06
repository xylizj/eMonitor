/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2009, All Rights Reserved  *
*************************************************************************

*************************************************************************
*                                                                       *
*               EEPROM Emulation Driver for S12XS/S12P/S12HY            *
*                                                                       *
* FILE NAME     :  DFlashProgram.c                                      *
* DATE          :  02.24.2009                                           *
*                                                                       *
* AUTHOR        :  Infosys Team, Mysore                                 *
* E-mail        :  r56611@freescale.com                                 *
*                                                                       *
************************************************************************/


/******************************* CHANGES ********************************
 0.0.0a   10.09.2007      Sindhu R01      Initial Version
 0.1.0    06.04.2008      Sindhu R01      Removed the SSD Configuration
                                          structure. CallBack and
                                          commandArray are global variables.
                                          Pointer to FlashCommandSequence is
                                          removed.
 0.2.0    02.04.2009      Jiben Prakash,  Changed the function prototype.
                          Vikas Kothari.  Parameter source is changed to a
                                          pointer.
 0.2.1    02.24.2009      Sindhu R01      Removed the calculation of global
                                          address.Updated code to handle
                                          the pointer passed.
************************************************************************/

#include "SSD_Types.h"
#include "SSD_SGF18.h"
#include "SSD_SGF18_Internal.h"

/************************************************************************
*
*  Function Name    : DFlashProgram.c
*  Description      : Program data into D Flash
*
*  Arguments        : UINT16, UINT16, UINT16*
*  Return Value     : UINT16
*
*************************************************************************/

UINT16 DFlashProgram(UINT16 dest, UINT16 size, UINT16 *SSD_SGF18_NEAR source)
{
    UINT8  counter;         /* outer loop counter */
    UINT8  index;           /* command index */
    UINT8  loopIndex;       /* inner loop index */
    UINT16 commandValue;    /* store command value */
    UINT16 returnCode;      /* return code variable */
    UINT16 endAddress;      /* end address for the program operation */

    returnCode = SGF_OK;

    /* check the size alignment */
    if (0 != size % HCS12X_WORD_SIZE)
    {
        returnCode = SGF_ERR_SIZE;
    }
    else
    {
        /* calculate end address */
        endAddress = dest + size;

        /* check the range of the address */
        if(endAddress > DFLASH_BLOCK_END_ADDR)
        {
            returnCode = SGF_ERR_RANGE;
        }
        else
        {
            commandValue = (((UINT16)(DFLASH_PROGRAM)) << 8);
            while (dest < endAddress)
            {
                #if 0 //added by xyl
                #if (SGF18_SELECT == S12XS_SGF18)
                /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
                   address to logical address, to read the D-Flash */
                  returnCode = FSL_ReadDFlash(dest);
                #else
                /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
                  returnCode = READ_DFLASH16(dest);
                #endif
                #endif
        
                //if(READ16(source) != returnCode)
                {                     
                  /* preparing passing parameter for program once command */
                  /* 1st element for the FCCOB register */
                  commandArray[0] = (commandValue|GLOBAL_ADDRESS_DFLASH);
                  /* 2nd element fot the FCCOB register */
                  commandArray[1] = dest;
                  if(size > FLASH_PHRASE_SIZE)
                  {
                      loopIndex = COMMAND_INDEX4;
                  }
                  else
                  {
                      loopIndex = (UINT8)(size/HCS12X_WORD_SIZE);
                  }

                  /* preparing rest of the parameters for FCCOB register */
                  for (counter = 0; counter < loopIndex; counter++)
                  {
                      commandArray[counter + 2] = READ16(source);
                      dest += HCS12X_WORD_SIZE;
                      source ++;
                      size -= HCS12X_WORD_SIZE;
                  }

                  index = loopIndex + 1; /* value of FCCOBIX at command launch */

                  /* calling flash command sequence API to execute the command */
                  returnCode = FlashCommandSequence( index );

                  /* checking for the success of command execution */
                  if(SGF_OK != returnCode)
                  {
                      break;
                  }
                
                
                }
            }
        }
    }

    return(returnCode);
}
