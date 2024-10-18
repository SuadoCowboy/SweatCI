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

namespace SweatCI {
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

    typedef void(*PrintFunction)(void* pData, const OutputLevel& level, const std::string& message);

    class Output {
    public:
        template<typename... Args>
        static void printf(const OutputLevel& level, const std::string& format, Args ... args) {
            print(level, formatString(format, args...));
        }

        static void print(const OutputLevel& level, const std::string& str);
        static void setPrintFunction(void* pData, PrintFunction printFunc);
        static void printUnknownCommand(const std::string& command);

    private:
        static PrintFunction printFunc;
        static void* printFuncData;
    };

    class Command;

    typedef void(*CommandCall)(void* pData, Command& command, const std::vector<std::string>& args);

    class Command {
    public:
        Command() {}

        Command(const std::string& name, unsigned char minArgs, unsigned char maxArgs,
            CommandCall commandCallFunc, const std::string& usage, void* pData = nullptr);
        
        static bool getCommand(const std::string& name, Command& outCommand, bool printError);
        
        static const std::vector<Command>& getCommands();
        
        static void printUsage(const Command &command);
        
        /// @return 1 if success
        static bool deleteCommand(const std::string& commandName);

        static void clear();
        
        void run(const std::vector<std::string>& args);

        std::string name = "";
        std::string usage = "";

        unsigned char minArgs = 0;
        unsigned char maxArgs = 0;
        
        CommandCall commandCallFunc = nullptr;

        void* pData = nullptr;

    private:
        static std::vector<Command> commands;
    };

    namespace BaseCommands {
        void init(std::unordered_map<std::string, std::string> *variables);

        void help(void*, Command&, const std::vector<std::string>& args);
        void commands(void*, Command&, const std::vector<std::string>&);
        void echo(void*, Command&, const std::vector<std::string>& args);
        void alias(void* pData, Command&, const std::vector<std::string>& args);
        void getVariables(void* pData, Command&, const std::vector<std::string>&);
        void variable(void* pData, Command&, const std::vector<std::string>& args);
        void incrementvar(void* pData, Command&, const std::vector<std::string>& args);
        void exec(void* pData, Command&, const std::vector<std::string>& args);
        void toggle(void* pData, Command&, const std::vector<std::string>& args);
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

    template<typename T>
    std::string numberToString(T value) {
        std::string str = std::to_string(value);
	if (str != "0") {
        	str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        	str.erase(str.find_last_not_of('.') + 1, std::string::npos);
	}
        return str;
    }

    namespace Utils {
        namespace Command {
            bool getBoolean(const std::string& str, bool& out);
            bool getFloat(const std::string& str, float& out);
            bool getDouble(const std::string& str, double& out);
            bool getInteger(const std::string& str, int& out);
            bool getShort(const std::string& str, short& out);
            bool getUnsignedShort(const std::string& str, unsigned short& out);
            bool getUnsignedChar(const std::string& str, unsigned char& out);
        }
    
        namespace Cvar {
            void setString(void* pData, const std::string& value);
            std::string getString(void* pData);

            void setBoolean(void* pData, const std::string& value);
            std::string getBoolean(void* pData);

            void setFloat(void* pData, const std::string& value);
            std::string getFloat(void* pData);

            void setInteger(void* pData, const std::string& value);
            std::string getInteger(void* pData);

            void setShort(void* pData, const std::string& value);
            std::string getShort(void* pData);

            void setUnsignedShort(void* pData, const std::string& value);
            std::string getUnsignedShort(void* pData);

            void setUnsignedChar(void* pData, const std::string& value);
            std::string getUnsignedChar(void* pData);
        }
    }

    struct CVariable {
        void (*set)(void* pData, const std::string& value);
        std::string (*toString)(void* pData);
    };

    class CVARStorage {
    public:
        /// @param set to convert a string into the same type and set the new value
        /// @param toString get the cvar value as string
        /// @param usage to be printed out to the console if the user uses help command in it
        static void setCvar(const std::string& name, void* pData, void(*set)(void* pData, const std::string& value), std::string (*toString)(void* pData), const std::string& usage);

        /// @brief Searches for the CVAR and returns it to a buffer
        /// @return false if could not get cvar
        static bool getCvar(const std::string& name, CVariable& buf);

    private:
        static std::unordered_map<std::string, CVariable> cvars;
        
        static void asCommand(void* pData, Command& command, const std::vector<std::string>& args);
    };

    extern std::vector<std::string> loopAliasesRunning;
    extern std::vector<std::string> toggleTypesRunning;

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
