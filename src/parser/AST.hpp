#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include "SemanticContext.hpp"
#include "../codegen/tac.hpp"

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
// 2. EXPRESSOES
//   genExp: gera o 3AC da expressao e devolve, em 'place', o
//   simbolo/temporaria que guarda o resultado.
// ==========================================================
class ExpNode : public ASTNode {
public:
    virtual Code genExp(CodeGen& cg, std::string& place) = 0;
};

class IntLiteralNode : public ExpNode {
    int value;
public:
    IntLiteralNode(int val) : value(val) {}
    void print(int nivel) const override { 
        imprimirIndentacao(nivel);
         std::cout << "IntLiteral: " << value << "\n"; 
        }
    std::string checkSemantic(SemanticContext&) override { return "int"; }
    Code genExp(CodeGen&, std::string& place) override 
    { 
        place = std::to_string(value);
         return Code(); 
        }
};

class BoolLiteralNode : public ExpNode {
    bool value;
public:
    BoolLiteralNode(bool val) : value(val) {}
    void print(int nivel) const override {
         imprimirIndentacao(nivel);
          std::cout << "BoolLiteral: " <<
           (value ? "true" : "false") << "\n"; 
        }
    std::string checkSemantic(SemanticContext&) override 
    { 
        return "boolean";
     }
    Code genExp(CodeGen&, std::string& place) override { 
        place = value ? "1" : "0";
        return Code(); 
    }
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
        
        // Tenta como campo herdado diretamente pela hierarquia de classes
        std::string t = ctx.lookup(name);
        if (t.empty()) t = ctx.classes.resolveFieldType(ctx.currentClass, name);
        if (t.empty()) { 
            ctx.error("Variavel '" + name + "' nao declarada."); 
            return TIPO_ERRO; 
        }
        
        // Garantia de inicialização: se for uma variável local, precisa ter sido atribuída
        if (ctx.isLocal(name) && !ctx.isAssigned(name))
            ctx.error("Erro Semântico: A variavel local '" + name + "' pode ser usada antes de ser inicializada.");
        return t;
    }
    Code genExp(CodeGen&, std::string& place) override { place = name; return Code(); }
};

class ThisNode : public ExpNode {
public:
    void print(int nivel) const override { 
        imprimirIndentacao(nivel); 
        std::cout << "ThisExp\n"; 
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        if (ctx.currentClass.empty()) { 
            ctx.error("Erro Semântico:'this' usado fora de uma classe."); 
            return TIPO_ERRO; 
        }
        return ctx.currentClass;
    }
    Code genExp(CodeGen&, std::string& place) override { place = "this"; return Code(); }
};

class BinOpNode : public ExpNode {
    std::string op;
    std::unique_ptr<ExpNode> left, right;
public:
    BinOpNode(std::string o, std::unique_ptr<ExpNode> l, std::unique_ptr<ExpNode> r)
        : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
    void print(int n) const override {
        imprimirIndentacao(n); std::cout << "BinOp [" << op << "]\n";
        if (left) left->print(n + 1);
        if (right) right->print(n + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string tL = left->checkSemantic(ctx);
        std::string tR = right->checkSemantic(ctx);
        bool err = (tL == TIPO_ERRO || tR == TIPO_ERRO);
        if (op == "+" || op == "-" || op == "*") {
            if (!err && (tL != "int" || tR != "int")) ctx.error("Operador '" + op + "' exige operandos do tipo 'int'.");
            return "int";
        } else if (op == "<") {
            if (!err && (tL != "int" || tR != "int")) ctx.error("Operador '<' exige operandos do tipo 'int'.");
            return "boolean";
        } else if (op == "&&") {
            if (!err && (tL != "boolean" || tR != "boolean")) ctx.error("Operador '&&' exige operandos do tipo 'boolean'.");
            return "boolean";
        }
        return TIPO_ERRO;
    }
    Code genExp(CodeGen& cg, std::string& place) override {
        std::string p1, p2;
        Code c = left->genExp(cg, p1);
        c.concat(right->genExp(cg, p2));
        place = cg.newTemp();
        TacOp o = TacOp::ADD;
        if (op == "+") o = TacOp::ADD;
        else if (op == "-") o = TacOp::SUB;
        else if (op == "*") o = TacOp::MULT;
        else if (op == "<") o = TacOp::LESS;
        else if (op == "&&") o = TacOp::AND;
        c.add(makeInstr(o, place, p1, p2));
        return c;
    }
};

class UnaryOpNode : public ExpNode {
    std::string op;
    std::unique_ptr<ExpNode> exp;
public:
    UnaryOpNode(std::string o, std::unique_ptr<ExpNode> e) : op(std::move(o)), exp(std::move(e)) {}
    void print(int n) const override { imprimirIndentacao(n); std::cout << "UnaryOp [" << op << "]\n"; if (exp) exp->print(n + 1); }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string t = exp->checkSemantic(ctx);
        if (op == "!") {
            if (t != TIPO_ERRO && t != "boolean") ctx.error("Operador '!' exige operando do tipo 'boolean'.");
            return "boolean";
        }
        return t;
    }
    Code genExp(CodeGen& cg, std::string& place) override {
        std::string p;
        Code c = exp->genExp(cg, p);
        place = cg.newTemp();
        c.add(makeInstr(TacOp::NOT, place, p));
        return c;
    }
};

