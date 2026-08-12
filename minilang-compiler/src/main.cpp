#include "Lexer.h"
#include "Parser.h"
#include "Optimizer.h"
#include "IRGen.h"
#include "VM.h"

#include <iostream>
#include <fstream>
#include <sstream>

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Could not open file: " + path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static void dumpIR(const std::vector<Instr>& code) {
    for (const auto& in : code) {
        if (in.op == "LABEL") { std::cout << in.result << ":\n"; continue; }
        std::cout << "    " << in.op;
        if (!in.arg1.empty()) std::cout << " " << in.arg1;
        if (!in.arg2.empty()) std::cout << " " << in.arg2;
        if (!in.result.empty() && in.op != "JMP" && in.op != "JZ") std::cout << " -> " << in.result;
        else if (in.op == "JMP" || in.op == "JZ") std::cout << " " << in.result;
        std::cout << "\n";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: minilangc <file.mini> [--dump-ir] [--no-optimize]\n";
        return 1;
    }

    std::string path = argv[1];
    bool dumpIrFlag = false, noOptimize = false;
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--dump-ir") dumpIrFlag = true;
        else if (arg == "--no-optimize") noOptimize = true;
    }

    try {
        std::string source = readFile(path);

        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        ASTPtr ast = parser.parseProgram();

        int foldCount = 0, deadCodeCount = 0;
        if (!noOptimize) {
            Optimizer opt;
            ast = opt.optimize(ast, foldCount, deadCodeCount);
        }

        IRGen irgen;
        auto code = irgen.generate(ast);

        if (dumpIrFlag) {
            std::cerr << "-- optimizer: " << foldCount << " constant(s) folded, "
                      << deadCodeCount << " dead branch(es)/loop(s) eliminated --\n";
            std::cerr << "-- IR (" << code.size() << " instructions) --\n";
            dumpIR(code);
            std::cerr << "-- program output --\n";
        }

        VM vm;
        vm.run(code);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
