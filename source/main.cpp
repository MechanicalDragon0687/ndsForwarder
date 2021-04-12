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


void denit() {
	romfsExit();
	fsExit();
	psExit();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	
}
void failWait(std::string message) {
		std::cout << message << '\n';
		std::cout << "\x1b[21;16HPress Start to exit.\n";
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START) break; // break in order to return to hbmenu
	}
	denit();
}
Result init() {
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	//consoleInit(GFX_BOTTOM, NULL);
	if (R_FAILED(fsInit())) {
		failWait("Failed to read init romfs\n");
	}
	if (R_FAILED(psInit())) {
		failWait("Failed to read init romfs\n");
	}
	if (R_FAILED(romfsInit())) {
		failWait("Failed to read init romfs\n");
	}
	return 0;
}

int main()
{
	if (R_FAILED(init()))
		return -1;
	
	if (!fileExists("sdmc:/3ds/forwarder")) {
		std::filesystem::create_directories(std::filesystem::path("sdmc:/3ds/forwarder"));
	}
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	Menu* menu = generateMenu(std::filesystem::path("/"),nullptr);
	Config* config = new Config();
	int perPage = 2;


    //FrameBuffer* screen = new FrameBuffer("top", false, GSP_RGB5_A1_OES);
//	printf("%dx%d\n",width,height);
	std::string filename = "romfs:/Mario Kart DS.nds";
	
	u16 bmp[0x400]={};
	// LoadIconFromNDS(filename.c_str(),bmp);
    // screen->displayImageAt(bmp,0,0,32,32);
//	memdump(stdout,"banner: ",&banner,sizeof(banner));
//	memdump(stdout,"bmp: ",&bmp,8*3);
	//Copy our image in the bottom screen's frame buffer
	//memcpy(fb, brew_bgr, brew_bgr_size);
	// for (int x=0;x<32;x++) {
	// 	memcpy(getFrameBufferLocation(fb,10+x,100,width,height,bytesPerPixel),&bmp[x*(32)],32*bytesPerPixel);//32*3);
	// }
	// Main loop
	//printf("\x1b[21;16HPress Start to exit.");
	//drawList(topFile,selected,menu);
	Builder b;
	b.initialize();
	//b.buildCIA("romfs:/roms/mkds.nds");
	while (aptMainLoop())
	{

		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();
		touchPosition touch;
		hidTouchRead(&touch);
		if (kDown & KEY_TOUCH)
			config->interact(&touch);
		if (kDown & KEY_START)  {
			break; // break in order to return to hbmenu

		}else if (kDown & KEY_DOWN) {
			menu->down();
		// 	selected++;
		// 	if (selected > perPage-1) {
		// 		selected=perPage-1;
		// 		if (menu.getSelections().size() > perPage+topFile)
		// 			topFile++;
		// 	}else if(selected+topFile >= menu.getSelections().size()) {
		// 		selected = menu.getSelections().size()-1;
//			}
			
		// 	// size_t pos = fileList.at(selected+topFile).find_last_of(".nds");
		// 	// if (pos != std::string::npos && pos == fileList.at(selected+topFile).length()-4) {
		// 	// 	u16 tmp[0x400] = {0xFFFF};
		// 	// 	screen->displayImageAt(tmp,0,0,32,32);
		// 	// 	free(tmp);
		// 	// }else{
		// 	// 	screen->displayImageAt(bmp,0,0,32,32);
		// 	// }
		// 	drawList(topFile,selected,menu);
		// 		// std::cout << menu.currentDirectory.generic_string() << "\n";
		// 		// std::cout << menu.currentDirectory.parent_path().generic_string() << "\n";
		 }else if (kDown & KEY_UP) {
		// 	selected--;
		// 	if (selected < 0) {
		// 		selected=0;
		// 		if (topFile > 0)
		// 			topFile--;
		// 	}
		// 	// size_t pos = fileList.at(selected+topFile).find_last_of(".nds");
		// 	// if (pos != std::string::npos && pos == fileList.at(selected+topFile).length()-4) {
		// 	// 	u16 tmp[0x400] = {0xFFFF};
		// 	// 	screen->displayImageAt(tmp,0,0,32,32);
		// 	// 	free(tmp);
		// 	// }else{
		// 	// 	screen->displayImageAt(bmp,0,0,32,32);
		// 	// }
		// 	drawList(topFile,selected,menu);
		// 		// std::cout << menu.currentDirectory.generic_string() << "\n";
		// 		// std::cout << menu.currentDirectory.parent_path().generic_string() << "\n";
		 	menu->up();
		 }else if (kDown & KEY_RIGHT) menu->pageDown();
		// 	topFile += perPage;
		// 	if (topFile+perPage > menu.getSelections().size()) 
		// 		topFile = menu.getSelections().size()-perPage;
		// 	drawList(topFile,selected,menu);
		 else if (kDown & KEY_LEFT) menu->pageUp();
		// 	topFile -= perPage;
		// 	if (topFile < 0) 
		// 		topFile = 0;
		// 	drawList(topFile,selected,menu);
		else if(kDown & KEY_A) menu->action();

		// 	MenuSelection m = menu.getSelections().at(topFile+selected);
		// 	if (m.isFolder) {
		// 		menu.currentDirectory = m.path;
		// 		 menu.setSelections(onChangeDir(m.path));
		// 		 selected = 0;
		// 		 topFile = 0;
		// 		 drawList(topFile,selected,menu);
		// 		std::cout << menu.currentDirectory.generic_string() << "\n";
		// 		std::cout << menu.currentDirectory.parent_path().generic_string() << "\n";
		// 	}else {
		// 		// u16 tmp[0x400] = {};
		// 		fs::path f = menu.getSelections().at(topFile+selected).path;
		// 		b.buildCIA(f.generic_string());
		// 		// LoadIconFromNDS(f.c_str(),tmp);
		// 		// screen->displayImageAt(tmp,0,0,32,32);
		// 	}
		else if(kDown & KEY_B) menu = menu->back();
		// 		if (menu.currentDirectory.has_parent_path()) {
		// 			menu.currentDirectory = menu.currentDirectory.parent_path();
		// 			menu.setSelections(onChangeDir(menu.currentDirectory));
		// 		}
		// 		selected = 0;
		// 		topFile = 0;
		// 		drawList(topFile,selected,menu);
		// 	std::cout << menu.currentDirectory << "\n";
		// }
			C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
			C2D_TargetClear(top, BGColor);
			C2D_TargetClear(bottom, C2D_Color32f(0,0,0,1));
			C2D_SceneBegin(top);	
			//drawList(topFile,selected,menu);
			menu->drawMenu();

			C2D_SceneBegin(bottom);
			config->draw();
			C3D_FrameEnd(0);
			menu = menu->handleQueue(&b,bottom,config);
		// Flush and swap framebuffers
		 //gfxFlushBuffers();
		 //gfxSwapBuffers();

		//Wait for VBlank
		 //gspWaitForVBlank();
	}
	denit();
	return 0;
	// Exit services

}
