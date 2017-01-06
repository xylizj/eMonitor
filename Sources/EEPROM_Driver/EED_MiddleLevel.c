/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2009, All Rights Reserved  *
************************************************************************/

/************************************************************************
*                                                                       *
*               EEPROM Emulation Driver for S12XS/S12P/S12HY            *
*                                                                       *
* FILE NAME     :  EED_MiddleLevel.c                                    *
* DATE          :  03.09.2009                                           *
* AUTHOR        :  Infosys Team, Mysore                                 *
* EMAIL         :  r56611@freescale.com                                 *
************************************************************************/

/******************************* CHANGES ********************************
 0.0.0a   06.11.2007    Sindhu_R01       Initial Version.
 0.0.1    06.20.2007    Jiben_Prakash    Added APIs: FSL_Erase()
                                         FSL_Program() FSL_ReadRecord()
 0.0.2    08.02.2007    Sindhu R01       Updated to support the
                                         Cosmic compiler
 0.0.3    10.17.2007    Sindhu R01       Updated code to clear the DFDIF
                                         bit and to set erasing cycles
                                         to 0 if it reaches 0xFFFE
 0.1.0    04.28.2008    Sindhu R01       Updated the functions
                                         FSL_InitSector(), FSL_SwapSector(),
                                         FSL_SearchLoop(), FSL_ReadRecord(),
                                         FSL_SectorStatus() and Added
                                         the function FSL_CheckMarginLevel()
                                         to fix partial programming
                                         and dead sector errors
 0.2.0    06.05.2008    Sindhu R01       Updated the functions to remove the
                                         SSD Flash Configuration sturcture.
                                         The CallBack is a global variable
                                         and the code for calling it is a
                                         compiler switch.
                                         Modified the FSL_SearchRecord()
 0.3.0    10.22.2008    Arvind Awasthi,  Updated the functions SwapSector(),
                        Vikas Kothari,   added the functions FSL_GetErasingCycles(),
                        Sindhu R01       FSL_InitFirstTime(), FSL_UpdtCacheTable()
                                         and FSL_ISR_Handler() to make the
                                         EED driver interrupt based.
 0.3.1    11.08.2008    Sindhu R01       Changed the FSL_UpdtCacheTable()
 0.4.0    02.04.2009    Jiben Prakash,   Modified the file to support
                        Vikas Kothari.   mutiple byte programming.
 0.4.1    02.20.2009    Sindhu R01       Updated code to handle 16-bit addresses
                                         instead of the 32-bit addressed.
 0.4.2    03.09.2009    Sindhu R01       Updated code to recover the approximate
                                         erase cycles of the block having
                                         ECC error in its erasing cycles field.
************************************************************************/

#include "ee_emulation.h"
#include <mc9s12hy64.h>     /* derivative information */
#include "MCU.h"
/* Global variables */
/* A command array which contains the data to be written to the FCCOB register. */
UINT16 commandArray[6]; /* command sequence array */

#if(CALLBACK_ENABLE == TRUE)
  /* A function pointer to the CallBack function */
  PCALLBACK      CallBack;
#endif



/***********************************************************************
*
*  Function Name    : FSL_Program
*  Description      : This function will program a range of contiguous
                      Flash locations and verify those.
*  Arguments        : destination,size,srcPtr
*  Return Value     : UNIT16
*
***********************************************************************/


UINT16 FSL_Program (UINT16 dest,UINT16 size,UINT16 *SSD_SGF18_NEAR srcPtr)
{
    /* declaraion of the local variables */
    UINT16 returnCode;

    /* If CallBack is enabled, then serve callback function */
    #if(CALLBACK_ENABLE == TRUE)
        CallBack();
    #endif

    /* DFlashProgram function called */
    returnCode=DFlashProgram(dest,size,srcPtr);

    if(returnCode == SGF_OK)
    {
        /* DFlashProgramVerify function called */
        returnCode=FlashProgramVerify(dest,size,srcPtr);
        if(returnCode != SGF_OK)
        {
            return(EED_ERROR_VERIFY);
        }
    }

    /* return the final return code based on the loop outcome */
    return(returnCode);
}

