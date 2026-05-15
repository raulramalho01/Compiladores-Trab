#include "SymbolTable.hpp"
#include <sstream>
#include <iomanip>

SymbolTable::SymbolTable() = default;

void SymbolTable::add(const std::string& name, const std::string& type) {
    table[name] = type;
}

bool SymbolTable::exists(const std::string& name) const {
    return table.find(name) != table.end();
}

std::string SymbolTable::toString() const {
    std::ostringstream sb;
    sb << "=== TABELA DE SÍMBOLOS ===\n";
    for (const auto& pair : table) {
        // Formatação equivalente ao %-15s do Java
        sb << "Variável: ";
        sb << std::left << std::setw(15) << pair.first;
        sb << " | Tipo: " << pair.second << "\n";
    }
    return sb.str();
}