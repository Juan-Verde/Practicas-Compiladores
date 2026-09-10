#include "../regex.h"

#include <stdio.h>


static void print_regex(regex r)
{
    for (int i = 0; i < r.size; i++)
    {
        printf("%c", r.items[i].value);
    }

    printf("\n");
}


int main(void)
{
    regex r;


    r = parse_regex("ab");
    printf("ab -> ");
    print_regex(r);


    r = parse_regex("a|b");
    printf("a|b -> ");
    print_regex(r);


    r = parse_regex("(ab)*");
    printf("(ab)* -> ");
    print_regex(r);


    r = parse_regex("a(b|c)*");
    printf("a(b|c)* -> ");
    print_regex(r);


    return 0;
}