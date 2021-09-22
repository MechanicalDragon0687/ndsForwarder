#include <3ds.h>
#include <string>
#include <fstream>
#include <cstdarg>
#include <json.hpp>
#include "lang.hpp"
#include "settings.hpp"
#include "helpers.hpp"
#include "logger.hpp"
Lang gLang=Lang();
Logger jsonLogger("JSON");
static char lang[12][3]=  {
    "jp","en","fr","de","it","es","zh","ko","nl","pt","ru","tw"
};

void Lang::loadStrings(u8 bLang) {
    if (bLang>11) {
        bLang=1;
    }
    FILE *f;
    std::string filename=ROMFS_LANG_DIR+"default.json";
    if (!fileExists(SDCARD_LANG_DIR+std::string(lang[bLang])+".json")) {
        if (fileExists(ROMFS_LANG_DIR+std::string(lang[bLang])+".json")) {
            filename = ROMFS_LANG_DIR+std::string(lang[bLang])+".json";
        }
    }else{
        filename = SDCARD_LANG_DIR+std::string(lang[bLang])+".json";
    }


    f=fopen(filename.c_str(),"r");
    if(f) {
        this->_jLang=nlohmann::json::parse(f,nullptr,false);
        fclose(f);
    }
    jsonLogger.debug(this->_jLang.dump());
    this->_ready=true;
    //jsonLogger.info(getString("config_foundTemplate"));
    //jsonLogger.info(this->_jLang["config_foundTemplate"]);
}
std::string Lang::parseString(std::string key,...) {
    /// I hate this
    va_list arglist;
    std::string parseThis = this->getString(key);
    char pt[255];
    va_start(arglist, key);
    vsnprintf(pt,255,parseThis.c_str(),arglist);
    va_end(arglist);
    return std::string(pt);
}
std::string Lang::getString(std::string key) {
    if (!this->isReady()) 
        return "unknown";
    if (this->_jLang.contains(key)) {
        return this->_jLang[key];
    }else{
        return key;
    }
}
bool Lang::isReady() {
    return this->_ready;
}