/***********************************************************************
*
*  Function Name    : FSL_CopyRecord
*  Description      : writes user data into Flash in a record format.
*  Arguments        : dataRecord,dest
*  Return Value     : UNIT16
*
***********************************************************************/
UINT16 FSL_CopyRecord(DATA_RECORD dataRecord, UINT16 dest)
{
    UINT16 returnCode;
    #if EED_CACHETABLE_ENABLE == ON
      UINT16 tempAddress;
    #endif  
    /* If CallBack is enabled, then serve callback function */
    #if(CALLBACK_ENABLE == TRUE)
        CallBack();
    #endif

    /* Program the data first */
    returnCode = FSL_Program(dest, EED_DATA_VALUE_SIZE, dataRecord.dataPtr);
    if ( EED_OK != returnCode )
    {
        return(returnCode);
    }
    dest += EED_DATA_VALUE_SIZE;

    /* Program the DataID next */
    returnCode = FSL_Program(dest, EED_DATA_ID_SIZE, &(dataRecord.dataID));
    if ( EED_OK != returnCode )
    {
        return(returnCode);
    }

    //!<==========================================================================
    /* Program the RecordStatus next */ /*added by xyl*/   
    //!<==========================================================================
    /*returnCode = FSL_Program(dest, EED_RECORD_STATUS_SIZE, &(dataRecord.dataID));
    if ( EED_OK != returnCode )
    {
        return(returnCode);
    } */
    
    #if EED_CACHETABLE_ENABLE == ON
    /* Check if the ID is a cache table entry */
    if( EED_MAX_CACHETABLE_ENTRY >= dataRecord.dataID )
    {
        /*update the cache table*/
        /* insert the data ID into the cache table */
        tempAddress = EED_CACHETABLE_START_ADDRESS;
        dest -= EED_DATA_VALUE_SIZE;

        /* update the cache table item with new value */
        WRITE16((tempAddress + EED_CTABLE_ITEM_SIZE * (dataRecord.dataID-1)), dest);/* Write 16-bit address */
    }
    #endif
    return(EED_OK);
}

/***********************************************************************
*
*  Function Name    : FSL_GetErasingCycles
*  Description      : Reads the erasing cycles of the sector
*  Arguments        : sectorAddress,pErrorGettingEC
*  Return Value     : UNIT16
*
***********************************************************************/
UINT16 FSL_GetErasingCycles (UINT16 sectorAddress,BOOL *SSD_SGF18_NEAR pErrorGettingEC)
{
    /* If S12XS family */
    #if (SGF18_SELECT == S12XS_SGF18)
      UINT8   saveEpage;          /* save EPAGE register */
      UINT8   epage;              /* for setting EPAGE register */
    #endif
    UINT16  erasingCycles;

    /* If S12XS family */
    #if (SGF18_SELECT == S12XS_SGF18)
      /* Save the current EPAGE value */
      saveEpage = REG_READ(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET);
      /* Calculate the page value of the global Address passed */
      epage = (UINT8)((DFLASH_LOWEST_START_PAGE)+(sectorAddress>>10));
      /* Write the calculate page value in the EPAGE register */
      REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, epage);
    #endif

    /* Store the erasing cycles of the sector before erasing the sector */
    erasingCycles = READ_DFLASH16(sectorAddress + EED_SECTOR_STATUS_SIZE);

    /* If S12XS family */
    #if (SGF18_SELECT == S12XS_SGF18)
      /* Restore the EPAGE register */
      REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, saveEpage);
    #endif

    /* check ferstat*/
    if(REG_BIT_TEST(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF) != 0)
    {
        /* Clear the DFDIF error flag bit */
        REG_BIT_SET(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF);

        *pErrorGettingEC = TRUE;
    }

    return(erasingCycles);
}

/***********************************************************************
*
*  Function Name    : FSL_PgmSectorStatus
*  Description      : initializes one sector to make it ready for EEPROM
*                     emulation.
*  Arguments        : sectorAddress,status,erasingCycles,ersVerify
*  Return Value     : UNIT16
*
***********************************************************************/
UINT16 FSL_PgmSectorStatus (UINT16 sectorAddress, UINT16 status, UINT16 erasingCycles, BOOL ersVerify)
{
    UINT16 returnCode;
    UINT16 tempValue;
    //UINT16 tempValue[2];//modified by xyl

    tempValue = EED_ZEROS_16;
    //tempValue[0] = EED_ZEROS_16; //modified by xyl
    //tempValue[1] = EED_ZEROS_16; //modified by xyl

    if(TRUE == ersVerify)
    {
        /* DFlashEraseVerify function called */
        returnCode = DFlashEraseVerify(sectorAddress,EED_SECTOR_SIZE);
        if(returnCode != EED_OK)
        {
              /* The sector was not erased */
              /* The sector should be initialized as DEAD sector */
              returnCode = FSL_Program(sectorAddress, EED_SECTOR_STATUS_SIZE, &tempValue);
              //returnCode = FSL_Program(sectorAddress, EED_SECTOR_STATUS_SIZE, tempValue); //modified by xyl
              if ( EED_OK != returnCode )
              {
                  return(returnCode);
              }
              else
              {
                  return(EED_DEAD_SECTOR);
              }

        }
    }

    if(TRUE == ersVerify || TRUE == gInitFirstTime)
    {

        tempValue = erasingCycles;
        //sectorAddress += 4; //added by xyl
        returnCode = FSL_Program((sectorAddress + 2), EED_ERASING_CYCLES_SIZE, &tempValue);
        if ( EED_OK != returnCode )
        {
           return(returnCode);
        }
    }

    /* If the sector has to be initialised as Active then write to the status bytes */
    if(EED_SECTOR_ACTIVE == status)
    {
         tempValue = SECTOR_STATUS_ACTIVE;
         returnCode = FSL_Program(sectorAddress, EED_SECTOR_STATUS_SIZE, &tempValue);
         if ( EED_OK != returnCode )
         {
            return(returnCode);
         }
    }
    return(returnCode);
}

