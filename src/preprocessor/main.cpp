#include "../lexer/Lexer.hpp"
#include "../parser/Parser.hpp"
#include "../parser/AST.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// Função auxiliar para verificar se uma flag foi passada no terminal
bool tem_flag(const std::vector<std::string>& args, const std::string& flag) {
    for (const auto& arg : args) {
        if (arg == flag) return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    // Transforma os argumentos do terminal em um vetor de strings para facilitar
    std::vector<std::string> args(argv, argv + argc);

    if (argc < 2) {
        std::cout << "Uso correto: ./compilador <arquivo_fonte> [flags]\n";
        std::cout << "Flags disponíveis:\n";
        std::cout << "  -tokens       Printar a lista de tokens gerada\n";
        std::cout << "  -ts           Printar a tabela de símbolos\n";
        return 1;
    }

    std::string arquivo_entrada = args[1];

    // 1. Ler o arquivo-fonte diretamente (sem pré-processador isolado)
    std::ifstream inFile(arquivo_entrada);
    if (!inFile.is_open()) {
        std::cerr << "Erro: Não foi possível abrir o arquivo: " << arquivo_entrada << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string input = buffer.str();

    std::cout << "========================================================\n";
    std::cout << ">>> PROCESSANDO: " << arquivo_entrada << "\n";
    std::cout << "========================================================\n";

    // 2. Executar Analisador Léxico (Nativo, tratando espaços e comentários)
    Lexer lexer(input);
    std::vector<Token> tokens = lexer.tokenize();

    // Se o usuário passou a flag "-tokens", nós imprimimos a lista
    if (tem_flag(args, "-tokens")) {
        std::cout << "\n=== [DEBUG] LISTA DE TOKENS GERADA ===\n";
        for (const auto& token : tokens) {
            // Se o token for EOF, não precisamos dar print em lexeme vazio
            if (token.getType() != TokenType::TOKEN_EOF) {
                std::cout << "Linha " << token.getLine() << ", Col " << token.getColumn() 
                          << " | Texto: '" << token.getLexeme() << "'\n";
            }
        }
        std::cout << "======================================\n\n";
    }

    // 3. Executar Analisador Sintático e Gerar a AST
    Parser parser(tokens);
    
    try {
        // Agora o parser nos devolve a raiz da árvore construída
        std::unique_ptr<ProgNode> astRoot = parser.parse();

        // Se a flag -ast estiver presente, nós disparamos a impressão
        if (tem_flag(args, "-ast")) {
            std::cout << "\n=== [DEBUG] ARVORE SINTATICA ABSTRATA (AST) ===\n";
            if (astRoot) {
                astRoot->print(0); // Começa a imprimir no nível 0 (sem recuo)
            }
            std::cout << "===============================================\n\n";
        }
        
    } catch (const std::exception& e) {
        // Se o parser estourar um erro sintático, ele é capturado aqui
        std::cerr << "Falha na compilação.\n";
    }

    // Se a flag -ts estiver presente, nós disparamos a impressão da tabela
    if (tem_flag(args, "-ts")) {
        std::cout << parser.getSymbolTable().toString(); // Vai precisar de um getter simples no Parser.hpp!
    }    

    return 0;
}