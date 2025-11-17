#include "ui.h"


void Draw_RoundedRectangleBody(u16 bbx, u16 bby)
{
    /*
     * 画圆角矩形的填充
     * */
    u16 bx;
    u16 by;

    bx=bbx;
    by=bby;
    LCD_Fill(bx+12,by+3,bx+81,by+48, STEEL_BLUE);
    LCD_Fill(bx+3,by+12,bx+12,by+39, STEEL_BLUE);
    LCD_Fill(bx+81,by+12,bx+90,by+39, STEEL_BLUE);

    LCD_Fill(bx+6,by+6,bx+12,by+12, STEEL_BLUE);
    LCD_Fill(bx+6,by+5,bx+7,by+5, STEEL_BLUE);
    LCD_Fill(bx+8,by+4,bx+9,by+5, STEEL_BLUE);
    LCD_Fill(bx+10,by+3,bx+12,by+5, STEEL_BLUE);
    LCD_Fill(bx+5,by+6,bx+5,by+7, STEEL_BLUE);
    LCD_Fill(bx+4,by+8,bx+5,by+9, STEEL_BLUE);
    LCD_Fill(bx+3,by+10,bx+5,by+12, STEEL_BLUE);

    bx=bbx+81;
    LCD_Fill(bx+0,by+6,bx+6,by+12, STEEL_BLUE);
    LCD_Fill(bx+0,by+3,bx+2,by+5, STEEL_BLUE);
    LCD_Fill(bx+3,by+4,bx+4,by+5, STEEL_BLUE);
    LCD_Fill(bx+5,by+5,bx+6,by+5, STEEL_BLUE);
    LCD_Fill(bx+7,by+6,bx+7,by+7, STEEL_BLUE);
    LCD_Fill(bx+7,by+8,bx+8,by+9, STEEL_BLUE);
    LCD_Fill(bx+7,by+10,bx+9,by+12, STEEL_BLUE);

    bx=bbx+81;
    by=bby+39;
    LCD_Fill(bx+0,by+0,bx+6,by+6, STEEL_BLUE);
    LCD_Fill(bx+0,by+7,bx+2,by+9, STEEL_BLUE);
    LCD_Fill(bx+3,by+7,bx+4,by+8, STEEL_BLUE);
    LCD_Fill(bx+5,by+7,bx+6,by+7, STEEL_BLUE);
    LCD_Fill(bx+7,by+5,bx+7,by+6, STEEL_BLUE);
    LCD_Fill(bx+7,by+3,bx+8,by+4, STEEL_BLUE);
    LCD_Fill(bx+7,by+0,bx+9,by+2, STEEL_BLUE);

    bx=bbx;
    by=bby+39;
    LCD_Fill(bx+6,by+0,bx+12,by+6, STEEL_BLUE);
    LCD_Fill(bx+3,by+0,bx+5,by+2, STEEL_BLUE);
    LCD_Fill(bx+4,by+3,bx+5,by+4, STEEL_BLUE);
    LCD_Fill(bx+5,by+5,bx+5,by+6, STEEL_BLUE);
    LCD_Fill(bx+6,by+7,bx+7,by+7, STEEL_BLUE);
    LCD_Fill(bx+8,by+7,bx+9,by+8, STEEL_BLUE);
    LCD_Fill(bx+10,by+7,bx+12,by+9, STEEL_BLUE);

}
void Draw_RoundedRectangleEdge(u16 bbx, u16 bby)
{
    /*
     * 画圆角矩形框
     * */
    u16 bx=bbx;
    u16 by=bby;

    LCD_Fill(bx+12,by+0,bx+80,by+2, PALE_SILVER);
    LCD_Fill(bx+0,by+12,bx+2,by+38, PALE_SILVER);
    LCD_Fill(bx+12,by+49,bx+80,by+51, PALE_SILVER);
    LCD_Fill(bx+91,by+12,bx+93,by+38, PALE_SILVER);

    LCD_Fill(bx+0,by+10,bx+2,by+12, PALE_SILVER);
    LCD_Fill(bx+1,by+7,bx+3,by+9, PALE_SILVER);
    LCD_Fill(bx+2,by+5,bx+4,by+6, PALE_SILVER);
    LCD_Fill(bx+4,by+7,bx+4,by+7, PALE_SILVER);
    LCD_Fill(bx+3,by+4,bx+4,by+4, PALE_SILVER);
    LCD_Fill(bx+4,by+3,bx+4,by+3, PALE_SILVER);
    LCD_Fill(bx+5,by+5,bx+5,by+5, PALE_SILVER);
    LCD_Fill(bx+5,by+2,bx+6,by+4, PALE_SILVER);
    LCD_Fill(bx+7,by+4,bx+7,by+4, PALE_SILVER);
    LCD_Fill(bx+7,by+1,bx+9,by+3, PALE_SILVER);
    LCD_Fill(bx+10,by+0,bx+12,by+2, PALE_SILVER);

    bx=bbx+81;
    LCD_Fill(bx+0,by+0,bx+2,by+2, PALE_SILVER);
    LCD_Fill(bx+3,by+1,bx+5,by+3, PALE_SILVER);
    LCD_Fill(bx+5,by+4,bx+5,by+4, PALE_SILVER);
    LCD_Fill(bx+6,by+2,bx+7,by+4, PALE_SILVER);
    LCD_Fill(bx+7,by+5,bx+7,by+5, PALE_SILVER);
    LCD_Fill(bx+8,by+3,bx+8,by+4, PALE_SILVER);
    LCD_Fill(bx+8,by+5,bx+10,by+6, PALE_SILVER);
    LCD_Fill(bx+9,by+4,bx+9,by+4, PALE_SILVER);
    LCD_Fill(bx+8,by+7,bx+8,by+7, PALE_SILVER);
    LCD_Fill(bx+9,by+7,bx+11,by+9, PALE_SILVER);
    LCD_Fill(bx+10,by+10,bx+12,by+12, PALE_SILVER);

    bx=bbx+81;
    by=bby+39;
    LCD_Fill(bx+10,by+0,bx+12,by+2, PALE_SILVER);
    LCD_Fill(bx+9,by+3,bx+11,by+5, PALE_SILVER);
    LCD_Fill(bx+8,by+5,bx+8,by+5, PALE_SILVER);
    LCD_Fill(bx+8,by+6,bx+10,by+7, PALE_SILVER);
    LCD_Fill(bx+7,by+7,bx+7,by+7, PALE_SILVER);
    LCD_Fill(bx+8,by+8,bx+9,by+8, PALE_SILVER);
    LCD_Fill(bx+8,by+9,bx+8,by+9, PALE_SILVER);
    LCD_Fill(bx+6,by+8,bx+7,by+10, PALE_SILVER);
    LCD_Fill(bx+5,by+8,bx+5,by+8, PALE_SILVER);
    LCD_Fill(bx+3,by+9,bx+5,by+11, PALE_SILVER);
    LCD_Fill(bx+0,by+10,bx+2,by+12, PALE_SILVER);

    bx=bbx+0;
    by=bby+39;
    LCD_Fill(bx+0,by+0,bx+2,by+2, PALE_SILVER);
    LCD_Fill(bx+1,by+3,bx+3,by+5, PALE_SILVER);
    LCD_Fill(bx+4,by+5,bx+4,by+5, PALE_SILVER);
    LCD_Fill(bx+2,by+6,bx+4,by+7, PALE_SILVER);
    LCD_Fill(bx+5,by+7,bx+5,by+7, PALE_SILVER);
    LCD_Fill(bx+3,by+8,bx+3,by+8, PALE_SILVER);
    LCD_Fill(bx+4,by+8,bx+4,by+9, PALE_SILVER);
    LCD_Fill(bx+5,by+8,bx+6,by+10, PALE_SILVER);
    LCD_Fill(bx+7,by+8,bx+7,by+8, PALE_SILVER);
    LCD_Fill(bx+7,by+9,bx+9,by+11, PALE_SILVER);
    LCD_Fill(bx+10,by+10,bx+12,by+12, PALE_SILVER);
}
void Draw_RoundedRectangleCorner(u16 bbx,u16 bby)
{
    /*
     * 圆角矩形的四个角，即非圆角的那些�?
     * */
    u16 bx,by;
    // 四角
    bx=bbx;
    by=bby;
    LCD_Fill(bx+0,by+1,bx+0,by+9, DARK_GRAY);
    LCD_Fill(bx+1,by+0,bx+1,by+6, DARK_GRAY);
    LCD_Fill(bx+2,by+0,bx+9,by+0, DARK_GRAY);
    LCD_Fill(bx+2,by+1,bx+6,by+1, DARK_GRAY);

    bx=bbx+81;
    by=bby;
    LCD_Fill(bx+3,by+0,bx+11,by+0, DARK_GRAY);
    LCD_Fill(bx+6,by+1,bx+12,by+1, DARK_GRAY);
    LCD_Fill(bx+11,by+2,bx+11,by+6, DARK_GRAY);
    LCD_Fill(bx+12,by+2,bx+12,by+9, DARK_GRAY);

    bx=bbx+81;
    by=bby+39;
    LCD_Fill(bx+3,by+12,bx+10,by+12, DARK_GRAY);
    LCD_Fill(bx+6,by+11,bx+10,by+11, DARK_GRAY);
    LCD_Fill(bx+11,by+6,bx+11,by+12, DARK_GRAY);
    LCD_Fill(bx+12,by+3,bx+12,by+11, DARK_GRAY);

    bx=bbx;
    by=bby+39;
    LCD_Fill(bx+0,by+3,bx+0,by+10, DARK_GRAY);
    LCD_Fill(bx+1,by+6,bx+1,by+10, DARK_GRAY);
    LCD_Fill(bx+0,by+11,bx+6,by+11, DARK_GRAY);
    LCD_Fill(bx+1,by+12,bx+9,by+12, DARK_GRAY);
}
void Draw_RoundedRectangle(u16 bx, u16 by)
{
    /*
     * 画完整的圆角矩形
     * */
    Draw_RoundedRectangleEdge(bx, by);
    Draw_RoundedRectangleBody(bx, by);
    Draw_RoundedRectangleCorner(bx,by);
}
void Draw_WaveLine() {
    /*
     * 画时序波形图
     * */
    u16 bx = 6;
    u16 by = 152;
    LCD_Fill(bx+94, by+0, bx+179, by+1, LIME_GREEN); // Rect 1
    LCD_Fill(bx+92, by+1, bx+93, by+2, LIME_GREEN); // Rect 2
    LCD_Fill(bx+180, by+1, bx+181, by+2, LIME_GREEN); // Rect 3
    LCD_Fill(bx+90, by+2, bx+91, by+3, LIME_GREEN); // Rect 4
    LCD_Fill(bx+94, by+2, bx+94, by+2, LIME_GREEN); // Rect 5
    LCD_Fill(bx+179, by+2, bx+179, by+2, LIME_GREEN); // Rect 6
    LCD_Fill(bx+182, by+2, bx+182, by+4, LIME_GREEN); // Rect 7
    LCD_Fill(bx+89, by+3, bx+89, by+6, LIME_GREEN); // Rect 8
    LCD_Fill(bx+92, by+3, bx+92, by+3, LIME_GREEN); // Rect 9
    LCD_Fill(bx+181, by+3, bx+181, by+3, LIME_GREEN); // Rect 10
    LCD_Fill(bx+183, by+3, bx+183, by+6, LIME_GREEN); // Rect 11
    LCD_Fill(bx+90, by+4, bx+90, by+5, LIME_GREEN); // Rect 12
    LCD_Fill(bx+184, by+4, bx+184, by+9, LIME_GREEN); // Rect 13
    LCD_Fill(bx+88, by+5, bx+88, by+9, LIME_GREEN); // Rect 14
    LCD_Fill(bx+87, by+6, bx+87, by+23, LIME_GREEN); // Rect 15
    LCD_Fill(bx+185, by+6, bx+185, by+22, LIME_GREEN); // Rect 16
    LCD_Fill(bx+186, by+8, bx+186, by+24, LIME_GREEN); // Rect 17
    LCD_Fill(bx+86, by+9, bx+86, by+24, LIME_GREEN); // Rect 18
    LCD_Fill(bx+88, by+11, bx+88, by+19, LIME_GREEN); // Rect 19
    LCD_Fill(bx+187, by+21, bx+187, by+25, LIME_GREEN); // Rect 20
    LCD_Fill(bx+85, by+23, bx+85, by+25, LIME_GREEN); // Rect 21
    LCD_Fill(bx+366, by+23, bx+369, by+32, LIME_GREEN); // Rect 22
    LCD_Fill(bx+2, by+24, bx+7, by+32, LIME_GREEN); // Rect 23
    LCD_Fill(bx+84, by+24, bx+84, by+26, LIME_GREEN); // Rect 24
    LCD_Fill(bx+188, by+24, bx+188, by+26, LIME_GREEN); // Rect 25
    LCD_Fill(bx+364, by+24, bx+365, by+31, LIME_GREEN); // Rect 26
    LCD_Fill(bx+370, by+24, bx+370, by+31, LIME_GREEN); // Rect 27
    LCD_Fill(bx+1, by+25, bx+1, by+31, LIME_GREEN); // Rect 28
    LCD_Fill(bx+8, by+25, bx+8, by+31, LIME_GREEN); // Rect 29
    LCD_Fill(bx+83, by+25, bx+83, by+27, LIME_GREEN); // Rect 30
    LCD_Fill(bx+189, by+25, bx+189, by+27, LIME_GREEN); // Rect 31
    LCD_Fill(bx+363, by+25, bx+363, by+30, LIME_GREEN); // Rect 32
    LCD_Fill(bx+371, by+25, bx+371, by+31, LIME_GREEN); // Rect 33
    LCD_Fill(bx+82, by+26, bx+82, by+27, LIME_GREEN); // Rect 34
    LCD_Fill(bx+190, by+26, bx+191, by+28, LIME_GREEN); // Rect 35
    LCD_Fill(bx+372, by+26, bx+372, by+29, LIME_GREEN); // Rect 36
    LCD_Fill(bx+0, by+27, bx+0, by+31, LIME_GREEN); // Rect 37
    LCD_Fill(bx+9, by+27, bx+9, by+30, LIME_GREEN); // Rect 38
    LCD_Fill(bx+79, by+27, bx+81, by+28, LIME_GREEN); // Rect 39
    LCD_Fill(bx+192, by+27, bx+193, by+28, LIME_GREEN); // Rect 40
    LCD_Fill(bx+10, by+28, bx+78, by+29, LIME_GREEN); // Rect 41
    LCD_Fill(bx+194, by+28, bx+274, by+29, LIME_GREEN); // Rect 42
    LCD_Fill(bx+79, by+29, bx+80, by+29, LIME_GREEN); // Rect 43
    LCD_Fill(bx+193, by+29, bx+193, by+29, LIME_GREEN); // Rect 44
    LCD_Fill(bx+275, by+29, bx+276, by+30, LIME_GREEN); // Rect 45
    LCD_Fill(bx+274, by+30, bx+274, by+30, LIME_GREEN); // Rect 46
    LCD_Fill(bx+277, by+30, bx+278, by+32, LIME_GREEN); // Rect 47
    LCD_Fill(bx+276, by+31, bx+276, by+31, LIME_GREEN); // Rect 48
    LCD_Fill(bx+279, by+31, bx+279, by+35, LIME_GREEN); // Rect 49
    LCD_Fill(bx+3, by+33, bx+6, by+33, LIME_GREEN); // Rect 50
    LCD_Fill(bx+278, by+33, bx+278, by+33, LIME_GREEN); // Rect 51
    LCD_Fill(bx+280, by+33, bx+280, by+40, LIME_GREEN); // Rect 52
    LCD_Fill(bx+367, by+33, bx+368, by+42, LIME_GREEN); // Rect 53
    LCD_Fill(bx+281, by+34, bx+281, by+44, LIME_GREEN); // Rect 54
    LCD_Fill(bx+282, by+39, bx+282, by+45, LIME_GREEN); // Rect 55
    LCD_Fill(bx+366, by+40, bx+366, by+45, LIME_GREEN); // Rect 56
    LCD_Fill(bx+283, by+43, bx+283, by+46, LIME_GREEN); // Rect 57
    LCD_Fill(bx+365, by+43, bx+365, by+46, LIME_GREEN); // Rect 58
    LCD_Fill(bx+367, by+43, bx+367, by+44, LIME_GREEN); // Rect 59
    LCD_Fill(bx+284, by+45, bx+284, by+47, LIME_GREEN); // Rect 60
    LCD_Fill(bx+364, by+45, bx+364, by+47, LIME_GREEN); // Rect 61
    LCD_Fill(bx+285, by+46, bx+285, by+48, LIME_GREEN); // Rect 62
    LCD_Fill(bx+363, by+46, bx+363, by+48, LIME_GREEN); // Rect 63
    LCD_Fill(bx+286, by+47, bx+288, by+48, LIME_GREEN); // Rect 64
    LCD_Fill(bx+361, by+47, bx+362, by+48, LIME_GREEN); // Rect 65
    LCD_Fill(bx+289, by+48, bx+360, by+49, LIME_GREEN); // Rect 66
    LCD_Fill(bx+287, by+49, bx+288, by+49, LIME_GREEN); // Rect 67
    LCD_Fill(bx+361, by+49, bx+361, by+49, LIME_GREEN); // Rect 68
}
void Draw_StaticText()
{
    POINT_COLOR=WHITE;

    LCD_ShowSongtiString2010(20,5,"TMAXCESW");
    LCD_ShowSongtiString2010(340,5,"Cnt:");
    LCD_ShowSongtiString2010(390,5,"0000001");

    LCD_ShowHeitiString2010(10, 41, "CH3");

    LCD_ShowHeitiString2010(70,46,"WM");
    LCD_ShowHeitiString2010(100,46,"AUTO");
    LCD_ShowHeitiString2010(170,46,"U/V");
    LCD_ShowHeitiString2010(210,46,"5.90");
    LCD_ShowHeitiString2010(280,46,"I/A");
    LCD_ShowHeitiString2010(320,46,"5000");

    LCD_ShowHeitiString2814(14,120,"1.2");
    LCD_ShowHeitiString2010(64,128,"MS");

    LCD_ShowHeitiString2814(106,120,"6.0");
    LCD_ShowHeitiString2010(156,128,"MS");

    LCD_ShowHeitiString2814(200,120,"1.0");
    LCD_ShowHeitiString2010(250,128,"MS");

    LCD_ShowHeitiString2814(296,120,"6.0");
    LCD_ShowHeitiString2010(346,128,"MS");

    LCD_ShowSongtiString2010(387, 72, "C");
    LCD_ShowSongtiChar1206(399,79,'1');

    LCD_ShowSongtiString2010(387, 132, "C");
    LCD_ShowSongtiChar1206(399,139,'2');

    LCD_ShowSongtiString2010(390, 192, "V");

    LCD_ShowSongtiString1407(420,38,"2.75");
    LCD_ShowSongtiString1407(456,38,"V");

    LCD_ShowSongtiString1407(420,54,"2.74");
    LCD_ShowSongtiString1407(456,54,"V");

    LCD_ShowSongtiString1407(420,70,"5.49");
    LCD_ShowSongtiString1407(456,70,"V");

    LCD_ShowSongtiString1407(420,98,"2.75");
    LCD_ShowSongtiString1407(456,98,"V");

    LCD_ShowSongtiString1407(420,114,"2.74");
    LCD_ShowSongtiString1407(456,114,"V");

    LCD_ShowSongtiString1407(420,130,"5.49");
    LCD_ShowSongtiString1407(456,130,"V");

    LCD_ShowHeitiString2814(400,164,"5.98");
    LCD_ShowHeitiString2814(460,164,"V");

    LCD_ShowHeitiString2010(35, 228, "SQ");
    LCD_ShowHeitiString2010(130, 228, "WE1");
    LCD_ShowHeitiString2010(220, 228, "COOL");
    LCD_ShowHeitiString2010(320, 228, "WE2");
    LCD_ShowHeitiString2010(415, 228, "SOV");

    LCD_ShowHeitiString2010(33, 283, "RCL");
    LCD_ShowHeitiString2010(130, 283, "MAN");
    LCD_ShowHeitiString2010(219, 283, "AUTO");
    LCD_ShowHeitiString2010(310, 283, "PARAM");
    LCD_ShowHeitiString2010(410, 283, "SAVE");
}