/***********************************************************************
*
*  Function Name    : FSL_SwapSector
*  Description      : Swaps the contents of the oldest active sector to
                      the oldest alternative sector
*  Arguments        : void
*  Return Value     : UNIT16
*
***********************************************************************/
/*DATA_RECORD dataRecord_Debug; DATA_RECORD dataRecord_Debug2;
UINT16  returnCode_Debug;
UINT16  returnCode_Debug1;
UINT16  recFoundAddress_Debug1; */

UINT16 FSL_SwapSector(void)
{
    UINT16  sectorAddress, endOldSector;
    UINT16  recordAddress;
    UINT16  recFoundAddress;
    UINT16  returnCode;
    UINT16  i;
    BOOL errorGettingEC;
    DATA_RECORD dataRecord;
    /* To hold the record data of the old sector written to the new update sector */
    UINT16 recordData[EED_DATA_VALUE_SIZE/EED_MIN_PROG_SIZE];

    /* If CallBack is enabled, then serve callback function */
    #if(CALLBACK_ENABLE == TRUE)
        CallBack();
    #endif

    sectorAddress = currentActiveSector;
    returnCode = SECTOR_STATUS_ACTIVE;

    /* The first Alternative sector after the current active sector is the ready sector used for updation */
    /* Skip any dead sectors if any */
    while ( SECTOR_STATUS_ERASED != returnCode )
    {
        sectorAddress += EED_SECTOR_SIZE;

        if (sectorAddress > EED_FLASH_END_ADDRESS )
        {
            sectorAddress = EED_FLASH_START_ADDRESS;
        }

        #if (SGF18_SELECT == S12XS_SGF18)
        /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
           global address to logical address, to read the D-Flash */
          returnCode = FSL_ReadDFlash(sectorAddress);
        #else
        /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
          returnCode = READ_DFLASH16(sectorAddress);
        #endif
    }

    currentActiveSector = sectorAddress;
    freeSpaceAddress = currentActiveSector + EED_SECTOR_INFO_SIZE;

    returnCode = SECTOR_STATUS_ERASED;

    /* The first active sector after the alternative sectors is the oldest Active sector */
    while ( SECTOR_STATUS_ACTIVE != returnCode )
    {
        sectorAddress += EED_SECTOR_SIZE;

        if (sectorAddress > EED_FLASH_END_ADDRESS )
        {
            sectorAddress = EED_FLASH_START_ADDRESS;
        }

        #if (SGF18_SELECT == S12XS_SGF18)
        /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
           global address to logical address, to read the D-Flash */
          returnCode = FSL_ReadDFlash(sectorAddress);
        #else
        /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
          returnCode = READ_DFLASH16(sectorAddress);
        #endif
    }

    endOldSector = sectorAddress + EED_SECTOR_SIZE - EED_SECTOR_WASTE;
    recordAddress = sectorAddress + EED_SECTOR_INFO_SIZE;

    /* Loop through the records of the oldest ACTIVE sector */
    while ( recordAddress < endOldSector )
    {
        //------------------------------------------------------------------------------
        //static UINT8 cnt;
        
        
        
        returnCode = FSL_ReadRecord(&dataRecord.dataID, recordAddress);
        recordAddress += EED_RECORD_LENGTH;

        /*if(0 == cnt)
        {
          cnt++;
          returnCode_Debug = returnCode;
          dataRecord_Debug = dataRecord;
        }*/
        
        

        /* If the record read is a valid record, then search for the record */
        if ( EED_RECORD_VALID == returnCode )
        {
            //static UINT8 cnt1; static UINT8 cnt2;
            
            returnCode = FSL_SearchLoop(dataRecord.dataID, &recFoundAddress);

            /*if(0 == cnt1)
            {
              cnt1++;
              returnCode_Debug1 = returnCode;
              recFoundAddress_Debug1 = recFoundAddress;
            } */
            
            
            if(EED_OK == returnCode)
            {
                /* If the record is found in the old ACTIVE sector,
                   then the record should be written to the UPDATE sector */
                if ( (recFoundAddress >= sectorAddress ) && ( recFoundAddress < endOldSector ))
                {
                    /* Copy the data into RAM */
                    for(i=0; i<(EED_DATA_VALUE_SIZE/EED_MIN_PROG_SIZE); i++)
                    {
                        /* to read the data value at the address.*/
                        #if (SGF18_SELECT == S12XS_SGF18)
                        /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
                           global address to logical address, to read the D-Flash */
                          recordData[i] = FSL_ReadDFlash(recFoundAddress);
                        #else
                        /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
                          recordData[i] = READ_DFLASH16(recFoundAddress);
                        #endif
                        recFoundAddress+=EED_MIN_PROG_SIZE;
                    }

                    dataRecord.dataPtr = (UINT16 *SSD_SGF18_NEAR)recordData;
                    
                    /*if(0 == cnt2)
                    {
                      cnt2++;
                      dataRecord_Debug2 = dataRecord;
                    }*/

                    returnCode = FSL_CopyRecord( dataRecord, freeSpaceAddress);
                    if (EED_OK != returnCode )
                    {
                        return(returnCode);
                    }
                    else
                    {
                        freeSpaceAddress += EED_RECORD_LENGTH;
                    }
                }
            }
        }
    }

    /*program cluster as active*/
     returnCode = FSL_PgmSectorStatus(currentActiveSector,EED_SECTOR_ACTIVE,EED_ZEROS_16,FALSE);
    if( EED_OK != returnCode )
    {
        return(returnCode);
    }

    /* Init the old sector as Alternative sector */
    gEraseCycles = FSL_GetErasingCycles(sectorAddress, &errorGettingEC);
    if(EED_MAX_ERASING_CYCLE_VALUE == gEraseCycles)
    {
        gEraseCycles = EED_ZEROS_16;
    }
    else
    {
        gEraseCycles++;
    }
    /* set job for main as swap*/
    gJobToMain = SWAP_CLUSTER;
    /*set global main status flag as started*/
    EE_Status = BUSY;
    /* Set the address of the cluster to be initialized as ALTERNATIVE */
    gAddressToMain = sectorAddress;
    returnCode = DFlashErase(sectorAddress);
    if(EED_OK != returnCode)
    {
        EE_Status = FAIL;
        return(returnCode);
    }
    return(EED_OK);
}

