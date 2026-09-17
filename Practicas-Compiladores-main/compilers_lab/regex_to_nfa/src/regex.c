/**
 * @file regex.c
 * @brief Implementación del parser de expresiones regulares.
 *
 * Convierte una expresión regular en notación infija a notación postfija
 * utilizando el algoritmo de Shunting-Yard. Antes de aplicar Shunting-Yard,
 * se insertan concatenaciones explícitas ('.') entre operandos adyacentes.
 *
 * Ejemplo:
 *   Entrada:  "ab|c*"
 *   Paso 1:   "a.b|c*"   (concatenación explícita)
 *   Salida:   "ab.c*|"   (notación postfija)
 */

#include "regex.h"
#include <stdbool.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Clasificación de caracteres                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Determina si un carácter es un operando (símbolo del alfabeto).
 *
 * Son operandos todos los caracteres que no son operadores ni paréntesis.
 *
 * @param c Carácter a clasificar.
 * @return true si es operando, false si es operador o delimitador.
 */
static bool is_operand(char c)
{
    return c != '|' &&
           c != '*' &&
           c != '+' &&
           c != '?' &&
           c != '(' &&
           c != ')' &&
           c != '.';
}

/**
 * @brief Determina si un carácter puede aparecer al final de una subexpresión.
 *
 * Se usa para decidir cuándo hay una concatenación implícita. Ejemplos:
 *   "ab"   -> 'a' puede terminar y 'b' puede comenzar -> "a.b"
 *   "a|b"  -> 'a' puede terminar, pero '|' no comienza expresión -> no se inserta '.'
 *
 * @param c Carácter a evaluar.
 * @return true si c puede cerrar una expresión.
 */
static bool can_end_expression(char c)
{
    return is_operand(c) ||
           c == ')' ||
           c == '*' ||
           c == '+' ||
           c == '?';
}

/**
 * @brief Determina si un carácter puede aparecer al inicio de una subexpresión.
 *
 * @param c Carácter a evaluar.
 * @return true si c puede iniciar una expresión.
 */
static bool can_start_expression(char c)
{
    return is_operand(c) ||
           c == '(';
}

/* ------------------------------------------------------------------ */
/* Preprocesamiento: concatenación explícita                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Inserta el operador de concatenación explícito '.' donde sea necesario.
 *
 * Recorre la expresión de entrada y agrega '.' entre dos caracteres cuando:
 *   - El primero puede cerrar una expresión.
 *   - El segundo puede iniciar una expresión.
 *
 * Ejemplo:
 *   Entrada:  "ab|c*"
 *   Salida:   "a.b|c*"
 *
 * @param input  Cadena de entrada (infijo sin concatenaciones explícitas).
 * @param output Cadena de salida (infijo con concatenaciones explícitas).
 *
 * @note `output` debe tener al menos el doble de capacidad que `input`
 *       para soportar el peor caso (todos los caracteres requieren '.').
 *       En la práctica, MAX_REGEX_SIZE cubre este requisito.
 */
static void insert_concatenation(
    const char *input,
    char *output
)
{
    int j = 0;
    int length = strlen(input);

    for (int i = 0; i < length; i++)
    {
        char current = input[i];

        output[j++] = current;

        /* Si es el último carácter, ya no hay siguiente con quién concatenar. */
        if (i == length - 1)
        {
            continue;
        }

        char next = input[i + 1];

        /* Insertar '.' si hay concatenación implícita. */
        if (can_end_expression(current) &&
            can_start_expression(next))
        {
            output[j++] = '.';
        }
    }

    output[j] = '\0';
}

/* ------------------------------------------------------------------ */
/* Shunting-Yard: infijo -> postfijo                                   */
/* ------------------------------------------------------------------ */

/**
 * @brief Devuelve la precedencia de un operador.
 *
 * Mayor número = mayor precedencia.
 *   '|' -> 1 (unión)
 *   '.' -> 2 (concatenación)
 *   otro -> 0
 *
 * @param operator Carácter del operador.
 * @return Entero con la precedencia.
 */
static int precedence(char operator)
{
    switch (operator)
    {
        case '|':
            return 1;

        case '.':
            return 2;

        default:
            return 0;
    }
}

/**
 * @brief Convierte una expresión con concatenación explícita de infijo a postfijo.
 *
 * Implementa el algoritmo de Shunting-Yard (Dijkstra). Reglas:
 *   - Operando:               va directo a la salida.
 *   - '*', '+', '?':          operadores postfijos, van directo a la salida.
 *   - '(':                    se apila.
 *   - ')':                    se desapila hasta encontrar '('.
 *   - '|', '.':               se desapila mientras el tope tenga precedencia >=.
 *
 * @param input Cadena en notación infija con concatenaciones explícitas.
 * @return regex con los elementos reordenados en notación postfija.
 */
static regex to_postfix(const char *input)
{
    regex result;
    result.size = 0;

    /* Pila auxiliar del algoritmo de Shunting-Yard. */
    char stack[MAX_REGEX_SIZE];
    int top = -1;

    int length = strlen(input);

    for (int i = 0; i < length; i++)
    {
        char c = input[i];

        /* Caso 1: operando -> va directo a la salida. */
        if (is_operand(c))
        {
            result.items[result.size++].value = c;
        }

        /* Caso 2: operadores postfijos -> van directo a la salida. */
        else if (c == '*' || c == '+' || c == '?')
        {
            result.items[result.size++].value = c;
        }

        /* Caso 3: paréntesis izquierdo -> se apila. */
        else if (c == '(')
        {
            stack[++top] = c;
        }

        /* Caso 4: paréntesis derecho -> desapilar hasta el '('. */
        else if (c == ')')
        {
            while (top >= 0 && stack[top] != '(')
            {
                result.items[result.size++].value = stack[top--];
            }

            /* Eliminar el '(' de la pila. */
            if (top >= 0 && stack[top] == '(')
            {
                top--;
            }
        }

        /* Caso 5: operadores binarios ('|' o '.'). */
        else if (c == '|' || c == '.')
        {
            while (
                top >= 0 &&
                stack[top] != '(' &&
                precedence(stack[top]) >= precedence(c)
            )
            {
                result.items[result.size++].value = stack[top--];
            }

            stack[++top] = c;
        }
    }

    /* Vaciar la pila al final. */
    while (top >= 0)
    {
        if (stack[top] != '(')
        {
            result.items[result.size++].value = stack[top];
        }
        top--;
    }

    return result;
}

/* ------------------------------------------------------------------ */
/* API pública                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Convierte una expresión regular en notación infija a postfija.
 *
 * Pipeline:
 *   1. insert_concatenation(): agrega '.' entre operandos adyacentes.
 *   2. to_postfix(): aplica Shunting-Yard sobre el resultado.
 *
 * @param input Cadena con la expresión regular en notación infija.
 * @return regex con los elementos en notación postfija, listos para Thompson.
 */
regex parse_regex(const char *input)
{
    char explicit_regex[MAX_REGEX_SIZE];

    /* Paso 1: hacer explícitas las concatenaciones. */
    insert_concatenation(input, explicit_regex);

    /* Paso 2: convertir a postfijo. */
    return to_postfix(explicit_regex);
}