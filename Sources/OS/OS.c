#include "mc9s12hy64.h"
#include "MCU.h"
#include "OS.h"
#include "mscan12.h"
#include "lcd.h"
#include "monitor.h"
#include "KEY.h"
#include "Adc.h"
#include "Platform_Types.h"
#include "SSD_Types.h"
#include "SSD_SGF18.h"
#include "ee_emulation.h"
#include "mscan12.h"
#include "NormalDemo.h"


#define RTI_CNT_THREHOLD    0xFFFF

#define KEY_GAP             Gap_10ms 
#define FSL_Main_GAP        Gap_10ms
#define READ_T15VOLT_GAP    Gap_10ms
#define CONFIM_T15DOWN_GAP  Gap_100ms
#define KEY_HANDLE_GAP      Gap_100ms
#define LNG_FRESH_GAP       Gap_1000ms
#define BACKLIGHT_GAP       Gap_20000ms


uint16 Task_lng_cnt;
uint16 Task_backlight_cnt;
uint16 Task_KEY_cnt;
uint16 Task_FSL_Main_cnt;
uint16 Task_KEY_Handle_cnt;
uint16 Task_read_T15_cnt;
uint16 Task_confirm_T15Down_cnt;


void TimeManage(void)
{
  if(RTI_Cnt >= RTI_CNT_THREHOLD)
  {
    RTI_Cnt = 0;
  }
  if((Task_lng_cnt+LNG_FRESH_GAP) >= RTI_CNT_THREHOLD)
  {
    Task_lng_cnt = 0;
  }
  if((Task_backlight_cnt+BACKLIGHT_GAP) >= RTI_CNT_THREHOLD)
  {
    Task_backlight_cnt = 0;
  }
  if((Task_KEY_Handle_cnt+KEY_HANDLE_GAP) >= RTI_CNT_THREHOLD)
  {
    Task_KEY_Handle_cnt = 0;
  }
  if((Task_FSL_Main_cnt+FSL_Main_GAP) >= RTI_CNT_THREHOLD)
  {
    Task_FSL_Main_cnt = 0;
  }
  if((Task_KEY_cnt+KEY_GAP) >= RTI_CNT_THREHOLD)
  {
    Task_KEY_cnt = 0;
  }
  if((Task_read_T15_cnt+READ_T15VOLT_GAP) >= RTI_CNT_THREHOLD)
  {
    Task_read_T15_cnt = 0;
  }
  if((Task_confirm_T15Down_cnt+CONFIM_T15DOWN_GAP) >= RTI_CNT_THREHOLD)
  {
    Task_confirm_T15Down_cnt = 0;
  }
}


#define READ_VERIFY 0
extern uint16 lastID;
void T15Down_PostProcess (void)  //需要处理不能成功下电的情况
{ 
  uint8 i;
  UINT16 returnCode;
  
  if((uint16)(lastID) >= EED_MAX_RECORD_NUMBER)    
  {
    lastID = 0;
  }
  lastID += 1;
  returnCode = FSL_WriteEeprom(lastID, (uint16*)(can_msgs_buf_array[AQ1_MSG_RECEIVE].pbuffer));
  if(EED_OK == returnCode)
  {
    while(BUSY == EE_Status)
    {
        FSL_Main();//由BSW调度器周期性调用
    }
  }
  
#if READ_VERIFY
  for( i = 0;i < EED_DATA_VALUE_SIZE ; i+=EED_MIN_PROG_SIZE)
  {
      if(((uint16*)(can_msgs_buf_array[AQ1_MSG_RECEIVE].pbuffer))[i/EED_MIN_PROG_SIZE] != READ_DFLASH16(freeSpaceAddress - EED_RECORD_LENGTH + i))
      {
          break;
      }
  }
  
  if(EED_DATA_VALUE_SIZE == i)
#endif  
  {   
    BAT_OFF();//电源BAT下电控制，低电平下电
  }
  
  {             //delay for completly power down
    uint32 i;
    for(i=0; i<0xFFFFFF; i++);
  }
}


