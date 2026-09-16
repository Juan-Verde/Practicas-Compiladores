#ifndef DFA_H
#define DFA_H

#include "nfa.h"
#include <stdbool.h>
#include <stddef.h>

/*
 * Representa un estado del DFA.
 * Cada estado del DFA es en realidad un subconjunto 
 * de estados del NFA. El arreglo nfa_states indica 
 * mediante valores booleanos qué estados del AFN 
 * pertenecen a dicho subconjunto.
 */
typedef struct
{
    int id;
    bool *nfa_states;
    bool is_accept;
} dfa_state;

/*
 * Representa una transición determinista.
 */
typedef struct
{
    int from;
    int to;
    char symbol;
} dfa_transition;

/*
 * Representa el Autómata Finito Determinista completo.
 */
typedef struct
{
    int start;

    dfa_state *states;
    int state_count;
    int state_capacity;

    dfa_transition *transitions;
    int transition_count;
    int transition_capacity;

} dfa;

/*
 * Convierte un NFA en un DFA aplicando el Algoritmo de 
 * Construcción de Subconjuntos.
 */
dfa nfa_to_dfa(nfa n);

/*
 * Algoritmo 1 de la Practica 2: Move(T, a)
 * Regresa el conjunto de estados alcanzables desde T
 * consumiendo exactamente el simbolo 'a'.
 */
bool* move_operation(nfa n, bool *T, char a);

/*
 * Imprime la tabla de transiciones
 */
void print_dfa_table(dfa d);

/*
 * Libera la memoria utilizada por el DFA.
 */
void free_dfa(dfa *d);

#endif