#pragma once
#include <3ds.h>
#include <string>
#include <vector>
#include <3ds.h>
#include <citro2d.h>

class Config {
    public:
    bool randomTID;
    bool customTitle;
    std::vector<std::string> templates;
    u8 currentTemplate;
    unsigned long dsiwareCount;
    void draw(bool interactive=false);
    void interact(touchPosition *touch);
    Config();
};