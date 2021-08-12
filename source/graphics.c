#include <3ds.h>
#include <string.h>
#include <citro2d.h>
#include "graphics.h"
u8* getFrameBufferLocation(u8* buffer, u16 x, u16 y, u16 width, u16 height,u8 bps) {
    return buffer+(((x*height)+y)*bps);
}
void displayImageAt(u8* framebuffer, u16* icon, u16 x, u16 y, u16 width, u16 height) {
     for (int c=0;c<32;c++) {
	 	memcpy(getFrameBufferLocation(framebuffer,x+c,y,width,height,2),&icon[x*(32)],32*2);
	 }
}

void drawPanelWithTitle(float x, float y, float z, float width, float height, float border, u32 bgColor, u32 color, const char* label, u32 fontColor) {
    #define TITLE_HEIGHT 40
    if ((bgColor & 0xFF) > 0)
        C2D_DrawRectSolid(x,y,z,width,height,bgColor);
    C2D_DrawRectSolid(x,y,z,width,TITLE_HEIGHT,color);
    C2DExtra_DrawRectHollow(x,y,z,width,TITLE_HEIGHT,1,color);
    C2DExtra_DrawRectHollow(x,y+TITLE_HEIGHT-border,z,width,height-TITLE_HEIGHT+border,border,color);
    C2D_TextBuf buf = C2D_TextBufNew(4096);
    C2D_Text text;
    C2D_TextParse(&text,buf,label);
    float textheight=0;
    C2D_TextGetDimensions(&text,0.67,0.67,NULL,&textheight);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_WithColor ,x+width/10,y+(TITLE_HEIGHT/2)-textheight/2,z,0.67,0.67,fontColor);
    C2D_TextBufDelete(buf);
}
void drawPanel(float x, float y, float z, float width, float height, float border, u32 bgColor, u32 color) {
    C2D_DrawRectSolid(x,y,z,width,height,bgColor);
    C2DExtra_DrawRectHollow(x,y,z,width,height,border,color);
}

void C2DExtra_DrawRectHollow(float x, float y, float z, float width, float height, float thickness, u32 color) {
    //Left wall
    C2D_DrawRectSolid(x,y,z,thickness,height,color);
    //Right wall
    C2D_DrawRectSolid(x+width-thickness,y,z,thickness,height,color);
    //top wall
    C2D_DrawRectSolid(x+thickness,y,z,width-thickness*2,thickness,color);
    //bottom wall
    C2D_DrawRectSolid(x+thickness,y+height-thickness,z,width-thickness*2,thickness,color);
}
void drawFilePanel(const char* text, float x, float y, float z, float width, float height, u32 bgcolor, u32 textcolor, C2D_Image* image) {
    C2D_DrawRectSolid(x,y,0,width,height,bgcolor);
    if (image != NULL)
        C2D_DrawImageAt(*image,width/10*2,(y+height/2) - (image->tex->height/2),z,NULL,0.5f,0.5f);
}
void drawText(float x, float y, float z, float scale, u32 bgColor, u32 fontColor, const char* text, u32 flags) {
    C2D_TextBuf buf = C2D_TextBufNew(4096);
    C2D_Text ctext;
    C2D_TextParse(&ctext,buf,text);
    float textheight=0;
    float textwidth=0;
    C2D_TextGetDimensions(&ctext,scale,scale,&textwidth,&textheight);
    C2D_TextOptimize(&ctext);
    float left = x;
    float top = y-(textheight/2);
    if ((flags & C2D_AlignCenter) > 0) {
        left = x-(textwidth/2);
    }else if((flags & C2D_AlignRight) > 0) {
        left = x-textwidth;
    }
    if ((flags & C2D_AtBaseline) > 0) {
        top = y-textheight;
    }

    if ((bgColor & 0xFF) > 0) {
        C2D_DrawRectSolid(left-3,top-3,0,textwidth+6,textheight+6,bgColor);
    }
    C2D_DrawText(&ctext, C2D_WithColor | flags,x,((flags & C2D_AtBaseline) > 0)?y:y-(textheight/2),z,scale,scale,fontColor);
    C2D_TextBufDelete(buf);
}
void drawCheckbox(float x, float y, float z, float width, float height, float scale, u32 bgColor, u32 borderColor, u32 color, const char* label, bool checked) {
    C2DExtra_DrawRectHollow(x,y,z,width,height,2,borderColor);
    if (checked)
        C2D_DrawRectSolid(x+3,y+3,z,width-6,height-6,borderColor);
    if (strlen(label) > 0)
        drawText(x+width+3,y+(height/2),z,scale,bgColor,color,label,0);
}
void drawArrow(float x, float y, float z, float height, float width, u32 color, bool flip) {
    if (flip) {
        C2D_DrawTriangle( x+width, y+height/2, color, x,y,color,x,y+height,color,0);
    }else{
        C2D_DrawTriangle( x, y+height/2, color, x+width,y,color,x+width,y+height,color,0);
    }
}