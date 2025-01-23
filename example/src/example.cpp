#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>

#include "SweatCI.h"

std::unordered_map<std::string, std::string> variables;

int test1 = 690420;
float test2 = 32.64f;
short test3 = 25059;
unsigned short test4 = 53021;
unsigned char test5 = 69;
bool test6 = false;
std::string test7 = "Jane Doe";
double test8 = 696969.696969;
unsigned int test9 = 2666969690;

static std::string outputLevelToString(const SweatCI::OutputLevel &level) {
    switch (level) {
    case SweatCI::OutputLevel::DEFAULT:
        return "DEFAULT";
    
    case SweatCI::OutputLevel::ECHO:
        return "ECHO";
    
    case SweatCI::OutputLevel::WARNING:
        return "WARNING";
    
    case SweatCI::OutputLevel::_ERROR:
        return "ERROR";
    };

    return "UNKNOWN";
}

static void print(void*, const SweatCI::OutputLevel& level, const std::string& message) {
    std::cout << outputLevelToString(level) << ": " << message;
}

bool running = true;
static void setRunningToFalseCommand(SweatCI::CommandContext&) {
    running = false;
}

static void infoSelfCommand(SweatCI::CommandContext& ctx) {
    std::stringstream info;
    if (ctx.args.size() > 0 && ctx.args[0] == "minimal") {
        info << ctx.args.size()
             << ' ' << ctx.lineCount
             << ' ' << ctx.lineIndex << ':' << ctx.columnIndex
             << ' ' << ctx.runningFrom
             << ' ' << ctx.filePath << "\n";
    } else {
        info << "ARGS:";
        for (auto& arg : ctx.args)
            info << " \"" << arg << "\";";

        info << "\nARGS COUNT: " << ctx.args.size()
            << "\nFILE PATH: " << ctx.filePath
            << "\nLINE:" << ctx.lineIndex << " COLUMN: " << ctx.columnIndex
            << "\nLINES: " << ctx.lineCount
            << "\nRUNNING FROM FLAGS: " << ctx.runningFrom << "\n";
    }


    SweatCI::print(SweatCI::ECHO, info.str());
}