/***********************************************************************
*
*  Function Name    : FSL_SearchRecord
*  Description      : Searches record with a specific record ID in a
                      sector. This function will parse the whole sector
                      to find out the latest copy of data records.
*  Arguments        : dataID,sectorAddress,recordAddress
*  Return Value     : UINT16
*
***********************************************************************/
UINT16 FSL_SearchRecord (UINT16 dataID, UINT16 sectorAddress, UINT16 *SSD_SGF18_NEAR recordAddress)
{

    UINT16 searchAddress, endAddress;
    UINT16 returnCode;
    BOOL   foundFlag = FALSE ;

    /* If CallBack is enabled, then declare counter for callback function calling */
    #if(CALLBACK_ENABLE == TRUE)
    UINT16  counter;
    CallBack();
    #endif

    /* If the search is for a blank record then start the search from the top
       Else search from the last record */
    if ( EED_ERASED_STATE16 == dataID )
    {
        /* set searchAddress to the data ID field of the first record of the sector */
        searchAddress = sectorAddress + EED_SECTOR_INFO_SIZE + EED_DATA_VALUE_SIZE;
        /* End address of the sector to be searched */
        endAddress = sectorAddress + EED_SECTOR_SIZE - EED_SECTOR_WASTE;

        /* Search all the records of the sector */
        while ( searchAddress < endAddress )
        {
            /* Read the data ID at searchAddress */
            #if (SGF18_SELECT == S12XS_SGF18)
            /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
               address to logical address, to read the D-Flash */
              returnCode = FSL_ReadDFlash(searchAddress);
            #else
            /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
              returnCode = READ_DFLASH16(searchAddress);
            #endif

            /* check for ECC error */
            if (REG_BIT_TEST(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF) != 0)
            {
                /* Clear the DFDIF error flag bit */
                REG_BIT_SET(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF);
            }
            /* Check if the data ID read matches the record ID being searched */
            else if (returnCode == dataID)
            {
                /* If data ID read is erased, check if the record is a blank record
                   or an invalid record */
                returnCode = FSL_ReadRecord(&dataID, searchAddress);

                if( EED_RECORD_BLANK == returnCode )
                {
                    /* A blank record is found */
                    foundFlag = TRUE;
                    *recordAddress = searchAddress - EED_DATA_VALUE_SIZE;
                    break;
                }
            }
            else
            {
                /* The data ID read does not match the record ID being searched.
                   Search the next record */
            }
            /* increment searchAddress by EED_RECORD_LENGTH */
            searchAddress += EED_RECORD_LENGTH;

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
        /* set searchAddress to the data ID field of the last record of the sector */
        searchAddress = sectorAddress + EED_SECTOR_SIZE - EED_RECORD_LENGTH - EED_SECTOR_WASTE + EED_DATA_VALUE_SIZE;

        /* Search all the records of the sector */
        while ( searchAddress > (sectorAddress + EED_SECTOR_INFO_SIZE) )
        {
            /* Read the data ID at searchAddress */
            #if (SGF18_SELECT == S12XS_SGF18)
            /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
               address to logical address, to read the D-Flash */
              returnCode = FSL_ReadDFlash(searchAddress);
            #else
            /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
              returnCode = READ_DFLASH16(searchAddress);
            #endif

            /* check for ECC error */
            if (REG_BIT_TEST(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF) != 0)
            {
                /* Clear the DFDIF error flag bit */
                REG_BIT_SET(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF);
            }
            /* Check if the data ID read matches the record ID being searched */
            else if (returnCode == dataID)
            {
                foundFlag = TRUE;
                *recordAddress = searchAddress - EED_DATA_VALUE_SIZE;
                break;
            }
            else
            {
                /* The data ID read does not match the record ID being searched.
                   Search the next record */
            }

            /* deccrement searchAddress by EED_RECORD_LENGTH */
            searchAddress -= EED_RECORD_LENGTH;

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

    if( TRUE == foundFlag )
    {
        return(EED_OK);
    }
    else
    {
        return(EED_ERROR_NOFND);
    }
}


/***********************************************************************
*
*  Function Name    : FSL_SearchLoop
*  Description      : Searches record with a specific record ID in all
                      active sector.
*  Arguments        : dataID,recordAddress
*  Return Value     : UINT16
*
***********************************************************************/
UINT16 FSL_SearchLoop (UINT16 dataID, UINT16 *SSD_SGF18_NEAR recordAddress)
{
    UINT16 tempAddress;
    UINT16 returnCode;
    #if EED_CACHETABLE_ENABLE == ON
      UINT16 recAddr;
    #endif

    /* If CallBack is enabled, then serve callback function */
    #if(CALLBACK_ENABLE == TRUE)
        CallBack();
    #endif

    /* check if cache table is enabled */
    #if EED_CACHETABLE_ENABLE == ON
    /* Check if the DataID is in the cache table */
    if( EED_MAX_CACHETABLE_ENTRY >= dataID )
    {
        tempAddress = (EED_CACHETABLE_START_ADDRESS + (EED_CTABLE_ITEM_SIZE * (dataID-1))) ;
        recAddr = READ16(tempAddress);/* Read the 16-bit address */

        if (EED_ERASED_STATE16 == recAddr )
        {
            return (EED_ERROR_NOFND);
        }
        else
        {
            *recordAddress = recAddr;
            return(EED_OK);
        }
    }
    #endif

    /* ID is not in the cache table */
    tempAddress = currentActiveSector;
    returnCode = SECTOR_STATUS_ACTIVE;

    /* Search in the ACTIVE sectors only */
    do
    {
        /* the first sector to be searched need not be a Active sector */
        if(SECTOR_STATUS_DEAD != returnCode)
        {
            returnCode = FSL_SearchRecord(dataID, tempAddress, recordAddress);
            if ( EED_OK == returnCode )
            {
                return(returnCode);
            }
        }

        /* search the next sector */
        /* Get the next sector address */
        if(EED_FLASH_START_ADDRESS == tempAddress)
        {
            tempAddress = EED_FLASH_END_ADDRESS - EED_SECTOR_SIZE + 1;
        }
        else
        {
            tempAddress -= EED_SECTOR_SIZE;
        }

        #if (SGF18_SELECT == S12XS_SGF18)
        /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
           global address to logical address, to read the D-Flash */
          returnCode = FSL_ReadDFlash(tempAddress);
        #else
        /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
          returnCode = READ_DFLASH16(tempAddress);
        #endif

    } while (SECTOR_STATUS_ERASED != returnCode);

    return(EED_ERROR_NOFND);
}

/***********************************************************************
*
*  Function Name    : FSL_ReadRecord
*  Description      : This function is used to read the data and ID
                      fields and the associated ECC and return the status
                      of the record.
*  Arguments        : dataID,readAddress
*  Return Value     : UNIT16
*
***********************************************************************/
UINT16 FSL_ReadRecord (UINT16 *SSD_SGF18_NEAR dataID,UINT16 readAddress)
{
    UINT16 temp;
    UINT8   i;
    BOOL blankFlag=TRUE;

    /* If CallBack is enabled, then serve callback function */
    #if(CALLBACK_ENABLE == TRUE)
        CallBack();
    #endif

    /*validate the data, but dont have to copy it. The pointer to start of data
    is available in the calling function*/
    for(i=0; i<(EED_DATA_VALUE_SIZE/EED_MIN_PROG_SIZE); i++)
    {
        /* to read the data value at the address.*/
        #if (SGF18_SELECT == S12XS_SGF18)
        /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
           global address to logical address, to read the D-Flash */
          temp = FSL_ReadDFlash(readAddress);
        #else
        /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
          temp = READ_DFLASH16(readAddress);
        #endif

        /* check for ECC error */
        if (REG_BIT_TEST(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF) != 0)
        {
            /* Clear the DFDIF error flag bit */
            REG_BIT_SET(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF);

            /* returns invalid code */
            return(EED_RECORD_INVALID);
        }

        if((EED_ERASED_STATE16 == temp) && (TRUE == blankFlag))
        {
            blankFlag = TRUE;
        }
        else
        {
            blankFlag = FALSE;
        }

        readAddress+=EED_MIN_PROG_SIZE;
    }
    /* readAddress points the recordID of the current record */

    /*validate the ID and copy it*/
    #if (SGF18_SELECT == S12XS_SGF18)
    /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
       global address to logical address, to read the D-Flash */
      temp = FSL_ReadDFlash(readAddress);
    #else
    /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
      temp = READ_DFLASH16(readAddress);
    #endif

    /* check for ECC error */
    if (REG_BIT_TEST(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF) != 0)
    {
        /* Clear the DFDIF error flag bit */
        REG_BIT_SET(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF);

        /* returns invalid code */
        return(EED_RECORD_INVALID);
    }

    if(EED_ERASED_STATE16 == temp)
    {
        /* check for blank Flag */
        if(TRUE == blankFlag)
        {
            /* temp is assigned to the structure variable */
            *dataID = temp;

             /* returns record blank code */
             return(EED_RECORD_BLANK);
        }
        else
        {
            /* returns invalid code */
            return(EED_RECORD_INVALID);
        }

    }
    else
    {
        /* An ID of 0x0000 is invalid */
        if(EED_ZEROS_16 == temp)
        {
            /* returns invalid code */
            return(EED_RECORD_INVALID);
        }

        /* temp is assigned to the structure variable */
        *dataID = temp;

        /* returns a valid code */
        return(EED_RECORD_VALID);
    }
}

/***********************************************************************
*
*  Function Name    : FSL_SectorStatus
*  Description      : This function returns the status of the sector
                      that is pointed to by 'startAddr' and 'endAddr'.
*  Arguments        : sectorAddress
*  Return Value     : UINT16
*
***********************************************************************/
UINT16 FSL_SectorStatus (UINT16 sectorAddress)
{
    UINT16 valueRead;
    UINT16 dataId;
    /* If S12XS family */
    #if (SGF18_SELECT == S12XS_SGF18)
      UINT8   saveEpage;          /* save EPAGE register */
      UINT8   epage;              /* for setting EPAGE register */

      /* Save the current EPAGE value */
      saveEpage = REG_READ(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET);
      /* Calculate the page value of the global Address passed */
      epage = (UINT8)((DFLASH_LOWEST_START_PAGE)+(sectorAddress>>10));
      /* Write the calculate page value in the EPAGE register */
      REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, epage);
    #endif

    /* If CallBack is enabled, then serve callback function */
    #if(CALLBACK_ENABLE == TRUE)
        CallBack();
    #endif

    /* Read the sector status field */
    valueRead = READ_DFLASH16(sectorAddress);

    /* If S12XS family */
    #if (SGF18_SELECT == S12XS_SGF18)
      /* Restore the EPAGE register */
      REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, saveEpage);
    #endif

    /* check ferstat*/
    if(REG_BIT_TEST(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF) != 0)
    {
        /* Clear the DFDIF error flag bit */
        REG_BIT_SET(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF);

        return(EED_SECTOR_INVALID);
    }

    if(EED_ZEROS_16 == valueRead)
    {
        return(EED_SECTOR_DEAD);
    }

    if(SECTOR_STATUS_ACTIVE == valueRead)
    {
        return(EED_SECTOR_ACTIVE);
    }

    /* If S12XS family */
    #if (SGF18_SELECT == S12XS_SGF18)
      /* Store the page value calculated for the sector to EPAGE register */
      REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, epage);
    #endif

    /* Read the erasing cycles field */
    valueRead = READ_DFLASH16(sectorAddress+EED_SECTOR_STATUS_SIZE);

    /* If S12XS family */
    #if (SGF18_SELECT == S12XS_SGF18)
      /* Restore the EPAGE register */
      REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, saveEpage);
    #endif

    /* check ferstat for ECC error */
    if(REG_BIT_TEST(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF) != 0)
    {
        /* Clear the DFDIF error flag bit */
        REG_BIT_SET(FLASH_FERSTAT_OFFSET, FLASH_FERSTAT_DFDIF);

        return(EED_SECTOR_INVALID);
    }

    if(EED_ERASED_STATE16 == valueRead)
    {
        return(EED_SECTOR_BLANK);
    }

    /* Read the first record of the sector */
    valueRead = FSL_ReadRecord(&dataId, (sectorAddress+EED_SECTOR_INFO_SIZE));

    if(EED_RECORD_BLANK == valueRead)
    {
        return(EED_SECTOR_ALTERNATIVE);
    }
    else
    {
        return(EED_SECTOR_UPDATE);
    }
}

