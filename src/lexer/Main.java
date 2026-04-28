package lexer;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

public class Main {
    public static void main(String[] args) {
        // Caminho do arquivo limpo que o pré-processador em C++ gerou
        String caminhoArquivo = "src/bin/saida.java";

        try {
            // 1. Lê todo o conteúdo do arquivo e salva em uma String
            String input = new String(Files.readAllBytes(Paths.get(caminhoArquivo)));
            
            System.out.println("=== CÓDIGO RECEBIDO DO PRÉ-PROCESSADOR ===");
            System.out.println(input);
            System.out.println("==========================================\n");

            // 2. Passa a String para o Lexer
            Lexer lexer = new Lexer(input);
            
            // 3. Pede para o Lexer fatiar tudo em Tokens
            List<Token> tokens = lexer.tokenize();

            // 4. Imprime token por token na tela
            System.out.println("=== LISTA DE TOKENS GERADA ===");
            for (Token token : tokens) {
                System.out.println(token);
            }

        } catch (IOException e) {
            System.err.println("Erro ao ler o arquivo: " + e.getMessage());
            System.err.println("Dica: Certifique-se de rodar o programa C++ primeiro para gerar o arquivo saida.java!");
        }
    }
}