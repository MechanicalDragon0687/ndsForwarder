#include <3ds.h>
#include <string>
#include <vector>
#include "aes.h"
#include <cstring>
#include "helpers.hpp"
std::string sha256(u8* data, u32 size) 
{
    u8 buf[0x20]={0};
    Result res = FSUSER_UpdateSha256Context(data,size,buf);
    if (R_SUCCEEDED(res)) {
        return std::string((char*)buf,0x20);
    }
    return "";
}
// taken from pseudocode in GBATEK
u16 crc16Modbus(const void* data, u32 size) {
    u8* values = (u8*)data;
  unsigned short crc = 0xFFFF;
  for (unsigned int i=0; i < size; i++) {
    crc=crc ^ values[i];
    for (unsigned char j=0;j<8;j++) {
      bool carry = (crc & 0x0001) > 0;
      crc=crc >> 1;
      if (carry)
        crc=crc^ 0xA001;
    }
  }
  return crc;
}
void memcpyrev(void* dest, void* source, u32 size) {
    for (u32 i=0; i<size; i++) {
        ((u8*)dest)[i] = ((u8*)source)[(size-1)-i];
    }
}
Result sign(u8* hash, u8* mod, u32 modSize, u8* exp, u32 expSize, u8* signature) {
  psRSAContext ctx({.rsa_bitsize=2048});
  memcpy(ctx.exponent,exp,expSize);
  memcpy(ctx.modulo,exp,modSize);
  return PS_SignRsaSha256(hash,&ctx,signature);
}

void encryptAES128CBC(u8* out, u8* iv, u8* key, u8* data, u32 size) {
  AES_ctx ctx;
  u8 buf[size]={0};
  memcpy(buf,data,size);
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CBC_encrypt_buffer(&ctx, buf, size);
  memcpy(out,buf,size);
}
std::string aligned(const void* data, u64 size, u64 padTo) {
    return aligned(std::string((char*)data,size),padTo);
}
std::string aligned(std::string data, u64 padTo) {
    return data + alignmentPadding(data.size(),padTo);
}
std::string alignmentPadding(u64 size, u64 padTo) {
    u64 count = padTo-(size % padTo);
    if (count > 0 && count < padTo) {
      char pad[count]={0};
      return std::string(pad,count);
    }
    return std::string();
}
