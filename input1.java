class Principal {
    public static void main(String[] args) {
        System.out.println(0);
    }
}

class Heroi extends Personagem {
    int nivel;
    boolean vivo;

    public int atacar(int danoBase) {
        int total;
        int critico;
        {
            total = danoBase + nivel * 2;
            if (total > 15) {
                critico = total * 2;
            } else {
                critico = total;
            }
        }
        return critico;
    }
}