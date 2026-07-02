#pragma once
#include <string>
#include <vector>
#include <sstream>
#include "../parser/SymbolTable.hpp"

// ==========================================================
//  Codigo de Tres Enderecos (3AC / TAC)
//
//  Cada instrucao tem um codigo de operacao (op) e ate TRES
//  referencias (res, arg1, arg2), que apontam para entradas
//  da tabela de simbolos (variaveis, temporarias e labels)
//  ou para constantes imediatas.
//
//  Forma geral:   res = arg1  op  arg2
// ==========================================================
enum class TacOp {
    ADD, SUB, MULT,     // res = arg1 + - * arg2
    LESS,               // res = arg1 < arg2
    AND,                // res = arg1 && arg2
    NOT,                // res = ! arg1
    COPY,               // res = arg1
    NEW_OBJECT,         // res = new arg1        (arg1 = nome da classe)
    NEW_ARRAY,          // res = new int[arg1]
    ARRAY_LOAD,         // res = arg1[arg2]
    ARRAY_STORE,        // res[arg1] = arg2
    LENGTH,             // res = length arg1
    PARAM,              // param arg1
    CALL,               // res = call arg1, arg2 (arg1 = metodo, arg2 = num params)
    LABEL,              // arg1:
    GOTO,               // goto arg1
    IF_FALSE,           // ifFalse arg1 goto arg2
    PRINT,              // print arg1
    RETURN,             // return arg1
    HALT                // halt
};

// Uma instrucao 3AC
struct TacInstr {
    TacOp op;
    std::string res;
    std::string arg1;
    std::string arg2;
};

// ---------- CRIACAO (funcao auxiliar de fabrica) ----------
inline TacInstr makeInstr(TacOp op, const std::string& res = "",
                          const std::string& arg1 = "", const std::string& arg2 = "") {
    return TacInstr{op, res, arg1, arg2};
}

// ==========================================================
//  Lista de instrucoes 3AC (um "trecho de codigo")
//  Oferece as tres operacoes: criacao, impressao e
//  concatenacao.
// ========================================================
class Code {
public:
    std::vector<TacInstr> instrs;

    // ---------- criacao / insercao ----------
    void add(const TacInstr& i) { instrs.push_back(i); }

    // ---------- concatenacao de listas ----------
    void concat(const Code& other) {
        instrs.insert(instrs.end(), other.instrs.begin(), other.instrs.end());
    }

    // ---------- impressao ----------
    std::string toString() const {
        std::ostringstream oss;
        int linha = 0;
        for (const auto& i : instrs) {
            // labels ficam sem numero, alinhadas a esquerda, o resto indentado
            if (i.op == TacOp::LABEL) {
                oss << i.arg1 << ":\n";
                continue;
            }
            oss << "  " << pad(linha++) << "  " << fmt(i) << "\n";
        }
        return oss.str();
    }

private:
    static std::string pad(int n) {
        std::ostringstream o; o << "[" << n << "]";
        std::string s = o.str();
        while (s.size() < 5) s += " ";
        return s;
    }

    static std::string fmt(const TacInstr& i) {
        switch (i.op) {
            case TacOp::ADD:         return i.res + " = " + i.arg1 + " + " + i.arg2;
            case TacOp::SUB:         return i.res + " = " + i.arg1 + " - " + i.arg2;
            case TacOp::MULT:        return i.res + " = " + i.arg1 + " * " + i.arg2;
            case TacOp::LESS:        return i.res + " = " + i.arg1 + " < " + i.arg2;
            case TacOp::AND:         return i.res + " = " + i.arg1 + " && " + i.arg2;
            case TacOp::NOT:         return i.res + " = ! " + i.arg1;
            case TacOp::COPY:        return i.res + " = " + i.arg1;
            case TacOp::NEW_OBJECT:  return i.res + " = new " + i.arg1;
            case TacOp::NEW_ARRAY:   return i.res + " = new int[" + i.arg1 + "]";
            case TacOp::ARRAY_LOAD:  return i.res + " = " + i.arg1 + "[" + i.arg2 + "]";
            case TacOp::ARRAY_STORE: return i.res + "[" + i.arg1 + "] = " + i.arg2;
            case TacOp::LENGTH:      return i.res + " = length " + i.arg1;
            case TacOp::PARAM:       return "param " + i.arg1;
            case TacOp::CALL:        return i.res + " = call " + i.arg1 + ", " + i.arg2;
            case TacOp::GOTO:        return "goto " + i.arg1;
            case TacOp::IF_FALSE:    return "ifFalse " + i.arg1 + " goto " + i.arg2;
            case TacOp::PRINT:       return "print " + i.arg1;
            case TacOp::RETURN:      return i.arg1.empty() ? "return" : "return " + i.arg1;
            case TacOp::HALT:        return "halt";
            case TacOp::LABEL:       return i.arg1 + ":";
        }
        return "???";
    }
};

// ==========================================================
//  Contexto de geracao de codigo.
//  Guarda a tabela de simbolos (hash) e cria temporarias/labels.
// ==========================================================
class CodeGen {
public:
    SymbolTable& syms;
    std::string currentClass;

    CodeGen(SymbolTable& s) : syms(s) {}

    // insere uma nova variavel temporaria na tabela de simbolos e devolve o nome
    std::string newTemp()  { return syms.newTemp(); }
    // insere uma nova label na tabela de simbolos e devolve o nome
    std::string newLabel() { return syms.newLabel(); }
};