#include "Lexer.hpp"
#include <cctype>

// Inicialização da Tabela de Palavras Reservadas
const std::unordered_set<std::string> Lexer::KEYWORDS = {
    "class", "public", "static", "void", "main", "String", "extends",
    "return", "int", "boolean", "if", "else", "while", "System",
    "out", "println", "new", "length", "true", "false", "this"
};

Lexer::Lexer(std::string input) 
    : input(std::move(input)), position(0), line(1), column(1) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token = nextToken();
    
    while (token.getType() != TokenType::TOKEN_EOF) {
        tokens.push_back(token);
        token = nextToken();
    }
    tokens.push_back(token); // Adiciona o EOF na lista também
    
    return tokens;
}

Token Lexer::nextToken() {
    if (position >= input.length()) {
        return Token(TokenType::TOKEN_EOF, "", line, column);
    }

    char currentChar = input[position];

    // Pular espaços em branco
    if (std::isspace(static_cast<unsigned char>(currentChar))) {
        advance();
        return nextToken();
    }

    int startColumn = column;
    int startLine = line;

    // Identificar Números
    if (std::isdigit(static_cast<unsigned char>(currentChar))) {
        return readNumber(startLine, startColumn);
    }

    // Identificar Letras (Identificadores ou Palavras Reservadas)
    if (std::isalpha(static_cast<unsigned char>(currentChar))) {
        return readIdentifierOrKeyword(startLine, startColumn);
    }

    // Identificar Operador de dois caracteres (&&)
    if (currentChar == '&') {
        advance();
        if (position < input.length() && input[position] == '&') {
            advance();
            return Token(TokenType::OPERATOR, "&&", startLine, startColumn);
        }
        return Token(TokenType::ERROR_TOKEN, "&", startLine, startColumn);
    }

    // Identificar Operadores simples e Delimitadores
    std::string symbol(1, currentChar);
    std::string operators = "<>+-*!=";
    if (operators.find(currentChar) != std::string::npos) {
        advance();
        return Token(TokenType::OPERATOR, symbol, startLine, startColumn);
    }

    std::string delimiters = "{}()[];,.";
    if (delimiters.find(currentChar) != std::string::npos) {
        advance();
        return Token(TokenType::DELIMITER, symbol, startLine, startColumn);
    }

    // Tratamento de Erro Léxico
    advance();
    return Token(TokenType::ERROR_TOKEN, symbol, startLine, startColumn);
}

void Lexer::advance() {
    if (position < input.length()) {
        if (input[position] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        position++;
    }
}

Token Lexer::readNumber(int startLine, int startColumn) {
    std::string builder;
    
    while (position < input.length() && std::isdigit(static_cast<unsigned char>(input[position]))) {
        builder += input[position];
        advance();
    }
    
    return Token(TokenType::NUMBER, builder, startLine, startColumn);
}

Token Lexer::readIdentifierOrKeyword(int startLine, int startColumn) {
    std::string builder;
    
    while (position < input.length() && 
          (std::isalnum(static_cast<unsigned char>(input[position])) || input[position] == '_')) {
        builder += input[position];
        advance();
    }
    
    if (KEYWORDS.find(builder) != KEYWORDS.end()) {
        return Token(TokenType::RESERVED_WORD, builder, startLine, startColumn);
    }
    
    return Token(TokenType::IDENTIFIER, builder, startLine, startColumn);
}