#include <mc9s12hy64.h>     /* derivative information */
#include "monitor.h"
#include "lcd.h"
#include "mscan12.h"
#include "MCU.h"
#include "OS.h"
#include "KEY.h"



void init_port(void)//initial the mcu port
{
 DDRP=0xff;
 PTP=0x01;
 DDRT=0x00;
 PTT=0xff;
 DDRB=0xff;
 PORTB=LEDOFF_MASK;
 DDRU=0xff;
 PTU=0x06;
 DDRV=0xff;
 PTV=0x00;   
}




uint8 g_err;
//uint8 have_dtc;
uint8 received_flag;  

	
dtc_type g_dtc={
  0,0,0,0
};



extern uint32 g_DSM_m_st_GlobalIndex0,g_DSM_m_st_GlobalIndex1;
extern uint32 g_TR_m_q_DieselTotal,g_TR_m_q_LNGTotal;         
extern uint16 g_SID_m_p_CNGPrs;
extern uint8 g_SID_m_t_CNGTemp;
extern uint8 g_T15_SWT, g_MODE_SWT, g_CUTV_ST, g_NG_REL, g_EPS_ST;
extern uint8 g_T15_SWT_last, g_MODE_SWT_last, g_CUTV_ST_last, g_NG_REL_last, g_EPS_ST_last;
extern uint32 g_TC_m_ti_NGET;

void display_detaildtc(void)
{
  uint8 i;
  uint8 dtc_count;

  if(g_dtc.total_cnt>4)
  {
    if(g_dtc.total_cnt >= (g_dtc.display_offset+4))    
    {
      dtc_count = 4;
    }
    else
    {
      dtc_count = g_dtc.total_cnt-g_dtc.display_offset;
    }
  }
  else
  {
    dtc_count = g_dtc.total_cnt;
    g_dtc.display_offset = 0;
  }

  for(i=0; i<dtc_count; i++)
  {
    dis_pic_row(i*2+LCD_ROW1,detail_dtc[g_dtc.dtc_ptr[i+g_dtc.display_offset]]);
  }  

  if(dtc_count<2)
  {
    dis_pic_row(LCD_ROW3,blank_row[0]);
  }
  if(dtc_count<3)
  {
    dis_pic_row(LCD_ROW5,blank_row[0]);
  }
  if(dtc_count<4)
  {
    dis_pic_row(LCD_ROW7,blank_row[0]);
  }
   
  if((g_dtc.total_cnt-g_dtc.display_offset)>4)//when eqal 4, total_cnt 是4的倍数，and all dtc been displayed
  {
    dis_pic_8(16,0x08,0x70,dis_sets_8_16[INDEX_DOWN_DIRECTION]);
  }
  else
  {
    dis_pic_8(16,0x08,0x70,dis_sets_8_16[INDEX_BLANK]);
  }
  
  if(g_dtc.display_offset>0)
  {
    dis_pic_8(16,0x01,0x70,dis_sets_8_16[INDEX_UP_DIRECTION]);
  }
  else
  {
    dis_pic_8(16,0x01,0x70,dis_sets_8_16[INDEX_BLANK]);
  }
}


