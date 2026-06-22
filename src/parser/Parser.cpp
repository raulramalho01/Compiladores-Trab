#include "Parser.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

// --------- Construtor: quebra o fonte em linhas para apontar erros ----------
Parser::Parser(std::vector<Token> tokens, std::string source, bool suggestions)
    : tokens(std::move(tokens)), current(0), suggestions(suggestions) {
    std::stringstream ss(source);
    std::string line;
    while (std::getline(ss, line)) sourceLines.push_back(line);
}

std::unique_ptr<ProgNode> Parser::parse() {
    try {
        std::unique_ptr<ProgNode> root = parseProg();
        std::cout << "\n[SUCESSO] Codigo esta correto sintaticamente!\n";
        return root;
    } catch (const std::runtime_error& e) {
        std::cerr << "\n[ERRO SINTATICO]\n" << e.what() << "\n";
        return nullptr;
    }
}

// ==========================================================
// ESTRUTURA
// ==========================================================
std::unique_ptr<ProgNode> Parser::parseProg() {
    auto mainClass = parseMainC();
    auto classes = parseDefCl();
    if (!isAtEnd())
        throw error(peek(), "Esperado fim de arquivo, mas encontrou " + peek().getLexeme());
    return std::make_unique<ProgNode>(std::move(mainClass), std::move(classes));
}

std::unique_ptr<MainClassNode> Parser::parseMainC() {
    consume(TokenType::RESERVED_WORD, "class", "Esperado 'class'");
    Token className = consume(TokenType::IDENTIFIER, "Esperado nome da classe");
    symbolTable.add(className.getLexeme(), "class", "Global");

    consume(TokenType::DELIMITER, "{", "Esperado '{'");
    symbolTable.enterScope("Classe " + className.getLexeme());

    consume(TokenType::RESERVED_WORD, "public", "Esperado 'public'");
    consume(TokenType::RESERVED_WORD, "static", "Esperado 'static'");
    consume(TokenType::RESERVED_WORD, "void", "Esperado 'void'");
    consume(TokenType::RESERVED_WORD, "main", "Esperado 'main'");

    symbolTable.enterScope("Metodo main");

    consume(TokenType::DELIMITER, "(", "Esperado '('");
    consume(TokenType::RESERVED_WORD, "String", "Esperado 'String'");
    consume(TokenType::DELIMITER, "[", "Esperado '['");
    consume(TokenType::DELIMITER, "]", "Esperado ']'");
    Token argName = consume(TokenType::IDENTIFIER, "Esperado nome do argumento");
    symbolTable.add(argName.getLexeme(), "String[]", "Parametro");

    consume(TokenType::DELIMITER, ")", "Esperado ')'");
    consume(TokenType::DELIMITER, "{", "Esperado '{'");

    auto commands = parseLcom();

    consume(TokenType::DELIMITER, "}", "Esperado '}'");
    symbolTable.exitScope();
    consume(TokenType::DELIMITER, "}", "Esperado '}'");
    symbolTable.exitScope();

    return std::make_unique<MainClassNode>(className.getLexeme(), argName.getLexeme(), std::move(commands));
}

std::vector<std::unique_ptr<MethodNode>> Parser::parseDefMet() {
    std::vector<std::unique_ptr<MethodNode>> methods;
    while (check(TokenType::RESERVED_WORD, "public")) {
        advance();
        std::string tipoRetorno = parseType();
        Token nomeMetodo = consume(TokenType::IDENTIFIER, "Esperado nome do metodo.");
        symbolTable.add(nomeMetodo.getLexeme(), tipoRetorno, "Metodo");
        symbolTable.enterScope("Metodo " + nomeMetodo.getLexeme());

        consume(TokenType::DELIMITER, "(", "Esperado '('");
        std::vector<VarDecl> params;
        if (!check(TokenType::DELIMITER, ")")) params = parseArgs();
        consume(TokenType::DELIMITER, ")", "Esperado ')'");
        consume(TokenType::DELIMITER, "{", "Esperado '{'");

        std::vector<VarDecl> locals = parseDefVar();
        auto commands = parseLcom();

        consume(TokenType::RESERVED_WORD, "return", "Esperado 'return'");
        auto returnExp = parseExp();
        consume(TokenType::DELIMITER, ";", "Esperado ';'");
        consume(TokenType::DELIMITER, "}", "Esperado '}'");

        symbolTable.exitScope();

        methods.push_back(std::make_unique<MethodNode>(
            nomeMetodo.getLexeme(), tipoRetorno, std::move(params), std::move(locals),
            std::move(commands), std::move(returnExp)));
    }
    return methods;
}

