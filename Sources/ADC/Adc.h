
#ifndef _ADC_H_
#define _ADC_H_

#include "Platform_Types.h"



typedef enum _adc_chan_tag
{
  BAT_chan = 6,
  T15_chan = 7,
}adc_chan_type;



#define BAT_ON() {PTR_PTR0 = 1; DDRR_DDRR0 = 1;}
#define BAT_OFF() {PTR_PTR0 = 0; DDRR_DDRR0 = 1;}


#define T15_THREHOLD_VOLT       5//unit V
#define T15_THREHOLD_ADC_VALUE  ((4096*T15_THREHOLD_VOLT/11)/5)  //resolution is 12bits


/*********** ADC Configure ********************/
#define  STD_ON    1      
#define  STD_OFF   0    
   
#define ADC_DEV_ERROR_DETECT   STD_OFF

#define ADC_DR0_ADDR  ((volatile uint16*) 0x00000080)

/*********** Function Define  ********************/
extern void Adc_Init(void);
extern uint16 Adc_Read(adc_chan_type channel_id);
extern void T15Down_PostProcess (void);  //需要处理不能成功下电的情况






















#endif /* #ifndef _ADC_H_ */

      
