#include <3ds.h>
#include <citro2d.h>
#include "config.hpp"
#include "graphics.h"
#include "settings.hpp"

Config::Config() {
    this->customTitle=false;
    this->randomTID=false;
}
void Config::draw(bool interactive) {
    drawPanelWithTitle(0,0,0,320,240,MENU_BORDER_HEIGHT,BGColor,BORDER_COLOR,"Settings",BORDER_FOREGROUND);
    drawCheckbox(MENU_BORDER_HEIGHT+10,MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+20,0,20,20,0.67,BGColor,BORDER_COLOR,FOREGROUND_COLOR,"Random TID",this->randomTID);
    drawCheckbox(MENU_BORDER_HEIGHT+10,MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+60,0,20,20,0.67,BGColor,BORDER_COLOR,FOREGROUND_COLOR,"Custom Title",this->customTitle);

}
void Config::interact(touchPosition *touch) {
    if (touch->px > MENU_BORDER_HEIGHT+10 && touch->px < MENU_BORDER_HEIGHT+30) {
        if (touch->py >= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+20 && touch->py <= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+50) {
            this->randomTID=!this->randomTID;
        }
        if (touch->py >= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+60 && touch->py <= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+80) {
            this->customTitle=!this->customTitle;
        }
    }

}

