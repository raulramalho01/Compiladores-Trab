/*
 * preprocessor.cpp
 *
 * Preprocessador de codigo C++ baseado em maquina de estados.
 *
 * Estados implementados (conforme diagrama):
 *   Normal          - lendo codigo normal
 *   Espaco_Pendente - viu espaco/tab/fim-de-linha, ainda nao decidiu se escreve
 *   Barra_Lida      - viu '/', aguardando '/' ou '*'
 *   Comment_Linha   - dentro de comentario inline (//)
 *   Comment_Bloco   - dentro de comentario multilinha
 *   Fim_Bloco       - dentro de bloco, viu '*', aguardando '/'
 *
 * Regra de whitespace:
 *   Whitespace original e colapsado (nunca eliminado por completo) quando
 *   havia pelo menos um token alfanumerico de algum dos dois lados.
 *   Em outras palavras, espaco e emitido quando:
 *     - prev_alnum (ultimo token emitido era alnum/identificador), OU
 *     - next_alnum (proximo token a ser emitido e alnum/identificador)
 *   Essa regra produz "int x = 5" e "x = a / b" corretamente.
 *   Simbolos adjacentes como "(x" ou "){" nao geram espaco extra.
 *
 * Excecoes de parenteses:
 *   Espaco entre '(' e token e descartado pois '(' nao e alnum e,
 *   apesar de next ser alnum, a aresta do diagrama diz que o espaco
 *   entre simbolo de abertura e o conteudo e suprimido.
 *   Para isso, rastreamos se o ultimo char emitido era um
 *   "abridor" como '(', '[', '{', '<'.
 */

#include "preprocessor.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

// ------------------------------------------------------------------ //
//  Helpers de classificacao                                           //
// ------------------------------------------------------------------ //

