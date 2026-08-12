#pragma once
#include "AST.h"

// Runs optimization passes over the AST in place (returns possibly-new root):
//   1. Constant folding   — evaluates BinaryOp/UnaryOp nodes whose operands
//      are compile-time constants into a single Number node.
//   2. Dead code elimination — collapses `if` statements whose condition
//      folds to a constant (keeping only the live branch), and removes
//      `while` loops whose condition folds to a compile-time-false value.
class Optimizer {
public:
    // Returns the number of nodes folded/eliminated, for reporting.
    ASTPtr optimize(ASTPtr root, int& foldCount, int& deadCodeCount);

private:
    ASTPtr foldConstants(ASTPtr node, int& foldCount);
    ASTPtr eliminateDeadCode(ASTPtr node, int& deadCodeCount);
    static bool isConstZero(const ASTPtr& node);
};
