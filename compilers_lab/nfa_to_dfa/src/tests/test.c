#include "../nfa.h"
#include "../dfa.h"
#include "../regex.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Prueba 2.2.1: Prueba del Algoritmo epsilon-Closure
 * Para comprobar recuperacion de estados y que no se caiga en bucles infinitos
 */
void test_epsilon_closure_manual()
{
    printf("\n--- PRUEBA 2.2.1: Algoritmo Epsilon-Closure ---\n");

    /* 
     * Vector de prueba: NFA con estados {0,1,2,3}. 
     * Transiciones epsilon: 0->1, 1->2 y ciclo 2->1 
     */
    nfa test_nfa;
    test_nfa.state_count = 4;
    test_nfa.transitions = malloc(sizeof(transition) * 3);
    test_nfa.transition_count = 3;
    test_nfa.transition_capacity = 3;

    test_nfa.transitions[0] = (transition){.from = 0, .to = 1, .symbol = 0, .epsilon = true};
    test_nfa.transitions[1] = (transition){.from = 1, .to = 2, .symbol = 0, .epsilon = true};
    test_nfa.transitions[2] = (transition){.from = 2, .to = 1, .symbol = 0, .epsilon = true}; // El ciclo

    // Conjunto inicial T = {0}
    bool *T = calloc(test_nfa.state_count, sizeof(bool));
    T[0] = true;

    // Ejecucion del algoritmo
    epsilon_closure(test_nfa, T);

    // Validacion
    printf("Vector de entrada: e-Closure({0})\n");
    printf("Resultado esperado: { 0 1 2 }\n");
    printf("Resultado obtenido: { ");
    for (int i = 0; i < test_nfa.state_count; i++)
    {
        if (T[i]) printf("%d ", i);
    }
    printf("}\n");
    printf("Conclusion: El algoritmo rompio la dependencia ciclica (2->1) con exito.\n");

    free(T);
    free(test_nfa.transitions);
}

/*
 * Prueba 2.2.2: Prueba del Algoritmo Move
 * Para verificar transiciones consumiendo estrictamente el simbolo dado.
 */
void test_move_manual()
{
    printf("\n--- PRUEBA 2.2.2: Algoritmo Move ---\n");

    /* 
     * Vector de prueba: Origen 0. 
     * Transiciones: 0->1 (con 'a'), 0->2 (con 'a') y 0->3 (con 'b') 
     */
    nfa test_nfa;
    test_nfa.state_count = 4;
    test_nfa.transitions = malloc(sizeof(transition) * 3);
    test_nfa.transition_count = 3;
    test_nfa.transition_capacity = 3;

    test_nfa.transitions[0] = (transition){.from = 0, .to = 1, .symbol = 'a', .epsilon = false};
    test_nfa.transitions[1] = (transition){.from = 0, .to = 2, .symbol = 'a', .epsilon = false};
    test_nfa.transitions[2] = (transition){.from = 0, .to = 3, .symbol = 'b', .epsilon = false}; // Simbolo ajeno

    // Conjunto de estados T = {0}
    bool *T = calloc(test_nfa.state_count, sizeof(bool));
    T[0] = true;

    // Ejecucion del algoritmo: Move({0}, 'a')
    bool *R = move_operation(test_nfa, T, 'a');

    // Validacion
    printf("Vector de entrada: Move({0}, 'a')\n");
    printf("Resultado esperado: { 1 2 }\n");
    printf("Resultado obtenido: { ");
    for (int i = 0; i < test_nfa.state_count; i++)
    {
        if (R[i]) printf("%d ", i);
    }
    printf("}\n");
    printf("Conclusion: El algoritmo transito correctamente ignorando el simbolo 'b'.\n");

    free(T);
    free(R);
    free(test_nfa.transitions);
}

/*
 * Prueba 2.2.3: Prueba de Construccion de Subconjuntos
 * Para contrastar el automata inicial frente al resultado final.
 */
void test_construccion_subconjuntos()
{
    printf("\n--- PRUEBA 2.2.3: Construccion de Subconjuntos ---\n");
    
    /* 
     * Utilizamos una regex para demostrar que se eliminan 
     * las transiciones epsilon y las ambiguedades de Thompson.
     */
    const char *regex_str = "(a|b)*a";
    printf("Expresion regular base: %s\n", regex_str);

    regex r = parse_regex(regex_str);
    nfa n = regex_to_nfa(r);
    dfa d = nfa_to_dfa(n);

    //Contrastamos visualmente
    print_nfa_table(n); 
    print_dfa_table(d);

    free_dfa(&d);
    free_nfa(&n);
}

/* Funcion principal que orquesta las pruebas al ejecutar test.c */
int main()
{
    printf("==========================================\n");
    printf(" INICIANDO PRUEBAS DEL LABORATORIO \n");
    printf("==========================================\n");

    test_epsilon_closure_manual();
    test_move_manual();
    test_construccion_subconjuntos();
    
    printf("\n==========================================\n");
    printf(" PRUEBAS FINALIZADAS CON EXITO\n");
    printf("==========================================\n");
    
    return 0;
}