/***********************************************************************
*
*  Function Name    : FSL_CheckMarginLevel
*  Description      : This function reads an address to check if written.
*  Arguments        : readAddress
*  Return Value     : UINT16
*
***********************************************************************/
UINT16 FSL_CheckMarginLevel (UINT16 readAddress)
{
    UINT16 normalReadValue;
    UINT16 levelReadValue;
    UINT16 returnCode;
    /* If S12XS family */
    #if (SGF18_SELECT == S12XS_SGF18)
      UINT8   saveEpage;          /* save EPAGE register */
      UINT8   epage;              /* for setting EPAGE register */

      /* Save the current EPAGE value */
      saveEpage = REG_READ(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET);
      /* Calculate the page value of the global Address passed */
      epage = (UINT8)((DFLASH_LOWEST_START_PAGE)+(readAddress>>10));
      /* Write the calculate page value in the EPAGE register */
      REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, epage);
    #endif

    /* If CallBack is enabled, then serve callback function */
    #if(CALLBACK_ENABLE == TRUE)
        CallBack();
    #endif

    /* Read the value at normal margin level */
    normalReadValue = READ_DFLASH16(readAddress);

    /* Set the margin read level to 0 */
    returnCode = FlashSetUserMargin(FLASH_USER_MARGIN_LEVEL0);
    if(SGF_OK != returnCode)
    {
        /* If S12XS family */
        #if (SGF18_SELECT == S12XS_SGF18)
          /* Restore the EPAGE register before return */
          REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, saveEpage);
        #endif

        return(returnCode);
    }

    /* read the value when the margin level is set to 0 */
    levelReadValue = READ_DFLASH16(readAddress);

    /* If the normal read value and level read value don't match return an error */
    if(levelReadValue != normalReadValue)
    {
        /* If S12XS family */
        #if (SGF18_SELECT == S12XS_SGF18)
          /* Restore the EPAGE register before return */
          REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, saveEpage);
        #endif

        return(EED_NOT_OK);
    }

    /* check for level 1 */
    /* Set the margin read level to 1 */
    returnCode = FlashSetUserMargin(FLASH_USER_MARGIN_LEVEL1);
    if(SGF_OK != returnCode)
    {
       return(returnCode);
    }

    /* read the value when the margin level is set to 1 */
    levelReadValue = READ_DFLASH16(readAddress);

    /* If S12XS family */
    #if (SGF18_SELECT == S12XS_SGF18)
      /* Restore the EPAGE register */
      REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, saveEpage);
    #endif


    /* If the normal read value and level read value don't match return an error */
    if(levelReadValue != normalReadValue)
    {
        return(EED_NOT_OK);
    }

    /* the margin reads have succeeded */

    /* Reset the margin read level to normal read value */
    returnCode = FlashSetUserMargin(FLASH_MARGIN_NORMAL);
    if(SGF_OK != returnCode)
    {
       return(returnCode);
    }
    return(EED_OK);
}

