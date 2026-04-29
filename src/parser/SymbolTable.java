package parser;

import java.util.HashMap;
import java.util.Map;

public class SymbolTable {
    // Guarda o nome do Identificador (chave) e o Tipo (valor)
    private Map<String, String> table;

    public SymbolTable() {
        this.table = new HashMap<>();
    }

    public void add(String name, String type) {
        table.put(name, type);
    }

    public boolean exists(String name) {
        return table.containsKey(name);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder("=== TABELA DE SÍMBOLOS ===\n");
        for (Map.Entry<String, String> entry : table.entrySet()) {
            sb.append(String.format("Variável: %-15s | Tipo: %s\n", entry.getKey(), entry.getValue()));
        }
        return sb.toString();
    }
}