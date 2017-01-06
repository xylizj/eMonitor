#include <hidef.h>      /* common defines and macros */
#include <mc9s12hy64.h>     /* derivative information */

#include "MCU.h"
#include "mscan12.h"
#include "OS.h"
#include "lcd.h"
#include "Adc.h"
#include "monitor.h"
#include "ee_emulation.h"
#include "NormalDemo.h"

UINT16  lastID;


void read_last(void)
{
  UINT8 i;

  lastID = READ_DFLASH16(freeSpaceAddress-2);

  if((EED_MAX_RECORD_NUMBER < lastID) || (EED_ZEROS_16 == lastID))
  {
      return;
  }
    
  //if(lastID > 0)
  {
    for(i=0; i<EED_DATA_VALUE_SIZE; i+=EED_MIN_PROG_SIZE)
    {
      ((UINT16 *)(can_msgs_buf_array[RESPONSE_MSG_SEND].pbuffer))[i/EED_MIN_PROG_SIZE] = READ_DFLASH16(freeSpaceAddress - EED_RECORD_LENGTH + i);
    } 
  }
  
  (void)can_tx_msg(RESPONSE_MSG_SEND);
}


void main(void) 
{
  DisableInterrupts;
  
  setbusclock();
  BAT_ON();    
  Adc_Init();     
  CAN_Init();
  init_port(); 
  lcd_init();
  lcd_clr();
  dis_pic_12864(main_menu[0]);

  myFuncInitEeprom();
  read_last();
     
  EnableInterrupts;

  for(;;) 
  { 
    can_rx_msg();
    can_period_handle(FALSE); //Task background--real time
    TimeManage();             //Task background*/
    
    Task_KeyScan();  //10ms 
    Task_KeyHandle();//100ms     
    Task_Display();   //1000ms
    Task_T15Monitor();
    //Task_FSL_Main(); 
  } 
}




