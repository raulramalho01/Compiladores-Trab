package lexer;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

import parser.Parser; 

public class Main {
    public static void main(String[] args) {
        String caminhoArquivo = (args.length > 0) ? args[0] : "src/bin/Program1.java";

        try {
            // lê todo o conteúdo do arquivo e salva em uma String
            String input = new String(Files.readAllBytes(Paths.get(caminhoArquivo)));
            
            System.out.println("=== CÓDIGO RECEBIDO DO PRÉ-PROCESSADOR ===");
            System.out.println(input);
            System.out.println("==========================================\n");

            // Passa a String para o Lexer
            Lexer lexer = new Lexer(input);
            
            // Pede para o Lexer fatiar tudo em Tokens
            List<Token> tokens = lexer.tokenize();

            // Imprime token por token na tela
            System.out.println("=== LISTA DE TOKENS GERADA ===");
            for (Token token : tokens) {
                System.out.println(token);
            }

            // Aciona o Analisador Sintático com a lista de tokens
            System.out.println("\n=== INICIANDO ANALISADOR SINTÁTICO ===");
            Parser parser = new Parser(tokens);
            parser.parse();

        } catch (IOException e) {
            System.err.println("Erro ao ler o arquivo: " + e.getMessage());
            System.err.println("Dica: Certifique-se de rodar o programa C++ primeiro para gerar o arquivo saida.java!");
        }
    }
}