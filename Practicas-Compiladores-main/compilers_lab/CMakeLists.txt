cmake_minimum_required(VERSION 3.16)
project(nfa_to_dfa C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Programa principal
add_executable(nfa_to_dfa
    ./src/main.c
    ./src/nfa.c
    ./src/regex.c
    ./src/dfa.c
)

# Ejecutable de pruebas unitarias
add_executable(test_nfa_to_dfa
    ./src/tests/test.c
    ./src/nfa.c
    ./src/regex.c
    ./src/dfa.c
)

target_include_directories(nfa_to_dfa PRIVATE ./src)
target_include_directories(test_nfa_to_dfa PRIVATE ./src)