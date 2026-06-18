#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include "SymbolTable.hpp"

// ==========================================================
// 1. CLASSE BASE DA ÁRVORE
// ==========================================================
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int nivel) const = 0;
    virtual std::string checkSemantic(SymbolTable& st) = 0;

protected:
    void imprimirIndentacao(int nivel) const {
        for (int i = 0; i < nivel; ++i) {
            std::cout << "  |";
        }
        std::cout << "__";
    }
};

// ==========================================================
// 2. EXPRESSÕES (Exp)
// ==========================================================
class ExpNode : public ASTNode {};

class IntLiteralNode : public ExpNode {
    int value;
public:
    IntLiteralNode(int val) : value(val) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "IntLiteral: " << value << "\n";
    }
    std::string checkSemantic(SymbolTable& st) override { return "int"; }
};

class BoolLiteralNode : public ExpNode {
    bool value;
public:
    BoolLiteralNode(bool val) : value(val) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "BoolLiteral: " << (value ? "true" : "false") << "\n";
    }
    std::string checkSemantic(SymbolTable& st) override { return "boolean"; }
};

class IdExpNode : public ExpNode {
    std::string name;
    std::string type; // NOVO: O nó agora guarda o seu próprio tipo!
public:
    IdExpNode(std::string n, std::string t) : name(n), type(t) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "IdExp: " << name << "\n";
    }

    std::string checkSemantic(SymbolTable& st) override {
        // Não precisa mais consultar a tabela aqui, ele já sabe quem ele é!
        return type; 
    }
};

class BinOpNode : public ExpNode {
    std::string op;
    std::unique_ptr<ExpNode> left;
    std::unique_ptr<ExpNode> right;
public:
    BinOpNode(std::string oper, std::unique_ptr<ExpNode> l, std::unique_ptr<ExpNode> r)
        : op(oper), left(std::move(l)), right(std::move(r)) {}
    
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "BinOp [" << op << "]\n";
        if (left) left->print(nivel + 1);
        if (right) right->print(nivel + 1);
    }

    std::string checkSemantic(SymbolTable& st) override {
        std::string tLeft = left->checkSemantic(st);
        std::string tRight = right->checkSemantic(st);

        if (op == "+" || op == "-" || op == "*") {
            if (tLeft != "int" || tRight != "int") {
                throw std::runtime_error("Erro Semântico: Operador '" + op + "' exige operandos do tipo 'int'.");
            }
            return "int";
        } else if (op == "<") {
            if (tLeft != "int" || tRight != "int") {
                throw std::runtime_error("Erro Semântico: Operador '<' exige operandos do tipo 'int'.");
            }
            return "boolean";
        } else if (op == "&&") {
            if (tLeft != "boolean" || tRight != "boolean") {
                throw std::runtime_error("Erro Semântico: Operador '&&' exige operandos do tipo 'boolean'.");
            }
            return "boolean";
        }
        return "void";
    }
};

class UnaryOpNode : public ExpNode {
    std::string op;
    std::unique_ptr<ExpNode> exp;
public:
    UnaryOpNode(std::string oper, std::unique_ptr<ExpNode> expression)
        : op(oper), exp(std::move(expression)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "UnaryOp [" << op << "]\n";
        if (exp) exp->print(nivel + 1);
    }
    std::string checkSemantic(SymbolTable& st) override { return exp->checkSemantic(st); }
};

class ThisNode : public ExpNode {
public:
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "ThisExp\n";
    }
    std::string checkSemantic(SymbolTable& st) override { return "this"; }
};

class ArrayAccessNode : public ExpNode {
    std::unique_ptr<ExpNode> arrayExp;
    std::unique_ptr<ExpNode> indexExp;
public:
    ArrayAccessNode(std::unique_ptr<ExpNode> arr, std::unique_ptr<ExpNode> idx)
        : arrayExp(std::move(arr)), indexExp(std::move(idx)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "ArrayAccess\n";
        if (arrayExp) arrayExp->print(nivel + 1);
        if (indexExp) indexExp->print(nivel + 1);
    }
    std::string checkSemantic(SymbolTable& st) override { 
        arrayExp->checkSemantic(st); 
        indexExp->checkSemantic(st); 
        return "int"; 
    }
};

// ==========================================================
// 3. COMANDOS (Cmd)
// ==========================================================
class CmdNode : public ASTNode {};

