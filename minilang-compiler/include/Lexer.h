#pragma once
#include <string>
#include <vector>

enum class TokenType {
    NUMBER, IDENT,
    LET, PRINT, IF, ELSE, WHILE,
    PLUS, MINUS, STAR, SLASH,
    ASSIGN, SEMI, LPAREN, RPAREN, LBRACE, RBRACE,
    LT, GT, EQ, NEQ,
    END
};

struct Token {
    TokenType type;
    std::string text;
    int intVal = 0;
    int line = 0;
};

// Converts MiniLang source text into a flat token stream.
// Reports lexical errors (unexpected characters) with line numbers.
class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string src;
    size_t pos = 0;
    int line = 1;

    char peek() const;
    char advance();
    bool match(char expected);
    void skipWhitespaceAndComments();
    Token number();
    Token identifierOrKeyword();
};
