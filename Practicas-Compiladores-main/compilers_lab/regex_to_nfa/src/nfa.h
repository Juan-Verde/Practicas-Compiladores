#ifndef NFA_H
#define NFA_H

#include "regex.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @file nfa.h
 * @brief Definición del NFA (Autómata Finito No Determinista) y sus operaciones.
 *
 * Incluye:
 *   - Estructuras `transition` y `nfa`.
 *   - Construcción de Thompson (regex -> NFA).
 *   - ε-closure (Algoritmo 2 de la Práctica 2).
 *   - Simulación del NFA sobre cadenas.
 *   - Serialización a JSON.
 */

/**
 * @struct transition
 * @brief Representa una transición del NFA.
 *
 * Puede ser una transición normal (consume un símbolo del alfabeto) o una
 * transición ε (no consume símbolo, indicada por `epsilon == true`).
 */
typedef struct
{
    int from;       /**< Estado origen. */
    int to;         /**< Estado destino. */
    char symbol;    /**< Símbolo consumido (ignorado si epsilon == true). */
    bool epsilon;   /**< true si es una transición vacía (ε). */
} transition;

/**
 * @struct nfa
 * @brief Representa el autómata finito no determinista completo.
 *
 * Los estados se numeran 0..state_count-1. El arreglo de transiciones
 * se redimensiona dinámicamente al duplicar su capacidad cuando se llena.
 */
typedef struct
{
    int start;                      /**< Estado inicial. */
    int accept;                     /**< Estado de aceptación. */
    int state_count;                /**< Número total de estados. */
    transition *transitions;        /**< Arreglo dinámico de transiciones. */
    int transition_count;           /**< Número actual de transiciones. */
    int transition_capacity;        /**< Capacidad actual del arreglo. */
} nfa;

/**
 * @brief Construye un NFA a partir de una regex postfija (Algoritmo de Thompson).
 *
 * @param r Regex en notación postfija (salida de parse_regex).
 * @return NFA construido con su estado inicial y de aceptación.
 */
nfa regex_to_nfa(regex r);

/**
 * @brief Simula el NFA sobre una cadena y determina si es aceptada.
 *
 * @param n    NFA de entrada.
 * @param text Cadena a evaluar.
 * @param len  Longitud de la cadena.
 * @return 1 si la cadena es aceptada, 0 en caso contrario.
 */
int match_nfa(nfa n, const char *text, size_t len);

/**
 * @brief Libera la memoria asociada al NFA. Segura ante punteros NULL.
 */
void free_nfa(nfa *n);

/**
 * @brief Calcula la ε-clausura de un conjunto de estados (in-place).
 *
 * Implementa el Algoritmo 2 de la Práctica 2:
 *   Pila <- T
 *   C <- T
 *   while Pila != vacío:
 *       t <- desapilar
 *       for all u in δ(t, ε):
 *           if u ∉ C: C <- C ∪ {u}; apilar(u)
 *
 * @param n      NFA de entrada.
 * @param states Arreglo booleano (size = n.state_count).
 *               Entrada: contiene T. Salida: contiene ε-closure(T).
 */
void epsilon_closure(nfa n, bool *states);

/**
 * @brief Imprime en consola la tabla de transiciones del NFA.
 */
void print_nfa_table(nfa n);

/**
 * @brief Serializa el NFA en un archivo JSON.
 *
 * @param n    NFA a serializar.
 * @param path Ruta del archivo de salida.
 * @return true si la escritura fue exitosa, false en caso contrario.
 */
bool save_nfa(const nfa *n, const char *path);

#endif