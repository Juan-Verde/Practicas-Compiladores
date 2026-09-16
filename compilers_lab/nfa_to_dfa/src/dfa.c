#include "dfa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Algoritmo 1 de la Practica 2: Move(T, a)
 * Regresa el conjunto de estados alcanzables desde T
 * consumiendo exactamente el simbolo 'a'.
 */
bool* move_operation(
    nfa n,
    bool *T,
    char a
)
{
    // R <- vacio
    bool *R = calloc(n.state_count, sizeof(bool));
    if (R == NULL) {
        fprintf(stderr, "Error de memoria en Move.\n");
        exit(EXIT_FAILURE);
    }

    //for all s in T
    for (int s = 0; s < n.state_count; s++)
    {
        if (T[s])
        {
            // Buscamos transiciones desde 's' con el simbolo 'a'
            for (int i = 0; i < n.transition_count; i++)
            {
                transition t = n.transitions[i];
                
                if (!t.epsilon && t.from == s && t.symbol == a)
                {
                    // R <- R U delta_N(s, a)
                    R[t.to] = true;
                }
            }
        }
    }

    return R;
}

/*
 * Funcion auxiliar para saber si un conjunto V esta vacio.
 */
static bool is_empty_set(bool *set, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (set[i]) return false;
    }
    return true;
}

/*
 * Funcion auxiliar para verificar si un conjunto V ya existe en Q_D.
 * Si existe, regresa su ID. Si no existe, regresa -1.
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

/*
 * Funcion auxiliar para obtener el alfabeto (Sigma) del NFA.
 */
static void get_alphabet(nfa n, char *alphabet, int *alpha_size)
{
    *alpha_size = 0;
    for (int i = 0; i < n.transition_count; i++)
    {
        char sym = n.transitions[i].symbol;
        if (!n.transitions[i].epsilon)
        {
            // Verificamos si ya lo agregamos
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

/*
 * Algoritmo 3 de la Practica 2: Construccion de Subconjuntos (NFA a DFA)
 */
dfa nfa_to_dfa(nfa n)
{
    dfa result;
    // Inicializamos las capacidades
    result.state_capacity = 128;
    result.states = malloc(sizeof(dfa_state) * result.state_capacity);
    result.state_count = 0;

    result.transition_capacity = 256;
    result.transitions = malloc(sizeof(dfa_transition) * result.transition_capacity);
    result.transition_count = 0;

    char alphabet[256];
    int alpha_size = 0;
    get_alphabet(n, alphabet, &alpha_size);

    // s0 <- e-closure({q0})
    bool *s0 = calloc(n.state_count, sizeof(bool));
    s0[n.start] = true;
    epsilon_closure(n, s0);

    // Q_D <- {s0}
    result.states[0].id = 0;
    result.states[0].nfa_states = s0;
    result.states[0].is_accept = false;
    result.state_count++;
    result.start = 0;

    /* 
     * Cola inicializada con [s0]
     * Usamos un indice 'unprocessed' sobre result.states.
     * Esto funciona como una cola: extraemos del frente
     * y agregamos nuevos estados al final.
     */
    int unprocessed = 0;

    // while Cola != vacio 
    while (unprocessed < result.state_count)
    {
        // U <- desencolar(Cola)
        int U_id = unprocessed;
        bool *U = result.states[U_id].nfa_states;
        unprocessed++;

        // for all a in Sigma
        for (int i = 0; i < alpha_size; i++)
        {
            char a = alphabet[i];

            // V <- e-closure(Move(U, a))
            bool *moved = move_operation(n, U, a);
            epsilon_closure(n, moved); // V esta contenido ahora en 'moved'

            // if V != vacio
            if (!is_empty_set(moved, n.state_count))
            {
                int V_id = find_state_in_QD(&result, moved, n.state_count);

                // if V no esta en Q_D 
                if (V_id == -1)
                {
                    // Q_D <- Q_D U {V} y 12: encolar(Cola, V)
                    V_id = result.state_count;
                    
                    result.states[V_id].id = V_id;
                    result.states[V_id].nfa_states = moved;
                    result.states[V_id].is_accept = false;
                    
                    result.state_count++;
                }
                else
                {
                    //Si ya existia, liberamos la memoria calculada
                    free(moved);
                }

                // delta_D(U, a) <- V
                result.transitions[result.transition_count].from = U_id;
                result.transitions[result.transition_count].to = V_id;
                result.transitions[result.transition_count].symbol = a;
                result.transition_count++;
            }
            else
            {
                free(moved);
            }
        }
    }

    // for all S in Q_D
    for (int i = 0; i < result.state_count; i++)
    {
        // if S intersecta F_N != vacio
        if (result.states[i].nfa_states[n.accept])
        {
            // F_D <- F_D U {S}
            result.states[i].is_accept = true;
        }
    }

    return result;
}

/*
 * Imprime la tabla de transiciones, sirve para validar 
 * visualmente la conversion.
 */
void print_dfa_table(dfa d)
{
    printf("\n========= TABLA DE TRANSICIONES DEL DFA =========\n");
    for (int i = 0; i < d.transition_count; i++)
    {
        dfa_transition t = d.transitions[i];
        
        printf(
            "q%d -- %c --> q%d\n", 
            t.from, 
            t.symbol, 
            t.to
        );
    }
    
    printf("\nEstado inicial: q%d\n", d.start);
    printf("Estados de aceptacion: ");
    
    for (int i = 0; i < d.state_count; i++)
    {
        if (d.states[i].is_accept) {
            printf("q%d ", d.states[i].id);
        }
    }
    printf("\n=================================================\n");
}

/*
 * Libera la memoria de forma segura.
 */
void free_dfa(dfa *d)
{
    for(int i = 0; i < d->state_count; i++) {
        free(d->states[i].nfa_states);
    }
    free(d->states);
    free(d->transitions);
    d->state_count = 0;
    d->transition_count = 0;
}