#include <3ds.h>
#include "settings.hpp"
#include "builder.hpp"
#include <iostream>
#include <fstream>
#include "logger.hpp"
#include <string>
#include <vector>
#include <map>
#include "helpers.hpp"
#include <cstring>
#include "nds.h"
#include "tmd.h"
#include "cia.h"
#include "ticket.h"
#include "bmp.hpp"
#include "lang.hpp"

Logger logger("Builder");


Result Builder::loadTemplate(std::string templateName) {
    this->launchPathLen = 0;
    this->launchPathLocation=0;
    if (templateName.empty())
        return -1;
    std::string srlFileName = templateName + ".nds";//ROMFS_SRL;
    std::string srlTemplate = templateName + ".fwd";//ROMFS_TEMPLATE;
    if (fileExists(SDCARD_TEMPLATE_DIR+srlFileName)) {
        if (fileExists(SDCARD_TEMPLATE_DIR+srlTemplate)) {
            srlFileName=SDCARD_TEMPLATE_DIR+srlFileName;
            srlTemplate=SDCARD_TEMPLATE_DIR+srlTemplate;
            logger.info(gLang.parseString("builder_loadingTemplate",templateName.c_str()));
        }else{
            logger.error(gLang.parseString("builder_missingTemplate",(std::string(SDCARD_TEMPLATE_DIR)+srlTemplate).c_str(),(std::string(SDCARD_TEMPLATE_DIR)+srlFileName).c_str()));
            return -1;
        }
    }else if(fileExists(ROMFS_TEMPLATE_DIR+srlTemplate) && fileExists(ROMFS_TEMPLATE_DIR+srlFileName)) {
        logger.info(gLang.parseString("builder_defaultTemplate",templateName.c_str()));
        srlTemplate=ROMFS_TEMPLATE_DIR+srlTemplate;
        srlFileName=ROMFS_TEMPLATE_DIR+srlFileName;
    }else{
        logger.info(gLang.parseString("builder_missingSRL",templateName.c_str(),(SDCARD_TEMPLATE_DIR).c_str()));
        return -1;
    }
    this->srl = readEntireFile(srlFileName);
    memcpy(&this->srlBannerLocation,this->srl.c_str() + 0x68,4);
    
    parseTemplate(srlTemplate);
    if (this->launchPathLocation == 0 || this->launchPathLen==0) return -1;
    return 0;

}


