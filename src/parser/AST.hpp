#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include "SemanticContext.hpp"

// Declaração simples de variável/campo/parâmetro (tipo + nome)
struct VarDecl {
    std::string type;
    std::string name;
};

// ==========================================================
// 1. CLASSE BASE DA ÁRVORE
// ==========================================================
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int nivel) const = 0;
    virtual std::string checkSemantic(SemanticContext& ctx) = 0;

protected:
    void imprimirIndentacao(int nivel) const {
        for (int i = 0; i < nivel; ++i) std::cout << "  |";
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
    std::string checkSemantic(SemanticContext&) override { return "int"; }
};

class BoolLiteralNode : public ExpNode {
    bool value;
public:
    BoolLiteralNode(bool val) : value(val) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "BoolLiteral: " << (value ? "true" : "false") << "\n";
    }
    std::string checkSemantic(SemanticContext&) override { return "boolean"; }
};

class IdExpNode : public ExpNode {
    std::string name;
public:
    IdExpNode(std::string n) : name(std::move(n)) {}
    const std::string& getName() const { return name; }
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "IdExp: " << name << "\n";
    }
    std::string checkSemantic(SemanticContext& ctx) override {

        std::string t = ctx.lookup(name);
        if (t.empty()) {
            // Tenta como campo herdado diretamente pela hierarquia de classes
            t = ctx.classes.resolveFieldType(ctx.currentClass, name);
        }
        if (t.empty()) {
            throw std::runtime_error("Erro Semantico: Variavel '" + name + "' nao declarada.");
        }
        // Garantia de inicialização: se for uma variável local, precisa ter sido atribuída
        if (ctx.isLocal(name) && !ctx.isAssigned(name)) {
            throw std::runtime_error("Erro Semantico: A variavel local '" + name +
                "' pode ser usada antes de ser inicializada.");
        }
        return t;
    }
};

class ThisNode : public ExpNode {
public:
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "ThisExp\n";
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        if (ctx.currentClass.empty()) {
            throw std::runtime_error("Erro Semantico: 'this' usado fora de uma classe.");
        }
        return ctx.currentClass;
    }
};

