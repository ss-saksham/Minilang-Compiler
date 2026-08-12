#pragma once
#include "Lexer.h"
#include "AST.h"

// Recursive-descent parser for MiniLang.
// Grammar (precedence low -> high):
//   program    := statement* EOF
//   statement  := "let" IDENT "=" expr ";"
//               | IDENT "=" expr ";"
//               | "print" expr ";"
//               | "if" "(" expr ")" block ("else" block)?
//               | "while" "(" expr ")" block
//   block      := "{" statement* "}"
//   expr       := comparison
//   comparison := additive (("<"|">"|"=="|"!=") additive)?
//   additive   := term (("+"|"-") term)*
//   term       := unary (("*"|"/") unary)*
//   unary      := "-" unary | primary
//   primary    := NUMBER | IDENT | "(" expr ")"
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    ASTPtr parseProgram();

private:
    std::vector<Token> toks;
    size_t pos = 0;

    const Token& peek() const;
    const Token& advance();
    bool check(TokenType t) const;
    bool matchTok(TokenType t);
    const Token& expect(TokenType t, const std::string& msg);

    ASTPtr statement();
    ASTPtr block();
    ASTPtr expr();
    ASTPtr comparison();
    ASTPtr additive();
    ASTPtr term();
    ASTPtr unary();
    ASTPtr primary();
};
