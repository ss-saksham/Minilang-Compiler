#pragma once
#include <string>
#include <vector>
#include <memory>

enum class NodeType {
    Program, Block,
    LetDecl, Assign, Print, If, While,
    BinaryOp, UnaryOp, Number, Variable
};

struct ASTNode;
using ASTPtr = std::shared_ptr<ASTNode>;

// Generic tagged AST node. Field usage depends on `type`:
//   Number:    intVal
//   Variable:  strVal = name
//   BinaryOp:  strVal = operator, children = {left, right}
//   UnaryOp:   strVal = operator, children = {operand}
//   LetDecl:   strVal = name, children = {initializer}
//   Assign:    strVal = name, children = {expr}
//   Print:     children = {expr}
//   If:        children = {cond, thenBlock, elseBlock?} (elseBlock may be null)
//   While:     children = {cond, body}
//   Block/Program: children = statement list
struct ASTNode {
    NodeType type;
    std::string strVal;
    int intVal = 0;
    std::vector<ASTPtr> children;

    ASTNode(NodeType t) : type(t) {}
};

inline ASTPtr makeNode(NodeType t) { return std::make_shared<ASTNode>(t); }