void handle_detaildtc(void)
{  	
  static uint32 g_DSM_m_st_GlobalIndex0_last_key5;
  static uint32 g_DSM_m_st_GlobalIndex1_last_key5;
	
	if(!CAN_MSG_TIMEOUT_CHECK(FC1_MSG_RECEIVE))
  {
    g_err &= ~ FC1_TIMEOUT_MASK;//LED_RED_OFF(); 
    if(g_DSM_m_st_GlobalIndex0!=g_DSM_m_st_GlobalIndex0_last_key5 
        || g_DSM_m_st_GlobalIndex1!=g_DSM_m_st_GlobalIndex1_last_key5
      )            
    {
        g_DSM_m_st_GlobalIndex0_last_key5 = g_DSM_m_st_GlobalIndex0;
        g_DSM_m_st_GlobalIndex1_last_key5 = g_DSM_m_st_GlobalIndex1;
        if(received_flag)//这样第一次收到的时候不显示更新信息
        {
          g_dtc.new_dtc_flag = 1;
        }
        received_flag = 1;
    }          
   
    if(g_dtc.total_cnt)
    { 
      g_err |= HAVE_DTC_MASK;
      //LED_RED_ON(); 
      display_detaildtc();        
    }
    else
    {
      g_err &= ~ HAVE_DTC_MASK;
      //LED_RED_OFF();
    }
    
    if(g_dtc.new_dtc_flag)
    {
      //dis_num(3,0x78,dis_sets_8_16[INDEX_Y]);
      dis_pic_1616(LCD_ROW3,112,fresh_icon[0]);           
    }
    else
    {
      dis_num(LCD_ROW3,112,dis_sets_8_16[INDEX_BLANK]);           
    } 
  }
  else //按下key5显示详细信息的时候CAN报文超时
  {
    g_err |= FC1_TIMEOUT_MASK;
    //LED_RED_ON(); //led_debug=1;
    g_key[MAIN_MENU_KEY].flag = 1;  //模拟按了一次key1,回到主界面
  }
}

static void display_FC1_timeout(void)
{
  uint8 i;
  
  for(i=0; i<8; i++) 
  {
    dis_num(0x07,BASE_ADDR+i*8,dis_sets_8_16[INDEX_SHORT_LINE]);        //17 -
  }
  dis_num(0x07,120, dis_sets_8_16[INDEX_BLANK]);
}
/*static void display_FC1_timeout(void)
{
  dis_num(0x07,BASE_ADDR,     dis_sets_8_16[INDEX_SHORT_LINE]);        //17 -
  dis_num(0x07,BASE_ADDR+0x08,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x07,BASE_ADDR+0x10,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x07,BASE_ADDR+0x18,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x07,BASE_ADDR+0x20,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x07,BASE_ADDR+0x28,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x07,BASE_ADDR+0x30,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x07,BASE_ADDR+0x38,dis_sets_8_16[INDEX_SHORT_LINE]);
  
  dis_num(0x07,120, dis_sets_8_16[INDEX_BLANK]);
}*/



//ENC8 dtc故障
void handle_FC1msg(void)
{
  uint8 i;
  //static uint8 got_msg_flag; 
  static uint32 g_DSM_m_st_GlobalIndex0_last;
  static uint32 g_DSM_m_st_GlobalIndex1_last;
  static uint8 lngdtc0[16]={0,};
  static uint8 lngdtc1[16]={0,};    
  
  if(!CAN_MSG_TIMEOUT_CHECK(FC1_MSG_RECEIVE))
  {
    g_err &= ~ FC1_TIMEOUT_MASK;
    
    if(CAN_MSG_CHECK(FC1_MSG_RECEIVE))
    /*{
      got_msg_flag = 1;  
    }
    
    if(got_msg_flag)// */
    {  
      uint16 dtcH2,dtcL2;
             
      if(g_DSM_m_st_GlobalIndex0!=g_DSM_m_st_GlobalIndex0_last)
      {
      	g_DSM_m_st_GlobalIndex0_last=g_DSM_m_st_GlobalIndex0;
        dtcL2=(word)g_DSM_m_st_GlobalIndex0;
        dtcH2=(word)(g_DSM_m_st_GlobalIndex0>>16);
        for(i=0; i<16; i++)
        {
         lngdtc0[i]=(uint8)((dtcH2>>i)&0x01)*2 + (uint8)((dtcL2>>i)&0x01);        
        }                    
      }
      for(i=0; i<16; i++) 
      {
        dis_pic_8(4,0x07,BASE_ADDR+(16-1-i)*4,  dtc_byte[lngdtc0[i]]);
      }
      
      if(g_DSM_m_st_GlobalIndex1!=g_DSM_m_st_GlobalIndex1_last)
      {
      	g_DSM_m_st_GlobalIndex1_last=g_DSM_m_st_GlobalIndex1;
        dtcL2=(word)g_DSM_m_st_GlobalIndex1;
        dtcH2=(word)(g_DSM_m_st_GlobalIndex1>>16);
        for(i=0; i<16; i++)
        {
         lngdtc1[i]=(uint8)((dtcH2>>i)&0x01)*2 + (uint8)((dtcL2>>i)&0x01);
        }       
      }
      for(i=0; i<16; i++) 
      {
        dis_pic_8(4,0x08,BASE_ADDR+(16-1-i)*4,  dtc_byte[lngdtc1[i]]);
      }
      
      if((0!=g_DSM_m_st_GlobalIndex0) || (0!=g_DSM_m_st_GlobalIndex1))
      {
        g_err |= HAVE_DTC_MASK;//have_dtc = 1;
        //LED_RED_ON();
        dis_num(0x07,0x78,dis_sets_8_16[INDEX_RIGHT_DIRECTION]);   
      }
      else
      {
        g_err &= ~ HAVE_DTC_MASK;//have_dtc = 0;
        //LED_RED_OFF();
        dis_num(0x07,0x78,dis_sets_8_16[INDEX_BLANK]);   
      }
    }
  }
  else   //timeout
  {
    g_err |= FC1_TIMEOUT_MASK;
    //LED_RED_ON(); 
    display_FC1_timeout();
  }
}   

