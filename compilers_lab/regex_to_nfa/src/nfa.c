#include "nfa.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * Un fragmento temporal de Thompson representa un NFA parcial.
 *
 * Cada fragmento tiene un estado inicial para marcar el inicio y final del NFA,
 * y un estado de aceptación. Los fragmentos parciales se combinan mediante
 * los operadores de una regex durante la construcción del NFA
 */
typedef struct
{
    int start;
    int accept;

} fragment;

/*
 * Crea un nuevo estado para el NFA y devuelve el número que tiene.
 */
static int new_state(nfa *n)
{
    int state = n->state_count; 
    
    // para tener automaticamente q0 q1 q2...qn

    n->state_count++;

    return state;
}

// como no sabemos cuántas transiciones tendra el NFA, utilizamo malloc/realloc.

/*
 * Agrega una transición al NFA.
 *
 * Si el arreglo de transiciones se llena, se redimensiona antes de almacenar
 * una nueva transición
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
            new_capacity =
                n->transition_capacity * 2;
        }


        transition *new_array =
            realloc(
                n->transitions,
                sizeof(transition) * new_capacity
            );


        if (new_array == NULL)
        {
            fprintf(stderr, "Error de memoria.\n");
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

/*
 * Crea el fragmento correspondiente
 * a un único símbolo.
 */
static fragment symbol_fragment(
    nfa *n,
    char symbol
)
{
    fragment f;

    f.start = new_state(n);
    f.accept = new_state(n);


    add_transition(
        n,
        f.start,
        f.accept,
        symbol,
        false
    );


    return f;
}

// para concatenar las partes del NFA

/*
 * Concatena dos fragmentos.
 * A.B
 * 
 * Se agrega una transición epsilon desde el estado de aceptación
 * del fragmento A al estado inicial del fragmento B
 */
static fragment concatenate_fragments(
    nfa *n,
    fragment a,
    fragment b
)
{
    add_transition(
        n,
        a.accept,
        b.start,
        0,
        true
    );


    fragment result;

    result.start = a.start;
    result.accept = b.accept;


    return result;
}

/*
 * para la unión A|B.
 */
static fragment union_fragments(
    nfa *n,
    fragment a,
    fragment b
)
{
    int start = new_state(n);
    int accept = new_state(n);


    /*
     * Desde el nuevo inicio vamos ir
     * a A o B sin consumir caracteres.
     */
    add_transition(
        n,
        start,
        a.start,
        0,
        true
    );

    add_transition(
        n,
        start,
        b.start,
        0,
        true
    );


    /*
     * entonces anto A como B pueden llegar
     * al nuevo estado final.
     */
    add_transition(
        n,
        a.accept,
        accept,
        0,
        true
    );

    add_transition(
        n,
        b.accept,
        accept,
        0,
        true
    );


    fragment result;

    result.start = start;
    result.accept = accept;


    return result;
}

/*
 * para la estrella de Kleene.
 *
 * A*
 */
static fragment star_fragment(
    nfa *n,
    fragment a
)
{
    int start = new_state(n);
    int accept = new_state(n);


    /*
     * Podemos entrar a A.
     */
    add_transition(
        n,
        start,
        a.start,
        0,
        true
    );


    /*
     * O saltar directamente
     * al final: cero repeticiones.
     */
    add_transition(
        n,
        start,
        accept,
        0,
        true
    );


    /*
     * Al terminar A podemos repetir.
     */
    add_transition(
        n,
        a.accept,
        a.start,
        0,
        true
    );


    /*
     * O terminar.
     */
    add_transition(
        n,
        a.accept,
        accept,
        0,
        true
    );


    fragment result;

    result.start = start;
    result.accept = accept;


    return result;
}

/*
 * para el operador +
 *
 * A+
 */
static fragment plus_fragment(
    nfa *n,
    fragment a
)
{
    int start = new_state(n);
    int accept = new_state(n);


    /*
     * Debemos entrar obligatoriamente a A al menos una vez.
     */
    add_transition(
        n,
        start,
        a.start,
        0,
        true
    );


    /*
     * Al terminar A, podemos repetir
     * volviendo al inicio de A.
     */
    add_transition(
        n,
        a.accept,
        a.start,
        0,
        true
    );


    /*
     * O salir hacia el estado final.
     */
    add_transition(
        n,
        a.accept,
        accept,
        0,
        true
    );


    fragment result;

    result.start = start;
    result.accept = accept;


    return result;
}

