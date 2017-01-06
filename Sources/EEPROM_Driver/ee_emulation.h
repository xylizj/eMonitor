/*************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2009, All Rights Reserved   *
*************************************************************************/

/************************************************************************
*                                                                       *
*             EEPROM Emulation Driver for S12XS/S12P/S12HY              *
*                                                                       *
* FILE NAME       ee_emulation.h                                        *
* DATE            02.25.2009                                            *
* AUTHOR          Infosys Team, Mysore                                  *
* EMAIL           r56611@freescale.com                                  *
************************************************************************/

/******************************* CHANGES *******************************
 0.0.0a   06.12.2007      Sindhu R01       Initial Version
 0.0.1    08.03.2007      Sindhu R01       Updated to support the
                                           Cosmic compiler
 0.0.2    10.17.2007      Sindhu R01       Added macro
                                           EED_MAX_ERASING_CYCLE_VALUE
 0.1.0    05.02.2008      Sindhu R01       Added the function prototype of
                                           FSL_CheckMarginLevel() and a few
                                           macros to fix partial programming
                                           and dead sector errors
 0.2.0    10.28.2008      Arvind Awasthi,  Added macros and function declarations
                          Vikas Kothari,   of the new functions added
                          Sindhu R01       to make the EED driver interrupt based.
 0.2.1    11.08.2008      Sindhu R01       Changed the prototype of the
                                           FSL_UpdtCacheTable()
 0.2.2    02.04.2009      Jiben Prakash,   Modified the Header file to support
                          Vikas Kothari.   mutiple byte programming.
                                           Added macro EED_MIN_PROG_SIZE,
                                           The structure variable dataVal changed
                                           to a pointer.
 0.2.3    02.25.2008      Sindhu R01       Changed the prototypes of the
                                           functions
***********************************************************************/

#ifndef _EE_EMULATION_H_
#define _EE_EMULATION_H_

#include "SSD_Types.h"
#include "SSD_SGF18.h"
#include "SSD_SGF18_Internal.h"

/* structure declaration */

typedef struct
{
    UINT16  dataID;    /* the unique data ID */
    //UINT16  dataID1;   //added by xyl just take a place
    //UINT16  dataID2;   //added by xyl just take a place
    //UINT16  dataID3;   //added by xyl just take a place
    UINT16 *SSD_SGF18_NEAR dataPtr;   /* the data array of 2 bytes each */
}DATA_RECORD;

#define OFF   0x0
#define ON   0x1

//!<=============================================================================================================
//!<=============================================================================================================
/*****************************************************************************
* User Configurable Macros
*****************************************************************************/

/* Macro to enable or disable Cache Table */
///#define EED_CACHETABLE_ENABLE       ON
#define EED_CACHETABLE_ENABLE       OFF//modified by xyl 2014.10.18

/* EEPROM size - user configurable*/ /*stored as a sum of all data in data records*/
//#define EED_EEPROM_SIZE             0x16E /*stored as a sum of all data in data records*/
#define EED_EEPROM_SIZE             1*(256-6)/5*4 //600 /*stored as a sum of all data in data records*/

/* Number of alternative sectors needed for Round Robin scheme */
//#define EED_ACTUAL_READY_SECTORS    0x2 /* This should be a minimum of 2 */
#define EED_ACTUAL_READY_SECTORS    0x0A /* This should be a minimum of 2 */

/* Number of extra alternative sectors needed to overcome the problem of DEAD sectors*/
#define EED_EXTRA_READY_SECTORS     0x1 /* This should be a minimum of 1 */

/* Number of extra active sectors needed for emulation */
#define EED_EXTRA_ACTIVE_SECTORS     0x0

/* Total number of alternative sectors used */
/* This should be a minimum of 3 */
#define EED_READY_SECTORS           (EED_ACTUAL_READY_SECTORS+EED_EXTRA_READY_SECTORS)

/* Start Address of Flash for EEPROM emulation. */
#if (SGF18_SELECT == S12XS_SGF18) /* If S12XS family */

/* For S12XS D-Flash start addr is 0x00100000 */
  #define EED_FLASH_START_ADDRESS       0x0000

#elif (SGF18_SELECT == S12P_SGF18) /* If S12P family */

/* For S12P D-Flash start addr is 0x00004400 */
  #define EED_FLASH_START_ADDRESS       0x4400

#elif (SGF18_SELECT == S12HY_SGF18) /* If S12HY family */

/* For S12HY D-Flash start addr is 0x00004400 */
  #define EED_FLASH_START_ADDRESS       0x4400

#endif

/* The length of the data corresponding to a data ID, stored in a data record. */
//#define EED_DATA_VALUE_SIZE         (0x6)
#define EED_DATA_VALUE_SIZE         (0x8) //modified by xyl