static bool is_ws(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool is_alnum(char c)
{
    return (std::isalnum(static_cast<unsigned char>(c)) != 0) || c == '_';
}

// Caracteres de "abertura" apos os quais whitespace e sempre descartado
// (pois o conteudo imediato e parte de uma expressao/declarador).
static bool is_opener(char c)
{
    return c == '(' || c == '[' || c == '<';
}

// Caracteres de "fechamento/terminador" antes dos quais whitespace e descartado
// (ex.: "x )" deve virar "x)", "5 ;" deve virar "5;").
static bool is_closer(char c)
{
    return c == ')' || c == ']' || c == '>' || c == ';' || c == ',';
}

/*
 * needs_space(prev, next)
 *
 * Retorna true se um espaco deve ser emitido entre 'prev' (ultimo char
 * efetivamente emitido) e 'next' (proximo char a ser emitido), dado que
 * havia whitespace original entre eles.
 *
 * Regra:
 *   - Se prev e opener ('(','[') -> false  (nunca espaco apos abridor)
 *   - Se next e closer (')',']',';',',') -> false  (nunca espaco antes de fechador)
 *   - Se prev e alnum OU next e alnum -> true  (preserva separacao de tokens)
 *   - Caso contrario (ambos simbolos nao-especiais) -> false
 */
static bool needs_space(char prev, char next)
{
    if (prev == '\0' || next == '\0') return false;
    if (is_opener(prev))              return false;   // "(x" sem espaco
    if (is_closer(next))              return false;   // "x)" sem espaco
    return is_alnum(prev) || is_alnum(next);
}

// ------------------------------------------------------------------ //
//  Maquina de estados principal                                       //
// ------------------------------------------------------------------ //

std::string preprocess(std::string_view input)
{
    std::string output;
    output.reserve(input.size());

    State state      = State::Normal;
    char  last_emit  = '\0';   // ultimo char efetivamente emitido no output
    bool  pending_ws = false;  // ha whitespace pendente a ser resolvido?
    bool  pending_nl = false;  // esse whitespace inclui pelo menos um '\n'?

    auto emit = [&](char c) {
        output += c;
        last_emit = c;
    };

    // Resolve whitespace pendente antes de emitir 'next'.
    auto flush_pending = [&](char next) {
        if (!pending_ws) return;
        pending_ws = false;

        if (pending_nl) {
            pending_nl = false;
            // Newline: emite '\n' se needs_space, senao descarta
            // (para nao gerar newlines desnecessarias entre bloco e codigo)
            if (needs_space(last_emit, next))
                emit('\n');
        } else {
            // Espaco/tab: emite ' ' se needs_space
            if (needs_space(last_emit, next))
                emit(' ');
        }
    };

    for (size_t i = 0; i < input.size(); ++i)
    {
        const char ch = input[i];

        switch (state)
        {
        // ---------------------------------------------------------- //
        case State::Normal:
        // ---------------------------------------------------------- //
            if (ch == '/') {
                state = State::Barra_Lida;
            }
            else if (is_ws(ch)) {
                pending_ws = true;
                if (ch == '\n' || ch == '\r') pending_nl = true;
                state = State::Espaco_Pendente;
            }
            else {
                flush_pending(ch);
                emit(ch);
            }
            break;

        // ---------------------------------------------------------- //
        case State::Espaco_Pendente:
        // ---------------------------------------------------------- //
            if (is_ws(ch)) {
                if (ch == '\n' || ch == '\r') pending_nl = true;
                // permanece em Espaco_Pendente
            }
            else if (ch == '/') {
                state = State::Barra_Lida;
            }
            else {
                flush_pending(ch);
                emit(ch);
                state = State::Normal;
            }
            break;

        // ---------------------------------------------------------- //
        case State::Barra_Lida:
        // ---------------------------------------------------------- //
            if (ch == '/') {
                // Comentario inline: descarta pending e tudo ate '\n'
                pending_ws = false;
                pending_nl = false;
                state = State::Comment_Linha;
            }
            else if (ch == '*') {
                // Comentario de bloco: descarta pending atual;
                // ao sair do bloco, marcamos pending_ws novamente.
                pending_ws = false;
                pending_nl = false;
                state = State::Comment_Bloco;
            }
            else {
                // Nao era comentario: '/' e divisao real.
                flush_pending('/');
                emit('/');
                if (is_ws(ch)) {
                    pending_ws = true;
                    if (ch == '\n' || ch == '\r') pending_nl = true;
                    state = State::Espaco_Pendente;
                } else {
                    flush_pending(ch);
                    emit(ch);
                    state = State::Normal;
                }
            }
            break;

        // ---------------------------------------------------------- //
        case State::Comment_Linha:
        // ---------------------------------------------------------- //
            if (ch == '\n') {
                // '\n' encerra comentario; marca newline pendente
                pending_ws = true;
                pending_nl = true;
                state = State::Espaco_Pendente;
            }
            break;

        // ---------------------------------------------------------- //
        case State::Comment_Bloco:
        // ---------------------------------------------------------- //
            if (ch == '*') {
                state = State::Fim_Bloco;
            }
            break;

        // ---------------------------------------------------------- //
        case State::Fim_Bloco:
        // ---------------------------------------------------------- //
            if (ch == '/') {
                // Bloco encerrado: vira whitespace pendente (sem newline)
                pending_ws = true;
                state = State::Espaco_Pendente;
            }
            else if (ch == '*') {
                state = State::Fim_Bloco;
            }
            else {
                state = State::Comment_Bloco;
            }
            break;
        }
    }

    if (state == State::Barra_Lida) {
        flush_pending('/');
        emit('/');
    }

    // Strip whitespace do inicio e do fim
    const size_t s = output.find_first_not_of(" \t\n\r");
    const size_t e = output.find_last_not_of(" \t\n\r");
    if (s == std::string::npos) return {};
    return output.substr(s, e - s + 1);
}

// ------------------------------------------------------------------ //
//  Sobrecarga                                                         //
// ------------------------------------------------------------------ //
std::string preprocess(const std::string& input)
{
    return preprocess(std::string_view{input});
}

// ------------------------------------------------------------------ //
//  Grava em arquivo                                                   //
// ------------------------------------------------------------------ //
void preprocess_to_file(std::string_view input, const std::string& output_path)
{
    const std::string result = preprocess(input);

    std::ofstream out(output_path);
    if (!out.is_open())
        throw std::runtime_error(
            "preprocessor: nao foi possivel abrir '" + output_path + "'");

    out << result;
    if (!out.good())
        throw std::runtime_error(
            "preprocessor: erro ao gravar em '" + output_path + "'");
}

// ------------------------------------------------------------------ //
//  Arquivo de entrada -> arquivo de saida                            //
// ------------------------------------------------------------------ //
void preprocess_file(const std::string& input_path,
                     const std::string& output_path)
{
    std::ifstream in(input_path);
    if (!in.is_open())
        throw std::runtime_error(
            "preprocessor: nao foi possivel abrir '" + input_path + "'");

    std::ostringstream ss;
    ss << in.rdbuf();
    preprocess_to_file(ss.str(), output_path);
}