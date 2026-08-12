#pragma once
#include "AST.h"
#include <string>
#include <vector>

// One three-address-code instruction.
// arg1/arg2 are either a variable/temp name or a numeric literal (as text).
// `op` is one of: MOV, ADD, SUB, MUL, DIV, LT, GT, EQ, NEQ, NEG,
//                 PRINT, LABEL, JMP, JZ
struct Instr {
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;
};

// Lowers an optimized AST into linear three-address code with explicit
// labels and jumps for control flow (the same shape a real compiler
// backend would hand to a register allocator / codegen stage).
class IRGen {
public:
    std::vector<Instr> generate(const ASTPtr& root);

private:
    std::vector<Instr> code;
    int tempCount = 0;
    int labelCount = 0;

    std::string newTemp();
    std::string newLabel();

    std::string genExpr(const ASTPtr& node);
    void genStmt(const ASTPtr& node);
};
