#ifndef _OS_H
#define _OS_H

#define Gap_10ms    1
#define Gap_20ms    2
#define Gap_100ms    10
#define Gap_200ms    20
#define Gap_500ms   50
#define Gap_1000ms   100
#define Gap_10000ms  1000
#define Gap_20000ms  2000

typedef struct _key_tag_
{
  uint8 flag;
  uint8 index;
}key_type;

extern key_type g_key[5];




extern void TimeManage(void);
extern void Task_Display(void);
extern void Task_KeyHandle(void);                                                                            
extern void Task_KeyScan(void);                                                                            
extern void Task_T15Monitor (void);
extern void Task_FSL_Main(void);

extern key_type g_key[5];



































#endif  /*_OS_H*/
