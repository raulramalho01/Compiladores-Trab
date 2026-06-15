#include "Parser.hpp"
#include <iostream>
#include <sstream>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current(0) {}

std::unique_ptr<ProgNode> Parser::parse() {
    try {
        // Captura a raiz da árvore gerada
        std::unique_ptr<ProgNode> root = parseProg();
        
        std::cout << "\n[SUCESSO] Código está correto sintaticamente!\n";
        // std::cout << symbolTable.toString(); // Pode deixar comentado se preferir focar na AST por enquanto
        
        return root; // Retorna a árvore para o main!
        
    } catch (const std::runtime_error& e) {
        std::cerr << "\n[ERRO SINTÁTICO] " << e.what() << "\n";
        return nullptr; // Se der erro sintático, retorna um ponteiro vazio
    }
}

std::unique_ptr<ProgNode> Parser::parseProg() {
    auto mainClass = parseMainC();
    auto classes = parseDefCl();
    
    if (!isAtEnd()) {
        throw error(peek(), "Esperado fim de arquivo, mas encontrou " + peek().getLexeme());
    }
    
    return std::make_unique<ProgNode>(std::move(mainClass), std::move(classes));
}

std::unique_ptr<MainClassNode> Parser::parseMainC() {
    consume(TokenType::RESERVED_WORD, "class", "Esperado 'class'");
    Token className = consume(TokenType::IDENTIFIER, "Esperado nome da classe");
    symbolTable.add(className.getLexeme(), "class");

    consume(TokenType::DELIMITER, "{", "Esperado '{'");
    consume(TokenType::RESERVED_WORD, "public", "Esperado 'public'");
    consume(TokenType::RESERVED_WORD, "static", "Esperado 'static'");
    consume(TokenType::RESERVED_WORD, "void", "Esperado 'void'");
    consume(TokenType::RESERVED_WORD, "main", "Esperado 'main'");
    consume(TokenType::DELIMITER, "(", "Esperado '('");
    consume(TokenType::RESERVED_WORD, "String", "Esperado 'String'");
    consume(TokenType::DELIMITER, "[", "Esperado '['");
    consume(TokenType::DELIMITER, "]", "Esperado ']'");
    
    Token argName = consume(TokenType::IDENTIFIER, "Esperado nome do argumento");
    symbolTable.add(argName.getLexeme(), "String[]");

    consume(TokenType::DELIMITER, ")", "Esperado ')'");
    consume(TokenType::DELIMITER, "{", "Esperado '{'");

    // Captura todos os comandos executados no main
    auto commands = parseLcom();

    consume(TokenType::DELIMITER, "}", "Esperado '}'");
    consume(TokenType::DELIMITER, "}", "Esperado '}'");

    return std::make_unique<MainClassNode>(className.getLexeme(), std::move(commands));
}

