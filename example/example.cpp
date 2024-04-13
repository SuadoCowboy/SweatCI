#include <iostream>
#include <unordered_map>

#include "HayBCMD.h"
#include <vector>

static void print(const std::string& string) {
    std::cout << string;
}

// for now this is the way I found to create CVARs... it's not the best way but that's what i have for now.
// either create a ConsoleVariable class, or a set/get function for each variable(whitch i don't like very much)
static bool tf_fuck = true;
static void CVAR_TF_FUCK_run(const HayBCMD::Command& command, const std::vector<std::string>& args) {
    if (args.size() == 0) {
        HayBCMD::Output::print(std::to_string(tf_fuck)+'\n');
        return;
    }

    try {
        tf_fuck = std::stoi(args[0]);
    }
    catch (...) {
        HayBCMD::Command::printUsage(command);
    }
}

int main()
{
    HayBCMD::Output::setPrintFunction(&print);

    std::unordered_map<std::string, std::string> variables{};
    HayBCMD::BaseCommands::init(&variables);

    HayBCMD::Command("tf_fuck", 0, 1, CVAR_TF_FUCK_run, "- type: boolean - 1/0");

    // Continue looping until the user enters "exit"
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