#include <iostream>
#include <unordered_map>
#include <vector>

#include "SweatCI.h"

std::unordered_map<std::string, std::string> variables;

int test1 = 690420;
float test2 = 32.64f;
short test3 = 25059;
unsigned short test4 = 53021;
unsigned char test5 = 69;
bool test6 = false;
std::string test7 = "Jane Doe";

static std::string outputLevelToString(const SweatCI::OutputLevel &level) {
    switch (level) {
    case SweatCI::OutputLevel::DEFAULT:
        return "DEFAULT";
    
    case SweatCI::OutputLevel::ECHO:
        return "ECHO";
    
    case SweatCI::OutputLevel::WARNING:
        return "WARNING";
    
    case SweatCI::OutputLevel::ERROR:
        return "ERROR";
    };

    return "UNKNOWN";
}

static void print(void*, const SweatCI::OutputLevel& level, const std::string& message) {
    std::cout << outputLevelToString(level) << ": " << message;
}

bool running = true;
static void setRunningToFalse(void*, SweatCI::Command&, const std::vector<std::string>&) {
    running = false;
}

static void init() {
    SweatCI::Output::setPrintFunction(nullptr, print);
    SweatCI::BaseCommands::init(&variables);

    SweatCI::Command("quit", 0, 0, setRunningToFalse, "- quits");

    SweatCI::CVARStorage::setCvar("t_int",
        &test1,
        SweatCI::Utils::Cvar::setInteger,
        SweatCI::Utils::Cvar::getInteger,
        "- int"
    );

    SweatCI::CVARStorage::setCvar("t_float",
        &test2,
        SweatCI::Utils::Cvar::setFloat,
        SweatCI::Utils::Cvar::getFloat,
        "- float"
    );

    SweatCI::CVARStorage::setCvar("t_short",
        &test3,
        SweatCI::Utils::Cvar::setShort,
        SweatCI::Utils::Cvar::getShort,
        "- short"
    );

    SweatCI::CVARStorage::setCvar("t_ushort",
        &test4,
        SweatCI::Utils::Cvar::setUnsignedShort,
        SweatCI::Utils::Cvar::getUnsignedShort,
        "- unsigned short"
    );

    SweatCI::CVARStorage::setCvar("t_uchar",
        &test5,
        SweatCI::Utils::Cvar::setUnsignedChar,
        SweatCI::Utils::Cvar::getUnsignedChar,
        "- unsigned char"
    );

    SweatCI::CVARStorage::setCvar("t_bool",
        &test6,
        SweatCI::Utils::Cvar::setBoolean,
        SweatCI::Utils::Cvar::getBoolean,
        "- bool"
    );

    SweatCI::CVARStorage::setCvar("t_string",
        &test7,
        SweatCI::Utils::Cvar::setString,
        SweatCI::Utils::Cvar::getString,
        "- string");
}

int main()
{
    variables = {};
    init();

    while (running) {
        std::string input;
        std::getline(std::cin, input);

        SweatCI::Lexer lexer = input;
        SweatCI::Parser(&lexer, &variables).parse();
        
        SweatCI::handleLoopAliasesRunning(&variables);
    }
}