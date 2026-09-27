#ifndef MIN_DFA_H
#define MIN_DFA_H

#include "dfa.h"

/*
 * Este header solo agrupa las funciones de minimización.
 * Las declaraciones ya están en dfa.h, pero se repiten aquí
 * por claridad y para que main.c pueda incluirlo directamente.
 */

dfa minimize_dfa(dfa d);
void print_dfa_min(dfa d);
bool test_string(dfa d, const char *input);

#endif