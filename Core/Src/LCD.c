#include "lcd.h"
#include "font.h"
#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "stdio.h"
#include <math.h>

//LCD的画笔颜色和背景色
uint16_t POINT_COLOR=WHITE;	//画笔颜色
uint16_t BACK_COLOR=BLACK;  //背景色

//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;

#define LCD_SPI        hspi3  // CubeMX生成的SPI句柄
#define LCD_CS_PIN     CS_Pin
#define LCD_CS_PORT    CS_GPIO_Port
#define LCD_DC_PIN     DC_Pin
#define LCD_DC_PORT    DC_GPIO_Port
#define LCD_RST_PIN    RST_Pin
#define LCD_RST_PORT   RST_GPIO_Port
#define LCD_LED_PIN    LED_Pin
#define LCD_LED_PORT   LED_GPIO_Port

// 发送命令（DC=0）
void LCD_WR_REG(uint8_t cmd) {
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET); // DC=0
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET); // CS=0
    HAL_SPI_Transmit(&LCD_SPI, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);   // CS=1
}

// 发送数据（DC=1）
void LCD_WR_DATA(uint8_t data) {
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);   // DC=1
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET); // CS=0
    HAL_SPI_Transmit(&LCD_SPI, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);   // CS=1
}

// 批量发送数据（优化连续传输）
void LCD_WR_DATA_BULK(uint8_t *data, uint32_t len) {
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);   // DC=1
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET); // CS=0
    HAL_SPI_Transmit(&LCD_SPI, data, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);   // CS=1
}

//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
    LCD_WR_REG(lcddev.wramcmd);
}

/**
  * @brief  LCD写GRAM数据（16位颜色值）
  * @param  RGB_Code: 16位颜色值（RGB565格式）
  * @retval None
  */
void LCD_WriteRAM(uint16_t RGB_Code)
{
    // 使用HAL库的SPI传输函数
    uint8_t data[2];

    // 拆分16位颜色值为两个8位数据（大端序）
    data[0] = (RGB_Code >> 8) & 0xFF;  // 高字节
    data[1] = RGB_Code & 0xFF;         // 低字节

    // 片选拉低
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

    // 设置为数据模式（DC=1）
    HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);

    // 通过SPI发送16位数据
    HAL_SPI_Transmit(&LCD_SPI, data, 2, HAL_MAX_DELAY);

    // 片选拉高
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}

//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(Xpos>>8);LCD_WR_DATA(Xpos&0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(Ypos>>8);LCD_WR_DATA(Ypos&0XFF);
}

//写寄存器
//LCD_Reg:寄存器编号
//LCD_RegValue:要写入的值
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{
    LCD_WR_REG(LCD_Reg);
    LCD_WriteRAM(LCD_RegValue);
}

//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(uint16_t x,uint16_t y)
{
    LCD_SetCursor(x,y);		//设置光标位置
    LCD_WriteRAM_Prepare();	//开始写入GRAM
    LCD_WriteRAM(POINT_COLOR);
}

//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
    //设置光标位置
    LCD_SetCursor(x,y);
    //写入颜色
    LCD_WriteReg(lcddev.wramcmd,color);
}

//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height.
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{
    uint16_t twidth,theight;
    twidth=sx+width-1;
    theight=sy+height-1;

    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(sx>>8);
    LCD_WR_DATA(sx&0XFF);
    LCD_WR_DATA(twidth>>8);
    LCD_WR_DATA(twidth&0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(sy>>8);
    LCD_WR_DATA(sy&0XFF);
    LCD_WR_DATA(theight>>8);
    LCD_WR_DATA(theight&0XFF);

}

//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{
    uint8_t temp,t1,t;
    uint16_t y0=y;
    uint8_t csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应点阵集所占的字节数
    num=num-' ';//得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）
    for(t=0;t<csize;t++)
    {
        if(size==16)temp=asc2_1608[num][t];	//调用1608字体
        else if(size==24)
        {
            //temp=asc2_2412[num][t];	//调用2412字体
        }
        else return;								//没有的字库
        for(t1=0;t1<8;t1++)
        {
            if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
            else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
            temp<<=1;
            y++;
            if(y>=lcddev.height)return;		//超区域了
            if((y-y0)==size)
            {
                y=y0;
                x++;
                if(x>=lcddev.width)return;	//超区域了
                break;
            }
        }
    }
}


