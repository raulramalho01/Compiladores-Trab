#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include "../lexer/Token.hpp"
#include "SymbolTable.hpp"
#include "AST.hpp"
#include <memory>

class Parser {
private:
    std::vector<Token> tokens;
    int current = 0;
    SymbolTable symbolTable;

    // Onde antes era 'void parseProg();' e 'void parse();'
    std::unique_ptr<ProgNode> parseProg();
    std::unique_ptr<MainClassNode> parseMainC();
    std::vector<std::unique_ptr<MethodNode>> parseDefMet();
    std::vector<std::unique_ptr<ClassNode>> parseDefCl();
    std::string parseType();
    bool isType();
    void parseDefVar();
    void parseArgs();
    
    
    // Comandos
    std::vector<std::unique_ptr<CmdNode>> parseLcom(); 
    std::unique_ptr<CmdNode> parseCmd();

    // Cascata de Precedência de Expressões (Retornando Nós)
    std::unique_ptr<ExpNode> parseExp();
    std::unique_ptr<ExpNode> parseAndExp();
    std::unique_ptr<ExpNode> parseRelExp();
    std::unique_ptr<ExpNode> parseAddExp();
    std::unique_ptr<ExpNode> parseMulExp();
    std::unique_ptr<ExpNode> parseUnExp();
    std::unique_ptr<ExpNode> parsePsfExp();
    std::unique_ptr<ExpNode> parsePriExp();
    std::vector<std::unique_ptr<ExpNode>> parseLexp(); // Retorna uma lista de argumentos

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
    std::unique_ptr<ProgNode> parse();
};