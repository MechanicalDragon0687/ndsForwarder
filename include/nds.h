/// data in this file was copied from libnds which is part of devkitpro
/// https://github.com/devkitPro/libnds
/// this header's structs are not my own and i make no claims to it
#pragma once
#include <3ds.h>
#include <citro2d.h>
typedef struct __attribute__((__packed__)) {
        char gameTitle[12];                     //!< 12 characters for the game title.
        char gameCode[4];                       //!< 4 characters for the game code.
        char makercode[2];                      //!< identifies the (commercial) developer.
        u8 unitCode;                            //!< identifies the required hardware.
        u8 deviceType;                          //!< type of device in the game card
        u8 deviceSize;                          //!< capacity of the device (1 << n Mbit)
        u8 reserved1[7];
        u8 dsi_flags;
        u8 dsi_region;
        u8 romversion;                          //!< version of the ROM.
        u8 flags;                                       //!< bit 2: auto-boot flag.

        u32 arm9romOffset;                      //!< offset of the arm9 binary in the nds file.
        u32 arm9executeAddress;               //!< adress that should be executed after the binary has been copied.
        u32 arm9destination;          //!< destination address to where the arm9 binary should be copied.
        u32 arm9binarySize;                     //!< size of the arm9 binary.

        u32 arm7romOffset;                      //!< offset of the arm7 binary in the nds file.
        u32 arm7executeAddress;               //!< adress that should be executed after the binary has been copied.
        u32 arm7destination;          //!< destination address to where the arm7 binary should be copied.
        u32 arm7binarySize;                     //!< size of the arm7 binary.

        u32 filenameOffset;                     //!< File Name Table (FNT) offset.
        u32 filenameSize;                       //!< File Name Table (FNT) size.
        u32 fatOffset;                          //!< File Allocation Table (FAT) offset.
        u32 fatSize;                            //!< File Allocation Table (FAT) size.

        u32 arm9overlaySource;          //!< File arm9 overlay offset.
        u32 arm9overlaySize;            //!< File arm9 overlay size.
        u32 arm7overlaySource;          //!< File arm7 overlay offset.
        u32 arm7overlaySize;            //!< File arm7 overlay size.

        u32 cardControl13;                      //!< Port 40001A4h setting for normal commands (used in modes 1 and 3)
        u32 cardControlBF;                      //!< Port 40001A4h setting for KEY1 commands (used in mode 2)
        u32 bannerOffset;                       //!< offset to the banner with icon and titles etc.

        u16 secureCRC16;                        //!< Secure Area Checksum, CRC-16.

        u16 readTimeout;                        //!< Secure Area Loading Timeout.

        u32 unknownRAM1;                        //!< ARM9 Auto Load List RAM Address (?)
        u32 unknownRAM2;                        //!< ARM7 Auto Load List RAM Address (?)

        u32 bfPrime1;                           //!< Secure Area Disable part 1.
        u32 bfPrime2;                           //!< Secure Area Disable part 2.
        u32 romSize;                            //!< total size of the ROM.

        u32 headerSize;                         //!< ROM header size.
        u32 zeros88[14];
        u8 gbaLogo[156];                        //!< Nintendo logo needed for booting the game.
        u16 logoCRC16;                          //!< Nintendo Logo Checksum, CRC-16.
        u16 headerCRC16;                        //!< header checksum, CRC-16.

} tNDSHeader;

typedef struct __attribute__((__packed__)) {
        tNDSHeader ndshdr;
        u32 debugRomSource;                     //!< debug ROM offset.
        u32 debugRomSize;                       //!< debug size.
        u32 debugRomDestination;        //!< debug RAM destination.
        u32 offset_0x16C;                       //reserved?

        u8 zero[0x10];

        u8 global_mbk_setting[5][4];
        u32 arm9_mbk_setting[3];
        u32 arm7_mbk_setting[3];
        u32 mbk9_wramcnt_setting;

        u32 region_flags;
        u32 access_control;
        u32 scfg_ext_mask;
        u8 offset_0x1BC[3];
        u8 appflags;

        void *arm9iromOffset;
        u32 offset_0x1C4;
        void *arm9idestination;
        u32 arm9ibinarySize;
        void *arm7iromOffset;
        u32 offset_0x1D4;
        void *arm7idestination;
        u32 arm7ibinarySize;

        u32 digest_ntr_start;
        u32 digest_ntr_size;
        u32 digest_twl_start;
        u32 digest_twl_size;
        u32 sector_hashtable_start;
        u32 sector_hashtable_size;
        u32 block_hashtable_start;
        u32 block_hashtable_size;
        u32 digest_sector_size;
        u32 digest_block_sectorcount;

        u32 banner_size;
        u32 offset_0x20C;
        u32 total_rom_size;
        u32 offset_0x214;
        u32 offset_0x218;
        u32 offset_0x21C;

        u32 modcrypt1_start;
        u32 modcrypt1_size;
        u32 modcrypt2_start;
        u32 modcrypt2_size;

        u32 tid_low;
        u32 tid_high;
        u32 public_sav_size;
        u32 private_sav_size;
        u8 reserved3[176];
        u8 age_ratings[0x10];

        u8 hmac_arm9[20];
        u8 hmac_arm7[20];
        u8 hmac_digest_master[20];
        u8 hmac_icon_title[20];
        u8 hmac_arm9i[20];
        u8 hmac_arm7i[20];
        u8 reserved4[40];
        u8 hmac_arm9_no_secure[20];
        u8 reserved5[2636];
        u8 debug_args[0x180];
        u8 rsa_signature[0x80];

} tDSiHeader;

/*!
        \brief the NDS banner format.
        See gbatek for more information.
*/

typedef struct sNDSBannerEx {
  u16 version;                  //!< version of the banner.
  u16 crcv1;                              //!< 16 bit crc/checksum of the banner.
  u16 crcv2;                              //!< 16 bit crc/checksum of the banner.
  u16 crcv3;                              //!< 16 bit crc/checksum of the banner.
  u16 crcv103;                              //!< 16 bit crc/checksum of the banner.
  u8 reserved[22];
  u8 icon[512];                 //!< 32*32 icon of the game with 4 bit per pixel.
  u16 palette[16];              //!< the pallete of the icon.
  u16 titles[16][128];   //!< title of the game in 6 different languages. extra 2+reserved come from extended format
  u8 animated_icons[8][512]; // each unique frame has its own icon
  u16 animated_palette[8][16]; // each icon has a palette ofc
  u16 animated_animation[64]; // the animation instructions
} tNDSBannerEx;


#define NDS_BANNER_SIZE_v1 0xA00
#define NDS_BANNER_SIZE_v2 0xA00
#define NDS_BANNER_SIZE_v3 0xC00 // does this even exist
#define NDS_BANNER_SIZE_v103 0x23C0


/// this is my stuff vv
void _DStoBMPorder(u8* store, u8 *source);
void rotateInPlace90(u8 *source,u32 width,u32 height);
void convertIconToBmp(u16* bmp, tNDSBannerEx* banner );
Result LoadIconFromNDS(const char* filename, u16* output);
