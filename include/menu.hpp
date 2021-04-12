#include <filesystem>
#include <vector>
#include <queue>
#include "builder.hpp"
#include "config.hpp"
//#include "menu.h"
#define ATTRIB_DIR 1
#define ATTRIB_HIDDEN 1<<1

enum MenuAction {
    OpenFolder,
    ReturnToMenu,
    Install,
    Install_All
};

class MenuSelection {
    public:
        std::filesystem::path path;
        std::string display;
        MenuAction action;
    MenuSelection(std::string s="",std::filesystem::path p=std::filesystem::path("/"));
    MenuSelection(MenuSelection* old);
    MenuSelection* setPath(std::filesystem::path p);
    MenuSelection* setDisplay(std::string s);

};
class Menu {
    private:
        std::vector<MenuSelection*>::iterator selection;
        std::vector<MenuSelection*>::iterator top;
        std::vector<MenuSelection*> entries;
        std::queue<MenuSelection> queue;
    public:
        std::filesystem::path currentDirectory;
        Menu(std::vector<MenuSelection*> entries);
        Menu();
        ~Menu();
        Menu* addEntry(MenuSelection* s);
        
//        Menu* setSelections(std::vector<MenuSelection> s);
//        std::vector<MenuSelection> getSelections();
        void drawMenu();
        void init();
        void down();
        void pageDown();
        void up();
        void pageUp();
        void action();
        Menu* back();
        Menu* handleQueue(Builder* builder, C3D_RenderTarget* target=nullptr, Config* config =nullptr);
        bool hasQueue();

};
Menu* generateMenu(std::filesystem::path path, Menu* prev);
bool sortMenuSelections(MenuSelection* a, MenuSelection* b);
std::string shorten(std::string s, u16 len);