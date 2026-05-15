#pragma once
#include <string>
#include <unordered_map>

class SymbolTable {
private:
    // Guarda o nome do Identificador (chave) e o Tipo (valor)
    std::unordered_map<std::string, std::string> table;

public:
    SymbolTable();
    void add(const std::string& name, const std::string& type);
    bool exists(const std::string& name) const;
    std::string toString() const;
};