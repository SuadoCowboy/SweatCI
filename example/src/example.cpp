#include <iostream>
#include <unordered_map>
#include <vector>

#include "HayBCMD.h"

static std::string outputLevelToString(const HayBCMD::OutputLevel &level) {
    switch (level) {
    case HayBCMD::OutputLevel::DEFAULT:
        return "DEFAULT";
    
    case HayBCMD::OutputLevel::ECHO:
        return "ECHO";
    
    case HayBCMD::OutputLevel::WARNING:
        return "WARNING";
    
    case HayBCMD::OutputLevel::ERROR:
        return "ERROR";
    };

    return "UNKNOWN";
}

static void print(void*, const HayBCMD::OutputLevel& level, const std::string& message) {
    std::cout << outputLevelToString(level) << ": " << message;
}

bool running = true;
void setRunningToFalse(void*, HayBCMD::Command&, const std::vector<std::string>&) {
    running = false;
}

int main()
{
    HayBCMD::Output::setPrintFunction(nullptr, print);

    std::unordered_map<std::string, std::string> variables{};
    HayBCMD::BaseCommands::init(&variables);
    
    HayBCMD::Command::create("quit", 0, 0, setRunningToFalse, "- quits");

    int test1 = 690420;
    HayBCMD::CVARStorage::setCvar("t_int",
        &test1,
        HayBCMD::CVARUtils::setInteger,
        HayBCMD::CVARUtils::getInteger,
        "- int"
    );

    float test2 = 32.64f;
    HayBCMD::CVARStorage::setCvar("t_float",
        &test2,
        HayBCMD::CVARUtils::setFloat,
        HayBCMD::CVARUtils::getFloat,
        "- float"
    );

    short test3 = 25059;
    HayBCMD::CVARStorage::setCvar("t_short",
        &test3,
        HayBCMD::CVARUtils::setShort,
        HayBCMD::CVARUtils::getShort,
        "- short"
    );

    unsigned short test4 = 53021;
    HayBCMD::CVARStorage::setCvar("t_ushort",
        &test4,
        HayBCMD::CVARUtils::setUnsignedShort,
        HayBCMD::CVARUtils::getUnsignedShort,
        "- unsigned short"
    );

    unsigned char test5 = 69;
    HayBCMD::CVARStorage::setCvar("t_uchar",
        &test5,
        HayBCMD::CVARUtils::setUnsignedChar,
        HayBCMD::CVARUtils::getUnsignedChar,
        "- unsigned char"
    );

    bool test6 = false;
    HayBCMD::CVARStorage::setCvar("t_bool",
        &test6,
        HayBCMD::CVARUtils::setBoolean,
        HayBCMD::CVARUtils::getBoolean,
        "- bool"
    );

    std::string test7 = "Jane Doe";
    HayBCMD::CVARStorage::setCvar("t_string",
        &test7,
        HayBCMD::CVARUtils::setString,
        HayBCMD::CVARUtils::getString,
        "- string");

    while (running) {
        std::string input;
        std::getline(std::cin, input);

        HayBCMD::Lexer lexer = input;
        HayBCMD::Parser(&lexer, variables).parse();
        
        HayBCMD::handleLoopAliasesRunning(variables);
    }
}