package lexer;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class Lexer {
    private final String input;
    private int position = 0;
    private int line = 1;
    private int column = 1;

    // Tabela de Palavras Reservadas extraídas da gramática
    private static final Set<String> KEYWORDS = Set.of(
        "class", "public", "static", "void", "main", "String", "extends",
        "return", "int", "boolean", "if", "else", "while", "System",
        "out", "println", "new", "length", "true", "false", "this"
    );

    public Lexer(String input) {
        this.input = input;
    }

    public List<Token> tokenize() {
        List<Token> tokens = new ArrayList<>();
        Token token;
        do {
            token = nextToken();
            tokens.add(token);
        } while (token.getType() != TokenType.EOF);
        return tokens;
    }

    public Token nextToken() {
        if (position >= input.length()) {
            return new Token(TokenType.EOF, "", line, column);
        }

        char currentChar = input.charAt(position);

        // 1. Pular espaços em branco
        if (Character.isWhitespace(currentChar)) {
            advance();
            return nextToken();
        }

        int startColumn = column;
        int startLine = line;

        // 2. Identificar Números
        if (Character.isDigit(currentChar)) {
            return readNumber(startLine, startColumn);
        }

        // 3. Identificar Letras (Identificadores ou Palavras Reservadas)
        if (Character.isLetter(currentChar)) {
            return readIdentifierOrKeyword(startLine, startColumn);
        }

        // 4. Identificar Operador de dois caracteres (&&)
        if (currentChar == '&') {
            advance();
            if (position < input.length() && input.charAt(position) == '&') {
                advance();
                return new Token(TokenType.OPERATOR, "&&", startLine, startColumn);
            }
            return new Token(TokenType.ERROR, "&", startLine, startColumn); // '&' sozinho é erro nesta gramática
        }

        // 5. Identificar Operadores simples e Delimitadores
        String symbol = String.valueOf(currentChar);
        if (">+-*!=".contains(symbol)) {
            advance();
            return new Token(TokenType.OPERATOR, symbol, startLine, startColumn);
        }

        if ("{}()[];,.".contains(symbol)) {
            advance();
            return new Token(TokenType.DELIMITER, symbol, startLine, startColumn);
        }

        // 6. Tratamento de Erro Léxico para caracteres não reconhecidos (ex: @, #)
        advance();
        return new Token(TokenType.ERROR, symbol, startLine, startColumn);
    }

    private void advance() {
        if (position < input.length()) {
            if (input.charAt(position) == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            position++;
        }
    }

    private Token readNumber(int startLine, int startColumn) {
        StringBuilder builder = new StringBuilder();
        
        while (position < input.length() && Character.isDigit(input.charAt(position))) {
            builder.append(input.charAt(position));
            advance();
        }
        
        return new Token(TokenType.NUMBER, builder.toString(), startLine, startColumn);
    }

    private Token readIdentifierOrKeyword(int startLine, int startColumn) {
        StringBuilder builder = new StringBuilder();
        
        // A gramática diz que pode conter letras, números ou '_'
        while (position < input.length() && 
              (Character.isLetterOrDigit(input.charAt(position)) || input.charAt(position) == '_')) {
            builder.append(input.charAt(position));
            advance();
        }
        
        String lexeme = builder.toString();
        
        // Verifica se a palavra formada está na nossa lista de palavras reservadas
        if (KEYWORDS.contains(lexeme)) {
            return new Token(TokenType.RESERVED_WORD, lexeme, startLine, startColumn);
        }
        
        return new Token(TokenType.IDENTIFIER, lexeme, startLine, startColumn);
    }
}