//显示汉字或者字符串
void Show_Str(uint16_t x, uint16_t y,uint8_t *str,uint8_t size,uint8_t mode)
{
    uint16_t x0=x;
    uint8_t bHz=0;     //字符或者中文
    while(*str!=0)//数据未结束
    {
        if(!bHz)
        {
            if(x>(lcddev.width-size/2)||y>(lcddev.height-size))
                return;
            if(*str>0x80)bHz=1;//中文
            else              //字符
            {
                if(*str==0x0D)//换行符号
                {
                    y+=size;
                    x=x0;
                    str++;
                }
                else
                {
                    if(size>=24)//字库中没有集成12X24 16X32的英文字体,用8X16代替
                    {
                        LCD_ShowChar(x,y,*str,24,mode);
                        x+=12; //字符,为全字的一半
                    }
                    else
                    {
                        LCD_ShowChar(x,y,*str,size,mode);
                        x+=size/2; //字符,为全字的一半
                    }
                }
                str++;

            }
        }
    }
}

#define u32 uint32_t
#define u8 uint8_t
#define u16 uint16_t


//m^n函数
//返回值:m^n次方.
u32 LCD_Pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--)result*=m;
    return result;
}

//显示数字,高位为0,则不显示
//x,y :起点坐标
//len :数字的位数
//size:字体大小
//color:颜色
//num:数值(0~4294967295);
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{
    u8 t,temp;
    u8 enshow=0;
    for(t=0;t<len;t++)
    {
        temp=(num/LCD_Pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
                continue;
            }else enshow=1;

        }
        LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0);
    }
}


//清屏函数
//color:要清屏的填充色
void LCD_Clear(uint16_t color)
{
    uint32_t index=0;
    uint32_t totalpoint=lcddev.width;
    totalpoint*=lcddev.height; 			     //得到总点数
    LCD_SetCursor(0x00,0x0000);	 //设置光标位置
    LCD_WriteRAM_Prepare();     		     //开始写入GRAM
    for(index=0;index<totalpoint;index++)LCD_WriteRAM(color);
}

void LCD_Init(void) {
    // 1. 复位序列
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(120);

    //************* Start Initial Sequence **********//
    HAL_Delay(120); // Delay 120ms
    LCD_WR_REG(0x11); // Sleep Out
    HAL_Delay(120); // Delay 120ms
    LCD_WR_REG(0xf0) ;
    LCD_WR_DATA(0xc3) ;
    LCD_WR_REG(0xf0) ;
    LCD_WR_DATA(0x96) ;
    LCD_WR_REG(0x36);
    LCD_WR_DATA(0x48);
    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);
    LCD_WR_REG(0xB4);
    LCD_WR_DATA(0x01);
    LCD_WR_REG(0xB7) ;
    LCD_WR_DATA(0xC6) ;
    LCD_WR_REG(0xe8);
    LCD_WR_DATA(0x40);
    LCD_WR_DATA(0x8a);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x29);
    LCD_WR_DATA(0x19);
    LCD_WR_DATA(0xa5);
    LCD_WR_DATA(0x33);
    LCD_WR_REG(0xc1);
    LCD_WR_DATA(0x06);
    LCD_WR_REG(0xc2);
    LCD_WR_DATA(0xa7);
    LCD_WR_REG(0xc5);
    LCD_WR_DATA(0x18);
    LCD_WR_REG(0xe0); //Positive Voltage Gamma Control
    LCD_WR_DATA(0xf0);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x0b);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x15);
    LCD_WR_DATA(0x2f);
    LCD_WR_DATA(0x54);
    LCD_WR_DATA(0x42);
    LCD_WR_DATA(0x3c);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x18);
    LCD_WR_DATA(0x1b);
    LCD_WR_REG(0xe1); //Negative Voltage Gamma Control
    LCD_WR_DATA(0xf0);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x0b);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x2d);
    LCD_WR_DATA(0x43);
    LCD_WR_DATA(0x42);
    LCD_WR_DATA(0x3b);
    LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x1b);
    LCD_WR_REG(0xf0);
    LCD_WR_DATA(0x3c);
    LCD_WR_REG(0xf0);
    LCD_WR_DATA(0x69);
    HAL_Delay(120); //Delay 120ms
    LCD_WR_REG(0x29);// Display on
}

