#include "Lexer.h"
#include <stdexcept>
#include <cctype>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"let", TokenType::LET}, {"print", TokenType::PRINT},
    {"if", TokenType::IF},   {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
};

Lexer::Lexer(std::string source) : src(std::move(source)) {}

char Lexer::peek() const {
    return pos < src.size() ? src[pos] : '\0';
}

char Lexer::advance() {
    char c = src[pos++];
    if (c == '\n') line++;
    return c;
}

bool Lexer::match(char expected) {
    if (peek() != expected) return false;
    pos++;
    return true;
}

void Lexer::skipWhitespaceAndComments() {
    for (;;) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && pos + 1 < src.size() && src[pos + 1] == '/') {
            while (peek() != '\n' && peek() != '\0') advance();
        } else {
            break;
        }
    }
}

Token Lexer::number() {
    std::string text;
    int startLine = line;
    while (isdigit(peek())) text += advance();
    Token t;
    t.type = TokenType::NUMBER;
    t.text = text;
    t.intVal = std::stoi(text);
    t.line = startLine;
    return t;
}

Token Lexer::identifierOrKeyword() {
    std::string text;
    int startLine = line;
    while (isalnum(peek()) || peek() == '_') text += advance();
    Token t;
    t.text = text;
    t.line = startLine;
    auto it = KEYWORDS.find(text);
    t.type = (it != KEYWORDS.end()) ? it->second : TokenType::IDENT;
    return t;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    for (;;) {
        skipWhitespaceAndComments();
        if (pos >= src.size()) break;
        char c = peek();
        int startLine = line;

        if (isdigit(c)) { tokens.push_back(number()); continue; }
        if (isalpha(c) || c == '_') { tokens.push_back(identifierOrKeyword()); continue; }

        advance();
        Token t; t.line = startLine;
        switch (c) {
            case '+': t.type = TokenType::PLUS; t.text = "+"; break;
            case '-': t.type = TokenType::MINUS; t.text = "-"; break;
            case '*': t.type = TokenType::STAR; t.text = "*"; break;
            case '/': t.type = TokenType::SLASH; t.text = "/"; break;
            case ';': t.type = TokenType::SEMI; t.text = ";"; break;
            case '(': t.type = TokenType::LPAREN; t.text = "("; break;
            case ')': t.type = TokenType::RPAREN; t.text = ")"; break;
            case '{': t.type = TokenType::LBRACE; t.text = "{"; break;
            case '}': t.type = TokenType::RBRACE; t.text = "}"; break;
            case '<': t.type = TokenType::LT; t.text = "<"; break;
            case '>': t.type = TokenType::GT; t.text = ">"; break;
            case '=':
                if (match('=')) { t.type = TokenType::EQ; t.text = "=="; }
                else { t.type = TokenType::ASSIGN; t.text = "="; }
                break;
            case '!':
                if (match('=')) { t.type = TokenType::NEQ; t.text = "!="; }
                else throw std::runtime_error("Lexer error line " + std::to_string(startLine) + ": unexpected '!'");
                break;
            default:
                throw std::runtime_error("Lexer error line " + std::to_string(startLine) +
                                          ": unexpected character '" + std::string(1, c) + "'");
        }
        tokens.push_back(t);
    }
    Token end; end.type = TokenType::END; end.text = "<eof>"; end.line = line;
    tokens.push_back(end);
    return tokens;
}