//LNG Mode Switch
void display_lngmode(void)
{
  dis_num(0x01,BASE_ADDR,     dis_sets_8_16[INDEX_O]);
  if(g_MODE_SWT)
	{     	
  	dis_num(0x01,BASE_ADDR+8, dis_sets_8_16[INDEX_N]);
  	dis_num(0x01,BASE_ADDR+16,dis_sets_8_16[INDEX_BLANK]);
	}
	else
	{
  	dis_num(0x01,BASE_ADDR+8, dis_sets_8_16[INDEX_F]);
  	dis_num(0x01,BASE_ADDR+16,dis_sets_8_16[INDEX_F]);
	} 
}
//Display System Workmode
void display_sysmode(void)
{
  if(g_NG_REL)
	{     	
  	dis_pic_1616(0x01,112,gas_bmp[0]);
	}
	else
	{
  	dis_pic_1616(0x01,112,oil_bmp[0]);
	} 
}

#define TEMP_THREHOLD 40//Display temperature
//#define TEMP_THREHOLD 0//Display temperature温度改为不减40
void display_temperature(void)
{
  uint8 i;
  uint8 temp_tmp;
  static uint8 lastlngtemp;
  static uint8 temp_indexs[3]={0,};
  static uint8 negative_flag;

  if(g_SID_m_t_CNGTemp!=lastlngtemp)
  {
    lastlngtemp=g_SID_m_t_CNGTemp;
    if(lastlngtemp>=TEMP_THREHOLD)
    {
      //dis_num(0x03,BASE_ADDR,dis_sets_8_16[INDEX_BLANK]);
      negative_flag = 0;
      temp_tmp=lastlngtemp-TEMP_THREHOLD;
    }
    else
    {
      //dis_num(0x03,BASE_ADDR,dis_sets_8_16[INDEX_SHORT_LINE]);
      temp_tmp=TEMP_THREHOLD-lastlngtemp;
      negative_flag = 1;
    }
    for(i=0; i<3; i++)
    {
      temp_indexs[i]=(uint8)(temp_tmp%10);
      temp_tmp=temp_tmp/10;
    }
  }
  if(temp_indexs[2])//如果是三位数，肯定为正	 
  {
    dis_num(0x03,BASE_ADDR+4,dis_sets_8_16[temp_indexs[2]]);
    dis_num(0x03,BASE_ADDR+0x08+4,dis_sets_8_16[temp_indexs[1]]);
    dis_num(0x03,BASE_ADDR+0x10+4,dis_sets_8_16[temp_indexs[0]]);
  }
  else
  {
    if(negative_flag)
      dis_num(0x03,BASE_ADDR+4,dis_sets_8_16[INDEX_SHORT_LINE]);
    else
      dis_num(0x03,BASE_ADDR+4,dis_sets_8_16[INDEX_BLANK]);
    if(temp_indexs[1])
    {
      dis_num(0x03,BASE_ADDR+0x08+4,dis_sets_8_16[temp_indexs[1]]);
      dis_num(0x03,BASE_ADDR+0x10+4,dis_sets_8_16[temp_indexs[0]]);
    }
    else
    {
      dis_num(0x03,BASE_ADDR+0x08+4,dis_sets_8_16[INDEX_BLANK]);
      dis_num(0x03,BASE_ADDR+0x10+4,dis_sets_8_16[temp_indexs[0]]);
    }
  }
}
//Display pressure
/*void display_pressure(void)
{
  uint8 i;
  uint16 pres_tmp;
  static uint16 lastlngpre;
  static uint8 pres_indexs[3]={0,}; //2,1,0依次为高位到低位
  
  if (g_SID_m_p_CNGPrs!=lastlngpre)
  {
    lastlngpre=g_SID_m_p_CNGPrs;
    pres_tmp=lastlngpre/10+100;//修正为绝对压力后加100Kpa
    for(i=0; i<3; i++)
    {
     pres_indexs[i]=(uint8)(pres_tmp%10);
     pres_tmp=pres_tmp/10;
    }		
  }
  if(pres_indexs[2])
  {
    dis_num(0x05,BASE_ADDR,dis_sets_8_16[pres_indexs[2]]);
  }
  else
  {
    dis_num(0x05,BASE_ADDR,dis_sets_8_16[INDEX_BLANK]);
  }
  dis_num(0x05,BASE_ADDR+0x08,dis_sets_8_16[pres_indexs[1]]);
  dis_num(0x05,BASE_ADDR+0x10,dis_sets_8_16[INDEX_POINT]);//.
  dis_num(0x05,BASE_ADDR+0x14,dis_sets_8_16[pres_indexs[0]]);
}*/

