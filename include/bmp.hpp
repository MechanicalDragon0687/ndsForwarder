#pragma once
#include <3ds.h>
#include "nds.h"

typedef struct __attribute__((__packed__)) { 
    u32 size;
    u32 width;
    u32 height;
    u16 planes;
    u16 bpp;
    u32 compression;
    u32 imageSizeCompressed;
    u32 ppmX;
    u32 ppmY;
    u32 colors;
    u32 importantColors;
} InfoHeader;
typedef struct __attribute__((__packed__)) {
    u16 magic;
    u32 filesize;
    u32 reserved;
    u32 dataOffset;
    InfoHeader infoHeader;
} BMPHeader;


void toDSPixelOrder(u8* store, u8* source);
void convertImage(u8* store, u8* source);
void convertColors(u16 palette[16], u32 BGR888_palette[16]);
Result loadBmpAsIcon(std::string filename, tNDSBanner* banner);