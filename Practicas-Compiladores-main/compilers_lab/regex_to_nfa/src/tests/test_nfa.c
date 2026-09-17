#include "../regex.h"
#include "../nfa.h"

#include <stdio.h>


static void print_nfa(nfa n)
{
    printf("Estado inicial: q%d\n", n.start);
    printf("Estado final: q%d\n", n.accept);

    printf("Estados: %d\n", n.state_count);

    printf("Transiciones:\n");


    for (int i = 0; i < n.transition_count; i++)
    {
        transition t = n.transitions[i];


        if (t.epsilon)
        {
            printf(
                "q%d --epsilon--> q%d\n",
                t.from,
                t.to
            );
        }
        else
        {
            printf(
                "q%d --%c--> q%d\n",
                t.from,
                t.symbol,
                t.to
            );
        }
    }
}


int main(void)
{
    regex r =
        parse_regex("ab");

    nfa n =
        regex_to_nfa(r);


    print_nfa(n);


    free_nfa(&n);


    return 0;
}