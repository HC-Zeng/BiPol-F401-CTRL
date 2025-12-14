#ifndef ST7796_SPI_LCD_H
#define ST7796_SPI_LCD_H
#include "main.h"

/////////////////////////////////////用户配置区///////////////////////////////////
//支持横竖屏快速定义切换
#define USE_LCM_DIR  	  2   	//定义液晶屏顺时针旋转方向 	0-0度旋转，1-180度旋转，2-270度旋转，3-90度旋转
#define USE_TP_TYPE   	RTP 	//定义触摸类型  CTP = 电容模式 , RTP = 电阻模式

#define RTP 0X00		//电阻模式

#define u16 uint16_t

//LCD重要参数集
typedef struct
{
    uint16_t width;			//LCD 宽度
    uint16_t height;		//LCD 高度
    uint16_t id;			//LCD ID
    uint8_t dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。
    uint16_t wramcmd;		//开始写gram指令
    uint16_t setxcmd;		//设置x坐标指令
    uint16_t setycmd;		//设置y坐标指令
}_lcd_dev;

//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数
//LCD的画笔颜色和背景色
extern uint16_t  POINT_COLOR;//默认红色
extern uint16_t  BACK_COLOR; //背景颜色.默认为白色


// 反相后的画笔颜色
#define WHITE         	 0x0000  // 原0xFFFF → 反相后0x0000
#define BLACK         	 0xFFFF  // 原0x0000 → 反相后0xFFFF
#define BRED             0x07E0  // 原0xF81F → 反相后0x07E0
#define GRED 			 0x001F  // 原0xFFE0 → 反相后0x001F
#define RED           	 0x07FF  // 原0xF800 → 反相后0x07FF
#define PALE_SILVER      0x3208  // RGB(204,188,188) 反相后的RGB565值
#define STEEL_BLUE 0xB4AB  // RGB(75,107,164) 反相后的RGB565值
#define DARK_GRAY 0xBDF7  // RGB(64,64,64) 反相后的RGB565值
#define DARK_RED_BROWN 0xAF3F  // RGB(87,25,2) 反相后的RGB565值
#define GOLDEN_YELLOW 0x01DF  // RGB(251,202,3) 反相后的RGB565值
#define OLIVE_GREEN 0xBC7F  // RGB(70,114,5) 反相后的RGB565值
#define DARK_BROWN 0xB65F  // RGB(77,55,3) 反相后的RGB565值
#define MIDNIGHT_BLUE 0xEF71  // RGB(19,17,118) 反相后的RGB565值
#define COBALT_BLUE 0xFDAD  // RGB(3,74,148) 反相后的RGB565值
#define CRIMSON_RED 0x5F9D  // RGB(166,15,20) 反相后的RGB565值
#define CORAL_PINK 0x0491  // RGB(254,111,113) 反相后的RGB565值
#define SAGE_GREEN 0x6B52  // RGB(147,148,108) 反相后的RGB565值
#define LIME_GREEN 0x717C  // RGB(136,211,26) 反相后的RGB565值

//// 正色画笔颜色（恢复后的颜色）
//#define WHITE         	 0xFFFF  // 反相后0x0000 → 正色0xFFFF
//#define BLACK         	 0x0000  // 反相后0xFFFF → 正色0x0000
//#define BRED             0xF81F  // 反相后0x07E0 → 正色0xF81F
//#define GRED 			 0xFFE0  // 反相后0x001F → 正色0xFFE0
//#define RED           	 0xF800  // 反相后0x07FF → 正色0xF800
//#define PALE_SILVER      0xCD73  // RGB(204,188,188) → 正色RGB(51,67,67) → RGB565 0xCD73
//#define STEEL_BLUE       0x4B54  // RGB(75,107,164) → 正色RGB(180,148,91) → RGB565 0xB495
//#define DARK_GRAY        0x4208  // RGB(64,64,64) → 正色RGB(191,191,191) → RGB565 0xFFFF
//#define DARK_RED_BROWN   0x57C0  // RGB(87,25,2) → 正色RGB(168,230,253) → RGB565 0xA7FF
//#define GOLDEN_YELLOW    0xFB20  // RGB(251,202,3) → 正色RGB(4,53,252) → RGB565 0x02BF
//#define OLIVE_GREEN      0x4685  // RGB(70,114,5) → 正色RGB(185,141,250) → RGB565 0xBC7F
//#define DARK_BROWN       0x4D40  // RGB(77,55,3) → 正色RGB(178,200,252) → RGB565 0xB3FF
//#define MIDNIGHT_BLUE    0x1311  // RGB(19,17,118) → 正色RGB(236,238,137) → RGB565 0xEF76
//#define COBALT_BLUE      0x034A  // RGB(3,74,148) → 正色RGB(252,181,107) → RGB565 0xFDAD
//#define CRIMSON_RED      0xA60F  // RGB(166,15,20) → 正色RGB(89,240,235) → RGB565 0x5F9D
//#define CORAL_PINK       0xFE6F  // RGB(254,111,113) → 正色RGB(1,144,142) → RGB565 0x0492
//#define SAGE_GREEN       0x9394  // RGB(147,148,108) → 正色RGB(108,107,147) → RGB565 0x6B53
//#define LIME_GREEN       0x88D3  // RGB(136,211,26) → 正色RGB(119,44,229) → RGB565 0x773D


void LCD_Init(void);			   //初始化
void LCD_Clear(uint16_t Color);	   //清屏
void LCD_Display_Dir(uint8_t dir); //设置LCD显示方向
void LCD_Turn_On_Backlight(void);
void Show_Str(uint16_t x, uint16_t y,uint8_t *str,uint8_t size,uint8_t mode);//显示中文
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);

void Show_Float(uint16_t x, uint16_t y, float value,uint8_t size,uint8_t mode);
void Show_Float_2(uint16_t x, uint16_t y, float value,uint8_t size,uint8_t mode);

void LCD_Draw_Circle(u16 x0,u16 y0,uint8_t r);
void LCD_Draw_Ring(u16 x0, u16 y0, u16 r, u16 width);

void LCD_ShowHeitiChar2010(uint16_t x, uint16_t y, char ch);
void LCD_ShowHeitiString2010(uint16_t x, uint16_t y, const char *str);
void LCD_ShowSongtiChar2010(uint16_t x, uint16_t y, char ch, uint8_t isCover);
void LCD_ShowSongtiString2010(uint16_t x, uint16_t y, const char *str,uint8_t isCover);

void LCD_ShowHeitiChar2412(uint16_t x, uint16_t y, char ch);
void LCD_ShowHeitiString2412(uint16_t x, uint16_t y, const char *str);

void LCD_ShowSongtiChar1206(uint16_t x, uint16_t y, char ch);
void LCD_ShowSongtiString1206(uint16_t x, uint16_t y, const char *str);

void LCD_ShowSongtiChar1407(uint16_t x, uint16_t y, char ch);
void LCD_ShowSongtiString1407(uint16_t x, uint16_t y, const char *str);

void LCD_ShowSongtiChar2814(uint16_t x, uint16_t y, char ch);
void LCD_ShowSongtiString2814(uint16_t x, uint16_t y, const char *str);

void LCD_ShowHeitiChar2814(uint16_t x, uint16_t y, char ch);
void LCD_ShowHeitiString2814(uint16_t x, uint16_t y, const char *str);

#endif //ST7796_SPI_LCD_H
