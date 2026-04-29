package parser;

import java.util.List;

import lexer.Token;
import lexer.TokenType;

public class Parser {
    private final List<Token> tokens;
    private int current = 0;
    private SymbolTable symbolTable;

    public Parser(List<Token> tokens) {
        this.tokens = tokens;
        this.symbolTable = new SymbolTable();
    }

    // Método principal para iniciar a análise
    public void parse() {
        try {
            parseProg();
            
            // Exigência 4: Mensagem de sucesso e tabela
            System.out.println("\n[SUCESSO] Código está correto sintaticamente!");
            System.out.println(symbolTable);
            
        } catch (RuntimeException e) {
            // Exigência 5: Informar o local exato e o tipo do erro
            System.err.println("\n[ERRO SINTÁTICO] " + e.getMessage());
        }
    }

    // Prog -> MainC DefCl
    private void parseProg() {
        parseMainC();
        parseDefCl();
        
        if (!isAtEnd()) {
            throw error(peek(), "Esperado fim de arquivo, mas encontrou " + peek().getLexeme());
        }
    }

    // MainC -> 'class' Id '{' 'public' 'static' 'void' 'main' '(' 'String' '[' ']' Id ')' '{' Cmd '}' '}'
    private void parseMainC() {
        consume(TokenType.RESERVED_WORD, "class", "Esperado palavra reservada 'class' no início.");
        
        Token className = consume(TokenType.IDENTIFIER, "Esperado nome da classe (Identificador).");
        symbolTable.add(className.getLexeme(), "class"); // Registra a classe principal
        
        consume(TokenType.DELIMITER, "{", "Esperado '{' após nome da classe.");
        consume(TokenType.RESERVED_WORD, "public", "Esperado 'public'.");
        consume(TokenType.RESERVED_WORD, "static", "Esperado 'static'.");
        consume(TokenType.RESERVED_WORD, "void", "Esperado 'void'.");
        consume(TokenType.RESERVED_WORD, "main", "Esperado 'main'.");
        consume(TokenType.DELIMITER, "(", "Esperado '('.");
        consume(TokenType.RESERVED_WORD, "String", "Esperado 'String'.");
        consume(TokenType.DELIMITER, "[", "Esperado '['.");
        consume(TokenType.DELIMITER, "]", "Esperado ']'.");
        
        Token argName = consume(TokenType.IDENTIFIER, "Esperado nome do argumento do main.");
        symbolTable.add(argName.getLexeme(), "String[]");
        
        consume(TokenType.DELIMITER, ")", "Esperado ')'.");
        consume(TokenType.DELIMITER, "{", "Esperado '{' para iniciar o main.");
        
        parseCmd(); // Chama a regra Cmd
        
        consume(TokenType.DELIMITER, "}", "Esperado '}' para fechar o main.");
        consume(TokenType.DELIMITER, "}", "Esperado '}' para fechar a classe principal.");
    }

    // Exemplo de como fica um método Fatorado (Type)
    private String parseType() {
        if (check(TokenType.RESERVED_WORD, "boolean")) {
            advance();
            return "boolean";
        } else if (check(TokenType.IDENTIFIER)) {
            Token id = advance();
            return id.getLexeme();
        } else if (check(TokenType.RESERVED_WORD, "int")) {
            advance();
            // Implementação da regra Type' (int [])
            if (check(TokenType.DELIMITER, "[")) {
                advance();
                consume(TokenType.DELIMITER, "]", "Esperado ']' após '[' na declaração de vetor.");
                return "int[]";
            }
            return "int";
        }
        throw error(peek(), "Tipo inválido. Esperado 'int', 'boolean' ou um Identificador.");
    }

    // ==========================================
    // Funções Auxiliares do Motor do Parser
    // ==========================================