std::vector<std::unique_ptr<ClassNode>> Parser::parseDefCl() {
    std::vector<std::unique_ptr<ClassNode>> classes;
    while (check(TokenType::RESERVED_WORD, "class")) {
        advance();
        Token nomeClasse = consume(TokenType::IDENTIFIER, "Esperado nome da classe.");
        symbolTable.add(nomeClasse.getLexeme(), "Classe", "Global");

        std::string parentName = "";
        if (check(TokenType::RESERVED_WORD, "extends")) {
            advance();
            parentName = consume(TokenType::IDENTIFIER, "Esperado classe pai.").getLexeme();
        }

        consume(TokenType::DELIMITER, "{", "Esperado '{'");
        symbolTable.enterScope("Classe " + nomeClasse.getLexeme());

        std::vector<VarDecl> fields = parseDefVar();
        auto methods = parseDefMet();

        consume(TokenType::DELIMITER, "}", "Esperado '}'");
        symbolTable.exitScope();

        classes.push_back(std::make_unique<ClassNode>(
            nomeClasse.getLexeme(), parentName, std::move(fields), std::move(methods)));
    }
    return classes;
}

std::vector<VarDecl> Parser::parseDefVar() {
    std::vector<VarDecl> decls;
    while (isType()) {
        std::string tipo = parseType();
        Token nome = consume(TokenType::IDENTIFIER, "Esperado nome da variavel.");
        consume(TokenType::DELIMITER, ";",
                "Esperado ';' apos a declaracao da variavel '" + nome.getLexeme() + "'.");
        symbolTable.add(nome.getLexeme(), tipo, "Variavel/Atributo");
        decls.push_back(VarDecl{tipo, nome.getLexeme()});
    }
    return decls;
}

std::vector<VarDecl> Parser::parseArgs() {
    std::vector<VarDecl> args;
    std::string tipo = parseType();
    Token nome = consume(TokenType::IDENTIFIER, "Esperado nome do argumento.");
    symbolTable.add(nome.getLexeme(), tipo, "Parametro");
    args.push_back(VarDecl{tipo, nome.getLexeme()});
    while (check(TokenType::DELIMITER, ",")) {
        advance();
        tipo = parseType();
        nome = consume(TokenType::IDENTIFIER, "Esperado nome do argumento apos ','.");
        symbolTable.add(nome.getLexeme(), tipo, "Parametro");
        args.push_back(VarDecl{tipo, nome.getLexeme()});
    }
    return args;
}

std::string Parser::parseType() {
    if (check(TokenType::RESERVED_WORD, "boolean")) {
        advance();
        return "boolean";
    } else if (check(TokenType::IDENTIFIER)) {
        return advance().getLexeme();
    } else if (check(TokenType::RESERVED_WORD, "int")) {
        advance();
        if (check(TokenType::DELIMITER, "[")) {
            advance();
            consume(TokenType::DELIMITER, "]", "Esperado ']' apos '[' na declaracao de vetor.");
            return "int[]";
        }
        return "int";
    }
    throw error(peek(), "Tipo invalido. Esperado 'int', 'boolean' ou um Identificador.");
}

bool Parser::isType() {
    if (check(TokenType::RESERVED_WORD, "int") || check(TokenType::RESERVED_WORD, "boolean"))
        return true;
    if (check(TokenType::IDENTIFIER) && current + 1 < (int)tokens.size())
        if (tokens[current + 1].getType() == TokenType::IDENTIFIER) return true;
    return false;
}

