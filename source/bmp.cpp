#include <3ds.h>
#include <string>
#include <fstream>
#include "bmp.hpp"
#include "logger.hpp"

Logger bmplogger("BmpHandler");
Result loadBmpAsIcon(std::string filename, tNDSBanner* banner) {
    BMPHeader bmpHeader={0};
    std::ifstream f(filename);
    if (f.fail()) {
        bmplogger.error("Failed to open "+filename);
        return -1;
    }
    f.read((char*)&bmpHeader, sizeof(bmpHeader));
    if (bmpHeader.magic !=0x4D42) {
        f.close();
        bmplogger.error(filename + " is not a valid bitmap file");
        bmplogger.error(std::to_string(bmpHeader.magic));
        return -1;
    }
    if (bmpHeader.infoHeader.width != 32 || bmpHeader.infoHeader.height != 32) {
        bmplogger.error(filename + " has invalid width or height. Must be 32x32");
        bmplogger.error(filename + " is " + std::to_string(bmpHeader.infoHeader.width) + "x"+std::to_string(bmpHeader.infoHeader.height));
        return -1;
    }
    if (bmpHeader.infoHeader.importantColors>16) {
        bmplogger.error(filename + " has too many colors.");
        return -1;
    }
    if (bmpHeader.infoHeader.bpp>4) {
        bmplogger.error(filename + " is too deep. use 4bit.");
        return -1;
    }
    if (bmpHeader.infoHeader.compression != 0) {
        bmplogger.error(filename + " is compressed. Compression is not supported.");
    }
    u32 BGR888_palette[16]={0};
    u16 palette[16]={0};
    f.read((char*)BGR888_palette,sizeof(u32)*16);
    f.seekg(bmpHeader.dataOffset);
    u8 BGR888_data[0x200]={0};
    f.read((char*)BGR888_data,0x200);
    f.close();
    convertColors(palette, BGR888_palette);
    u8 image[0x200]={0};
    convertImage(image,BGR888_data);
    memcpy(banner->icon,image,0x200);
    memcpy(banner->palette,palette,32);
    return 0;
}
void convertColors(u16 palette[16], u32 BGR888_palette[16]) {
    for (u8 i=0;i<16;i++) {
        u32 color = BGR888_palette[i];
        u16 r = (color >> 19) & 0b11111;
        u16 g = (color >> 11) & 0b11111;
        u16 b = (color >> 3) & 0b11111;
        palette[i] = b<<10|g<<5|r;
    }
}
void convertImage(u8* store, u8* source) {
  toDSPixelOrder(store, source);
}
void toDSPixelOrder(u8* store, u8* source) {
    
//   // ds icons are 8x8 pixel boxes, with 4bit width. 4x8 byte boxes
//   // they are arranged in a 4 wide by 4 tall pattern of boxes to 32x32 pixels
   u8 tmp[0x200];
   int offset = 0;
   for (int i=0;i<0x200;i++) {
       source[i]=(source[i]<<4)|(source[i]>>4);
   }
  // start at the bottom set of boxes
  for (int yy=3;yy>=0;yy--) {
    // start at the bottom row of the bottom set of boxes
      for(int y=7;y>=0;y--) {
          for (int cc=0;cc<4;cc++) {
                  memcpy(&tmp[y*4+cc*32+yy*128],source+offset,4);
                  offset+=4;
          }
      }
  }
  memcpy(store,tmp,0x200);
}