#include "min_dfa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * Auxiliares
 * ============================================================ */

static int get_dfa_alphabet(dfa d, char *alphabet)
{
    int size = 0;
    for (int i = 0; i < d.transition_count; i++)
    {
        char sym = d.transitions[i].symbol;
        bool exists = false;
        for (int j = 0; j < size; j++)
        {
            if (alphabet[j] == sym) { exists = true; break; }
        }
        if (!exists) alphabet[size++] = sym;
    }
    return size;
}

static int symbol_index(char *alphabet, int alpha_size, char c)
{
    for (int i = 0; i < alpha_size; i++)
    {
        if (alphabet[i] == c) return i;
    }
    return -1;
}

/* ============================================================
 * Minimización (refinamiento de particiones)
 * ============================================================ */
dfa minimize_dfa(dfa d)
{
    dfa result;

    /* Caso borde: DFA vacío */
    if (d.state_count == 0)
    {
        result.start = -1;
        result.states = NULL;
        result.state_count = 0;
        result.state_capacity = 0;
        result.transitions = NULL;
        result.transition_count = 0;
        result.transition_capacity = 0;
        return result;
    }

    char alphabet[256];
    int alpha_size = get_dfa_alphabet(d, alphabet);

    /* Caso borde: sin alfabeto */
    if (alpha_size == 0)
    {
        result.state_capacity = 1;
        result.states = malloc(sizeof(dfa_state));
        result.states[0].id = 0;
        result.states[0].nfa_states = NULL;
        result.states[0].is_accept = d.states[d.start].is_accept;
        result.state_count = 1;
        result.start = 0;
        result.transitions = NULL;
        result.transition_count = 0;
        result.transition_capacity = 0;
        return result;
    }

    int orig_n = d.state_count;
    int sink = orig_n;        /* índice del sumidero */
    int work_n = orig_n + 1;  /* +1 para el sumidero */

    /* Tabla de transiciones completa: estado * alpha_size + símbolo */
    int *trans = malloc(sizeof(int) * work_n * alpha_size);
    bool *is_accept = calloc(work_n, sizeof(bool));

    if (!trans || !is_accept)
    {
        fprintf(stderr, "Error de memoria en minimize_dfa.\n");
        exit(EXIT_FAILURE);
    }

    /* Inicialmente todo apunta al sumidero */
    for (int s = 0; s < work_n; s++)
    {
        for (int i = 0; i < alpha_size; i++)
        {
            trans[s * alpha_size + i] = sink;
        }
    }
    for (int s = 0; s < orig_n; s++)
    {
        is_accept[s] = d.states[s].is_accept;
    }
    is_accept[sink] = false;

    /* Llenamos con las transiciones reales */
    for (int t = 0; t < d.transition_count; t++)
    {
        dfa_transition tr = d.transitions[t];
        int si = symbol_index(alphabet, alpha_size, tr.symbol);
        if (si >= 0 && tr.from >= 0 && tr.from < orig_n &&
            tr.to >= 0 && tr.to < orig_n)
        {
            trans[tr.from * alpha_size + si] = tr.to;
        }
    }

    /* ---------- Estados alcanzables desde el inicio ---------- */
    bool *reachable = calloc(work_n, sizeof(bool));
    int *stack = malloc(sizeof(int) * work_n);
    int top = -1;

    reachable[d.start] = true;
    stack[++top] = d.start;

    while (top >= 0)
    {
        int s = stack[top--];
        for (int i = 0; i < alpha_size; i++)
        {
            int t = trans[s * alpha_size + i];
            if (!reachable[t])
            {
                reachable[t] = true;
                stack[++top] = t;
            }
        }
    }
    free(stack);

    /* ---------- Partición inicial {F, Q\F} ---------- */
    int *block_of = malloc(sizeof(int) * work_n);
    for (int s = 0; s < work_n; s++) block_of[s] = -1;

    int num_blocks = 0;
    int acc_block = -1, nonacc_block = -1;

    for (int s = 0; s < work_n; s++)
    {
        if (!reachable[s]) continue;
        if (is_accept[s])
        {
            if (acc_block == -1) acc_block = num_blocks++;
            block_of[s] = acc_block;
        }
        else
        {
            if (nonacc_block == -1) nonacc_block = num_blocks++;
            block_of[s] = nonacc_block;
        }
    }

    /* ---------- Refinamiento iterativo ---------- */
    int *sig = malloc(sizeof(int) * alpha_size);
    int *new_id_of = malloc(sizeof(int) * work_n);
    int *sub_block = malloc(sizeof(int) * work_n);
    int *sub_sig = malloc(sizeof(int) * work_n * alpha_size);

    bool changed = true;
    while (changed)
    {
        changed = false;
        int subgroup_count = 0;

        for (int s = 0; s < work_n; s++)
        {
            if (!reachable[s]) { new_id_of[s] = -1; continue; }

            for (int i = 0; i < alpha_size; i++)
            {
                int t = trans[s * alpha_size + i];
                sig[i] = block_of[t];
            }

            int found = -1;
            for (int g = 0; g < subgroup_count; g++)
            {
                if (sub_block[g] != block_of[s]) continue;
                bool match = true;
                for (int i = 0; i < alpha_size; i++)
                {
                    if (sub_sig[g * alpha_size + i] != sig[i])
                    {
                        match = false;
                        break;
                    }
                }
                if (match) { found = g; break; }
            }

            if (found == -1)
            {
                found = subgroup_count++;
                sub_block[found] = block_of[s];
                for (int i = 0; i < alpha_size; i++)
                {
                    sub_sig[found * alpha_size + i] = sig[i];
                }
            }
            new_id_of[s] = found;
        }

        if (subgroup_count > num_blocks)
        {
            memcpy(block_of, new_id_of, sizeof(int) * work_n);
            num_blocks = subgroup_count;
            changed = true;
        }
    }

    /* ---------- Construir el DFA minimizado ---------- */
    int *rep = malloc(sizeof(int) * num_blocks);
    for (int b = 0; b < num_blocks; b++) rep[b] = -1;
    for (int s = 0; s < work_n; s++)
    {
        if (reachable[s] && block_of[s] >= 0 && rep[block_of[s]] == -1)
        {
            rep[block_of[s]] = s;
        }
    }

    result.state_capacity = num_blocks;
    result.states = malloc(sizeof(dfa_state) * num_blocks);
    result.state_count = num_blocks;

    result.transition_capacity = num_blocks * alpha_size;
    if (result.transition_capacity == 0) result.transition_capacity = 1;
    result.transitions = malloc(sizeof(dfa_transition) * result.transition_capacity);
    result.transition_count = 0;

    for (int b = 0; b < num_blocks; b++)
    {
        result.states[b].id = b;
        result.states[b].nfa_states = NULL;
        result.states[b].is_accept = is_accept[rep[b]];
    }
    result.start = block_of[d.start];

    for (int b = 0; b < num_blocks; b++)
    {
        int s = rep[b];
        for (int i = 0; i < alpha_size; i++)
        {
            int t = trans[s * alpha_size + i];
            int tb = block_of[t];
            if (tb < 0) continue;
            result.transitions[result.transition_count].from = b;
            result.transitions[result.transition_count].to = tb;
            result.transitions[result.transition_count].symbol = alphabet[i];
            result.transition_count++;
        }
    }

    /* ---------- Liberar memoria temporal ---------- */
    free(trans);
    free(is_accept);
    free(reachable);
    free(block_of);
    free(sig);
    free(new_id_of);
    free(sub_block);
    free(sub_sig);
    free(rep);

    return result;
}

/* ============================================================
 * Impresión y simulación
 * ============================================================ */
void print_dfa_min(dfa d)
{
    printf("\n======= TABLA DE TRANSICIONES DEL DFA MINIMIZADO =======\n");
    for (int i = 0; i < d.transition_count; i++)
    {
        dfa_transition t = d.transitions[i];
        printf("q%d -- %c --> q%d\n", t.from, t.symbol, t.to);
    }

    printf("\nEstado inicial: q%d\n", d.start);
    printf("Estados de aceptacion: ");
    for (int i = 0; i < d.state_count; i++)
    {
        if (d.states[i].is_accept) printf("q%d ", d.states[i].id);
    }
    printf("\n========================================================\n");
}

bool test_string(dfa d, const char *input)
{
    int current = d.start;
    for (int i = 0; input[i] != '\0'; i++)
    {
        char c = input[i];
        int next = -1;
        for (int t = 0; t < d.transition_count; t++)
        {
            if (d.transitions[t].from == current &&
                d.transitions[t].symbol == c)
            {
                next = d.transitions[t].to;
                break;
            }
        }
        if (next == -1) return false;
        current = next;
    }
    return d.states[current].is_accept;
}