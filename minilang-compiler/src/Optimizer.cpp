#include "Optimizer.h"
#include <stdexcept>

ASTPtr Optimizer::optimize(ASTPtr root, int& foldCount, int& deadCodeCount) {
    foldCount = 0;
    deadCodeCount = 0;
    root = foldConstants(root, foldCount);
    root = eliminateDeadCode(root, deadCodeCount);
    return root;
}

bool Optimizer::isConstZero(const ASTPtr& node) {
    return node->type == NodeType::Number && node->intVal == 0;
}

ASTPtr Optimizer::foldConstants(ASTPtr node, int& foldCount) {
    if (!node) return node;

    for (auto& child : node->children) {
        if (child) child = foldConstants(child, foldCount);
    }

    if (node->type == NodeType::UnaryOp) {
        auto& operand = node->children[0];
        if (operand->type == NodeType::Number) {
            auto folded = makeNode(NodeType::Number);
            folded->intVal = (node->strVal == "-") ? -operand->intVal : operand->intVal;
            foldCount++;
            return folded;
        }
    }

    if (node->type == NodeType::BinaryOp) {
        auto& l = node->children[0];
        auto& r = node->children[1];
        if (l->type == NodeType::Number && r->type == NodeType::Number) {
            int a = l->intVal, b = r->intVal, result = 0;
            const std::string& op = node->strVal;
            if (op == "+") result = a + b;
            else if (op == "-") result = a - b;
            else if (op == "*") result = a * b;
            else if (op == "/") {
                if (b == 0) return node; // preserve runtime divide-by-zero semantics
                result = a / b;
            }
            else if (op == "<") result = (a < b) ? 1 : 0;
            else if (op == ">") result = (a > b) ? 1 : 0;
            else if (op == "==") result = (a == b) ? 1 : 0;
            else if (op == "!=") result = (a != b) ? 1 : 0;
            else return node;

            auto folded = makeNode(NodeType::Number);
            folded->intVal = result;
            foldCount++;
            return folded;
        }
    }
    return node;
}

ASTPtr Optimizer::eliminateDeadCode(ASTPtr node, int& deadCodeCount) {
    if (!node) return node;

    for (auto& child : node->children) {
        if (child) child = eliminateDeadCode(child, deadCodeCount);
    }

    if (node->type == NodeType::If) {
        auto& cond = node->children[0];
        if (cond->type == NodeType::Number) {
            deadCodeCount++;
            if (cond->intVal != 0) {
                return node->children[1]; // always-true: keep only "then"
            } else if (node->children[2]) {
                return node->children[2]; // always-false: keep only "else"
            } else {
                auto empty = makeNode(NodeType::Block); // always-false, no else: drop entirely
                return empty;
            }
        }
    }

    if (node->type == NodeType::While) {
        auto& cond = node->children[0];
        if (isConstZero(cond)) {
            deadCodeCount++;
            return makeNode(NodeType::Block); // loop body is provably unreachable
        }
    }

    // Flatten Program/Block children in case a nested If/While collapsed to
    // an empty Block above - keep the tree tidy for IR generation.
    if (node->type == NodeType::Program || node->type == NodeType::Block) {
        std::vector<ASTPtr> flattened;
        for (auto& c : node->children) {
            if (c->type == NodeType::Block && c->children.empty()) continue; // drop no-op blocks
            flattened.push_back(c);
        }
        node->children = flattened;
    }

    return node;
}
