#ifndef BIPOL_F401_CTRL_UI_H
#define BIPOL_F401_CTRL_UI_H

#include "LCD.h"

void Draw_RoundedRectangleBody(u16 bbx, u16 bby);
void Draw_RoundedRectangleEdge(u16 bbx, u16 bby);
void Draw_RoundedRectangleCorner(u16 bbx,u16 bby);
void Draw_RoundedRectangle(u16 bx, u16 by);
void Draw_WaveLine();
void Draw_StaticText();

#endif //BIPOL_F401_CTRL_UI_H
