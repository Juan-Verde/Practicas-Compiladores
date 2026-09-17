#ifndef REGEX_H
#define REGEX_H

/**
 * @file regex.h
 * @brief Definición de la estructura `regex` y la función de parsing
 *        (infijo -> postfijo) mediante el algoritmo de Shunting-Yard.
 */

/**
 * @brief Tamaño máximo de la expresión regular (en caracteres).
 */
#define MAX_REGEX_SIZE 2048

/**
 * @struct regex_item
 * @brief Representa un elemento individual de la expresión regular.
 *
 * Por ahora solo almacena el carácter (operador u operando), pero el struct
 * está preparado para crecer si se requieren atributos adicionales.
 */
typedef struct
{
    char value;   /**< Carácter de la regex (operador u operando). */
} regex_item;

/**
 * @struct regex
 * @brief Representa una expresión regular ya procesada.
 *
 * `items` contiene la lista de elementos en el orden en que aparecen.
 * Tras llamar a parse_regex(), `items` estará en notación postfija.
 */
typedef struct
{
    regex_item items[MAX_REGEX_SIZE];  /**< Arreglo de elementos. */
    int size;                          /**< Número de elementos válidos. */
} regex;

/**
 * @brief Convierte una expresión regular en notación infija a postfija.
 *
 * Pasos:
 *   1. Inserta concatenaciones explícitas ('a' seguido de 'b' -> "a.b").
 *   2. Aplica el algoritmo de Shunting-Yard.
 *
 * @param input Cadena con la expresión regular en notación infija.
 * @return regex con los elementos en notación postfija.
 *
 * @note No valida sintaxis: se asume que la entrada está bien formada.
 *       Si se ingresa una expresión inválida, el comportamiento es indefinido.
 */
regex parse_regex(const char *input);

#endif