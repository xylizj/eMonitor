#ifndef _LCD_H_
#define _LCD_H_
#include <mc9s12hy64.h>

#define VOLT         PT1AD_PT1AD6
#define SCL          PTH_PTH0
#define SDA          PTH_PTH3
#define BEEP         PTP_PTP0
#define KEY          PTT
#define KEY1         PTT_PTT1
#define KEY2         PTT_PTT2
#define KEY3         PTT_PTT3
#define KEY4         PTT_PTT4
#define KEY5         PTT_PTT5
#define LED          PORTB
#define GREEN_LED    PORTB_PB3
#define RED_LED    PORTB_PB2
#define RED_MASK     0x04
#define GREEN_MASK   0x08
#define ORANGE_MASK  0x00
#define LEDOFF_MASK  0x0C

#define LED_GREEN_ON()    {LED &= ~ GREEN_MASK;}
#define LED_GREEN_OFF()    {LED |= GREEN_MASK;}
#define LED_GREEN_IS_ON()  (0==GREEN_LED)
#define LED_GREEN_IS_OFF()  (1==GREEN_LED)
#define LED_RED_ON()      {LED &= ~ RED_MASK;}
#define LED_RED_IS_ON()   (0==RED_LED)
#define LED_RED_IS_OFF()  (1==RED_LED)
#define LED_RED_OFF()     {LED |= RED_MASK;}

#define BL           PTU_PTU0
#define CS           PTU_PTU1
#define RST          PTU_PTU2
#define RS           PTU_PTU4
#define WR           PTU_PTU5
#define E            PTU_PTU6
#define PWR          PTR_PTR0
#define DATA         PTV


//新版硬件改动
#define BACKLIGHT_OFF() {BL=0;}
#define BACKLIGHT_ON() {BL=1;}

typedef enum _num_index_tag
{
  INDEX_ZERO,             //0
  INDEX_ONE,              //1
  INDEX_TWO,              //2
  INDEX_THREE,            //3
  INDEX_FOUR,             //4
  INDEX_FIVE,             //5
  INDEX_SIX,              //6
  INDEX_SEVEN,            //7
  INDEX_EIGHT,            //8
  INDEX_NINE,             //9
  INDEX_10,                //10
  INDEX_11,                //11
  INDEX_12,                //12
  INDEX_13,                //13
  INDEX_14,                //14
  INDEX_15,                //15
  INDEX_POINT,            //16 .
  INDEX_SHORT_LINE,       //17
  INDEX_RIGHT_DIRECTION,  //18
  INDEX_UP_DIRECTION,     //19
  INDEX_DOWN_DIRECTION,   //20
  INDEX_O,
  INDEX_N,
  INDEX_F,
  //INDEX_Y,
  INDEX_BLANK,            
}num_index_type;



//LCD 8 rows. 8*8=64
#define LCD_ROW1 1
#define LCD_ROW2 2
#define LCD_ROW3 3
#define LCD_ROW4 4
#define LCD_ROW5 5
#define LCD_ROW6 6
#define LCD_ROW7 7
#define LCD_ROW8 8


extern const byte main_menu[][16];
extern const byte dis_sets_8_16[][16];
extern const byte bat[][32];

extern const byte dtc_byte[][4];
extern const byte blank_row[16][16];
extern const byte *detail_dtc[];
extern const byte gas_log[][16];
extern const byte gas_bmp[][16];
extern const byte oil_bmp[][16];
extern const byte fresh_icon[][16];
extern const byte fresh11[][16];

extern void lcd_init(void);
extern void lcd_clr(void);
extern void dis_pic_12864(const byte *pic);
extern void dis_pic_1616(byte page,byte column,const byte *pic);
extern void dis_pic_row(byte page,const byte *pic);
extern void dis_pic_816(byte page,byte column,const byte *pic);
extern void dis_num(byte page,byte column,const byte *pic);
extern void dis_pic_8(byte col,byte page,byte column,const byte *pic);
extern void dis_pic_16(byte col,byte page,byte column,const byte *pic);
extern void dis_pic_blankrow(byte page);

#endif
