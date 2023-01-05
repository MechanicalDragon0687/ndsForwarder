#include <3ds.h>
#include <citro2d.h>
#include "config.hpp"
#include "graphics.h"
#include "settings.hpp"
#include <vector>
#include <filesystem>
#include <algorithm>
#include "logger.hpp"
#include "lang.hpp"
Logger configLogger("Config");

Config::Config() {
    this->customTitle=false;
    this->randomTID=false;
    this->forceInstall=false;
    this->dsiwareCount=0;
    this->templates = std::vector<std::string>();
    this->templates.push_back("sdcard");
    for (const auto & entry : std::filesystem::directory_iterator(ROMFS_TEMPLATE_DIR)) {
        std::string filename = entry.path().filename();
        if (entry.path().extension().generic_string() != ".fwd") continue;
        configLogger.info(gLang.parseString("config_foundTemplate",filename.c_str(),"romfs"));
        this->templates.push_back(filename.substr(0,filename.find_last_of('.')));
    }

    configLogger.info(gLang.parseString("config_searchTemplates","SD"));
    std::filesystem::create_directories(SDCARD_TEMPLATE_DIR);
    for (const auto & entry : std::filesystem::directory_iterator(SDCARD_TEMPLATE_DIR)) {
        std::string filename = entry.path().filename();
        configLogger.info(filename);
        if (entry.path().extension().generic_string() != ".fwd") continue;
        configLogger.info(gLang.parseString("config_foundTemplate",filename.c_str(),"sd"));
        //configLogger.info("Found "+filename+" on sdmc");
        this->templates.push_back(filename.substr(0,filename.find_last_of('.')));
    }
    //deduplicate
    configLogger.info(gLang.getString("config_deduplicating"));

    std::sort( this->templates.begin(), this->templates.end() );
    this->templates.erase( std::unique( this->templates.begin(), this->templates.end() ), this->templates.end() );
    if (this->templates.size() <= 0)
        configLogger.warn(gLang.getString("config_noTemplates"));
    this->currentTemplate=0;
}
void Config::draw(bool interactive) {
    drawPanelWithTitle(0,0,0,320,240,MENU_BORDER_HEIGHT,BGColor,BORDER_COLOR,VERSION,BORDER_FOREGROUND);
    drawCheckbox(MENU_BORDER_HEIGHT+10,MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+20,0,20,20,0.67,BGColor,BORDER_COLOR,FOREGROUND_COLOR,gLang.parseString("config_randomTID").c_str(),this->randomTID);
    drawCheckbox(MENU_BORDER_HEIGHT+10,MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+60,0,20,20,0.67,BGColor,BORDER_COLOR,FOREGROUND_COLOR,gLang.parseString("config_customTitle").c_str(),this->customTitle);
    drawCheckbox(MENU_BORDER_HEIGHT+10,MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+100,0,20,20,0.67,BGColor,BORDER_COLOR,FOREGROUND_COLOR,gLang.parseString("config_forceInstall").c_str(),this->forceInstall);
    if (this->templates.size() > 1) {
        drawText(MENU_BORDER_HEIGHT+10,MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+140,0,0.67,BGColor,FOREGROUND_COLOR,"Template:",0);
        drawArrow(MENU_BORDER_HEIGHT+10,MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+160,0,10,10,FOREGROUND_COLOR,false);
        drawArrow(MENU_BORDER_HEIGHT+25,MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+160,0,10,10,FOREGROUND_COLOR,true);
        drawText(MENU_BORDER_HEIGHT+50,MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+164,0,0.67,BORDER_COLOR,BORDER_FOREGROUND,this->templates.at(this->currentTemplate).c_str(),0);
    }
}
void Config::interact(touchPosition *touch) {
    if (touch->px > MENU_BORDER_HEIGHT+10 && touch->px < MENU_BORDER_HEIGHT+30) {
        if (touch->py >= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+20 && touch->py <= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+50) {
            this->randomTID=!this->randomTID;
        }
        if (touch->py >= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+60 && touch->py <= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+80) {
            this->customTitle=!this->customTitle;
        }
        if (touch->py >= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+100 && touch->py <= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+120) {
            this->forceInstall=!this->forceInstall;
        }
    }
    if (touch->px >= MENU_BORDER_HEIGHT+10 && touch->px <= MENU_BORDER_HEIGHT+20 &&
        touch->py >= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+160 && touch->py <= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+170) {
            if (this->currentTemplate > 0)
                this->currentTemplate--;
            else if(this->templates.size() > 0)
                this->currentTemplate=this->templates.size()-1;
    }
    if (touch->px >= MENU_BORDER_HEIGHT+25 && touch->px <= MENU_BORDER_HEIGHT+35 &&
        touch->py >= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+160 && touch->py <= MENU_BORDER_HEIGHT+MENU_HEADING_HEIGHT+170) {
            if (this->templates.size() > this->currentTemplate+1)
                this->currentTemplate++;
            else
                this->currentTemplate=0;
    }
        

}

void Config::interactKey(u32* key) {
    if (*key & KEY_X) {
        this->randomTID = !this->randomTID;
    }
    if (*key & KEY_Y) {
        this->customTitle=!this->customTitle;
    }
    if (*key & KEY_A) {
        this->forceInstall=!this->forceInstall;
    }
    if (*key & KEY_LEFT) {
        this->currentTemplate--;
        if (this->currentTemplate < 0) {
            this->currentTemplate = this->templates.size()-1;
        }
    }
    if (*key & KEY_RIGHT) {
        this->currentTemplate++;
        if (this->currentTemplate >= this->templates.size()) {
            this->currentTemplate = 0;
        }
    }
}