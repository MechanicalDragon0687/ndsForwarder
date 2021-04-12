#pragma once

#include <string>
#include <3ds.h>

class FrameBuffer {
    u16 bps;
    u8* framebuffer;
    u16 height;
    u16 width;

    public:
    FrameBuffer(std::string screen, bool doubleBuffer, GSPGPU_FramebufferFormat format);
    u8* getFrameBufferLocation(u16 x, u16 y) ;
    void displayImageAt(u16* image, u16 x, u16 y, u16 iwidth, u16 iheight) ;
};