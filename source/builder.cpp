#include <3ds.h>
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

Logger logger("Builder");



Result Builder::initialize() {

    // READ SRL TEMPLATE FROM SD or ROMFS
    std::string srlFileName = ROMFS_SRL;
    std::string srlTemplate = ROMFS_TEMPLATE;
    if (fileExists(SDCARD_SRL)) {
        if (fileExists(SDCARD_TEMPLATE)) {
            srlFileName=SDCARD_SRL;
            srlTemplate=SDCARD_TEMPLATE;
            logger.info("Loaded SDCard Templates from SDCARD.");
        }else{
            logger.error("Missing '"+std::string(SDCARD_TEMPLATE)+"'. Unable to load "+std::string(SDCARD_SRL)+" due to missing file.");
        }
    }else{
        logger.info(std::string(SDCARD_SRL)+" not found. Using built in template.");
    }
    this->srl = readEntireFile(srlFileName);
    memcpy(&this->srlBannerLocation,this->srl.c_str() + 0x68,4);

    parseTemplate(srlTemplate);
    
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
            char buf[50]={0};
            sprintf(buf,"Failed to open certs.db file. res: %lx",res);
            logger.error(std::string(buf));
            return res;
        }
        
    }

    return 0;
}
std::string Builder::buildSRL(std::string filename, bool randomTid, std::string customTitle) {
    if (filename.size() > this->launchPathLen) {
        return "";
    }
    //TODO Load nds file
    sNDSHeader header={};
    sNDSBanner banner={};
    char animatedIconData[0x1180] = {0};
    char extraTitles[2][0x100] = {0};
    const u8 noAnimation[] = {0x01,0x00,0x00,0x01};
    std::string customBannerFilename = filename.substr(0,filename.find_last_of('.'))+".bin";
    logger.info("looking for banner at "+customBannerFilename);
    bool customBanner = fileExists(customBannerFilename) && fileSize(customBannerFilename) == 0x23C0;
    if (!customBanner) {
        customBannerFilename=filename.substr(filename.find_last_of('/'),filename.find_last_of('.')-filename.find_last_of('/'))+".bin";
        customBannerFilename=SDCARD_BANNER_PATH+customBannerFilename;
        logger.info("looking for banner at "+customBannerFilename);
        customBanner = fileExists(customBannerFilename) && fileSize(customBannerFilename) == 0x23C0;
    }
    std::ifstream f(filename);
    f.seekg(0);
    f.read((char*)&header,sizeof(header));
    f.seekg(header.bannerOffset);
    f.read((char*)&banner,sizeof(banner));
    if ((banner.version & 0xFF) > 1) {
        f.read(extraTitles[0],0x100);
    }else{
        memcpy(extraTitles[0],(u8*)&banner.titles[0],0x100);
    }
    if ((banner.version & 0xFF) > 2) {
        f.read(extraTitles[0],0x100);
    }else{
        memcpy(extraTitles[1],(u8*)&banner.titles[0],0x100);
    }
    if ((banner.version & 0x100) > 0) {
        f.seekg(header.bannerOffset+0x1240);
        f.read(animatedIconData,0x1180);
    }
    

    f.close();
    
    if (customBanner==true) {
        std::ifstream f(customBannerFilename);
        f.read((char*)&banner,sizeof(banner));
        if ((banner.version & 0xFF) > 1) {
            f.read(extraTitles[0],0x100);
        }else{
            memcpy(extraTitles[0],(u8*)&banner.titles[0],0x100);
        }
        if ((banner.version & 0xFF) > 2) {
            f.read(extraTitles[0],0x100);
        }else{
            memcpy(extraTitles[1],(u8*)&banner.titles[0],0x100);
        }
        if ((banner.version & 0x100) > 0) {
            f.seekg(0x1240);
            f.read(animatedIconData,0x1180);
        }
        f.close();
    }
    //TODO apply nds file to srl
    std::string dsiware = this->srl;
    if (randomTid) {
        PS_GenerateRandomBytes(header.gameCode,4);
        for (int i = 0;i<4;i++) {
            unsigned char c = header.gameCode[i];
            if (c > 'Z' || c < 'A') c='A'+(c%26);
            header.gameCode[i] = c;
        }
    }
    if (!customTitle.empty()) {
        char cTitle[0x100/2] = {0};
        strncpy(cTitle,customTitle.c_str(),0x100/2);
        for(int i=0;i<6;i++) {
            for (int x=0;x<0x100/2;x++) {
                banner.titles[i][x] = cTitle[x];
            }
        }
        for(int i=0;i<2;i++) {
            for (int x=0;x<0x100/2;x++) {
                extraTitles[i][x] = cTitle[x];
            }
        }
    }
    // Set header
    // could be 1 command but this is easier to read
    dsiware.replace(0,0x0C,header.gameTitle,0x0C);
    dsiware.replace(0x0C,0x04,header.gameCode,0x04);
    char emagCode[] = {header.gameCode[0x03],header.gameCode[0x02],header.gameCode[0x01],header.gameCode[0x00]};
    dsiware.replace(0x230,0x04,emagCode,0x04);
    dsiware.replace(0x10,0x02,header.makercode,0x02);
    // Set Banner

     // basic banner info
     dsiware.replace(this->srlBannerLocation+0x20,0x200,(char*)banner.icon,0x200); // icon
     dsiware.replace(this->srlBannerLocation+0x220,0x20,(char*)banner.palette,0x20); // palette
     dsiware.replace(this->srlBannerLocation+0x240,0x600,(char*)banner.titles,0x600); // titles
     dsiware.replace(this->srlBannerLocation+0x840,0x200,(char*)extraTitles,0x200); // titles
//   dsiware.replace(this->srlBannerLocation,sizeof(banner),&banner,sizeof(banner));

    // animated banner
    if ((banner.version & 0x100) > 0) {
         dsiware.replace(this->srlBannerLocation + 0x1240,0x1180,animatedIconData,0x1180);
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
void writeArray(const void* data, u32 size) {
    for(u32 i=0; i<size; ++i)
        std::cout << std::hex << (int)((u8*)data)[i];
    std::cout << std::endl;
}

Result Builder::buildCIA(std::string filename, bool randomTid, std::string customTitle) {

//    const u16 contentCount = 1;
    // GET RANDOM CONTENT ID
    u8 contentID[4]={0x00,0x02,0x03,0x04};
    PS_GenerateRandomBytes(contentID+1,3);

    this->sections["content"] = buildSRL(filename, randomTid, customTitle);
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
    std::cout << "Created cia file data" << std::endl;

#ifdef DEBUG
    std::ofstream fo(filename.substr(0,filename.find_last_of('.')+1)+"cia");
    fo.write(cia.c_str(),cia.size());
    fo.close();
#endif
    Handle ciaInstallFileHandle={};
    u32 installSize = cia.size(), installOffset = 0;
	u32 bytes_written = 0;
    u32 size = cia.size();

    Result ret =amInit();
    if (R_FAILED(ret)) {
		std::cout << "Failed to Initialize AM\n" << std::endl;
        return ret;
	}
	ret = AM_StartCiaInstall(MEDIATYPE_NAND, &ciaInstallFileHandle);
	if (R_FAILED(ret)) {
		std::cout << "Error in:\nAM_StartCiaInstall\nret: " << std::hex << ret << std::endl;
        return ret;
	}
    std::cout << "Writing to file";
	do {
        if (size > installSize)
            size=installSize;
		ret = FSFILE_Write(ciaInstallFileHandle, &bytes_written, installOffset, cia.c_str(), size, FS_WRITE_FLUSH);
        if (R_FAILED(ret)) {
    		std::cout << "\nError in:\nwriting to file\nret: " << std::hex << ret << std::endl;
            AM_CancelCIAInstall(ciaInstallFileHandle);
            return ret;
        }
        std::cout << ".";
		installOffset += bytes_written;
	} while(installOffset < installSize);
    std::cout << "done\n";
	ret = AM_FinishCiaInstall(ciaInstallFileHandle);
	if (R_FAILED(ret)) {
		printf("Error in:\nAM_FinishCiaInstall\n");
		return ret;
	}
    std::cout << "Installed Forwarder" << std::endl;
    amExit();
    
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