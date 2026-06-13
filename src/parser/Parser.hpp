#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include "../lexer/Token.hpp"
#include "SymbolTable.hpp"

class Parser {
private:
    std::vector<Token> tokens;
    int current = 0;
    SymbolTable symbolTable;

    void parseProg();
    void parseMainC();
    void parseDefMet();
    std::string parseType();
    bool isType();
    void parseDefVar();
    void parseArgs();
    void parseDefCl();
    
    // Comandos
    void parseLcom(); 
    void parseCmd();

    // Cascata de Precedência de Expressões
    void parseExp();
    void parseAndExp();
    void parseRelExp();
    void parseAddExp();
    void parseMulExp();
    void parseUnExp();
    void parsePsfExp();
    void parsePriExp();
    void parseLexp();

    // Funções Auxiliares
    Token consume(TokenType type, const std::string& errorMessage);
    Token consume(TokenType type, const std::string& lexeme, const std::string& errorMessage);
    bool check(TokenType type) const;
    bool check(TokenType type, const std::string& lexeme) const;
    Token advance();
    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    std::runtime_error error(const Token& token, const std::string& message) const;

public:
    Parser(std::vector<Token> tokens);
    void parse();
};