Result Builder::initialize() {

    
    // READ CERTCHAIN FROM CERTS.DB
    {
        std::vector<u32> offsetSizePairs{
            0xC10,0x1F0,
            0x3A00,0x210,
            0x3F10,0x300,
            0x3C10,0x300
        };
        Handle hFile={};
        char buf[0x300]={0};
        u32 readCount=0;
        std::string ciacert;
        std::map<u32,std::string> certParts;

        Result res = FSUSER_OpenFileDirectly(&hFile, ARCHIVE_NAND_CTR_FS, fsMakePath(PATH_EMPTY,""), fsMakePath(PATH_ASCII,"/dbs/certs.db"), FS_OPEN_READ, 0);
        if (R_SUCCEEDED(res)) {
            
            for (size_t i=0;i<offsetSizePairs.size();i+=2) {
                u32 offset = offsetSizePairs.at(i);
                u32 size = offsetSizePairs.at(i+1);
                FSFILE_Read (hFile, &readCount, offset, buf, size);
                certParts[offset]=std::string(buf,size);
            }
            this->ciaCertChain=certParts[0xc10]+certParts[0x3a00]+certParts[0x3f10]+certParts[0x3c10];
            this->ticketCertChain=certParts[0x3f10]+certParts[0xc10]+certParts[0x3a00];
            FSFILE_Read(hFile,&readCount,0x3c10,buf,0x300);
            memcpy(this->rsaSignModulo,buf+0x1c8,100);
            memcpy(this->rsaSignExponent,buf+0x2c8,4);
            FSFILE_Close (hFile);

        }else{
            //char buf[50]={0};
            //sprintf(buf,"Failed to open certs.db file. res: %lx",res);
            logger.error(gLang.parseString("builder_noCertsDB",res));
            return res;
        }
        this->launchPathLen = 0;
        this->launchPathLocation=0;
    }

    return 0;
}
std::string Builder::buildSRL(std::string filename, bool randomTid, std::string customTitle) {
    if (filename.size() > this->launchPathLen) {
        return "";
    }
    //TODO Load nds file
    tDSiHeader header = {};
    // sNDSHeader header={};
    sNDSBannerEx banner={};
    // char animatedIconData[0x1180] = {0};
    // char extraTitles[2][0x100] = {0};
    const u8 noAnimation[] = {0x01,0x00,0x00,0x01};
    std::string customBannerFilename = filename.substr(0,filename.find_last_of('.'))+".bin";
    std::string customIconFilename = filename.substr(0,filename.find_last_of('.'))+".bmp";

    logger.info(gLang.parseString("builder_searchBanner",customBannerFilename.c_str()));
    bool customBanner = fileExists(customBannerFilename) && fileSize(customBannerFilename) == 0x23C0;
    bool customBMPIcon = fileExists(customIconFilename);
    if (!customBanner) {
        customBannerFilename=filename.substr(filename.find_last_of('/')+1,filename.find_last_of('.')-filename.find_last_of('/')-1)+".bin";
        customBannerFilename=SDCARD_BANNER_PATH+customBannerFilename;
        logger.info(gLang.parseString("builder_searchBanner",customBannerFilename.c_str()));
        customBanner = fileExists(customBannerFilename) && fileSize(customBannerFilename) == 0x23C0;
    }
    if (!customBMPIcon) {
        customIconFilename=filename.substr(filename.find_last_of('/')+1,filename.find_last_of('.')-filename.find_last_of('/')-1)+".bmp";
        customIconFilename=SDCARD_ICON_PATH+customIconFilename;
        logger.info(gLang.parseString("builder_searchIcon",customIconFilename.c_str()));
        customBMPIcon = fileExists(customIconFilename);
    }
    std::ifstream f(filename);
    f.seekg(0);
    f.read((char*)&header,sizeof(header.ndshdr));
    bool extendedHeader = (header.ndshdr.headerSize == 0x4000);
    if (extendedHeader) {
        f.seekg(0);
        f.read((char*)&header,sizeof(header));
    }
    u16 headerCRC = crc16Modbus((char*)&header,0x15E);
    if (headerCRC != header.ndshdr.headerCRC16) {
        logger.error(gLang.parseString("builder_invalidHeaderCRC",headerCRC,header.ndshdr.headerCRC16));
        f.close();
        return "";
    }
    if (extendedHeader && (
            (header.tid_high & 0xFF) > 0) &&
            (memcmp(header.ndshdr.gameCode,"HNEA",4)!=0)
    ) {
        logger.error(gLang.getString("builder_invalidROMType"));
        f.close();
        return "";
    }
    if (header.ndshdr.bannerOffset == 0) {
        logger.error(gLang.getString("builder_noNDSBanner"));
        f.close();
        return "";
    }
    u16 banner_version=1;
    u32 seekLocation = 0;
    if (!customBanner) {
        seekLocation = header.ndshdr.bannerOffset;
        f.seekg(header.ndshdr.bannerOffset);

    }else{
        f.close();
        f.open(customBannerFilename);
    }
    f.read((char*)&banner_version,2);
    f.seekg(seekLocation);
    switch(banner_version) {
        case 0x03:
            f.read((char*)&banner,NDS_BANNER_SIZE_v3);
            break;
        case 0x103:
            f.read((char*)&banner,NDS_BANNER_SIZE_v103);
            break;
        default:
            f.read((char*)&banner,NDS_BANNER_SIZE_v1);
            break;
    }
    f.close();
    #ifdef DEBUG
    std::ofstream dbgfile(filename+".dbgbnr");
    dbgfile.write((char*)&banner,sizeof(banner));
    dbgfile.close();
    #endif
    u16 expectedCRC = crc16Modbus(&(banner.icon),0x820);
    if (expectedCRC != banner.crcv1) {
        logger.debug(gLang.parseString("debug_crc",banner.crcv1,expectedCRC));
        logger.error(gLang.parseString("builder_invalidBannerCRC","1"));
        return "";
    }
    if (banner.version > 1) {
        expectedCRC = crc16Modbus(&(banner.icon),0x920);
        if (expectedCRC != banner.crcv2) {
            logger.debug(gLang.parseString("debug_crc",banner.crcv2,expectedCRC));
            logger.error(gLang.parseString("builder_invalidBannerCRC","2"));
            return "";
        }
    }
    if (banner.version > 2) {
        expectedCRC = crc16Modbus(&(banner.icon),0xA20);
        if (expectedCRC != banner.crcv3) {
            logger.debug(gLang.parseString("debug_crc",banner.crcv3,expectedCRC));
            logger.error(gLang.parseString("builder_invalidBannerCRC","3"));
            return "";
        }
    }
    
    if ((banner.version & 0x100) > 0) {
        expectedCRC = crc16Modbus(&(banner.animated_icons),0x1180);
        if (expectedCRC != banner.crcv103) {
            logger.debug(gLang.parseString("debug_crc",banner.crcv103,expectedCRC));
            logger.error(gLang.parseString("builder_invalidBannerCRC","4"));
            return "";
        }
    }
    
    if(customBMPIcon) {
        if (R_SUCCEEDED(loadBmpAsIcon(customIconFilename,&banner))) {
            banner.version &= 3; // clear animated icon identifier
            banner.crcv1 = crc16Modbus((char*)&banner+0x20,0x820);
            if (banner.version > 1) banner.crcv2 = crc16Modbus((char*)&banner+0x20,0x920);
            if (banner.version > 2) banner.crcv3 = crc16Modbus((char*)&banner+0x20,0xA20);
        }
    }
    //TODO apply nds file to srl
    std::string dsiware = this->srl;
    if (randomTid) {
        PS_GenerateRandomBytes(header.ndshdr.gameCode,4);
        for (int i = 0;i<4;i++) {
            unsigned char c = header.ndshdr.gameCode[i];
            if (c > 'Z' || c < 'A') c='A'+(c%26);
            header.ndshdr.gameCode[i] = c;
        }
    }
    if (!customTitle.empty()) {
        uint16_t cTitle[0x80] = {0};
        utf8_to_utf16(cTitle, (u8*)customTitle.c_str(), 0x80);
        for(int i=0;i<8;i++) {
            memcpy(banner.titles[i], cTitle, 0x80 * sizeof(uint16_t));
        }
        // for(int i=0;i<2;i++) {
        //     memcpy(extraTitles[i], cTitle, 0x80 * sizeof(uint16_t));
        // }
    }
    // Set header
    // could be 1 command but this is easier to read
    dsiware.replace(0,0x0C,header.ndshdr.gameTitle,0x0C);
    dsiware.replace(0x0C,0x04,header.ndshdr.gameCode,0x04);
    char emagCode[0x04] = {};
    if (!extendedHeader)
        memcpyrev(emagCode,header.ndshdr.gameCode,4);
    else
        memcpy(emagCode,(char*)&header.tid_low,4);
    dsiware.replace(0x230,0x04,emagCode,0x04);
    dsiware.replace(0x10,0x02,header.ndshdr.makercode,0x02);
    // Set Banner

     // basic banner info
     dsiware.replace(this->srlBannerLocation+0x20,0x200,(char*)banner.icon,0x200); // icon
     dsiware.replace(this->srlBannerLocation+0x220,0x20,(char*)banner.palette,0x20); // palette
     dsiware.replace(this->srlBannerLocation+0x240,0x600,(char*)banner.titles,0x800); // titles
//     dsiware.replace(this->srlBannerLocation+0x840,0x200,(char*)banner.titles[0x07],0x200); // titles
//   dsiware.replace(this->srlBannerLocation,sizeof(banner),&banner,sizeof(banner));

    // animated banner
    if ((banner.version & 0x100) > 0) {
         dsiware.replace(this->srlBannerLocation + 0x1240,0x1180,(char*)banner.animated_icons,0x1180);
    }else{
        // fill all icon data with default
        // then replace animation sequence with static image
        for (u8 i = 0;i<8;i++) {
            dsiware.replace(this->srlBannerLocation+0x1240+(0x200 * i),0x200,(char*)banner.icon,0x200);
            dsiware.replace(this->srlBannerLocation+0x2240+(0x20 * i),0x20,(char*)banner.palette,0x20);
        }
        dsiware.replace(this->srlBannerLocation+0x2340,0x80,0x80,'\0');
        dsiware.replace(this->srlBannerLocation+0x2340,0x4,(char*)noAnimation,0x04);
    }
    
    // SET PATH
    std::string modifiedPath = filename.substr(filename.find_first_of('/')+1);
    dsiware.replace(this->launchPathLocation,this->launchPathLen,this->launchPathLen,'\0');
    dsiware.replace(this->launchPathLocation,modifiedPath.size(),modifiedPath);

    u16 bannerCrcs[]={
        crc16Modbus(&dsiware[this->srlBannerLocation+0x20],0x820),
        crc16Modbus(&dsiware[this->srlBannerLocation+0x20],0x920),
        crc16Modbus(&dsiware[this->srlBannerLocation+0x20],0xA20),
        crc16Modbus(&dsiware[this->srlBannerLocation+0x1240],0x1180)
    };
    u16 headerCrc = crc16Modbus(dsiware.c_str(),0x15E);
    dsiware.replace(this->srlBannerLocation+0x02,8,(char*)&bannerCrcs,8);
    dsiware.replace(0x15E,2,(char*)&headerCrc,2);
#ifdef DEBUG
    std::fstream of(filename.substr(0,filename.find_last_of("."))+".srl",std::ios_base::binary | std::ios_base::out);
    of << dsiware;
    of.close();
#endif
    //build twl
    return dsiware;
}
#ifdef DEBUG
void writeArray(const void* data, u32 size) {
    for(u32 i=0; i<size; ++i)
        std::cout << std::hex << (int)((u8*)data)[i];
    std::cout << std::endl;
}
#endif
Result Builder::buildCIA(std::string filename, bool randomTid, std::string customTitle) {

//    const u16 contentCount = 1;
    // GET RANDOM CONTENT ID
    u8 contentID[4]={0x00,0x02,0x03,0x04};
    PS_GenerateRandomBytes(contentID+1,3);

    this->sections["content"] = buildSRL(filename, randomTid, customTitle);
    if (this->sections["content"].size() == 0) {
        logger.error(gLang.getString("builder_fail"));
        return -1;
    }
	std::string srlSha = sha256( (u8*)this->sections["content"].c_str(), this->sections["content"].size());
    this->sections["ticket"]=buildTicket(filename);
    this->sections["tmd"]=this->buildTMD(contentID);
    
    sCiaHeader header={};
    header.certchainSize = this->ciaCertChain.size();
    header.contentSize = this->sections["content"].size();
    header.headerSize=sizeof(header);
    header.metaSize=0;
    header.ticketSize=this->sections["ticket"].size();
    header.tmdSize=this->sections["tmd"].size();
    memset(header.contentIdx,0,0x2000);
    header.contentIdx[0x0] = 0x80;
    std::string cia = aligned(std::string((char*)&header,sizeof(header)),0x40) + aligned(this->ciaCertChain,0x40) + aligned(this->sections["ticket"],0x40) + aligned(this->sections["tmd"],0x40) + aligned(this->sections["content"],0x40);
#ifdef DEBUG
    std::cout << "Created cia file data" << std::endl;

    std::ofstream fo(filename.substr(0,filename.find_last_of('.')+1)+"cia");
    fo.write(cia.c_str(),cia.size());
    fo.close();
#endif
    Handle ciaInstallFileHandle={};
    u32 installSize = cia.size(), installOffset = 0;
	u32 bytes_written = 0;
    u32 size = cia.size();

	Result ret = AM_StartCiaInstall(MEDIATYPE_NAND, &ciaInstallFileHandle);
    if (R_FAILED(ret)) {
        logger.error(gLang.parseString("builder_ErrInRet","AM_StartCiaInstall",ret));
        return ret;
	}
    logger.debug(gLang.getString("debug_writingToFile"));
	do {
        if (size > installSize)
            size=installSize;
		ret = FSFILE_Write(ciaInstallFileHandle, &bytes_written, installOffset, cia.c_str(), size, FS_WRITE_FLUSH);
        if (R_FAILED(ret)) {
            logger.error(gLang.parseString("builder_ErrInRet","writing to file",ret));
            AM_CancelCIAInstall(ciaInstallFileHandle);
            return ret;
        }
		installOffset += bytes_written;
	} while(installOffset < installSize);
    logger.debug(gLang.getString("debug_done"));
	ret = AM_FinishCiaInstall(ciaInstallFileHandle);
	if (R_FAILED(ret)) {
		logger.error(gLang.parseString("builder_ErrInRet","AM_FinishCiaInstall",ret));
		return ret;
	}
    logger.debug(gLang.getString("debug_installedForwarder"));
    
return 0;
}