//dir:方向选择 	0-0度旋转，1-180度旋转，2-270度旋转，3-90度旋转
void LCD_Display_Dir(uint8_t dir)
{
    if(dir==0||dir==1)			//竖屏
    {
        lcddev.dir=0;	//竖屏
        lcddev.width=320;
        lcddev.height=480;

        lcddev.wramcmd=0X2C;
        lcddev.setxcmd=0X2A;
        lcddev.setycmd=0X2B;

        if(dir==0)        //0-0度旋转
        {
            LCD_WR_REG(0x36);
            LCD_WR_DATA((1<<3)|(0<<7)|(1<<6)|(0<<5));
        }else							//1-180度旋转
        {
            LCD_WR_REG(0x36);
            LCD_WR_DATA((1<<3)|(1<<7)|(0<<6)|(0<<5));
        }

    }else if(dir==2||dir==3)
    {

        lcddev.dir=1;	//横屏
        lcddev.width=480;
        lcddev.height=320;

        lcddev.wramcmd=0X2C;
        lcddev.setxcmd=0X2A;
        lcddev.setycmd=0X2B;

        if(dir==2)				//2-270度旋转
        {
            LCD_WR_REG(0x36);
            LCD_WR_DATA((1<<3)|(1<<7)|(1<<6)|(1<<5));

        }else							//3-90度旋转
        {
            LCD_WR_REG(0x36);
            LCD_WR_DATA((1<<3)|(0<<7)|(0<<6)|(1<<5));
        }
    }


    //设置显示区域
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(0);LCD_WR_DATA(0);
    LCD_WR_DATA((lcddev.width-1)>>8);LCD_WR_DATA((lcddev.width-1)&0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(0);LCD_WR_DATA(0);
    LCD_WR_DATA((lcddev.height-1)>>8);LCD_WR_DATA((lcddev.height-1)&0XFF);
}

void LCD_Turn_On_Backlight(void)
{
    HAL_GPIO_WritePin(LCD_LED_PORT,LCD_LED_PIN,1);  // Turn on backlight
}

//画线
//x1,y1:起点坐标
//x2,y2:终点坐标
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
    u16 t;
    int xerr=0,yerr=0,delta_x,delta_y,distance;
    int incx,incy,uRow,uCol;
    delta_x=x2-x1; //计算坐标增量
    delta_y=y2-y1;
    uRow=x1;
    uCol=y1;
    if(delta_x>0)incx=1; //设置单步方向
    else if(delta_x==0)incx=0;//垂直线
    else {incx=-1;delta_x=-delta_x;}
    if(delta_y>0)incy=1;
    else if(delta_y==0)incy=0;//水平线
    else{incy=-1;delta_y=-delta_y;}
    if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴
    else distance=delta_y;
    for(t=0;t<=distance+1;t++ )//画线输出
    {
        LCD_DrawPoint(uRow,uCol);//画点
        xerr+=delta_x ;
        yerr+=delta_y ;
        if(xerr>distance)
        {
            xerr-=distance;
            uRow+=incx;
        }
        if(yerr>distance)
        {
            yerr-=distance;
            uCol+=incy;
        }
    }
}

//画矩形
//(x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
    LCD_DrawLine(x1,y1,x2,y1);
    LCD_DrawLine(x1,y1,x1,y2);
    LCD_DrawLine(x1,y2,x2,y2);
    LCD_DrawLine(x2,y1,x2,y2);
}

