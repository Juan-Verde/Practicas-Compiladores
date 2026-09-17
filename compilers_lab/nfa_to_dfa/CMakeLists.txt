cmake_minimum_required(VERSION 3.16)
project(nfa_to_dfa C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compilar el programa principal
add_executable(nfa_to_dfa
    ./src/main.c 
    ./src/regex.c 
    ./src/nfa.c 
    ./src/dfa.c
)

# Compilar las pruebas
add_executable(test_nfa_to_dfa 
    ./src/tests/test.c 
    ./src/regex.c 
    ./src/nfa.c 
    ./src/dfa.c
)