/*
 * para el operador ?
 *
 * A?
 */
static fragment question_fragment(
    nfa *n,
    fragment a
)
{
    int start = new_state(n);
    int accept = new_state(n);


    /*
     * Podemos entrar a A.
     */
    add_transition(
        n,
        start,
        a.start,
        0,
        true
    );


    /*
     * O saltar directamente al final:
     * cero repeticiones permitidas.
     */
    add_transition(
        n,
        start,
        accept,
        0,
        true
    );


    /*
     * Al terminar A, vamos directamente al final.
     */
    add_transition(
        n,
        a.accept,
        accept,
        0,
        true
    );


    fragment result;

    result.start = start;
    result.accept = accept;


    return result;
}

/*
 * obtiene el epsilon-closure de un conjunto de estados.
 *
 * states[i] == true significa que el estado qi
 * pertenece actualmente al conjunto.
 */
static void epsilon_closure(
    nfa n,
    bool *states
)
{
    /*
     * Utilizamos una pila para recorrer los estados
     * alcanzables mediante transiciones epsilon.
     */
    int *stack =
        malloc(sizeof(int) * n.state_count);

    if (stack == NULL)
    {
        fprintf(stderr, "Error de memoria.\n");
        exit(EXIT_FAILURE);
    }


    int top = -1;


    /*
     * Inicialmente metemos a la pila todos
     * los estados que ya forman parte del conjunto.
     */
    for (int i = 0; i < n.state_count; i++)
    {
        if (states[i])
        {
            stack[++top] = i;
        }
    }


    /*
     * Recorremos todos los estados alcanzables
     * mediante transiciones epsilon.
     */
    while (top >= 0)
    {
        int current = stack[top--];


        for (int i = 0; i < n.transition_count; i++)
        {
            transition t = n.transitions[i];


            /*
             * Buscamos únicamente transiciones epsilon
             * que salgan del estado actual.
             */
            if (
                t.epsilon &&
                t.from == current
            )
            {
                /*
                 * Si todavía no habíamos visitado
                 * el destino, lo agregamos.
                 */
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

nfa regex_to_nfa(regex r)
{
    nfa result;

    result.start = -1;
    result.accept = -1;

    result.state_count = 0;

    result.transitions = NULL;
    result.transition_count = 0;
    result.transition_capacity = 0;


    /*
     * Pila donde albergamos los fragmentos usados por el
     * algoritmo de Thompson.
     * 
     * Cada operando de la expresión posfija genera un fragmento que
     * se apila. Cada operador extrae de la pila los fragmentos que
     * necesita, los combina y vuelve a colocar el resultado.
     */
    fragment stack[MAX_REGEX_SIZE];

    int top = -1;


    for (int i = 0; i < r.size; i++)
    {
        char token = r.items[i].value;


        /*
         * CONCATENACIÓN
         */
        if (token == '.')
        {
            fragment b = stack[top--];
            fragment a = stack[top--];

            fragment combined =
                concatenate_fragments(
                    &result,
                    a,
                    b
                );

            stack[++top] = combined;
        }


        /*
         * UNIÓN
         */
        else if (token == '|')
        {
            fragment b = stack[top--];
            fragment a = stack[top--];

            fragment combined =
                union_fragments(
                    &result,
                    a,
                    b
                );

            stack[++top] = combined;
        }


        /*
         * ESTRELLA DE KLEENE
         */
        else if (token == '*')
        {
            fragment a = stack[top--];

            fragment combined =
                star_fragment(
                    &result,
                    a
                );

            stack[++top] = combined;
        }

        /*
         * OPERADOR +
         */
        else if (token == '+')
        {
            fragment a = stack[top--];

            fragment combined =
                plus_fragment(
                    &result,
                    a
                );

            stack[++top] = combined;
        }

        /*
         * OPERADOR ?
         */
        else if (token == '?')
        {
            fragment a = stack[top--];

            fragment combined =
                question_fragment(
                    &result,
                    a
                );

            stack[++top] = combined;
        }

        /*
         * SÍMBOLO NORMAL
         */
        else
        {
            fragment f =
                symbol_fragment(
                    &result,
                    token
                );

            stack[++top] = f;
        }
    }


    /*
     * Al terminar debería existir un solo fragmento.
     */
    if (top == 0)
    {
        result.start = stack[0].start;
        result.accept = stack[0].accept;
    }


    return result;
}

void free_nfa(nfa *n)
{
    if (n == NULL)
    {
        return;
    }


    free(n->transitions);

    n->transitions = NULL;

    n->transition_count = 0;
    n->transition_capacity = 0;

    n->state_count = 0;

    n->start = -1;
    n->accept = -1;
}

//match NFA
int match_nfa(
    nfa n,
    const char *text,
    size_t len
)
{
    /*
     * current[i] indica si actualmente
     * podemos estar en el estado qi.
     */
    bool *current =
        calloc(n.state_count, sizeof(bool));

    if (current == NULL)
    {
        fprintf(stderr, "Error de memoria.\n");
        exit(EXIT_FAILURE);
    }


    /*
     * Comenzamos en el estado inicial.
     */
    current[n.start] = true;


    /*
     * Antes de leer cualquier carácter,
     * debemos seguir todas las transiciones epsilon.
     */
    epsilon_closure(n, current);


    /*
     * Procesamos la cadena carácter por carácter.
     */
    for (size_t position = 0; position < len; position++)
    {
        char symbol = text[position];


        /*
         * Conjunto de estados para el siguiente paso.
         */
        bool *next =
            calloc(n.state_count, sizeof(bool));

        if (next == NULL)
        {
            free(current);

            fprintf(stderr, "Error de memoria.\n");
            exit(EXIT_FAILURE);
        }


        /*
         * Revisamos todas las transiciones del NFA.
         */
        for (int i = 0; i < n.transition_count; i++)
        {
            transition t = n.transitions[i];


            /*
             * Una transición puede utilizarse cuando:
             *
             * 1. No es epsilon.
             * 2. Su símbolo coincide con el carácter actual.
             * 3. Actualmente podemos estar en su estado origen.
             */
            if (
                !t.epsilon &&
                t.symbol == symbol &&
                current[t.from]
            )
            {
                next[t.to] = true;
            }
        }


        /*
         * Después de consumir el carácter,
         * seguimos todas las transiciones epsilon.
         */
        epsilon_closure(n, next);


        /*
         * El conjunto next pasa a ser current.
         */
        free(current);

        current = next;
    }


    /*
     * La cadena es aceptada si, después de consumir
     * todos los caracteres, podemos encontrarnos
     * en el estado de aceptación.
     */
    int accepted =
        current[n.accept] ? 1 : 0;


    free(current);


    return accepted;
}

/*
 * Guarda el NFA en un archivo de texto
 * utilizando un formato estructurado (JSON).
 */
bool save_nfa(
    const nfa *n,
    const char *path
)
{
    /*
     * Verificamos que los punteros sean válidos.
     */
    if (n == NULL || path == NULL)
    {
        return false;
    }

    /*
     * Abrimos el archivo en modo escritura.
     */
    FILE *file = fopen(path, "w");

    if (file == NULL)
    {
        fprintf(
            stderr,
            "Error: No se pudo abrir el archivo '%s' para guardar el NFA.\n",
            path
        );
        return false;
    }

    /*
     * Serializamos los metadatos principales del autómata.
     */
    fprintf(file, "{\n");
    fprintf(file, "  \"state_count\": %d,\n", n->state_count);
    fprintf(file, "  \"start\": %d,\n", n->start);
    fprintf(file, "  \"accept\": %d,\n", n->accept);
    fprintf(file, "  \"transition_count\": %d,\n", n->transition_count);

    /*
     * Serializamos el arreglo de transiciones.
     */
    fprintf(file, "  \"transitions\": [\n");

    for (int i = 0; i < n->transition_count; i++)
    {
        transition t = n->transitions[i];

        fprintf(file, "    {\n");
        fprintf(file, "      \"from\": %d,\n", t.from);
        fprintf(file, "      \"to\": %d,\n", t.to);
        fprintf(file, "      \"symbol\": %d,\n", (int)t.symbol);
        fprintf(file, "      \"epsilon\": %s\n", t.epsilon ? "true" : "false");
        fprintf(
            file,
            "    }%s\n",
            (i == n->transition_count - 1) ? "" : ","
        );
    }

    fprintf(file, "  ]\n");
    fprintf(file, "}\n");

    /*
     * Cerramos el archivo correctamente.
     */
    fclose(file);

    return true;
}

