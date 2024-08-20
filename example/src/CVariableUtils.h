#pragma once

#include <string>

namespace CVARUtils {
    struct UnsignedCharBitwiseFlag {
        unsigned char& flags;
        unsigned char bit;

        UnsignedCharBitwiseFlag(unsigned char& flags, unsigned char bit) : flags(flags), bit(bit) {}
    };

    void setString(void *pData, const std::string& value);
    std::string getString(void *pData);

    void setUnsignedCharBitwiseFlag(void *pData, const std::string& value);
    std::string getUnsignedCharBitwiseFlag(void *pData);
}