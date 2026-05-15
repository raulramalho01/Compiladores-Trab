#include "preprocessor.hpp"
#include "../lexer/Lexer.hpp"
#include "../parser/Parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

// Função que executa o pipeline completo para um único arquivo
void compilar_arquivo(const std::string& arquivo_entrada, const std::string& arquivo_saida) {
    std::cout << "========================================================\n";
    std::cout << ">>> COMPILANDO: " << arquivo_entrada << "\n";
    std::cout << "========================================================\n";

    try {
        // 1. Passa pelo Pré-processador (Cria o arquivo limpo na pasta bin)
        preprocess_file(arquivo_entrada, arquivo_saida);

        // 2. Lê todo o conteúdo do arquivo limpo recém-gerado
        std::ifstream inFile(arquivo_saida);
        if (!inFile.is_open()) {
            std::cerr << "Erro: Não foi possível ler o arquivo gerado: " << arquivo_saida << "\n";
            return;
        }
        
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        std::string input = buffer.str();

        // 3. Analisador Léxico
        Lexer lexer(input);
        std::vector<Token> tokens = lexer.tokenize();

        // 4. Analisador Sintático (já embute a Tabela de Símbolos)
        Parser parser(tokens);
        parser.parse();
        
    } catch (const std::exception& e) {
        std::cerr << "Erro inesperado ao compilar " << arquivo_entrada << ": " << e.what() << "\n";
    }
    
    std::cout << "\n\n"; // Espaço entre um teste e outro
}

int main() {
    // Processa todos os arquivos em lote (Batch processing)
    compilar_arquivo("input1.java", "src/bin/output.java");
    compilar_arquivo("Program1.ling", "src/bin/Program1.java");
    compilar_arquivo("Program2.ling", "src/bin/Program2.java");
    compilar_arquivo("Program3.ling", "src/bin/Program3.java");
    compilar_arquivo("Program4.ling", "src/bin/Program4.java");
    compilar_arquivo("Program5.ling", "src/bin/Program5.java");

    return 0;
}