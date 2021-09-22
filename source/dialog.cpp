#include <3ds.h>
#include <citro2d.h>
#include <vector>
#include <string>
#include "dialog.hpp"
#include "graphics.h"
#include "settings.hpp"
void Dialog::draw() {
    C2D_TextBuf buf = C2D_TextBufNew(4096);
    C2D_Text ctext;
    C2D_TextParse(&ctext,buf,"0");
    float textheight=0;
    C2D_TextGetDimensions(&ctext,0.67,0.67,NULL,&textheight);
    C2D_TextBufDelete(buf);

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_SceneBegin(this->target);	
    drawPanel(this->x,this->y,0,this->width, this->height,MENU_BORDER_HEIGHT,BGColor,BORDER_COLOR);
    float drawx = this->x+MENU_BORDER_HEIGHT;
    float drawxOffset = (this->width/this->options.size());
    float drawyOffset = (this->height*2/3);
    float drawy = this->y+MENU_BORDER_HEIGHT+drawyOffset+(this->height/6);
    for (size_t i=0;i<this->message.size();i++)
        drawText(drawx+(this->width/2),this->y+(this->height*2/12)+((1+textheight)*i),0,0.67,0,FOREGROUND_COLOR,this->message[i].c_str(),C2D_AlignCenter);
    for (size_t i=0;i<this->options.size();i++) {
        drawText(drawx+(drawxOffset*(i+0.5)),drawy,0,0.67,(this->selected==i)?HIGHLIGHT_BGCOLOR:BGColor,(this->selected==i)?HIGHLIGHT_FOREGROUND:FOREGROUND_COLOR, this->options[i].c_str(),C2D_AlignCenter);
    }
    C3D_FrameEnd(0);
}
int Dialog::handle() {
    touchPosition pos;
    touchPosition oldPos;
    if (this->options.size() < 1) {
        this->draw();
        return -1;
    }
    while (aptMainLoop())
    {
        hidScanInput();
        oldPos = pos;
		u32 kDown = hidKeysDown();
        hidTouchRead(&pos);
        
        if (pos.px != oldPos.px || pos.py != oldPos.py) {
            // check tap position
        }
        if (kDown & KEY_LEFT) {
            this->selected--;
            if (this->selected < 0) this->selected=this->options.size()-1;
        }else if(kDown & KEY_RIGHT) {
            this->selected++;
            if (this->selected >= this->options.size()) this->selected=0;
        }else if(kDown & KEY_A || kDown & KEY_START) {
            return this->selected;
        }
        this->draw();
    }
    return -1;
}
Dialog::Dialog(C3D_RenderTarget* target, float x, float y, float width, float height, std::string message, std::initializer_list<std::string> options, int defaultChoice) {
    this->options = std::vector(options.begin(),options.end());
    this->message.push_back(message);
    this->target=target;
    this->selected=defaultChoice;
    this->x=x;
    this->y=y;
    this->width=width;
    this->height=height;
}
Dialog::Dialog(C3D_RenderTarget* target, float x, float y, float width, float height, std::initializer_list<std::string> message, std::initializer_list<std::string> options, int defaultChoice) {
    this->options = std::vector(options.begin(),options.end());
    this->message= std::vector(message.begin(),message.end());
    this->target=target;
    this->selected=defaultChoice;
    this->x=x;
    this->y=y;
    this->width=width;
    this->height=height;
}

