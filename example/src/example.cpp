#include <iostream>
#include <unordered_map>

#include "HayBCMD.h"
#include <vector>

static void print(const std::string& string) {
    std::cout << string;
}

int main()
{
    HayBCMD::PrintFunction printFunc = &print;
    HayBCMD::Output::setPrintFunction(printFunc);

    std::unordered_map<std::string, std::string> variables{};
    HayBCMD::BaseCommands::init(&variables);

    HayBCMD::CVARStorage::cvar("cl_auto_respawn", true, "whether respawn automatically or not");
    HayBCMD::CVARStorage::cvar("cl_respawn_message", "You died. Respawn?", "");
    HayBCMD::CVARStorage::cvar("cl_respawn_cooldown", 69.420, "cooldown to respawn");
    
    {
        std::string input = "alias example \"echo \\\"Auto Respawn: \\\\\\\"$cl_auto_respawn\\\\\\\"\\\"; echo \\\"Respawn Message: \\\\\\\"$cl_respawn_message\\\\\\\"\\\"; echo \\\"Respawn Cooldown: \\\\\\\"$cl_respawn_cooldown\\\\\\\"\\\"\"; example";
        
        HayBCMD::Output::print(input+'\n');
        
        HayBCMD::Lexer* lexer = new HayBCMD::Lexer(input);
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

        HayBCMD::Lexer* lexer = new HayBCMD::Lexer(input);
        HayBCMD::Parser(lexer, variables).parse();

        delete lexer;
        lexer = nullptr;
    }
    
    return 0;
}