    // Método auxiliar para prever se estamos diante de um Tipo
    private boolean isType() {
        if (check(TokenType.RESERVED_WORD, "int") || check(TokenType.RESERVED_WORD, "boolean")) {
            return true;
        }
        // Se for um Identificador, damos um "lookahead" (espiadinha) no próximo token.
        // Se o próximo for outro Identificador, é uma declaração (ex: Heroi nivel;)
        if (check(TokenType.IDENTIFIER) && current + 1 < tokens.size()) {
            if (tokens.get(current + 1).getType() == TokenType.IDENTIFIER) {
                return true;
            }
        }
        return false;
    }

    // DefVar -> Type Id ';' DefVar | λ
    private void parseDefVar() {
        // Enquanto o próximo token for um tipo válido, continuamos lendo variáveis
        while (isType()) {
            String tipoDaVariavel = parseType();
            
            Token nomeDaVariavel = consume(TokenType.IDENTIFIER, "Esperado nome da variável.");
            consume(TokenType.DELIMITER, ";", "Esperado ';' após declaração da variável '" + nomeDaVariavel.getLexeme() + "'.");
            
            // Registra a variável na Tabela de Símbolos
            symbolTable.add(nomeDaVariavel.getLexeme(), tipoDaVariavel);
        }
    }

    // Args -> Type Id | Type Id ',' Args
    private void parseArgs() {
        String tipoArg = parseType();
        Token nomeArg = consume(TokenType.IDENTIFIER, "Esperado nome do argumento.");
        symbolTable.add(nomeArg.getLexeme(), tipoArg);

        // Se tiver uma vírgula, fazemos um loop para pegar os próximos
        while (check(TokenType.DELIMITER, ",")) {
            advance(); // Consome a vírgula
            tipoArg = parseType();
            nomeArg = consume(TokenType.IDENTIFIER, "Esperado nome do argumento após ','.");
            symbolTable.add(nomeArg.getLexeme(), tipoArg);
        }
    }

    // DefMet -> 'public' Type Id '(' Args ')' '{' DefVar Cmd 'return' Exp ';' '}' DefMet | λ
    private void parseDefMet() {
        // Enquanto encontrar 'public', significa que há um método para ler
        while (check(TokenType.RESERVED_WORD, "public")) {
            advance(); // Consome o 'public'
            
            String tipoRetorno = parseType();
            Token nomeMetodo = consume(TokenType.IDENTIFIER, "Esperado nome do método.");
            symbolTable.add(nomeMetodo.getLexeme(), "Método (" + tipoRetorno + ")");
            
            consume(TokenType.DELIMITER, "(", "Esperado '(' após o nome do método.");
            
            // Se o próximo token NÃO for ')', então temos argumentos para processar
            if (!check(TokenType.DELIMITER, ")")) {
                parseArgs();
            }
            
            consume(TokenType.DELIMITER, ")", "Esperado ')' para fechar os argumentos.");
            consume(TokenType.DELIMITER, "{", "Esperado '{' para iniciar o corpo do método.");
            
            // Lê o recheio do método
            parseDefVar();
            parseCmd(); // Ainda vamos implementar
            
            consume(TokenType.RESERVED_WORD, "return", "Esperado 'return' no final do método.");
            parseExp(); // Ainda vamos implementar
            consume(TokenType.DELIMITER, ";", "Esperado ';' após a expressão de retorno.");
            consume(TokenType.DELIMITER, "}", "Esperado '}' para fechar o método.");
        }
    }

    // DefCl -> 'class' Id '{' DefVar DefMet '}' DefCl 
    //        | 'class' Id 'extends' Id '{' DefVar DefMet '}' DefCl 
    //        | λ
    private void parseDefCl() {
        // Enquanto encontrar a palavra 'class', tem classe nova sendo criada
        while (check(TokenType.RESERVED_WORD, "class")) {
            advance(); // Consome o 'class'
            
            Token nomeClasse = consume(TokenType.IDENTIFIER, "Esperado nome da classe.");
            symbolTable.add(nomeClasse.getLexeme(), "Classe");
            
            // Lógica do 'extends' (Fatoração à esquerda aplicada aqui!)
            if (check(TokenType.RESERVED_WORD, "extends")) {
                advance(); // Consome o 'extends'
                consume(TokenType.IDENTIFIER, "Esperado nome da classe pai após 'extends'.");
            }
            
            consume(TokenType.DELIMITER, "{", "Esperado '{' para iniciar o corpo da classe.");
            
            parseDefVar();
            parseDefMet();
            
            consume(TokenType.DELIMITER, "}", "Esperado '}' para fechar a classe.");
        }
    }