//在指定区域内填充指定颜色
//区域大小:(xend-xsta+1)*(yend-ysta+1)
//xsta
//color:要填充的颜色
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{
    u16 i,j;
    u16 xlen=0;
    u16 temp;
    if((lcddev.id==0X6804)&&(lcddev.dir==1))	//6804横屏的时候特殊处理
    {
        temp=sx;
        sx=sy;
        sy=lcddev.width-ex-1;
        ex=ey;
        ey=lcddev.width-temp-1;
        lcddev.dir=0;
        lcddev.setxcmd=0X2A;
        lcddev.setycmd=0X2B;
        LCD_Fill(sx,sy,ex,ey,color);
        lcddev.dir=1;
        lcddev.setxcmd=0X2B;
        lcddev.setycmd=0X2A;
    }else
    {
        xlen=ex-sx+1;
        for(i=sy;i<=ey;i++)
        {
            LCD_SetCursor(sx,i);      				//设置光标位置
            LCD_WriteRAM_Prepare();     			//开始写入GRAM
            for(j=0;j<xlen;j++)LCD_WriteRAM(color);	//设置光标位置
        }
    }
}


// 显示浮点数，整数部分1-2位，小数部分1位
void Show_Float(uint16_t x, uint16_t y, float value,uint8_t size,uint8_t mode)
{
    char buffer[10]; // 足够存放"xx.x"格式的数字

    // 格式化为：整数部分最多2位，小数部分1位
    sprintf(buffer, "%.1f", value);

    Show_Str(x, y, buffer, size, mode);
}

// 显示浮点数，整数部分1位，小数部分2位
void Show_Float_2(uint16_t x, uint16_t y, float value,uint8_t size,uint8_t mode)
{
    char buffer[10]; // 足够存放"xx.x"格式的数字

    // 格式化为：整数部分最多2位，小数部分1位
    sprintf(buffer, "%.2f", value);

    Show_Str(x, y, buffer, size, mode);
}

//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
//void LCD_Draw_Circle(u16 x0,u16 y0,u8 r)
//{
//    int a,b;
//    int di;
//    a=0;b=r;
//    di=3-(r<<1);             //判断下个点位置的标志
//    while(a<=b)
//    {
//        LCD_DrawPoint(x0+a,y0-b);             //5
//        LCD_DrawPoint(x0+b,y0-a);             //0
//        LCD_DrawPoint(x0+b,y0+a);             //4
//        LCD_DrawPoint(x0+a,y0+b);             //6
//        LCD_DrawPoint(x0-a,y0+b);             //1
//        LCD_DrawPoint(x0-b,y0+a);
//        LCD_DrawPoint(x0-a,y0-b);             //2
//        LCD_DrawPoint(x0-b,y0-a);             //7
//        a++;
//        //使用Bresenham算法画圆
//        if(di<0)di +=4*a+6;
//        else
//        {
//            di+=10+4*(a-b);
//            b--;
//        }
//    }
//}

void LCD_Draw_Circle(u16 x0,u16 y0,u8 r)
{
    u32 i;
    u32 imax = ((u32)r*707)/1000+1;
    u32 sqmax = (u32)r*(u32)r+(u32)r/2;
    u32 x=r;
    LCD_DrawLine(x0-r,y0,x0+r,y0);
    for (i=1;i<=imax;i++)
    {
        if ((i*i+x*x)>sqmax)// draw lines from outside
        {
            if (x>imax)
            {
                LCD_DrawLine(x0-i+1,y0+x,x0-i+1+2*(i-1),y0+x);
                LCD_DrawLine(x0-i+1,y0-x,x0-i+1+2*(i-1),y0-x);
            }
            x--;
        }
        // draw lines from inside (center)
        LCD_DrawLine(x0-x,y0+i,x0+x,y0+i);
        LCD_DrawLine(x0-x,y0-i,x0+x,y0-i);
    }
}

