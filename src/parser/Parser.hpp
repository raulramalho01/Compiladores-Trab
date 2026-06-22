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
    SymbolTable symbolTable;            // usada apenas para exibição (-ts)
    std::vector<std::string> sourceLines; // linhas do fonte (para apontar o erro)
    bool suggestions = false;          // flag -suggest

    // Estrutura
    std::unique_ptr<ProgNode> parseProg();
    std::unique_ptr<MainClassNode> parseMainC();
    std::vector<std::unique_ptr<MethodNode>> parseDefMet();
    std::vector<std::unique_ptr<ClassNode>> parseDefCl();
    std::string parseType();
    bool isType();
    std::vector<VarDecl> parseDefVar();
    std::vector<VarDecl> parseArgs();

    // Comandos
    std::vector<std::unique_ptr<CmdNode>> parseLcom();
    std::unique_ptr<CmdNode> parseCmd();

    // Expressões (cascata de precedência)
    std::unique_ptr<ExpNode> parseExp();
    std::unique_ptr<ExpNode> parseAndExp();
    std::unique_ptr<ExpNode> parseRelExp();
    std::unique_ptr<ExpNode> parseAddExp();
    std::unique_ptr<ExpNode> parseMulExp();
    std::unique_ptr<ExpNode> parseUnExp();
    std::unique_ptr<ExpNode> parsePsfExp();
    std::unique_ptr<ExpNode> parsePriExp();
    std::vector<std::unique_ptr<ExpNode>> parseLexp();

    // Auxiliares
    Token consume(TokenType type, const std::string& errorMessage);
    Token consume(TokenType type, const std::string& lexeme, const std::string& errorMessage);
    bool check(TokenType type) const;
    bool check(TokenType type, const std::string& lexeme) const;
    Token advance();
    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;

    // Erros (com renderização e sugestões quando -suggest está ativo)
    std::runtime_error error(const Token& token, const std::string& message) const;
    std::runtime_error error(const Token& token, const std::string& message,
                             const std::string& expectedLexeme) const;
    std::string renderError(const Token& token, const std::string& message,
                            const std::string& expectedLexeme) const;
    // Erro com sugestão de palavra-chave ("voce quis dizer 'while'?")
    std::runtime_error errorKeyword(const Token& token, const std::string& message,
                                    const std::string& idLexeme) const;
    std::string didYouMeanKeyword(const std::string& lexeme) const;

public:
    Parser(std::vector<Token> tokens, std::string source = "", bool suggestions = false);
    std::unique_ptr<ProgNode> parse();
    const SymbolTable& getSymbolTable() const { return symbolTable; }
};