/***********************************************************************
*
*  Function Name    : FSL_ReadDFlash
*  Description      : This function will read a D-Flash memory location
*                     by handling the page register
*  Arguments        : destination
*  Return Value     : UNIT16
*
***********************************************************************/
/* If S12XS family */
#if (SGF18_SELECT == S12XS_SGF18)

UINT16 FSL_ReadDFlash (UINT16 destination)
{
    UINT8   saveEpage;          /* save EPAGE register */
    UINT8   epage;              /* for setting EPAGE register */
    UINT16   data;              /* to store the data read */

    /* Save the current EPAGE value */
    saveEpage = REG_READ(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET);

    /* Calculate the page value of the global Address passed */
    epage = (UINT8)((DFLASH_LOWEST_START_PAGE)+(destination >>10));
    /* Write the calculate page value in the EPAGE register */
    REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, epage);
    /* Read the data from the D-Flash, by converting the global address to logical address */
    data = READ16((destination & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);

    /* Restore the EPAGE register */
    REG_WRITE(SSD_MCU_REGISTER_BASE + MMC2_EPAGE_OFFSET, saveEpage);

    return(data);
}
#endif

/***********************************************************************
*
*  Function Name    : FSL_InitFirstTime
*  Description      : This function will intialize all the clusters for
*                     the first time
*  Arguments        : None
*  Return Value     : UNIT16
*
***********************************************************************/
UINT16 FSL_InitFirstTime()
{

    UINT16 returnCode;
    UINT8  count;

    returnCode = EED_OK;

    for (count = gActiveCount+gAltCount; count < EED_SECTORS_ALLOTED; count++)
    {
        if(count < EED_ACTIVE_SECTORS)
        {
            /* initialize the Sector as ACTIVE */
            returnCode = FSL_PgmSectorStatus(gAddressToMain,EED_SECTOR_ACTIVE,EED_ZEROS_16, FALSE);
            if(EED_OK != returnCode)
            {
                   return(returnCode);
            }
        }
        else
        {
            /* initialize the Sector as ALTERNATIVE */
            returnCode = FSL_PgmSectorStatus(gAddressToMain, EED_SECTOR_ALTERNATIVE, EED_ZEROS_16,FALSE);
            if(EED_OK != returnCode)
            {
                return(returnCode);
            }
        }
        gAddressToMain += EED_SECTOR_SIZE;
    }
    return(returnCode);
}

