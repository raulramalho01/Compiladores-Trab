#pragma once

enum class TokenType {
    RESERVED_WORD,
    IDENTIFIER,
    NUMBER,
    OPERATOR,
    DELIMITER,
    ERROR_TOKEN,
    TOKEN_EOF // Renomeado para evitar conflito com a macro padrão EOF do C
};