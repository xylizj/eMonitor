#include <mc9s12hy64.h>  
#include "Platform_Types.h"  
#include "Adc.h"

        
#define  ADC_ERROR_VALUE   0xFFFF


void Adc_Init(void) 
{
  /*Wrap Around Channel Select*/
  /* ADC0:Wraparound to AN0 after Converting AN7 */
  ATDCTL0 = 0x07; 

  /*A/D Resolution=12bits,no External Trigger,No discharge before sampling*/

  /*ADC0: resolution is 12bits*/
  ATDCTL1|= ATDCTL1_SRES1_MASK;
  ATDCTL1&=(~ATDCTL1_SRES0_MASK);

  /*AFFC=1,ICLKSTP=0,ETRIGLE=0,ETRIGP=0,ETRIGE=0,ASCIE=0,ACMPIE=0.*/
  /*AFFC=1:Read access to the result register will cause the associated
  CCF[n] to clear automatically*/
  ATDCTL2 = 0x40;

  /*DJM=1,S8C=0,S4C=0,S2C=0,S1C=0,FIFO=0,FRZ[1:0]=10*/
  /*Right justified data in the result registers;
  The number of conversions per sequence is 8 ;
  Background debug freeze application:Finish current conversion, then freeze ;
  Conversion results are placed in the corresponding result register */          
  ATDCTL3 = 0x82; 

  /*ATDnCTL4= RESET => ATDCLK Frequence=BUS Frequence/12,Sample Time=4*ATD Clock Cycle*/
  //ATDCTL4 = 0xE7;
  ATDCTL4 = 0xE3;  
  
  /*SC=0,SCAN=1,MULI=1,CD=0,CC=0,CB=0,CA=0*/    //AN0作为第一个转换通道；
  /*Scan Mode,Multi-Channel Sample ,First Channel is AN0*/
  ATDCTL5 = 0x30;
}





uint16 Adc_Read(adc_chan_type channel_id)
{
  uint16 result;
  uint8  offset;
  if(channel_id <= 15)     //Channel ID of ADC0 and ADC1
  {                        // 0--15 for AN0--AN15 of ADC0 
    offset = channel_id;
    result = (uint16)(*(uint16*)(ADC_DR0_ADDR + offset)); 
  }
  else
  {
    result = ADC_ERROR_VALUE;
  }
  return(result);
}



 

