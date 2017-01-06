#ifndef __CAN_PROCESS_H_
#define __CAN_PROCESS_H_

#include "mscan12.h"
//#include "msg.h"

#include "stddef.h"
#include "stdtypes.h"
#include "Platform_Types.h"




#define CMD_READ  ((uint8)0xAA)
#define CMD_CLEAR ((uint8)0x11)

//未使用位采用0x00填充

typedef struct _FC1_tag
{
  uint32 DSM_m_st_GlobalIndex0;  /* ENC8故障全局指示0 */ //为1的位表示相应的故障
  uint32 DSM_m_st_GlobalIndex1;  /* ENC8故障全局指示1 */ //为1的位表示相应的故障
}FC1_MsgType;

typedef struct _AQ1_tag
{
  uint32 TR_m_q_DieselTotal;     /* 燃油消耗累积量 */
  uint32 TR_m_q_LNGTotal;        /* 天然气消耗累积量 */
}AQ1_MsgType;

typedef struct _SIG1_tag
{
  //High bytes
  uint16 SID_m_p_CNGPrs;        /* 天然气压力 */
  uint8 SID_m_t_CNGTemp;        /* 天然气温度 */
  
  //Lowest bit
  uint16 T15_SWT         :1;        /* 点火开关 */          //0：T15没有上电
                                                          //1：T15上电
                                                      
  uint16 MODE_SWT        :1;        /* LNG模式开关 */       //0：请求纯油模式
                                                          //1：请求双燃料模式
                                                         
  uint16 CUTV_ST         :1;        /* 切断阀驱动状态 */    //0: 切断阀OFF
                                                          //1: 切断阀ON
                                                        
  uint16 NG_REL          :1;        /* 系统当前工作模式 */  //0：系统在纯油下工作
                                                          //1：系统在双燃料模式下工作
 
  //High 4 bits                                                         
  uint16 EPS_ST          :4;        /* 发动机同步状态 */    //0：无信号
                                                          //1：已同步
                                                          //2：不同步
                                                          //3：正在同步中   
  uint32 TC_m_ti_NGET;          /* 天然气喷射时长 */
}SIG1_MsgType;


typedef struct _cmd_tag
{
  //High byte
  uint8 CMD_Read;                                                         
  uint8 CMD_Clear;
  
  uint8 reservedByte2;
  uint8 reservedByte3;
  uint8 reservedByte4;
  uint8 reservedByte5;
  uint8 reservedByte6;
  uint8 reservedByte7;       
}CMD_MsgType;





















extern void SIG1_Msg_Process(can_msg_t msg_index);
extern void FC1_Msg_Process(can_msg_t msg_index);
extern void AQ1_Msg_Process(can_msg_t msg_index);
extern void CMD_Msg_Process(can_msg_t msg_index);
extern void send_sysStatus(void);






#endif/*__CAN_PROCESS_H_*/