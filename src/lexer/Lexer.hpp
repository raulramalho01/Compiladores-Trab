#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include "Token.hpp"

class Lexer {
private:
    std::string input;
    int position;
    int line;
    int column;
    static const std::unordered_set<std::string> KEYWORDS;

    void advance();
    Token readNumber(int startLine, int startColumn);
    Token readIdentifierOrKeyword(int startLine, int startColumn);

public:
    Lexer(std::string input);
    std::vector<Token> tokenize();
    Token nextToken();
};