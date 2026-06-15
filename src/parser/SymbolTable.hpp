#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

// Estrutura para guardar os detalhes de cada identificador
struct SymbolInfo {
    std::string type;
    std::string scopeLevel; // Ex: "Classe", "Método", "Parâmetro", "Local"
};

class SymbolTable {
private:
    // A nossa Pilha de Escopos. O último elemento (.back()) é sempre o escopo atual.
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;
    
    // Auxiliar apenas para o toString() imprimir nomes bonitos (ex: "Escopo: Classe Mago")
    std::vector<std::string> scopeNames; 

    // NOVO: Guarda o texto dos escopos que já foram fechados e removidos da pilha
    std::string tableHistory = "";

public:
    SymbolTable();

    // Entra em um novo nível de chaves {
    void enterScope(const std::string& scopeName);
    
    // Sai de um nível de chaves }
    void exitScope();

    // Adiciona um símbolo APENAS no escopo atual (topo da pilha)
    bool add(const std::string& name, const std::string& type, const std::string& scopeLevel);

    // Procura um símbolo de dentro para fora (Top-Down na pilha)
    SymbolInfo* resolve(const std::string& name);

    // Imprime a tabela para a flag do terminal
    std::string toString() const;
};