#pragma once
#include <string>
#include "TokenType.hpp"

class Token {
private:
    TokenType type;
    std::string lexeme;
    int line;
    int column;

public:
    Token(TokenType type, std::string lexeme, int line, int column)
        : type(type), lexeme(std::move(lexeme)), line(line), column(column) {}

    TokenType getType() const { return type; }
    std::string getLexeme() const { return lexeme; }
    int getLine() const { return line; }
    int getColumn() const { return column; }
};