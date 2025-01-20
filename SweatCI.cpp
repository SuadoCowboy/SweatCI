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

#include "SweatCI.h"

#include <regex>
#include <algorithm>
#include <fstream>

namespace SweatCI {
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

    Token::Token() {}
    Token::~Token() {}

    Token::Token(const Token& other) : type(other.type), value(other.value) {}

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

    PrintCallback printCallback = nullptr;
    void* pPrintCallbackData = nullptr;

    void setPrintCallback(void* pData, PrintCallback callback) {
        printCallback = callback;
        pPrintCallbackData = pData;
    }

    void print(const OutputLevel &level, const std::string& str) {
        printCallback(pPrintCallbackData, level, str);
    }

    void printUnknownCommand(const std::string& command) {
        printf(OutputLevel::ERROR, "unknown command \"{}\"\n", command);
    }

    void registerCommand(const std::string& name, unsigned char minArgs, unsigned char maxArgs,
            CommandCallback callback, const std::string& usage, void* pData) {
        Command::commands.emplace_back(name, minArgs, maxArgs, callback, usage, pData);
    }

    void registerCommand(const Command& command) {
        Command::commands.push_back(command);
    }

    Command::Command(const std::string& name, unsigned char minArgs, unsigned char maxArgs, CommandCallback callback, const std::string& usage, void* pData)
      : name(name), usage(usage), minArgs(minArgs), maxArgs(maxArgs), callback(callback), pData(pData) {
        for (const auto& c : commands) {
            if (c.name == name) {
                printf(OutputLevel::ERROR, "command with name \"{}\" already exists\n", name);
                return;
            }
        }
    }

    bool Command::getCommand(const std::string& name, Command*& pCommandOut, bool printError) {
        for (auto& command : commands)
            if (command.name == name) {
                pCommandOut = &command;
                return true;
            }

        if (printError)
            printUnknownCommand(name);

        return false;
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
        print(OutputLevel::WARNING, command.name + ' ' + command.usage + '\n');
    }

    void Command::clear() {
        commands.clear();
    }

    void Command::run(CommandContext& ctx) {
        ctx.pCommand = this;

        callback(ctx);
    }

    std::vector<Command> Command::commands;

    void BaseCommands::init(std::unordered_map<std::string, std::string>* pVariables) {
        // Add commands
        registerCommand("help", 0, 1, help, "<command> - shows the usage of the command specified");
        registerCommand("commands", 0, 0, commands, "- shows a list of commands with their usages");
        registerCommand("echo", 1, 1, echo, "<message> - echoes a message to the console");
        registerCommand("alias", 1, 2, alias, "<var> <commands?> - creates/deletes variables", pVariables);
        registerCommand("variables", 0, 0, getVariables, "- list of variables", pVariables);
        registerCommand("variable", 1, 1, variable, "- shows variable value", pVariables);
        registerCommand("incrementvar", 4, 4, incrementvar, "<var|cvar> <minValue> <maxValue> <delta> - increments the value of a variable", pVariables);
        registerCommand("exec", 1, 1, exec, "- executes a .cfg file that contains SweatCI script", pVariables);
        registerCommand("toggle", 3, 3, toggle, "<var|cvar> <option1> <option2> - toggles value between option1 and option2", pVariables);
    }

    void BaseCommands::help(CommandContext& ctx) {
        if (ctx.args.size() == 1) {
            // Print usage for a specific command
            Command* pCommand = nullptr;
            if (Command::getCommand(ctx.args[0], pCommand, true))
                Command::printUsage(*pCommand);
        } else
            printf(OutputLevel::WARNING, "{} {} - see \"commands\" command to get a list of commands\n", ctx.pCommand->name, ctx.pCommand->usage);
    }

    void BaseCommands::commands(CommandContext&) {
        std::stringstream out;
        for (auto& command : Command::getCommands())
            out << command.name << " " << command.usage << "\n";

        print(OutputLevel::ECHO, out.str());
    }