std::string Builder::buildTicket(std::string filename) {

    std::string hash = sha256((u8*)filename.c_str(),filename.size());

    sSignature sig = {.sigType={0x00,0x01,0x00,0x04}};
    sTicketHeader header={0};
    //memset(&header,0,sizeof(header));
    u8 issuer[]="Root-CA00000003-XS0000000c";
    
    // https://github.com/Tiger21820/ctr_toolkit/blob/master/make_cia/ticket.h#L31
    u8 CONTENT_INDEX[] =
    {   0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, 0xAC, 0x00, 0x00, 0x00, 0x14, 0x00, 0x01, 0x00, 0x14, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x84, 
        0x00, 0x00, 0x00, 0x84, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00
    };

    header.version=1;
    memcpy(header.signatureIssuer,issuer,26);
    memcpy(header.ticketId,hash.c_str(),8);
    readTWLTID(header.titleId,this->sections["content"].c_str());
    // we dont actually need a valid encrypted title key, right?
    // lets just use a title key of all 0 and normal key of all 0 and see where that takes us
    u8 iv[0x10] = {0};
    memcpy(iv,header.titleId,8);
    u8 key[0x10] = {0};
    encryptAES128CBC(header.titleKey,iv,key,header.titleKey,0x10);
    memcpy(header.contentIndex,CONTENT_INDEX,0x30);

    //sign((u8*)sha256((u8*)&header,sizeof(header)).c_str(),this->rsaSignModulo, 0x100 ,this->rsaSignExponent, 0x100 ,sig.signature);
    // who needs a real signature either
    // sigpatches ftw
    sig.signature[0x100-1]=1;
    return (std::string((char*)&sig,sizeof(sig))+std::string((char*)&header,sizeof(header)));
}
std::string Builder::buildTMD(u8* contentId) {
    
    static const u8 sig_type[4] =  { 0x00,0x01,0x00,0x04 };

    //TMD HEADER
    sTMDHeader tmd={};
    memset(&tmd,0,sizeof(tmd));
    u8 issuer[]="Root-CA00000003-CP0000000b";
    memcpy(tmd.signatureIssuer,issuer,26);
    tmd.version=1;
    readTWLTID(tmd.titleId,this->sections["content"].c_str());
    tmd.titleType[0x3]=0x40; // title type: CTR
    memcpy(tmd.saveDataSizeLE +0x5A,this->sections["content"].c_str()+0x238,4); // will always be 0 unless template changes
    memcpy(tmd.privateSaveDataSizeLE,this->sections["content"].c_str()+0x23C,4); // will always be 0 unless template changes
    tmd.srlFlag=this->sections["content"][0x1C];
    memcpy(tmd.titleVersion,this->sections["content"].c_str()+0x1D,2); // title version
    tmd.contentCount[1] = 1;
    
    sContentInfoRecord infoRecords[64] = {0};
    infoRecords[0].contentCommandCount[1]=1;
    sContentChunkRecord contentChunkRecord={0};
    memcpy(contentChunkRecord.contentId,contentId,4);
    u64 contentSize = this->sections["content"].size();
    memcpyrev(&contentChunkRecord.contentSize,(u8*)&contentSize,8);
    memcpy(contentChunkRecord.hash,sha256((u8*)this->sections["content"].c_str(),this->sections["content"].size()).c_str(),0x20);
    
    memcpy(infoRecords[0].contentChunkHash,sha256((u8*)&contentChunkRecord,sizeof(contentChunkRecord)).c_str(),0x20);
    memcpy( tmd.contentInfoHash,sha256((u8*)&infoRecords,sizeof(sContentInfoRecord)*64).c_str(),0x20);
    
    std::string hash = sha256((u8*)&tmd,sizeof(sTMDHeader));
    u8 signature[0x100] = {0};
    // sign((u8*)hash.c_str(),this->rsaSignModulo, 0x100 ,this->rsaSignExponent, 0x100 ,signature);
    // you have sig patches, right? cool...
    signature[0x100-1]=1;
    std::string tmdSection;
    char reserved[0x3c]={0};
    tmdSection += std::string((char*)sig_type,4);
    tmdSection += std::string((char*)signature,0x100);
    tmdSection += std::string(reserved, 0x3C);
    tmdSection += std::string((char*)&tmd,sizeof(tmd));
    tmdSection += std::string((char*)infoRecords,sizeof(sContentInfoRecord)*64);
    tmdSection += std::string((char*)&contentChunkRecord,sizeof(contentChunkRecord));
    return tmdSection;
}
std::string Builder::getTWLTID(u8* srl) {
    u8 twltitle[8]={0x00,0x04,0x80,0x00,0x00,0x00,0x00,0x00};
    for (int i=0;i<6;i++)
        twltitle[i]=srl[0x233+i];
    return std::string((char*)twltitle,8);
}
void Builder::readTWLTID(void* titleid, const void* srl) {
    u8 twltitle[8]={0x00,0x04,0x80,0x00,0x00,0x00,0x00,0x00};
    memcpyrev(twltitle+3,(u8*)srl + 0x230,5);
    memcpy(titleid,twltitle,8);
}

void Builder::parseTemplate(std::string path) {
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line))
    {
     
        u32 value = 0;
        u8 base  = 16;
        __try {
            std::string sval = line.substr(line.find_first_of('=')+1);
            if (strncmp(sval.c_str(), "0x", 2)!=0) {
                base = 10;
            }
            value = stoul(sval,nullptr,base);
            std::string key(line.substr(0,line.find_first_of('=')));
        
            if (key=="gamepath_location") {
                this->launchPathLocation = value;
            }
            if (key=="gamepath_length") {
                this->launchPathLen = value;
            }
        }
        __catch(const std::invalid_argument & e) { }
        
    }
    f.close();
   
}