// ==========================================================
// COMANDOS
// ==========================================================
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
        // Heurística: 'identificador (' no início de um comando quase sempre é
        // uma palavra-chave (if/while) digitada errada.
        if (check(TokenType::DELIMITER, "(")) {
            throw errorKeyword(id, "'" + id.getLexeme() + "' nao e um comando valido.", id.getLexeme());
        }
        if (check(TokenType::DELIMITER, "[")) {
            advance();
            auto indexExp = parseExp();
            consume(TokenType::DELIMITER, "]", "Esperado ']'");
            consume(TokenType::OPERATOR, "=", "Esperado '='");
            auto valueExp = parseExp();
            consume(TokenType::DELIMITER, ";", "Esperado ';'");
            return std::make_unique<ArrayAssignNode>(id.getLexeme(), std::move(indexExp), std::move(valueExp));
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
    throw error(peek(), "Comando invalido.");
}

// ==========================================================
// EXPRESSÕES
// ==========================================================
std::unique_ptr<ExpNode> Parser::parseExp() { return parseAndExp(); }

std::unique_ptr<ExpNode> Parser::parseAndExp() {
    auto left = parseRelExp();
    while (check(TokenType::OPERATOR, "&&")) {
        Token op = advance();
        auto right = parseRelExp();
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
    }
    return parsePsfExp();
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
                exp = std::make_unique<LengthNode>(std::move(exp));
            } else if (check(TokenType::IDENTIFIER)) {
                Token methodId = advance();
                consume(TokenType::DELIMITER, "(", "Esperado '('.");
                auto args = parseLexp();
                consume(TokenType::DELIMITER, ")", "Esperado ')'.");
                exp = std::make_unique<MethodCallNode>(std::move(exp), methodId.getLexeme(), std::move(args));
            } else {
                throw error(peek(), "Esperado 'length' ou Identificador apos '.'.");
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
        return exp;
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
        return std::make_unique<IntLiteralNode>(std::stoi(advance().getLexeme()));
    } else if (check(TokenType::IDENTIFIER)) {
        return std::make_unique<IdExpNode>(advance().getLexeme());
    } else if (check(TokenType::RESERVED_WORD, "new")) {
        advance();
        if (check(TokenType::IDENTIFIER)) {
            Token className = advance();
            consume(TokenType::DELIMITER, "(", "Esperado '('.");
            consume(TokenType::DELIMITER, ")", "Esperado ')'.");
            return std::make_unique<NewObjectNode>(className.getLexeme());
        } else if (check(TokenType::RESERVED_WORD, "int")) {
            advance();
            consume(TokenType::DELIMITER, "[", "Esperado '['.");
            auto sizeExp = parseExp();
            consume(TokenType::DELIMITER, "]", "Esperado ']'.");
            return std::make_unique<NewArrayNode>(std::move(sizeExp));
        } else {
            throw error(peek(), "Esperado Identificador ou 'int' apos 'new'.");
        }
    }
    throw error(peek(), "Expressao primaria invalida. Encontrado: " + peek().getLexeme());
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

// ==========================================================
// AUXILIARES
// ==========================================================
Token Parser::consume(TokenType type, const std::string& errorMessage) {
    if (check(type)) return advance();
    throw error(peek(), errorMessage);
}

