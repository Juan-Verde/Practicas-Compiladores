/**
 * @file nfa.c
 * @brief Implementación del NFA (Autómata Finito No Determinista).
 *
 * Contiene:
 *   - Construcción de Thompson (regex postfija -> NFA).
 *   - Cálculo de ε-closure (Algoritmo 2 de la Práctica 2).
 *   - Simulación del NFA sobre cadenas de entrada.
 *   - Serialización del NFA a formato JSON.
 *
 * Robustez:
 *   - `add_transition` verifica overflow de int antes de duplicar capacidad.
 *   - Todas las reservas de memoria se verifican (malloc/realloc).
 */

#include "nfa.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/**
 * @struct fragment
 * @brief Fragmento parcial de Thompson.
 *
 * Cada fragmento representa un sub-NFA con un estado inicial y un estado
 * de aceptación. Los fragmentos se combinan mediante los operadores de la
 * expresión regular durante la construcción del NFA completo.
 */
typedef struct
{
    int start;   /**< Estado inicial del fragmento. */
    int accept;  /**< Estado de aceptación del fragmento. */
} fragment;

/* ------------------------------------------------------------------ */
/* Utilidades de construcción                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Crea un nuevo estado en el NFA y devuelve su índice.
 *
 * Los estados se numeran secuencialmente: q0, q1, q2, ...
 *
 * @param n NFA al que se agrega el estado.
 * @return Índice del estado recién creado.
 */
static int new_state(nfa *n)
{
    int state = n->state_count;
    n->state_count++;
    return state;
}

/**
 * @brief Agrega una transición al NFA, redimensionando si es necesario.
 *
 * ROBUSTEZ:
 *   - Verifica overflow de int al duplicar la capacidad.
 *   - Verifica que realloc no falle.
 *
 * @param n       NFA destino.
 * @param from    Estado origen.
 * @param to      Estado destino.
 * @param symbol  Símbolo consumido (ignorado si epsilon == true).
 * @param epsilon true si la transición es vacía (ε).
 */
static void add_transition(
    nfa *n,
    int from,
    int to,
    char symbol,
    bool epsilon
)
{
    if (n->transition_count >= n->transition_capacity)
    {
        int new_capacity;

        if (n->transition_capacity == 0)
        {
            new_capacity = 16;
        }
        else
        {
            /* Verificación contra overflow de int. */
            if (n->transition_capacity > (INT_MAX / (2 * (int)sizeof(transition))))
            {
                fprintf(stderr,
                        "Error fatal: el NFA excede el limite de memoria.\n");
                exit(EXIT_FAILURE);
            }
            new_capacity = n->transition_capacity * 2;
        }

        transition *new_array = realloc(
            n->transitions,
            sizeof(transition) * new_capacity
        );

        if (new_array == NULL)
        {
            fprintf(stderr,
                    "Error fatal: no hay memoria para nuevas transiciones.\n");
            exit(EXIT_FAILURE);
        }

        n->transitions = new_array;
        n->transition_capacity = new_capacity;
    }

    transition t;
    t.from = from;
    t.to = to;
    t.symbol = symbol;
    t.epsilon = epsilon;

    n->transitions[n->transition_count] = t;
    n->transition_count++;
}

/* ------------------------------------------------------------------ */
/* Fragmentos de Thompson                                              */
/* ------------------------------------------------------------------ */

/**
 * @brief Crea el fragmento para un único símbolo del alfabeto.
 *
 *   (q_start) --symbol--> ((q_accept))
 *
 * @param n      NFA destino.
 * @param symbol Símbolo del alfabeto.
 * @return Fragmento con start y accept recién creados.
 */
static fragment symbol_fragment(nfa *n, char symbol)
{
    fragment f;
    f.start  = new_state(n);
    f.accept = new_state(n);

    add_transition(n, f.start, f.accept, symbol, false);
    return f;
}

/**
 * @brief Concatena dos fragmentos: A . B
 *
 * Se agrega una transición ε desde el estado de aceptación de A hasta el
 * estado inicial de B. El fragmento resultante hereda el start de A y el
 * accept de B.
 *
 * @param n NFA destino.
 * @param a Fragmento A.
 * @param b Fragmento B.
 * @return Fragmento concatenado.
 */
