/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2008, All Rights Reserved  *
*************************************************************************

*************************************************************************
*                                                                       *
*               EEPROM Emulation Driver for S12XS/S12P/S12HY            *
*                                                                       *
* FILE NAME     :  FlashInit.c                                          *
* DATE          :  08.18.2008                                           *
*                                                                       *
* AUTHOR        :  Infosys Team, Mysore                                 *
* E-mail        :  r56611@freescale.com                                 *
*                                                                       *
*************************************************************************/

/******************************* CHANGES *******************************
 0.0.0a   10.09.2007      Sindhu R01      Initial Version
 0.1.0    06.04.2008      Sindhu R01      Removed the SSD Configuration
                                          structure.
 0.2.0    08.18.2008      Sindhu R01      Computation of flash divider
                                          value changed
***********************************************************************/

#include "SSD_Types.h"
#include "SSD_SGF18.h"
#include "SSD_SGF18_Internal.h"

/*********************************************************************
*
*  Function Name    : FlashInit
*  Description      : Initialize the Flash memory
*
*  Arguments        : void
*  Return Value     : UNIT16
*
**********************************************************************/

UINT16 FlashInit (void)
{
    UINT8  flashDivider;    /* flash clock divider */
    UINT8  registerValue;   /* store data read from flash register */
    UINT16 clockValue;      /* clock in MHz */
    UINT16 returnCode;      /* return code variable */

    returnCode = SGF_OK;

    /* clear ACCERR & FPVIOL flag in flash status register */
    REG_WRITE((SSD_MCU_REGISTER_BASE + FLASH_FSTAT_OFFSET), \
              (FLASH_FSTAT_ACCERR | FLASH_FSTAT_FPVIOL));

    /* clear DFDIF,SFDIF & EPVIOLIF flag in flash error status register */
    REG_WRITE((SSD_MCU_REGISTER_BASE + FLASH_FERSTAT_OFFSET), \
              (FLASH_FERSTAT_DFDIF | FLASH_FERSTAT_SFDIF));

    /* Set the ignore single bit fault value, so that the single bit faults are ignored */
    REG_BIT_SET((SSD_MCU_REGISTER_BASE + FLASH_FCNFG_OFFSET), \
                     FLASH_FCNFG_IGNSF);
    /*Get flash clock divider register value */
    registerValue = REG_READ(SSD_MCU_REGISTER_BASE + FLASH_FCLKDIV_OFFSET);

    if(FALSE == (registerValue & FLASH_FCLKDIV_FDIVLD)) /* clock not set */
    {
        /* clock not set */
        /* If S12XS family */
        #if (SGF18_SELECT == S12XS_SGF18)
            /* oscillator clock in MHz */
            clockValue = SSD_OSCILLATOR_CLOCK/100;
            /* calculating flash divider value */
            if((clockValue >= 2) && (clockValue < 21))
            {
                flashDivider = clockValue - 1;
            }
            else if((clockValue >= 21) && (clockValue < 42))
            {
                flashDivider = clockValue - 2;
            }
            else if((clockValue >= 42) && (clockValue <= 50))
            {
                flashDivider = clockValue - 3;
            }
            else
            {
                /* if oscillator clock is less than or equal to 1MHz or greater than 50MHz,
                   return an invalid clk error code SGF_ERR_INVALIDCLK */
                returnCode = SGF_ERR_INVALIDCLK;
            }
        #else
        /* If S12P/S12HY family */
            /* bus clock in MHz */
            clockValue = SSD_BUS_CLOCK/100;
            /* calculating flash divider value */
            if((clockValue >= 1) && (clockValue <= 32))
            {
                flashDivider = clockValue - 1;
            }
            else
            {
                /* if bus clock is less than 1MHz or greater than 32MHz,
                   return an invalid clk error code SGF_ERR_INVALIDCLK */
                returnCode = SGF_ERR_INVALIDCLK;
            }
        #endif

        if(SGF_OK == returnCode )
        {
            /* write the flashDivider value into the FCLKDIV register */
            REG_WRITE((SSD_MCU_REGISTER_BASE + FLASH_FCLKDIV_OFFSET), \
                      (UINT8)flashDivider);
                            /* If S12P family */
            #if (SGF18_SELECT == S12P_SGF18)
                /* write the flashDivider value into the FCLKDIV register */
                REG_BIT_SET((SSD_MCU_REGISTER_BASE + FLASH_FCLKDIV_OFFSET), \
                             FLASH_FCLKDIV_FDIVLCK);
            #endif

            #if (SGF18_SELECT == S12HY_SGF18)
                /* write the flashDivider value into the FCLKDIV register */
                REG_BIT_SET((SSD_MCU_REGISTER_BASE + FLASH_FCLKDIV_OFFSET), \
                             FLASH_FCLKDIV_FDIVLCK);
            #endif
        }
    }
    else
    {
        /* clock set : do nothing */
    }

    return(returnCode);
}
