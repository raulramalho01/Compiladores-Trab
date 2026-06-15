#include "SymbolTable.hpp"
#include <sstream>

SymbolTable::SymbolTable() {
    // Já começa com o escopo "Global" aberto na base da pilha
    enterScope("Global"); 
}

void SymbolTable::enterScope(const std::string& scopeName) {
    scopes.push_back(std::unordered_map<std::string, SymbolInfo>());
    scopeNames.push_back(scopeName);
}

bool SymbolTable::add(const std::string& name, const std::string& type, const std::string& scopeLevel) {
    auto& currentScope = scopes.back();
    
    // Verifica se a variável já existe no MESMO escopo (não pode declarar 'int x' duas vezes no mesmo método)
    if (currentScope.find(name) != currentScope.end()) {
        return false; // Erro: Símbolo já declarado neste escopo
    }

    currentScope[name] = {type, scopeLevel};
    return true;
}

SymbolInfo* SymbolTable::resolve(const std::string& name) {
    // Procura de trás pra frente (rbegin até rend)
    // Isso garante que ele ache Parâmetros > Atributos > Globais
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &(found->second); // Retorna o endereço da informação encontrada
        }
    }
    return nullptr; // Símbolo não existe em nenhum escopo ativo
}

void SymbolTable::exitScope() {
    if (scopes.size() > 1) { // Nunca removemos o escopo Global
        
        // NOVO: "Tira uma foto" do escopo antes de destruí-lo
        std::ostringstream oss;
        int topo = scopes.size() - 1;
        oss << ">> Escopo: " << scopeNames[topo] << " <<\n";
        if (scopes[topo].empty()) {
            oss << "  (Vazio)\n";
        } else {
            for (const auto& pair : scopes[topo]) {
                oss << "  - Nome: " << pair.first 
                    << " | Tipo: " << pair.second.type 
                    << " | Nível: " << pair.second.scopeLevel << "\n";
            }
        }
        oss << "\n";
        
        // Salva a foto no histórico (colocando no topo para ficar em ordem de leitura)
        tableHistory = oss.str() + tableHistory; 

        // Destrói o escopo normalmente
        scopes.pop_back();
        scopeNames.pop_back();
    }
}

std::string SymbolTable::toString() const {
    std::ostringstream oss;
    oss << "\n=== TABELA DE SÍMBOLOS ===\n";
    
    // Imprime o escopo ativo (que no final será só o Global)
    for (size_t i = 0; i < scopes.size(); ++i) {
        oss << ">> Escopo: " << scopeNames[i] << " <<\n";
        if (scopes[i].empty()) {
            oss << "  (Vazio)\n";
        } else {
            for (const auto& pair : scopes[i]) {
                oss << "  - Nome: " << pair.first 
                    << " | Tipo: " << pair.second.type 
                    << " | Nível: " << pair.second.scopeLevel << "\n";
            }
        }
        oss << "\n";
    }
    
    // NOVO: Adiciona todas as "fotos" dos escopos que foram fechados durante o parse
    oss << tableHistory;
    
    oss << "==========================\n";
    return oss.str();
}