static fragment concatenate_fragments(nfa *n, fragment a, fragment b)
{
    add_transition(n, a.accept, b.start, 0, true);

    fragment result;
    result.start  = a.start;
    result.accept = b.accept;
    return result;
}

/**
 * @brief Une dos fragmentos: A | B
 *
 *   - Nuevo estado inicial con ε hacia start(A) y start(B).
 *   - Nuevo estado final con ε desde accept(A) y accept(B).
 *
 * @param n NFA destino.
 * @param a Fragmento A.
 * @param b Fragmento B.
 * @return Fragmento de unión.
 */
static fragment union_fragments(nfa *n, fragment a, fragment b)
{
    int start  = new_state(n);
    int accept = new_state(n);

    add_transition(n, start, a.start, 0, true);
    add_transition(n, start, b.start, 0, true);
    add_transition(n, a.accept, accept, 0, true);
    add_transition(n, b.accept, accept, 0, true);

    fragment result;
    result.start  = start;
    result.accept = accept;
    return result;
}

/**
 * @brief Aplica la estrella de Kleene a un fragmento: A*
 *
 *   - Nuevo start con ε a start(A) y a accept(A).
 *   - Nuevo accept con ε desde accept(A).
 *   - ε desde accept(A) a start(A) para permitir repetición.
 *
 * @param n NFA destino.
 * @param a Fragmento A.
 * @return Fragmento de Kleene.
 */
static fragment star_fragment(nfa *n, fragment a)
{
    int start  = new_state(n);
    int accept = new_state(n);

    add_transition(n, start, a.start, 0, true);
    add_transition(n, start, accept, 0, true);
    add_transition(n, a.accept, a.start, 0, true);
    add_transition(n, a.accept, accept, 0, true);

    fragment result;
    result.start  = start;
    result.accept = accept;
    return result;
}

/**
 * @brief Aplica el operador '+' a un fragmento: A+
 *
 *   - Nuevo start con ε a start(A).
 *   - ε desde accept(A) a start(A) para repetición.
 *   - ε desde accept(A) a accept (salida).
 *
 * @param n NFA destino.
 * @param a Fragmento A.
 * @return Fragmento A+.
 */
static fragment plus_fragment(nfa *n, fragment a)
{
    int start  = new_state(n);
    int accept = new_state(n);

    add_transition(n, start, a.start, 0, true);
    add_transition(n, a.accept, a.start, 0, true);
    add_transition(n, a.accept, accept, 0, true);

    fragment result;
    result.start  = start;
    result.accept = accept;
    return result;
}

/**
 * @brief Aplica el operador '?' a un fragmento: A?
 *
 *   - Nuevo start con ε a start(A) y a accept.
 *   - ε desde accept(A) a accept.
 *
 * @param n NFA destino.
 * @param a Fragmento A.
 * @return Fragmento A?.
 */
static fragment question_fragment(nfa *n, fragment a)
{
    int start  = new_state(n);
    int accept = new_state(n);

    add_transition(n, start, a.start, 0, true);
    add_transition(n, start, accept, 0, true);
    add_transition(n, a.accept, accept, 0, true);

    fragment result;
    result.start  = start;
    result.accept = accept;
    return result;
}

/* ------------------------------------------------------------------ */
/* Algoritmo 2 de la Práctica 2: ε-Closure                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Calcula la ε-clausura de un conjunto de estados (in-place).
 *
 * Implementa el Algoritmo 2 del manual:
 *   Pila <- inicializar con todos los elementos de T
 *   C <- T
 *   while Pila != vacío:
 *       t <- desapilar(Pila)
 *       for all u in δ_N(t, ε):
 *           if u no está en C:
 *               C <- C U {u}
 *               apilar(Pila, u)
 *   return C
 *
 * El parámetro `states` actúa simultáneamente como entrada T y salida C.
 *
 * ROBUSTEZ:
 *   - Se apilan los estados iniciales solo una vez (no hay duplicados porque
 *     se verifica `states[t.to]` antes de apilar).
 *   - Los ciclos (ej. 2 -> 1 -> 2) se rompen porque el conjunto C crece
 *     monótonamente y un estado solo se apila la primera vez que se descubre.
 *
 * @param n      NFA de entrada.
 * @param states Arreglo booleano (size = n.state_count) con los estados
 *               iniciales en true. Al terminar contiene la ε-clausura.
 */
