#ifndef REGEX_H
#define REGEX_H

#define MAX_REGEX_SIZE 2048

/*
 * Representa un elemento de la expresión regular.
 * Por ahora solo necesitamos guardar el carácter.
 */
typedef struct
{
    char value;
} regex_item;


/*
 * Representa una expresión regular ya procesada.
 *
 * items contiene los elementos de la expresión.
 * size indica cuántos elementos hay.
 */
typedef struct
{
    regex_item items[MAX_REGEX_SIZE];
    int size;
} regex;


/*
 * Recibe una expresión regular en notación infija
 * y la convierte a notación postfija.
 */
regex parse_regex(const char *input);

#endif