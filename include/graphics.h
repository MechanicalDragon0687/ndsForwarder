#pragma once
#include <3ds.h>
#include <citro2d.h>
#ifdef __cplusplus
extern "C" {
#endif
#define HexColor(hex) C2D_Color32((hex>>16) & 0xFF,(hex>>8)&0xFF,(hex&0xFF),0xFF)

// #define BGColor HexColor(0x17b3c1)
// #define BORDER_COLOR HexColor(0x2794eb)
// #define BORDER_FOREGROUND HexColor(0xbff8d4)
// #define HIGHLIGHT_BGCOLOR HexColor(0x47d6b6)
// #define HIGHLIGHT_FOREGROUND HexColor(0)
// #define FOREGROUND_COLOR HexColor(0)

u8* getFrameBufferLocation(u8* buffer, u16 x, u16 y, u16 width, u16 height,u8 bps);
void displayImageAt(u8* framebuffer, u16* icon, u16 x, u16 y, u16 width, u16 height);
void drawPanelWithTitle(float x, float y, float z, float width, float height, float border, u32 bgColor, u32 color, const char* label, u32 fontColor);
void drawPanel(float x, float y, float z, float width, float height, float border, u32 bgColor, u32 color);
void drawText(float x, float y, float z, float scale, u32 bgColor, u32 fontColor, const char* text, u32 flags);
void drawCheckbox(float x, float y, float z, float width, float height, float scale, u32 bgColor, u32 borderColor, u32 color, const char* label, bool checked);
void C2DExtra_DrawRectHollow(float x, float y, float z, float width, float height, float thickness, u32 color);
void drawArrow(float x, float y, float z, float height, float width, u32 color, bool flip);
#ifdef __cplusplus
}
#endif