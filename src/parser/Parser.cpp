#include "Parser.hpp"
#include <iostream>
#include <sstream>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current(0) {}

void Parser::parse() {
    try {
        parseProg();
        std::cout << "\n[SUCESSO] Código está correto sintaticamente!\n";
        std::cout << symbolTable.toString();
    } catch (const std::runtime_error& e) {
        std::cerr << "\n[ERRO SINTÁTICO] " << e.what() << "\n";
    }
}

void Parser::parseProg() {
    parseMainC();
    parseDefCl();
    if (!isAtEnd()) {
        throw error(peek(), "Esperado fim de arquivo, mas encontrou " + peek().getLexeme());
    }
}

void Parser::parseMainC() {
    consume(TokenType::RESERVED_WORD, "class", "Esperado palavra reservada 'class' no início.");
    Token className = consume(TokenType::IDENTIFIER, "Esperado nome da classe (Identificador).");
    symbolTable.add(className.getLexeme(), "class");

    consume(TokenType::DELIMITER, "{", "Esperado '{' após nome da classe.");
    consume(TokenType::RESERVED_WORD, "public", "Esperado 'public'.");
    consume(TokenType::RESERVED_WORD, "static", "Esperado 'static'.");
    consume(TokenType::RESERVED_WORD, "void", "Esperado 'void'.");
    consume(TokenType::RESERVED_WORD, "main", "Esperado 'main'.");
    consume(TokenType::DELIMITER, "(", "Esperado '('.");
    consume(TokenType::RESERVED_WORD, "String", "Esperado 'String'.");
    consume(TokenType::DELIMITER, "[", "Esperado '['.");
    consume(TokenType::DELIMITER, "]", "Esperado ']'.");

    Token argName = consume(TokenType::IDENTIFIER, "Esperado nome do argumento do main.");
    symbolTable.add(argName.getLexeme(), "String[]");

    consume(TokenType::DELIMITER, ")", "Esperado ')'.");
    consume(TokenType::DELIMITER, "{", "Esperado '{' para iniciar o main.");

    parseLcom();

    consume(TokenType::DELIMITER, "}", "Esperado '}' para fechar o main.");
    consume(TokenType::DELIMITER, "}", "Esperado '}' para fechar a classe principal.");
}

void Parser::parseDefMet() {
    while (check(TokenType::RESERVED_WORD, "public")) {
        advance();

        std::string tipoRetorno = parseType();
        Token nomeMetodo = consume(TokenType::IDENTIFIER, "Esperado nome do método.");
        symbolTable.add(nomeMetodo.getLexeme(), "Método (" + tipoRetorno + ")");

        consume(TokenType::DELIMITER, "(", "Esperado '(' após o nome do método.");

        if (!check(TokenType::DELIMITER, ")")) {
            parseArgs();
        }

        consume(TokenType::DELIMITER, ")", "Esperado ')' para fechar os argumentos.");
        consume(TokenType::DELIMITER, "{", "Esperado '{' para iniciar o corpo do método.");

        parseDefVar();
        parseLcom();

        consume(TokenType::RESERVED_WORD, "return", "Esperado 'return' no final do método.");
        parseExp();
        consume(TokenType::DELIMITER, ";", "Esperado ';' após a expressão de retorno.");
        consume(TokenType::DELIMITER, "}", "Esperado '}' para fechar o método.");
    }
}

std::string Parser::parseType() {
    if (check(TokenType::RESERVED_WORD, "boolean")) {
        advance();
        return "boolean";
    } else if (check(TokenType::IDENTIFIER)) {
        Token id = advance();
        return id.getLexeme();
    } else if (check(TokenType::RESERVED_WORD, "int")) {
        advance();
        if (check(TokenType::DELIMITER, "[")) {
            advance();
            consume(TokenType::DELIMITER, "]", "Esperado ']' após '[' na declaração de vetor.");
            return "int[]";
        }
        return "int";
    }
    throw error(peek(), "Tipo inválido. Esperado 'int', 'boolean' ou um Identificador.");
}

bool Parser::isType() {
    if (check(TokenType::RESERVED_WORD, "int") || check(TokenType::RESERVED_WORD, "boolean")) {
        return true;
    }
    if (check(TokenType::IDENTIFIER) && current + 1 < tokens.size()) {
        if (tokens[current + 1].getType() == TokenType::IDENTIFIER) {
            return true;
        }
    }
    return false;
}

