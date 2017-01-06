/*************************************************************************
 * (c) Copyright Freescale Semiconductor, Inc 2008, All Rights Reserved  *
 *************************************************************************

 ************************************************************************
 *                                                                      *
 *             EEPROM Emulation Driver for S12XS/S12P/S12HY             *                                                                     *
 * FILE NAME     :  SSD_Types.h                                         *
 * DATE          :  10.09.2007                                          *
 *                                                                      *
 * AUTHOR        :  Infosys Team, Mysore                                *
 * E-mail        :  r56611@freescale.com                                *
 *                                                                      *
 ************************************************************************/


/******************************* CHANGES *******************************
 0.0.0a   10.09.2007         Sindhu R01               Initial Version
***********************************************************************/

#ifndef _SSD_TYPES_H_
#define _SSD_TYPES_H_

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

typedef unsigned char BOOL;

typedef signed char INT8;
typedef unsigned char UINT8;
typedef volatile signed char VINT8;
typedef volatile unsigned char VUINT8;

typedef signed int INT16;
typedef unsigned int UINT16;
typedef volatile signed int VINT16;
typedef volatile unsigned int VUINT16;

typedef signed long INT32;
typedef unsigned long UINT32;
typedef volatile signed long VINT32;
typedef volatile unsigned long VUINT32;

#endif /* _SSD_TYPES_H_ */
