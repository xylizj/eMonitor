
#include "MCU.h"
#include "lcd.h"

volatile uint16 RTI_Cnt  = 0;

void setbusclock(void)
{   
 
    CPMUINT  = 0x00;     //disable rti int
    CPMUFLG  = 0x80;     //clear rti flag
    CPMUPROT = 0x26;     //Disable protection of clock configuration registers 
    CPMUCLKS = 0x80;     //Enable the PLL to allow write to divider registers    
    CPMUREFDIV = 0x40;   //REFDV = 0
    CPMUSYNR = 0x03;      
    CPMUPOSTDIV = 0x00;  //pllclock=2*osc*(1+SYNR)/(1+REFDV)
    CPMUOSC = 0x80;
    CPMUPLL = 0x00;      //Set the PLL frequency modulation 
    while(CPMUFLG_UPOSC == 0U) {}  // Wait until the oscillator is qualified by the PLL
    CPMUCLKS |= 0x02; 
    CPMUPROT = 0x00;               // Enable protection of clock configuration registers 
    CPMUCOP = 0x60;                //disable watchdog
    CPMURTI = 0xB3;                //10ms once
    RTI_INT_ENABLE();
}



#pragma CODE_SEG NON_BANKED
#pragma TRAP_PROC
void near interrupt  VectorNumber_Vrti  RTI1ISR(void)
//void RTI1ISR(void)
{
     RTI_Cnt++;         
     CPMUFLG_RTIF = 1;
}
#pragma CODE_SEG DEFAULT