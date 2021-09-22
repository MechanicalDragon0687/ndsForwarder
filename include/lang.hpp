#pragma once
#include <3ds.h>
#include <string>
#include "json.hpp"

class Lang {
    private:
        nlohmann::json _jLang;
        bool _ready;
    public:
        bool isReady();
        void loadStrings(u8 lang);
        std::string getString(std::string key);
        std::string parseString(std::string key,...);
};

extern Lang gLang;