void Task_T15Monitor (void)
{
  static uint8 T15DownIndex;
  
  if((RTI_Cnt-Task_read_T15_cnt) >= READ_T15VOLT_GAP)// 10ms
  {
    Task_read_T15_cnt=RTI_Cnt;
    if(T15_THREHOLD_ADC_VALUE >= Adc_Read(T15_chan))
      T15DownIndex++;  
  }
  
  if((RTI_Cnt-Task_confirm_T15Down_cnt) >= CONFIM_T15DOWN_GAP)//100ms
  {
    Task_confirm_T15Down_cnt=RTI_Cnt;
    if(T15DownIndex >= 8)
    {
      RTI_INT_DISABLE();//added by xyl 2014.10.18
      T15Down_PostProcess();
    }
    else
    {
      T15DownIndex = 0;
    }
  }
}  


void Task_FSL_Main(void)
{ 
  if((RTI_Cnt-Task_FSL_Main_cnt) >= FSL_Main_GAP) 
  {
    Task_FSL_Main_cnt=RTI_Cnt;    
    FSL_Main();
  }
}




   
#define KEY_PRESSED_DOWN_LEVEL  0
#define KEY_NOT_DOWN_LEVEL      1
#define DEBOUNCE_TIMES        6  //消抖次数
#define KEY_TOTAL_NUM           5

key_type g_key[KEY_TOTAL_NUM] = 
{
  {0,  },
  {0,  },
  {0,  },
  {0,  },
  {0,  },
};

void Task_KeyScan(void)
{ 
  uint8 i;
  
  if((RTI_Cnt-Task_KEY_cnt) >= KEY_GAP) 
  {
    Task_KEY_cnt=RTI_Cnt;

    for(i=0; i<KEY_TOTAL_NUM; i++)
    {
      if(KEY_PRESSED_DOWN_LEVEL == (PTT & (2<<i)))
      {
        g_key[i].index++;
      }
    }
  }
}


void Task_KeyHandle(void)
{
  uint8 i;
  uint8 key_pressed_cnt;
  static uint8 cnt;
  static uint8 pressed_flag;
  static uint8 backlight_off_mode;
  static uint16 Task_backlight_cnt_bak;
     
  if((RTI_Cnt-Task_KEY_Handle_cnt) >= KEY_HANDLE_GAP) 
  {
    Task_KEY_Handle_cnt = RTI_Cnt;
    
    if(!backlight_off_mode)
    {
      for(i=0; i<KEY_TOTAL_NUM; i++)
      {
        if(g_key[i].index>=DEBOUNCE_TIMES)
        {
          g_key[i].flag = 1;
          Task_backlight_cnt_bak = RTI_Cnt;
        }
      }
    }
    else
    {
      key_pressed_cnt = 0;
      for(i=0; i<KEY_TOTAL_NUM; i++)
      {
        if(g_key[i].index>=DEBOUNCE_TIMES)
        {       
          key_pressed_cnt++;          
        }
      }
      if(key_pressed_cnt)
      {
        pressed_flag = 1;
        BACKLIGHT_ON();
        //backlight_off_mode = 0;//不能放在此处，避免一次按键后backlight_off_mode变0后又被当作下一次按键
      }
      if(pressed_flag)
        cnt++;//
      if(cnt>=2)        //延时一个 KEY_HANDLE_GAP后再改变backlight_off_mode
      {
        cnt = 0;
        pressed_flag = 0;        
        backlight_off_mode = 0;      
      } 
    }
    
    for(i=0; i<KEY_TOTAL_NUM; i++)
    {
      g_key[i].index = 0;
    }
    
    if(!backlight_off_mode)
    {
      Task_backlight_cnt = RTI_Cnt;
    }
    else
    {
      Task_backlight_cnt_bak = RTI_Cnt;
    }
  }  
  
  if(Task_backlight_cnt >= (Task_backlight_cnt_bak+BACKLIGHT_GAP))
  {
    Task_backlight_cnt_bak = Task_backlight_cnt;
    BACKLIGHT_OFF();
    backlight_off_mode = 1;
  }     
}


extern uint8 g_err;
void Task_Display(void) 
{
  if((RTI_Cnt-Task_lng_cnt) >= LNG_FRESH_GAP) 
  {
    Task_lng_cnt = RTI_Cnt;

    display_fresh();
      //if(LED_RED_IS_OFF())  
    GREEN_LED = !GREEN_LED;
    if(g_err)
    {
      LED_RED_ON();//RED_LED = !RED_LED;
    }
    else
    {
      LED_RED_OFF(); 
    }
    send_sysStatus();//for debug    
  }
}
