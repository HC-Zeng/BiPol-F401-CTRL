#ifndef BIPOL_F401_CTRL_UI_H
#define BIPOL_F401_CTRL_UI_H

#include "LCD.h"

void Draw_RoundedRectangleBody(u16 bbx, u16 bby);
void Draw_RoundedRectangleEdge(u16 bbx, u16 bby);
void Draw_RoundedRectangleCorner(u16 bbx,u16 bby);
void Draw_RoundedRectangle(u16 bx, u16 by);
void Draw_WaveLine();
void Draw_StaticText();
void ShowCnt(uint32_t cnt);
void ShowC1C2(float vc1, float vc2);
void ShowC3C4(float vc3, float vc4);
void ShowSetVoltage(float v);
void ShowSQDuration(float dur);
void ShowWE1Duration(float dur);
void ShowCOOLDuration(float dur);
void ShowWE2Duration(float dur);
void ShowWeldingCurrent(uint16_t curr);
void initUI();

#endif //BIPOL_F401_CTRL_UI_H
