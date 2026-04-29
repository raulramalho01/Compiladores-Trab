/* * Bem-vindo à Dungeon Diagonal. 
 * Este é um comentário de bloco longo
 * que deve ser totalmente removido.
 */

class Heroi {
    // Atributos do personagem
    int nivel = 6; 
    String nome = "Aranya"; // Comentário de fim de linha

    /* Comentário de uma linha só em bloco */
    public void atacar() {
        int danoBase = 10;
        
        // O pré-processador deve manter o espaço entre 'int' e 'total'
        // Mas pode remover espaços ao redor do '+' e '='
        int total = danoBase + nivel * 2; 
        
        if (total > 15) {
            /* O multiplicador abaixo não deve sumir, 
            mesmo que tenha um / por perto */
            int critico = total * 2; // total*2;
            System.out.println("Ataque Crítico!");
        }
    }
}

/** * Teste de borda: / no meio de comentário // 
 * E um asterisco isolado *
 */