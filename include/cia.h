/*
0x00	0x04	Archive Header Size (Usually = 0x2020 bytes)
0x04	0x02	Type
0x06	0x02	Version
0x08	0x04	Certificate chain size
0x0C	0x04	Ticket size
0x10	0x04	TMD file size
0x14	0x04	Meta size (0 if no Meta data is present)
0x18	0x08	Content size
0x20	0x2000	Content Index
*/
#include <3ds.h>
typedef struct 
{
    u32 headerSize;
    u16 type;
    u16 version;
    u32 certchainSize;
    u32 ticketSize;
    u32 tmdSize;
    u32 metaSize;
    u64 contentSize;
    u8 contentIdx[0x2000];
} __attribute__((__packed__)) 
sCiaHeader;