class ArrayAccessNode : public ExpNode {
    std::unique_ptr<ExpNode> arrayExp, indexExp;
public:
    ArrayAccessNode(std::unique_ptr<ExpNode> a, std::unique_ptr<ExpNode> i)
        : arrayExp(std::move(a)), indexExp(std::move(i)) {}
    void print(int n) const override {
        imprimirIndentacao(n); std::cout << "ArrayAccess\n";
        if (arrayExp) arrayExp->print(n + 1);
        if (indexExp) indexExp->print(n + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string tA = arrayExp->checkSemantic(ctx);
        std::string tI = indexExp->checkSemantic(ctx);
        if (tA != TIPO_ERRO && tA != "int[]") ctx.error("Acesso indexado exige um vetor 'int[]'.");
        if (tI != TIPO_ERRO && tI != "int") ctx.error("O indice de um vetor deve ser do tipo 'int'.");
        return "int";
    }
    Code genExp(CodeGen& cg, std::string& place) override {
        std::string pa, pi;
        Code c = arrayExp->genExp(cg, pa);
        c.concat(indexExp->genExp(cg, pi));
        place = cg.newTemp();
        c.add(makeInstr(TacOp::ARRAY_LOAD, place, pa, pi));
        return c;
    }
};

class LengthNode : public ExpNode {
    std::unique_ptr<ExpNode> arrayExp;
public:
    LengthNode(std::unique_ptr<ExpNode> a) : arrayExp(std::move(a)) {}
    void print(int n) const override { imprimirIndentacao(n); std::cout << "Length\n"; if (arrayExp) arrayExp->print(n + 1); }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string t = arrayExp->checkSemantic(ctx);
        if (t != TIPO_ERRO && t != "int[]") ctx.error("'.length' so pode ser aplicado a um vetor 'int[]'.");
        return "int";
    }
    Code genExp(CodeGen& cg, std::string& place) override {
        std::string pa;
        Code c = arrayExp->genExp(cg, pa);
        place = cg.newTemp();
        c.add(makeInstr(TacOp::LENGTH, place, pa));
        return c;
    }
};

class NewArrayNode : public ExpNode {
    std::unique_ptr<ExpNode> sizeExp;
public:
    NewArrayNode(std::unique_ptr<ExpNode> s) : sizeExp(std::move(s)) {}
    void print(int n) const override { imprimirIndentacao(n); std::cout << "NewArray (int[])\n"; if (sizeExp) sizeExp->print(n + 1); }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string t = sizeExp->checkSemantic(ctx);
        if (t != TIPO_ERRO && t != "int") ctx.error("O tamanho de 'new int[]' deve ser do tipo 'int'.");
        return "int[]";
    }
    Code genExp(CodeGen& cg, std::string& place) override {
        std::string ps;
        Code c = sizeExp->genExp(cg, ps);
        place = cg.newTemp();
        c.add(makeInstr(TacOp::NEW_ARRAY, place, ps));
        return c;
    }
};

