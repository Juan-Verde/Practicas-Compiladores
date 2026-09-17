#include "dfa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file dfa.c
 * @brief Implementación del algoritmo de construcción de subconjuntos (NFA -> DFA).
 *
 * Este archivo contiene:
 *   - move_operation: Algoritmo 1 de la práctica.
 *   - nfa_to_dfa:     Algoritmo 3 (construcción de subconjuntos).
 *   - Utilidades para manejo dinámico de memoria (realloc).
 *
 * NOTA SOBRE ROBUSTEZ:
 *   Todas las operaciones de crecimiento de arreglos usan realloc y verifican
 *   el resultado. Si realloc falla, el programa termina con EXIT_FAILURE en
 *   lugar de continuar con memoria corrupta.
 */

/* ------------------------------------------------------------------ */
/* Funciones auxiliares internas                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Verifica si un conjunto representado como arreglo booleano está vacío.
 *
 * @param set  Arreglo booleano.
 * @param size Tamaño del arreglo.
 * @return true si ningún elemento es true.
 */
static bool is_empty_set(bool *set, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (set[i]) return false;
    }
    return true;
}

/**
 * @brief Busca un subconjunto V dentro del arreglo de estados del DFA.
 *
 * @param d              DFA donde se busca.
 * @param V              Subconjunto a buscar (arreglo booleano).
 * @param nfa_state_count Tamaño de los subconjuntos (= n.state_count).
 * @return ID del estado si se encuentra, o -1 si no existe.
 */
static int find_state_in_QD(dfa *d, bool *V, int nfa_state_count)
{
    for (int i = 0; i < d->state_count; i++)
    {
        bool match = true;
        for (int j = 0; j < nfa_state_count; j++)
        {
            if (d->states[i].nfa_states[j] != V[j])
            {
                match = false;
                break;
            }
        }
        if (match) return d->states[i].id;
    }
    return -1;
}

/**
 * @brief Extrae el alfabeto (Sigma) del NFA, sin duplicados.
 *
 * @param n          NFA de entrada.
 * @param alphabet   Arreglo de salida (debe tener al menos 256 posiciones).
 * @param alpha_size Puntero donde se escribe el tamaño del alfabeto.
 */
static void get_alphabet(nfa n, char *alphabet, int *alpha_size)
{
    *alpha_size = 0;
    for (int i = 0; i < n.transition_count; i++)
    {
        char sym = n.transitions[i].symbol;
        if (!n.transitions[i].epsilon)
        {
            bool exists = false;
            for (int j = 0; j < *alpha_size; j++) {
                if (alphabet[j] == sym) exists = true;
            }
            if (!exists) {
                alphabet[*alpha_size] = sym;
                (*alpha_size)++;
            }
        }
    }
}

/**
 * @brief Asegura que el arreglo de estados tenga capacidad suficiente.
 *        Si state_count >= state_capacity, duplica la capacidad con realloc.
 *
 * @param d DFA a redimensionar.
 *
 * @note Si realloc falla, termina el programa para evitar corrupción de memoria.
 *       Esta es una decisión de diseño: preferimos abortar que continuar con
 *       un arreglo parcialmente redimensionado.
 */
static void ensure_state_capacity(dfa *d)
{
    if (d->state_count < d->state_capacity) return;

    int new_capacity = d->state_capacity * 2;
    dfa_state *new_states = realloc(d->states, sizeof(dfa_state) * new_capacity);

    if (new_states == NULL)
    {
        fprintf(stderr, "Error fatal: no se pudo redimensionar el arreglo de estados.\n");
        exit(EXIT_FAILURE);
    }

    d->states = new_states;
    d->state_capacity = new_capacity;
}

/**
 * @brief Asegura que el arreglo de transiciones tenga capacidad suficiente.
 *        Si transition_count >= transition_capacity, duplica la capacidad.
 *
 * @param d DFA a redimensionar.
 */
static void ensure_transition_capacity(dfa *d)
{
    if (d->transition_count < d->transition_capacity) return;

    int new_capacity = d->transition_capacity * 2;
    dfa_transition *new_transitions =
        realloc(d->transitions, sizeof(dfa_transition) * new_capacity);

    if (new_transitions == NULL)
    {
        fprintf(stderr, "Error fatal: no se pudo redimensionar el arreglo de transiciones.\n");
        exit(EXIT_FAILURE);
    }

    d->transitions = new_transitions;
    d->transition_capacity = new_capacity;
}

/* ------------------------------------------------------------------ */
/* Algoritmos públicos                                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Algoritmo 1 de la Práctica 2: Move(T, a).
 *
 * Recorre todos los estados en T y acumula los destinos de las transiciones
 * que consumen exactamente el símbolo 'a'. Ignora transiciones epsilon.
 *
 * Pseudocódigo de referencia:
 *   R <- vacío
 *   for all s in T:
 *       R <- R U delta_N(s, a)
 *   return R
 */
