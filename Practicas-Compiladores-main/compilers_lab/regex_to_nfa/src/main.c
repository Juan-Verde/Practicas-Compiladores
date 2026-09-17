/**
 * @file main.c
 * @brief Punto de entrada del programa regex_to_nfa.
 *
 * Este programa recibe una expresión regular por entrada estándar (stdin)
 * y ofrece tres modos de operación mediante banderas de línea de comandos:
 *
 *   -r          Imprime la expresión regular en notación postfija.
 *   -t          Lee cadenas por stdin y reporta si son aceptadas por el NFA.
 *   -o <ruta>   Serializa el NFA resultante en un archivo (formato JSON).
 *
 * Ejemplos de uso:
 *   echo "a(b|c)*" | ./regex_to_nfa -r
 *   echo "ab*"     | ./regex_to_nfa -t <<< $'ab\nabb\nac'
 *   echo "(a|b)*"  | ./regex_to_nfa -o salida.nfa
 *
 * Arquitectura:
 *   main.c orquesta las llamadas a:
 *     - parse_regex()   (regex.c)   : infijo -> postfijo (Shunting-Yard)
 *     - regex_to_nfa()  (nfa.c)     : postfijo -> NFA (Thompson)
 *     - match_nfa()     (nfa.c)     : simulación del NFA
 *     - save_nfa()      (nfa.c)     : serialización a JSON
 */

#include "regex.h"
#include "nfa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

/* Tamaño máximo del buffer para leer la regex desde stdin. */
#define MAX_INPUT_REGEX 1024

/* Tamaño máximo del buffer para leer cadenas de prueba en modo -t. */
#define MAX_TEST_STRING 1024

/* ------------------------------------------------------------------ */
/* Utilidades de impresión                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Imprime una expresión regular en notación postfija.
 *
 * Recorre el arreglo `r.items` y escribe cada carácter consecutivamente,
 * sin separadores, seguido de un salto de línea.
 *
 * @param r Regex ya procesada por parse_regex().
 */
void print_postfix(regex r)
{
    for (int i = 0; i < r.size; i++)
    {
        printf("%c", r.items[i].value);
    }
    printf("\n");
}

/* ------------------------------------------------------------------ */
/* Modos de operación                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Modo -t: prueba cadenas contra el NFA.
 *
 * Construye el NFA a partir de la regex dada y luego lee cadenas de stdin
 * (una por línea) hasta EOF. Para cada línea imprime "1" si la cadena es
 * aceptada por el NFA y "0" en caso contrario. Al final imprime un salto
 * de línea.
 *
 * @param regex_str Expresión regular en notación infija.
 *
 * @note Las cadenas se leen con fgets, por lo que se eliminan los
 *       caracteres de fin de línea ('\r' y '\n') antes de simular.
 */
void test_strings_stdin(const char *regex_str)
{
    regex r = parse_regex(regex_str);
    nfa n = regex_to_nfa(r);

    char buf[MAX_TEST_STRING];

    while (fgets(buf, sizeof(buf), stdin))
    {
        /* Eliminar '\r' y '\n' del final de la línea. */
        buf[strcspn(buf, "\r\n")] = '\0';

        int result = match_nfa(n, buf, strlen(buf));
        printf("%d", result ? 1 : 0);
    }
    printf("\n");

    free_nfa(&n);
}

/**
 * @brief Modo -o: serializa el NFA construido a partir de una regex.
 *
 * @param regex_str   Expresión regular en notación infija.
 * @param output_path Ruta del archivo JSON de salida.
 * @return 0 si la serialización fue exitosa, 1 en caso contrario.
 */
int serialize_nfa_from_regex(const char *regex_str, const char *output_path)
{
    regex r = parse_regex(regex_str);
    nfa n = regex_to_nfa(r);

    bool ok = save_nfa(&n, output_path);
    free_nfa(&n);

    if (!ok)
    {
        fprintf(stderr,
                "Error: No se pudo serializar el NFA en '%s'.\n",
                output_path);
        return 1;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* Punto de entrada                                                    */
/* ------------------------------------------------------------------ */

/**
 * @brief Programa principal. Parsea los argumentos y despacha al modo elegido.
 *
 * Opciones soportadas (getopt):
 *   -r           Modo postfijo.
 *   -t           Modo de prueba de cadenas.
 *   -o <ruta>    Modo de serialización.
 *
 * La expresión regular se lee de stdin (una sola línea). Si está vacía
 * se reporta un error y se termina la ejecución.
 */
int main(int argc, char *argv[])
{
    int opt;
    char regex_str[MAX_INPUT_REGEX];
    char *output_file = NULL;
    int mode = 0;

    /* -------------------------------------------------------------- */
    /* 1. Parseo de argumentos de línea de comandos                   */
    /* -------------------------------------------------------------- */
    while ((opt = getopt(argc, argv, "rto:")) != -1)
    {
        switch (opt)
        {
            case 'r':
                if (mode != 0)
                {
                    fprintf(stderr,
                            "Error: Solo puedes usar una opcion de modo entre -r, -t o -o.\n");
                    return 1;
                }
                mode = 'r';
                break;

            case 't':
                if (mode != 0)
                {
                    fprintf(stderr,
                            "Error: Solo puedes usar una opcion de modo entre -r, -t o -o.\n");
                    return 1;
                }
                mode = 't';
                break;

            case 'o':
                if (mode != 0)
                {
                    fprintf(stderr,
                            "Error: Solo puedes usar una opcion de modo entre -r, -t o -o.\n");
                    return 1;
                }
                mode = 'o';
                output_file = optarg;
                break;

            default:
                fprintf(stderr, "Usage: %s -r | -t | -o <archivo.nfa>\n", argv[0]);
                return 1;
        }
    }

    /* Validar que se haya elegido exactamente un modo. */
    if (mode == 0)
    {
        fprintf(stderr, "Usage: %s -r | -t | -o <archivo.nfa>\n", argv[0]);
        return 1;
    }

    /* -------------------------------------------------------------- */
    /* 2. Lectura de la expresión regular desde stdin                 */
    /* -------------------------------------------------------------- */
    if (!fgets(regex_str, sizeof(regex_str), stdin))
    {
        fprintf(stderr, "Error: No se pudo leer la expresion regular desde stdin.\n");
        return 1;
    }

    /* Eliminar '\r' y '\n' del final. */
    regex_str[strcspn(regex_str, "\r\n")] = '\0';

    /* Validación: la regex no puede estar vacía. */
    if (strlen(regex_str) == 0)
    {
        fprintf(stderr, "Error: La expresion regular esta vacia.\n");
        return 1;
    }

    /* -------------------------------------------------------------- */
    /* 3. Despacho al modo seleccionado                               */
    /* -------------------------------------------------------------- */
    if (mode == 'r')
    {
        print_postfix(parse_regex(regex_str));
        return 0;
    }

    if (mode == 't')
    {
        test_strings_stdin(regex_str);
        return 0;
    }

    /* mode == 'o' */
    return serialize_nfa_from_regex(regex_str, output_file);
}