void LCD_ShowHeitiChar2010(uint16_t x, uint16_t y, char ch)
{
    uint8_t idx = 0;
    switch(ch) {
        case '0': idx = 0; break;
        case '1': idx = 1; break;
        case '2': idx = 2; break;
        case '3': idx = 3; break;
        case '4': idx = 4; break;
        case '5': idx = 5; break;
        case '6': idx = 6; break;
        case '7': idx = 7; break;
        case '8': idx = 8; break;
        case '9': idx = 9; break;
        case 'A': idx = 10; break;
        case 'B': idx = 11; break;
        case 'C': idx = 12; break;
        case 'D': idx = 13; break;
        case 'E': idx = 14; break;
        case 'F': idx = 15; break;
        case 'G': idx = 16; break;
        case 'H': idx = 17; break;
        case 'I': idx = 18; break;
        case 'J': idx = 19; break;
        case 'K': idx = 20; break;
        case 'L': idx = 21; break;
        case 'M': idx = 22; break;
        case 'N': idx = 23; break;
        case 'O': idx = 24; break;
        case 'P': idx = 25; break;
        case 'Q': idx = 26; break;
        case 'R': idx = 27; break;
        case 'S': idx = 28; break;
        case 'T': idx = 29; break;
        case 'U': idx = 30; break;
        case 'V': idx = 31; break;
        case 'W': idx = 32; break;
        case 'X': idx = 33; break;
        case 'Y': idx = 34; break;
        case 'Z': idx = 35; break;
        case '/': idx = 36; break;
        case '.': idx = 37; break;
        case ':': idx = 38; break;
        default: idx = 255; break; // 无效字符返回255
    }
    uint16_t temp;
    for(uint8_t i=0;i<20;i++)
    {
        temp = (Heiti_2010[idx][2 * i + 1] << 8) + (Heiti_2010[idx][2 * i]);
        for(uint8_t j=0;j<10;j++)
        {
            if(temp&0x0001)
            {
                LCD_Fast_DrawPoint(x+j,y+i,POINT_COLOR);
            }
            else
            {
                LCD_Fast_DrawPoint(x+j,y+i,BACK_COLOR);
            }
            temp>>=1;
        }
    }
}

void LCD_ShowHeitiString2010(uint16_t x, uint16_t y, const char *str)
{
    uint16_t current_x = x;

    // 遍历字符串中的每个字符
    while (*str != '\0') {
        LCD_ShowHeitiChar2010(current_x, y, *str);
        current_x += 10;  // 每个字符宽度为10像素，根据实际情况调整
        str++;  // 移动到下一个字符
    }
}

void LCD_ShowSongtiChar2010(uint16_t x, uint16_t y, char ch, uint8_t isCover) {
    uint8_t idx = 0;
    switch(ch) {
        case '0': idx = 0; break;
        case '1': idx = 1; break;
        case '2': idx = 2; break;
        case '3': idx = 3; break;
        case '4': idx = 4; break;
        case '5': idx = 5; break;
        case '6': idx = 6; break;
        case '7': idx = 7; break;
        case '8': idx = 8; break;
        case '9': idx = 9; break;
        case 'A': idx = 10; break;
        case 'B': idx = 11; break;
        case 'C': idx = 12; break;
        case 'D': idx = 13; break;
        case 'E': idx = 14; break;
        case 'F': idx = 15; break;
        case 'G': idx = 16; break;
        case 'H': idx = 17; break;
        case 'I': idx = 18; break;
        case 'J': idx = 19; break;
        case 'K': idx = 20; break;
        case 'L': idx = 21; break;
        case 'M': idx = 22; break;
        case 'N': idx = 23; break;
        case 'O': idx = 24; break;
        case 'P': idx = 25; break;
        case 'Q': idx = 26; break;
        case 'R': idx = 27; break;
        case 'S': idx = 28; break;
        case 'T': idx = 29; break;
        case 'U': idx = 30; break;
        case 'V': idx = 31; break;
        case 'W': idx = 32; break;
        case 'X': idx = 33; break;
        case 'Y': idx = 34; break;
        case 'Z': idx = 35; break;
        case '/': idx = 36; break;
        case '.': idx = 37; break;
        case ':': idx = 38; break;
        case 'n': idx = 39; break;
        case 't': idx = 40; break;
        default: idx = 255; break; // 无效字符返回255
    }
    uint16_t temp;
    for(uint8_t i=0;i<20;i++)
    {
        temp = (Songti_2010[idx][2 * i + 1] << 8) + (Songti_2010[idx][2 * i]);
        for(uint8_t j=0;j<10;j++)
        {
            if(temp&0x0001)
            {
                LCD_Fast_DrawPoint(x+j,y+i,POINT_COLOR);
            }
            else
            {
                if(isCover)
                {
                    LCD_Fast_DrawPoint(x+j,y+i,BACK_COLOR);
                }
            }
            temp>>=1;
        }
    }
}