bool* move_operation(nfa n, bool *T, char a)
{
    bool *R = calloc(n.state_count, sizeof(bool));
    if (R == NULL)
    {
        fprintf(stderr, "Error fatal: no hay memoria para el conjunto resultado.\n");
        exit(EXIT_FAILURE);
    }

    for (int s = 0; s < n.state_count; s++)
    {
        if (!T[s]) continue;

        for (int i = 0; i < n.transition_count; i++)
        {
            transition t = n.transitions[i];
            if (!t.epsilon && t.from == s && t.symbol == a)
            {
                R[t.to] = true;
            }
        }
    }

    return R;
}

/**
 * @brief Algoritmo 3 de la Práctica 2: Construcción de Subconjuntos.
 *
 * Convierte un NFA en un DFA equivalente. Explora todos los subconjuntos
 * alcanzables desde el estado inicial. Los subconjuntos que contienen al
 * estado de aceptación del NFA se marcan como aceptación en el DFA.
 *
 * El DFA resultante no contiene estados inalcanzables por construcción.
 */
dfa nfa_to_dfa(nfa n)
{
    dfa result;

    /* Capacidades iniciales pequeñas; el crecimiento es dinámico. */
    result.state_capacity = 16;
    result.states = malloc(sizeof(dfa_state) * result.state_capacity);
    result.state_count = 0;

    result.transition_capacity = 32;
    result.transitions = malloc(sizeof(dfa_transition) * result.transition_capacity);
    result.transition_count = 0;

    /* Validación de memoria inicial */
    if (result.states == NULL || result.transitions == NULL)
    {
        fprintf(stderr, "Error fatal: no se pudo reservar memoria inicial para el DFA.\n");
        exit(EXIT_FAILURE);
    }

    /* Obtener el alfabeto del NFA */
    char alphabet[256];
    int alpha_size = 0;
    get_alphabet(n, alphabet, &alpha_size);

    /* s0 = e-closure({q0}) */
    bool *s0 = calloc(n.state_count, sizeof(bool));
    if (s0 == NULL)
    {
        fprintf(stderr, "Error fatal: no se pudo reservar s0.\n");
        exit(EXIT_FAILURE);
    }
    s0[n.start] = true;
    epsilon_closure(n, s0);

    /* Q_D <- {s0} */
    result.states[0].id = 0;
    result.states[0].nfa_states = s0;
    result.states[0].is_accept = false;
    result.state_count++;
    result.start = 0;

    /* Cola implícita: índice unprocessed sobre result.states */
    int unprocessed = 0;

    while (unprocessed < result.state_count)
    {
        int U_id = unprocessed;
        bool *U = result.states[U_id].nfa_states;
        unprocessed++;

        for (int i = 0; i < alpha_size; i++)
        {
            char a = alphabet[i];

            /* V <- e-closure(Move(U, a)) */
            bool *moved = move_operation(n, U, a);
            epsilon_closure(n, moved);

            if (is_empty_set(moved, n.state_count))
            {
                free(moved);
                continue;
            }

            int V_id = find_state_in_QD(&result, moved, n.state_count);

            if (V_id == -1)
            {
                /* Estado nuevo: agregar a Q_D */
                ensure_state_capacity(&result);

                V_id = result.state_count;
                result.states[V_id].id = V_id;
                result.states[V_id].nfa_states = moved;
                result.states[V_id].is_accept = false;
                result.state_count++;
            }
            else
            {
                /* Ya existía: liberar la copia temporal */
                free(moved);
            }

            /* delta_D(U, a) <- V */
            ensure_transition_capacity(&result);
            result.transitions[result.transition_count].from = U_id;
            result.transitions[result.transition_count].to = V_id;
            result.transitions[result.transition_count].symbol = a;
            result.transition_count++;
        }
    }

    /* Marcar estados de aceptación */
    for (int i = 0; i < result.state_count; i++)
    {
        if (result.states[i].nfa_states[n.accept])
        {
            result.states[i].is_accept = true;
        }
    }

    return result;
}

/**
 * @brief Imprime la tabla de transiciones y los metadatos del DFA.
 */
void print_dfa_table(dfa d)
{
    printf("\n========= TABLA DE TRANSICIONES DEL DFA =========\n");
    printf("Total de estados:     %d\n", d.state_count);
    printf("Estado inicial:       q%d\n", d.start);
    printf("Estados de aceptacion: ");
    for (int i = 0; i < d.state_count; i++)
    {
        if (d.states[i].is_accept) printf("q%d ", d.states[i].id);
    }
    printf("\n\nTransiciones:\n");

    for (int i = 0; i < d.transition_count; i++)
    {
        dfa_transition t = d.transitions[i];
        printf("q%d -- %c --> q%d\n", t.from, t.symbol, t.to);
    }
    printf("=================================================\n");
}

/**
 * @brief Libera la memoria del DFA. Segura ante punteros NULL.
 */
void free_dfa(dfa *d)
{
    if (d == NULL) return;

    if (d->states != NULL)
    {
        for (int i = 0; i < d->state_count; i++)
        {
            free(d->states[i].nfa_states);
            d->states[i].nfa_states = NULL;
        }
        free(d->states);
    }

    free(d->transitions);

    d->states = NULL;
    d->transitions = NULL;
    d->state_count = 0;
    d->transition_count = 0;
    d->state_capacity = 0;
    d->transition_capacity = 0;
}