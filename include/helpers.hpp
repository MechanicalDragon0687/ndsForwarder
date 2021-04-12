#include <3ds.h>
#include <vector>
#define REVERSE32 (val) ((val&0xFF)<<24)|((val&0xFF00)<<8)|((val&0xFF0000)>>8)|((val>>24)&0xFF)

std::string sha256(u8* data, u32 size);
u16 crc16Modbus(const void* data, u32 size);
void memcpyrev(void* dest, void* source, u32 size);
Result sign(u8* hash, u8* mod, u32 modSize, u8* exp, u32 expSize, u8* signature);
void encryptAES128CBC(u8* out, u8* iv, u8* key, u8* data, u32 size) ;

std::string aligned(const void* data, u64 size, u64 padTo);
std::string aligned(std::string data, u64 padTo);
std::string alignmentPadding(u64 size, u64 padTo);

typedef struct 
{
    u8 sigType[4];
    u8 signature[0x100];
    u8 padding[0x3c];
}__attribute__((__packed__)) 
sSignature;

