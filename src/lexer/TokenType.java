package lexer;

public enum TokenType {
    // Palavras reservadas (ex: if, while, int)
    RESERVED_WORD,
    
    // Identificadores (nomes de variáveis, funções)
    IDENTIFIER,
    
    // Literais numéricos e de texto
    NUMBER,
    STRING_LITERAL,
    
    // Operadores (+, -, =, ==, >, etc)
    OPERATOR,
    
    // Delimitadores (;, ,, (, ), {, })
    DELIMITER,
    
    // Marcador de fim de arquivo
    EOF,
    
    // Caso de erro léxico
    ERROR
}