    void BaseCommands::echo(CommandContext& ctx) {
        std::stringstream message;
        for (const auto& arg : ctx.args)
            message << arg;
        message << "\n";

        print(OutputLevel::ECHO, message.str());
    }

    void BaseCommands::alias(CommandContext& ctx) {
        auto pVariables = static_cast<std::unordered_map<std::string, std::string>*>(ctx.pCommand->pData);
        
        if (ctx.args.size() == 1) {
            if (pVariables->count(ctx.args[0]) == 0) {
                SweatCI::printf(SweatCI::ERROR, "\"{}\" variable not found\n", ctx.args[0]);
                return;
            }

            pVariables->erase(ctx.args[0]);
            if (ctx.args[0].front() == '!') {
                auto it = std::find(loopAliasesRunning.begin(), loopAliasesRunning.end(), ctx.args[0]);
                if (it != loopAliasesRunning.end())
                    loopAliasesRunning.erase(it);
            }

            else if (ctx.args[0].front() == '+') {
                auto it = std::find(toggleTypesRunning.begin(), toggleTypesRunning.end(), ctx.args[0].substr(1));
                if (it != toggleTypesRunning.end())
                    toggleTypesRunning.erase(it);
            }

            return;
        }

        {
            Command* pCommand = nullptr;
            if (Command::getCommand(ctx.args[0], pCommand, false)) {
                print(OutputLevel::ERROR, "varName is a command name, therefore this variable can not be created\n");
                return;
            }
        }

        std::regex whitespace_regex("\\S+"); // TODO: regex is unecessary here I think(use old ways to do this instead of using regex)
        if (!std::regex_match(ctx.args[0], whitespace_regex)) {
            print(OutputLevel::ERROR, "variable name can not have whitespace.\n");
            return;
        }

        std::string negativeVarName = '-'+ctx.args[0].substr(1);
        if (ctx.args[0].front() == '+' && pVariables->count(negativeVarName) == 0) {
            (*pVariables)[negativeVarName] = " ";
        }

        (*pVariables)[ctx.args[0]] = ctx.args[1];
    }

    void BaseCommands::getVariables(CommandContext& ctx) {
        auto pVariables = static_cast<std::unordered_map<std::string, std::string>*>(ctx.pCommand->pData);

        std::stringstream out;

        out << "amount of variables: " << pVariables->size();
        for (const auto& pair : *pVariables)
            out << "\n" << pair.first << " = \"" << pair.second << "\"";
        out << "\n";

        print(OutputLevel::ECHO, out.str());
    }

    void BaseCommands::variable(CommandContext& ctx) {
        auto pVariables = static_cast<std::unordered_map<std::string, std::string>*>(ctx.pCommand->pData);

        const std::string& key = ctx.args[0];
        auto it = pVariables->find(key);
        if (it == pVariables->end()) {
            printf(OutputLevel::ERROR, "variable \"{}\" does not exist\n", key);
            return;
        }

        printf(OutputLevel::ECHO, "{} = \"{}\"\n", key, it->second);
    }

    void BaseCommands::incrementvar(CommandContext& ctx) {
        auto pVariables = static_cast<std::unordered_map<std::string, std::string>*>(ctx.pCommand->pData);
        double minValue, maxValue, delta;

        if (!Utils::Command::getDouble(ctx.args[1], minValue) ||
            !Utils::Command::getDouble(ctx.args[2], maxValue) ||
            !Utils::Command::getDouble(ctx.args[3], delta))
            return;

        if (minValue > maxValue) {
            print(OutputLevel::ERROR, "minValue is higher than maxValue\n");
            return;
        }

        { // cvar
            CVariable* pCvar = nullptr;
            Command* pCvarCommand = nullptr;
            if (CVARStorage::getCvar(ctx.args[0], pCvar) && Command::getCommand(ctx.args[0], pCvarCommand, false)) {
                double variableValue;
                if (!Utils::Command::getDouble(pCvar->toString(pCvarCommand->pData), variableValue))
                    return;

                variableValue += delta;
                if (variableValue > maxValue)
                    variableValue = minValue;

                else if (variableValue < minValue)
                    variableValue = maxValue;
                
                pCvar->set(pCvarCommand->pData, numberToString(variableValue));

                return;
            }
        }

        // var
        auto it = pVariables->find(ctx.args[0]);
        if (it == pVariables->end()) {
            printf(OutputLevel::ERROR, "unknown variable \"{}\"\n", ctx.args[0]);
            return;
        }

        double variableValue;
        if (!Utils::Command::getDouble(it->second, variableValue))
            return;

        variableValue += delta;
        if (variableValue > maxValue)
            variableValue = minValue;

        else if (variableValue < minValue)
            variableValue = maxValue;

        it->second = numberToString(variableValue);
    }

