/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2008, All Rights Reserved  *
*************************************************************************

*************************************************************************
*                                                                       *
*               EEPROM Emulation Driver for S12XS/S12P/S12HY            *
*                                                                       *
* FILE NAME     :  FlashProgramVerify.c                                 *
* DATE          :  02.23.2009                                           *
*                                                                       *
* AUTHOR        :  Infosys Team, Mysore                                 *
* E-mail        :  r56611@freescale.com                                 *
*                                                                       *
************************************************************************/


/******************************* CHANGES ***********************************
 0.0.0a   10.09.2007      Sindhu R01      Initial Version
 0.1.0    06.04.2008      Sindhu R01      Removed the SSD Configuration
                                          structure. CallBack and
                                          commandArray are global variables.
                                          Pointer to FlashCommandSequence is
                                          removed. Added EPAGE manipulation for
                                          reading D-Flash locations.
0.2.0     02.23.2009      Sindhu R01      Changed the parameters passed to the
                                          function and their handling.
***************************************************************************/

#include "SSD_Types.h"
#include "SSD_SGF18.h"
#include "SSD_SGF18_Internal.h"

/***************************************************************************
*
*  Function Name    : FlashProgramVerify.c
*  Description      : Verify the programmed data in the flash
*
*  Arguments        : UINT16, UINT16, UINT16*
*  Return Value     : UINT16
*
***************************************************************************/
UINT16 FlashProgramVerify(UINT16 destination, UINT16 size, UINT16 *SSD_SGF18_NEAR source)
{
    UINT16  data;               /* data read from Flash address */
    UINT16  returnCode;         /* return code */
    UINT16  sourceData;         /* data read from source buffer */
    UINT16  endAddress;         /* the end address for operation */

    /* If CallBack is enabled, then declare counter for callback function calling */
    #if(CALLBACK_ENABLE == TRUE)
      UINT16  counter;
      counter = 0;
    #endif

    returnCode = SGF_OK;

    /* checking size alignment (word aligned 2 Bytes) */
    if(0 != (size % HCS12X_WORD_SIZE))
    {
        returnCode = SGF_ERR_SIZE;
    }

    /* checking address alignment */
    else if(0 != (destination % HCS12X_WORD_SIZE))
    {
        returnCode = SGF_ERR_ADDR;
    }
    else
    {
        /* Calculate the end address for the operation */
        endAddress = destination + size;

        /* D flash range checking */
        if((destination < DFLASH_BLOCK_START_ADDR) || \
           (destination >= DFLASH_BLOCK_END_ADDR) || \
           (endAddress > DFLASH_BLOCK_END_ADDR) || \
           (size > (DFLASH_BLOCK_END_ADDR - DFLASH_BLOCK_START_ADDR)))

        {
            returnCode = SGF_ERR_RANGE;
        }

    }

    if(SGF_OK == returnCode)
    {
        /* program verification operation */
        while (destination < endAddress)
        {

            /* read data at the destination */
            #if (SGF18_SELECT == S12XS_SGF18)
            /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
               address to logical address, to read the D-Flash */
              data = FSL_ReadDFlash(destination);
            #else
            /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
              data = READ_DFLASH16(destination);
            #endif

            /* read the data at the source */
            sourceData = READ16(source);

            if(data != sourceData)
            {
                /* set the return code to DATAMISMATCH */
                returnCode = SGF_ERR_DATAMISMATCH;
                break;
            }
            else
            {
                /* increment the destination for the next word verification */
                destination += HCS12X_WORD_SIZE;

                /* increment the source for the next word verification */
                source ++;
            }

            /* If CallBack is enabled, then serve callback function if counter reaches limitation */
            #if(CALLBACK_ENABLE == TRUE)
              /* Check if need to serve callback function */
              if( (++counter) >= FLASH_CALLBACK )
              {
                  CallBack();
                  /* Reset counter */
                  counter = 0;
              }
            #endif

        }
    }
    else
    {
        /* error occurred */
    }

    return(returnCode);
}


