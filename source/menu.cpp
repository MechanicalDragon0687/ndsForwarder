#include <3ds.h>
#include <citro2d.h>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "menu.hpp"
extern "C" {
#include "graphics.h"
#include "nds.h"
}
#include "builder.hpp"
#include "dialog.hpp"
#include "settings.hpp"
#include "config.hpp"
#include "helpers.hpp"
#include "lang.hpp"

#define MAX_DSIWARE 40
//class MenuSelection {

    MenuSelection::MenuSelection(std::string s,std::filesystem::path p) {
        this->display=s;
        this->path=p;
    }
    MenuSelection::MenuSelection(MenuSelection* old) {
        this->display=old->display;
        this->path=old->path;
        this->action=old->action;
    }
    MenuSelection* MenuSelection::setPath(std::filesystem::path p) {
        this->path=p;
        return this;
    }
    MenuSelection* MenuSelection::setDisplay(std::string s) {
        this->display=s;
        return this;
    }


    Menu::~Menu() {
        for ( auto item : this->entries ) delete item;
        this->entries.clear();
    }
    Menu::Menu() {

    }
    Menu::Menu(std::vector<MenuSelection*> entries) {
        this->entries=entries;
    }
    Menu* Menu::addEntry(MenuSelection* s) {
        this->entries.push_back(s);
        return this;
    }
    void Menu::drawMenu() {
			//draw
            std::string title =  shorten(this->currentDirectory.generic_string(),30);
			drawPanelWithTitle(0,0,0.50, 400,240,MENU_BORDER_HEIGHT,0, BORDER_COLOR,title.c_str(),BORDER_FOREGROUND);
            u16 offset = 0;
            u8 counter=0;
            for (std::vector<MenuSelection*>::iterator entry=this->top;entry!=this->entries.end() && counter < MAX_ENTRY_COUNT;entry++) {
                C2D_DrawRectSolid(MENU_BORDER_HEIGHT,MENU_HEADING_HEIGHT+offset,0,400-MENU_BORDER_HEIGHT,ENTRY_HEIGHT,(entry==this->selection)?HIGHLIGHT_BGCOLOR:BGColor);
                C2DExtra_DrawRectHollow(0,MENU_HEADING_HEIGHT+offset,0,400,ENTRY_HEIGHT,1,BORDER_COLOR);
                C2D_TextBuf buf = C2D_TextBufNew(4096);
                C2D_Text text;
                C2D_TextParse(&text,buf,&(*entry)->display.c_str()[0]);
                float textheight=0;
                C2D_TextGetDimensions(&text,0.67,0.67,NULL,&textheight);
                C2D_TextOptimize(&text);
                C2D_DrawText(&text, C2D_WithColor,40,MENU_HEADING_HEIGHT+offset+(ENTRY_HEIGHT/2)-(textheight/2),0,0.67,0.67,(entry==this->selection)?HIGHLIGHT_FOREGROUND:FOREGROUND_COLOR);
                C2D_TextBufDelete(buf);
                offset+=ENTRY_HEIGHT;
                counter++;
            }

    }
    bool validExtension(const char* extension) {
        char extensions[][5] = {".nds", ".srl", ".ids"};
        for (int i=0;i<3;i++) {
            if (strcasecmp(extension,extensions[i])==0) return true;
        }
        return false;
    }
    Menu* generateMenu(std::filesystem::path path, Menu* prev) {
        delete prev;
        std::vector<MenuSelection*> entries;
        
        bool ndsFilesVisible=false;
        for (const auto & entry : std::filesystem::directory_iterator(path)) {
            std::string filename = entry.path().filename();
            if (
                filename[0]=='.' || 
                !(entry.is_directory() || validExtension(entry.path().extension().c_str())) ||
                (filename=="_nds" && path.generic_string()=="/")
            )
                continue;
            MenuSelection* menuEntry = new MenuSelection();
            menuEntry->path=entry.path();
            menuEntry->display=filename;
            if (entry.is_directory())  {
                menuEntry->action=OpenFolder;
                
            }else{
                menuEntry->action=Install;
                ndsFilesVisible=true;
                
            }
            entries.push_back(menuEntry);
        }
        std::sort(entries.begin(), entries.end(), sortMenuSelections);
        
        if (path.has_parent_path() && path.parent_path().compare(path)) {
            MenuSelection* prevFolder = new MenuSelection();
            prevFolder->display="..";
            prevFolder->action=OpenFolder;
            prevFolder->path=path.parent_path();
            entries.insert(entries.begin(),prevFolder);
        }
        if (ndsFilesVisible) {
            MenuSelection* installAll = new MenuSelection();
            installAll->action=Install_All;
            installAll->display=gLang.getString("menu_installAll");
            installAll->path=path;
            entries.insert(entries.begin(),installAll);
        }
        Menu* menu = new Menu(entries);
        menu->currentDirectory=path.generic_string();
        menu->init();
        return menu;
    }
    void Menu::init() {
        this->top=entries.begin();
        this->selection=entries.begin();
    }
    void Menu::down() {
        if (this->selection+1 == this->entries.end()) {
            this->selection=this->entries.begin();
            this->top=this->entries.begin();
        }else{
            if (this->selection+1==this->top+MAX_ENTRY_COUNT) {
                top++;
            }
            this->selection++;
        }
    }
    void Menu::up() {
        if (this->selection == this->entries.begin()) {
            this->selection=this->entries.end()-1;
            if (this->entries.size() > MAX_ENTRY_COUNT)
                this->top=this->entries.end()-MAX_ENTRY_COUNT;
        }else{
            if (this->selection==this->top) {
                this->top--;
            }
            this->selection--;
        }
    }
    void Menu::action() {
        this->queue.push(MenuSelection(*this->selection));
    }
    void Menu::pageDown() { 
        if (this->top+MAX_ENTRY_COUNT == this->entries.end()) {

            this->selection=this->entries.end()-1;

        } else {
            
            for (int i=0;i < MAX_ENTRY_COUNT && this->top+MAX_ENTRY_COUNT != this->entries.end(); i++) {
                this->top++;
                this->selection++;
            }
        }
     }
    void Menu::pageUp() {
        if (this->top==this->entries.begin()) {
            this->selection=this->entries.begin();
        }else{
            for (int i=0;i<MAX_ENTRY_COUNT && this->top!=this->entries.begin();i++) {
                this->top--;
                this->selection--;
            }
        }
    }
    Menu* Menu::back() {
        return generateMenu(this->currentDirectory.parent_path(),this);
    }
    bool Menu::hasQueue() {
        return this->queue.size() > 0;
    }
    Menu* Menu::handleQueue(Builder* builder, C3D_RenderTarget* target, Config* config) {
        if (!this->hasQueue())
            return this;
        if (target==nullptr)
            target = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
        while (queue.size() > 0) {
            MenuSelection entry = this->queue.front();
            switch (entry.action) {
                case Install:
                    if (config->dsiwareCount >= MAX_DSIWARE) {
                        Dialog(target,0,0,320,240,{gLang.getString("menu_tooManyDSiWare"),std::to_string(config->dsiwareCount)},{gLang.getString("menu_ok")}).handle();
                        break;
                    }
                    if (!(builder->loadTemplate(config->templates.at(config->currentTemplate)))->isSuccess()) {
                        Dialog(target,0,0,320,240,{gLang.getString("menu_installFailed"),gLang.getString("menu_noTemplate")},{gLang.getString("menu_ok")}).handle();
                        break;
                    }
                    if (Dialog(target,0,0,320,240,{gLang.getString("menu_installTitleQ"),entry.path.filename().generic_string()},{gLang.getString("menu_yes"),gLang.getString("menu_no")}).handle()==0) {
                        ReturnResult* buildResult=nullptr;
                        if (config!=nullptr) {
                            std::string customTitle="";
                            if (config->customTitle) {
                                char customTitleBuffer[0x51] = {0};
                                SwkbdState kbstate;
                                swkbdInit(&kbstate,SWKBD_TYPE_NORMAL,2,0x50);
                                swkbdSetHintText(&kbstate,gLang.getString("menu_customTitleQ").c_str());
                                swkbdSetFeatures(&kbstate,SWKBD_MULTILINE | SWKBD_DEFAULT_QWERTY);
                                swkbdInputText(&kbstate,customTitleBuffer,0x51);
                                customTitle=std::string(customTitleBuffer);
                            }
                            buildResult = builder->loadTemplate(config->templates.at(config->currentTemplate));
                            if (buildResult->isSuccess()) {
                                delete buildResult;
                                buildResult = builder->buildCIA(entry.path.generic_string(),config->randomTID,customTitle,config->forceInstall);
                            }
                        } else {
                            buildResult = builder->buildCIA(entry.path.generic_string());
                        }
                        
                        if (buildResult->isSuccess()) {
                            Dialog(target,0,0,320,240,gLang.getString("menu_installComplete"),{gLang.getString("menu_ok")}).handle();
                        }else{
                            Dialog(target,0,0,320,240,{gLang.getString("menu_installFailed"),gLang.getString("dialog_checkLog"),gLang.parseString("format_hex",(u32)buildResult->code)},{gLang.getString("menu_ok")}).handle();
                        }
                        delete buildResult;
                    }
                    break;
                case Install_All:
                    if (Dialog(target,0,0,320,240,{gLang.getString("menu_installTitleQ"),gLang.getString("menu_allForwarders"),(!entry.path.filename().generic_string().empty())?entry.path.filename().generic_string():"/"},{gLang.getString("menu_yes"),gLang.getString("menu_no")}).handle()==0) {
                        if (!(builder->loadTemplate(config->templates.at(config->currentTemplate)))->isSuccess()) {
                            Dialog(target,0,0,320,240,{gLang.getString("menu_installFailed"),gLang.getString("menu_noTemplate")},{gLang.getString("menu_ok")}).handle();
                            break;
                        }
                        for (const auto & dEntry : std::filesystem::directory_iterator(entry.path)) {
                            if (config->dsiwareCount >= MAX_DSIWARE) {
                                Dialog(target,0,0,320,240,{gLang.getString("menu_tooManyDSiWare"),std::to_string(config->dsiwareCount)},{gLang.getString("menu_ok")}).handle();
                                break;
                            }
                            std::string filename = dEntry.path().filename();
                            if (filename[0]=='.' || !validExtension(dEntry.path().extension().c_str()))
                                continue;
                            std::string shortname = shorten(dEntry.path().filename().generic_string(),25);
                            Dialog(target,0,0,320,240,{gLang.getString("menu_installing"),shortname},{},0).handle();
                            ReturnResult* buildResult=nullptr;
                            if (config!=nullptr) {
                                std::string customTitle="";
                                if (config->customTitle) {
                                    char customTitleBuffer[0x51] = {0};
                                    SwkbdState kbstate;
                                    swkbdInit(&kbstate,SWKBD_TYPE_NORMAL,2,0x50);
                                    swkbdSetHintText(&kbstate,shortname.c_str());
                                    swkbdSetFeatures(&kbstate,SWKBD_MULTILINE | SWKBD_DEFAULT_QWERTY);
                                    swkbdInputText(&kbstate,customTitleBuffer,0x51);
                                    customTitle=std::string(customTitleBuffer);
                                }
                                buildResult = builder->buildCIA(dEntry.path().generic_string(),config->randomTID,customTitle, config->forceInstall);
                            } else {
                                buildResult = builder->buildCIA(dEntry.path().generic_string());
                            }
                            if (!buildResult->isSuccess()) {
                                Dialog(target,0,0,320,240,{gLang.getString("menu_installFailed"),shortname,std::to_string((u32)buildResult->code)},{gLang.getString("menu_ok")}).handle();
                            }else{
                                config->dsiwareCount++;
                            }
                            delete buildResult;
                        }
                        Dialog(target,0,0,320,240,gLang.getString("menu_installComplete"),{gLang.getString("menu_ok")}).handle();
                    }
                    break;
                case OpenFolder:
                    while (this->queue.size() > 0) this->queue.pop();
                    return generateMenu(entry.path,this);
                default:
                    break; 
            }
            this->queue.pop();
        }
        return this;
    }
    bool sortMenuSelections(MenuSelection* a, MenuSelection* b) {
        std::string aDisplay = toLowerCase(a->display);
        std::string bDisplay = toLowerCase(b->display);
        if ((a->action==OpenFolder && b->action==OpenFolder) || (!a->action==OpenFolder && !b->action==OpenFolder))
            return aDisplay<bDisplay;
        return a->action==OpenFolder;
    }
std::string shorten(std::string s, u16 len) {
    if (len > 8 && s.length() > len) {
        return s.substr(0,5)+"..."+s.substr(s.length()-(len-8));
    }
    return s;
}
