#pragma once

#include <3ds.h>
#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <sstream>
#include <fstream>

//#define ROMFS_SRL "romfs:/sdcard.nds"
//#define ROMFS_TEMPLATE "romfs:/sdcard.fwd"
//#define SDCARD_SRL "sdmc:/3ds/forwarder/sdcard.nds"
//#define SDCARD_TEMPLATE "sdmc:/3ds/forwarder/sdcard.fwd"
#define SDCARD_BANNER_PATH "sdmc:/3ds/forwarder/banners"
#define NDSV1_HEADER_SIZE 0xA00
#define NDSV2_HEADER_SIZE 0x1240
#define NDSV3_HEADER_SIZE 0x2340

class Builder {
    private:
    std::string srl;
    std::map<std::string,std::string> sections;
    u32 launchPathLocation=0x229BC;
    u32 launchPathLen=252;
    u32 srlBannerLocation=0x30200;
    u8 rsaSignModulo[0x100];
    u8 rsaSignExponent[0x100];
    std::string ciaCertChain;
    std::string ticketCertChain;
    std::string buildSRL(std::string filename, bool randomTid=false, std::string customTitle="");
    std::string buildTMD(u8* contentId);
    std::string getTWLTID(u8* srl);
    std::string buildTicket();
    void readTWLTID(void* titleid, const void* srl);
    std::string buildTicket(std::string filename);
    void parseTemplate(std::string path);
    public:
    Result initialize();
    Result loadTemplate(std::string templateName);
    Result buildCIA(std::string filename, bool randomTid=false, std::string customTitle="");
    Result installCIA();
};
inline std::string readEntireFile(const std::string& path) {
  std::ostringstream buf; 
  std::ifstream input (path.c_str()); 
  buf << input.rdbuf(); 
  return buf.str();
}

inline bool fileExists (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}
inline unsigned long fileSize (const std::string& name) {
  struct stat buffer;   
  if (stat (name.c_str(), &buffer)==0) {
    return buffer.st_size;
  }
  return 0;
}
