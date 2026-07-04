#include "../lexer/Lexer.hpp"
#include "../parser/Parser.hpp"
#include "../parser/AST.hpp"
#include "../parser/SemanticContext.hpp"
#include "../codegen/tac.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

bool tem_flag(const std::vector<std::string>& args, const std::string& flag) {
    for (const auto& arg : args)
        if (arg == flag) return true;
    return false;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);

    if (argc < 2) {
        std::cout << "Uso correto: ./compilador <arquivo_fonte> [flags]\n";
        std::cout << "Flags disponiveis:\n";
        std::cout << "  -tokens       Imprime a lista de tokens gerada\n";
        std::cout << "  -ast          Imprime a Arvore Sintatica Abstrata\n";
        std::cout << "  -ts           Imprime a tabela de simbolos\n";
        std::cout << "  -lex-strict   Para no PRIMEIRO erro lexico (em vez de listar todos)\n";
        std::cout << "  -suggest      Mostra a linha do erro e sugestoes de correcao\n";
        std::cout << "  -3ac          Gera e imprime o codigo intermediario (3 enderecos)\n";
        return 1;
    }

    std::string arquivo_entrada = args[1];
    bool lexStrict = tem_flag(args, "-lex-strict");
    bool suggest   = tem_flag(args, "-suggest");

    std::ifstream inFile(arquivo_entrada);
    if (!inFile.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo: " << arquivo_entrada << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string input = buffer.str();

    std::cout << "========================================================\n";
    std::cout << ">>> PROCESSANDO: " << arquivo_entrada << "\n";
    std::cout << "========================================================\n";

    // 1. ANALISADOR LEXICO
    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();

    if (tem_flag(args, "-tokens")) {
        std::cout << "\n=== [DEBUG] LISTA DE TOKENS GERADA ===\n";
        for (const auto& token : tokens)
            if (token.getType() != TokenType::TOKEN_EOF)
                std::cout << "Linha " << token.getLine() << ", Col " << token.getColumn()
                          << " | Texto: '" << token.getLexeme() << "'\n";
        std::cout << "======================================\n\n";
    }

    // 1b. TRATAMENTO DE ERROS LEXICOS
    // Por padrao: reporta TODOS os erros lexicos e aborta.
    // Com -lex-strict: morre no PRIMEIRO erro lexico encontrado.
    int errosLexicos = 0;
    for (const auto& token : tokens) {
        if (token.getType() == TokenType::ERROR_TOKEN) {
            errosLexicos++;
            std::cerr << "[ERRO LEXICO] Linha " << token.getLine() << ", coluna "
                      << token.getColumn() << ": caractere/simbolo invalido '"
                      << token.getLexeme() << "'.\n";
            if (lexStrict) {
                std::cerr << "Compilacao abortada no primeiro erro lexico (-lex-strict).\n";
                return 1;
            }
        }
    }
    if (errosLexicos > 0) {
        std::cerr << "Foram encontrados " << errosLexicos
                  << " erro(s) lexico(s). Compilacao abortada.\n";
        return 1;
    }

    // 2. ANALISADOR SINTATICO (constroi a AST)
    Parser parser(tokens, input, suggest);

    try {
        std::unique_ptr<ProgNode> astRoot = parser.parse();

        if (tem_flag(args, "-ast")) {
            std::cout << "\n=== [DEBUG] ARVORE SINTATICA ABSTRATA (AST) ===\n";
            if (astRoot) astRoot->print(0);
            std::cout << "===============================================\n\n";
        }

        if (astRoot) {
            // 3. ANALISE SEMANTICA EM DUAS PASSADAS (coleta TODOS os erros)
            ClassTable classTable;
            SemanticContext ctx(classTable);
            astRoot->buildClassTable(classTable, ctx.errors); // 1a passada: grafo + erros de classe
            astRoot->checkSemantic(ctx);                      // 2a passada: tipos, dispatch, init

            if (ctx.errors.empty()) {
                std::cout << "\n[SUCESSO] Analise Semantica concluida sem erros de tipos, "
                             "declaracoes, heranca ou inicializacao\n";

                // 4. GERACAO DE CODIGO INTERMEDIARIO (3AC)
                if (tem_flag(args, "-3ac") || tem_flag(args, "-ir")) {
                    SymbolTable tabelaGeracao;      // tabela (hash) para temporarias e labels
                    CodeGen cg(tabelaGeracao);
                    Code codigo = astRoot->gen(cg); // percorre a AST gerando 3AC
                    std::cout << "\n=== CODIGO INTERMEDIARIO (3 ENDERECOS) ===\n";
                    std::cout << codigo.toString();
                    std::cout << "==========================================\n";
                }
            } else {
                std::cerr << "\n[ERROS SEMANTICOS] Foram encontrados "
                          << ctx.errors.size() << " erro(s):\n";
                int i = 1;
                for (const auto& e : ctx.errors)
                    std::cerr << "  " << i++ << ") " << e << "\n";
                std::cerr << "Falha na compilacao.\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "\n" << e.what() << "\n";
        std::cerr << "Falha na compilacao.\n";
    }

    if (tem_flag(args, "-ts"))
        std::cout << parser.getSymbolTable().toString();

    return 0;
}