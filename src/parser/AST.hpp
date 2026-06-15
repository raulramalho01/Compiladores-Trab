#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>

// ==========================================================
// 1. CLASSE BASE DA ÁRVORE
// ==========================================================
class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    // Todo nó da árvore é obrigado a saber se imprimir
    virtual void print(int nivel) const = 0;

protected:
    // Função auxiliar para desenhar os galhos da árvore no terminal
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

// Nó para números literais (ex: 10, 42)
class IntLiteralNode : public ExpNode {
    int value;
public:
    IntLiteralNode(int val) : value(val) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "IntLiteral: " << value << "\n";
    }
};

// Nó para identificadores (ex: x, soma, mana)
class IdExpNode : public ExpNode {
    std::string name;
public:
    IdExpNode(std::string n) : name(n) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "IdExp: " << name << "\n";
    }
};

// Nó para Operações Binárias (ex: a + b, x < 10)
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
};

// Nó para Operadores Unários (ex: !ativo)
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
};

// Nó para Literais Booleanos (true / false)
class BoolLiteralNode : public ExpNode {
    bool value;
public:
    BoolLiteralNode(bool val) : value(val) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "BoolLiteral: " << (value ? "true" : "false") << "\n";
    }
};

// Nó para a palavra-chave 'this'
class ThisNode : public ExpNode {
public:
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "ThisExp\n";
    }
};

// Nó para Acesso a Vetor (ex: array[10])
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
};

// ==========================================================
// 3. COMANDOS (Cmd)
// ==========================================================
class CmdNode : public ASTNode {};

// Nó de Atribuição (ex: x = 10;)
class AssignNode : public CmdNode {
    std::string id;
    std::unique_ptr<ExpNode> exp;
public:
    AssignNode(std::string name, std::unique_ptr<ExpNode> expression)
        : id(name), exp(std::move(expression)) {}

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Assign: " << id << "\n";
        if (exp) exp->print(nivel + 1);
    }
};

// Nó de Impressão (ex: System.out.println(10);)
class PrintNode : public CmdNode {
    std::unique_ptr<ExpNode> exp;
public:
    PrintNode(std::unique_ptr<ExpNode> expression) : exp(std::move(expression)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Print\n";
        if (exp) exp->print(nivel + 1);
    }
};

// ==========================================================
// 4. ESTRUTURAS GLOBAIS (Métodos, Classes e Programa)
// ==========================================================

// ==========================================================
// ESTRUTURAS DE CONTROLE (Comandos Complexos)
// ==========================================================

// Nó de IF / ELSE
class IfNode : public CmdNode {
    std::unique_ptr<ExpNode> condition;
    std::vector<std::unique_ptr<CmdNode>> ifBlock;
    std::vector<std::unique_ptr<CmdNode>> elseBlock; // Pode ser vazio
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
};

// Nó de WHILE
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
};

// ==========================================================
// ESTRUTURAS DE CLASSE E MÉTODOS
// ==========================================================

// Nó de Método
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
};

// Nó de Classe Comum
class ClassNode : public ASTNode {
    std::string name;
    std::string parent; // Pode ser vazio caso não tenha 'extends'
    std::vector<std::unique_ptr<MethodNode>> methods;
public:
    ClassNode(std::string n, std::string p, std::vector<std::unique_ptr<MethodNode>> m)
        : name(n), parent(p), methods(std::move(m)) {}

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Class " << name << (parent.empty() ? "" : " extends " + parent) << "\n";
        for (const auto& met : methods) if (met) met->print(nivel + 1);
    }
};

// Nó da Classe Main
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
};

// Nó do Programa Principal (Substitua o antigo por este)
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
};