static void init() {
    SweatCI::setPrintCallback(nullptr, print);
    SweatCI::BaseCommands::init(&variables);

    SweatCI::registerCommand("info_self", 0, 10, infoSelfCommand, "- prints out its own context");

    SweatCI::registerCommand("quit", 0, 0, setRunningToFalseCommand, "- quits");

    SweatCI::CVARStorage::setCvar("t_int",
        &test1,
        SweatCI::Utils::Cvar::setInteger,
        SweatCI::Utils::Cvar::getInteger,
        "- int"
    );

    SweatCI::CVARStorage::setCvar("t_float",
        &test2,
        SweatCI::Utils::Cvar::setFloat,
        SweatCI::Utils::Cvar::getFloat,
        "- float"
    );

    SweatCI::CVARStorage::setCvar("t_short",
        &test3,
        SweatCI::Utils::Cvar::setShort,
        SweatCI::Utils::Cvar::getShort,
        "- short"
    );

    SweatCI::CVARStorage::setCvar("t_ushort",
        &test4,
        SweatCI::Utils::Cvar::setUnsignedShort,
        SweatCI::Utils::Cvar::getUnsignedShort,
        "- unsigned short"
    );

    SweatCI::CVARStorage::setCvar("t_uchar",
        &test5,
        SweatCI::Utils::Cvar::setUnsignedChar,
        SweatCI::Utils::Cvar::getUnsignedChar,
        "- unsigned char"
    );

    SweatCI::CVARStorage::setCvar("t_bool",
        &test6,
        SweatCI::Utils::Cvar::setBoolean,
        SweatCI::Utils::Cvar::getBoolean,
        "- bool"
    );

    SweatCI::CVARStorage::setCvar("t_string",
        &test7,
        SweatCI::Utils::Cvar::setString,
        SweatCI::Utils::Cvar::getString,
        "- string");

    SweatCI::CVARStorage::setCvar("t_double",
        &test8,
        SweatCI::Utils::Cvar::setDouble,
        SweatCI::Utils::Cvar::getDouble,
        " - double");

    SweatCI::CVARStorage::setCvar("t_uint",
        &test9,
        SweatCI::Utils::Cvar::setUnsignedInteger,
        SweatCI::Utils::Cvar::getUnsignedInteger,
        " - unsigned int");

    { // unsigned char bits
        SweatCI::CVARStorage::setCvar("t_uchar_bit1",
            &test5,
            SweatCI::Utils::Cvar::setBit1UnsignedChar,
            SweatCI::Utils::Cvar::getBit1UnsignedChar,
            " - unsigned char bit");

        SweatCI::CVARStorage::setCvar("t_uchar_bit2",
            &test5,
            SweatCI::Utils::Cvar::setBit2UnsignedChar,
            SweatCI::Utils::Cvar::getBit2UnsignedChar,
            " - unsigned char bit");

        SweatCI::CVARStorage::setCvar("t_uchar_bit3",
            &test5,
            SweatCI::Utils::Cvar::setBit3UnsignedChar,
            SweatCI::Utils::Cvar::getBit3UnsignedChar,
            " - unsigned char bit");

        SweatCI::CVARStorage::setCvar("t_uchar_bit4",
            &test5,
            SweatCI::Utils::Cvar::setBit4UnsignedChar,
            SweatCI::Utils::Cvar::getBit4UnsignedChar,
            " - unsigned char bit");

        SweatCI::CVARStorage::setCvar("t_uchar_bit5",
            &test5,
            SweatCI::Utils::Cvar::setBit5UnsignedChar,
            SweatCI::Utils::Cvar::getBit5UnsignedChar,
            " - unsigned char bit");

        SweatCI::CVARStorage::setCvar("t_uchar_bit6",
            &test5,
            SweatCI::Utils::Cvar::setBit6UnsignedChar,
            SweatCI::Utils::Cvar::getBit6UnsignedChar,
            " - unsigned char bit");

        SweatCI::CVARStorage::setCvar("t_uchar_bit7",
            &test5,
            SweatCI::Utils::Cvar::setBit7UnsignedChar,
            SweatCI::Utils::Cvar::getBit7UnsignedChar,
            " - unsigned char bit");

        SweatCI::CVARStorage::setCvar("t_uchar_bit8",
            &test5,
            SweatCI::Utils::Cvar::setBit8UnsignedChar,
            SweatCI::Utils::Cvar::getBit8UnsignedChar,
            " - unsigned char bit");
    }

    { // unsigned short bits
        SweatCI::CVARStorage::setCvar("t_ushort_bit1",
            &test4,
            SweatCI::Utils::Cvar::setBit1UnsignedShort,
            SweatCI::Utils::Cvar::getBit1UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit2",
            &test4,
            SweatCI::Utils::Cvar::setBit2UnsignedShort,
            SweatCI::Utils::Cvar::getBit2UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit3",
            &test4,
            SweatCI::Utils::Cvar::setBit3UnsignedShort,
            SweatCI::Utils::Cvar::getBit3UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit4",
            &test4,
            SweatCI::Utils::Cvar::setBit4UnsignedShort,
            SweatCI::Utils::Cvar::getBit4UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit5",
            &test4,
            SweatCI::Utils::Cvar::setBit5UnsignedShort,
            SweatCI::Utils::Cvar::getBit5UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit6",
            &test4,
            SweatCI::Utils::Cvar::setBit6UnsignedShort,
            SweatCI::Utils::Cvar::getBit6UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit7",
            &test4,
            SweatCI::Utils::Cvar::setBit7UnsignedShort,
            SweatCI::Utils::Cvar::getBit7UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit8",
            &test4,
            SweatCI::Utils::Cvar::setBit8UnsignedShort,
            SweatCI::Utils::Cvar::getBit8UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit9",
            &test4,
            SweatCI::Utils::Cvar::setBit9UnsignedShort,
            SweatCI::Utils::Cvar::getBit9UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit10",
            &test4,
            SweatCI::Utils::Cvar::setBit10UnsignedShort,
            SweatCI::Utils::Cvar::getBit10UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit11",
            &test4,
            SweatCI::Utils::Cvar::setBit11UnsignedShort,
            SweatCI::Utils::Cvar::getBit11UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit12",
            &test4,
            SweatCI::Utils::Cvar::setBit12UnsignedShort,
            SweatCI::Utils::Cvar::getBit12UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit13",
            &test4,
            SweatCI::Utils::Cvar::setBit13UnsignedShort,
            SweatCI::Utils::Cvar::getBit13UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit14",
            &test4,
            SweatCI::Utils::Cvar::setBit14UnsignedShort,
            SweatCI::Utils::Cvar::getBit14UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit15",
            &test4,
            SweatCI::Utils::Cvar::setBit15UnsignedShort,
            SweatCI::Utils::Cvar::getBit15UnsignedShort,
            " - unsigned short bit");

        SweatCI::CVARStorage::setCvar("t_ushort_bit16",
            &test4,
            SweatCI::Utils::Cvar::setBit16UnsignedShort,
            SweatCI::Utils::Cvar::getBit16UnsignedShort,
            " - unsigned short bit");
    }
}

int main()
{
    variables = {};
    init();

    while (running) {
        std::string input;
        std::getline(std::cin, input);

        SweatCI::CommandContext ctx = { .runningFrom = SweatCI::CONSOLE };
        SweatCI::Lexer lexer = {ctx, input};
        SweatCI::Parser(&lexer, &variables).parse();
        
        SweatCI::handleLoopAliasesRunning(&variables);
    }
}