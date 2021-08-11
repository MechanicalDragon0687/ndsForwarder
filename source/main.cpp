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
#define FORWARDER_DIR "sdmc:/3ds/forwarder"

void denit() {
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
Result init() {
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	if (R_FAILED(fsInit())) {
		return failWait("Failed to read init romfs\n");
	}
	if (R_FAILED(psInit())) {
		return failWait("Failed to read init romfs\n");
	}
	if (R_FAILED(romfsInit())) {
		return failWait("Failed to read init romfs\n");
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
