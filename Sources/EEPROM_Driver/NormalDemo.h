/************************************************************************
* (c) Copyright Freescale Semiconductor, Inc 2007, All Rights Reserved  *
************************************************************************/

/************************************************************************
*               EEPROM Emulation Driver for S12XS/S12P/S12HY            *
*                                                                       *
*   FILE NAME     :  NormalDemo.h                                       *
*   DATE          :  06.09.2008                                         *
*   AUTHOR        :  Infosys Team, Mysore                               *
*   EMAIL         :  r56611@freescale.com                               *
*************************************************************************/

/****************************** CHANGES *********************************
* 0.0a   06.21.2007      Sindhu R01       Draft Version
* 0.1    08.10.2007      Sindhu R01       Removed Macro BDMENABLE
* 1.0    06.09.2008      Sindhu R01       Removed the Configuration
                                          parameters macro. Added marcos
                                          for data IDs and values.
*************************************************************************/

#ifndef _NORMALDEMO_H_
#define _NORMALDEMO_H_

/* Data ID's written to EED */
#define DATA_ID_ONE               0x01
#define DATA_ID_TEN               0x10

/* Data value's written to EED */
#define DATA_VALUE                0x10
#define DATA_VALUE_TEN            0xCECE

/* Prototype of error trap funciton */
extern void ErrorTrap(void);

#endif /* _NORMALDEMO_H_ */