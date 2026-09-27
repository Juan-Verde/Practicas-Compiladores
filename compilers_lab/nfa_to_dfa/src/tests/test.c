#include "../nfa.h"
#include "../dfa.h"
#include "../min_dfa.h"
#include "../regex.h"
#include <stdio.h>
#include <stdlib.h>

void test_epsilon_closure_manual()
{
    printf("\n--- PRUEBA 2.2.1: Algoritmo Epsilon-Closure ---\n");

    nfa test_nfa;
    test_nfa.state_count = 4;
    test_nfa.transitions = malloc(sizeof(transition) * 3);
    test_nfa.transition_count = 3;
    test_nfa.transition_capacity = 3;

    test_nfa.transitions[0] = (transition){.from = 0, .to = 1, .symbol = 0, .epsilon = true};
    test_nfa.transitions[1] = (transition){.from = 1, .to = 2, .symbol = 0, .epsilon = true};
    test_nfa.transitions[2] = (transition){.from = 2, .to = 1, .symbol = 0, .epsilon = true};

    bool *T = calloc(test_nfa.state_count, sizeof(bool));
    T[0] = true;

    epsilon_closure(test_nfa, T);

    printf("Resultado obtenido: { ");
    for (int i = 0; i < test_nfa.state_count; i++)
        if (T[i]) printf("%d ", i);
    printf("}\n");

    free(T);
    free(test_nfa.transitions);
}

void test_move_manual()
{
    printf("\n--- PRUEBA 2.2.2: Algoritmo Move ---\n");

    nfa test_nfa;
    test_nfa.state_count = 4;
    test_nfa.transitions = malloc(sizeof(transition) * 3);
    test_nfa.transition_count = 3;
    test_nfa.transition_capacity = 3;

    test_nfa.transitions[0] = (transition){.from = 0, .to = 1, .symbol = 'a', .epsilon = false};
    test_nfa.transitions[1] = (transition){.from = 0, .to = 2, .symbol = 'a', .epsilon = false};
    test_nfa.transitions[2] = (transition){.from = 0, .to = 3, .symbol = 'b', .epsilon = false};

    bool *T = calloc(test_nfa.state_count, sizeof(bool));
    T[0] = true;

    bool *R = move_operation(test_nfa, T, 'a');

    printf("Resultado obtenido: { ");
    for (int i = 0; i < test_nfa.state_count; i++)
        if (R[i]) printf("%d ", i);
    printf("}\n");

    free(T);
    free(R);
    free(test_nfa.transitions);
}

void test_construccion_subconjuntos()
{
    printf("\n--- PRUEBA 2.2.3: Construccion de Subconjuntos ---\n");

    const char *regex_str = "(a|b)*a";
    printf("Expresion regular base: %s\n", regex_str);

    regex r = parse_regex(regex_str);
    nfa n = regex_to_nfa(r);
    dfa d = nfa_to_dfa(n);

    print_nfa_table(n);
    print_dfa_table(d);

    free_dfa(&d);
    free_nfa(&n);
}

void test_nfa_hardcoded_to_dfa()
{
    printf("\n--- PRUEBA ADICIONAL: NFA hardcodeado -> DFA ---\n");

    nfa n;
    n.state_count = 3;
    n.start = 0;
    n.accept = 2;
    n.transition_capacity = 8;
    n.transition_count = 4;
    n.transitions = malloc(sizeof(transition) * n.transition_capacity);

    n.transitions[0] = (transition){.from = 0, .to = 0, .symbol = 'a', .epsilon = false};
    n.transitions[1] = (transition){.from = 0, .to = 1, .symbol = 'a', .epsilon = false};
    n.transitions[2] = (transition){.from = 0, .to = 0, .symbol = 'b', .epsilon = false};
    n.transitions[3] = (transition){.from = 1, .to = 2, .symbol = 'b', .epsilon = false};

    print_nfa_table(n);
    dfa d = nfa_to_dfa(n);
    print_dfa_table(d);

    free_dfa(&d);
    free_nfa(&n);
}