Token Parser::consume(TokenType type, const std::string& lexeme, const std::string& errorMessage) {
    if (check(type, lexeme)) return advance();
    throw error(peek(), errorMessage, lexeme);
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

bool Parser::isAtEnd() const { return peek().getType() == TokenType::TOKEN_EOF; }
Token Parser::peek() const { return tokens[current]; }
Token Parser::previous() const { return tokens[current - 1]; }

// ----------------------------------------------------------
// Erros: mensagem dura sempre; sugestões só com a flag -suggest
// ----------------------------------------------------------
static int distanciaEdicao(const std::string& a, const std::string& b) {
    std::vector<std::vector<int>> d(a.size() + 1, std::vector<int>(b.size() + 1));
    for (size_t i = 0; i <= a.size(); ++i) d[i][0] = (int)i;
    for (size_t j = 0; j <= b.size(); ++j) d[0][j] = (int)j;
    for (size_t i = 1; i <= a.size(); ++i)
        for (size_t j = 1; j <= b.size(); ++j) {
            int custo = (a[i - 1] == b[j - 1]) ? 0 : 1;
            d[i][j] = std::min({d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + custo});
        }
    return d[a.size()][b.size()];
}

std::string Parser::didYouMeanKeyword(const std::string& lexeme) const {
    static const std::vector<std::string> kws = {
        "class","public","static","void","main","String","extends","return",
        "int","boolean","if","else","while","System","out","println","new",
        "length","true","false","this"};
    std::string melhor; int melhorDist = 1000;
    for (const auto& k : kws) {
        int d = distanciaEdicao(lexeme, k);
        if (d < melhorDist) { melhorDist = d; melhor = k; }
    }
    if (melhorDist > 0 && melhorDist <= 2)
        return "voce quis dizer '" + melhor + "'?";
    return "";
}

std::string Parser::renderError(const Token& token, const std::string& message,
                                const std::string& expectedLexeme) const {
    std::ostringstream oss;
    oss << "Erro na linha " << token.getLine() << ", coluna " << token.getColumn() << ": " << message;
    if (token.getType() == TokenType::TOKEN_EOF)
        oss << " (Fim de arquivo inesperado)";
    else
        oss << " (Token encontrado: '" << token.getLexeme() << "')";

    if (!suggestions) return oss.str();

    // Renderiza a linha-fonte com um '^' apontando a coluna do erro
    int li = token.getLine() - 1;
    if (li >= 0 && li < (int)sourceLines.size()) {
        const std::string& src = sourceLines[li];
        oss << "\n\n  " << src << "\n  ";
        int col = token.getColumn() - 1;
        for (int i = 0; i < col && i < (int)src.size(); ++i)
            oss << (src[i] == '\t' ? '\t' : ' ');
        oss << "^";
    }

    // Dica direcionada
    oss << "\n  Sugestao: ";
    if (!expectedLexeme.empty()) {
        oss << "parece faltar '" << expectedLexeme << "' nesta posicao.";
        if (expectedLexeme == ";")
            oss << " Talvez voce tenha esquecido o ponto-e-virgula no fim do comando anterior.";
    } else if (token.getType() == TokenType::IDENTIFIER) {
        std::string dym = didYouMeanKeyword(token.getLexeme());
        oss << (dym.empty() ? "verifique a sintaxe esperada neste ponto." : dym);
    } else {
        oss << "verifique a sintaxe esperada neste ponto.";
    }
    return oss.str();
}

std::runtime_error Parser::error(const Token& token, const std::string& message) const {
    return std::runtime_error(renderError(token, message, ""));
}

std::runtime_error Parser::error(const Token& token, const std::string& message,
                                 const std::string& expectedLexeme) const {
    return std::runtime_error(renderError(token, message, expectedLexeme));
}

std::runtime_error Parser::errorKeyword(const Token& token, const std::string& message,
                                        const std::string& idLexeme) const {
    std::ostringstream oss;
    oss << "Erro na linha " << token.getLine() << ", coluna " << token.getColumn()
        << ": " << message << " (Token encontrado: '" << token.getLexeme() << "')";
    if (suggestions) {
        int li = token.getLine() - 1;
        if (li >= 0 && li < (int)sourceLines.size()) {
            const std::string& src = sourceLines[li];
            oss << "\n\n  " << src << "\n  ";
            int col = token.getColumn() - 1;
            for (int i = 0; i < col && i < (int)src.size(); ++i)
                oss << (src[i] == '\t' ? '\t' : ' ');
            oss << "^";
        }
        std::string dym = didYouMeanKeyword(idLexeme);
        oss << "\n  Sugestao: " << (dym.empty()
            ? "comandos validos comecam com atribuicao, 'if', 'while' ou 'System.out.println'."
            : dym);
    }
    return std::runtime_error(oss.str());
}