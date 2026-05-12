#include "preprocessor.hpp"
#include <fstream>
#include <sstream>

int main() {
    preprocess_file("input1.java", "src/bin/output.java");
    preprocess_file("Program1.ling", "src/bin/Program1.java");
    preprocess_file("Program2.ling", "src/bin/Program2.java");
    preprocess_file("Program3.ling", "src/bin/Program3.java");
    preprocess_file("Program4.ling", "src/bin/Program4.java");
    preprocess_file("Program5.ling", "src/bin/Program5.java");
}