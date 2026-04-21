/*
 * preprocessor.hpp
 *
 * Interface publica do preprocessador de codigo C++.
 */

#pragma once

#include <string>
#include <string_view>

// ------------------------------------------------------------------ //
//  Estados da maquina (enum class para type-safety)                  //
// ------------------------------------------------------------------ //
enum class State {
    Normal,           // lendo codigo normal
    Espaco_Pendente,  // viu whitespace, aguardando proximo char util
    Barra_Lida,       // viu '/', aguardando '/' ou '*'
    Comment_Linha,    // dentro de comentario inline (//)
    Comment_Bloco,    // dentro de comentario multilinha (/* */)
    Fim_Bloco         // viu '*' dentro de bloco, aguardando '/'
};

// ------------------------------------------------------------------ //
//  API publica                                                        //
// ------------------------------------------------------------------ //

/*
 * preprocess(input) -> string
 *
 * Processa o texto e retorna o codigo limpo como std::string.
 * Nao altera o arquivo original.
 */
std::string preprocess(std::string_view input);
std::string preprocess(const std::string& input);   // sobrecarga por conveniencia

/*
 * preprocess_to_file(input, output_path)
 *
 * Processa 'input' (string com o codigo) e grava o resultado em
 * 'output_path'. Lanca std::runtime_error em caso de falha de I/O.
 */
void preprocess_to_file(std::string_view input, const std::string& output_path);

/*
 * preprocess_file(input_path, output_path)
 *
 * Le o arquivo em 'input_path', processa e grava em 'output_path'.
 * Lanca std::runtime_error em caso de falha de I/O.
 */
void preprocess_file(const std::string& input_path, const std::string& output_path);