class NewObjectNode : public ExpNode {
    std::string className;
public:
    NewObjectNode(std::string n) : className(std::move(n)) {}
    void print(int n) const override { imprimirIndentacao(n); std::cout << "NewObject [" << className << "]\n"; }
    std::string checkSemantic(SemanticContext& ctx) override {
        if (!ctx.classes.isDefined(className)) {
            ctx.error("Tentativa de instanciar a classe '" + className + "', que nao foi declarada.");
            return TIPO_ERRO;
        }
        return className;
    }
    Code genExp(CodeGen& cg, std::string& place) override {
        Code c;
        place = cg.newTemp();
        c.add(makeInstr(TacOp::NEW_OBJECT, place, className));
        return c;
    }
};

class MethodCallNode : public ExpNode {
    std::unique_ptr<ExpNode> receiver;
    std::string method;
    std::vector<std::unique_ptr<ExpNode>> args;
public:
    MethodCallNode(std::unique_ptr<ExpNode> r, std::string m, std::vector<std::unique_ptr<ExpNode>> a)
        : receiver(std::move(r)), method(std::move(m)), args(std::move(a)) {}
    void print(int n) const override {
        imprimirIndentacao(n); std::cout << "MethodCall [" << method << "]\n";
        imprimirIndentacao(n + 1); std::cout << "Receiver:\n";
        if (receiver) receiver->print(n + 2);
        if (!args.empty()) {
            imprimirIndentacao(n + 1); std::cout << "Args:\n";
            for (const auto& a : args) if (a) a->print(n + 2);
        }
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string recvType = receiver->checkSemantic(ctx);
        std::vector<std::string> argTypes;
        for (auto& a : args) argTypes.push_back(a->checkSemantic(ctx));
        if (recvType == TIPO_ERRO) return TIPO_ERRO;
        if (!ctx.classes.isDefined(recvType)) {
            ctx.error("Chamada do metodo '" + method + "' sobre algo que nao e um objeto (tipo '" + recvType + "').");
            return TIPO_ERRO;
        }
        const MethodSig* sig = ctx.classes.resolveMethod(recvType, method);
        if (!sig) {
            ctx.error("O metodo '" + method + "' nao existe na classe '" + recvType + "' nem em suas superclasses.");
            return TIPO_ERRO;
        }
        if (argTypes.size() != sig->paramTypes.size())
            ctx.error("O metodo '" + method + "' espera " + std::to_string(sig->paramTypes.size()) +
                      " argumento(s), mas recebeu " + std::to_string(argTypes.size()) + ".");
        size_t lim = std::min(argTypes.size(), sig->paramTypes.size());
        for (size_t i = 0; i < lim; ++i)
            if (!ctx.classes.assignableTo(sig->paramTypes[i], argTypes[i]))
                ctx.error("Argumento " + std::to_string(i + 1) + " do metodo '" + method +
                          "' esperava '" + sig->paramTypes[i] + "', mas recebeu '" + argTypes[i] + "'.");
        return sig->returnType;
    }
    Code genExp(CodeGen& cg, std::string& place) override {
        // 1) gera codigo do receptor e dos argumentos (filhos primeiro)
        std::string pr;
        Code c = receiver->genExp(cg, pr);
        std::vector<std::string> aps;
        for (auto& a : args) { std::string ap; c.concat(a->genExp(cg, ap)); aps.push_back(ap); }
        // 2) empilha parametros: o receptor (this) primeiro, depois os argumentos
        c.add(makeInstr(TacOp::PARAM, "", pr));
        for (auto& ap : aps) c.add(makeInstr(TacOp::PARAM, "", ap));
        // 3) chamada: t = call metodo, num_params
        place = cg.newTemp();
        c.add(makeInstr(TacOp::CALL, place, method, std::to_string(aps.size() + 1)));
        return c;
    }
};

// ==========================================================
// 3. COMANDOS
//   genCmd: gera o 3AC do comando.
// ==========================================================
class CmdNode : public ASTNode {
public:
    virtual Code genCmd(CodeGen& cg) = 0;
};