    void BaseCommands::exec(CommandContext& ctx) {
        auto pVariables = static_cast<std::unordered_map<std::string, std::string>*>(ctx.pCommand->pData);
        execConfigFile(ctx, ctx.args[0], pVariables);
    }

    void BaseCommands::toggle(CommandContext& ctx) {
        auto pVariables = static_cast<std::unordered_map<std::string, std::string>*>(ctx.pCommand->pData);

        { // CVAR
            CVariable* pCvar = nullptr;
            Command* pCvarCommand = nullptr;
            if (CVARStorage::getCvar(ctx.args[0], pCvar) && Command::getCommand(ctx.args[0], pCvarCommand, false)) {
                std::string asString = pCvar->toString(pCvarCommand->pData);

                if (asString == ctx.args[1])
                    pCvar->set(pCvarCommand->pData, ctx.args[2]);
                else
                    pCvar->set(pCvarCommand->pData, ctx.args[1]);

                return;
            }
        }
        
        // var
        auto it = pVariables->find(ctx.args[0]);
        if (it == pVariables->end()) {
            printf(OutputLevel::ERROR, "unknown variable \"{}\"\n", ctx.args[0]);
            return;
        }

        if (it->second == ctx.args[1])
            it->second = ctx.args[2];
        else
            it->second = ctx.args[1];
    }

    Lexer::Lexer(const CommandContext& ctx, const std::string& input) : ctx(ctx), input(input) {}

    bool Lexer::nextPosition() {
        ++position;
        ++ctx.columnIndex;

        if (input[position] == '\n') {
            ++ctx.lineIndex;
            ctx.columnIndex = 0;
            return true;
        }

        return false;
    }

