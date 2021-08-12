#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>
extern "C" {
#include "nds.h"
#include "graphics.h"
}
#include "image.hpp"
#include "menu.hpp"
#include "builder.hpp"
#include "settings.hpp"
#include "config.hpp"
namespace fs = std::filesystem;

#define SYSTEM_APP_COUNT 6
static u64 systemApps[SYSTEM_APP_COUNT] = { 
					 0x00048005484E4441,
					 0x0004800542383841,
					 0x0004800F484E4841,
					 0x0004800F484E4C41,
					 0x00048005484E4443,
					 0x00048005484E444B
};
inline bool isSystemApp(u64 tid) {
	for (int i =0;i<SYSTEM_APP_COUNT;i++) {
		if (tid==systemApps[i]) {
			return true;
		}
	}
	return false;
}
void denit() {
	amExit();
	romfsExit();
	fsExit();
	psExit();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	
}
int failWait(std::string message) {
	consoleInit(GFX_BOTTOM, NULL);
	std::cout << message << '\n';
	std::cout << "\x1b[21;16HPress Start to exit.\n";
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START) break; 
	}
	denit();
	return -1;
}
u32 getDsiWareCount() {
	u32 title_count=0;
	Result res = AM_GetTitleCount(MEDIATYPE_NAND, &title_count);
	if (R_SUCCEEDED(res)) {
		u64 titleID[title_count]={0};
		u32 titles_read=0;
		res = AM_GetTitleList(&titles_read,MEDIATYPE_NAND,title_count,titleID);
		if (R_FAILED(res)) {
			title_count=1000;
		}else{
			title_count=0;
			for (u32 i=0;i<titles_read;i++) {
				u16 uCategory = (u16)((titleID[i] >> 32) & 0xFFFF);
				if (uCategory==0x8004 || uCategory==0x8005 || uCategory==0x800F || uCategory==0x8015 ) {
						if (!isSystemApp(titleID[i]))
							title_count+=1;
				}
			}
		}
		return title_count;
	}
	return 1000;
}
Result init() {
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	if (R_FAILED(fsInit())) {
		return failWait("Failed to read init fs\n");
	}
	if (R_FAILED(psInit())) {
		return failWait("Failed to read init ps\n");
	}
	if (R_FAILED(romfsInit())) {
		return failWait("Failed to read init romfs\n");
	}
	if (R_FAILED(amInit())) {
		return failWait("Failed to read init am\n");
	}
	if (!fileExists(FORWARDER_DIR)) {
		std::filesystem::create_directories(std::filesystem::path(FORWARDER_DIR));
	}

	return 0;
}

int main()
{
	if (R_FAILED(init()))
		return -1;
	
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	
	Menu* menu = generateMenu(std::filesystem::path("/"),nullptr);
	Config* config = new Config();
	config->dsiwareCount=getDsiWareCount();
	Builder b;
	b.initialize();

	while (aptMainLoop())
	{

		//Scan all the inputs. This should be done once for each frame
		hidScanInput();
		u32 kDown = hidKeysDown();

		// Touch Handling
		touchPosition touch;
		hidTouchRead(&touch);
		
		if (kDown & KEY_TOUCH) config->interact(&touch);

		// Button Handling

		if (kDown & KEY_START) break; // break in order to return to hbmenu

		else if (kDown & KEY_DOWN) menu->down();

		else if (kDown & KEY_UP) menu->up();
		 
		else if (kDown & KEY_RIGHT) menu->pageDown();

		else if (kDown & KEY_LEFT) menu->pageUp();

		else if(kDown & KEY_A) menu->action();


		else if(kDown & KEY_B) menu = menu->back();

		// Draw Screens

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, BGColor);
		C2D_TargetClear(bottom, C2D_Color32f(0,0,0,1));
		C2D_SceneBegin(top);	
		menu->drawMenu();
		C2D_SceneBegin(bottom);
		config->draw();
		C3D_FrameEnd(0);

		// Process queue
		menu = menu->handleQueue(&b,bottom,config);

	}
	denit();
	return 0;


}