    // Cmd -> '{' Cmd '}' | 'if' '(' Exp ')' Cmd 'else' Cmd | 'while' '(' Exp ')' Cmd 
    //      | 'System.out.println' '(' Exp ')' ';' | Id '=' Exp ';' | Id '[' Exp ']' '=' Exp ';'
    private void parseCmd() {
        if (check(TokenType.DELIMITER, "{")) {
            advance();
            // A gramática diz '{' Cmd '}', mas como na prática blocos têm vários comandos, 
            // usamos um loop até encontrar a chave de fechamento.
            while (!check(TokenType.DELIMITER, "}")) {
                parseCmd();
            }
            consume(TokenType.DELIMITER, "}", "Esperado '}' para fechar o bloco de comandos.");
        } 
        else if (check(TokenType.RESERVED_WORD, "if")) {
            advance();
            consume(TokenType.DELIMITER, "(", "Esperado '(' após 'if'.");
            parseExp();
            consume(TokenType.DELIMITER, ")", "Esperado ')' após expressão do 'if'.");
            parseCmd();
            consume(TokenType.RESERVED_WORD, "else", "Esperado 'else' após o comando do 'if'.");
            parseCmd();
        } 
        else if (check(TokenType.RESERVED_WORD, "while")) {
            advance();
            consume(TokenType.DELIMITER, "(", "Esperado '(' após 'while'.");
            parseExp();
            consume(TokenType.DELIMITER, ")", "Esperado ')' após expressão do 'while'.");
            parseCmd();
        } 
        else if (check(TokenType.RESERVED_WORD, "System")) {
            advance();
            consume(TokenType.DELIMITER, ".", "Esperado '.' após System.");
            consume(TokenType.RESERVED_WORD, "out", "Esperado 'out'.");
            consume(TokenType.DELIMITER, ".", "Esperado '.' após out.");
            consume(TokenType.RESERVED_WORD, "println", "Esperado 'println'.");
            consume(TokenType.DELIMITER, "(", "Esperado '('.");
            parseExp();
            consume(TokenType.DELIMITER, ")", "Esperado ')'.");
            consume(TokenType.DELIMITER, ";", "Esperado ';' após println.");
        } 
        else if (check(TokenType.IDENTIFIER)) {
            // Fatoração à Esquerda: Lê o Identificador e olha o que vem depois!
            Token id = advance();
            
            // É um vetor? (Id '[' Exp ']')
            if (check(TokenType.DELIMITER, "[")) {
                advance();
                parseExp();
                consume(TokenType.DELIMITER, "]", "Esperado ']' na atribuição do vetor.");
            }
            
            consume(TokenType.OPERATOR, "=", "Esperado '=' na atribuição da variável '" + id.getLexeme() + "'.");
            parseExp();
            consume(TokenType.DELIMITER, ";", "Esperado ';' no fim da atribuição.");
        } 
        else {
            throw error(peek(), "Comando inválido. Esperado '{', 'if', 'while', 'System' ou uma atribuição.");
        }
    }

