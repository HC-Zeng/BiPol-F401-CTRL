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

////画笔颜色
//#define WHITE         	 0xFFFF
//#define BLACK         	 0x0000
//#define BLUE         	     0x001F
//#define BRED               0XF81F
//#define GRED 			     0XFFE0
//#define GBLUE			     0X07FF
//#define RED           	 0xF800
//#define MAGENTA       	 0xF81F
//#define GREEN         	 0x07E0
//#define CYAN          	 0x7FFF
//#define YELLOW        	 0xFFE0
//#define BROWN 			 0XBC40 //棕色
//#define BRRED 			 0XFC07 //棕红色
//#define GRAY  			 0X8430 //灰色
////GUI颜色
//
//#define DARKBLUE      	 0X01CF	//深蓝色
//#define LIGHTBLUE      	 0X7D7C	//浅蓝色
//#define GRAYBLUE       	 0X5458 //灰蓝色
////以上三色为PANEL的颜色
//
//#define LIGHTGREEN     	 0X841F //浅绿色
//#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色
//
//#define LGRAYBLUE          0XA651 //浅灰蓝色(中间层颜色)
//#define LBBLUE             0X2B12 //浅棕蓝色(选择条目的反色)

// 反相后的画笔颜色
#define WHITE         	 0x0000  // 原0xFFFF → 反相后0x0000
#define BLACK         	 0xFFFF  // 原0x0000 → 反相后0xFFFF
#define BLUE         	 0xFFE0  // 原0x001F → 反相后0xFFE0
#define BRED             0x07E0  // 原0xF81F → 反相后0x07E0
#define GRED 			 0x001F  // 原0xFFE0 → 反相后0x001F
#define GBLUE			 0xF800  // 原0x07FF → 反相后0xF800
#define RED           	 0x07FF  // 原0xF800 → 反相后0x07FF
#define MAGENTA       	 0x07E0  // 原0xF81F → 反相后0x07E0
#define GREEN         	 0xF81F  // 原0x07E0 → 反相后0xF81F
#define CYAN          	 0x8000  // 原0x7FFF → 反相后0x8000
#define YELLOW        	 0x001F  // 原0xFFE0 → 反相后0x001F
#define BROWN 			 0x43BF  // 原0xBC40 → 反相后0x43BF
#define BRRED 			 0x03F8  // 原0xFC07 → 反相后0x03F8
#define GRAY  			 0x7BCF  // 原0x8430 → 反相后0x7BCF

// GUI颜色
#define DARKBLUE      	 0xFE30  // 原0x01CF → 反相后0xFE30
#define LIGHTBLUE      	 0x8283  // 原0x7D7C → 反相后0x8283
#define GRAYBLUE       	 0xABA7  // 原0x5458 → 反相后0xABA7

#define LIGHTGREEN     	 0x7BE0  // 原0x841F → 反相后0x7BE0
#define LGRAY 			 0x39E7  // 原0xC618 → 反相后0x39E7

#define LGRAYBLUE        0x59AE  // 原0xA651 → 反相后0x59AE
#define LBBLUE           0xD4ED  // 原0x2B12 → 反相后0xD4ED

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
void LCD_ShowSongtiChar2010(uint16_t x, uint16_t y, char ch);
void LCD_ShowSongtiString2010(uint16_t x, uint16_t y, const char *str);

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
