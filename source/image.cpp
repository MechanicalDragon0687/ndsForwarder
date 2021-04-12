#include <3ds.h>
#include <string>
#include <string.h>
#include "image.hpp"
#include <iostream>
    FrameBuffer::FrameBuffer(std::string screen, bool doubleBuffer, GSPGPU_FramebufferFormat format) {
        gfxSetDoubleBuffering(((screen=="top")?GFX_TOP:GFX_BOTTOM), doubleBuffer);
	    gfxSetScreenFormat (((screen=="top")?GFX_TOP:GFX_BOTTOM), format);
	    this->bps = gspGetBytesPerPixel(format);
        framebuffer = gfxGetFramebuffer(((screen=="top")?GFX_TOP:GFX_BOTTOM), GFX_LEFT,&this->height,&this->width);
    }
    u8* FrameBuffer::getFrameBufferLocation(u16 x, u16 y) {
        return this->framebuffer+(((x*this->height)+y)*this->bps);
    }
    void FrameBuffer::displayImageAt(u16* image, u16 x, u16 y, u16 iwidth, u16 iheight) {
        std::cout << x << ", " << y << ", " << iwidth << ", " << iheight << "\n";
        std::cout << this->width << ", " << this->height << ", " << this->bps << "\n";
        int fromtop = (y);
        for (int c=0;c<iwidth;c++) {
	 	    memcpy(getFrameBufferLocation(x+c,fromtop),&image[c*(iheight)],iwidth*this->bps);
	    }
	// for (int x=0;x<32;x++) {
	// 	memcpy(getFrameBufferLocation(fb,10+x,100,width,height,bytesPerPixel),&bmp[x*(32)],32*bytesPerPixel);//32*3);
	// }

    }