//2015-04-24 modified by xyl
void display_pressure(void)
{
  uint8 i;
  uint16 pres_tmp;
  static uint16 lastlngpre;
  static uint8 pres_indexs[3]={0,}; //2,1,0依次为高位到低位
  
  if (g_SID_m_p_CNGPrs!=lastlngpre)
  {
    lastlngpre=g_SID_m_p_CNGPrs;
    pres_tmp=lastlngpre/10;
    pres_tmp=pres_tmp/10;
    for(i=0; i<3; i++)
    {
     pres_indexs[i]=(uint8)(pres_tmp%10);
     pres_tmp=pres_tmp/10;
    }		
  }
  if(pres_indexs[2])
  {
    dis_num(0x05,BASE_ADDR,dis_sets_8_16[pres_indexs[2]]);
  }
  else
  {
    dis_num(0x05,BASE_ADDR,dis_sets_8_16[INDEX_BLANK]);
  }
  dis_num(0x05,BASE_ADDR+0x08,dis_sets_8_16[pres_indexs[1]]);
  dis_num(0x05,BASE_ADDR+0x10,dis_sets_8_16[INDEX_POINT]);//.
  dis_num(0x05,BASE_ADDR+0x14,dis_sets_8_16[pres_indexs[0]]);
}
    
    
void display_SIG1_timeout(void)
{
  dis_num(0x01,BASE_ADDR,     dis_sets_8_16[INDEX_SHORT_LINE]);        //17 -
  dis_num(0x01,BASE_ADDR+0x08,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x01,BASE_ADDR+0x10,dis_sets_8_16[INDEX_SHORT_LINE]);
  
  //dis_pic_1616(0x01,112,gas_bmp[0]);
  dis_num(0x01,112,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x01,120,dis_sets_8_16[INDEX_SHORT_LINE]);
    
  dis_num(0x03,BASE_ADDR+4,     dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x03,BASE_ADDR+0x08+4,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x03,BASE_ADDR+0x10+4,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x05,BASE_ADDR,     dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x05,BASE_ADDR+0x08,dis_sets_8_16[INDEX_SHORT_LINE]);
  dis_num(0x05,BASE_ADDR+0x10,dis_sets_8_16[INDEX_POINT]);//.
  dis_num(0x05,BASE_ADDR+0x14,dis_sets_8_16[INDEX_SHORT_LINE]);
  
  dis_num(0x05,112,dis_sets_8_16[INDEX_BLANK]);
  dis_num(0x05,120,dis_sets_8_16[INDEX_BLANK]);
}