/* Macro to determine the number of IDs to be stored in Cache table */
#if EED_CACHETABLE_ENABLE == ON
  #define EED_MAX_CACHETABLE_ENTRY        0x8       /* Number of Cache Table entries */
  #define EED_CACHETABLE_START_ADDRESS    0x3FD0    /* start address of the cache table in RAM */
  #define EED_CTABLE_ITEM_SIZE            0x02      /* Cache Table size is 2 */
#endif

/* EED Sector size */
#define EED_SECTOR_SIZE             0x0100

/**** End of User Configurable Macros ****/
//!<=============================================================================================================
//!<=============================================================================================================
    


/* The minimum number alternative of sectors that should be
   allotted for emulation Eeprom */
#define EED_MIN_READY_SECTORS          0x03

#if EED_READY_SECTORS < EED_MIN_READY_SECTORS

    #error "EED_READY_SECTORS (Number of alternative sectors alloted for emulation Eeprom) is not enough!"

#endif

/* Return code definition */
#define EED_OK                      0x0000
#define EED_ERROR_NOT_BLANK         0x0001
#define EED_ERROR_SIZE              0x0002
#define EED_ERROR_NOFND             0x0003
#define EED_ERROR_RANGE             0x0004
#define EED_ERROR_SSTAT             0x0005
#define EED_ERROR_VERIFY            0x0006
#define EED_ERROR_IDRNG             0x0007
#define EED_ERROR_ACCERR            0x0008
#define EED_ERROR_PVIOL             0x0010
#define EED_ERROR_MGSTAT0           0x0020
#define EED_ERROR_MGSTAT1           0x0040
#define EED_ERROR_INVALIDCLK        0x0800

/* sector state value */
#define SECTOR_STATUS_ERASED        0xFFFF
#define SECTOR_STATUS_ACTIVE        0xFACF
#define SECTOR_STATUS_DEAD          0x0000

/* Macro to denote(indicate) the Sector Status */
#define EED_SECTOR_ACTIVE           0x0000
#define EED_SECTOR_ALTERNATIVE      0x5555
#define EED_SECTOR_BLANK            0xFFFF
#define EED_SECTOR_UPDATE           0xAAAA
#define EED_SECTOR_INVALID          0x5A5A
#define EED_SECTOR_DEAD             0xA5A5

/* Macro to denote the Record Status */
#define EED_RECORD_VALID            0x0000
#define EED_RECORD_BLANK            0xFFFF
#define EED_RECORD_INVALID          0xA5A5

/*****************************************************************************
* Macros related to Sector's status information size
*****************************************************************************/
#define EED_SECTOR_STATUS_SIZE      0x02
//#define EED_SECTOR_STATUS_SIZE      0x04  //modified by xyl

#define EED_ERASING_CYCLES_SIZE     0x02

#define EED_SECTOR_INFO_SIZE        (EED_ERASING_CYCLES_SIZE+EED_SECTOR_STATUS_SIZE)
//#define EED_SECTOR_INFO_SIZE        (EED_ERASING_CYCLES_SIZE+EED_SECTOR_STATUS_SIZE+12)//modified by xyl


/*****************************************************************************
* Macros related to data record size
*****************************************************************************/
/* Data ID Size */
#define EED_DATA_ID_SIZE            0x02
//#define EED_DATA_ID_SIZE            0x08     //modified by xyl
//#define EED_RECORD_STATUS_SIZE            0x04     //added by xyl

/* Data Record Size  */
#define EED_RECORD_LENGTH           (EED_DATA_ID_SIZE+EED_DATA_VALUE_SIZE)

/* Number of Data Records is calculated if we know the total Data Size. */
 #if (EED_EEPROM_SIZE % EED_DATA_VALUE_SIZE == 0)
   #define EED_MAX_RECORD_NUMBER      (EED_EEPROM_SIZE/EED_DATA_VALUE_SIZE)
 #else
   #define EED_MAX_RECORD_NUMBER      ((EED_EEPROM_SIZE/EED_DATA_VALUE_SIZE)+1)
 #endif

/****************************************************************************
* Macro to determine the total number of sectors alloted for EEPROM Emulation.
****************************************************************************/

/* Number of records that can be stored in a sector */
#define EED_SECTOR_CAPACITY       ((EED_SECTOR_SIZE-EED_SECTOR_INFO_SIZE)/EED_RECORD_LENGTH)

/* Number of bytes in a sector that are not used for emulation */
#define EED_SECTOR_WASTE          (EED_SECTOR_SIZE- (EED_SECTOR_INFO_SIZE+(EED_RECORD_LENGTH*EED_SECTOR_CAPACITY)))

/* Number of sectors required to store the specified EEPROM size */
#if ((EED_MAX_RECORD_NUMBER+1) % EED_SECTOR_CAPACITY == 0)

  #define  EED_ACTIVE_SECTOR_REQUIRED    ((EED_MAX_RECORD_NUMBER+1)/EED_SECTOR_CAPACITY)

