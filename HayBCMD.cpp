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

#include <regex>
#include <algorithm>
#include <fstream>

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
        case NOTHING:
            return "NOTHING";
        }

        return "UNKNOWN";
    }

    Token::Token() : type(NOTHING), value("") {}
    Token::~Token() {}

    Token::Token(const HayBCMD::Token& other) : type(other.type), value(other.value) {}

    Token::Token(const TokenType& type, const std::string& value) : type(type), value(value) {}

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

    void Output::print(const OutputLevel &level, const std::string& str) {
        printFunc(level, str);
    }

    void Output::printUnknownCommand(const std::string& command) {
        printf(OutputLevel::ERROR, "unknown command \"{}\"\n", command);
    }

    PrintFunction Output::printFunc;

    void Command::addCommand(Command& command) {
        for (const auto& c : commands) {
            if (c.name == command.name) {
                Output::printf(OutputLevel::ERROR, "Command with name \"{}\" already exists\n", command.name);
                return;
            }
        }
        commands.emplace_back(command);
    }

    Command::Command(const std::string& name, unsigned char minArgs, unsigned char maxArgs, CommandCall commandCallFunc, const std::string& usage, void* pData)
        : name(name), usage(usage), minArgs(minArgs), maxArgs(maxArgs),
        commandCallFunc(commandCallFunc), pData(pData) {
        addCommand(*this);
    }

    Command* Command::getCommand(const std::string& name, bool printError) {
        for (auto &command : commands)
            if (command.name == name) return &command;

        if (printError)
            Output::printUnknownCommand(name);

        return nullptr;
    }

    bool Command::deleteCommand(const std::string& commandName) {
        for (size_t i = 0; i < commands.size(); ++i)
            if (commands[i].name == commandName) {
                commands.erase(commands.begin() + i);
                return true;
            }

        return false;
    }

    const std::vector<Command>& Command::getCommands() {
        return commands;
    }

    void Command::printUsage(const Command &command) {
        Output::print(OutputLevel::WARNING, command.name + ' ' + command.usage + '\n');
    }

    void Command::clear() {
        commands.clear();
    }

    void Command::run(const std::vector<std::string>& args) {
        commandCallFunc(pData, args);
    }

    std::vector<Command> Command::commands;

    void BaseCommands::init(std::unordered_map<std::string, std::string> *_variables) {
        variables = _variables;

        // Add commands
        Command("help", 0, 1, help, "<command> - shows the usage of the command specified");
        Command("commands", 0, 0, commands, "- shows a list of commands with their usages");
        Command("echo", 1, 1, echo, "<message> - echoes a message to the console");
        Command("alias", 1, 2, alias, "<var> <commands?> - creates/deletes variables");
        Command("variables", 0, 0, getVariables, "- list of variables");
        Command("variable", 1, 1, variable, "- shows variable value");
        Command("incrementvar", 4, 4, incrementvar, "<var> <minValue> <maxValue> <delta> - increments the value of a variable");
        Command("exec", 1, 1, exec, "- executes a .cfg file that contains HayBCMD script");
    }

    void BaseCommands::help(void*, const std::vector<std::string>& args) {
        Command* pCommand = Command::getCommand("help", false);

        if (args.size() == 1) {
            // Print usage for a specific command
            Command* command = Command::getCommand(args[0], true);
            if (command != nullptr)
                Command::printUsage(*(Command*)command);
            return;
        } else
            Output::printf(OutputLevel::WARNING, "{} {} - see \"commands\" command to get a list of commands\n", pCommand->name, pCommand->usage);
    }

    void BaseCommands::commands(void*, const std::vector<std::string>&) {
        std::stringstream out;
        for (auto& command : Command::getCommands())
            out << command.name << " " << command.usage << "\n";

        Output::print(OutputLevel::ECHO, out.str());
    }

    void BaseCommands::echo(void*, const std::vector<std::string>& args) {
        std::string message;
        for (const auto &arg : args) {
            message += arg;
        }
        Output::print(OutputLevel::ECHO, message + '\n');
    }

    void BaseCommands::alias(void*, const std::vector<std::string>& args) {
        if (args.size() == 1 && variables->count(args[0]) != 0) {
            variables->erase(args[0]);
            if (args[0].front() == '!') {
                auto it = std::find(loopAliasesRunning.begin(), loopAliasesRunning.end(), args[0]);
                if (it != loopAliasesRunning.end())
                    loopAliasesRunning.erase(it);
            }

            else if (args[0].front() == '+') {
                auto it = std::find(toggleTypesRunning.begin(), toggleTypesRunning.end(), args[0].substr(1));
                if (it != toggleTypesRunning.end())
                    toggleTypesRunning.erase(it);
            }

            return;
        }

        if (Command::getCommand(args[0], false)) {
            Output::print(OutputLevel::ERROR, "varName is a command name, therefore this variable can not be created\n");
            return;
        }

        std::regex whitespace_regex("\\S+");
        if (!std::regex_match(args[0], whitespace_regex)) {
            Output::print(OutputLevel::ERROR, "variable name can not have whitespace.\n");
            return;
        }

        std::string negativeVarName = '-'+args[0].substr(1);
        if (args[0].front() == '+' && variables->count(negativeVarName) == 0) {
            (*variables)[negativeVarName] = " ";
        }

        (*variables)[args[0]] = args[1];
    }

    void BaseCommands::getVariables(void*, const std::vector<std::string>&) {
        std::stringstream out;

        out << "amount of variables: " << variables->size();
        for (const auto &pair : *variables)
            out << "\n" << pair.first << " = \"" << pair.second << "\"";

        Output::print(OutputLevel::ECHO, out.str()+'\n');
    }

    void BaseCommands::variable(void*, const std::vector<std::string>& args) {
        const std::string& key = args[0];
        auto it = variables->find(key);
        if (it == variables->end()) {
            Output::print(OutputLevel::ERROR, "variable \"" + key + "\" does not exist\n");
            return;
        }

        Output::print(OutputLevel::ECHO, key + " = \"" + it->second + "\"\n");
    }

    void BaseCommands::incrementvar(void*, const std::vector<std::string>& args) {
        const std::string& variable = args[0];
        double minValue, maxValue, delta;

        try {
            minValue = std::stod(args[1]);
            maxValue = std::stod(args[2]);
            delta = std::stod(args[3]);
        }
        catch (...) {
            Output::print(OutputLevel::ERROR, "one of the variables is not a number");
            return;
        }

        if (minValue > maxValue) {
            Output::print(OutputLevel::ERROR, "minValue is higher than maxValue");
            return;
        }

        auto it = variables->find(variable);
        if (it == variables->end()) {
            Output::print(OutputLevel::ERROR, "unknown variable \"" + variable + "\"\n");
            return;
        }

        double variableValue;
        try {
            variableValue = std::stod(it->second);
        }
        catch (...) {
            Output::print(OutputLevel::ERROR, "variable value \"" + it->second + "\" is not a number");
            return;
        }

        variableValue += delta;
        if (variableValue > maxValue)
            variableValue = minValue;

        else if (variableValue < minValue)
            variableValue = maxValue;

        (*variables)[variable] = std::to_string(variableValue);
    }

    void BaseCommands::exec(void*, const std::vector<std::string>& args) {
        execConfigFile(args[0], *variables);
    }

    std::unordered_map<std::string, std::string> *BaseCommands::variables;

    Lexer::Lexer(const std::string& input) : input(input), position(0) {}

    Token Lexer::nextToken() {
        if (position >= input.length()) {
            lastToken = {TokenType::_EOF, ""};
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
        for (const auto &command : Command::getCommands()) {

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

    void CVARStorage::setCvar(const std::string& name, const std::function<void(const std::string& value)>& set, const std::function<std::string()>& toString, const std::string& usage) {
        cvars[name] = {set, toString};

        Command(name, 0, 1, asCommand, usage);
        
        // after so much trouble I finally got it to work. But I don't know if that's the best method...
        Command* pCommand = Command::getCommand(name, false); 
        pCommand->pData = pCommand;
    }

    bool CVARStorage::getCvar(const std::string& name, CVariable*& buf) {
        if (cvars.count(name) == 0)
            return false;

        buf = &cvars[name];
        return true;
    }

    void CVARStorage::asCommand(void* pData, const std::vector<std::string>& args) {
        Command* pCommand = (Command*)pData;
        
        CVariable* buf;
        getCvar(pCommand->name, buf);

        // if should print to output
        if (args.size() == 0) {
            Output::printf(OutputLevel::ECHO, "{}\n", buf->toString());
            return;
        }

        // if should set value
        try {
            buf->set(args[0]);
        } catch (...) {
            Command::printUsage(*pCommand);
        }
    }

    std::unordered_map<std::string, CVariable> CVARStorage::cvars;

    std::vector<std::string> loopAliasesRunning = {};
    std::vector<std::string> toggleTypesRunning = {};

    void handleLoopAliasesRunning(std::unordered_map<std::string, std::string>& variables) {
        for (auto& loopAlias : loopAliasesRunning) {
            Lexer lexer{variables[loopAlias]};
            Parser(&lexer, variables).parse();
        }
    }

    Parser::Parser(Lexer *lexer, std::unordered_map<std::string, std::string>& variables) : lexer(lexer), variables(variables) {
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
                        if (it != variables.end())
                            result += it->second; // add variable
                        
                        else { // search in cvars
                            CVariable* buf;
                            if (CVARStorage::getCvar(variable, buf))
                                result += buf->toString();
                            else
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
            for (const auto &argument : arguments) {
                stringBuilder += argument + " ";
            }
            stringBuilder.pop_back(); // remove last space
            arguments.clear();
            arguments.push_back(stringBuilder);
        }

        // checks if arguments size is within the allowed
        if (arguments.size() > (size_t)command->maxArgs || arguments.size() < (size_t)command->minArgs) {
            Command::printUsage(*command);
            if (!arguments.empty())
                Output::print(OutputLevel::ECHO, "arguments size must be within range [" + std::to_string(command->minArgs) + "," + std::to_string(command->maxArgs) + "], but size is " + std::to_string(arguments.size()) + '\n');
            return;
        }

        if (command->name[0] == '+') {
            if (std::find(toggleTypesRunning.begin(), toggleTypesRunning.end(), command->name.substr(1)) != toggleTypesRunning.end())
                return;
            
            toggleTypesRunning.push_back(command->name.substr(1));
        }
            
        else if (command->name[0] == '-') {
            auto it = std::find(toggleTypesRunning.begin(), toggleTypesRunning.end(), command->name.substr(1));
            if (it == toggleTypesRunning.end())
                return;
            
            toggleTypesRunning.erase(it);
        }

        command->run(arguments);
    }

    bool Parser::handleSpecialAliases() {
        std::string varName = currentToken.getValue();
        char front = varName.front();
        
        if (front == '!') {
            auto it = std::find(loopAliasesRunning.begin(), loopAliasesRunning.end(), varName);
            // if is already running, stop
            if (it != loopAliasesRunning.end())
                loopAliasesRunning.erase(it);
            else // if not, make it run
                loopAliasesRunning.push_back(varName);
            
            return false;
        }
        
        if (front == '+') { // if it is not running, start
            auto it = std::find(toggleTypesRunning.begin(), toggleTypesRunning.end(), varName.substr(1));
            if (it == toggleTypesRunning.end()) {
                toggleTypesRunning.push_back(varName.substr(1));
                return true;
            }

            return false;
        }
        
        if (front == '-') { // else: the + version had already ran, so now it will turn off
            auto it = std::find(toggleTypesRunning.begin(), toggleTypesRunning.end(), varName.substr(1));

            if (it != toggleTypesRunning.end()) {
                toggleTypesRunning.erase(it);
                return true;
            }
            
            return false;
        }

        return true;
    }

    void Parser::handleAliasLexer(const std::string& input) {
        std::vector<Lexer*> tempLexers;
        tempLexers.push_back(lexer);

        lexer = new Lexer(input);
        advance();

        while (currentToken.getType() != TokenType::_EOF) {
            std::string variable = getVariableFromCurrentTokenValue();

            if (!variable.empty()) {
                if (handleSpecialAliases()) {
                    tempLexers.push_back(lexer);
                    lexer = new Lexer(variable);
                }
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

            if (!variableValue.empty()) {
                if (handleSpecialAliases())
                    handleAliasLexer(variableValue);
                
                advanceUntil({ TokenType::EOS });
            }

            else if (currentToken.getType() == TokenType::COMMAND)
                handleCommandToken();

            else if (currentToken.getType() == TokenType::STRING) {
                Output::printUnknownCommand(currentToken.getValue());
                advanceUntil({ TokenType::EOS });
            }

            advance();
        }
    }

    void execConfigFile(const std::string& path, std::unordered_map<std::string, std::string>& variables) {
        std::ifstream file(path);

        if (!file) {
            Output::printf(OutputLevel::ERROR, "could not load file \"{}\"\n", path);
            return;
        }

        std::stringstream content;
        while (file.good()) {
            std::string line;
            std::getline(file, line);
            for (size_t i = 0; i < line.size(); ++i) {
                if (line[i] == '/' && line.size()-1 != i && line[i+1] == '/') {
                    line = line.substr(0, i);
                    break;
                }
            }
            content << line << ";";
        }

        Lexer lexer{content.str()};
        Parser parser(&lexer, variables);
        parser.parse();
    }
}