    Token Lexer::nextToken() {
        if (position >= input.length()) {
            lastToken = {TokenType::_EOF, ""};
            return lastToken;
        }

        char currentChar = input[position];
        if (currentChar == '\n') {
            ++position;
            lastToken = Token(TokenType::EOS, "\n");
            return lastToken;
        }

        while (std::isspace(currentChar)) {
            if (nextPosition()) {
                lastToken = Token(TokenType::EOS, "\n");
                return lastToken;
            }

            if (position >= input.length()) {
                lastToken = Token(TokenType::_EOF, "");
                return lastToken;
            }

            currentChar = input[position];
        }

        if ((input[position] == ';' || input[position] == '\n') && (position == 0 || input[position-1] != '\\')) {
            nextPosition();
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
        while (position < input.length() && !std::isspace(input[position]) && input[position] != ';' && input[position] != '\n') {
            tokenValue += input[position];
            nextPosition();
        }

		// TODO: wtf is that "x == Nothing || x != Command"????? Why not just "x != Command"???
        if (isCommand(tokenValue) && (lastToken.getType() == TokenType::NOTHING || lastToken.getType() != TokenType::COMMAND))
            return Token(TokenType::COMMAND, tokenValue);
        else
            return Token(TokenType::STRING, tokenValue);
    }

    Token Lexer::parseString() {
        std::string tokenValue = "";

        ++position; // Skip the first double quote
        ++ctx.columnIndex;

        while (position < input.length() && input[position] != '"') {
            // escape '\\'
            if (input[position] == '\\' && position + 1 < input.length() && input[position+1] == '\\')
                nextPosition();
            
            // escape '"'
            else if (input[position] == '\\' && position + 1 < input.length() && input[position+1] == '"')
                nextPosition();

            tokenValue += input[position];
            nextPosition();
        }

        if (input[position] == '"')
            nextPosition(); // Skip the last double quote if exists

        return Token(TokenType::STRING, tokenValue);
    }

    void Utils::Cvar::setString(void* pData, const std::string& value) {
        *(std::string*)pData = value;
    }

    std::string Utils::Cvar::getString(void* pData) {
        return *(std::string*)pData;
    }

    void Utils::Cvar::setBoolean(void* pData, const std::string& value) {
        try {
            if (std::stoi(value) <= 0) *(bool*)pData = false;
            else *(bool*)pData = true;
        } catch (...) {return;}
    }

    std::string Utils::Cvar::getBoolean(void* pData) {
        return std::to_string(*(bool*)pData);
    }

#define _MAKE_CVARUTILS_NUMBER_FUNCTIONS(type, convertFunc, setFuncName, getFuncName) \
    void Utils::Cvar::setFuncName(void* pData, const std::string& value) { \
        try { \
            *static_cast<type*>(pData) = convertFunc(value); \
        } catch (...) {return;} \
    } \
    std::string Utils::Cvar::getFuncName(void* pData) { \
        return numberToString(*(type*)pData); \
    }

    _MAKE_CVARUTILS_NUMBER_FUNCTIONS(float, std::stof, setFloat, getFloat);
    _MAKE_CVARUTILS_NUMBER_FUNCTIONS(int, std::stoi, setInteger, getInteger);
    _MAKE_CVARUTILS_NUMBER_FUNCTIONS(short, (short)std::stoi, setShort, getShort);
    _MAKE_CVARUTILS_NUMBER_FUNCTIONS(unsigned short, (unsigned short)std::stoi, setUnsignedShort, getUnsignedShort);
    _MAKE_CVARUTILS_NUMBER_FUNCTIONS(unsigned char, (unsigned char)std::stoi, setUnsignedChar, getUnsignedChar);

    bool Utils::Command::getBoolean(const std::string& str, bool& out) {
        try {
            int i = std::stoi(str);
            if (i <= 0) out = false;
            else out = true;

            return true;
        } catch (...) {
            printf(ERROR, "\"{}\" is not a boolean\n", str);
            return false;
        }
    }

#define _MAKE_COMMANDUTILS_FUNCTIONS(type, convertFunc, getFuncName, wrongTypeFmt) \
    bool Utils::Command::getFuncName(const std::string& str, type& out) { \
        try { \
            out = static_cast<type>(convertFunc(str)); \
            return true; \
        }  catch (...) { \
            printf(ERROR, wrongTypeFmt, str); \
            return false; \
        } \
    } \

    _MAKE_COMMANDUTILS_FUNCTIONS(float, std::stof, getFloat, "\"{}\" is not a float\n");
    _MAKE_COMMANDUTILS_FUNCTIONS(double, std::stod, getDouble, "\"{}\" is not a double\n");
    _MAKE_COMMANDUTILS_FUNCTIONS(int, std::stoi, getInteger, "\"{}\" is not a integer\n");
    _MAKE_COMMANDUTILS_FUNCTIONS(short, std::stoi, getShort, "\"{}\" is not a short\n");
    _MAKE_COMMANDUTILS_FUNCTIONS(unsigned short, std::stoi, getUnsignedShort, "\"{}\" is not a unsigned short\n");
    _MAKE_COMMANDUTILS_FUNCTIONS(unsigned char, std::stoi, getUnsignedChar, "\"{}\" is not a unsigned char\n");

    void CVARStorage::setCvar(const std::string& name, void* pData, void(*set)(void* pData, const std::string& value), std::string (*toString)(void* pData), const std::string& usage) {
        cvars[name] = {set, toString};
        registerCommand(name, 0, 1, asCommand, usage, pData);
    }

    bool CVARStorage::getCvar(const std::string& name, CVariable*& pBuf) {
        if (cvars.count(name) == 0)
            return false;

        pBuf = &cvars[name];
        return true;
    }

    void CVARStorage::asCommand(CommandContext& ctx) {
        CVariable* pCvar = nullptr;
        if (!getCvar(ctx.pCommand->name, pCvar)) {
            printf(ERROR, "\"{}\" CVAR not found", ctx.pCommand->name);
            return;
        }

        // if should print to output
        if (ctx.args.size() == 0) {
            printf(OutputLevel::ECHO, "{}\n", pCvar->toString(ctx.pCommand->pData));
            return;
        }

        // if should set value
        try {
            pCvar->set(ctx.pCommand->pData, ctx.args[0]);
        } catch (...) {
            Command::printUsage(*ctx.pCommand);
        }
    }

    std::unordered_map<std::string, CVariable> CVARStorage::cvars;

    std::vector<std::string> loopAliasesRunning = {};
    std::vector<std::string> toggleTypesRunning = {};

    void handleLoopAliasesRunning(std::unordered_map<std::string, std::string>* pVariables) {
        for (auto& loopAlias : loopAliasesRunning) {
            Lexer lexer{ { .runningFrom = ALIAS|LOOP_ALIAS|INTERNAL }, pVariables->at(loopAlias)};
            Parser(&lexer, pVariables).parse();
        }
    }

    Parser::Parser(Lexer* pLexer, std::unordered_map<std::string, std::string>* pVariables) : pLexer(pLexer), pVariables(pVariables) {
        advance();
    }

    void Parser::advance() {
        currentToken = pLexer->nextToken();
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
                        
                        auto it = pVariables->find(variable);
                        if (it != pVariables->end())
                            result += it->second; // add variable
                        
                        else { // search in cvars
                            CVariable* pCvar = nullptr;
                            Command* pCommand = nullptr;
                            if (CVARStorage::getCvar(variable, pCvar) && Command::getCommand(variable, pCommand, false))
                                result += pCvar->toString(pCommand->pData);
                            else
                                result += "$" + variable; // or else just add with the $
                        }

                        continue;
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
        auto it = pVariables->find(currentToken.getValue());
        if (it != pVariables->end())
            return it->second;
        return "";
    }

    void Parser::handleCommandToken() {
        std::string commandString = currentToken.getValue();

        Command* pCommand = nullptr;
        if (!Command::getCommand(commandString, pCommand, true))
            return;

        advance(); // skips the command token

        pLexer->ctx.args = getArguments();

        // make it include whitespaces in that case
        if (pCommand->maxArgs == 1 && !pLexer->ctx.args.empty()) {
            std::string stringBuilder;
            for (const auto& argument : pLexer->ctx.args)
                stringBuilder += argument + " ";

            stringBuilder.pop_back(); // remove last space
            pLexer->ctx.args.clear();
            pLexer->ctx.args.push_back(stringBuilder);
        }

        // checks if arguments size is within the allowed
        if (pLexer->ctx.args.size() > (size_t)pCommand->maxArgs || pLexer->ctx.args.size() < (size_t)pCommand->minArgs) {
            Command::printUsage(*pCommand);
            if (!pLexer->ctx.args.empty())
                print(OutputLevel::ECHO, "arguments size must be within range [" + std::to_string(pCommand->minArgs) + "," + std::to_string(pCommand->maxArgs) + "], but size is " + std::to_string(pLexer->ctx.args.size()) + '\n');
            return;
        }

        if (pCommand->name[0] == '+') {
            if (std::find(toggleTypesRunning.begin(), toggleTypesRunning.end(), pCommand->name.substr(1)) != toggleTypesRunning.end())
                return;
            
            toggleTypesRunning.push_back(pCommand->name.substr(1));
        }
            
        else if (pCommand->name[0] == '-') {
            auto it = std::find(toggleTypesRunning.begin(), toggleTypesRunning.end(), pCommand->name.substr(1));
            if (it == toggleTypesRunning.end())
                return;
            
            toggleTypesRunning.erase(it);
        }

        pCommand->run(pLexer->ctx);
    }

    bool Parser::isSpecialAlias() {
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
        tempLexers.push_back(pLexer);

        pLexer->ctx.runningFrom |= ALIAS;

        pLexer = new Lexer(pLexer->ctx, input);
        advance();

        while (currentToken.getType() != TokenType::_EOF) {
            std::string variable = getVariableFromCurrentTokenValue();

            if (!variable.empty()) {
                if (isSpecialAlias()) {
                    tempLexers.push_back(pLexer);
                    pLexer = new Lexer(pLexer->ctx, variable);
                }
            }

            else if (currentToken.getType() == TokenType::COMMAND)
                handleCommandToken();

            else if (currentToken.getType() == TokenType::STRING) {
                printUnknownCommand(currentToken.getValue());
                advanceUntil({ TokenType::EOS });
            }

            advance();

            if (tempLexers.size() == aliasMaxCalls) {
                delete pLexer;

                for (size_t i = 1; i < tempLexers.size(); ++i) {
                    delete tempLexers[i];
                }

                break;
            }

            while (currentToken.getType() == TokenType::_EOF && tempLexers.size() > 1) {
                delete pLexer;

                pLexer = tempLexers.back();
                advance();

                tempLexers.pop_back();
            }
        }

        pLexer = tempLexers[0];
        advanceUntil({ TokenType::EOS }); // if there's something between the alias and the end of statement, we don't care!
    }

    void Parser::parse() {
        while (currentToken.getType() != TokenType::_EOF) {
            std::string variableValue = getVariableFromCurrentTokenValue();

            if (!variableValue.empty()) {
                if (isSpecialAlias())
                    handleAliasLexer(variableValue);
            }

            else if (currentToken.getType() == TokenType::COMMAND)
                handleCommandToken();

            else if (currentToken.getType() == TokenType::STRING) {
                printUnknownCommand(currentToken.getValue());
                advanceUntil({ TokenType::EOS });
            }

            advance();
        }
    }

    void execConfigFile(CommandContext ctx, const std::string& path, std::unordered_map<std::string, std::string>* pVariables) {
        std::ifstream file(path);

        if (!file) {
            printf(OutputLevel::ERROR, "could not load file \"{}\"\n", path);
            return;
        }

        bool inComment = false;
        bool inQuotes = false;
        bool removeOneFromIndex = false;

        std::stringstream content;
        while (file.good()) {
            std::string line;
            std::getline(file, line);
            ++ctx.lineCount;

            for (size_t i = 0; i < line.size(); ++i) {
                if (removeOneFromIndex) {
                    --i;
                    removeOneFromIndex = false;
                }

                if (!inQuotes && line[i] == '*' && line.size()-1 != i) {
                    if (line[i+1] != '/') continue;

                    if (inComment) {
                        inComment = false;
                        line = line.substr(i+2);
                        i = 0;
                        removeOneFromIndex = true;
                    } else { // comment everything before and above
                        content.str(std::string());
                        line = line.substr(i+2);
                        i = 0;
                        removeOneFromIndex = true;
                    }
                
                    continue;
                }

                if (inComment) continue;

                if (line[i] == '"' && (i == 0 || line[i-1] != '\\')) {
                    inQuotes = !inQuotes;
                    continue;
                }

                if (inQuotes) continue;

                if (line[i] == '/' && line.size()-1 != i) {
                    if (line[i+1] == '*') {
                        inComment = true;
                        content << line.substr(0, i);
                        i = 0;
                        removeOneFromIndex = true;
                        continue;
                    } else if (line[i+1] == '/') {
                        line = line.substr(0, i);
                        break;
                    }
                }
            }

            if (!inComment) {
                content << line;
                if (!inQuotes)
                    content << '\n';
            }
            /*
            if (inQuotes)
                content << ' ';
            else
                content << ';';
            */
        }

        ctx.runningFrom |= FILE;
        ctx.filePath = path;
        Lexer lexer = {ctx, content.str()};
        Parser(&lexer, pVariables).parse();
    }
}