/***********************************************************************
*
*  Function Name    : FSL_UpdtCacheTable
*  Description      : This function will update the cachetable
*  Arguments        : None
*  Return Value     : UNIT16
*
***********************************************************************/
/* Initialise the cache table with all the values, if it is enabled */
#if EED_CACHETABLE_ENABLE == ON
 void FSL_UpdtCacheTable ()
 {

    UINT16 tempAddress;
    UINT16 dataID;
    UINT16 tempValue;
    UINT8  count;
    UINT16 blankAddress;
    UINT16 sectorAddress;

    /* hold the start address of cache table */
    tempAddress = EED_CACHETABLE_START_ADDRESS;
    /* Loop through the data ID that should be written to the Cache Table */
    for(dataID = 1 ; dataID <= EED_MAX_CACHETABLE_ENTRY ; dataID++)
    {
        tempValue = SECTOR_STATUS_ACTIVE;
        sectorAddress = currentActiveSector;
        count = FALSE;/*used as a flag*/

        /* Search all the ACTIVE sectors */
        while (SECTOR_STATUS_ERASED != tempValue)
        {
            /* The DEAD sector should not be searched */
            if(SECTOR_STATUS_DEAD != tempValue)
            {
                /* blankAddress is used to hold the address of the record*/
                tempValue = FSL_SearchRecord(dataID, sectorAddress, &blankAddress);
            }

            if ( EED_OK == tempValue )
            {
                count = TRUE;
                break;
            }
            else
            {
                if (EED_FLASH_START_ADDRESS == sectorAddress)
                {
                    sectorAddress = EED_FLASH_END_ADDRESS - EED_SECTOR_SIZE + 1 ;
                }
                else
                {
                    sectorAddress -= EED_SECTOR_SIZE;
                }

                #if (SGF18_SELECT == S12XS_SGF18)
                /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
                   global address to logical address, to read the D-Flash */
                  tempValue = FSL_ReadDFlash(sectorAddress);
                #else
                /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
                  tempValue = READ_DFLASH16(sectorAddress);
                #endif
            }
        }
        if(TRUE == count)
        {
            WRITE16(tempAddress, blankAddress);/* Write 16-bit address */
        }
        else
        {
            WRITE16(tempAddress, EED_ERASED_STATE16);/* Write 16-bit address */
        }
        tempAddress += EED_CTABLE_ITEM_SIZE;
    }
    return;
 }