// ... (dentro de COMANDOS) ...
class AssignNode : public CmdNode {
    std::string id;
    std::string varType; // NOVO: Guarda o tipo da variável que vai receber o valor
    std::unique_ptr<ExpNode> exp;
public:
    AssignNode(std::string name, std::string vType, std::unique_ptr<ExpNode> expression)
        : id(name), varType(vType), exp(std::move(expression)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Assign: " << id << "\n";
        if (exp) exp->print(nivel + 1);
    }

    std::string checkSemantic(SymbolTable& st) override {
        std::string expType = exp->checkSemantic(st);
        
        // Compara diretamente com o tipo salvo na criação do nó
        if (varType != expType) {
            throw std::runtime_error("Erro Semântico: Incompatibilidade de tipos na variável '" + id + "'. Esperado '" + varType + "', recebido '" + expType + "'.");
        }
        return "void";
    }
};

class ArrayAssignNode : public CmdNode {
    std::string id;
    std::string varType;
    std::unique_ptr<ExpNode> indexExp; // Guarda o índice do vetor
    std::unique_ptr<ExpNode> valueExp; // Guarda o valor a ser atribuído
public:
    ArrayAssignNode(std::string name, std::string vType, std::unique_ptr<ExpNode> idx, std::unique_ptr<ExpNode> val)
        : id(name), varType(vType), indexExp(std::move(idx)), valueExp(std::move(val)) {}

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "ArrayAssign: " << id << "\n";
        if (indexExp) indexExp->print(nivel + 1);
        if (valueExp) valueExp->print(nivel + 1);
    }

    std::string checkSemantic(SymbolTable& st) override {
        // O índice obrigatoriamente tem que ser um inteiro
        if (indexExp->checkSemantic(st) != "int") {
            throw std::runtime_error("Erro Semântico: O índice do vetor '" + id + "' deve ser do tipo 'int'.");
        }
        
        // Se a variável é int[], o valor atribuído a ela deve ser int
        std::string expectedType = varType.substr(0, varType.length() - 2); // Transforma "int[]" em "int"
        std::string valType = valueExp->checkSemantic(st);
        
        if (expectedType != valType) {
            throw std::runtime_error("Erro Semântico: Incompatibilidade de tipos na atribuição do vetor '" + id + "'. Esperado '" + expectedType + "', recebido '" + valType + "'.");
        }
        return "void";
    }
};

class PrintNode : public CmdNode {
    std::unique_ptr<ExpNode> exp;
public:
    PrintNode(std::unique_ptr<ExpNode> expression) : exp(std::move(expression)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Print\n";
        if (exp) exp->print(nivel + 1);
    }
    std::string checkSemantic(SymbolTable& st) override {
        if (exp) exp->checkSemantic(st);
        return "void";
    }
};

class IfNode : public CmdNode {
    std::unique_ptr<ExpNode> condition;
    std::vector<std::unique_ptr<CmdNode>> ifBlock;
    std::vector<std::unique_ptr<CmdNode>> elseBlock;
public:
    IfNode(std::unique_ptr<ExpNode> cond, std::vector<std::unique_ptr<CmdNode>> ifB, std::vector<std::unique_ptr<CmdNode>> elseB)
        : condition(std::move(cond)), ifBlock(std::move(ifB)), elseBlock(std::move(elseB)) {}

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "If\n";
        if (condition) condition->print(nivel + 1);
        
        imprimirIndentacao(nivel); std::cout << "Then:\n";
        for (const auto& cmd : ifBlock) if (cmd) cmd->print(nivel + 1);
        
        if (!elseBlock.empty()) {
            imprimirIndentacao(nivel); std::cout << "Else:\n";
            for (const auto& cmd : elseBlock) if (cmd) cmd->print(nivel + 1);
        }
    }

    std::string checkSemantic(SymbolTable& st) override {
        if (condition) {
            std::string condType = condition->checkSemantic(st);
            if (condType != "boolean") {
                throw std::runtime_error("Erro Semântico: A condição do 'if' deve ser do tipo 'boolean'.");
            }
        }
        for (auto& cmd : ifBlock) if (cmd) cmd->checkSemantic(st);
        for (auto& cmd : elseBlock) if (cmd) cmd->checkSemantic(st);
        return "void";
    }
};

