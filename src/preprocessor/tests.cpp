/*
 * tests.cpp
 *
 * Suite de testes do preprocessador C++17.
 *
 * Compile e execute:
 *   g++ -std=c++17 -Wall -Wextra preprocessor.cpp tests.cpp -o tests
 *   ./tests
 *
 * Regras de whitespace implementadas pelo preprocessador:
 *
 *   O preprocessador colapsa whitespace mas NAO o elimina quando ha
 *   tokens dos dois lados. A regra de espaco e:
 *
 *     needs_space(prev, next) = true quando:
 *       - prev NAO e abridor  ('(', '[', '<')
 *       - next NAO e fechador (')', ']', '>', ';', ',')
 *       - pelo menos um dos lados e alfanumerico (alnum ou '_')
 *
 *   Consequencias observaveis:
 *     "int   x   =   5;"  ->  "int x = 5;"    (alnum<->simbolo: espaco)
 *     "if (x"             ->  "if (x"          ('(' e opener, sem espaco apos)
 *     "x == 1)"           ->  "x == 1)"        (')' e closer, sem espaco antes)
 *     "b) {"              ->  "b){"            (')' e closer, '{'e simbolo: sem espaco)
 *     "return 0;}"        ->  "return 0;}"     (';' e closer: sem espaco antes '}')
 *     "} int"             ->  "} int"          ('}' nao e opener; 'int' e alnum: espaco)
 *
 *   Newlines sao colapsadas: varias '\n' consecutivas viram uma unica.
 *   Comentarios (// e ( barra-asterisco ... asterisco-barra ) sao inteiramente removidos.
 */

#include "preprocessor.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// ------------------------------------------------------------------ //
//  Infraestrutura                                                     //
// ------------------------------------------------------------------ //

static int s_pass = 0;
static int s_fail = 0;

static std::string visible(const std::string& s)
{
    std::string out;
    for (unsigned char c : s) {
        switch (c) {
            case '\n': out += "\\n";  break;
            case '\t': out += "\\t";  break;
            case '\r': out += "\\r";  break;
            default:   out += static_cast<char>(c);
        }
    }
    return out;
}

static void check(const std::string& name,
                  const std::string& input,
                  const std::string& expected)
{
    const std::string got = preprocess(input);
    const bool ok = (got == expected);
    if (ok) ++s_pass; else ++s_fail;

    std::cout << (ok ? "\033[32m[PASS]\033[0m" : "\033[31m[FAIL]\033[0m")
              << "  " << name << "\n";
    if (!ok) {
        std::cout << "       entrada  : " << std::quoted(visible(input))    << "\n";
        std::cout << "       esperado : " << std::quoted(visible(expected)) << "\n";
        std::cout << "       obtido   : " << std::quoted(visible(got))      << "\n";
    }
    std::cout << "\n";
}

// ------------------------------------------------------------------ //
//  Testes                                                             //
// ------------------------------------------------------------------ //

