# Compiladores---Trab
In this repository, we created a small compiled programming language that targets a subset of Assembler language.

# How to compile the preprocessor

Inside Compiladores-Trab folder, try:

>g++ -std=c++17 src/preprocessor/preprocessor.cpp src/preprocessor/tests.cpp -o src/bin/test

If you want to see manually the .java file. Do:

>preprocess_file("your_input_file.java", "src/bin/saida.java");

Then, with the compiled src/bin/test file:

>./src/bin/test src/your_input_java_file.java src/bin/your_formated_java_file.java

# How to compile the lexer

Inside Compiladores-Trab folder, try:

>javac src/lexer/*.java

>java -cp src lexer.Main

# How to compile the parser

Inside Compiladores-Trab folder, try:

>javac src/lexer/*.java

also add on the same line:

src/parser/*.java

and then:

java -cp src lexer.Main


# Technical Details

In this project we used C++ v. 17 and GCC GNU compiler.

We strong encourage the use of the same version.

# Participants

Raul Ramalho Lucena

Thiago Raquel
