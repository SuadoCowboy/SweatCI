/*
 * MIT License
 *
 * Copyright (c) 2024 Lucca Rieffel Silva, also as Suado Cowboy
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HayBCMD.h"

#include <stdexcept>
#include <regex>
#include <sstream>
#include <algorithm>

namespace HayBCMD {
    std::string tokenTypeToString(const TokenType& type) {
        switch (type) {
        case STRING:
            return "STRING";
        case COMMAND:
            return "COMMAND";
        case _EOF:
            return "_EOF";
        case EOS:
            return "EOS";
        }

        return "UNKNOWN";
    }

    Data::Data(int i) : type(Type::INT), i(i) {}
    Data::Data(double d) : type(Type::DOUBLE), d(d) {}
    Data::Data(float f) : type(Type::FLOAT), f(f) {}
    Data::Data(bool b) : type(Type::BOOL), b(b) {}
    Data::Data(std::string s) : type(Type::STRING), s(s) {}
    Data::Data(const char* _s) : type(Type::STRING) {
        s = _s;
    }
    Data::Data(char _s) : type(Type::STRING) {
        s = _s;
    }

    std::string Data::toString() {
        if (type == Type::INT)
            return std::to_string(i);
        else if (type == Type::DOUBLE)
            return std::to_string(d);
        else if (type == Type::FLOAT)
            return std::to_string(f);
        else if (type == Type::BOOL)
            return b? "true" : "false";
        else
            return s;
    }

    std::string formatString(std::string format, const std::vector<Data>& args) {
        for (auto arg : args) {
            size_t idx = format.find("{}");

            if (idx == std::string::npos)
                throw std::runtime_error("Too much arguments for little format variables");
            
            std::stringstream oss;
            oss << format.substr(0, idx) << arg.toString() << format.substr(idx+2);
            format = oss.str();
        }

        return format;
    }

    Token::Token() : type(NOTHING), value("") {}
    Token::~Token() {}

    Token::Token(const TokenType& _type, const std::string& _value) : type(_type), value(_value) {}

    Token& Token::operator=(const Token& other) {
        type = other.type;
        value = other.value;

        return *this;
    }

    const TokenType& Token::getType() const {
        return type;
    }

    const std::string& Token::getValue() {
        return value;
    }

    std::string Token::string() const {
        return "Token(" + tokenTypeToString(type) + ", \"" + value + "\")";
    }

    void Output::setPrintFunction(PrintFunction _printFunc) {
        printFunc = _printFunc;
    }

    void Output::printf(const std::string& format, const std::vector<Data>& args) {
        print(formatString(format, args));
    }

    void Output::print(const std::string& str) {
        printFunc(str);
    }

    PrintFunction Output::printFunc;

    void Command::addCommand(Command* pCommand) {
        for (const auto& c : commands) {
            if (c.name == pCommand->name) {
                Output::printf("ERROR: Command with name \"{s}\" already exists\n", {{pCommand->name}});
                return;
            }
        }
        commands.push_back(*pCommand);
    }

    Command::Command(const std::string& name, int minArgs, int maxArgs, CommandCall commandCallFunc, const std::string& usage)
        : name(name), minArgs(minArgs), maxArgs(maxArgs), commandCallFunc(commandCallFunc), usage(usage) {
        addCommand(this);
    }

    Command* Command::getCommand(const std::string& name, bool printError) {
        for (auto& command : commands)
            if (command.name == name) return &command;

        if (printError)
            Output::print("unknown command \"" + name + "\"\n");

        return nullptr;
    }

    bool Command::deleteCommand(const std::string& commandName) {
        for (size_t i = 0; i < commands.size(); ++i) {
            if (commands[i].name == commandName) {
                commands.erase(commands.begin() + i);
                return true;
            }
        }

        return false;
    }

    const std::vector<Command>& Command::getCommands() {
        return commands;
    }

    void Command::printUsage(const Command& command) {
        Output::print(command.name + ' ' + command.usage + '\n');
    }

    void Command::run(const std::vector<std::string>& args) {
        (*commandCallFunc)(this, args);
    }

    std::vector<Command> Command::commands;

    void BaseCommands::init(std::unordered_map<std::string, std::string>* _variables) {
        variables = _variables;

        // Add commands
        Command("help", 0, 1, (CommandCall)help, "<command?> - shows a list of commands usages or the usage of a specific command");
        Command("echo", 1, 1, (CommandCall)echo, "<message> - echoes a message to the console");
        Command("alias", 1, 2, (CommandCall)alias, "<var> <commands?> - creates/deletes variables");
        Command("variables", 0, 0, (CommandCall)getVariables, "- list of variables");
        Command("variable", 1, 1, (CommandCall)variable, "- shows variable value");
        Command("incrementvar", 4, 4, (CommandCall)incrementvar, "<var> <minValue> <maxValue> <delta> - increments the value of a variable");
    }

    void BaseCommands::help(Command* _pCommand, const std::vector<std::string>& args) {
        if (args.size() == 1) {
            // Print usage for a specific command
            Command* command = Command::getCommand(args[0], true);
            if (command != nullptr)
                Command::printUsage(*(Command*)command);
            return;
        }

        // Print usage for all commands
        for (const auto& command : Command::getCommands()) {
            Command::printUsage(command);
        }
    }

    void BaseCommands::echo(Command* _pCommand, const std::vector<std::string>& args) {
        std::string message;
        for (const auto& arg : args) {
            message += arg;
        }
        Output::print(message + '\n');
    }

    void BaseCommands::alias(Command* _pCommand, const std::vector<std::string>& args) {
        if (args.size() == 1) {
            variables->erase(args[0]);
            return;
        }

        if (Command::getCommand(args[0], false)) {
            Output::print("varName is a command name, therefore this variable can not be created\n");
            return;
        }

        std::regex whitespace_regex("\\S+");
        if (!std::regex_match(args[0], whitespace_regex)) {
            Output::print("variable name can not have whitespace.\n");
            return;
        }

        (*variables)[args[0]] = args[1];
    }

    void BaseCommands::getVariables(Command* _pCommand, const std::vector<std::string>& args) {
        std::string output;
        int count = 0;

        for (const auto& pair : *variables) {
            output += pair.first + " = \"" + pair.second + "\"\n";
            count++;
        }

        std::stringstream out;
        out << "amount of variables: " << count << '\n';
        if (!output.empty()) {
            output.pop_back(); // Remove trailing newline
            out << output << '\n';
        }

        Output::print(out.str());
    }

    void BaseCommands::variable(Command* _pCommand, const std::vector<std::string>& args) {
        const std::string& key = args[0];
        auto it = variables->find(key);
        if (it == variables->end()) {
            Output::print("variable \"" + key + "\" does not exist\n");
            return;
        }

        Output::print(key + " = \"" + it->second + "\"\n");
    }

    void BaseCommands::incrementvar(Command* _pCommand, const std::vector<std::string>& args) {
        const std::string& variable = args[0];
        double minValue, maxValue, delta;

        try {
            minValue = std::stod(args[1]);
            maxValue = std::stod(args[2]);
            delta = std::stod(args[3]);
        }
        catch (...) {
            Output::print("one of the variables is not a number");
            return;
        }

        if (minValue > maxValue) {
            Output::print("minValue is higher than maxValue");
            return;
        }

        auto it = variables->find(variable);
        if (it == variables->end()) {
            Output::print("unknown variable \"" + variable + "\"\n");
            return;
        }

        double variableValue;
        try {
            variableValue = std::stod(it->second);
        }
        catch (...) {
            Output::print("variable value \"" + it->second + "\" is not a number");
            return;
        }

        variableValue += delta;
        if (variableValue > maxValue)
            variableValue = minValue;

        else if (variableValue < minValue)
            variableValue = maxValue;

        (*variables)[variable] = std::to_string(variableValue);
    }

    std::unordered_map<std::string, std::string>* BaseCommands::variables;

    Lexer::Lexer(const std::string& _input) : position(0), input(_input) {}

    Token Lexer::nextToken() {
        if (position >= input.length()) {
            lastToken = Token(TokenType::_EOF, "");
            return lastToken;
        }

        char currentChar = input[position];
        while (std::isspace(currentChar)) {
            position++;

            if (position >= input.length()) {
                lastToken = Token(TokenType::_EOF, "");
                return lastToken;
            }

            currentChar = input[position];
        }

        if (input[position] == ';') {
            position++;
            lastToken = Token(TokenType::EOS, ";");
            return lastToken;
        }

        lastToken = parseToken();
        return lastToken;
    }

    bool Lexer::isCommand(const std::string& commandName) {
        for (const auto& command : Command::getCommands()) {

            if (command.name == commandName)
                return true;
        }

        return false;
    }

    Token Lexer::parseToken() {
        if (input[position] == '"')
            return parseString();

        std::string tokenValue;
        while (position < input.length() && !std::isspace(input[position]) && input[position] != ';') {
            tokenValue += input[position];
            position++;
        }

        if (isCommand(tokenValue) && (lastToken.getType() == TokenType::NOTHING || lastToken.getType() != TokenType::COMMAND))
            return Token(TokenType::COMMAND, tokenValue);
        else
            return Token(TokenType::STRING, tokenValue);
    }

    Token Lexer::parseString() {
        std::string tokenValue = "";

        position++; // Skip the first double quote

        while (position < input.length() && input[position] != '"') {
            // escape '\\'
            if (input[position] == '\\' && position + 1 < input.length() && input[position+1] == '\\')
                position++;
            
            // escape '"'
            else if (input[position] == '\\' && position + 1 < input.length() && input[position+1] == '"')
                position++;

            tokenValue += input[position];
            position++;
        }

        if (input[position] == '"')
            position++; // Skip the last double quote if exists

        return Token(TokenType::STRING, tokenValue);
    }

    void CVARStorage::cvar(const std::string& name, bool value, const std::string& usage) {
            boolCvars[name] = value;

            Command(name, 0, 1, (CommandCall)asCommand,
                    formatString("(boolean) - {}", {{usage}}));
        }
        
    void CVARStorage::cvar(const std::string& name, float value, const std::string& usage) {
        floatCvars[name] = value;

        Command(name, 0, 1, (CommandCall)asCommand,
                formatString("(float) - {}", {{usage}}));
    }
    
    void CVARStorage::cvar(const std::string& name, const std::string& value, const std::string& usage) {
        stringCvars[name] = value;

        Command(name, 0, 1, (CommandCall)asCommand,
                formatString("(string) - {}", {{usage}}));
    }

    void CVARStorage::cvar(const std::string& name, const char* value, const std::string& usage) {
        stringCvars[name] = {value};

        Command(name, 0, 1, (CommandCall)asCommand,
                formatString("(string) - {}", {{usage}}));
    }

    void CVARStorage::setCvar(const std::string& name, bool value) {
        if (boolCvars.count(name) == 0) {
            Output::printf("ERROR: tried to change value of non-existent boolean CVAR \"{}\"", {{name}});
            return;
        }

        boolCvars[name] = value;
    }

    void CVARStorage::setCvar(const std::string& name, float value) {
        if (floatCvars.count(name) == 0) {
            Output::printf("ERROR: tried to change value of non-existent float CVAR \"{}\"", {{name}});
            return;
        }

        floatCvars[name] = value;
    }

    void CVARStorage::setCvar(const std::string& name, const std::string& value) {
        if (stringCvars.count(name) == 0) {
            Output::printf("ERROR: tried to change value of non-existent string CVAR \"{}\"", {{name}});
            return;
        }

        stringCvars[name] = value;
    }

    void CVARStorage::setCvar(const std::string& name, const char* value) {
        if (stringCvars.count(name) == 0) {
            Output::printf("ERROR: tried to change value of non-existent string CVAR \"{}\"", {{name}});
            return;
        }

        stringCvars[name] = {value};
    }

    // Searches for the CVAR and returns it to a buffer
    // @return false if could not get cvar
    bool CVARStorage::getCvar(const std::string& name, bool& buf) {
        for (auto cvar : boolCvars)
            if (cvar.first == name) {buf = cvar.second; return true;}
        return false;
    }

    // Searches for the CVAR and returns it to a buffer
    // @return false if could not get cvar
    bool CVARStorage::getCvar(const std::string& name, std::string& buf) {
        for (auto cvar : stringCvars)
            if (cvar.first == name) {buf = cvar.second; return true;}
        return false;
    }

    // Searches for the CVAR and returns it to a buffer
    // @return false if could not get cvar
    bool CVARStorage::getCvar(const std::string& name, float& buf) {
        for (auto cvar : floatCvars)
            if (cvar.first == name) {buf = cvar.second; return true;}
        return false;
    }
    
    void CVARStorage::asCommand(Command* pCommand, const std::vector<std::string>& args) {
        char type = pCommand->usage.at(1); // usage = "(string/float/boolean) [...]"; this gets the first char after '('
        
        // if should print to output
        if (args.size() == 0) {
            if (type == 'b') {
                bool buf;
                getCvar(pCommand->name, buf);
                Output::printf("{}\n", {{buf}});
            
            } else if (type == 'f') {
                float buf;
                getCvar(pCommand->name, buf);
                Output::printf("{}\n", {{buf}});
            
            } else if (type == 's') {
                std::string buf;
                getCvar(pCommand->name, buf);
                Output::printf("{}\n", {{buf}});
            }
            return;
        }

        // if should set value
        if (type == 'b')
            try {
                boolCvars[pCommand->name] = (bool)std::stoi(args[0]);
            } catch (...) {
                Command::printUsage(*pCommand);
            }
        
        else if (type == 'f')
            try {
                floatCvars[pCommand->name] = std::stof(args[0]);
            } catch (...) {
                Command::printUsage(*pCommand);
            }
        
        else if (type == 's')
            try {
                stringCvars[pCommand->name] = args[0];
            } catch (...) {
                Command::printUsage(*pCommand);
            }
    }

    char CVARStorage::getCvarType(const std::string& name) {
        bool bBuf;
        std::string sBuf;
        float fBuf;
        if (getCvar(name, bBuf)) return 'b';
        if (getCvar(name, sBuf)) return 's';
        if (getCvar(name, fBuf)) return 'f';

        return 'n';
    }

    std::unordered_map<std::string, bool> CVARStorage::boolCvars;
    std::unordered_map<std::string, float> CVARStorage::floatCvars;
    std::unordered_map<std::string, std::string> CVARStorage::stringCvars;

    Parser::Parser(Lexer* lexer, std::unordered_map<std::string, std::string>& variables) : lexer(lexer), variables(variables) {
        advance();
    }

    void Parser::advance() {
        currentToken = lexer->nextToken();
    }

    void Parser::advanceUntil(const std::vector<TokenType>& tokenTypes) {
        advance(); // always skip the first one

        // checks if EOF is reached because if not, it would run forever
        while (std::find(tokenTypes.begin(), tokenTypes.end(), currentToken.getType()) == tokenTypes.end() && currentToken.getType() != TokenType::_EOF)
            advance();
    }

    std::vector<std::string> Parser::getArguments() {
        std::vector<std::string> arguments;

        while (currentToken.getType() != TokenType::_EOF && currentToken.getType() != TokenType::EOS) {
            // yes... it's just appending command type
            if (currentToken.getType() == TokenType::COMMAND)
                arguments.push_back(currentToken.getValue());
            
            else if (currentToken.getType() == TokenType::STRING) {
                std::string result = "";
                size_t position = 0;
                
                while (position < currentToken.getValue().length()) {
                    if (currentToken.getValue()[position] == '$') { // if is variable or cvariable
                        // get variable name
                        
                        position++; // skip '$'

                        std::string variable = "";
                        while (position < currentToken.getValue().length() && currentToken.getValue()[position] != ' ' && currentToken.getValue()[position] != '"') {
                            variable += currentToken.getValue()[position];
                            position++;
                        }
                        
                        auto it = variables.find(variable);
                        if (it != variables.end()) {
                            result += it->second; // add variable
                        
                        } else { // search in cvars
                            char cvarType = CVARStorage::getCvarType(variable);
                            if (cvarType == 'b') {
                                bool buf;
                                CVARStorage::getCvar(variable, buf);
                                result += std::to_string(buf);
                            
                            } else if (cvarType == 'f') {
                                float buf;
                                CVARStorage::getCvar(variable, buf);
                                result += std::to_string(buf);
                            
                            } else if (cvarType == 's') {
                                std::string buf;
                                CVARStorage::getCvar(variable, buf);
                                result += buf;
                            
                            } else
                                result += "$" + variable; // or else just add with the $
                        }
                    }

                    if (currentToken.getValue()[position] == '\\' && position+1 < currentToken.getValue().length() && currentToken.getValue()[position+1] == '$')
                        position++;

                    result += currentToken.getValue()[position];
                    position++;
                }

                arguments.push_back(result);
            }

            advance();
        }

        return arguments;
    }

    std::string Parser::getVariableFromCurrentTokenValue() {
        auto it = variables.find(currentToken.getValue());
        if (it != variables.end())
            return it->second;
        return "";
    }

    void Parser::handleCommandToken() {
        std::string commandString = currentToken.getValue();

        Command* command = Command::getCommand(commandString, true);
        if (command == nullptr)
            return;

        advance(); // skips the command token

        std::vector<std::string> arguments = getArguments();

        // make it include whitespaces in that case
        if (command->maxArgs == 1 && !arguments.empty()) {
            std::string stringBuilder;
            for (const auto& argument : arguments) {
                stringBuilder += argument + " ";
            }
            stringBuilder.pop_back(); // remove last space
            arguments.clear();
            arguments.push_back(stringBuilder);
        }

        // checks if arguments size is within the allowed
        if (arguments.size() > command->maxArgs || arguments.size() < command->minArgs) {
            Command::printUsage(*command);
            if (!arguments.empty())
                Output::print("arguments size must be within range [" + std::to_string(command->minArgs) + "," + std::to_string(command->maxArgs) + "], but size is " + std::to_string(arguments.size()) + '\n');
            return;
        }

        command->run(arguments);
    }

    void Parser::handleAliasLexer(const std::string& input) {
        std::vector<Lexer*> tempLexers;
        tempLexers.push_back(lexer);

        lexer = new Lexer(input);
        advance();

        while (currentToken.getType() != TokenType::_EOF) {
            std::string variable = getVariableFromCurrentTokenValue();

            if (variable != "") {
                tempLexers.push_back(lexer);

                lexer = new Lexer(variable);
            }

            else if (currentToken.getType() == TokenType::COMMAND)
                handleCommandToken();

            else if (currentToken.getType() == TokenType::STRING) {
                Output::printUnknownCommand(currentToken.getValue());
                advanceUntil({ TokenType::EOS });
            }

            advance();

            if (tempLexers.size() == aliasMaxCalls) {
                delete lexer;

                for (size_t i = 1; i < tempLexers.size(); ++i) {
                    delete tempLexers[i];
                }

                break;
            }

            while (currentToken.getType() == TokenType::_EOF && tempLexers.size() > 1) {
                delete lexer;

                lexer = tempLexers.back();
                advance();

                tempLexers.pop_back();
            }
        }

        lexer = tempLexers[0];
        advance();
    }

    void Parser::parse() {
        while (currentToken.getType() != TokenType::_EOF) {
            std::string variableValue = getVariableFromCurrentTokenValue();

            if (!variableValue.empty())
                handleAliasLexer(variableValue);

            else if (currentToken.getType() == TokenType::COMMAND)
                handleCommandToken();

            else if (currentToken.getType() == TokenType::STRING) {
                Output::printUnknownCommand(currentToken.getValue());
                advanceUntil({ TokenType::EOS });
            }

            advance();
        }
    }

    unsigned int Parser::aliasMaxCalls = 50000;
}