int main()
{
    std::cout << "=============================================\n";
    std::cout << "  PREPROCESSADOR C++17 -- SUITE DE TESTES  \n";
    std::cout << "=============================================\n\n";

    // ================================================================
    std::cout << "=== 1. REMOCAO DE COMENTARIOS INLINE (//)\n\n";
    // ================================================================

    check("comentario no final da linha",
          "int x = 5; // atribuicao",
          "int x = 5;");

    check("linha inteiramente comentada",
          "// linha ignorada\nint x = 1;",
          "int x = 1;");

    check("comentario com texto especial",
          "x = y; // seta: x <- y",
          "x = y;");

    check("multiplas linhas com comentarios",
          "int a = 1; // primeiro\n"
          "int b = 2; // segundo\n"
          "int c = 3;",
          "int a = 1;\n"
          "int b = 2;\n"
          "int c = 3;");

    check("comentario sem newline no final",
          "return 0; // fim",
          "return 0;");

    check("comentario seguido de codigo na linha seguinte",
          "// comentario\nint x = 42;",
          "int x = 42;");

    // ================================================================
    std::cout << "=== 2. REMOCAO DE COMENTARIOS MULTILINHA (/* */)\n\n";
    // ================================================================

    check("bloco inline entre tokens",
          "int x = /* valor */ 5;",
          "int x = 5;");

    check("bloco ocupando multiplas linhas",
          "int x = 5;\n"
          "/* linha 1\n"
          "   linha 2 */\n"
          "int y = 10;",
          "int x = 5;\n"
          "int y = 10;");

    check("bloco no inicio do arquivo",
          "/* cabecalho */\nint x = 1;",
          "int x = 1;");

    check("bloco estilo doxygen (/** ... */)",
          "/** doc\n * param x\n */\nvoid f();",
          "void f();");

    check("asterisco dentro do bloco nao encerra",
          "/* a * b */ int x;",
          "int x;");

    check("dois blocos na mesma linha",
          "/* a */ x /* b */ = 1;",
          "x = 1;");

    check("bloco terminando com ***/ ",
          "x = /***/ 5;",
          "x = 5;");

    // ================================================================
    std::cout << "=== 3. COLAPSO DE WHITESPACE\n\n";
    // ================================================================

    check("espacos multiplos entre alnum e simbolo",
          "int   x   =   5;",
          "int x = 5;");

    check("tabs substituidos por espaco",
          "int\t\tx\t=\t5;",
          "int x = 5;");

    check("multiplas newlines colapsam em uma",
          "int x = 1;\n\n\nint y = 2;",
          "int x = 1;\n"
          "int y = 2;");

    check("espaco entre dois tokens alfanumericos",
          "int   main",
          "int main");

    check("sem espaco entre operadores adjacentes",
          "a+b",
          "a+b");

    check("entrada so com whitespace",
          "   \t\n  ",
          "");

    // Regra opener: sem espaco apos '('
    // Regra closer: sem espaco antes de ')', ';', ','
    // Simbolo->simbolo sem alnum: sem espaco
    //   "if ( x"  -> '(' opener, sem espaco antes de 'x'
    //   "1 )"     -> ')' closer, sem espaco
    //   "1 ) {"   -> ')' closer + '{' simbolo: sem espaco em nenhum dos dois
    //   "{ return"-> '{' nao e opener; 'return' e alnum -> espaco
    check("whitespace ao redor de parenteses e chaves",
          "if ( x == 1 ) { return 0; }",
          "if (x == 1){ return 0;}");

    check("sem espaco entre virgula e alnum",
          "f(a,   b,   c)",
          "f(a, b, c)");

    // ================================================================
    std::cout << "=== 4. BARRA DE DIVISAO (nao e comentario)\n\n";
    // ================================================================

    check("divisao inteira",
          "int r = 20 / 2;",
          "int r = 20 / 2;");

    check("divisao com variaveis",
          "x = a / b;",
          "x = a / b;");

    check("divisao composta (x /= 2)",
          "x /= 2;",
          "x /= 2;");

    // ================================================================
    std::cout << "=== 5. COMBINACOES\n\n";
    // ================================================================

    check("inline + whitespace na mesma linha",
          "int x = 5;   // comentario\nint y = 10;",
          "int x = 5;\n"
          "int y = 10;");

    check("bloco inline + whitespace",
          "int x   =   /* medio */ 5;",
          "int x = 5;");

    check("bloco multilinha + declaracao seguinte",
          "int x = 5;\n"
          "/* ignorar tudo isso\n"
          "   incluindo esta linha */\n"
          "int y = 10;",
          "int x = 5;\n"
          "int y = 10;");

    // Programa realista:
    // - "int b) {" -> "int b){" (')' e closer, '{' e simbolo)
    // - "return a + b;" -> "return a + b;" (alnum ao redor de '+')
    // - "}\n\nint" -> "}\nint" (newlines colapsam; '}' nao e opener, 'int' e alnum -> '\n')
    check("programa realista completo",
          "#include <iostream>\n"
          "\n"
          "// calcula soma\n"
          "int soma(int a,   int b) {\n"
          "    /* retorna soma */\n"
          "    return a + b; // resultado\n"
          "}\n"
          "\n"
          "int main() {\n"
          "    int r = soma(3,  4);\n"
          "    return 0;\n"
          "}",
          "#include <iostream>\n"
          "int soma(int a, int b){\n"
          "return a + b;}\n"
          "int main(){\n"
          "int r = soma(3, 4);\n"
          "return 0;}");

    // ================================================================
    std::cout << "=== 6. SAIDA PARA ARQUIVO\n\n";
    // ================================================================

    {
        const std::string codigo =
            "// programa de teste\n"
            "int main() {\n"
            "    /* inicializa */\n"
            "    int x = 42; // resposta\n"
            "    return x;\n"
            "}";

        const std::string caminho = "preprocessor_output.cpp";

        // Resultado esperado (consistente com as regras acima):
        // "int main(){" porque ')' e closer e '{' e simbolo
        const std::string esperado =
            "int main(){\n"
            "int x = 42;\n"
            "return x;}";

        try {
            preprocess_to_file(codigo, caminho);

            std::ifstream f(caminho);
            std::ostringstream ss; ss << f.rdbuf();
            const std::string lido = ss.str();
            const bool ok = (lido == esperado);
            if (ok) ++s_pass; else ++s_fail;

            std::cout << (ok ? "\033[32m[PASS]\033[0m" : "\033[31m[FAIL]\033[0m")
                      << "  preprocess_to_file (\" + caminho + \")\n";
            if (!ok) {
                std::cout << "       esperado : " << std::quoted(visible(esperado)) << "\n";
                std::cout << "       obtido   : " << std::quoted(visible(lido))     << "\n";
            }
        } catch (const std::exception& e) {
            ++s_fail;
            std::cout << "\033[31m[FAIL]\033[0m  preprocess_to_file: " << e.what() << "\n";
        }
        std::cout << "\n";
    }

    {
        // preprocess_file: grava um arquivo de entrada e le a saida
        const std::string entrada_path = "preprocessor_input.cpp";
        const std::string saida_path   = "preprocessor_file_out.cpp";

        const std::string codigo =
            "/* bloco */\n"
            "int x = 1; // inline\n"
            "int y = 2;";

        const std::string esperado = "int x = 1;\nint y = 2;";

        std::ofstream fin(entrada_path);
        fin << codigo;
        fin.close();

        try {
            preprocess_file(entrada_path, saida_path);

            std::ifstream fout(saida_path);
            std::ostringstream ss; ss << fout.rdbuf();
            const std::string lido = ss.str();
            const bool ok = (lido == esperado);
            if (ok) ++s_pass; else ++s_fail;

            std::cout << (ok ? "\033[32m[PASS]\033[0m" : "\033[31m[FAIL]\033[0m")
                      << "  preprocess_file (arquivo->arquivo)\n";
            if (!ok) {
                std::cout << "       esperado : " << std::quoted(visible(esperado)) << "\n";
                std::cout << "       obtido   : " << std::quoted(visible(lido))     << "\n";
            }
        } catch (const std::exception& e) {
            ++s_fail;
            std::cout << "\033[31m[FAIL]\033[0m  preprocess_file: " << e.what() << "\n";
        }
        std::cout << "\n";
    }

    // ================================================================
    //  Sumario final
    // ================================================================
    const int total = s_pass + s_fail;
    std::cout << "=============================================\n";
    std::cout << "  " << total << " testes executados\n";
    std::cout << "  \033[32m" << s_pass << " passaram\033[0m\n";
    if (s_fail > 0)
        std::cout << "  \033[31m" << s_fail << " falharam\033[0m\n";
    else
        std::cout << "  \033[32m0 falharam\033[0m  -- tudo ok!\n";
    std::cout << "=============================================\n\n";

    return (s_fail == 0) ? 0 : 1;
}