std::vector<std::unique_ptr<MethodNode>> Parser::parseDefMet() {
    std::vector<std::unique_ptr<MethodNode>> methods;
    
    while (check(TokenType::RESERVED_WORD, "public")) {
        advance();
        std::string tipoRetorno = parseType();
        Token nomeMetodo = consume(TokenType::IDENTIFIER, "Esperado nome do método.");
        symbolTable.add(nomeMetodo.getLexeme(), "Método (" + tipoRetorno + ")");

        consume(TokenType::DELIMITER, "(", "Esperado '('");
        if (!check(TokenType::DELIMITER, ")")) {
            parseArgs();
        }
        consume(TokenType::DELIMITER, ")", "Esperado ')'");
        consume(TokenType::DELIMITER, "{", "Esperado '{'");

        parseDefVar();
        
        // Puxa a lógica do corpo
        auto commands = parseLcom();

        consume(TokenType::RESERVED_WORD, "return", "Esperado 'return'");
        auto returnExp = parseExp();
        consume(TokenType::DELIMITER, ";", "Esperado ';'");
        consume(TokenType::DELIMITER, "}", "Esperado '}'");
        
        methods.push_back(std::make_unique<MethodNode>(nomeMetodo.getLexeme(), tipoRetorno, std::move(commands), std::move(returnExp)));
    }
    return methods;
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

std::vector<std::unique_ptr<ClassNode>> Parser::parseDefCl() {
    std::vector<std::unique_ptr<ClassNode>> classes;
    
    while (check(TokenType::RESERVED_WORD, "class")) {
        advance();
        Token nomeClasse = consume(TokenType::IDENTIFIER, "Esperado nome da classe.");
        symbolTable.add(nomeClasse.getLexeme(), "Classe");

        std::string parentName = "";
        if (check(TokenType::RESERVED_WORD, "extends")) {
            advance();
            Token parentToken = consume(TokenType::IDENTIFIER, "Esperado classe pai.");
            parentName = parentToken.getLexeme();
        }

        consume(TokenType::DELIMITER, "{", "Esperado '{'");
        parseDefVar();
        
        // Puxa todos os métodos construídos
        auto methods = parseDefMet();
        
        consume(TokenType::DELIMITER, "}", "Esperado '}'");
        
        classes.push_back(std::make_unique<ClassNode>(nomeClasse.getLexeme(), parentName, std::move(methods)));
    }
    return classes;
}

std::vector<std::unique_ptr<CmdNode>> Parser::parseLcom() {
    std::vector<std::unique_ptr<CmdNode>> commands;
    while (check(TokenType::IDENTIFIER) || check(TokenType::RESERVED_WORD, "if") || 
           check(TokenType::RESERVED_WORD, "while") || check(TokenType::RESERVED_WORD, "System")) {
        commands.push_back(parseCmd());
    }
    return commands;
}

std::unique_ptr<CmdNode> Parser::parseCmd() {
    if (check(TokenType::IDENTIFIER)) {
        Token id = advance();
        if (check(TokenType::DELIMITER, "[")) {
            advance();
            parseExp(); // Opcional: Criar nó de array assign depois se quiser refinar
            consume(TokenType::DELIMITER, "]", "Esperado ']'");
        }
        consume(TokenType::OPERATOR, "=", "Esperado '='");
        auto exp = parseExp();
        consume(TokenType::DELIMITER, ";", "Esperado ';'");
        
        return std::make_unique<AssignNode>(id.getLexeme(), std::move(exp));
        
    } else if (check(TokenType::RESERVED_WORD, "if")) {
        advance();
        consume(TokenType::DELIMITER, "(", "Esperado '('");
        auto condition = parseExp();
        consume(TokenType::DELIMITER, ")", "Esperado ')'");
        consume(TokenType::DELIMITER, "{", "Esperado '{'");
        
        auto ifBlock = parseLcom();
        
        consume(TokenType::DELIMITER, "}", "Esperado '}'");
        
        std::vector<std::unique_ptr<CmdNode>> elseBlock;
        if (check(TokenType::RESERVED_WORD, "else")) {
            advance();
            consume(TokenType::DELIMITER, "{", "Esperado '{'");
            elseBlock = parseLcom();
            consume(TokenType::DELIMITER, "}", "Esperado '}'");
        }
        
        return std::make_unique<IfNode>(std::move(condition), std::move(ifBlock), std::move(elseBlock));
        
    } else if (check(TokenType::RESERVED_WORD, "while")) {
        advance();
        consume(TokenType::DELIMITER, "(", "Esperado '('");
        auto condition = parseExp();
        consume(TokenType::DELIMITER, ")", "Esperado ')'");
        consume(TokenType::DELIMITER, "{", "Esperado '{'");
        
        auto block = parseLcom();
        
        consume(TokenType::DELIMITER, "}", "Esperado '}'");
        
        return std::make_unique<WhileNode>(std::move(condition), std::move(block));
        
    } else if (check(TokenType::RESERVED_WORD, "System")) {
        advance();
        consume(TokenType::DELIMITER, ".", "Esperado '.'");
        consume(TokenType::RESERVED_WORD, "out", "Esperado 'out'");
        consume(TokenType::DELIMITER, ".", "Esperado '.'");
        consume(TokenType::RESERVED_WORD, "println", "Esperado 'println'");
        consume(TokenType::DELIMITER, "(", "Esperado '('");
        
        auto exp = parseExp();
        
        consume(TokenType::DELIMITER, ")", "Esperado ')'");
        consume(TokenType::DELIMITER, ";", "Esperado ';'");
        
        return std::make_unique<PrintNode>(std::move(exp));
    }
    
    throw error(peek(), "Comando inválido.");
}

std::unique_ptr<ExpNode> Parser::parseExp() {
    return parseAndExp();
}

std::unique_ptr<ExpNode> Parser::parseAndExp() {
    auto left = parseRelExp();
    while (check(TokenType::OPERATOR, "&&")) {
        Token op = advance(); // Consome o '&&'
        auto right = parseRelExp();
        // Constrói o nó pai, engolindo a esquerda e a direita
        left = std::make_unique<BinOpNode>(op.getLexeme(), std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpNode> Parser::parseRelExp() {
    auto left = parseAddExp();
    while (check(TokenType::OPERATOR, "<")) {
        Token op = advance();
        auto right = parseAddExp();
        left = std::make_unique<BinOpNode>(op.getLexeme(), std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpNode> Parser::parseAddExp() {
    auto left = parseMulExp();
    while (check(TokenType::OPERATOR, "+") || check(TokenType::OPERATOR, "-")) {
        Token op = advance();
        auto right = parseMulExp();
        left = std::make_unique<BinOpNode>(op.getLexeme(), std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpNode> Parser::parseMulExp() {
    auto left = parseUnExp();
    while (check(TokenType::OPERATOR, "*")) {
        Token op = advance();
        auto right = parseUnExp();
        left = std::make_unique<BinOpNode>(op.getLexeme(), std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpNode> Parser::parseUnExp() {
    if (check(TokenType::OPERATOR, "!")) {
        Token op = advance();
        auto exp = parseUnExp();
        return std::make_unique<UnaryOpNode>(op.getLexeme(), std::move(exp));
    } else {
        return parsePsfExp();
    }
}

std::unique_ptr<ExpNode> Parser::parsePsfExp() {
    auto exp = parsePriExp();
    
    while (true) {
        if (check(TokenType::DELIMITER, "[")) {
            advance();
            auto indexExp = parseExp();
            consume(TokenType::DELIMITER, "]", "Esperado ']'.");
            exp = std::make_unique<ArrayAccessNode>(std::move(exp), std::move(indexExp));
        } else if (check(TokenType::DELIMITER, ".")) {
            advance();
            if (check(TokenType::RESERVED_WORD, "length")) {
                advance();
                // Opcional: Se quiser, crie um LengthNode no AST.hpp. Por enquanto, ignoramos.
            } else if (check(TokenType::IDENTIFIER)) {
                Token methodId = advance();
                consume(TokenType::DELIMITER, "(", "Esperado '('.");
                auto args = parseLexp();
                consume(TokenType::DELIMITER, ")", "Esperado ')'.");
                // Opcional: Aqui iria o MethodCallNode
            } else {
                throw error(peek(), "Esperado 'length' ou Identificador após '.'.");
            }
        } else {
            break;
        }
    }
    return exp;
}

std::unique_ptr<ExpNode> Parser::parsePriExp() {
    if (check(TokenType::DELIMITER, "(")) {
        advance();
        auto exp = parseExp();
        consume(TokenType::DELIMITER, ")", "Esperado ')'.");
        return exp; // O parêntese é só sintático, a AST guarda só a lógica!
    } else if (check(TokenType::RESERVED_WORD, "true")) {
        advance();
        return std::make_unique<BoolLiteralNode>(true);
    } else if (check(TokenType::RESERVED_WORD, "false")) {
        advance();
        return std::make_unique<BoolLiteralNode>(false);
    } else if (check(TokenType::RESERVED_WORD, "this")) {
        advance();
        return std::make_unique<ThisNode>();
    } else if (check(TokenType::NUMBER)) {
        Token num = advance();
        return std::make_unique<IntLiteralNode>(std::stoi(num.getLexeme()));
    } else if (check(TokenType::IDENTIFIER)) {
        Token id = advance();
        return std::make_unique<IdExpNode>(id.getLexeme());
    } else if (check(TokenType::RESERVED_WORD, "new")) {
        advance();
        if (check(TokenType::IDENTIFIER)) {
            Token className = advance();
            consume(TokenType::DELIMITER, "(", "Esperado '('.");
            consume(TokenType::DELIMITER, ")", "Esperado ')'.");
            return nullptr; // Opcional: NewObjNode
        } else if (check(TokenType::RESERVED_WORD, "int")) {
            advance();
            consume(TokenType::DELIMITER, "[", "Esperado '['.");
            auto sizeExp = parseExp();
            consume(TokenType::DELIMITER, "]", "Esperado ']'.");
            return nullptr; // Opcional: NewArrayNode
        } else {
            throw error(peek(), "Esperado Identificador ou 'int' após 'new'.");
        }
    } else {
        throw error(peek(), "Expressão Primária inválida. Encontrado: " + peek().getLexeme());
    }
}

std::vector<std::unique_ptr<ExpNode>> Parser::parseLexp() {
    std::vector<std::unique_ptr<ExpNode>> args;
    if (check(TokenType::DELIMITER, ")")) return args;
    
    args.push_back(parseExp());
    while (check(TokenType::DELIMITER, ",")) {
        advance();
        args.push_back(parseExp());
    }
    return args;
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