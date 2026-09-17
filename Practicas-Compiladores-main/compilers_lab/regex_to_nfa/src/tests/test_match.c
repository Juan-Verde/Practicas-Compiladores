#include "../regex.h"
#include "../nfa.h"

#include <stdio.h>
#include <string.h>


static void test(
    nfa n,
    const char *text
)
{
    int result =
        match_nfa(
            n,
            text,
            strlen(text)
        );


    printf(
        "\"%s\" -> %d\n",
        text,
        result
    );
}


int main(void)
{
    regex r =
        parse_regex("(ab)*");


    nfa n =
        regex_to_nfa(r);


    test(n, "");
    test(n, "ab");
    test(n, "aba");
    test(n, "abab");
    test(n, "ababab");
    test(n, "a");
    test(n, "b");


    free_nfa(&n);


    return 0;
}