    // Método Auxiliar: Termo -> Coisas que não começam com 'Exp' 
    private void parseTermo() {
        if (check(TokenType.RESERVED_WORD, "true") || check(TokenType.RESERVED_WORD, "false") || 
            check(TokenType.RESERVED_WORD, "this") || check(TokenType.NUMBER)) {
            advance();
        } 
        else if (check(TokenType.IDENTIFIER)) {
            advance(); // Pode ser uma variável solta na expressão
        } 
        else if (check(TokenType.RESERVED_WORD, "new")) {
            // Fatoração: new int[...] vs new Id()
            advance();
            if (check(TokenType.RESERVED_WORD, "int")) {
                advance();
                consume(TokenType.DELIMITER, "[", "Esperado '['.");
                parseExp();
                consume(TokenType.DELIMITER, "]", "Esperado ']'.");
            } else if (check(TokenType.IDENTIFIER)) {
                advance();
                consume(TokenType.DELIMITER, "(", "Esperado '('.");
                consume(TokenType.DELIMITER, ")", "Esperado ')'.");
            } else {
                throw error(peek(), "Esperado 'int' ou Identificador após 'new'.");
            }
        } 
        else if (check(TokenType.OPERATOR, "!")) {
            advance();
            parseExp();
        } 
        else if (check(TokenType.DELIMITER, "(")) {
            advance();
            parseExp();
            consume(TokenType.DELIMITER, ")", "Esperado ')'.");
        } 
        else {
            throw error(peek(), "Expressão inválida. Encontrado: " + peek().getLexeme());
        }
    }

    // ExpPrincipal: Lê o termo base e engole os operadores que vierem na sequência
    private void parseExp() {
        parseTermo(); // Base
        
        // Loop que resolve a recursão à esquerda (o Exp')
        while (true) {
            if (check(TokenType.OPERATOR, "&&") || check(TokenType.OPERATOR, ">") ||
                check(TokenType.OPERATOR, "+")  || check(TokenType.OPERATOR, "-") ||
                check(TokenType.OPERATOR, "*")) {
                advance();
                parseExp();
            } 
            else if (check(TokenType.DELIMITER, "[")) {
                advance();
                parseExp();
                consume(TokenType.DELIMITER, "]", "Esperado ']'.");
            } 
            else if (check(TokenType.DELIMITER, ".")) {
                advance();
                if (check(TokenType.RESERVED_WORD, "length")) {
                    advance();
                } else { // Chamada de método encadeado: . Exp ( ListExp )
                    parseExp();
                    consume(TokenType.DELIMITER, "(", "Esperado '('.");
                    parseListExp();
                    consume(TokenType.DELIMITER, ")", "Esperado ')'.");
                }
            } 
            else {
                break; // Se não tiver mais operadores ou chamadas, a expressão acabou.
            }
        }
    }

    // Auxiliar: ListExp -> Exp ',' ListExp | Exp | λ
    private void parseListExp() {
        if (check(TokenType.DELIMITER, ")")) return; // Tratamento do λ (vazio)
        
        parseExp();
        while (check(TokenType.DELIMITER, ",")) {
            advance();
            parseExp();
        }
    }

    // ==========================================
    // 4. Funções Auxiliares do Motor do Parser (continuam no final do arquivo)
    // ==========================================

    private Token consume(TokenType type, String errorMessage) {
        if (check(type)) return advance();
        throw error(peek(), errorMessage);
    }

    private Token consume(TokenType type, String lexeme, String errorMessage) {
        if (check(type, lexeme)) return advance();
        throw error(peek(), errorMessage);
    }

    private boolean check(TokenType type) {
        if (isAtEnd()) return false;
        return peek().getType() == type;
    }

    private boolean check(TokenType type, String lexeme) {
        if (isAtEnd()) return false;
        return peek().getType() == type && peek().getLexeme().equals(lexeme);
    }

    private Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    private boolean isAtEnd() {
        return peek().getType() == TokenType.EOF;
    }

    private Token peek() {
        return tokens.get(current);
    }

    private Token previous() {
        return tokens.get(current - 1);
    }

    private RuntimeException error(Token token, String message) {
        String baseMsg = String.format("Erro na linha %d, coluna %d: %s", token.getLine(), token.getColumn(), message);
        if (token.getType() == TokenType.EOF) {
            return new RuntimeException(baseMsg + " (Fim de arquivo inesperado)");
        } else {
            return new RuntimeException(baseMsg + " (Token encontrado: '" + token.getLexeme() + "')");
        }
    }
}