void LCD_ShowSongtiString2010(uint16_t x, uint16_t y, const char *str, uint8_t isCover)
{
    uint16_t current_x = x;

    // 遍历字符串中的每个字符
    while (*str != '\0') {
        LCD_ShowSongtiChar2010(current_x, y, *str, isCover);
        current_x += 10;  // 每个字符宽度为10像素，根据实际情况调整
        str++;  // 移动到下一个字符
    }
}

void LCD_ShowHeitiChar2412(uint16_t x, uint16_t y, char ch) {
    uint8_t idx = 0;
    switch(ch) {
        case 'A': idx = 0; break;
        case 'B': idx = 1; break;
        case 'C': idx = 2; break;
        case 'D': idx = 3; break;
        case 'E': idx = 4; break;
        case 'F': idx = 5; break;
        case 'G': idx = 6; break;
        case 'H': idx = 7; break;
        case 'I': idx = 8; break;
        case 'J': idx = 9; break;
        case 'K': idx = 10; break;
        case 'L': idx = 11; break;
        case 'M': idx = 12; break;
        case 'N': idx = 13; break;
        case 'O': idx = 14; break;
        case 'P': idx = 15; break;
        case 'Q': idx = 16; break;
        case 'R': idx = 17; break;
        case 'S': idx = 18; break;
        case 'T': idx = 19; break;
        case 'U': idx = 20; break;
        case 'V': idx = 21; break;
        case 'W': idx = 22; break;
        case 'X': idx = 23; break;
        case 'Y': idx = 24; break;
        case 'Z': idx = 25; break;
        case '1': idx = 26; break;
        case '2': idx = 27; break;
        case '3': idx = 28; break;
        case '4': idx = 29; break;
        case '5': idx = 30; break;
        case '6': idx = 31; break;
        case '7': idx = 32; break;
        case '8': idx = 33; break;
        case '9': idx = 34; break;
        case '0': idx = 35; break;
        default: idx = 255; break; // 无效字符返回255
    }
    uint16_t temp;
    for(uint8_t i=0;i<24;i++)
    {
        temp = (Heiti_2412[idx][2 * i + 1] << 8) + (Heiti_2412[idx][2 * i]);
        for(uint8_t j=0;j<12;j++)
        {
            if(temp&0x0001)
            {
                LCD_Fast_DrawPoint(x+j,y+i,POINT_COLOR);
            }
            temp>>=1;
        }
    }
}

void LCD_ShowHeitiString2412(uint16_t x, uint16_t y, const char *str)
{
    uint16_t current_x = x;

    // 遍历字符串中的每个字符
    while (*str != '\0') {
        LCD_ShowHeitiChar2412(current_x, y, *str);
        current_x += 12;  // 每个字符宽度为12像素，根据实际情况调整
        str++;  // 移动到下一个字符
    }
}

void LCD_ShowSongtiChar1206(uint16_t x, uint16_t y, char ch) {
    uint8_t idx = 0;
    switch(ch) {
        case '1': idx = 0; break;
        case '2': idx = 1; break;
        default: idx = 255; break; // 无效字符返回255
    }
    uint16_t temp;
    for(uint8_t i=0;i<12;i++)
    {
        temp = Songti_1206[idx][i];
        for(uint8_t j=0;j<8;j++)
        {
            if(temp&0x01)
            {
                LCD_Fast_DrawPoint(x+j,y+i,POINT_COLOR);
            }
            temp>>=1;
        }
    }
}

void LCD_ShowSongtiString1206(uint16_t x, uint16_t y, const char *str)
{
    uint16_t current_x = x;

    // 遍历字符串中的每个字符
    while (*str != '\0') {
        LCD_ShowSongtiChar1206(current_x, y, *str);
        current_x += 6;  // 每个字符宽度为10像素，根据实际情况调整
        str++;  // 移动到下一个字符
    }
}