void epsilon_closure(nfa n, bool *states)
{
    int *stack = malloc(sizeof(int) * n.state_count);

    if (stack == NULL)
    {
        fprintf(stderr, "Error fatal: no hay memoria para la pila de ε-closure.\n");
        exit(EXIT_FAILURE);
    }

    int top = -1;

    /* Inicializar la pila con los estados de T. */
    for (int i = 0; i < n.state_count; i++)
    {
        if (states[i])
        {
            stack[++top] = i;
        }
    }

    /* Explorar transiciones ε desde los estados en la pila. */
    while (top >= 0)
    {
        int current = stack[top--];

        for (int i = 0; i < n.transition_count; i++)
        {
            transition t = n.transitions[i];

            if (t.epsilon && t.from == current)
            {
                if (!states[t.to])
                {
                    states[t.to] = true;
                    stack[++top] = t.to;
                }
            }
        }
    }

    free(stack);
}

/* ------------------------------------------------------------------ */
/* Construcción de Thompson                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Construye un NFA a partir de una regex en notación postfija.
 *
 * Implementa el algoritmo de Thompson usando una pila de fragmentos:
 *   - Operando:   se apila un fragmento para ese símbolo.
 *   - '.' (conc):  se desapilan dos fragmentos, se concatenan y se apila el resultado.
 *   - '|' (unión): se desapilan dos, se unen y se apila el resultado.
 *   - '*', '+', '?': se desapila uno, se aplica el operador y se apila.
 *
 * Al final debe quedar exactamente un fragmento en la pila, cuyos estados
 * start y accept se copian al NFA resultante.
 *
 * @param r Regex en notación postfija.
 * @return NFA construido.
 */
nfa regex_to_nfa(regex r)
{
    nfa result;

    result.start  = -1;
    result.accept = -1;
    result.state_count = 0;
    result.transitions = NULL;
    result.transition_count = 0;
    result.transition_capacity = 0;

    fragment stack[MAX_REGEX_SIZE];
    int top = -1;

    for (int i = 0; i < r.size; i++)
    {
        char token = r.items[i].value;

        /* Concatenación. */
        if (token == '.')
        {
            fragment b = stack[top--];
            fragment a = stack[top--];
            stack[++top] = concatenate_fragments(&result, a, b);
        }
        /* Unión. */
        else if (token == '|')
        {
            fragment b = stack[top--];
            fragment a = stack[top--];
            stack[++top] = union_fragments(&result, a, b);
        }
        /* Kleene. */
        else if (token == '*')
        {
            fragment a = stack[top--];
            stack[++top] = star_fragment(&result, a);
        }
        /* '+' */
        else if (token == '+')
        {
            fragment a = stack[top--];
            stack[++top] = plus_fragment(&result, a);
        }
        /* '?' */
        else if (token == '?')
        {
            fragment a = stack[top--];
            stack[++top] = question_fragment(&result, a);
        }
        /* Operando. */
        else
        {
            stack[++top] = symbol_fragment(&result, token);
        }
    }

    /* Al final debe quedar exactamente un fragmento. */
    if (top == 0)
    {
        result.start  = stack[0].start;
        result.accept = stack[0].accept;
    }

    return result;
}

/**
 * @brief Libera la memoria del NFA. Segura ante punteros NULL.
 */
void free_nfa(nfa *n)
{
    if (n == NULL) return;

    free(n->transitions);
    n->transitions = NULL;
    n->transition_count = 0;
    n->transition_capacity = 0;
    n->state_count = 0;
    n->start  = -1;
    n->accept = -1;
}

/* ------------------------------------------------------------------ */
/* Simulación del NFA                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Determina si una cadena es aceptada por el NFA.
 *
 * Simula el NFA manteniendo el conjunto de estados alcanzables en cada paso:
 *   1. current = {n.start}, luego aplicar ε-closure.
 *   2. Para cada carácter c de la entrada:
 *        next = { δ(t, c) | t ∈ current }
 *        next = ε-closure(next)
 *        current = next
 *   3. Aceptar si el estado final del NFA está en current.
 *
 * @param n    NFA de entrada.
 * @param text Cadena a evaluar.
 * @param len  Longitud de la cadena.
 * @return 1 si es aceptada, 0 en caso contrario.
 */
