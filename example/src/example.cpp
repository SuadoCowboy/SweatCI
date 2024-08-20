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

void print(const HayBCMD::OutputLevel& level, const std::string& message) {
    std::cout << outputLevelToString(level) << ": " << message;
}

bool running = true;
void setRunningToFalse(void*, const std::vector<std::string>&) {
    running = false;
}

int main()
{
    HayBCMD::Output::setPrintFunction(print);

    std::unordered_map<std::string, std::string> variables{};
    HayBCMD::BaseCommands::init(&variables);
    
    HayBCMD::Command("quit", 0, 0, setRunningToFalse, "- quits");

    std::string userName = "Jane Doe";
    HayBCMD::CVARStorage::setCvar("user_name",
        &userName,
        HayBCMD::CVARUtils::setString,
        HayBCMD::CVARUtils::getString,
        "- the name of the user :P");

    unsigned char flags = 0;
    
    HayBCMD::CVARUtils::UnsignedCharBitwiseFlag godmodeFlag {flags, 1};
    HayBCMD::CVARStorage::setCvar("godmode",
        &godmodeFlag,
        HayBCMD::CVARUtils::setUnsignedCharBitwiseFlag,
        HayBCMD::CVARUtils::getUnsignedCharBitwiseFlag,
        "- 1/0, whether is godmode or not"
    );

    HayBCMD::CVARUtils::UnsignedCharBitwiseFlag sendDieMessageFlag {flags, 2};
    HayBCMD::CVARStorage::setCvar("send_die_message",
        &sendDieMessageFlag,
        HayBCMD::CVARUtils::setUnsignedCharBitwiseFlag,
        HayBCMD::CVARUtils::getUnsignedCharBitwiseFlag,
        "- 1/0, whether send die message or not"
    );

    HayBCMD::Command("+test", 0, 0, [](void*, const std::vector<std::string>&){
        HayBCMD::Output::print(HayBCMD::ECHO, ":)\n");
    }, ":)");

    HayBCMD::Command("-test", 0, 0, [](void*, const std::vector<std::string>&){
        HayBCMD::Output::print(HayBCMD::ECHO, ":(\n");
    }, ":)");

    while (running) {
        std::string input;
        std::getline(std::cin, input);

        HayBCMD::Lexer lexer = input;
        HayBCMD::Parser(&lexer, variables).parse();
        
        HayBCMD::handleLoopAliasesRunning(variables);
    }
}