void LCD_ShowSongtiChar1407(uint16_t x, uint16_t y, char ch) {
    uint8_t idx = 0;
    switch(ch) {
        case '1': idx = 0; break;
        case '2': idx = 1; break;
        case '3': idx = 2; break;
        case '4': idx = 3; break;
        case '5': idx = 4; break;
        case '6': idx = 5; break;
        case '7': idx = 6; break;
        case '8': idx = 7; break;
        case '9': idx = 8; break;
        case '0': idx = 9; break;
        case '.': idx = 10; break;
        case 'V': idx = 11; break;
        default: idx = 255; break; // 无效字符返回255
    }
    uint16_t temp;
    for(uint8_t i=0;i<14;i++)
    {
        temp = Songti_1407[idx][i];
        for(uint8_t j=0;j<7;j++)
        {
            if(temp&0x01)
            {
                LCD_Fast_DrawPoint(x+j,y+i,POINT_COLOR);
            }
            else
            {
                LCD_Fast_DrawPoint(x+j,y+i,BACK_COLOR);
            }
            temp>>=1;
        }
    }
}

void LCD_ShowSongtiString1407(uint16_t x, uint16_t y, const char *str)
{
    uint16_t current_x = x;

    // 遍历字符串中的每个字符
    while (*str != '\0') {
        LCD_ShowSongtiChar1407(current_x, y, *str);
        current_x += 8;  // 每个字符宽度为10像素，根据实际情况调整
        str++;  // 移动到下一个字符
    }
}

void LCD_ShowSongtiChar2814(uint16_t x, uint16_t y, char ch) {
    uint8_t idx = 0;
    switch(ch) {
        case '1': idx = 0; break;
        case '2': idx = 1; break;
        case '3': idx = 2; break;
        case '4': idx = 3; break;
        case '5': idx = 4; break;
        case '6': idx = 5; break;
        case '7': idx = 6; break;
        case '8': idx = 7; break;
        case '9': idx = 8; break;
        case '0': idx = 9; break;
        case '.': idx = 10; break;
        case 'V': idx = 11; break;
        default: idx = 255; break; // 无效字符返回255
    }
    uint16_t temp;
    for(uint8_t i=0;i<28;i++)
    {
        temp = (Songti_2814[idx][2 * i + 1] << 8) + (Songti_2814[idx][2 * i]);
        for(uint8_t j=0;j<14;j++)
        {
            if(temp&0x0001)
            {
                LCD_Fast_DrawPoint(x+j,y+i,POINT_COLOR);
            }
            temp>>=1;
        }
    }
}

void LCD_ShowSongtiString2814(uint16_t x, uint16_t y, const char *str)
{
    uint16_t current_x = x;

    // 遍历字符串中的每个字符
    while (*str != '\0') {
        LCD_ShowSongtiChar2814(current_x, y, *str);
        current_x += 14;  // 每个字符宽度为10像素，根据实际情况调整
        str++;  // 移动到下一个字符
    }
}

void LCD_ShowHeitiChar2814(uint16_t x, uint16_t y, char ch) {
    uint8_t idx = 0;
    switch(ch) {
        case '1': idx = 0; break;
        case '2': idx = 1; break;
        case '3': idx = 2; break;
        case '4': idx = 3; break;
        case '5': idx = 4; break;
        case '6': idx = 5; break;
        case '7': idx = 6; break;
        case '8': idx = 7; break;
        case '9': idx = 8; break;
        case '0': idx = 9; break;
        case '.': idx = 10; break;
        case 'V': idx = 11; break;
        case ' ': idx = 12; break;
        default: idx = 255; break; // 无效字符返回255
    }
    uint16_t temp;
    for(uint8_t i=0;i<28;i++)
    {
        temp = (Heiti_2814[idx][2 * i + 1] << 8) + (Heiti_2814[idx][2 * i]);
        for(uint8_t j=0;j<14;j++)
        {
            if(temp&0x0001)
            {
                LCD_Fast_DrawPoint(x+j,y+i,POINT_COLOR);
            }
            else
            {
                LCD_Fast_DrawPoint(x+j,y+i,BACK_COLOR);
            }
            temp>>=1;
        }
    }
}

void LCD_ShowHeitiString2814(uint16_t x, uint16_t y, const char *str)
{
    uint16_t current_x = x;

    // 遍历字符串中的每个字符
    while (*str != '\0') {
        LCD_ShowHeitiChar2814(current_x, y, *str);
        current_x += 14;  // 每个字符宽度为10像素，根据实际情况调整
        str++;  // 移动到下一个字符
    }
}