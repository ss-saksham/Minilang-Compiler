#include "Parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens) : toks(std::move(tokens)) {}

const Token& Parser::peek() const { return toks[pos]; }
const Token& Parser::advance() { return toks[pos++]; }
bool Parser::check(TokenType t) const { return peek().type == t; }

bool Parser::matchTok(TokenType t) {
    if (check(t)) { pos++; return true; }
    return false;
}

const Token& Parser::expect(TokenType t, const std::string& msg) {
    if (check(t)) return advance();
    throw std::runtime_error("Parse error line " + std::to_string(peek().line) +
                              ": expected " + msg + " but got '" + peek().text + "'");
}

ASTPtr Parser::parseProgram() {
    auto prog = makeNode(NodeType::Program);
    while (!check(TokenType::END)) prog->children.push_back(statement());
    return prog;
}

ASTPtr Parser::block() {
    expect(TokenType::LBRACE, "'{'");
    auto blk = makeNode(NodeType::Block);
    while (!check(TokenType::RBRACE) && !check(TokenType::END))
        blk->children.push_back(statement());
    expect(TokenType::RBRACE, "'}'");
    return blk;
}

ASTPtr Parser::statement() {
    if (matchTok(TokenType::LET)) {
        auto name = expect(TokenType::IDENT, "identifier");
        expect(TokenType::ASSIGN, "'='");
        auto init = expr();
        expect(TokenType::SEMI, "';'");
        auto node = makeNode(NodeType::LetDecl);
        node->strVal = name.text;
        node->children.push_back(init);
        return node;
    }
    if (matchTok(TokenType::PRINT)) {
        auto val = expr();
        expect(TokenType::SEMI, "';'");
        auto node = makeNode(NodeType::Print);
        node->children.push_back(val);
        return node;
    }
    if (matchTok(TokenType::IF)) {
        expect(TokenType::LPAREN, "'('");
        auto cond = expr();
        expect(TokenType::RPAREN, "')'");
        auto thenBlk = block();
        ASTPtr elseBlk = nullptr;
        if (matchTok(TokenType::ELSE)) elseBlk = block();
        auto node = makeNode(NodeType::If);
        node->children.push_back(cond);
        node->children.push_back(thenBlk);
        node->children.push_back(elseBlk); // may be nullptr
        return node;
    }
    if (matchTok(TokenType::WHILE)) {
        expect(TokenType::LPAREN, "'('");
        auto cond = expr();
        expect(TokenType::RPAREN, "')'");
        auto body = block();
        auto node = makeNode(NodeType::While);
        node->children.push_back(cond);
        node->children.push_back(body);
        return node;
    }
    // Assignment: IDENT = expr ;
    if (check(TokenType::IDENT)) {
        auto name = advance();
        expect(TokenType::ASSIGN, "'='");
        auto val = expr();
        expect(TokenType::SEMI, "';'");
        auto node = makeNode(NodeType::Assign);
        node->strVal = name.text;
        node->children.push_back(val);
        return node;
    }
    throw std::runtime_error("Parse error line " + std::to_string(peek().line) +
                              ": unexpected token '" + peek().text + "'");
}

ASTPtr Parser::expr() { return comparison(); }

ASTPtr Parser::comparison() {
    auto left = additive();
    if (check(TokenType::LT) || check(TokenType::GT) ||
        check(TokenType::EQ) || check(TokenType::NEQ)) {
        auto opTok = advance();
        auto right = additive();
        auto node = makeNode(NodeType::BinaryOp);
        node->strVal = opTok.text;
        node->children = {left, right};
        return node;
    }
    return left;
}

ASTPtr Parser::additive() {
    auto left = term();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        auto opTok = advance();
        auto right = term();
        auto node = makeNode(NodeType::BinaryOp);
        node->strVal = opTok.text;
        node->children = {left, right};
        left = node;
    }
    return left;
}

ASTPtr Parser::term() {
    auto left = unary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        auto opTok = advance();
        auto right = unary();
        auto node = makeNode(NodeType::BinaryOp);
        node->strVal = opTok.text;
        node->children = {left, right};
        left = node;
    }
    return left;
}

ASTPtr Parser::unary() {
    if (matchTok(TokenType::MINUS)) {
        auto operand = unary();
        auto node = makeNode(NodeType::UnaryOp);
        node->strVal = "-";
        node->children.push_back(operand);
        return node;
    }
    return primary();
}

ASTPtr Parser::primary() {
    if (check(TokenType::NUMBER)) {
        auto tok = advance();
        auto node = makeNode(NodeType::Number);
        node->intVal = tok.intVal;
        return node;
    }
    if (check(TokenType::IDENT)) {
        auto tok = advance();
        auto node = makeNode(NodeType::Variable);
        node->strVal = tok.text;
        return node;
    }
    if (matchTok(TokenType::LPAREN)) {
        auto e = expr();
        expect(TokenType::RPAREN, "')'");
        return e;
    }
    throw std::runtime_error("Parse error line " + std::to_string(peek().line) +
                              ": expected expression, got '" + peek().text + "'");
}
