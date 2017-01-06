#include "mc9s12hy64.h"
#include "mscan12.h"
#include "MCU.h"
#include "OS.h"
#include "can_process.h"



void CAN_Init(void) //250K
{
  CANCTL1 |= CANCTL1_CANE_MASK; //enable MSCAN12
  //!<=========================================================================
  //!<            Must Enable CAN Moudle First
  //!<=========================================================================
  
  CANCTL0 |= CANCTL0_INITRQ_MASK; // apply for initialize
  CANCTL0 |= CANCTL0_TIME_MASK;
  while(!(CANCTL1 & CANCTL1_INITAK_MASK)); //wait initialization be completed
  CANCTL1 &= ~ CANCTL1_CLKSRC_MASK;  //select external oscillator clock source  
  CANCTL1 &= ~ CANCTL1_LOOPB_MASK;  //disable loopback
  CANCTL1 &= ~ CANCTL1_LISTEN_MASK; //disable listen only mode
 
  CANBTR0 = CANBTR0V;
  CANBTR1 = CANBTR1V;
  //CANIDAC_IDAM = CANIDACV;
  /* Acceptance filters mode configure */
    CANIDAC = (CANIDAC&(~CANIDAC_IDAM_MASK)) | (CANIDAC_IDAM_MASK& (CAN0_FILTE_MODE<<CANIDAC_IDAM_BITNUM));
  
  #if(CAN0_FILTE_MODE == TWO_32_FILTERS )
    {
       CANIDAR0 = CAN0_ACC_CODE_EXTD_ID1_HIGH_H ;
       CANIDAR1 = CAN0_ACC_CODE_EXTD_ID1_HIGH_L ;
       CANIDAR2 = CAN0_ACC_CODE_EXTD_ID1_LOW_H ;
       CANIDAR3 = CAN0_ACC_CODE_EXTD_ID1_LOW_L ;
       
       CANIDAR4 = CAN0_ACC_CODE_EXTD_ID2_HIGH_H ;
       CANIDAR5 = CAN0_ACC_CODE_EXTD_ID2_HIGH_L ;
       CANIDAR6 = CAN0_ACC_CODE_EXTD_ID2_LOW_H ;
       CANIDAR7 = CAN0_ACC_CODE_EXTD_ID2_LOW_L ;
    
       CANIDMR0 = CAN0_MASK_CODE_EXTD_ID1_HIGH_H ;
       CANIDMR1 = CAN0_MASK_CODE_EXTD_ID1_HIGH_L ;
       CANIDMR2 = CAN0_MASK_CODE_EXTD_ID1_LOW_H ;
       CANIDMR3 = CAN0_MASK_CODE_EXTD_ID1_LOW_L ;
       
       CANIDMR4 = CAN0_MASK_CODE_EXTD_ID2_HIGH_H ;
       CANIDMR5 = CAN0_MASK_CODE_EXTD_ID2_HIGH_L ;
       CANIDMR6 = CAN0_MASK_CODE_EXTD_ID2_LOW_H ;
       CANIDMR7 = CAN0_MASK_CODE_EXTD_ID2_LOW_L ;
      }
   #endif
   
  CANTIER = CANTIERV;
  CANRIER = 0;
  
  CANCTL0 &= ~ CANCTL0_INITRQ_MASK;
  
  while((CANCTL1 | CANCTL1_INITAK_MASK) == 1);
}




static uint8 FC1_Msg_receive_buf[8];
static uint8 AQ1_Msg_receive_buf[8];
static uint8 AQ1_Msg_send_buf[8];
static uint8 SIG1_Msg_receive_buf[8];
static uint8 CMD_Msg_receive_buf[8];
static uint8 SysStatus_Send_buf[8];