class WhileNode : public CmdNode {
    std::unique_ptr<ExpNode> condition;
    std::vector<std::unique_ptr<CmdNode>> block;
public:
    WhileNode(std::unique_ptr<ExpNode> cond, std::vector<std::unique_ptr<CmdNode>> blk)
        : condition(std::move(cond)), block(std::move(blk)) {}

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "While\n";
        if (condition) condition->print(nivel + 1);
        
        imprimirIndentacao(nivel); std::cout << "Do:\n";
        for (const auto& cmd : block) if (cmd) cmd->print(nivel + 1);
    }

    std::string checkSemantic(SymbolTable& st) override {
        if (condition) {
            std::string condType = condition->checkSemantic(st);
            if (condType != "boolean") {
                throw std::runtime_error("Erro Semântico: A condição do 'while' deve ser do tipo 'boolean'.");
            }
        }
        for (auto& cmd : block) if (cmd) cmd->checkSemantic(st);
        return "void";
    }
};

// ==========================================================
// 4. ESTRUTURAS GLOBAIS (Métodos, Classes e Programa)
// ==========================================================
class MethodNode : public ASTNode {
    std::string name;
    std::string returnType;
    std::vector<std::unique_ptr<CmdNode>> commands;
    std::unique_ptr<ExpNode> returnExp;
public:
    MethodNode(std::string n, std::string t, std::vector<std::unique_ptr<CmdNode>> cmds, std::unique_ptr<ExpNode> ret)
        : name(n), returnType(t), commands(std::move(cmds)), returnExp(std::move(ret)) {}

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Method [" << returnType << " " << name << "()]\n";
        for (const auto& cmd : commands) if (cmd) cmd->print(nivel + 1);
        
        imprimirIndentacao(nivel + 1); std::cout << "Return:\n";
        if (returnExp) returnExp->print(nivel + 2);
    }

    std::string checkSemantic(SymbolTable& st) override {
        for (auto& cmd : commands) {
            if (cmd) cmd->checkSemantic(st);
        }
        if (returnExp) {
            std::string retType = returnExp->checkSemantic(st);
            if (retType != returnType) {
                throw std::runtime_error("Erro Semântico: O método '" + name + "' prometeu retornar '" + returnType + "', mas tentou retornar '" + retType + "'.");
            }
        }
        return "void";
    }
};

class ClassNode : public ASTNode {
    std::string name;
    std::string parent;
    std::vector<std::unique_ptr<MethodNode>> methods;
public:
    ClassNode(std::string n, std::string p, std::vector<std::unique_ptr<MethodNode>> m)
        : name(n), parent(p), methods(std::move(m)) {}

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Class " << name << (parent.empty() ? "" : " extends " + parent) << "\n";
        for (const auto& met : methods) if (met) met->print(nivel + 1);
    }

    std::string checkSemantic(SymbolTable& st) override {
        if (methods.empty()) {
            throw std::runtime_error("Erro Semântico: A classe '" + name + "' não pode ser vazia (sem atributos ou métodos).");
        }
        for (auto& met : methods) {
            if (met) met->checkSemantic(st);
        }
        return "void";
    }
};

class MainClassNode : public ASTNode {
    std::string name;
    std::vector<std::unique_ptr<CmdNode>> commands;
public:
    MainClassNode(std::string n, std::vector<std::unique_ptr<CmdNode>> cmds)
        : name(n), commands(std::move(cmds)) {}

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "MainClass " << name << "\n";
        for (const auto& cmd : commands) if (cmd) cmd->print(nivel + 1);
    }

    std::string checkSemantic(SymbolTable& st) override {
        for (auto& cmd : commands) {
            if (cmd) cmd->checkSemantic(st);
        }
        return "void";
    }
};

class ProgNode : public ASTNode {
    std::unique_ptr<MainClassNode> mainClass;
    std::vector<std::unique_ptr<ClassNode>> classes;
public:
    ProgNode(std::unique_ptr<MainClassNode> mc, std::vector<std::unique_ptr<ClassNode>> cls)
        : mainClass(std::move(mc)), classes(std::move(cls)) {}
    
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "ProgramRoot\n";
        if (mainClass) mainClass->print(nivel + 1);
        for (const auto& c : classes) {
            if (c) c->print(nivel + 1);
        }
    }

    std::string checkSemantic(SymbolTable& st) override {
        if (mainClass) mainClass->checkSemantic(st);
        for (auto& c : classes) {
            if (c) c->checkSemantic(st);
        }
        return "void";
    }
};