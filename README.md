# Compiladores---Trab
In this repository, we created a small compiled programming language that targets a subset of Assembler language.

###  Como compilar e executar 

Certifique-se de ter o compilador **G++** (suporte a C++17) instalado. 
Abra o terminal na raiz do projeto (`Compiladores-Trab`) e execute o seguinte comando para compilar todos os módulos:

```bash
g++ -std=c++17 src/preprocessor/preprocessor.cpp src/lexer/Lexer.cpp src/parser/SymbolTable.cpp src/parser/Parser.cpp src/preprocessor/main.cpp -o src/bin/compilador
```

### Dentro da pasta raíz, faça:

.\src\bin\compilador.exe (windows)

./src/bin/compilador (linux)

### Novo comando de compilação usado na 2a unidade(NOVO)

g++ -std=c++17 src/lexer/Lexer.cpp src/parser/SymbolTable.cpp src/parser/Parser.cpp src/preprocessor/main.cpp -o src/bin/compilador

.\src\bin\compilador.exe teste_novo.ling -tokens -ast -ts

./src/bin/compilador teste_correto.ling

## Tabela de flags

### sem flag: só compila e diz se passou (sintático + semântico)
./src/bin/compilador teste_correto.ling

### -tokens : imprime a lista de tokens
./src/bin/compilador teste_correto.ling -tokens

### -ast : imprime a árvore sintática (mostra os nós novos: MethodCall, NewObject...)
./src/bin/compilador teste_correto.ling -ast

### -ts : imprime a tabela de símbolos
./src/bin/compilador teste_correto.ling -ts

### -lex-strict : para no PRIMEIRO erro léxico (padrão é listar todos)
./src/bin/compilador arquivo_com_erro_lexico.ling -lex-strict

### -suggest : mostra a linha do erro com ^ e uma sugestão de correção
./src/bin/compilador arquivo_com_erro_sintatico.ling -suggest

### Technical Details

In this project we used C++ v. 17 and GCC GNU compiler.

We strong encourage the use of the same version.

# Participants

Raul Ramalho Lucena

Thiago de Medeiros Raquel

Moisés Átila Rodrigues Lima

Marcos Antônio Fontes Leite


### Old way to compile the project( all deprecated)

# How to compile the preprocessor(deprecated)

Inside Compiladores-Trab folder, try:

>g++ -std=c++17 src/preprocessor/preprocessor.cpp src/preprocessor/tests.cpp -o src/bin/test

If you want to see manually the .java file. Do:

>preprocess_file("your_input_file.java", "src/bin/saida.java");

Then, with the compiled src/bin/test file:

>./src/bin/test src/your_input_java_file.java src/bin/your_formated_java_file.java

# How to compile the lexer(deprecated)

Inside Compiladores-Trab folder, try:

>javac -cp src src/lexer/Main.java src/parser/Parser.java src/parser/SymbolTable.java

>java -cp src lexer.Main

# How to compile the parser(deprecated)

Inside Compiladores-Trab folder, try:

>javac -cp src src/parser/SymbolTable.java src/parser/Parser.java

also add on the same line:

>src/parser/*.java

and then:

>java -cp src lexer.Main