/* ============================================================
 * PRUEBAS DE MINIMIZACIÓN (Práctica 3)
 * ============================================================ */

static void run_test_suite(dfa dmin,
                           const char **accept, const char **reject,
                           int n_accept, int n_reject)
{
    int passed = 0;
    int total = n_accept + n_reject;

    printf("\n[Aceptacion]\n");
    for (int i = 0; i < n_accept; i++)
    {
        bool res = test_string(dmin, accept[i]);
        printf("  \"%s\" -> %s\n", accept[i], res ? "PASS" : "FAIL");
        if (res) passed++;
    }

    printf("[Rechazo]\n");
    for (int i = 0; i < n_reject; i++)
    {
        bool res = !test_string(dmin, reject[i]);
        printf("  \"%s\" -> %s\n", reject[i], res ? "PASS" : "FAIL");
        if (res) passed++;
    }

    printf("Resultado: %d/%d pruebas superadas.\n", passed, total);
}

static void test_minimizacion(const char *regex_str,
                              const char **accept, const char **reject)
{
    printf("\n### Regex: %s\n", regex_str);

    regex r = parse_regex(regex_str);
    nfa n = regex_to_nfa(r);
    dfa d = nfa_to_dfa(n);
    dfa dmin = minimize_dfa(d);

    print_dfa_table(d);
    print_dfa_min(dmin);

    printf("\nComparacion: |Q| = %d, |Q'| = %d\n",
           d.state_count, dmin.state_count);

    if (d.state_count >= dmin.state_count)
        printf("OK: |Q| >= |Q'|\n");
    else
        printf("ERROR: la minimizacion aumento estados\n");

    run_test_suite(dmin, accept, reject, 10, 10);

    free_dfa(&dmin);
    free_dfa(&d);
    free_nfa(&n);
}

void test_minimizacion_3_regex()
{
    printf("\n==========================================\n");
    printf(" PRUEBA 3: Minimizacion de DFA\n");
    printf("==========================================\n");

    /* ---------- Regex 1: (a|b)*abb ---------- */
    const char *acc1[] = {
        "abb", "aabb", "babb", "aababb", "bbabb",
        "aaabb", "bababb", "bbababb", "abbabb", "baabb"
    };
    const char *rej1[] = {
        "", "a", "b", "ab", "ba",
        "aba", "bab", "bba", "aab", "bbb"
    };
    test_minimizacion("(a|b)*abb", acc1, rej1);

    /* ---------- Regex 2: (0|1)*01(0|1)* ---------- */
    const char *acc2[] = {
        "01", "001", "010", "011", "101",
        "0100", "0011", "1101", "01010", "10101"
    };
    const char *rej2[] = {
        "", "0", "1", "00", "11",
        "10", "000", "111", "100", "110"
    };
    test_minimizacion("(0|1)*01(0|1)*", acc2, rej2);

    /* ---------- Regex 3: (a|b)*ab(a|b)* ---------- */
    const char *acc3[] = {
        "ab", "aab", "abb", "aba", "bab",
        "aabb", "abab", "babb", "abba", "abbb"
    };
    const char *rej3[] = {
        "", "a", "b", "aa", "bb",
        "ba", "aaa", "bbb", "bba", "aaaa"
    };
    test_minimizacion("(a|b)*ab(a|b)*", acc3, rej3);
}

/* ============================================================
 * main
 * ============================================================ */
int main()
{
    printf("==========================================\n");
    printf(" INICIANDO PRUEBAS DEL LABORATORIO \n");
    printf("==========================================\n");

    test_epsilon_closure_manual();
    test_move_manual();
    test_construccion_subconjuntos();
    test_nfa_hardcoded_to_dfa();
    test_minimizacion_3_regex();

    printf("\n==========================================\n");
    printf(" PRUEBAS FINALIZADAS CON EXITO\n");
    printf("==========================================\n");

    return 0;
}