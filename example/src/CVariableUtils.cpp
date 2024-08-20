#include "CVariableUtils.h"

void CVARUtils::setString(void *pData, const std::string& value) {
    *(std::string*)pData = value;
}

std::string CVARUtils::getString(void *pData) {
    return *(std::string*)pData;
}

void CVARUtils::setUnsignedCharBitwiseFlag(void *pData, const std::string& value) {
    try {
        auto data = *(UnsignedCharBitwiseFlag*)pData;
        if (std::stoi(value) <= 0) data.flags &= ~(data.bit);
        else data.flags |= data.bit;
    } catch (...) {return;}
}

std::string CVARUtils::getUnsignedCharBitwiseFlag(void *pData) {
    auto data = *(UnsignedCharBitwiseFlag*)pData;
    return std::to_string((data.flags & data.bit) == data.bit);
}