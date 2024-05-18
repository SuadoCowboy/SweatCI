#pragma once

/*
 * MIT License
 *
 * Copyright (c) 2024 Lucca Rieffel Silva, also as Suado Cowboy
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the �Software�),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <string>
#include <unordered_map>
#include <vector>

namespace HayBCMD {
    enum TokenType {
        NOTHING = 0,
        STRING,
        COMMAND,
        _EOF,
        EOS
    };

    // Structure for formatString.
    // Usually you won't need to use this structure.
    struct Data {
        enum class Type {INT, DOUBLE, FLOAT, BOOL, STRING};

        int i;
        double d;
        float f;
        bool b;
        std::string s;
        Type type;

        Data(int i);
        Data(double d);
        Data(float f);
        Data(bool b);
        Data(std::string s);
        Data(const char* s);
        Data(char s);

        std::string toString();
    };

    std::string formatString(std::string format, const std::vector<Data>& args);

    std::string tokenTypeToString(const TokenType& type);

    class Token {
    public:
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

    typedef void (*PrintFunction)(const std::string& message);

    class Output {
    public:
        static void printf(const std::string& format, const std::vector<Data>& args);

        static void print(const std::string& str);

        static void setPrintFunction(PrintFunction printFunc);

        static void printUnknownCommand(std::string command) {
            print("unknown command \"" + command + "\"\n");
        }

    private:
        static PrintFunction printFunc;
    };

    class Command;

    typedef void (*CommandCall)(Command* pCommand, const std::vector<std::string>& args);

    class Command {
    public:
        std::string name;
        int minArgs;
        int maxArgs;
        std::string usage;

        Command(const std::string& name, int minArgs, int maxArgs, CommandCall commandCallFunc, const std::string& usage);
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