#else

  #define EED_ACTIVE_SECTOR_REQUIRED     (((EED_MAX_RECORD_NUMBER+1)/EED_SECTOR_CAPACITY) + 1)

#endif

/* Total number of ACTIVE sectors alloted */
#define EED_ACTIVE_SECTORS        (EED_EXTRA_ACTIVE_SECTORS+EED_ACTIVE_SECTOR_REQUIRED)

/* Total number of sectors alloted will also include some 'alternative sectors' */
#define EED_SECTORS_ALLOTED       (EED_ACTIVE_SECTORS+EED_READY_SECTORS)


/* End Address of Flash from EEPROM emulation. */
#define EED_FLASH_END_ADDRESS          (EED_FLASH_START_ADDRESS + (EED_SECTORS_ALLOTED*EED_SECTOR_SIZE)-1)

#define EED_MAX_ERASING_CYCLE_VALUE    0xFFFE

/* The mininum programmable Data size, 2 bytes */
#define EED_MIN_PROG_SIZE           (0x02)

/* Macros used internally */
#define EED_BLANK_RECID           0xFFFF
#define EED_DEAD_SECTOR           0x6666
#define EED_ERASED_STATE16        0xFFFF
#define EED_ERASED_STATE32        0xFFFFFFFF
#define EED_ZEROS_16              0x0000
#define EED_ZEROS_32              0x00000000
#define EED_ONE                   0x0001
#define EED_NOT_OK                0x8888

/***************Globals and macros in EED for Interrupt support*************/
/*states for main function*/
#define INIT                            (0x01)
#define SWAP_CLUSTER                    (0x02)
#define DEINIT                          (0x03)

/* Flag to keep track of Init State*/
extern EE_StateType EE_Status;
extern UINT8 gJobToMain;
extern UINT8 gActiveCount;
extern UINT8 gAltCount;
/* Global variable to keep track of which cluster is being erased */
extern UINT16 gAddressToMain;

/* Global Variable Declaration */
extern BOOL   gInitFirstTime;

/* global variable to keep track of error code*/
extern VUINT16 gEE_Error;

/************************************************************************/

/* Global variables */
extern UINT16 currentActiveSector;/* Start address of the current active sector */

extern UINT16 freeSpaceAddress;/* The blank address of the current active sector
                                where the next data record is written */
/*record used in copying data between write, main and swap cluster*/
extern DATA_RECORD gDataRecord;
/*stores erase cycles before erasing the cluster*/
extern UINT16 gEraseCycles;

/* function declarations */
/* middle level functions */
extern UINT16 FSL_Program (UINT16 dest,UINT16 size,UINT16 *SSD_SGF18_NEAR srcPtr);
extern UINT16 FSL_CopyRecord(DATA_RECORD dataRecord, UINT16 dest);
extern UINT16 FSL_SwapSector(void);
extern UINT16 FSL_SectorStatus (UINT16 sectorAddress);
extern UINT16 FSL_CheckMarginLevel (UINT16 readAddress);
extern UINT16 FSL_GetErasingCycles (UINT16 sectorAddress,BOOL *SSD_SGF18_NEAR pErrorGettingEC);
extern UINT16 FSL_PgmSectorStatus (UINT16 sectorAddress, UINT16 status, UINT16 erasingCycles, BOOL ersVerify);
extern UINT16 FSL_InitFirstTime(void);
extern UINT16 FSL_SearchRecord (UINT16 dataID, UINT16 sectorAddress, UINT16 *SSD_SGF18_NEAR recordAddress);
extern UINT16 FSL_SearchLoop (UINT16 dataID, UINT16 *SSD_SGF18_NEAR recordAddress);
extern UINT16 FSL_ReadRecord (UINT16 *SSD_SGF18_NEAR dataID,UINT16 readAddress);
//extern void SSD_SGF18_NEAR FSL_ISR_Handler(void);
extern void myFuncInitEeprom(void);//added by xyl

/* If cache table is enabled */
#if EED_CACHETABLE_ENABLE == ON
extern void FSL_UpdtCacheTable (void);
#endif

/* If S12XS family */
#if (SGF18_SELECT == S12XS_SGF18)
  extern UINT16 FSL_ReadDFlash (UINT16 destination);
#endif

/* high level functions */
extern UINT16 FSL_InitEeprom (void);
extern UINT16 FSL_WriteEeprom (UINT16 dataID, UINT16 *SSD_SGF18_NEAR dataPtr);
extern UINT16 FSL_DeinitEeprom (void);
extern UINT16 FSL_ReadEeprom (UINT16 dataID, UINT16 *SSD_SGF18_NEAR recordAddr);
extern UINT16 FSL_ReportEepromStatus(UINT16 *SSD_SGF18_NEAR erasingCycles);
extern void FSL_Main(void);

#endif /*_EE_EMULATION_H_*/
