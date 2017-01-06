/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2009, All Rights Reserved  *
************************************************************************/

/************************************************************************
*                                                                       *
*               EEPROM Emulation Driver for S12XS/S12P/S12HY                  *
*                                                                       *
* FILE NAME     :  EED_HighLevel.c                                      *
* DATE          :  03.09.2009                                           *
* AUTHOR        :  Infosys Team, Mysore                                 *
* EMAIL         :  r56611@freescale.com                                 *
************************************************************************/

/******************************* CHANGES ********************************
 0.0.0a   06.18.2007    Sindhu_R01        Initial Version
 0.0.1    07.17.2007    Sindhu_R01        Changed the code for updating
                                          the cache table in FSL_InitEeprom
 0.0.2    08.02.2007    Sindhu R01        Updated to support the
                                          Cosmic compiler
 0.1.0    04.28.2008    Sindhu R01        Updated the functions
                                          FSL_InitEeprom(), FSL_ReadEeprom()
                                          and FSL_WriteEeprom()
                                          to fix partial programming
                                          and dead sector errors
 0.2.0    06.05.2008    Sindhu R01        Updated the functions to remove the
                                          SSD Flash Configuration sturcture.
                                          The CallBack is a global variable
                                          and the code for calling it is a
                                          compiler switch.
 0.2.1    09.25.2008    Sindhu R01        Bug fixed.Modified code to set
                                          sector address to point to
                                          last sector of emulation.
 0.3.0    10.23.2008    Arvind Awasthi,   Updated the functions InitEeprom(),WriteEeprom()
                        Vikas Kothari,    added the function FSL_Main()
                        Sindhu R01        to make the EED driver interrupt based.
 0.3.1    11.08.2008    Sindhu R01        Updated the function
                                          FSL_InitEeprom()
 0.3.2    12.06.2008    Sindhu R01        Modified the code to reset the
                                          global variable gInitFirstTime
                                          to FALSE, after the first time
                                          initialization is completed
 0.4.0    02.04.2009    Jiben Prakash,    Modified the file to support
                        Vikas Kothari.    mutiple byte programming.
                                          Changed the function prototype of
                                          FSL_WriteEeprom(), Parameter source
                                          is changed to a pointer.
 0.4.1    02.23.2009    Sindhu_R01        Updated code to handle 16-bit addresses
                                          instead of the 32-bit addressed.
 0.4.2    03.09.2009    Sindhu R01        Updated code to recover the approximate
                                          erase cycles of the block having
                                          ECC error in its erasing cycles field
                                          and when a brownout occurs during earse.
************************************************************************/

#include "ee_emulation.h"

/* Global variables */
/* Start address of the current active sector */
UINT16 currentActiveSector;

/* The blank address of the current active sector
   where the next data record is written */
UINT16 freeSpaceAddress;

DATA_RECORD gDataRecord;
UINT16 gAddressToMain;
UINT16 gEraseCycles;
VUINT16 gEE_Error;
UINT8  gJobToMain;
UINT8  gActiveCount;
UINT8  gAltCount;
BOOL   gInitFirstTime;

/* Flag to keep track of Main function state State*/
EE_StateType EE_Status = IDLE;
/* Flag to keep track of Erase State*/
volatile EE_StateType eraseStatusFlag = IDLE;

/* Static Variable Declaration, Scope: EED_HighLevel.c */
static UINT16 sStatusToMain;
static UINT8  sCounter;