void Parser::parseDefVar() {
    while (isType()) {
        std::string tipoDaVariavel = parseType();
        Token nomeDaVariavel = consume(TokenType::IDENTIFIER, "Esperado nome da variável.");
        consume(TokenType::DELIMITER, ";", "Esperado ';' após declaração da variável '" + nomeDaVariavel.getLexeme() + "'.");
        symbolTable.add(nomeDaVariavel.getLexeme(), tipoDaVariavel);
    }
}

void Parser::parseArgs() {
    std::string tipoArg = parseType();
    Token nomeArg = consume(TokenType::IDENTIFIER, "Esperado nome do argumento.");
    symbolTable.add(nomeArg.getLexeme(), tipoArg);

    while (check(TokenType::DELIMITER, ",")) {
        advance();
        tipoArg = parseType();
        nomeArg = consume(TokenType::IDENTIFIER, "Esperado nome do argumento após ','.");
        symbolTable.add(nomeArg.getLexeme(), tipoArg);
    }
}

void Parser::parseDefCl() {
    while (check(TokenType::RESERVED_WORD, "class")) {
        advance();
        Token nomeClasse = consume(TokenType::IDENTIFIER, "Esperado nome da classe.");
        symbolTable.add(nomeClasse.getLexeme(), "Classe");

        if (check(TokenType::RESERVED_WORD, "extends")) {
            advance();
            consume(TokenType::IDENTIFIER, "Esperado nome da classe pai após 'extends'.");
        }

        consume(TokenType::DELIMITER, "{", "Esperado '{' para iniciar o corpo da classe.");
        parseDefVar();
        parseDefMet();
        consume(TokenType::DELIMITER, "}", "Esperado '}' para fechar a classe.");
    }
}

void Parser::parseLcom() {
    while (check(TokenType::IDENTIFIER) || 
           check(TokenType::RESERVED_WORD, "if") || 
           check(TokenType::RESERVED_WORD, "while") || 
           check(TokenType::RESERVED_WORD, "System")) {
        parseCmd();
    }
}

void Parser::parseCmd() {
    if (check(TokenType::IDENTIFIER)) {
        Token id = advance();
        if (check(TokenType::DELIMITER, "[")) {
            advance();
            parseExp();
            consume(TokenType::DELIMITER, "]", "Esperado ']' na atribuição do vetor.");
        }
        consume(TokenType::OPERATOR, "=", "Esperado '=' na atribuição da variável '" + id.getLexeme() + "'.");
        parseExp();
        consume(TokenType::DELIMITER, ";", "Esperado ';' no fim da atribuição.");
        
    } else if (check(TokenType::RESERVED_WORD, "if")) {
        advance();
        consume(TokenType::DELIMITER, "(", "Esperado '(' após 'if'.");
        parseExp();
        consume(TokenType::DELIMITER, ")", "Esperado ')' após expressão do 'if'.");
        consume(TokenType::DELIMITER, "{", "Esperado '{' para iniciar o bloco do if.");
        parseLcom();
        consume(TokenType::DELIMITER, "}", "Esperado '}' para fechar o bloco do if.");
        
        if (check(TokenType::RESERVED_WORD, "else")) {
            advance();
            consume(TokenType::DELIMITER, "{", "Esperado '{' para iniciar o bloco do else.");
            parseLcom();
            consume(TokenType::DELIMITER, "}", "Esperado '}' para fechar o bloco do else.");
        }
        
    } else if (check(TokenType::RESERVED_WORD, "while")) {
        advance();
        consume(TokenType::DELIMITER, "(", "Esperado '(' após 'while'.");
        parseExp();
        consume(TokenType::DELIMITER, ")", "Esperado ')' após expressão do 'while'.");
        consume(TokenType::DELIMITER, "{", "Esperado '{' para iniciar o bloco do while.");
        parseLcom();
        consume(TokenType::DELIMITER, "}", "Esperado '}' para fechar o bloco do while.");
        
    } else if (check(TokenType::RESERVED_WORD, "System")) {
        advance();
        consume(TokenType::DELIMITER, ".", "Esperado '.' após System.");
        consume(TokenType::RESERVED_WORD, "out", "Esperado 'out'.");
        consume(TokenType::DELIMITER, ".", "Esperado '.' após out.");
        consume(TokenType::RESERVED_WORD, "println", "Esperado 'println'.");
        consume(TokenType::DELIMITER, "(", "Esperado '('.");
        parseExp();
        consume(TokenType::DELIMITER, ")", "Esperado ')'.");
        consume(TokenType::DELIMITER, ";", "Esperado ';' após println.");
    } else {
        throw error(peek(), "Comando inválido. Esperado 'if', 'while', 'System' ou uma atribuição.");
    }
}