const can_msg_config_t can_msgs_cfg_array[] = 
{
  {FC1_ID, TRUE, FALSE, FC1_Msg_Process},    /*FC1_Msg RECEIVE*/ 
  //{AQ1_ID, TRUE, FALSE, AQ1_Msg_Process},                /*AQ1_MSG_RECEIVE*/
  {AQ1_ID, TRUE, FALSE, NULL},                /*AQ1_MSG_RECEIVE*/
  {TX_RESPONSE_ID, TRUE, TRUE, NULL},          /*RESPONSE_MSG_SEND*/ 
  {SIG1_ID, TRUE, FALSE, SIG1_Msg_Process},  /*SIG1_MSG_RECEIVE*/
  {RX_CMD_ID, TRUE, FALSE, CMD_Msg_Process},  /*CMD_MSG_RECEIVE*/
  {TX_SYS_STATUS_ID, TRUE, TRUE, NULL},  /*TX_SYS_STATUS_SEND*/

};

can_msg_buf_cfg_t can_msgs_buf_array[] =
{     
  {0, 8, FC1_Msg_receive_buf, 0},          /*FC1_Msg RECEIVE 10ms sys RTI tick*/
  {0, 8, AQ1_Msg_receive_buf, 0},          /*AQ1_MSG_RECEIVE*/
  {0, 8, AQ1_Msg_send_buf, 0},             /*RESPONSE_MSG_SEND*/
  {0, 8, SIG1_Msg_receive_buf,0},          /*SIG1_MSG_RECEIVE*/
  {0, 8, CMD_Msg_receive_buf,0},          /*CMD_MSG_RECEIVE*/
  {0, 8, SysStatus_Send_buf,0},          /*CMD_MSG_RECEIVE*/
};






/*
 * Function:can_tx_msg
 * Param<can_msg_t msg_index>: Message information index in can_msgs_cfg_array(global)
 *                             and message buffer information index in can_msgs_buf_array(global)
 * Return<io_err_t>: IO_ERR_OK if transmit message trig or transmit buffer idle(when msg_index is -1), else IO_ERR_BUSY
 * REQ IDs:TPMS_SW_BASE_0048,TPMS_SW_BASE_0049,TPMS_SW_BASE_0050,TPMS_SW_BASE_0051
 * Discription:
 * Note: For check last message is transmit or not, prameter msg_index used -1.
 *       Return IO_ERR_BUSY, means BUSY,else Success.
*/
io_err_t can_tx_msg(can_msg_t msg_index)
{
    uint8 msg_buffer_index = 0;
    uint8 idle_buf = 0;

    idle_buf = CANTFLG_TXE;
    /*check tx buffer only*/
    if (0 == idle_buf)
    {
        if (CAN_MSG_EMPTY_CHECK(msg_index))
        {
            can_msgs_buf_array[msg_index].msg_st = CAN_MSG_NEW_MASK;
        }
        else
        {
            return IO_ERR_BUSY;
        }
    }    
    else
    {        
        CANTBSEL_TX = idle_buf; /*The lowest numbered bit places the respective transmit buffer in the CANTXFG register space*/
         /*set ID*/
        if (TRUE == can_msgs_cfg_array[msg_index].is_ext_id)
        {
            /*data frame*/
            *((uint32 *)(&CANTXIDR0)) = (((can_msgs_cfg_array[msg_index].id & 0x1FFC0000) << 3) \
                                                                 |0x00180000|((can_msgs_cfg_array[msg_index].id & 0x0003FFFF)<<1));
        }
        else
        {
            /*Data frame*/
            *((uint16 *)(&CANTXIDR0)) = (uint16)(can_msgs_cfg_array[msg_index].id << 5);
        }              
     
        /*load buffer*/
        if (can_msgs_cfg_array[msg_index].process_fun != NULL)
        {
            can_msgs_cfg_array[msg_index].process_fun(msg_index);
        } 
        
        CANTXDLR = can_msgs_buf_array[msg_index].dlc;
        for (msg_buffer_index = 0; msg_buffer_index < can_msgs_buf_array[msg_index].dlc; msg_buffer_index++)
        {
            *((uint8 *)(&CANTXDSR0 + msg_buffer_index)) = can_msgs_buf_array[msg_index].pbuffer[msg_buffer_index];
        }
        
        can_msgs_buf_array[msg_index].msg_st = CAN_MSG_EMPTY;
         
        /*Start transmit message*/
        CANTFLG_TXE = 1;
        //CANTIER_TXEIE) = 1;
    }
   
    return IO_ERR_OK;
}



