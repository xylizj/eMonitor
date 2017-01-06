#ifndef _MONITOR_H_
#define _MONITOR_H_
#include "Platform_Types.h"



#define MAIN_MENU_KEY   0
#define FLUSH_KEY       1
#define PAGE_UP_KEY     2
#define PAGE_DOWN_KEY   3
#define DETAIL_DTC_KEY  4 

#define HAVE_DTC_MASK       0x01
#define FC1_TIMEOUT_MASK    0x02
#define SIG1_TIMEOUT_MASK   0x04

#define BASE_ADDR (0x40-0x10)


typedef struct _DTC_TYPE_
{
  uint8 total_cnt;
  uint8 display_offset;
  uint8 new_dtc_flag;
  uint8 display_detaildtc_flag;
  uint8 dtc_ptr[64];  //考虑并测试改为指针不可信
}dtc_type;








extern void init_port(void);
extern void display_fresh(void);
extern void display_detaildtc(void);





































#endif