void Parser::parseExp() {
    parseAndExp();
}

void Parser::parseAndExp() {
    parseRelExp();
    while (check(TokenType::OPERATOR, "&&")) {
        advance();
        parseRelExp();
    }
}

void Parser::parseRelExp() {
    parseAddExp();
    while (check(TokenType::OPERATOR, "<")) {
        advance();
        parseAddExp();
    }
}

void Parser::parseAddExp() {
    parseMulExp();
    while (check(TokenType::OPERATOR, "+") || check(TokenType::OPERATOR, "-")) {
        advance();
        parseMulExp();
    }
}

void Parser::parseMulExp() {
    parseUnExp();
    while (check(TokenType::OPERATOR, "*")) {
        advance();
        parseUnExp();
    }
}

void Parser::parseUnExp() {
    if (check(TokenType::OPERATOR, "!")) {
        advance();
        parseUnExp();
    } else {
        parsePsfExp();
    }
}

void Parser::parsePsfExp() {
    parsePriExp();
    while (true) {
        if (check(TokenType::DELIMITER, "[")) {
            advance();
            parseExp();
            consume(TokenType::DELIMITER, "]", "Esperado ']'.");
        } else if (check(TokenType::DELIMITER, ".")) {
            advance();
            if (check(TokenType::RESERVED_WORD, "length")) {
                advance();
            } else if (check(TokenType::IDENTIFIER)) {
                advance();
                consume(TokenType::DELIMITER, "(", "Esperado '('.");
                parseLexp();
                consume(TokenType::DELIMITER, ")", "Esperado ')'.");
            } else {
                throw error(peek(), "Esperado 'length' ou Identificador após '.'.");
            }
        } else {
            break;
        }
    }
}

void Parser::parsePriExp() {
    if (check(TokenType::DELIMITER, "(")) {
        advance();
        parseExp();
        consume(TokenType::DELIMITER, ")", "Esperado ')'.");
    } else if (check(TokenType::RESERVED_WORD, "true") || 
               check(TokenType::RESERVED_WORD, "false") ||
               check(TokenType::RESERVED_WORD, "this") ||
               check(TokenType::NUMBER) ||
               check(TokenType::IDENTIFIER)) {
        advance();
    } else if (check(TokenType::RESERVED_WORD, "new")) {
        advance();
        if (check(TokenType::IDENTIFIER)) {
            advance();
            consume(TokenType::DELIMITER, "(", "Esperado '('.");
            consume(TokenType::DELIMITER, ")", "Esperado ')'.");
        } else if (check(TokenType::RESERVED_WORD, "int")) {
            advance();
            consume(TokenType::DELIMITER, "[", "Esperado '['.");
            parseExp();
            consume(TokenType::DELIMITER, "]", "Esperado ']'.");
        } else {
            throw error(peek(), "Esperado Identificador ou 'int' após 'new'.");
        }
    } else {
        throw error(peek(), "Expressão Primária inválida. Encontrado: " + peek().getLexeme());
    }
}

void Parser::parseLexp() {
    if (check(TokenType::DELIMITER, ")")) return;
    parseExp();
    while (check(TokenType::DELIMITER, ",")) {
        advance();
        parseExp();
    }
}

Token Parser::consume(TokenType type, const std::string& errorMessage) {
    if (check(type)) return advance();
    throw error(peek(), errorMessage);
}

Token Parser::consume(TokenType type, const std::string& lexeme, const std::string& errorMessage) {
    if (check(type, lexeme)) return advance();
    throw error(peek(), errorMessage);
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().getType() == type;
}

bool Parser::check(TokenType type, const std::string& lexeme) const {
    if (isAtEnd()) return false;
    return peek().getType() == type && peek().getLexeme() == lexeme;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().getType() == TokenType::TOKEN_EOF;
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

std::runtime_error Parser::error(const Token& token, const std::string& message) const {
    std::ostringstream oss;
    oss << "Erro na linha " << token.getLine() << ", coluna " << token.getColumn() << ": " << message;
    if (token.getType() == TokenType::TOKEN_EOF) {
        oss << " (Fim de arquivo inesperado)";
    } else {
        oss << " (Token encontrado: '" << token.getLexeme() << "')";
    }
    return std::runtime_error(oss.str());
}