/*
 * Function:can_rx_msg
 * Param<void>:
 * Return<void>:
 * REQ IDs:TPMS_SW_BASE_0052,TPMS_SW_BASE_0053,TPMS_SW_BASE_0054,TPMS_SW_BASE_0055,TPMS_SW_BASE_0056,
 *         TPMS_SW_BASE_0057,TPMS_SW_BASE_0058
 * Discription: Receive all message from rx-buffer.
 *              If ID match, store message in corresponding message buffer.
 *              Only check messages in can_msgs_cfg_array, other messages ignore.
 * Note: 
*/
void can_rx_msg(void)
{  
  uint8  msg_index;
  uint8  dt_index;
  uint32 real_id = 0;
  boolean   ext_id = FALSE;

  while (1 == CANRFLG_RXF)
  {
    ext_id = CANRXIDR1_IDE;
    if (TRUE == ext_id) /*CAN ID type match*/
    {
      real_id = *((uint32 *)(uint16)(&CANRXIDR0));
      real_id = ((real_id >> 1) & 0x0003FFFF)|((real_id >> 3) & 0x1FFC0000);
    }
    else
    {
        real_id = *((uint16 *)(uint16)(&CANRXIDR0)) >> 5;
    }
    /*Search ID config array*/
    for (msg_index = 0; msg_index < MSGS_RECEIVE_MAXCOUNT; msg_index++)
    {
      if ((FALSE == can_msgs_cfg_array[msg_index].is_tx) && (can_msgs_cfg_array[msg_index].is_ext_id == ext_id))
      {
        if((can_msgs_cfg_array[msg_index].id == real_id))
        {
          can_msgs_buf_array[msg_index].time_stamp = RTI_Cnt;//added by xyl
          can_msgs_buf_array[msg_index].dlc = CANRXDLR & 0x0F;
          for (dt_index = 0; dt_index < can_msgs_buf_array[msg_index].dlc; dt_index++)
          {
              can_msgs_buf_array[msg_index].pbuffer[dt_index] = *((uint8 *)(&CANRXDSR0 + dt_index));
          }
          
          if (can_msgs_cfg_array[msg_index].process_fun != NULL)
          {
              can_msgs_cfg_array[msg_index].process_fun(msg_index);//这里使用的是解包函数unpack
          }
          
          if (0 != (can_msgs_buf_array[msg_index].msg_st & CAN_MSG_NEW_MASK))
          {
              can_msgs_buf_array[msg_index].msg_st = CAN_MSG_OVER_MASK;
          }
          else
          {
              can_msgs_buf_array[msg_index].msg_st = CAN_MSG_NEW_MASK;
          }
        }
      }
    }
    CANRFLG_RXF = 1; 
  }
}

/*
 * Function:can_period_handle
 * Param<BOOL tx_rx>:TRUE for handle TX else handle RX
 * Return<void>:
 * REQ IDs:TPMS_SW_BASE_0457
 * Discription:
 * Note: 
*/
#define CAN_TIMEOUT Gap_10000ms
//void can_period_handle(boolean tx_rx)
void can_period_handle(boolean tx_rx)
{
  can_msg_t msg_index;
    
  for (msg_index=0; msg_index<MSGS_RECEIVE_MAXCOUNT; msg_index++)
  {
    if(RTI_Cnt>(can_msgs_buf_array[msg_index].time_stamp+CAN_TIMEOUT))
    {
      CAN_MSG_TIMEOUT_SET(msg_index);
    }
    else
    {
      CAN_MSG_TIMEOUT_CLEAR(msg_index);
    }
  }
}
