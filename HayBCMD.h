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
        Token(const Token& other);
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
        static void printUnknownCommand(const std::string& command);

    private:
        static PrintFunction printFunc;
    };

    class Command;

    typedef std::function<void(Command* pCommand, const std::vector<std::string>& args)> CommandCall;

    class Command {
    public:
        Command(const std::string& name, unsigned char minArgs, unsigned char maxArgs, CommandCall commandCallFunc, const std::string& usage);
        static Command* getCommand(const std::string& name, bool printError);
        static const std::vector<Command>& getCommands();
        static void printUsage(const Command &command);
        static bool deleteCommand(const std::string& commandName); // @return 1 if success
        void run(const std::vector<std::string>& args);

        std::string name;
        std::string usage;

        unsigned char minArgs;
        unsigned char maxArgs;
        
        CommandCall commandCallFunc;

    private:
        static std::vector<Command> commands;
        static void addCommand(Command* pCommand);
    };

    class BaseCommands {
    public:
        static void init(std::unordered_map<std::string, std::string> *variables);

    private:
        static std::unordered_map<std::string, std::string> *variables;

        static void help(Command*, const std::vector<std::string>& args);
        static void echo(Command*, const std::vector<std::string>& args);
        static void alias(Command*, const std::vector<std::string>& args);
        static void getVariables(Command*, const std::vector<std::string>&);
        static void variable(Command*, const std::vector<std::string>& args);
        static void incrementvar(Command*, const std::vector<std::string>& args);
        static void exec(Command*, const std::vector<std::string>& args);
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

    class CVariable {
    public:
        std::function<void(const std::string& value)> set;
        std::function<std::string()> toString;

        virtual void* get() {return nullptr;}
    };

    template<typename T>
    class CVariableImpl : public CVariable {
    public:
        T* pointer;

        void* get() override { return pointer; }

        CVariableImpl(T* pointer, const std::function<void(const std::string& value)>& _set, const std::function<std::string()>& _toString)
            : pointer(pointer) {
                set = _set;
                toString = _toString;
            }
    };

    class CVARStorage {
    public:
        template<typename T>
        static void cvar(const std::string& name, T* value, const std::function<void(const std::string& value)>& set, const std::function<std::string()>& toString, const std::string& usage) {
            cvars[name] = CVariableImpl<T>(value, set, toString);

            Command(name, 0, 1, (CommandCall)asCommand, usage);
        }

        /// @brief Searches for the CVAR and returns it to a buffer
        /// @return false if could not get cvar
        static bool getCvar(const std::string& name, CVariable*& buf);

    private:
        static std::unordered_map<std::string, CVariable> cvars;
        
        static void asCommand(Command* pCommand, const std::vector<std::string>& args);
    };

    extern std::vector<std::string> loopAliasesRunning;
    extern std::vector<std::string> toggleAliasesRunning;

    void handleLoopAliasesRunning(std::unordered_map<std::string, std::string>& variables);

    class Parser {
    public:
        Parser(Lexer *lexer, std::unordered_map<std::string, std::string>& variables);
        void parse();

        unsigned short aliasMaxCalls = 50000;

    private:
        std::vector<std::string> getArguments();
        void advance();
        void advanceUntil(const std::vector<TokenType>& tokenTypes);
        void handleCommandToken();
        /// @return true if should execute alias
        bool handleSpecialAliases();
        void handleAliasLexer(const std::string& input);

        Token currentToken;
        Lexer *lexer = nullptr;
        std::unordered_map<std::string, std::string>& variables;
        std::string getVariableFromCurrentTokenValue();
    };

    void execConfigFile(const std::string& path, std::unordered_map<std::string, std::string>& variables);
}