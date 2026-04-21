class Heroi {
int nivel = 5;
String nome ="Aranya";
public void atacar(){
int danoBase = 10;
int total = danoBase + nivel * 2;
if (total> 15){
int critico = total * 2;
System.out.println("Ataque Crítico!");}}}