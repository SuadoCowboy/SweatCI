#include <iostream>
#include <unordered_map>

#include "HayBCMD.h"
#include <vector>

static void print(const std::string& string) {
    std::cout << string;
}

class CVARStorage {
public:
    static void cvar(const std::string& name, bool value, const std::string& usage) {
        boolCvars[name] = value;

        HayBCMD::Command(name, 0, 1, (HayBCMD::CommandCall)asCommand,
                HayBCMD::formatString("(boolean) - {}", {{usage}}));
    }
    
    static void cvar(const std::string& name, float value, const std::string& usage) {
        floatCvars[name] = value;

        HayBCMD::Command(name, 0, 1, (HayBCMD::CommandCall)asCommand,
                HayBCMD::formatString("(float) - {}", {{usage}}));
    }
    
    static void cvar(const std::string& name, const std::string& value, const std::string& usage) {
        stringCvars[name] = value;

        HayBCMD::Command(name, 0, 1, (HayBCMD::CommandCall)asCommand,
                HayBCMD::formatString("(string) - {}", {{usage}}));
    }

    static void cvar(const std::string& name, const char* value, const std::string& usage) {
        stringCvars[name] = {value};

        HayBCMD::Command(name, 0, 1, (HayBCMD::CommandCall)asCommand,
                HayBCMD::formatString("(string) - {}", {{usage}}));
    }

    static void setCvar(const std::string& name, bool value) {
        if (boolCvars.count(name) == 0) {
            HayBCMD::Output::printf("ERROR: tried to change value of non-existent boolean CVAR \"{}\"", {{name}});
            return;
        }

        boolCvars[name] = value;
    }

    static void setCvar(const std::string& name, float value) {
        if (floatCvars.count(name) == 0) {
            HayBCMD::Output::printf("ERROR: tried to change value of non-existent float CVAR \"{}\"", {{name}});
            return;
        }

        floatCvars[name] = value;
    }

    static void setCvar(const std::string& name, const std::string& value) {
        if (stringCvars.count(name) == 0) {
            HayBCMD::Output::printf("ERROR: tried to change value of non-existent string CVAR \"{}\"", {{name}});
            return;
        }

        stringCvars[name] = value;
    }

    static void setCvar(const std::string& name, const char* value) {
        if (stringCvars.count(name) == 0) {
            HayBCMD::Output::printf("ERROR: tried to change value of non-existent string CVAR \"{}\"", {{name}});
            return;
        }

        stringCvars[name] = {value};
    }

    // Searches for the CVAR and returns it to a buffer
    // @return false if could not get cvar
    static bool getCvar(const std::string& name, bool& buf) {
        for (auto cvar : boolCvars)
            if (cvar.first == name) {buf = cvar.second; return true;}
        return false;
    }

    // Searches for the CVAR and returns it to a buffer
    // @return false if could not get cvar
    static bool getCvar(const std::string& name, std::string& buf) {
        for (auto cvar : stringCvars)
            if (cvar.first == name) {buf = cvar.second; return true;}
        return false;
    }

    // Searches for the CVAR and returns it to a buffer
    // @return false if could not get cvar
    static bool getCvar(const std::string& name, float& buf) {
        for (auto cvar : floatCvars)
            if (cvar.first == name) {buf = cvar.second; return true;}
        return false;
    }

private:
    static std::unordered_map<std::string, bool> boolCvars;
    static std::unordered_map<std::string, float> floatCvars;
    static std::unordered_map<std::string, std::string> stringCvars;
    
    static void asCommand(HayBCMD::Command* pCommand, const std::vector<std::string>& args) {
        char type = pCommand->usage.at(1); // usage = "(string/float/boolean) [...]"; this gets the first char after '('
        
        // if should print to output
        if (args.size() == 0) {
            if (type == 'b') {
                bool buf;
                getCvar(pCommand->name, buf);
                HayBCMD::Output::printf("{}\n", {{buf}});
            
            } else if (type == 'f') {
                float buf;
                getCvar(pCommand->name, buf);
                HayBCMD::Output::printf("{}\n", {{buf}});
            
            } else if (type == 's') {
                std::string buf;
                getCvar(pCommand->name, buf);
                HayBCMD::Output::printf("{}\n", {{buf}});
            }
            return;
        }

        // if should set value
        if (type == 'b')
            try {
                boolCvars[pCommand->name] = (bool)std::stoi(args[0]);
            } catch (...) {
                HayBCMD::Command::printUsage(*pCommand);
            }
        
        else if (type == 'f')
            try {
                floatCvars[pCommand->name] = std::stof(args[0]);
            } catch (...) {
                HayBCMD::Command::printUsage(*pCommand);
            }
        
        else if (type == 's')
            try {
                stringCvars[pCommand->name] = args[0];
            } catch (...) {
                HayBCMD::Command::printUsage(*pCommand);
            }
    }
};

std::unordered_map<std::string, bool> CVARStorage::boolCvars;
std::unordered_map<std::string, float> CVARStorage::floatCvars;
std::unordered_map<std::string, std::string> CVARStorage::stringCvars;

int main()
{
    HayBCMD::PrintFunction printFunc = &print;
    HayBCMD::Output::setPrintFunction(printFunc);

    std::unordered_map<std::string, std::string> variables{};
    HayBCMD::BaseCommands::init(&variables);

    CVARStorage::cvar("cl_auto_respawn", true, "whether respawn automatically or not");
    CVARStorage::cvar("cl_respawn_message", "You died. Respawn?", "");
    CVARStorage::cvar("cl_respawn_cooldown", 69.420f, "cooldown to respawn");
    
    HayBCMD::Output::printf("Mixed types: {}, {}, {}, {}, {}\n", {{42}, {3.14}, {2.0f}, {"Ez banana"}, {false}});

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