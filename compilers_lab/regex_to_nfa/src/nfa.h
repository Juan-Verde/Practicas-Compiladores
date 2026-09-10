#ifndef NFA_H
#define NFA_H

#include "regex.h"

#include <stdbool.h>
#include <stddef.h>


/*
 * Representamos la transición del NFA.
 *
 * from: estado origen
 * to: estado destino
 * symbol: carácter que provoca la transición
 * epsilon: indica si la transición es epsilon
 */
typedef struct
{
    int from;
    int to;
    char symbol;
    bool epsilon;

} transition;


/*
 * Representa el autómata completo.
 */
typedef struct
{
    int start;
    int accept;

    int state_count;

    transition *transitions;

    int transition_count;
    int transition_capacity;

} nfa;


/*
 * Convierte la regex postfija en un NFA
 * utilizamos el algoritmo de Thompson.
 */
nfa regex_to_nfa(regex r);


/*
 * Determina si una cadena es aceptada por el NFA.
 */
int match_nfa(
    nfa n,
    const char *text,
    size_t len
);


/*
 * Libera la memoria utilizada por el NFA.
 */
void free_nfa(nfa *n);


/*
 * Guarda el NFA en un archivo.
 */
bool save_nfa(
    const nfa *n,
    const char *path
);


#endif