#include "regex.h"
#include <stdbool.h>
#include <string.h>

/*
 * Determina si un carácter es un símbolo normal
 * de la expresión regular y no un operador.
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

/*
 * Determinamos si un carácter puede estar al final de una expresión.
 */
static bool can_end_expression(char c)
{
    return is_operand(c) ||
           c == ')' ||
           c == '*' || 
           c == '+' ||
           c == '?';
}

/*
 * Determina si un carácter puede comenzar una expresión.
 */
static bool can_start_expression(char c)
{
    return is_operand(c) ||
           c == '(';
}

/*
 * Convertimos las concatenaciones implícitas en explícitas.
 *
 * Ejemplo:
 *
 *      ab
 *
 * se convierte en:
 *
 *      a.b
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

        /*
         * Si estamos en el último carácter,
         * ya no existe un siguiente elemento.
         */
        if (i == length - 1)
        {
            continue;
        }

        char next = input[i + 1];

        /*
         * Si el actual puede terminar una expresión
         * y el siguiente puede comenzar otra,
         * existe una concatenación implícita.
         */
        if (can_end_expression(current) &&
            can_start_expression(next))
        {
            output[j++] = '.';
        }
    }

    /*
     * Terminamos la cadena.
     */
    output[j] = '\0';
}

/*
 * Definimos las precedencias
 * Devuelve la precedencia de los operadores.
 *
 * Concatenación tiene mayor precedencia que unión.
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

/*
 * Convertimos una regex con concatenación explícita
 * de notación infija a notación postfija.
 */
static regex to_postfix(const char *input)
{
    regex result;

    result.size = 0;

    /*
     * esta es la pila que utiliza Shunting Yard.
     */
    char stack[MAX_REGEX_SIZE];

    int top = -1;

    int length = strlen(input);


    for (int i = 0; i < length; i++)
    {
        char c = input[i];

        /*
         * CASO 1:
         * Es un símbolo normal.
         *
         * lo mandamos directamente a la salida. (en clase teoria lo vimos como lista pero se entiende)
         */
        if (is_operand(c))
        {
            result.items[result.size++].value = c;
        }

        /*
         * CASO 2:
         * pa la estrella de Kleene.
         *
         * Como es un operador postfijo,
         * se manda directamente a la salida.
         */
        else if (c == '*' || c == '+' || c == '?')
        {
            result.items[result.size++].value = c;
        }


        /*
         * CASO 3:
         * Paréntesis izquierdo.
         *
         * Se mete a la pila.
         */
        else if (c == '(')
        {
            stack[++top] = c;
        }


        /*
         * CASO 4:
         * Paréntesis derecho.
         *
         * Sacamos los operadores hasta encontrar otro '('.
         */
        else if (c == ')')
        {
            while (top >= 0 && stack[top] != '(')
            {
                result.items[result.size++].value =
                    stack[top--];
            }

            /*
             * Eliminamos el '('.
             */
            if (top >= 0 && stack[top] == '(')
            {
                top--;
            }
        }


        /*
         * CASO 5:
         * Unión o concatenación.
         */
        else if (c == '|' || c == '.')
        {
            /*
             * Mientras haya un operador de igual
             * o mayor precedencia, lo sacamos.
             */
            while (
                top >= 0 &&
                stack[top] != '(' &&
                precedence(stack[top]) >= precedence(c)
            )
            {
                result.items[result.size++].value =
                    stack[top--];
            }

            /*
             * Metemos el operador actual.
             */
            stack[++top] = c;
        }
    }


    /*
     * Al terminar, vaciamos la pila.
     */
    while (top >= 0)
    {
        if (stack[top] != '(')
        {
            result.items[result.size++].value =
                stack[top];
        }

        top--;
    }


    return result;
}

// el parse regex aqui
regex parse_regex(const char *input)
{
    char explicit_regex[MAX_REGEX_SIZE];

    /*
     * Paso 1:
     * Hacemos explícitas las concatenaciones.
     */
    insert_concatenation(
        input,
        explicit_regex
    );

    /*
     * Paso 2:
     * Convertir la expresión a postfijo.
     */
    return to_postfix(explicit_regex);
}