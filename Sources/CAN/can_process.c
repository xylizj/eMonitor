#include "can_process.h"
#include "ee_emulation.h"
#include "NormalDemo.h"
#include "MCU.h"
#include <hidef.h>      /* common defines and macros */

uint32 g_DSM_m_st_GlobalIndex0,g_DSM_m_st_GlobalIndex1;
uint32 g_TR_m_q_DieselTotal,g_TR_m_q_LNGTotal;
uint16 g_SID_m_p_CNGPrs;
uint8 g_SID_m_t_CNGTemp;
uint8 g_T15_SWT, g_MODE_SWT, g_CUTV_ST, g_NG_REL, g_EPS_ST;
uint8 g_T15_SWT_last, g_MODE_SWT_last, g_CUTV_ST_last, g_NG_REL_last, g_EPS_ST_last;
uint32 g_TC_m_ti_NGET;

void FC1_Msg_Process(can_msg_t msg_index)
{
    FC1_MsgType *p_FC1_Msg  = (FC1_MsgType *)(can_msgs_buf_array[msg_index].pbuffer);//(&FC1_Msg_receive_buf[0]);
    g_DSM_m_st_GlobalIndex0 = p_FC1_Msg->DSM_m_st_GlobalIndex0;
    g_DSM_m_st_GlobalIndex1 = p_FC1_Msg->DSM_m_st_GlobalIndex1;   
}



extern uint16 lastID;

/*void AQ1_Msg_Process(can_msg_t msg_index)
{
    lastID++;
    if((uint16)(lastID) > EED_MAX_RECORD_NUMBER)    
      lastID = DATA_ID_ONE;
    FSL_WriteEeprom(lastID, (uint16*)(can_msgs_buf_array[msg_index].pbuffer));
    FSL_Main();
} */


void SIG1_Msg_Process(can_msg_t msg_index)
{
    SIG1_MsgType *p_SIG1_Msg = (SIG1_MsgType *)(can_msgs_buf_array[msg_index].pbuffer);
    
    g_SID_m_p_CNGPrs= p_SIG1_Msg->SID_m_p_CNGPrs;  //=g_SID_m_p_CNGPrs  
    g_SID_m_t_CNGTemp=p_SIG1_Msg->SID_m_t_CNGTemp;//g_SID_m_t_CNGTemp = 
    //g_T15_SWT         = p_SIG1_Msg->T15_SWT;
    g_MODE_SWT        = p_SIG1_Msg->MODE_SWT;
    //g_CUTV_ST         = p_SIG1_Msg->CUTV_ST;
    g_NG_REL          = p_SIG1_Msg->NG_REL;
    //g_EPS_ST          = p_SIG1_Msg->EPS_ST; 
    //g_TC_m_ti_NGET    = p_SIG1_Msg->TC_m_ti_NGET;   
}

void send_sysStatus(void)
{
   ((UINT16 *)can_msgs_buf_array[TX_SYS_STATUS_SEND].pbuffer)[0] = lastID;
   
   (void)can_tx_msg(TX_SYS_STATUS_SEND);
}

void read_send_dataRecord(can_msg_t msg_index)
{
  UINT16 recordAddr;
  //uint16 lastID;
  
  //lastID = myFuncSearceIDTable();
  if(EED_OK == FSL_ReadEeprom(lastID, &recordAddr))
  {
    UINT8 i;
    for(i=0; i<EED_DATA_VALUE_SIZE; i+=EED_MIN_PROG_SIZE)
    {
      ((UINT16 *)(can_msgs_buf_array[msg_index].pbuffer))[i/EED_MIN_PROG_SIZE] = READ_DFLASH16(recordAddr + i);
    }
  } 
  //can_msgs_buf_array[msg_index].pbuffer = can_msgs_buf_array[AQ1_MSG_RECEIVE].pbuffer;
    
  (void)can_tx_msg(msg_index);
}

void CMD_Msg_Process(can_msg_t msg_index)
{  
  CMD_MsgType *p_CMD_Msg = (CMD_MsgType *)(can_msgs_buf_array[msg_index].pbuffer);
  
  if(CMD_READ == p_CMD_Msg->CMD_Read)
  {
     read_send_dataRecord(RESPONSE_MSG_SEND);
  }

  if(CMD_CLEAR == p_CMD_Msg->CMD_Clear)//need clear
  {
    //RTI_INT_DISABLE();//added by xyl 2014.10.18
    //UINT16 returnCode;
    
    //returnCode = 
    FSL_DeinitEeprom();
    //if(SGF_OK==returnCode)
    {
      while(BUSY == EE_Status)
      {
          FSL_Main();
      }
    }
    FSL_InitEeprom();
    while(BUSY == EE_Status)
    {
        FSL_Main();
    }
  }
}
