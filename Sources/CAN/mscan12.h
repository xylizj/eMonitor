#ifndef _MSCAN12_H
#define _MSCAN12_H

#include "stddef.h"
#include "stdtypes.h"
#include "Platform_Types.h"




#define CANBTR0V 0x00  //SJW[1:0]=00,synchronization jump width is 1 Tq
                        //BRP[5:0]=000100,the prescaler value is 5
                        
#define CANBTR1V 0x49  //SAMP=0:one sample per bit
                        //TSEG22,21,20=100:time segment2 is 5 Tq
                        //TSEG13,12,11,10=1001:time segment1 is 10 Tq */



/* Message filterring */
 #define TWO_32_FILTERS                  0x00U
 #define FOUR_16_FILTERS                 0x01U
 #define EIGHT_8_FILTERS                 0x02U
 #define FILTER_CLOSED                   0x03U
 
 /* Frame formats */
 #define STANDARD_FORMAT                 0x00U
 #define EXTENDED_FORMAT                 0x01U

 /* Frame types   */
 #define DATA_FRAME                      0x00U
 #define REMOTE_FRAME                    0x01U
 
/*
 ***********************************************************************
 ******************Set CAN Accrption Code ****************************** 
 ***********************************************************************
 */ 
#define CAN0_FILTE_MODE        TWO_32_FILTERS

#define SRR_TYPE   1            // Substitute Remote Request
#define RTR_TYPE   DATA_FRAME   // Remote Transmission Request



#define CANIDACV  0x00  //2x32bit filters

#define CAN0_TARGET_RXID1 0x18FFFF21         
#define CAN0_TARGET_RXID2 0x18FFFF21 //bit 11-8 not care

#define FC1_ID            0x18FFF121
#define AQ1_ID            0x18FFF221
#define SIG1_ID           0x18FFF321

#define RX_CMD_ID            0x18FFF521
#define TX_RESPONSE_ID       0x18FF21F5
#define TX_SYS_STATUS_ID     0x18FF21FF

#define CAN_ACC_MASK_ALL_EXTID     0x00000F00  //29 bit

#define CAN0_MASK_ACC_CODE1        CAN_ACC_MASK_ALL_EXTID        
#define CAN0_MASK_ACC_CODE2        CAN_ACC_MASK_ALL_EXTID

#define SRR_TYPE_MASK              1
#define ID1_FORMAT_MASK            1
#define RTR_TYPE_MASK              1

/* This is only used for externd ID :2 filter */
/* Acceptance Code Definitions */ 
#define CAN0_ACC_CODE_EXTD_ID1_HIGH_H   ((uint8)(CAN0_TARGET_RXID1 >> 21))
#define CAN0_ACC_CODE_EXTD_ID1_HIGH_L  (((uint8)(CAN0_TARGET_RXID1 >> 13) & 0xE0) | (SRR_TYPE<<4) | (EXTENDED_FORMAT<<3)|\
                                        ((uint8)(CAN0_TARGET_RXID1 >> 15) & 0x07) )
#define CAN0_ACC_CODE_EXTD_ID1_LOW_H    ((uint8)(CAN0_TARGET_RXID1 >> 7))
#define CAN0_ACC_CODE_EXTD_ID1_LOW_L    ((uint8)(CAN0_TARGET_RXID1 << 1) |RTR_TYPE)

#define CAN0_ACC_CODE_EXTD_ID2_HIGH_H   ((uint8)(CAN0_TARGET_RXID2 >> 21))
#define CAN0_ACC_CODE_EXTD_ID2_HIGH_L  (((uint8)(CAN0_TARGET_RXID2 >> 13) & 0xE0) | (SRR_TYPE<<4) | (EXTENDED_FORMAT<<3)|\
                                        ((uint8)(CAN0_TARGET_RXID2 >> 15) & 0x07) )
#define CAN0_ACC_CODE_EXTD_ID2_LOW_H    ((uint8)(CAN0_TARGET_RXID2 >> 7))
#define CAN0_ACC_CODE_EXTD_ID2_LOW_L    ((uint8)(CAN0_TARGET_RXID2 << 1) |RTR_TYPE)

 /* Mask Code Definitions */
#define CAN0_MASK_CODE_EXTD_ID1_HIGH_H  ((uint8)(CAN0_MASK_ACC_CODE1>>21))
#define CAN0_MASK_CODE_EXTD_ID1_HIGH_L (((uint8)(CAN0_MASK_ACC_CODE1>>13) & 0xE0) | (SRR_TYPE_MASK<<4) | (ID1_FORMAT_MASK<<3)|\
                                        ((uint8)(CAN0_MASK_ACC_CODE1>>15) & 0x07) )
