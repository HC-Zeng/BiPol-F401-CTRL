#ifndef BIPOL_F401_CTRL_UI_H
#define BIPOL_F401_CTRL_UI_H

#include "LCD.h"

void Draw_RoundedRectangleBody(u16 bbx, u16 bby);
void Draw_RoundedRectangleEdge(u16 bbx, u16 bby, u16 color);
void Draw_RoundedRectangleCorner(u16 bbx,u16 bby, u16 color);
void Draw_RoundedRectangle(u16 bx, u16 by);
void Draw_WaveLine();
void Draw_StaticText();
void ShowCnt(uint32_t cnt);
void ShowC1C2(float vc1, float vc2);
void ShowC3C4(float vc3, float vc4);
void ShowSetVoltage(float v,uint8_t editing);
void ShowSQDuration(float dur,uint8_t editing);
void ShowWE1Duration(float dur,uint8_t editing);
void ShowCOOLDuration(float dur,uint8_t editing);
void ShowWE2Duration(float dur,uint8_t editing);
void ShowWeldingCurrent(uint16_t curr);
void initUI();
void ShowSelectedButton(uint8_t idx);
void ShowWeldingMode(uint8_t mod,uint8_t editing);

#endif //BIPOL_F401_CTRL_UI_H