int match_nfa(nfa n, const char *text, size_t len)
{
    bool *current = calloc(n.state_count, sizeof(bool));

    if (current == NULL)
    {
        fprintf(stderr, "Error fatal: no hay memoria para la simulación.\n");
        exit(EXIT_FAILURE);
    }

    /* Estado inicial + ε-closure. */
    current[n.start] = true;
    epsilon_closure(n, current);

    /* Procesar cada carácter. */
    for (size_t position = 0; position < len; position++)
    {
        char symbol = text[position];

        bool *next = calloc(n.state_count, sizeof(bool));
        if (next == NULL)
        {
            free(current);
            fprintf(stderr, "Error fatal: no hay memoria para la simulación.\n");
            exit(EXIT_FAILURE);
        }

        /* δ(t, c) para cada estado actual. */
        for (int i = 0; i < n.transition_count; i++)
        {
            transition t = n.transitions[i];

            if (!t.epsilon && t.symbol == symbol && current[t.from])
            {
                next[t.to] = true;
            }
        }

        /* Aplicar ε-closure después de consumir el símbolo. */
        epsilon_closure(n, next);

        free(current);
        current = next;
    }

    int accepted = current[n.accept] ? 1 : 0;
    free(current);
    return accepted;
}

/* ------------------------------------------------------------------ */
/* Impresión y serialización                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Imprime en consola la tabla de transiciones del NFA.
 *        Útil para contrastar visualmente con el DFA resultante.
 */
void print_nfa_table(nfa n)
{
    printf("\n=== TABLA DE TRANSICIONES DEL NFA (INICIAL) ===\n");

    for (int i = 0; i < n.transition_count; i++)
    {
        transition t = n.transitions[i];

        if (t.epsilon)
        {
            printf("q%d -- epsilon --> q%d\n", t.from, t.to);
        }
        else
        {
            printf("q%d -- %c --> q%d\n", t.from, t.symbol, t.to);
        }
    }

    printf("\nEstado inicial: q%d\n", n.start);
    printf("Estado de aceptacion: q%d\n", n.accept);
    printf("===============================================\n");
}

/**
 * @brief Serializa el NFA en un archivo con formato JSON.
 *
 * Esquema de salida:
 * {
 *   "state_count": int,
 *   "start": int,
 *   "accept": int,
 *   "transition_count": int,
 *   "transitions": [ { "from": int, "to": int, "symbol": int, "epsilon": bool }, ... ]
 * }
 *
 * @param n    NFA a serializar.
 * @param path Ruta del archivo de salida.
 * @return true si la escritura fue exitosa, false en caso contrario.
 */
bool save_nfa(const nfa *n, const char *path)
{
    if (n == NULL || path == NULL) return false;

    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        fprintf(stderr,
                "Error: No se pudo abrir el archivo '%s' para guardar el NFA.\n",
                path);
        return false;
    }

    fprintf(file, "{\n");
    fprintf(file, "  \"state_count\": %d,\n", n->state_count);
    fprintf(file, "  \"start\": %d,\n", n->start);
    fprintf(file, "  \"accept\": %d,\n", n->accept);
    fprintf(file, "  \"transition_count\": %d,\n", n->transition_count);

    fprintf(file, "  \"transitions\": [\n");

    for (int i = 0; i < n->transition_count; i++)
    {
        transition t = n->transitions[i];

        fprintf(file, "    {\n");
        fprintf(file, "      \"from\": %d,\n", t.from);
        fprintf(file, "      \"to\": %d,\n", t.to);
        fprintf(file, "      \"symbol\": %d,\n", (int)t.symbol);
        fprintf(file, "      \"epsilon\": %s\n", t.epsilon ? "true" : "false");
        fprintf(file, "    }%s\n",
                (i == n->transition_count - 1) ? "" : ",");
    }

    fprintf(file, "  ]\n");
    fprintf(file, "}\n");

    fclose(file);
    return true;
}