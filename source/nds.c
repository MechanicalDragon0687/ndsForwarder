#include <3ds.h>
#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <citro2d.h>
//#define BGR5A1_RGB5A1(src)  (((src >> 10) & 0b11111) | ((src & 0b11111) << 10) | (src & 0b1111100000))

u16 RGB5A1_BGR5A1(u16 color) {
    u16 r = ((color >> 10) & 0b11111);
    u16 g = ((color >> 5) & 0b11111);
    u16 b = ((color) & 0b11111);
    return b<<11|g<<6|r<<1|1;
}
void _DStoBMPorder(u8* store, u8 *source) {
  
  // ds icons are 8x8 pixel boxes, with 4bit width. 4x8 byte boxes
  // they are arranged in a 4 wide by 4 tall pattern of boxes to 32x32 pixels
  u8 tmp[0x200];
  int offset = 0;
  // start at the bottom set of boxes
  for (int yy=3;yy>=0;yy--) {
    // start at the bottom row of the bottom set of boxes
      for(int y=7;y>=0;y--) {
          for (int cc=0;cc<4;cc++) {
                  memcpy(tmp+offset,&source[y*4+cc*32+yy*128],4);
                  offset+=4;
          }
      }
  }
  for(int x=0;x<0x200;x++) {
      store[x*2]=tmp[x] & 0xF;
      store[1+x*2]=(tmp[x] >> 4) & 0xF;
  }
}
void convertIconToBmp(u16* bmp, tNDSBanner* banner ) {
	u8 store[0x400]={0};
	_DStoBMPorder(store,banner->icon);
	rotateInPlace90(store,32,32);
	u16 colors[16]={0};
     for (u8 x=0;x<16;x++) {
         colors[x] = RGB5A1_BGR5A1(banner->palette[x]);
        //u16 color=banner->palette[x];
        // colors[x][2]=(color & 0b11111) << 3;
        // colors[x][1]=((color >> 5) & 0b11111) << 3;
        // colors[x][0]=((color >> 10) & 0b11111) << 3;
     }
	for (u32 x=0;x<0x400;x++) {
		memcpy(bmp+(x),&colors[store[x]],2);
//		printf("%d|",store[x]);
	}
	return;
}
void rotateInPlace90(u8 *source,u32 width,u32 height) {
	u8 n[width][height];
	for (u32 x=0;x<width;x++) {
		for(u32 y=0;y<height;y++) {
			memcpy(&n[y][x],source+((x*height)+y),1);
		}
	}
	memcpy(source,n,width*height);
}
Result LoadIconFromNDS(const char* filename, u16* output) {
    
    FILE* f = fopen(filename,"rb");
    if (ferror(f)) {
        return -1;
    }
    tNDSHeader* header = malloc(sizeof(tNDSHeader));
	tNDSBanner* banner=malloc(sizeof(tNDSBanner));

	//Initialize console on top screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
	 fread(header,sizeof(tNDSHeader),1,f);
	 if (header->bannerOffset > 0) {
	 	fseek(f,header->bannerOffset,SEEK_SET);
		fread(banner,sizeof(tNDSBanner),1,f);
        fclose(f);
        free(header);
        header=NULL;
        
        convertIconToBmp(output,banner);

        free(banner);
        banner=NULL;
        return 0;
     }
     return -1;
}
