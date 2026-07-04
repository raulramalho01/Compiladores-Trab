#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>

// Tipo-sentinela: indica "tipo desconhecido/com erro".
// Serve para SUPRIMIR erros em cascata (um erro nao gera dez outros).
static const std::string TIPO_ERRO = "$err";

// ==========================================================
// Informações coletadas na 1a passada (tabela de classes)
// ==========================================================
struct MethodSig {
    std::string returnType;
    std::vector<std::string> paramTypes;
    std::vector<std::string> paramNames;
    std::string declaringClass;
};

struct ClassInfo {
    std::string name;
    std::string parent; // "" se não houver herança
    std::unordered_map<std::string, std::string> fields;     // nome -> tipo
    std::unordered_map<std::string, MethodSig> methods;      // nome -> assinatura
};

// ==========================================================
// Tabela global de classes (grafo de herança)
// ==========================================================
class ClassTable {
private:
    std::unordered_map<std::string, ClassInfo> classes;

public:
    bool addClass(const std::string& name, const std::string& parent) {
        if (classes.count(name)) return false; // classe duplicada
        classes[name] = ClassInfo{name, parent, {}, {}};
        return true;
    }

    ClassInfo* get(const std::string& name) {
        auto it = classes.find(name);
        return it == classes.end() ? nullptr : &it->second;
    }

    bool isDefined(const std::string& name) const {
        return classes.count(name) > 0;
    }

    // Procura um campo subindo na hierarquia de herança (herança de atributos)
    std::string resolveFieldType(const std::string& className, const std::string& field) {
        std::string cur = className;
        std::unordered_set<std::string> visited;
        while (!cur.empty() && classes.count(cur) && !visited.count(cur)) {
            visited.insert(cur);
            auto& ci = classes[cur];
            auto it = ci.fields.find(field);
            if (it != ci.fields.end()) return it->second;
            cur = ci.parent;
        }
        return ""; // não encontrado
    }

    // Procura um método subindo na hierarquia (base do despacho dinâmico)
    const MethodSig* resolveMethod(const std::string& className, const std::string& method) {
        std::string cur = className;
        std::unordered_set<std::string> visited;
        while (!cur.empty() && classes.count(cur) && !visited.count(cur)) {
            visited.insert(cur);
            auto& ci = classes[cur];
            auto it = ci.methods.find(method);
            if (it != ci.methods.end()) return &it->second;
            cur = ci.parent;
        }
        return nullptr;
    }

    // 'sub' é subtipo de 'super'? (igualdade ou cadeia de herança)

    bool isSubtype(const std::string& sub, const std::string& super) {
        if (sub == super) return true;
        if (!classes.count(sub) || !classes.count(super)) return false; // primitivos só batem
        std::string cur = sub;
        std::unordered_set<std::string> visited;
        while (!cur.empty() && classes.count(cur) && !visited.count(cur)) {
            if (cur == super) return true;
            visited.insert(cur);
            cur = classes[cur].parent;
        }
        return false;
    }

        // 'source' pode ser atribuído a um destino do tipo 'target'?

    bool assignableTo(const std::string& target, const std::string& source) {
        if (target == TIPO_ERRO || source == TIPO_ERRO) return true; // suprime cascata
        if (target == source) return true;
        return isSubtype(source, target);
    }

    // Verifica a hierarquia e ACUMULA os erros encontrados em 'out'
    void verify(std::vector<std::string>& out) {
        for (auto& kv : classes) {
            ClassInfo& ci = kv.second;

            // 1. Pai precisa existir
            if (!ci.parent.empty() && !classes.count(ci.parent)) {
                out.push_back("Erro Semantico: A classe '" + ci.name +
                    "' tenta herdar de '" + ci.parent + "', que nao foi declarada.");
                continue; // sem pai valido, nao da para checar ciclo/override deste
            }

            // 2. Sem ciclos de heranca
            {
                std::unordered_set<std::string> visited;
                std::string cur = ci.name;
                bool ciclo = false;
                while (!cur.empty() && classes.count(cur)) {
                    if (visited.count(cur)) { ciclo = true; break; }
                    visited.insert(cur);
                    cur = classes[cur].parent;
                }
                if (ciclo) {
                    out.push_back("Erro Semantico: Ciclo de heranca detectado envolvendo a classe '" + ci.name + "'.");
                    continue;
                }
            }

            // 3. Override compatível: se um ancestral define o mesmo método,
            //    a assinatura (retorno + parâmetros) deve ser idêntica.
            std::string cur = ci.parent;
            std::unordered_set<std::string> seen;
            while (!cur.empty() && classes.count(cur) && !seen.count(cur)) {
                seen.insert(cur);
                ClassInfo& anc = classes[cur];
                for (auto& m : ci.methods) {
                    auto it = anc.methods.find(m.first);
                    if (it != anc.methods.end()) {
                        if (m.second.returnType != it->second.returnType ||
                            m.second.paramTypes != it->second.paramTypes) {
                            out.push_back("Erro Semantico: O metodo '" + m.first +
                                "' em '" + ci.name + "' sobrescreve '" + cur +
                                "' com assinatura incompativel (override invalido).");
                        }
                    }
                }
                cur = anc.parent;
            }
        }
    }
};

// ==========================================================
// Contexto da 2a passada (checagem de tipos / escopos / init)
// ==========================================================
class SemanticContext {
public:
    ClassTable& classes;
    std::string currentClass;
    std::string currentReturnType;

        // Pilha de escopos de variáveis (nome -> tipo)

    std::vector<std::unordered_map<std::string, std::string>> envStack;
    std::set<std::string> declaredLocals;
    std::set<std::string> assignedVars;

    // Lista de TODOS os erros semanticos encontrados (modo "coletar tudo")
    std::vector<std::string> errors;

    SemanticContext(ClassTable& ct) : classes(ct) {}

    void error(const std::string& msg) { errors.push_back("Erro Semantico: " + msg); }

    void pushScope() { envStack.push_back({}); }
    void popScope()  { if (!envStack.empty()) envStack.pop_back(); }

    void declare(const std::string& name, const std::string& type) {
        if (!envStack.empty()) envStack.back()[name] = type;
    }

        // Resolve uma variável de dentro para fora

    std::string lookup(const std::string& name) {
        for (auto it = envStack.rbegin(); it != envStack.rend(); ++it) {
            auto f = it->find(name);
            if (f != it->end()) return f->second;
        }
        return "";
    }

    bool isLocal(const std::string& name)    { return declaredLocals.count(name) > 0; }
    bool isAssigned(const std::string& name) { return assignedVars.count(name) > 0; }
    void markAssigned(const std::string& name) { assignedVars.insert(name); }
};

inline std::set<std::string> intersectAssigned(const std::set<std::string>& a,
                                               const std::set<std::string>& b) {
    std::set<std::string> r;
    for (const auto& x : a) if (b.count(x)) r.insert(x);
    return r;
}