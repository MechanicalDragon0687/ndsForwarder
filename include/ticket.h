/*
0x0	0x40	Issuer
0x40	0x3C	ECC PublicKey
0x7C	0x1	Version (For 3DS this is always 1)
0x7D	0x1	CaCrlVersion
0x7E	0x1	SignerCrlVersion
0x7F	0x10	TitleKey (normal-key encrypted using one of the common keyYs; see below)
0x8F	0x1	Reserved
0x90	0x8	TicketID
0x98	0x4	ConsoleID
0x9C	0x8	TitleID
0xA4	0x2	Reserved
0xA6	0x2	Ticket title version
0xA8	0x8	Reserved
0xB0	0x1	License Type
0xB1	0x1	Index to the common keyY used for this ticket, usually 0x1 for retail system titles; see below.
0xB2	0x2A	Reserved
0xDC	0x4	eShop Account ID?
0xE0	0x1	Reserved
0xE1	0x1	Audit
0xE2	0x42	Reserved
0x124	0x40	Limits
0x164	X	Content Index
*/
#include <3ds.h>
typedef struct
{
	u8 signatureIssuer[0x40];
	u8 pubKey[0x3C];
	u8 version;
	u8 ca_crl_version;
	u8 signer_crl_version;
	u8 titleKey[0x10];
	u8 reserved1;
	u8 ticketId[8];
	u8 consoleId[4];
	u8 titleId[8];
    u8 reserved2[2];
    u8 ticketTitleVersion[2];
	u8 reserved3[8];
	u8 licenseType;
	u8 keyYIdx;
	u8 reserved4[0x2A]; // Zero for CXI Content0
	u8 accountId[0x04];
	u8 reserved5;
	u8 audit;
	u8 reserved6[0x42];
	u8 limits[0x40];
    u8 contentIndex[0xAC];
} __attribute__((__packed__)) 
sTicketHeader;