/***********************************************************************
*
*  Function Name    : FSL_InitEeprom
*  Description      : This function will perform the flash module clock
                      initialization.This function will also determine active,
                      alternative and brown out affected sectors and
                      erase/update the sectors. Initializing variables
                      that hold active sector related information like
                      the start and end addresses of the active sector
                      and the blank space available is also done in this
                      function. Cache table is also initialized in this
                      function. If no sectors are initialized then,
                      this function shall initialize all the sectors in
                      a round robin queue
*  Arguments        : void
*  Return Value     : UNIT16
*
***********************************************************************/
UINT16 FSL_InitEeprom (void)
{
    UINT16 sectorAddress,tempAddress, blankAddress, altSector;
    UINT16 returnCode;
    UINT16 prevFreeSpace,freeSpace,tempValue;
    UINT8  count;
    UINT8  blkCount;
    UINT8  updtCount;
    BOOL   eraseFlag;
    BOOL   errorGettingEC;
    #if EED_CACHETABLE_ENABLE == ON
      UINT16 dataID;
    #endif
    /*Initialize flag*/
    errorGettingEC = FALSE;
    /* Initialize counter for number of erase to 1*/
    sCounter = 1;
    /* Initilize first time flag as false*/
    gInitFirstTime = FALSE;
    returnCode = EED_OK;
    blkCount = 0;
    updtCount = 0;
    gActiveCount=0;
    gAltCount=0;
    eraseFlag = FALSE;
    altSector = EED_ERASED_STATE16;
    currentActiveSector = EED_ERASED_STATE16;
    freeSpaceAddress = EED_ERASED_STATE16;

    /* If CallBack is enabled, then serve callback function */
    #if(CALLBACK_ENABLE == TRUE)
        CallBack();
    #endif

    /* Initialise the Flash clock */
    returnCode = FlashInit();
    if ( EED_OK != returnCode )
    {
        return(returnCode);
    }

    sectorAddress = EED_FLASH_START_ADDRESS;
    prevFreeSpace = EED_SECTOR_SIZE;

    /*the loop checks the status of all sectors used in emulation*/
    for(count = 0; count < EED_SECTORS_ALLOTED; count++)
    {
        /* get the status of the sectors */
        returnCode = FSL_SectorStatus(sectorAddress);

        if(EED_SECTOR_BLANK == returnCode)
        {
            blkCount++;
            tempAddress = sectorAddress;
        }
        else if(( EED_SECTOR_UPDATE == returnCode ) || (EED_SECTOR_INVALID == returnCode ))
        {
            updtCount++;
            tempAddress = sectorAddress;
        }
        else if(EED_SECTOR_ACTIVE == returnCode)
        {
            gActiveCount++;

            /* find the free space of the sector to */
            returnCode = FSL_SearchRecord(EED_BLANK_RECID,sectorAddress,&blankAddress);
            if(returnCode == EED_OK)
            {
                /* the sector with the minimum free space is the current active sector */
                freeSpace = (sectorAddress + EED_SECTOR_SIZE) - blankAddress;
                if(( freeSpace < prevFreeSpace ) && ((EED_RECORD_LENGTH+EED_SECTOR_WASTE) <= freeSpace))
                {
                    prevFreeSpace = freeSpace;
                    currentActiveSector = sectorAddress;
                    freeSpaceAddress = blankAddress;
                }
            }
        }
        else if(EED_SECTOR_DEAD == returnCode)
        {
            gAltCount++;
        }
        else
        {
            /* The sector is alternative */
            gAltCount++;
            altSector = sectorAddress ;
        }
        sectorAddress += EED_SECTOR_SIZE;

        /* If CallBack is enabled, then serve callback function */
        #if(CALLBACK_ENABLE == TRUE)
            CallBack();
        #endif
    }

    /* Initialise the cache table with all the values as 0xFFFFFFFF if it is enabled */
    #if EED_CACHETABLE_ENABLE == ON
   /* cache table should be inititlised */
    sectorAddress = EED_CACHETABLE_START_ADDRESS; /* re used hold the start address of cache table */
    for(dataID = 1 ;dataID <= EED_MAX_CACHETABLE_ENTRY; dataID++)
    {
        WRITE16(sectorAddress, EED_ERASED_STATE16);/* the values should be 0xFFFF(16-bit value) */
        sectorAddress += EED_CTABLE_ITEM_SIZE;
    }
    #endif

    /*first time initialization of the EEE and handling of
    brownout condition that occurs during first time initialization */
    if( blkCount > 1 )
    {
        /* set initialization stage as first time */
        gInitFirstTime = TRUE;
        if(1 == updtCount )
        {
            /* An invalid sector should be initialized as alternative sector */
            gAddressToMain = tempAddress;
            eraseFlag = TRUE;
            sStatusToMain = EED_SECTOR_ALTERNATIVE;
        }
        /* Brownout with one Alternative sector */
        else if((gActiveCount < EED_ACTIVE_SECTORS) && (gAltCount == EED_ONE))
        {
            /* Brownout with one Alternative sector */
            gAddressToMain = altSector;
            eraseFlag = TRUE;
            sStatusToMain = EED_SECTOR_ACTIVE;
            gActiveCount++;
            gAltCount = 0;
        }
        /* First Time Initialization, with no brownouts */
        else if(EED_SECTORS_ALLOTED == blkCount)
        {
            sectorAddress = EED_FLASH_START_ADDRESS;
            gAddressToMain = sectorAddress;
            eraseFlag = FALSE;
         }
        /* Brownout with no Alternative sector */
        else if((gActiveCount <= EED_ACTIVE_SECTORS) && (EED_ZEROS_16 == gAltCount))
        {
            /* Last initialized sector is ACTIVE */
            sectorAddress = EED_FLASH_START_ADDRESS + (EED_SECTOR_SIZE * (gActiveCount-1)) ;
            returnCode = FSL_CheckMarginLevel(sectorAddress);
            if(EED_NOT_OK == returnCode)
            {

                /*The status not programmed correctly, erase required*/
                gAddressToMain = sectorAddress;
                eraseFlag = TRUE;
                sStatusToMain = EED_SECTOR_ACTIVE;
            }
            else
            {
                /* The status is programmed correctly */
                sectorAddress += EED_SECTOR_SIZE;
                gAddressToMain = sectorAddress;
                eraseFlag = FALSE;
            }
        }
        else
        {
            /* Last initialized sector is ALTERNATIVE */
            sectorAddress = EED_FLASH_START_ADDRESS + (EED_SECTOR_SIZE * (gActiveCount+gAltCount-1));
            returnCode = FSL_CheckMarginLevel(sectorAddress + EED_SECTOR_STATUS_SIZE);
            if(EED_NOT_OK == returnCode)
            {
                /*The status not programmed correctly, erase required*/
                gAddressToMain = sectorAddress;
                eraseFlag = TRUE;
                sStatusToMain = EED_SECTOR_ALTERNATIVE;
            }
            else
            {
               /* The status is programmed correctly */
               sectorAddress += EED_SECTOR_SIZE;
               gAddressToMain = sectorAddress;
               eraseFlag = FALSE;
            }
        }
        if (FALSE == eraseFlag)
        {
           /* No erase required */
           returnCode = FSL_InitFirstTime();
           /* set the values pertaining to the current active sector */
           currentActiveSector = EED_FLASH_START_ADDRESS;
           freeSpaceAddress = EED_FLASH_START_ADDRESS + EED_SECTOR_INFO_SIZE;
           /* set initialization stage as first time to FALSE */
           gInitFirstTime = FALSE;
        }
        else
        {
            /* Get the erasing cycles of the the current EE sector */
            gEraseCycles = FSL_GetErasingCycles (gAddressToMain, &errorGettingEC);
            if(TRUE == errorGettingEC)
            {
                /* Set erasing cycles to 1,as the brownout occurred during
                   first time initialization */
                gEraseCycles = EED_ONE;
            }
            else
            {
                gEraseCycles++;
            }

            /* start the erase operation and exit */
            /*set the job to be done by main as Init*/
            gJobToMain = INIT;
            /*set global init status flag as started*/
            EE_Status = BUSY;

            returnCode = DFlashErase(gAddressToMain);
            if (returnCode != SGF_OK)
            {
               EE_Status = FAIL;
            }
        }
        return(returnCode);
    }

    /* Check out for other brown out scenarios */
    if(1 == blkCount)
    {
        gAddressToMain = tempAddress;
        sStatusToMain = EED_SECTOR_ALTERNATIVE;

        /* DFlashEraseVerify function called */
        returnCode = DFlashEraseVerify(tempAddress,EED_SECTOR_SIZE);
        /* If the sector is not erased, then erase and initialize it as ALTERNATIVE */
        if(returnCode != SGF_OK)
        {
            /* A blank sector should be initialised as alternative sector */
            eraseFlag = TRUE;
        }
        else
        {
            /* Erase not required*/
            eraseFlag = FALSE;
            /* Get the erasing cycles to be programmed from the previous sector */
            /* Read the erasing cycles of the previous sector */
            if(EED_FLASH_START_ADDRESS == tempAddress)
            {
                /* if current sector is first sector, get the erasing cycles of last sector */
                gEraseCycles = FSL_GetErasingCycles((EED_FLASH_END_ADDRESS - EED_SECTOR_SIZE + 1), &errorGettingEC);
            }
            else
            {
                /* get erasing cycles of previous sector */
                gEraseCycles = FSL_GetErasingCycles((tempAddress - EED_SECTOR_SIZE), &errorGettingEC);
            }
            if(EED_MAX_ERASING_CYCLE_VALUE == gEraseCycles)
            {
                gEraseCycles = EED_ZEROS_16;
            }
            else
            {
                gEraseCycles++;
            }
            /* Initialize the sector as Alternative sector */
            returnCode = FSL_PgmSectorStatus(gAddressToMain,sStatusToMain,gEraseCycles,/*erase verify*/TRUE);
            if(EED_OK != returnCode)
            {
                return(returnCode);
            }
        }
    }
    else if(1 == updtCount )
    {
        /* An invalid sector or an update sector should be initialized as alternative sector */
        gAddressToMain = tempAddress;
        eraseFlag = TRUE;
        sStatusToMain = EED_SECTOR_ALTERNATIVE;
    }
    else
    {
        /* Do Nothing */
    }

    /* If it is not first time initialization */
    /* Check if the current active sector has been identified */
    if(EED_ERASED_STATE16 == currentActiveSector)
    {
        /* If the current active sector has not been identified,
           then the first active before the alternative sector is the current active sector */
        if (EED_FLASH_START_ADDRESS  == altSector)
        {
            /* if the start of emulation address has been reached,
               then set sectorAddress to the last sector of emulation */
            sectorAddress = EED_FLASH_END_ADDRESS - EED_SECTOR_SIZE + 1 ;
        }
        else
        {
            sectorAddress = altSector - EED_SECTOR_SIZE;
        }

        #if (SGF18_SELECT == S12XS_SGF18)
        /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
           address to logical address, to read the D-Flash */
          returnCode = FSL_ReadDFlash(sectorAddress);
        #else
        /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
          returnCode = READ_DFLASH16(sectorAddress);
        #endif

        /* The first active sector before the alternative sectors is the latest Active sector */
        while ( SECTOR_STATUS_ACTIVE != returnCode )
        {
            if (EED_FLASH_START_ADDRESS  == sectorAddress)
            {
                /* if the start of emulation address has been reached,
                   then set sectorAddress to the last sector of emulation */
                sectorAddress = EED_FLASH_END_ADDRESS - EED_SECTOR_SIZE + 1 ;
            }
            else
            {
                sectorAddress = sectorAddress - EED_SECTOR_SIZE;
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
        freeSpaceAddress = sectorAddress + EED_SECTOR_SIZE - EED_SECTOR_WASTE ;
    }

    /* Check if the number of active sectors present is more than EED_ACTIVE_SECTORS */
    if(gActiveCount > EED_ACTIVE_SECTORS )
    {
        /* If the number of active sectors present is more than EED_ACTIVE_SECTORS,
           then the oldest sector which has been compressed has to be made alternative */
        sectorAddress = currentActiveSector;
        eraseFlag = TRUE;
        sStatusToMain = EED_SECTOR_ALTERNATIVE;

        /* Check the sector status for partial programming */
        returnCode = FSL_CheckMarginLevel(sectorAddress);
        if(EED_NOT_OK == returnCode)
        {
            /* If the lastest active sector has a partial programming error,
               then this sector is initialized as ALTERNATIVE */
            gAddressToMain = sectorAddress;

            /* The previous active sector before this is the current ACTIVE sector */
            if(EED_FLASH_START_ADDRESS == currentActiveSector)
            {
                /* if the start of emulation address has been reached,
                   then set sectorAddress to the last sector of emulation */
                sectorAddress = EED_FLASH_END_ADDRESS - EED_SECTOR_SIZE + 1;
            }
            else
            {
                sectorAddress = currentActiveSector - EED_SECTOR_SIZE;
            }

            #if (SGF18_SELECT == S12XS_SGF18)
            /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
               global address to logical address, to read the D-Flash */
              returnCode = FSL_ReadDFlash(sectorAddress);
            #else
            /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
              returnCode = READ_DFLASH16(sectorAddress);
            #endif

            /* The first active sector before the alternative sectors is the latest Active sector */
            while ( SECTOR_STATUS_ACTIVE != returnCode )
            {
                if(EED_FLASH_START_ADDRESS == sectorAddress)
                {
                    /* if the start of emulation address has been reached,
                       then set sectorAddress to the last sector of emulation */
                    sectorAddress = EED_FLASH_END_ADDRESS - EED_SECTOR_SIZE + 1;
                }
                else
                {
                    sectorAddress = sectorAddress - EED_SECTOR_SIZE;
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
            freeSpaceAddress = sectorAddress + EED_SECTOR_SIZE - EED_SECTOR_WASTE;
        }
        else
        {
            /* The status is programmed correctly */
            /* Find the oldest ACTIVE sector and initialize it as ALTERNATIVE */

            returnCode = EED_ZEROS_16;

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
                   address to logical address, to read the D-Flash */
                  returnCode = FSL_ReadDFlash(sectorAddress);
                #else
                /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
                  returnCode = READ_DFLASH16(sectorAddress);
                #endif
            }

            /* If the oldest active sector is initialized as ALTERNATIVE */
            gAddressToMain = sectorAddress;
        }
    }
    else
    {
         tempValue = EED_ZEROS_16;

        /* Check the last record written for partial programming */
        tempAddress = freeSpaceAddress - EED_DATA_ID_SIZE;
        returnCode = FSL_CheckMarginLevel(tempAddress);
        if(EED_NOT_OK == returnCode)
        {
            /* Partial programming error. Invalidate the record by programming the record ID as 0x0000 */
            /* Program the data ID */
            returnCode = FSL_Program(tempAddress, EED_DATA_ID_SIZE, &tempValue);
            if ( EED_OK != returnCode )
            {
                return(returnCode);
            }
        }
        /* The status is programmed correctly */
    }

   if(FALSE == eraseFlag)
   {
       #if EED_CACHETABLE_ENABLE == ON
         FSL_UpdtCacheTable ();
       #endif
       return (EED_OK);
   }

   /*** Else set the parameters for FSL_Main as Erase is required ***/
    /* Get the erasing cycles of the the current EE sector */
    gEraseCycles = FSL_GetErasingCycles (gAddressToMain, &errorGettingEC);

    /*set the job to be done by main as Init*/
    gJobToMain = INIT;
    /*set global init status flag as started*/
    EE_Status = BUSY;

    /* The erasing cycles have an ECC error or the erasing cycles are lost
       due to brownout during erase */
    if((TRUE == errorGettingEC) || (EED_ERASED_STATE16 == gEraseCycles))
    {
        /* The erasing cycles have an ECC error or the erasing cycles are lost
           due to brownout during erase */
        /* Read the erasing cycles of the previous sector */
        if(EED_FLASH_START_ADDRESS == sectorAddress)
        {
            /* if current sector is first sector, get the erasing cycles of last sector */
            gEraseCycles = FSL_GetErasingCycles((EED_FLASH_END_ADDRESS - EED_SECTOR_SIZE + 1), &errorGettingEC);
        }
        else
        {
            /* get erasing cycles of previous sector */
            gEraseCycles = FSL_GetErasingCycles((sectorAddress - EED_SECTOR_SIZE), &errorGettingEC);
        }
    }
    if(EED_MAX_ERASING_CYCLE_VALUE == gEraseCycles)
    {
        gEraseCycles = EED_ZEROS_16;
    }
    else
    {
        gEraseCycles++;
    }

    returnCode = DFlashErase(gAddressToMain);
    if (returnCode != SGF_OK)
    {
       EE_Status = FAIL;
    }
    return(returnCode);
}

//!<====================================================================================================
//!<  added by xyl
//!<====================================================================================================
void myFuncInitEeprom(void)
{
  FSL_InitEeprom();

  while(BUSY == EE_Status)
  {
      FSL_Main();//由BSW调度器周期性调用
  }
}


/***********************************************************************
*
*  Function Name    : FSL_ReadEeprom
*  Description      : This function is to read the specific data record.
                      The starting address of the record data will be
                      returned.
*  Arguments        : dataID,*recordAddr
*  Return Value     : UNIT16
*
***********************************************************************/
UINT16 FSL_ReadEeprom (UINT16 dataID, UINT16 *SSD_SGF18_NEAR recordAddr)
{
    UINT16 returnCode;

    /* Check if the ID is in the range or equal to zero */
    if((EED_MAX_RECORD_NUMBER < dataID) || (EED_ZEROS_16 == dataID))
    {
        return(EED_ERROR_IDRNG);
    }
    else
    {
        /* search data for the record */
        returnCode = FSL_SearchLoop(dataID, recordAddr);
        return (returnCode);
    }
}

/***********************************************************************
*
*  Function Name    : FSL_WriteEeprom
*  Description      : This function will be used to encapsulate the user
                      data in a record and write it to the emulated
                      EEPROM. If there is no enough free space in active
                      sector, this main routine, which is invoked periodically
                      shall check if the next sector available is an
                      'active sector'. Otherwise,this routine will initiate
                      sector swapping to clean up the EEPROM.
*  Arguments        : dataID,dataPtr
*  Return Value     : UNIT16
*
***********************************************************************/
UINT16 FSL_WriteEeprom (UINT16 dataID, UINT16 *SSD_SGF18_NEAR dataPtr)
{
    UINT16 returnCode;

    /* If CallBack is enabled, then serve callback function */
    #if(CALLBACK_ENABLE == TRUE)
        CallBack();
    #endif

    /* Check if the ID is within the range or equal to zero */
    if((EED_MAX_RECORD_NUMBER < dataID) || (EED_ZEROS_16 == dataID))
    {
        return(EED_ERROR_IDRNG);
    }

    /* copying the data ID and the data Values into the strcuture */
    gDataRecord.dataID = dataID;
    //!<==============================================================
    //!<---------added by xyl-----------
    /*gDataRecord.dataID1 = 0;
    gDataRecord.dataID2 = 0;
    gDataRecord.dataID3 = 0;*/
    //!<==============================================================

    /* copying the dataPtr into the strucutre */
    (gDataRecord.dataPtr) = dataPtr;

    /* calculate free space available in the current active sector */
    /* check if the space is enough to write a record */
    if(EED_RECORD_LENGTH <= ((currentActiveSector+EED_SECTOR_SIZE) - freeSpaceAddress))
    {
        /* No need for swap. just write the record and return */
        returnCode = FSL_CopyRecord(gDataRecord, freeSpaceAddress);
        if ( EED_OK != returnCode )
        {
            return(returnCode);
        }
        else
        {
            freeSpaceAddress += EED_RECORD_LENGTH;
            return(returnCode);
        }
    }
    else
    {
        /* check if a swap is necessary */
        if ((currentActiveSector + EED_SECTOR_SIZE) < EED_FLASH_END_ADDRESS )
        {
            returnCode = FSL_SectorStatus(currentActiveSector + EED_SECTOR_SIZE);

            if(EED_SECTOR_ACTIVE == returnCode)
            {
                /* make this the current active sector */
                currentActiveSector = currentActiveSector + EED_SECTOR_SIZE;
                freeSpaceAddress = currentActiveSector + EED_SECTOR_INFO_SIZE;
                /* write the new record in this sector */
                returnCode = FSL_CopyRecord(gDataRecord, freeSpaceAddress);

                if ( EED_OK != returnCode )
                {
                    return(returnCode);
                }
                else
                {
                    freeSpaceAddress += EED_RECORD_LENGTH;
                    return(returnCode);
                }
            }
            /* contents of the old sector should be swapped to the oldest alternative sector */
        }
        sCounter = 1;
        returnCode = FSL_SwapSector();
        if ( EED_OK != returnCode )
        {
            return(returnCode);
        }
    }
}

/***********************************************************************
*
*  Function Name    : FSL_ReportEepromStatus
*  Description      : This function reports statistics like active
                      emulation sector erasure cycles, check the
                      emulation sector status.
*  Arguments        : erasingCycles
*  Return Value     : UNIT16
*
***********************************************************************/
UINT16 FSL_ReportEepromStatus(UINT16 *SSD_SGF18_NEAR erasingCycles)
{
    UINT16 returnCode;

    /* check the status of the currrent active sector */
    returnCode = FSL_SectorStatus(currentActiveSector);
    if(EED_SECTOR_ACTIVE != returnCode )
    {
        return(EED_ERROR_SSTAT);
    }
    else
    {
        #if (SGF18_SELECT == S12XS_SGF18)
        /* If the S12XS family is used, call the FSL_ReadDFlash function which converts the
           address to logical address, to read the D-Flash */
          *erasingCycles = FSL_ReadDFlash(currentActiveSector + EED_SECTOR_STATUS_SIZE);
        #else
        /* If the S12P/S12HY family is used, the macro READ_DFLASH16 is used the read the D-Flash */
          *erasingCycles = READ_DFLASH16(currentActiveSector + EED_SECTOR_STATUS_SIZE);
        #endif

        return(EED_OK);
    }
}

/***********************************************************************
*
*  Function Name    : FSL_DeinitEeprom
*  Description      : This function is to release all the Flash used to
                      EEPROM emulation. After de-initialized, the Flash
                      pages for emulation will be fully erased.
*  Arguments        : None
*  Return Value     : UNIT16
*
***********************************************************************/
UINT16 FSL_DeinitEeprom (void)
{
    UINT16 returnCode;
    /* erase the sectors used for emulation */
    gJobToMain = DEINIT;
    /* Initilize the global counter to 1 */
    sCounter = 1;
    EE_Status = BUSY;
    returnCode = DFlashErase(EED_FLASH_START_ADDRESS);
    if (returnCode != SGF_OK)
    {
        EE_Status = FAIL;
    }
    return(returnCode);
}

/***********************************************************************
*
*  Function Name    : FSL_Main()
*  Description      : Completes the initialization of sectors and other
                      operations to make it ready for EEPROM emulation,
                      after an erase is initiated by Init, Write or
                      Deinit APIs. The case IDLE is redundant and has been
                      provided for readability.
*  Arguments        : None
*  Return Value     : None
*
***********************************************************************/
void FSL_Main(void)//由BSW调度器周期性调用
{
    UINT16 tempAddress;
    UINT16 returnCode;

    if(DONE == eraseStatusFlag )
    {
        eraseStatusFlag = IDLE;
        switch(gJobToMain)
        {

            case INIT:
                    /* Erase the whole sector */
                    /* EE sector can be multiple of DFlash Sector */
                    if( sCounter < (EED_SECTOR_SIZE / DFLASH_SECTOR_SIZE))
                    {
                        /* get the address of the next Dflash sector to be erased*/
                        tempAddress = gAddressToMain + (sCounter * DFLASH_SECTOR_SIZE);

                        sCounter ++;
                        returnCode = DFlashErase(tempAddress);
                        if(returnCode != EED_OK)
                        {
                            gEE_Error = returnCode;
                            EE_Status = FAIL;
                            gInitFirstTime = FALSE;
                        }
                        break;
                    }
                    else
                    {
                        returnCode = FSL_PgmSectorStatus(gAddressToMain,sStatusToMain,gEraseCycles,TRUE);
                        if((EED_OK == returnCode) || (EED_DEAD_SECTOR == returnCode))
                        {
                            if(TRUE == gInitFirstTime )
                            {
                                gAddressToMain += EED_SECTOR_SIZE;
                                returnCode = FSL_InitFirstTime();
                                if(returnCode != EED_OK)
                                {
                                    gEE_Error = returnCode;
                                    EE_Status = FAIL;
                                    gJobToMain = IDLE;
                                    gInitFirstTime = FALSE;
                                    break;
                                }
                                /* set the values pertaining to the current active sector */
                                currentActiveSector = EED_FLASH_START_ADDRESS;
                                freeSpaceAddress = EED_FLASH_START_ADDRESS + EED_SECTOR_INFO_SIZE;
                                EE_Status = DONE;
                                gJobToMain = IDLE;
                                gInitFirstTime = FALSE;
                            }
                            else
                            {
                                #if EED_CACHETABLE_ENABLE == ON
                                  FSL_UpdtCacheTable ();
                                #endif
                                EE_Status = DONE;
                                gJobToMain = IDLE;
                            }
                        }
                        else
                        {

                            gEE_Error = returnCode;
                            EE_Status = FAIL;
                            gJobToMain = IDLE;
                            gInitFirstTime = FALSE;
                            break;
                        }
                    }
                    break;

            /*the state checks for erase initiated by SwapSector() and if there is
            sufficient space it writes the record*/
            case SWAP_CLUSTER:

                    /*if more dflash blocks of the cluster have to be erased*/
                    if( sCounter < (EED_SECTOR_SIZE / DFLASH_SECTOR_SIZE))
                    {
                        /* get the address of the next Dflash sector to be erased*/
                        tempAddress = gAddressToMain + (sCounter * DFLASH_SECTOR_SIZE);

                        sCounter++;
                        /*erase the next sector of the cluster*/
                        returnCode = DFlashErase(tempAddress);
                        if(returnCode != EED_OK)
                        {
                            gEE_Error = returnCode;
                            EE_Status = FAIL;
                            gJobToMain = IDLE;
                            break;
                        }
                    }
                    /*erase verify*/
                    else
                    {
                        /*erase verify and program old cluster as alternative or dead*/
                        returnCode = FSL_PgmSectorStatus(gAddressToMain,EED_SECTOR_ALTERNATIVE,gEraseCycles,/*erase verify*/TRUE);
                        if ( EED_OK != returnCode )
                        {
                            gEE_Error= returnCode;
                            EE_Status = FAIL;
                            gJobToMain = IDLE;
                            break;
                        }

                        /* If there is sufficient space for the new record, program the status word
                           and write the record*/
                        if(EED_RECORD_LENGTH <= ((currentActiveSector+EED_SECTOR_SIZE) - freeSpaceAddress))
                        {
                            /* No need for swap. just write the record and return */
                            returnCode = FSL_CopyRecord(gDataRecord, freeSpaceAddress);
                            if ( EED_OK != returnCode )
                            {
                                gEE_Error= returnCode;
                                EE_Status = FAIL;
                                gJobToMain = IDLE;
                            }
                            else
                            {
                                freeSpaceAddress += EED_RECORD_LENGTH;
                                /*set job status as Done */
                                EE_Status = DONE;
                                gJobToMain = IDLE;
                            }
                        }
                        /*else if there is insufficient space initiate a swap cluster
                        and erase the old active cluster*/
                        else
                        {
                            sCounter = 1;
                            gEE_Error = FSL_SwapSector();
                        }
                    }
                    break;

            case DEINIT:
                    if( sCounter < ((EED_SECTORS_ALLOTED * EED_SECTOR_SIZE)/DFLASH_SECTOR_SIZE))
                    {
                        /* get the address of the next Dflash sector to be erased*/
                        tempAddress = EED_FLASH_START_ADDRESS + (sCounter * DFLASH_SECTOR_SIZE);

                        sCounter ++;
                        returnCode = DFlashErase(tempAddress);
                        if(returnCode != SGF_OK)
                        {
                            gEE_Error = returnCode;
                            EE_Status = FAIL;
                            gJobToMain = IDLE;
                            break;
                        }
                    }
                    else
                    {
                        returnCode = DFlashEraseVerify(EED_FLASH_START_ADDRESS,(EED_SECTORS_ALLOTED * EED_SECTOR_SIZE));
                        if(returnCode != SGF_OK)
                        {
                            gEE_Error = EED_ERROR_NOT_BLANK;
                            EE_Status = FAIL;
                            gJobToMain = IDLE;
                            break;
                        }
                        EE_Status = DONE;
                        gJobToMain = IDLE;
                    }
                    break;
            case IDLE: /* No job to be done */
            default:
                    /*Do nothing*/
                    break;

         }/* End of Switch case */
    }
    else if(FAIL == eraseStatusFlag )
    {
        /* An error occured while erasing the sector */
        /* Set the status to FAIL. gEE_Error contains the error code returned by erase */
        EE_Status = FAIL;

    }
    else
    {
        /*Do nothing*/
    }
}


/***********************************************************************
*
* End of file EED_HighLevel.c
*
***********************************************************************/