#include <iostream>
#include <unordered_map>

#include "HayBCMD.h"
#include <vector>

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

static void print(HayBCMD::OutputLevel level, const std::string &string) {
    std::cout << outputLevelToString(level) << ": " << string;
}

int main()
{
    HayBCMD::Output::setPrintFunction(print);

    std::unordered_map<std::string, std::string> variables{};
    HayBCMD::BaseCommands::init(&variables);

    HayBCMD::CVARStorage::cvar("cl_auto_respawn", true, "whether respawn automatically or not");
    HayBCMD::CVARStorage::cvar("cl_respawn_message", std::string("You died. Respawn?"), "");
    HayBCMD::CVARStorage::cvar("cl_respawn_cooldown", 69.420f, "cooldown to respawn");
    
    std::string text = HayBCMD::formatString(
        "This is a {} text that accepts {}+ types\nHere are some examples:\nbool: {}/{}\ndouble {}\nfloat {}F\nlong long {}LL\n",
        "formatted", 5, true, false, (double)1.23456789, 3.1415f, LONG_LONG_MAX);
    
    HayBCMD::Output::print(HayBCMD::OutputLevel::DEFAULT, text);
    HayBCMD::Output::printf(HayBCMD::OutputLevel::DEFAULT,
        "This is a text being printed form printf method that is just a normal print with formatString with it.\n{} {} {} {} {}",
        1, 3.2f, 4.5, true, "ez banana\n");

    {
        std::string input = "alias example \"echo \\\"Auto Respawn: \\\\\\\"$cl_auto_respawn\\\\\\\"\\\"; echo \\\"Respawn Message: \\\\\\\"$cl_respawn_message\\\\\\\"\\\"; echo \\\"Respawn Cooldown: \\\\\\\"$cl_respawn_cooldown\\\\\\\"\\\"\"; example";
        
        HayBCMD::Output::print(HayBCMD::OutputLevel::DEFAULT, input+'\n');
        
        HayBCMD::Lexer *lexer = new HayBCMD::Lexer(input);
        HayBCMD::Parser(lexer, variables).parse();

        delete lexer;
        lexer = nullptr;
    }

    // Continue looping until the user enters "quit"
    std::string input;
    while (true) {
        std::getline(std::cin, input);

        if (input == "quit")
            break;

        HayBCMD::Lexer lexer = input;
        HayBCMD::Parser(&lexer, variables).parse();
    }
    
    return 0;
}