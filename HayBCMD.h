#pragma once

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
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <functional>

namespace HayBCMD {
    enum TokenType {
        NOTHING = 0,
        STRING,
        COMMAND,
        _EOF,
        EOS
    };

    /// @warning this function is not meant to be used outside this header
    template<typename ...>
    static void _formatStringValue(const std::string& format, std::stringstream& buf) {
        buf << format;
    }

    /// @warning this function is not meant to be used outside this header
    template<typename T, typename ... Args>
    static void _formatStringValue(const std::string& format, std::stringstream& buf, T value, Args& ... args) {
        size_t idx = format.find("{}");
        if (idx == std::string::npos) {
            buf << format;
            return;
        }

        buf << format.substr(0, idx) << value;
        
        _formatStringValue(format.substr(idx+2), buf, args...);
    }

    template<typename ... Args>
    std::string formatString(const std::string& format, Args ... args)
    {
        std::stringstream buf;
        _formatStringValue(format, buf, args...);
        return buf.str();
    }

    std::string tokenTypeToString(const TokenType& type);

    class Token {
    public:
        Token(const HayBCMD::Token& other);
        Token();
        ~Token();

        Token(const TokenType& type, const std::string& value);
        const TokenType& getType() const;
        const std::string& getValue();
        std::string string() const;

        Token& operator=(const Token& other);

    private:
        TokenType type;
        std::string value;
    };


    enum OutputLevel {
        DEFAULT = 0, // text that is not involved by user interaction
        ECHO, // any text that came from a command that is not an error
        WARNING,
        ERROR, // anything that went wrong
    };

    typedef std::function<void(const OutputLevel& level, const std::string& message)> PrintFunction;

    class Output {
    public:
        template<typename... Args>
        static void printf(const OutputLevel& level, const std::string& format, Args ... args) {
            print(level, formatString(format, args...));
        }

        static void print(const OutputLevel& level, const std::string& str);

        static void setPrintFunction(PrintFunction printFunc);

        static void printUnknownCommand(std::string command) {
            print(OutputLevel::ERROR, "unknown command \"" + command + "\"\n");
        }

    private:
        static PrintFunction printFunc;
    };

    class Command;

    typedef std::function<void(Command* pCommand, const std::vector<std::string>& args)> CommandCall;

    class Command {
    public:
        std::string name;
        unsigned char minArgs;
        unsigned char maxArgs;
        std::string usage;

        Command(const std::string& name, unsigned char minArgs, unsigned char maxArgs, CommandCall commandCallFunc, const std::string& usage);
        static Command* getCommand(const std::string& name, bool printError);
        static const std::vector<Command>& getCommands();
        static void printUsage(const Command& command);
        static bool deleteCommand(const std::string& commandName); // @return 1 if success
        void run(const std::vector<std::string>& args);

    private:
        CommandCall commandCallFunc;

        static std::vector<Command> commands;
        static void addCommand(Command* pCommand);
    };

    class BaseCommands {
    public:
        static void init(std::unordered_map<std::string, std::string>* variables);

    private:
        static std::unordered_map<std::string, std::string>* variables;

        static void help(Command* _pCommand, const std::vector<std::string>& args);
        static void echo(Command* _pCommand, const std::vector<std::string>& args);
        static void alias(Command* _pCommand, const std::vector<std::string>& args);
        static void getVariables(Command* _pCommand, const std::vector<std::string>& args);
        static void variable(Command* _pCommand, const std::vector<std::string>& args);
        static void incrementvar(Command* _pCommand, const std::vector<std::string>& args);
    };

    class Lexer {
    public:
        Lexer(const std::string& input);

        Token nextToken();

    private:
        bool isVariable(const std::string& identifier);
        bool isCommand(const std::string& commandName);
        Token parseToken();
        Token parseString();

        std::string input;
        size_t position;
        Token lastToken;
    };

    class CVARStorage {
    public:
        static void cvar(const std::string& name, bool value, const std::string& usage);
        static void cvar(const std::string& name, double value, const std::string& usage);
        static void cvar(const std::string& name, const std::string& value, const std::string& usage);
        static void cvar(const std::string& name, const char* value, const std::string& usage);

        static void setCvar(const std::string& name, bool value);
        static void setCvar(const std::string& name, double value);
        static void setCvar(const std::string& name, const std::string& value);
        static void setCvar(const std::string& name, const char* value);

        // Searches for the CVAR and returns it to a buffer
        // @return false if could not get cvar
        static bool getCvar(const std::string& name, bool& buf);
        static bool getCvar(const std::string& name, std::string& buf);
        static bool getCvar(const std::string& name, double& buf);
        
        // @return n = not found; s = string; b = bool; f = float
        static char getCvarType(const std::string& name);

    private:
        static std::unordered_map<std::string, bool> boolCvars;
        static std::unordered_map<std::string, double> doubleCvars;
        static std::unordered_map<std::string, std::string> stringCvars;
        
        static void asCommand(HayBCMD::Command* pCommand, const std::vector<std::string>& args);
    };

    class Parser {
    public:
        Parser(Lexer* lexer, std::unordered_map<std::string, std::string>& variables);
        void parse();

        static unsigned int aliasMaxCalls;

    private:
        std::vector<std::string> getArguments();
        void advance();
        void advanceUntil(const std::vector<TokenType>& tokenTypes);
        void handleCommandToken();
        void handleAliasLexer(const std::string& input);

        Token currentToken;
        Lexer* lexer;
        std::unordered_map<std::string, std::string>& variables;
        std::string getVariableFromCurrentTokenValue();
    };
}