#endif

/*********************************************************************
*
*  Function Name    : FSL_ISRHandler
*  Description      : Interrupt service routine for Command Complete.
*  Arguments        : void
*  Return Value     : void
*
**********************************************************************/
#if 1

#pragma CODE_SEG NON_BANKED
#pragma TRAP_PROC              
//void SSD_SGF18_NEAR FSL_ISR_Handler(void) /* Flash interrupt service function */
void  interrupt VectorNumber_Vflash  FSL_ISR_Handler(void) /* Flash interrupt service function */
{
    UINT8  registerValue;    /* store data read from flash register */

    RTI_INT_DISABLE();//added by xyl

    /* Disable all flash interrupts */
    FlashInterruptSet(CCIE_INTERRUPT_DISABLE);

    /*Sector erase operation completed */
    /* Check error bits */
    /*Get flash status register value */
    registerValue = REG_READ(SSD_MCU_REGISTER_BASE + FLASH_FSTAT_OFFSET);
    /* checking access error */
    if(0 != (registerValue & FLASH_FSTAT_ACCERR))
    {
        gEE_Error = SGF_ERR_ACCERR;
        eraseStatusFlag = FAIL;
    }
    /* checking protection error */
    else if(0 != (registerValue & FLASH_FSTAT_FPVIOL))
    {
        gEE_Error = SGF_ERR_PVIOL;
        eraseStatusFlag = FAIL;
    }
    /* checking MGSTAT1 error */
    else if(0 != (registerValue & FLASH_FSTAT_MGSTAT1))
    {
        gEE_Error = SGF_ERR_MGSTAT1;
        eraseStatusFlag = FAIL;
    }
    /* checking MGSTAT0 non-correctable error */
    else if(0 != (registerValue & FLASH_FSTAT_MGSTAT0))
    {
        gEE_Error = SGF_ERR_MGSTAT0;
        eraseStatusFlag = FAIL;
    }
    else
    {
        eraseStatusFlag = DONE;
    }

    RTI_INT_ENABLE(); //added by xyl
}

#pragma CODE_SEG DEFAULT
#endif
/***********************************************************************
*
* End of file EED_MiddleLevel.c
*
***********************************************************************/