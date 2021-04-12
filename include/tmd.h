#include <3ds.h>
typedef struct
{
	u8 signatureIssuer[0x40];
	u8 version;
	u8 ca_crl_version;
	u8 signer_crl_version;
	u8 reserved1;
	u8 systemVersion[8];
	u8 titleId[8];
	u8 titleType[4];
	u8 groupId[2];
	u8 saveDataSizeLE[4];
	u8 privateSaveDataSizeLE[4]; // Zero for CXI Content0
	u8 reserved2[4];
	u8 srlFlag; // Zero for CXI Content0
	u8 reserved_3[0x31];
	u8 accessRights[4];
	u8 titleVersion[2];
	u8 contentCount[2];
	u8 bootContent[2];
	u8 padding[2];
	u8 contentInfoHash[0x20];	
} __attribute__((__packed__)) 
sTMDHeader;

typedef struct 
{
    u8 contentIdxOffset[2];
    u8 contentCommandCount[2];
    u8 contentChunkHash[0x20];
}__attribute__((__packed__)) 
sContentInfoRecord;

typedef struct 
{
    u8 contentId[4];
    u8 contentIdx[2];
    u8 contentType[2];
    u8 contentSize[8];
    u8 hash[0x20];
}__attribute__((__packed__)) 
sContentChunkRecord;
