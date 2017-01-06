#ifndef _MCU_H_
#define _MCU_H_

//#include <hidef.h>      /* common defines and macros */
#include <mc9s12hy64.h>     /* derivative information */
#include "Platform_Types.h"

#define RTI_INT_ENABLE() {CPMUINT |= CPMUINT_RTIE_MASK;}
#define RTI_INT_DISABLE() {CPMUINT &= ~ CPMUINT_RTIE_MASK;}

extern void setbusclock(void);

extern volatile uint16 RTI_Cnt;


 //extern void RTI1ISR(void);















































#endif /* #ifndef _MCU_H_ */