class BinOpNode : public ExpNode {
    std::string op;
    std::unique_ptr<ExpNode> left;
    std::unique_ptr<ExpNode> right;
public:
    BinOpNode(std::string oper, std::unique_ptr<ExpNode> l, std::unique_ptr<ExpNode> r)
        : op(std::move(oper)), left(std::move(l)), right(std::move(r)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "BinOp [" << op << "]\n";
        if (left) left->print(nivel + 1);
        if (right) right->print(nivel + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string tLeft = left->checkSemantic(ctx);
        std::string tRight = right->checkSemantic(ctx);
        if (op == "+" || op == "-" || op == "*") {
            if (tLeft != "int" || tRight != "int")
                throw std::runtime_error("Erro Semantico: Operador '" + op + "' exige operandos do tipo 'int'.");
            return "int";
        } else if (op == "<") {
            if (tLeft != "int" || tRight != "int")
                throw std::runtime_error("Erro Semantico: Operador '<' exige operandos do tipo 'int'.");
            return "boolean";
        } else if (op == "&&") {
            if (tLeft != "boolean" || tRight != "boolean")
                throw std::runtime_error("Erro Semantico: Operador '&&' exige operandos do tipo 'boolean'.");
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
        : op(std::move(oper)), exp(std::move(expression)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "UnaryOp [" << op << "]\n";
        if (exp) exp->print(nivel + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string t = exp->checkSemantic(ctx);
        if (op == "!") {
            if (t != "boolean")
                throw std::runtime_error("Erro Semantico: Operador '!' exige operando do tipo 'boolean'.");
            return "boolean";
        }
        return t;
    }
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
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string tArr = arrayExp->checkSemantic(ctx);
        std::string tIdx = indexExp->checkSemantic(ctx);
        if (tArr != "int[]")
            throw std::runtime_error("Erro Semantico: Acesso indexado exige um vetor 'int[]'.");
        if (tIdx != "int")
            throw std::runtime_error("Erro Semantico: O indice de um vetor deve ser do tipo 'int'.");
        return "int";
    }
};

class LengthNode : public ExpNode {
    std::unique_ptr<ExpNode> arrayExp;
public:
    LengthNode(std::unique_ptr<ExpNode> arr) : arrayExp(std::move(arr)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Length\n";
        if (arrayExp) arrayExp->print(nivel + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        if (arrayExp->checkSemantic(ctx) != "int[]")
            throw std::runtime_error("Erro Semantico: '.length' so pode ser aplicado a um vetor 'int[]'.");
        return "int";
    }
};

class NewArrayNode : public ExpNode {
    std::unique_ptr<ExpNode> sizeExp;
public:
    NewArrayNode(std::unique_ptr<ExpNode> sz) : sizeExp(std::move(sz)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "NewArray (int[])\n";
        if (sizeExp) sizeExp->print(nivel + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        if (sizeExp->checkSemantic(ctx) != "int")
            throw std::runtime_error("Erro Semantico: O tamanho de 'new int[]' deve ser do tipo 'int'.");
            
        std::cout << "  [INFO SEMANTICO] 'new int[]': Memoria simulada alocada. Posicoes inicializadas com 0.\n";
        
        return "int[]";
    }
};

class NewObjectNode : public ExpNode {
    std::string className;
public:
    NewObjectNode(std::string n) : className(std::move(n)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "NewObject [" << className << "]\n";
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        if (!ctx.classes.isDefined(className))
            throw std::runtime_error("Erro Semantico: Tentativa de instanciar a classe '" +
                className + "', que nao foi declarada.");
        
        // NOVO: Mensagem de log para mostrar que a regra 4.6 foi cumprida!
        std::cout << "  [INFO SEMANTICO] 'new " << className << "()': Memoria simulada alocada. "
                  << "Campos inicializados com padrao (0, false, null).\n";
        
        return className;
    }
};

class MethodCallNode : public ExpNode {
    std::unique_ptr<ExpNode> receiver;
    std::string method;
    std::vector<std::unique_ptr<ExpNode>> args;
public:
    MethodCallNode(std::unique_ptr<ExpNode> recv, std::string m,
                   std::vector<std::unique_ptr<ExpNode>> a)
        : receiver(std::move(recv)), method(std::move(m)), args(std::move(a)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "MethodCall [" << method << "]\n";
        imprimirIndentacao(nivel + 1); std::cout << "Receiver:\n";
        if (receiver) receiver->print(nivel + 2);
        if (!args.empty()) {
            imprimirIndentacao(nivel + 1); std::cout << "Args:\n";
            for (const auto& a : args) if (a) a->print(nivel + 2);
        }
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string recvType = receiver->checkSemantic(ctx);
        if (!ctx.classes.isDefined(recvType)) {
            throw std::runtime_error("Erro Semantico: Chamada do metodo '" + method +
                "' sobre algo que nao e um objeto (tipo '" + recvType + "').");
        }
        // Despacho: resolve o método subindo na hierarquia a partir do tipo do receptor
        const MethodSig* sig = ctx.classes.resolveMethod(recvType, method);
        if (!sig) {
            throw std::runtime_error("Erro Semantico: O metodo '" + method +
                "' nao existe na classe '" + recvType + "' nem em suas superclasses.");
        }
        if (args.size() != sig->paramTypes.size()) {
            throw std::runtime_error("Erro Semantico: O metodo '" + method + "' espera " +
                std::to_string(sig->paramTypes.size()) + " argumento(s), mas recebeu " +
                std::to_string(args.size()) + ".");
        }
        for (size_t i = 0; i < args.size(); ++i) {
            std::string at = args[i]->checkSemantic(ctx);
            if (!ctx.classes.assignableTo(sig->paramTypes[i], at)) {
                throw std::runtime_error("Erro Semantico: Argumento " + std::to_string(i + 1) +
                    " do metodo '" + method + "' esperava '" + sig->paramTypes[i] +
                    "', mas recebeu '" + at + "'.");
            }
        }
        return sig->returnType;
    }
};

// ==========================================================
// 3. COMANDOS (Cmd)
// ==========================================================
class CmdNode : public ASTNode {};

class AssignNode : public CmdNode {
    std::string id;
    std::unique_ptr<ExpNode> exp;
public:
    AssignNode(std::string name, std::unique_ptr<ExpNode> expression)
        : id(std::move(name)), exp(std::move(expression)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Assign: " << id << "\n";
        if (exp) exp->print(nivel + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        // Avalia o lado direito ANTES de marcar o destino como inicializado
        std::string expType = exp->checkSemantic(ctx);

        std::string varType = ctx.lookup(id);
        if (varType.empty()) varType = ctx.classes.resolveFieldType(ctx.currentClass, id);
        if (varType.empty())
            throw std::runtime_error("Erro Semantico: Atribuicao a variavel nao declarada '" + id + "'.");

        if (!ctx.classes.assignableTo(varType, expType)) {
            throw std::runtime_error("Erro Semantico: Incompatibilidade de tipos na variavel '" + id +
                "'. Esperado '" + varType + "', recebido '" + expType + "'.");
        }
        // Marca como definitivamente atribuída (definite assignment)
        ctx.markAssigned(id);
        return "void";
    }
};

class ArrayAssignNode : public CmdNode {
    std::string id;
    std::unique_ptr<ExpNode> indexExp;
    std::unique_ptr<ExpNode> valueExp;
public:
    ArrayAssignNode(std::string name, std::unique_ptr<ExpNode> idx, std::unique_ptr<ExpNode> val)
        : id(std::move(name)), indexExp(std::move(idx)), valueExp(std::move(val)) {}
    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "ArrayAssign: " << id << "\n";
        if (indexExp) indexExp->print(nivel + 1);
        if (valueExp) valueExp->print(nivel + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string varType = ctx.lookup(id);
        if (varType.empty()) varType = ctx.classes.resolveFieldType(ctx.currentClass, id);
        if (varType.empty())
            throw std::runtime_error("Erro Semantico: Atribuicao a vetor nao declarado '" + id + "'.");
        if (varType != "int[]")
            throw std::runtime_error("Erro Semantico: A variavel '" + id + "' nao e um vetor 'int[]'.");
        if (ctx.isLocal(id) && !ctx.isAssigned(id))
            throw std::runtime_error("Erro Semantico: O vetor local '" + id +
                "' pode ser usado antes de ser inicializado (faltou 'new int[]').");
        if (indexExp->checkSemantic(ctx) != "int")
            throw std::runtime_error("Erro Semantico: O indice do vetor '" + id + "' deve ser do tipo 'int'.");
        if (valueExp->checkSemantic(ctx) != "int")
            throw std::runtime_error("Erro Semantico: O valor atribuido ao vetor '" + id + "' deve ser do tipo 'int'.");
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
    std::string checkSemantic(SemanticContext& ctx) override {
        if (exp) exp->checkSemantic(ctx);
        return "void";
    }
};

class IfNode : public CmdNode {
    std::unique_ptr<ExpNode> condition;
    std::vector<std::unique_ptr<CmdNode>> ifBlock;
    std::vector<std::unique_ptr<CmdNode>> elseBlock;
public:
    IfNode(std::unique_ptr<ExpNode> cond, std::vector<std::unique_ptr<CmdNode>> ifB,
           std::vector<std::unique_ptr<CmdNode>> elseB)
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
    std::string checkSemantic(SemanticContext& ctx) override {
        if (condition && condition->checkSemantic(ctx) != "boolean")
            throw std::runtime_error("Erro Semantico: A condicao do 'if' deve ser do tipo 'boolean'.");

        // Fluxo de inicialização: uma variável só é "definitivamente atribuída"
        // após o if se for atribuída em AMBOS os ramos.
        std::set<std::string> before = ctx.assignedVars;

        ctx.assignedVars = before;
        for (auto& cmd : ifBlock) if (cmd) cmd->checkSemantic(ctx);
        std::set<std::string> afterThen = ctx.assignedVars;

        ctx.assignedVars = before;
        for (auto& cmd : elseBlock) if (cmd) cmd->checkSemantic(ctx);
        std::set<std::string> afterElse = ctx.assignedVars;

        if (elseBlock.empty())
            ctx.assignedVars = before;
        else
            ctx.assignedVars = intersectAssigned(afterThen, afterElse);
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
    std::string checkSemantic(SemanticContext& ctx) override {
        if (condition && condition->checkSemantic(ctx) != "boolean")
            throw std::runtime_error("Erro Semantico: A condicao do 'while' deve ser do tipo 'boolean'.");
        std::set<std::string> before = ctx.assignedVars;
        for (auto& cmd : block) if (cmd) cmd->checkSemantic(ctx);
        ctx.assignedVars = before;
        return "void";
    }
};

// ==========================================================
// 4. ESTRUTURAS GLOBAIS
// ==========================================================
class MethodNode : public ASTNode {
    std::string name;
    std::string returnType;
    std::vector<VarDecl> params;
    std::vector<VarDecl> locals;
    std::vector<std::unique_ptr<CmdNode>> commands;
    std::unique_ptr<ExpNode> returnExp;
public:
    MethodNode(std::string n, std::string t, std::vector<VarDecl> p, std::vector<VarDecl> l,
               std::vector<std::unique_ptr<CmdNode>> cmds, std::unique_ptr<ExpNode> ret)
        : name(std::move(n)), returnType(std::move(t)), params(std::move(p)),
          locals(std::move(l)), commands(std::move(cmds)), returnExp(std::move(ret)) {}

    const std::string& getName() const { return name; }
    const std::string& getReturnType() const { return returnType; }
    const std::vector<VarDecl>& getParams() const { return params; }

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Method [" << returnType << " " << name << "(";
        for (size_t i = 0; i < params.size(); ++i)
            std::cout << (i ? ", " : "") << params[i].type << " " << params[i].name;
        std::cout << ")]\n";
        for (const auto& cmd : commands) if (cmd) cmd->print(nivel + 1);
        imprimirIndentacao(nivel + 1); std::cout << "Return:\n";
        if (returnExp) returnExp->print(nivel + 2);
    }

    std::string checkSemantic(SemanticContext& ctx) override {
        ctx.pushScope();
        std::string savedReturn = ctx.currentReturnType;
        std::set<std::string> savedLocals = ctx.declaredLocals;
        std::set<std::string> savedAssigned = ctx.assignedVars;

        ctx.currentReturnType = returnType;
        ctx.declaredLocals.clear();
        ctx.assignedVars.clear();

        // Parâmetros: já chegam inicializados
        for (auto& p : params) {
            ctx.declare(p.name, p.type);
            ctx.markAssigned(p.name);
        }
        // Locais: declarados, mas ainda NÃO inicializados
        for (auto& l : locals) {
            ctx.declare(l.name, l.type);
            ctx.declaredLocals.insert(l.name);
        }

        for (auto& cmd : commands) if (cmd) cmd->checkSemantic(ctx);

        if (returnExp) {
            std::string retType = returnExp->checkSemantic(ctx);
            if (!ctx.classes.assignableTo(returnType, retType))
                throw std::runtime_error("Erro Semantico: O metodo '" + name + "' prometeu retornar '" +
                    returnType + "', mas tentou retornar '" + retType + "'.");
        }

        ctx.currentReturnType = savedReturn;
        ctx.declaredLocals = savedLocals;
        ctx.assignedVars = savedAssigned;
        ctx.popScope();
        return "void";
    }
};

class ClassNode : public ASTNode {
    std::string name;
    std::string parent;
    std::vector<VarDecl> fields;
    std::vector<std::unique_ptr<MethodNode>> methods;
public:
    ClassNode(std::string n, std::string p, std::vector<VarDecl> f,
              std::vector<std::unique_ptr<MethodNode>> m)
        : name(std::move(n)), parent(std::move(p)), fields(std::move(f)), methods(std::move(m)) {}

    const std::string& getName() const { return name; }
    const std::string& getParent() const { return parent; }
    const std::vector<VarDecl>& getFields() const { return fields; }
    const std::vector<std::unique_ptr<MethodNode>>& getMethods() const { return methods; }

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "Class " << name << (parent.empty() ? "" : " extends " + parent) << "\n";
        for (const auto& f : fields) {
            imprimirIndentacao(nivel + 1);
            std::cout << "Field: " << f.type << " " << f.name << "\n";
        }
        for (const auto& met : methods) if (met) met->print(nivel + 1);
    }

    std::string checkSemantic(SemanticContext& ctx) override {
        if (methods.empty() && fields.empty())
            throw std::runtime_error("Erro Semantico: A classe '" + name +
                "' nao pode ser vazia (sem atributos ou metodos).");

        ctx.currentClass = name;
        ctx.pushScope();

        // Herança de atributos: empilha os campos desta classe E de todos os
        // ancestrais, para que métodos enxerguem campos herdados.
        std::string cur = name;
        std::set<std::string> visited;
        while (!cur.empty() && !visited.count(cur)) {
            visited.insert(cur);
            ClassInfo* ci = ctx.classes.get(cur);
            if (!ci) break;
            for (auto& fld : ci->fields)
                if (ctx.lookup(fld.first).empty()) // a classe mais derivada tem prioridade
                    ctx.declare(fld.first, fld.second);
            cur = ci->parent;
        }

        for (auto& met : methods) if (met) met->checkSemantic(ctx);

        ctx.popScope();
        ctx.currentClass.clear();
        return "void";
    }
};

class MainClassNode : public ASTNode {
    std::string name;
    std::string argName;
    std::vector<std::unique_ptr<CmdNode>> commands;
public:
    MainClassNode(std::string n, std::string arg, std::vector<std::unique_ptr<CmdNode>> cmds)
        : name(std::move(n)), argName(std::move(arg)), commands(std::move(cmds)) {}

    const std::string& getName() const { return name; }

    void print(int nivel) const override {
        imprimirIndentacao(nivel);
        std::cout << "MainClass " << name << "\n";
        for (const auto& cmd : commands) if (cmd) cmd->print(nivel + 1);
    }

    std::string checkSemantic(SemanticContext& ctx) override {
        ctx.currentClass = name;
        ctx.pushScope();
        ctx.declaredLocals.clear();
        ctx.assignedVars.clear();
        ctx.declare(argName, "String[]");
        ctx.markAssigned(argName);
        for (auto& cmd : commands) if (cmd) cmd->checkSemantic(ctx);
        ctx.popScope();
        ctx.currentClass.clear();
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
        for (const auto& c : classes) if (c) c->print(nivel + 1);
    }

    // 1a PASSADA: coleta classes, campos e assinaturas de métodos no grafo global
    void buildClassTable(ClassTable& ct) {
        if (mainClass) ct.addClass(mainClass->getName(), "");
        for (auto& c : classes) {
            if (!c) continue;
            if (!ct.addClass(c->getName(), c->getParent()))
                throw std::runtime_error("Erro Semantico: A classe '" + c->getName() +
                    "' foi declarada mais de uma vez.");
            ClassInfo* ci = ct.get(c->getName());
            for (auto& f : c->getFields()) {
                if (ci->fields.count(f.name))
                    throw std::runtime_error("Erro Semantico: Atributo '" + f.name +
                        "' declarado duas vezes na classe '" + c->getName() + "'.");
                ci->fields[f.name] = f.type;
            }
            for (auto& m : c->getMethods()) {
                if (ci->methods.count(m->getName()))
                    throw std::runtime_error("Erro Semantico: Metodo '" + m->getName() +
                        "' declarado duas vezes na classe '" + c->getName() + "'.");
                MethodSig sig;
                sig.returnType = m->getReturnType();
                sig.declaringClass = c->getName();
                for (auto& p : m->getParams()) {
                    sig.paramTypes.push_back(p.type);
                    sig.paramNames.push_back(p.name);
                }
                ci->methods[m->getName()] = sig;
            }
        }
        ct.verify();
    }

    std::string checkSemantic(SemanticContext& ctx) override {
        if (mainClass) mainClass->checkSemantic(ctx);
        for (auto& c : classes) if (c) c->checkSemantic(ctx);
        return "void";
    }
};