class AssignNode : public CmdNode {
    std::string id;
    std::unique_ptr<ExpNode> exp;
public:
    AssignNode(std::string n, std::unique_ptr<ExpNode> e) : id(std::move(n)), exp(std::move(e)) {}
    void print(int n) const override { imprimirIndentacao(n); std::cout << "Assign: " << id << "\n"; if (exp) exp->print(n + 1); }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string expType = exp->checkSemantic(ctx);
        std::string varType = ctx.lookup(id);
        if (varType.empty()) varType = ctx.classes.resolveFieldType(ctx.currentClass, id);
        if (varType.empty()) { ctx.error("Atribuicao a variavel nao declarada '" + id + "'."); ctx.markAssigned(id); return "void"; }
        if (!ctx.classes.assignableTo(varType, expType))
            ctx.error("Incompatibilidade de tipos na variavel '" + id + "'. Esperado '" + varType + "', recebido '" + expType + "'.");
        ctx.markAssigned(id);
        return "void";
    }
    Code genCmd(CodeGen& cg) override {
        std::string p;
        Code c = exp->genExp(cg, p);
        c.add(makeInstr(TacOp::COPY, id, p));
        return c;
    }
};

class ArrayAssignNode : public CmdNode {
    std::string id;
    std::unique_ptr<ExpNode> indexExp, valueExp;
public:
    ArrayAssignNode(std::string n, std::unique_ptr<ExpNode> i, std::unique_ptr<ExpNode> v)
        : id(std::move(n)), indexExp(std::move(i)), valueExp(std::move(v)) {}
    void print(int n) const override {
        imprimirIndentacao(n); std::cout << "ArrayAssign: " << id << "\n";
        if (indexExp) indexExp->print(n + 1);
        if (valueExp) valueExp->print(n + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string varType = ctx.lookup(id);
        if (varType.empty()) varType = ctx.classes.resolveFieldType(ctx.currentClass, id);
        std::string tI = indexExp->checkSemantic(ctx);
        std::string tV = valueExp->checkSemantic(ctx);
        if (varType.empty()) { ctx.error("Atribuicao a vetor nao declarado '" + id + "'."); return "void"; }
        if (varType != "int[]") ctx.error("A variavel '" + id + "' nao e um vetor 'int[]'.");
        if (ctx.isLocal(id) && !ctx.isAssigned(id))
            ctx.error("O vetor local '" + id + "' pode ser usado antes de ser inicializado (faltou 'new int[]').");
        if (tI != TIPO_ERRO && tI != "int") ctx.error("O indice do vetor '" + id + "' deve ser do tipo 'int'.");
        if (tV != TIPO_ERRO && tV != "int") ctx.error("O valor atribuido ao vetor '" + id + "' deve ser do tipo 'int'.");
        return "void";
    }
    Code genCmd(CodeGen& cg) override {
        std::string pi, pv;
        Code c = indexExp->genExp(cg, pi);
        c.concat(valueExp->genExp(cg, pv));
        c.add(makeInstr(TacOp::ARRAY_STORE, id, pi, pv));
        return c;
    }
};

class PrintNode : public CmdNode {
    std::unique_ptr<ExpNode> exp;
public:
    PrintNode(std::unique_ptr<ExpNode> e) : exp(std::move(e)) {}
    void print(int n) const override { imprimirIndentacao(n); std::cout << "Print\n"; if (exp) exp->print(n + 1); }
    std::string checkSemantic(SemanticContext& ctx) override { if (exp) exp->checkSemantic(ctx); return "void"; }
    Code genCmd(CodeGen& cg) override {
        std::string p;
        Code c = exp->genExp(cg, p);
        c.add(makeInstr(TacOp::PRINT, "", p));
        return c;
    }
};

class IfNode : public CmdNode {
    std::unique_ptr<ExpNode> condition;
    std::vector<std::unique_ptr<CmdNode>> ifBlock, elseBlock;
public:
    IfNode(std::unique_ptr<ExpNode> c, std::vector<std::unique_ptr<CmdNode>> i, std::vector<std::unique_ptr<CmdNode>> e)
        : condition(std::move(c)), ifBlock(std::move(i)), elseBlock(std::move(e)) {}
    void print(int n) const override {
        imprimirIndentacao(n); std::cout << "If\n";
        if (condition) condition->print(n + 1);
        imprimirIndentacao(n); std::cout << "Then:\n";
        for (const auto& c : ifBlock) if (c) c->print(n + 1);
        if (!elseBlock.empty()) {
            imprimirIndentacao(n); std::cout << "Else:\n";
            for (const auto& c : elseBlock) if (c) c->print(n + 1);
        }
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string t = condition ? condition->checkSemantic(ctx) : TIPO_ERRO;
        if (t != TIPO_ERRO && t != "boolean") ctx.error("A condicao do 'if' deve ser do tipo 'boolean'.");
        std::set<std::string> before = ctx.assignedVars;
        ctx.assignedVars = before;
        for (auto& c : ifBlock) if (c) c->checkSemantic(ctx);
        std::set<std::string> afterThen = ctx.assignedVars;
        ctx.assignedVars = before;
        for (auto& c : elseBlock) if (c) c->checkSemantic(ctx);
        std::set<std::string> afterElse = ctx.assignedVars;
        ctx.assignedVars = elseBlock.empty() ? before : intersectAssigned(afterThen, afterElse);
        return "void";
    }
    Code genCmd(CodeGen& cg) override {
        std::string pc;
        Code c = condition->genExp(cg, pc);
        if (elseBlock.empty()) {
            std::string Lend = cg.newLabel();
            c.add(makeInstr(TacOp::IF_FALSE, "", pc, Lend));
            for (auto& s : ifBlock) if (s) c.concat(s->genCmd(cg));
            c.add(makeInstr(TacOp::LABEL, "", Lend));
        } else {
            std::string Lelse = cg.newLabel();
            std::string Lend = cg.newLabel();
            c.add(makeInstr(TacOp::IF_FALSE, "", pc, Lelse));
            for (auto& s : ifBlock) if (s) c.concat(s->genCmd(cg));
            c.add(makeInstr(TacOp::GOTO, "", Lend));
            c.add(makeInstr(TacOp::LABEL, "", Lelse));
            for (auto& s : elseBlock) if (s) c.concat(s->genCmd(cg));
            c.add(makeInstr(TacOp::LABEL, "", Lend));
        }
        return c;
    }
};

class WhileNode : public CmdNode {
    std::unique_ptr<ExpNode> condition;
    std::vector<std::unique_ptr<CmdNode>> block;
public:
    WhileNode(std::unique_ptr<ExpNode> c, std::vector<std::unique_ptr<CmdNode>> b)
        : condition(std::move(c)), block(std::move(b)) {}
    void print(int n) const override {
        imprimirIndentacao(n); std::cout << "While\n";
        if (condition) condition->print(n + 1);
        imprimirIndentacao(n); std::cout << "Do:\n";
        for (const auto& c : block) if (c) c->print(n + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        std::string t = condition ? condition->checkSemantic(ctx) : TIPO_ERRO;
        if (t != TIPO_ERRO && t != "boolean") ctx.error("A condicao do 'while' deve ser do tipo 'boolean'.");
        std::set<std::string> before = ctx.assignedVars;
        for (auto& c : block) if (c) c->checkSemantic(ctx);
        ctx.assignedVars = before;
        return "void";
    }
    Code genCmd(CodeGen& cg) override {
        std::string Lstart = cg.newLabel();
        std::string Lend = cg.newLabel();
        Code c;
        c.add(makeInstr(TacOp::LABEL, "", Lstart));
        std::string pc;
        c.concat(condition->genExp(cg, pc));
        c.add(makeInstr(TacOp::IF_FALSE, "", pc, Lend));
        for (auto& s : block) if (s) c.concat(s->genCmd(cg));
        c.add(makeInstr(TacOp::GOTO, "", Lstart));
        c.add(makeInstr(TacOp::LABEL, "", Lend));
        return c;
    }
};

// ==========================================================
// 4. ESTRUTURAS
// ==========================================================
class MethodNode : public ASTNode {
    std::string name, returnType;
    std::vector<VarDecl> params, locals;
    std::vector<std::unique_ptr<CmdNode>> commands;
    std::unique_ptr<ExpNode> returnExp;
public:
    MethodNode(std::string n, std::string t, std::vector<VarDecl> p, std::vector<VarDecl> l,
               std::vector<std::unique_ptr<CmdNode>> c, std::unique_ptr<ExpNode> r)
        : name(std::move(n)), returnType(std::move(t)), params(std::move(p)),
          locals(std::move(l)), commands(std::move(c)), returnExp(std::move(r)) {}
    const std::string& getName() const { return name; }
    const std::string& getReturnType() const { return returnType; }
    const std::vector<VarDecl>& getParams() const { return params; }
    void print(int n) const override {
        imprimirIndentacao(n); std::cout << "Method [" << returnType << " " << name << "(";
        for (size_t i = 0; i < params.size(); ++i) std::cout << (i ? ", " : "") << params[i].type << " " << params[i].name;
        std::cout << ")]\n";
        for (const auto& c : commands) if (c) c->print(n + 1);
        imprimirIndentacao(n + 1); std::cout << "Return:\n";
        if (returnExp) returnExp->print(n + 2);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        ctx.pushScope();
        std::string savedReturn = ctx.currentReturnType;
        std::set<std::string> savedLocals = ctx.declaredLocals;
        std::set<std::string> savedAssigned = ctx.assignedVars;
        ctx.currentReturnType = returnType;
        ctx.declaredLocals.clear();
        ctx.assignedVars.clear();
        for (auto& p : params) { ctx.declare(p.name, p.type); ctx.markAssigned(p.name); }
        for (auto& l : locals) { ctx.declare(l.name, l.type); ctx.declaredLocals.insert(l.name); }
        for (auto& c : commands) if (c) c->checkSemantic(ctx);
        if (returnExp) {
            std::string rt = returnExp->checkSemantic(ctx);
            if (!ctx.classes.assignableTo(returnType, rt))
                ctx.error("O metodo '" + name + "' prometeu retornar '" + returnType + "', mas tentou retornar '" + rt + "'.");
        }
        ctx.currentReturnType = savedReturn;
        ctx.declaredLocals = savedLocals;
        ctx.assignedVars = savedAssigned;
        ctx.popScope();
        return "void";
    }
    Code gen(CodeGen& cg) {
        Code c;
        c.add(makeInstr(TacOp::LABEL, "", cg.currentClass + "." + name));
        for (auto& cmd : commands) if (cmd) c.concat(cmd->genCmd(cg));
        std::string pr;
        if (returnExp) { c.concat(returnExp->genExp(cg, pr)); c.add(makeInstr(TacOp::RETURN, "", pr)); }
        else c.add(makeInstr(TacOp::RETURN));
        return c;
    }
};

class ClassNode : public ASTNode {
    std::string name, parent;
    std::vector<VarDecl> fields;
    std::vector<std::unique_ptr<MethodNode>> methods;
public:
    ClassNode(std::string n, std::string p, std::vector<VarDecl> f, std::vector<std::unique_ptr<MethodNode>> m)
        : name(std::move(n)), parent(std::move(p)), fields(std::move(f)), methods(std::move(m)) {}
    const std::string& getName() const { return name; }
    const std::string& getParent() const { return parent; }
    const std::vector<VarDecl>& getFields() const { return fields; }
    const std::vector<std::unique_ptr<MethodNode>>& getMethods() const { return methods; }
    void print(int n) const override {
        imprimirIndentacao(n); std::cout << "Class " << name << (parent.empty() ? "" : " extends " + parent) << "\n";
        for (const auto& f : fields) { imprimirIndentacao(n + 1); std::cout << "Field: " << f.type << " " << f.name << "\n"; }
        for (const auto& m : methods) if (m) m->print(n + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        if (methods.empty() && fields.empty()) ctx.error("A classe '" + name + "' nao pode ser vazia (sem atributos ou metodos).");
        ctx.currentClass = name;
        ctx.pushScope();
        std::string cur = name;
        std::set<std::string> visited;
        while (!cur.empty() && !visited.count(cur)) {
            visited.insert(cur);
            ClassInfo* ci = ctx.classes.get(cur);
            if (!ci) break;
            for (auto& fld : ci->fields) if (ctx.lookup(fld.first).empty()) ctx.declare(fld.first, fld.second);
            cur = ci->parent;
        }
        for (auto& m : methods) if (m) m->checkSemantic(ctx);
        ctx.popScope();
        ctx.currentClass.clear();
        return "void";
    }
    Code gen(CodeGen& cg) {
        cg.currentClass = name;
        Code c;
        for (auto& m : methods) if (m) c.concat(m->gen(cg));
        return c;
    }
};

class MainClassNode : public ASTNode {
    std::string name, argName;
    std::vector<std::unique_ptr<CmdNode>> commands;
public:
    MainClassNode(std::string n, std::string a, std::vector<std::unique_ptr<CmdNode>> c)
        : name(std::move(n)), argName(std::move(a)), commands(std::move(c)) {}
    const std::string& getName() const { return name; }
    void print(int n) const override {
        imprimirIndentacao(n); std::cout << "MainClass " << name << "\n";
        for (const auto& c : commands) if (c) c->print(n + 1);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        ctx.currentClass = name;
        ctx.pushScope();
        ctx.declaredLocals.clear();
        ctx.assignedVars.clear();
        ctx.declare(argName, "String[]");
        ctx.markAssigned(argName);
        for (auto& c : commands) if (c) c->checkSemantic(ctx);
        ctx.popScope();
        ctx.currentClass.clear();
        return "void";
    }
    Code gen(CodeGen& cg) {
        cg.currentClass = name;
        Code c;
        c.add(makeInstr(TacOp::LABEL, "", "main"));
        for (auto& cmd : commands) if (cmd) c.concat(cmd->genCmd(cg));
        c.add(makeInstr(TacOp::HALT));
        return c;
    }
};

class ProgNode : public ASTNode {
    std::unique_ptr<MainClassNode> mainClass;
    std::vector<std::unique_ptr<ClassNode>> classes;
public:
    ProgNode(std::unique_ptr<MainClassNode> mc, std::vector<std::unique_ptr<ClassNode>> cls)
        : mainClass(std::move(mc)), classes(std::move(cls)) {}
    void print(int n) const override {
        imprimirIndentacao(n); std::cout << "ProgramRoot\n";
        if (mainClass) mainClass->print(n + 1);
        for (const auto& c : classes) if (c) c->print(n + 1);
    }
    void buildClassTable(ClassTable& ct, std::vector<std::string>& errs) {
        if (mainClass) ct.addClass(mainClass->getName(), "");
        for (auto& c : classes) {
            if (!c) continue;
            if (!ct.addClass(c->getName(), c->getParent())) {
                errs.push_back("Erro Semantico: A classe '" + c->getName() + "' foi declarada mais de uma vez.");
                continue;
            }
            ClassInfo* ci = ct.get(c->getName());
            for (auto& f : c->getFields()) {
                if (ci->fields.count(f.name)) errs.push_back("Erro Semantico: Atributo '" + f.name + "' declarado duas vezes na classe '" + c->getName() + "'.");
                else ci->fields[f.name] = f.type;
            }
            for (auto& m : c->getMethods()) {
                if (ci->methods.count(m->getName())) { errs.push_back("Erro Semantico: Metodo '" + m->getName() + "' declarado duas vezes na classe '" + c->getName() + "'."); continue; }
                MethodSig sig;
                sig.returnType = m->getReturnType();
                sig.declaringClass = c->getName();
                for (auto& p : m->getParams()) { sig.paramTypes.push_back(p.type); sig.paramNames.push_back(p.name); }
                ci->methods[m->getName()] = sig;
            }
        }
        ct.verify(errs);
    }
    std::string checkSemantic(SemanticContext& ctx) override {
        if (mainClass) mainClass->checkSemantic(ctx);
        for (auto& c : classes) if (c) c->checkSemantic(ctx);
        return "void";
    }
    // Gera o codigo intermediario (3AC) do programa inteiro
    Code gen(CodeGen& cg) {
        Code c;
        if (mainClass) c.concat(mainClass->gen(cg));
        for (auto& cl : classes) if (cl) c.concat(cl->gen(cg));
        return c;
    }
};