#define CAN0_MASK_CODE_EXTD_ID1_LOW_H   ((uint8)(CAN0_MASK_ACC_CODE1>>7))
#define CAN0_MASK_CODE_EXTD_ID1_LOW_L   ((uint8)(CAN0_MASK_ACC_CODE1<<1) |RTR_TYPE_MASK)

#define CAN0_MASK_CODE_EXTD_ID2_HIGH_H  ((uint8)(CAN0_MASK_ACC_CODE2>>21))
#define CAN0_MASK_CODE_EXTD_ID2_HIGH_L (((uint8)(CAN0_MASK_ACC_CODE2>>13) & 0xE0) | (SRR_TYPE_MASK<<4) | (ID1_FORMAT_MASK<<3)|\
                                        ((uint8)(CAN0_MASK_ACC_CODE2>>15) & 0x07) )
#define CAN0_MASK_CODE_EXTD_ID2_LOW_H   ((uint8)(CAN0_MASK_ACC_CODE2>>7))
#define CAN0_MASK_CODE_EXTD_ID2_LOW_L   ((uint8)(CAN0_MASK_ACC_CODE2<<1) |RTR_TYPE_MASK)
#define CANTIERV 0x00  //disable receive interrupt
















#define CAN_MSG_EMPTY     0x00
#define CAN_MSG_NEW_MASK  0x01
#define CAN_MSG_OVER_MASK 0x02
#define CAN_MSG_NEW_OVER  0x03
#define CAN_MSG_TIMEOUT   0x04
#define CAN_MSG_UNTIMEOUT_MASK 0xFB

#define CAN_MSG_CHECK(msg_index) (can_msgs_buf_array[msg_index].msg_st & CAN_MSG_NEW_OVER)
#define CAN_MSG_CELAR(msg_index) can_msgs_buf_array[msg_index].msg_st = CAN_MSG_EMPTY
#define CAN_MSG_EMPTY_CHECK(msg_index) (can_msgs_buf_array[msg_index].msg_st & CAN_MSG_UNTIMEOUT_MASK) == CAN_MSG_EMPTY /*REQ IDs: TPMS_SW_BASE_0377*/
#define CAN_MSG_TIMEOUT_SET(msg_index) can_msgs_buf_array[msg_index].msg_st |= CAN_MSG_TIMEOUT
#define CAN_MSG_TIMEOUT_CLEAR(msg_index) can_msgs_buf_array[msg_index].msg_st &= CAN_MSG_UNTIMEOUT_MASK
#define CAN_MSG_TIMEOUT_CHECK(msg_index) (can_msgs_buf_array[msg_index].msg_st & CAN_MSG_TIMEOUT)




/*******************************************************************************
*                     Global Data Types                                     
*******************************************************************************/
typedef enum __IO_ERR_T__
{
  IO_ERR_OK = 0,
  IO_ERR_PARAMETER_ERROR,
  IO_ERR_BUSY,
  IO_ERR_RUN_ERROR,
  IO_ERR_OTHER_FAILURE  
}io_err_t; /*REQ IDs: TPMS_SW_BASE_0011*/



typedef enum __CAN_MSG_T__
{
    FC1_MSG_RECEIVE=0,
    AQ1_MSG_RECEIVE,
    RESPONSE_MSG_SEND,    
    SIG1_MSG_RECEIVE,
    CMD_MSG_RECEIVE,
    TX_SYS_STATUS_SEND,
    MSGS_RECEIVE_MAXCOUNT,    
}can_msg_t;

typedef struct __CAN_MSG_CONFIG_T__
{
    uint32 id;
    boolean     is_ext_id;
    boolean     is_tx;
    void (*process_fun)(can_msg_t msg_index);
}can_msg_config_t;

typedef struct __CAN_MSG_BUF_CFG_T__
{
    uint8 msg_st;
    uint8 dlc;
    uint8 *pbuffer;
    uint32 time_stamp;
    //uint16 period; /* [10ms] */
    //uint16 tick;
}can_msg_buf_cfg_t;

extern can_msg_buf_cfg_t can_msgs_buf_array[];



extern void CAN_Init(void);
extern void can_rx_msg(void);
extern void can_period_handle(boolean tx_rx);
extern io_err_t can_tx_msg(can_msg_t msg_index);  
























#endif
