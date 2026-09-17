#ifndef DFA_H
#define DFA_H

#include "nfa.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @file dfa.h
 * @brief Definición del Autómata Finito Determinista (DFA) y las operaciones
 *        para la conversión desde un NFA (Algoritmo de Construcción de Subconjuntos).
 */

/**
 * @struct dfa_state
 * @brief Representa un estado del DFA como un subconjunto de estados del NFA.
 *
 * Cada estado del DFA es, por definición de la construcción por subconjuntos,
 * un subconjunto de estados del NFA original. El arreglo `nfa_states` es un
 * arreglo booleano de tamaño `nfa.state_count`, donde `nfa_states[i] == true`
 * indica que el estado `i` del NFA pertenece a este subconjunto.
 */
typedef struct
{
    int id;              /**< Identificador único del estado en el DFA. */
    bool *nfa_states;    /**< Subconjunto de estados del NFA (arreglo booleano). */
    bool is_accept;      /**< true si el estado es de aceptación. */
} dfa_state;

/**
 * @struct dfa_transition
 * @brief Representa una transición determinista del DFA.
 */
typedef struct
{
    int from;    /**< Estado origen en el DFA. */
    int to;      /**< Estado destino en el DFA. */
    char symbol; /**< Símbolo que provoca la transición. */
} dfa_transition;

/**
 * @struct dfa
 * @brief Representa el Autómata Finito Determinista completo.
 *
 * Los arreglos `states` y `transitions` crecen dinámicamente conforme se
 * descubren nuevos subconjuntos durante la construcción. Las capacidades
 * se duplican cuando se llenan (ver `ensure_state_capacity` y
 * `ensure_transition_capacity` en dfa.c).
 */
typedef struct
{
    int start;                          /**< ID del estado inicial. */
    dfa_state *states;                  /**< Arreglo dinámico de estados. */
    int state_count;                    /**< Número actual de estados. */
    int state_capacity;                 /**< Capacidad actual del arreglo. */
    dfa_transition *transitions;        /**< Arreglo dinámico de transiciones. */
    int transition_count;               /**< Número actual de transiciones. */
    int transition_capacity;            /**< Capacidad actual del arreglo. */
} dfa;

/**
 * @brief Convierte un NFA en un DFA equivalente.
 *        Implementa el Algoritmo 3 (Construcción de Subconjuntos).
 *
 * @param n NFA de entrada (pasado por valor, no se modifica).
 * @return DFA equivalente con estados alcanzables.
 */
dfa nfa_to_dfa(nfa n);

/**
 * @brief Algoritmo 1 de la Práctica 2: Move(T, a).
 *        Regresa el conjunto de estados alcanzables desde T consumiendo
 *        exactamente el símbolo 'a'.
 *
 * @param n NFA de entrada.
 * @param T Conjunto de estados origen (arreglo booleano de tamaño n.state_count).
 * @param a Símbolo de entrada.
 * @return Arreglo booleano con los estados destino (memoria reservada con malloc,
 *         el llamador debe liberarla).
 */
bool* move_operation(nfa n, bool *T, char a);

/**
 * @brief Imprime en consola la tabla de transiciones del DFA
 *        para validación visual.
 */
void print_dfa_table(dfa d);

/**
 * @brief Libera toda la memoria asociada al DFA.
 *        Es seguro llamarla con un DFA parcialmente inicializado.
 */
void free_dfa(dfa *d);

#endif