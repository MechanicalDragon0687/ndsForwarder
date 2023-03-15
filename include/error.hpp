#pragma once
#include <string>

#define ERROR_SUCCESS 0x0
#define ERROR_SRL       0x010000
#define ERROR_TEMPLATE  0x020000
#define ERROR_INSTALL   0x030000


#define ERROR_CRC                       ERROR_SRL|0x000100
#define ERROR_PATH                      ERROR_SRL|0x000200
#define ERROR_CRC_HEADER                (ERROR_CRC|0x01)
#define ERROR_CRC_BANNER_1              (ERROR_CRC|0x02)
#define ERROR_CRC_BANNER_2              (ERROR_CRC|0x03)
#define ERROR_CRC_BANNER_3              (ERROR_CRC|0x04)
#define ERROR_CRC_BANNER_ANIMATED       (ERROR_CRC|0x05)
#define ERROR_BANNERLOC                 ERROR_SRL|0x000300
#define ERROR_NOT_ENOUGH_SPACE          ERROR_INSTALL|0x000101

#define ERROR_TEMPLATE_PATH             ERROR_TEMPLATE|0x000100
#define ERROR_TEMPLATE_FWD_NOT_FOUND    ERROR_TEMPLATE|0x000200
#define ERROR_TEMPLATE_NDS_NOT_FOUND    ERROR_TEMPLATE|0x000300
#define ERROR_TEMPLATE_PARSE            ERROR_TEMPLATE|0x000400
class ReturnResult {
    public:
    u32 code;
    std::string message;
    bool isSuccess() { return this->code == ERROR_SUCCESS;}
    ReturnResult(u32 code, std::string message) {
        this->code = code;
        this->message = message;
    }
};