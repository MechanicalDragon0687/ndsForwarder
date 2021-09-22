#pragma once

#define VERSION "v1.2.0-beta"
//#define DEBUG 1

#define FORWARDER_DIR std::string("sdmc:/3ds/forwarder")
#define SDCARD_TEMPLATE_DIR FORWARDER_DIR+std::string("/templates/")
#define ROMFS_TEMPLATE_DIR std::string("romfs:/templates/")
#define ROMFS_LANG_DIR std::string("romfs:/lang/")
#define SDCARD_BANNER_PATH FORWARDER_DIR+std::string("/banners/")
#define SDCARD_ICON_PATH FORWARDER_DIR+std::string("/icons/")
#define SDCARD_LANG_DIR FORWARDER_DIR+std::string("/lang/")
#define ENTRY_HEIGHT 48
#define FILELIST_HEIGHT (240-MENU_HEADING_HEIGHT-MENU_BORDER_HEIGHT)
#define MENU_BORDER_HEIGHT 8
#define MENU_HEADING_HEIGHT 40
#define MAX_ENTRY_COUNT ((FILELIST_HEIGHT-(FILELIST_HEIGHT%ENTRY_HEIGHT))/ENTRY_HEIGHT)

#define BGColor HexColor(0x293B5F)
#define BORDER_COLOR HexColor(0x77ACF1)
#define BORDER_FOREGROUND HexColor(0x293B5F)
#define HIGHLIGHT_BGCOLOR HexColor(0xDBE6FD)
#define HIGHLIGHT_FOREGROUND HexColor(0x293B5F)
#define FOREGROUND_COLOR HexColor(0xDBE6FD)
