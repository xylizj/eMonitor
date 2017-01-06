#ifndef __CAN_PROCESS_H_
#define __CAN_PROCESS_H_

#include "mscan12.h"
//#include "msg.h"

#include "stddef.h"
#include "stdtypes.h"
#include "Platform_Types.h"




#define CMD_READ  ((uint8)0xAA)
#define CMD_CLEAR ((uint8)0x11)

//δʹ��λ����0x00���

typedef struct _FC1_tag
{
  uint32 DSM_m_st_GlobalIndex0;  /* ENC8����ȫ��ָʾ0 */ //Ϊ1��λ��ʾ��Ӧ�Ĺ���
  uint32 DSM_m_st_GlobalIndex1;  /* ENC8����ȫ��ָʾ1 */ //Ϊ1��λ��ʾ��Ӧ�Ĺ���
}FC1_MsgType;

typedef struct _AQ1_tag
{
  uint32 TR_m_q_DieselTotal;     /* ȼ�������ۻ��� */
  uint32 TR_m_q_LNGTotal;        /* ��Ȼ�������ۻ��� */
}AQ1_MsgType;

typedef struct _SIG1_tag
{
  //High bytes
  uint16 SID_m_p_CNGPrs;        /* ��Ȼ��ѹ�� */
  uint8 SID_m_t_CNGTemp;        /* ��Ȼ���¶� */
  
  //Lowest bit
  uint16 T15_SWT         :1;        /* ��𿪹� */          //0��T15û���ϵ�
                                                          //1��T15�ϵ�
                                                      
  uint16 MODE_SWT        :1;        /* LNGģʽ���� */       //0��������ģʽ
                                                          //1������˫ȼ��ģʽ
                                                         
  uint16 CUTV_ST         :1;        /* �жϷ�����״̬ */    //0: �жϷ�OFF
                                                          //1: �жϷ�ON
                                                        
  uint16 NG_REL          :1;        /* ϵͳ��ǰ����ģʽ */  //0��ϵͳ�ڴ����¹���
                                                          //1��ϵͳ��˫ȼ��ģʽ�¹���
 
  //High 4 bits                                                         
  uint16 EPS_ST          :4;        /* ������ͬ��״̬ */    //0�����ź�
                                                          //1����ͬ��
                                                          //2����ͬ��
                                                          //3������ͬ����   
  uint32 TC_m_ti_NGET;          /* ��Ȼ������ʱ�� */
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