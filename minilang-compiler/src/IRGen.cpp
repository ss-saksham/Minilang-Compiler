#include "IRGen.h"
#include <stdexcept>
#include <unordered_map>

std::string IRGen::newTemp() { return "t" + std::to_string(tempCount++); }
std::string IRGen::newLabel() { return "L" + std::to_string(labelCount++); }

std::vector<Instr> IRGen::generate(const ASTPtr& root) {
    code.clear();
    tempCount = 0;
    labelCount = 0;
    genStmt(root);
    return code;
}

std::string IRGen::genExpr(const ASTPtr& node) {
    switch (node->type) {
        case NodeType::Number:
            return std::to_string(node->intVal);
        case NodeType::Variable:
            return node->strVal;
        case NodeType::UnaryOp: {
            std::string src = genExpr(node->children[0]);
            std::string dst = newTemp();
            code.push_back({"NEG", src, "", dst});
            return dst;
        }
        case NodeType::BinaryOp: {
            std::string l = genExpr(node->children[0]);
            std::string r = genExpr(node->children[1]);
            std::string dst = newTemp();
            static const std::unordered_map<std::string, std::string> opMap = {
                {"+", "ADD"}, {"-", "SUB"}, {"*", "MUL"}, {"/", "DIV"},
                {"<", "LT"}, {">", "GT"}, {"==", "EQ"}, {"!=", "NEQ"},
            };
            auto it = opMap.find(node->strVal);
            if (it == opMap.end())
                throw std::runtime_error("IRGen: unknown operator '" + node->strVal + "'");
            code.push_back({it->second, l, r, dst});
            return dst;
        }
        default:
            throw std::runtime_error("IRGen: node is not an expression");
    }
}

void IRGen::genStmt(const ASTPtr& node) {
    switch (node->type) {
        case NodeType::Program:
        case NodeType::Block:
            for (auto& c : node->children) genStmt(c);
            break;

        case NodeType::LetDecl:
        case NodeType::Assign: {
            std::string val = genExpr(node->children[0]);
            code.push_back({"MOV", val, "", node->strVal});
            break;
        }

        case NodeType::Print: {
            std::string val = genExpr(node->children[0]);
            code.push_back({"PRINT", val, "", ""});
            break;
        }

        case NodeType::If: {
            std::string cond = genExpr(node->children[0]);
            std::string elseLabel = newLabel();
            std::string endLabel = newLabel();
            code.push_back({"JZ", cond, "", elseLabel});
            genStmt(node->children[1]);
            code.push_back({"JMP", "", "", endLabel});
            code.push_back({"LABEL", "", "", elseLabel});
            if (node->children[2]) genStmt(node->children[2]);
            code.push_back({"LABEL", "", "", endLabel});
            break;
        }

        case NodeType::While: {
            std::string startLabel = newLabel();
            std::string endLabel = newLabel();
            code.push_back({"LABEL", "", "", startLabel});
            std::string cond = genExpr(node->children[0]);
            code.push_back({"JZ", cond, "", endLabel});
            genStmt(node->children[1]);
            code.push_back({"JMP", "", "", startLabel});
            code.push_back({"LABEL", "", "", endLabel});
            break;
        }

        default:
            throw std::runtime_error("IRGen: node is not a statement");
    }
}