void handle_SIG1Msg(void)
{   
  //static uint8 got_msg_flag; 

  if(!CAN_MSG_TIMEOUT_CHECK(SIG1_MSG_RECEIVE))
  {         	
  	g_err &= ~ SIG1_TIMEOUT_MASK;
  	//LED_RED_OFF();
  	if(CAN_MSG_CHECK(SIG1_MSG_RECEIVE))
    /*{
      got_msg_flag = 1;  
    }
    if(got_msg_flag)*/
    {
      display_lngmode();
    	display_sysmode();
    	display_temperature();
      display_pressure();
    }    
  }
  else   
  {
    g_err |= SIG1_TIMEOUT_MASK;
    //LED_RED_ON(); 
    display_SIG1_timeout();
  }   
}
    



void process_keyflag(void)
{  
  if(g_key[MAIN_MENU_KEY].flag)
  {
    g_key[MAIN_MENU_KEY].flag = 0;
    g_dtc.display_detaildtc_flag = 0;
    g_dtc.display_offset = 0;
    g_dtc.new_dtc_flag = 0;
    received_flag = 0;
    dis_pic_12864(main_menu[0]);//Menu
  }
  
  if(g_key[DETAIL_DTC_KEY].flag)
  {
    g_key[DETAIL_DTC_KEY].flag = 0; 
    //if(have_dtc)
    if(g_err & HAVE_DTC_MASK)
    {      
      g_dtc.display_detaildtc_flag = 1;
    }
  } 
}

static uint8 displayed_detaildtc;
//Process key flags when displaying detail dtc
void process_detaildtc_keyflag(void)
{  
  if(!displayed_detaildtc || g_key[FLUSH_KEY].flag)
  {
    uint8 i;
    uint8 dtc_tmp;
    
    displayed_detaildtc = 1;
    g_key[FLUSH_KEY].flag = 0;
    g_dtc.new_dtc_flag = 0;     
    g_dtc.total_cnt = 0;
    for(i=0; i<32; i++)
    {
      dtc_tmp = (((uint8)(g_DSM_m_st_GlobalIndex0>>(uint8)(31-i)))&0x01);  //最高位是第0位
      if(dtc_tmp)
      {
       g_dtc.dtc_ptr[g_dtc.total_cnt] = i; 
       g_dtc.total_cnt++;
      }  
    }
    for(i=0; i<28; i++)//low 4 bits not use,reserve
    {
      dtc_tmp = (((uint8)(g_DSM_m_st_GlobalIndex1>>(uint8)(31-i)))&0x01);//右移后按位0x01可以保证结果是8位值以内
      if(dtc_tmp)
      {
       g_dtc.dtc_ptr[g_dtc.total_cnt] = i+32; 
       g_dtc.total_cnt++;
      }  
    }    
  }
    
  if(g_key[PAGE_UP_KEY].flag)
  {
    g_key[PAGE_UP_KEY].flag = 0;
    if(g_dtc.display_offset > 0)
    {
      g_dtc.display_offset -= 4;
    }
  }
  
  if(g_key[PAGE_DOWN_KEY].flag)
  {
    g_key[PAGE_DOWN_KEY].flag = 0;
    if((g_dtc.display_offset+4) < g_dtc.total_cnt)
    {
      g_dtc.display_offset += 4;
    }
  }
}




void display_fresh(void)//一直都显示，只在数据变化时做计算处理
{           
  process_keyflag();
  
  if(g_dtc.display_detaildtc_flag)//如果按下key5，则处理故障报文详细信息并显示
  {   
    process_detaildtc_keyflag();
    handle_detaildtc();
  }
  else //否则处理主界面的报文信息，二者不必同时处理
  {   
    g_key[FLUSH_KEY].flag = 0;
    g_key[PAGE_UP_KEY].flag = 0;
    g_key[PAGE_DOWN_KEY].flag = 0;
    //g_dtc.new_dtc_flag = 0;
    displayed_detaildtc = 0;
    handle_FC1msg();
    handle_SIG1Msg();
  }  
}