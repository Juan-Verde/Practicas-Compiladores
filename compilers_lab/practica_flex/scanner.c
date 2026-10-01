#include <stdio.h>

#include "scanner.h"

extern int yylex(void);
extern char *yytext;

int main(void)
{
    int token;

    while ((token = yylex()) != TOK_EOF) {
        printf("[%s:%s]\n", scanner_token_name(token), yytext);
    }

    return 0;
}

const char *scanner_token_name(int token)
{
    switch (token) {

        case TOK_EOF: return "EOF";
        case TOK_ERROR: return "ERROR";

        /* Tipos de datos */
        case TOK_KW_MANA: return "KW_MANA";
        case TOK_KW_AURA: return "KW_AURA";
        case TOK_KW_HALO: return "KW_HALO";
        case TOK_KW_GLIFO: return "KW_GLIFO";
        case TOK_KW_PROFECIA: return "KW_PROFECIA";
        case TOK_KW_AUGURIO: return "KW_AUGURIO";
        case TOK_KW_VACIO: return "KW_VACIO";

        /* Estructuras de control */
        case TOK_KW_PRESAGIO: return "KW_PRESAGIO";
        case TOK_KW_MAL_AUGURIO: return "KW_MAL_AUGURIO";
        case TOK_KW_RITUAL: return "KW_RITUAL";
        case TOK_KW_CONJURO: return "KW_CONJURO";
        case TOK_KW_CANALIZAR: return "KW_CANALIZAR";

        /* Control de flujo */
        case TOK_KW_OTORGAR: return "KW_OTORGAR";
        case TOK_KW_DESTERRAR: return "KW_DESTERRAR";
        case TOK_KW_TRASCENDER: return "KW_TRASCENDER";

        /* Funciones */
        case TOK_KW_HECHIZO: return "KW_HECHIZO";
        case TOK_KW_INVOCAR: return "KW_INVOCAR";

        /* Declaraciones */
        case TOK_KW_ORBE: return "KW_ORBE";
        case TOK_KW_RUNA: return "KW_RUNA";

        /* Entrada y salida */
        case TOK_KW_REVELAR: return "KW_REVELAR";
        case TOK_KW_CONSULTAR: return "KW_CONSULTAR";

        /* Valores especiales */
        case TOK_KW_FAVORABLE: return "KW_FAVORABLE";
        case TOK_KW_ADVERSO: return "KW_ADVERSO";
        case TOK_KW_ABISMO: return "KW_ABISMO";

        /* Identificadores y literales */
        case TOK_IDENTIFIER: return "IDENTIFIER";
        case TOK_INT_LITERAL: return "INT_LITERAL";
        case TOK_FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TOK_STRING_LITERAL: return "STRING_LITERAL";
        case TOK_CHAR_LITERAL: return "CHAR_LITERAL";

        /* Incremento, decremento y asignacion */
        case TOK_INC: return "INC";
        case TOK_DEC: return "DEC";
        case TOK_PLUS_ASSIGN: return "PLUS_ASSIGN";
        case TOK_MINUS_ASSIGN: return "MINUS_ASSIGN";
        case TOK_MUL_ASSIGN: return "MUL_ASSIGN";
        case TOK_DIV_ASSIGN: return "DIV_ASSIGN";
        case TOK_MOD_ASSIGN: return "MOD_ASSIGN";
        case TOK_ASSIGN: return "ASSIGN";

        /* Comparacion */
        case TOK_EQ: return "EQ";
        case TOK_NEQ: return "NEQ";
        case TOK_LT: return "LT";
        case TOK_LE: return "LE";
        case TOK_GT: return "GT";
        case TOK_GE: return "GE";

        /* Logicos */
        case TOK_AND: return "AND";
        case TOK_OR: return "OR";
        case TOK_NOT: return "NOT";

        /* Aritmeticos */
        case TOK_PLUS: return "PLUS";
        case TOK_MINUS: return "MINUS";
        case TOK_MUL: return "MUL";
        case TOK_DIV: return "DIV";
        case TOK_MOD: return "MOD";

        /* Bits */
        case TOK_SHL: return "SHL";
        case TOK_SHR: return "SHR";
        case TOK_BIT_AND: return "BIT_AND";
        case TOK_BIT_OR: return "BIT_OR";
        case TOK_BIT_XOR: return "BIT_XOR";
        case TOK_BIT_NOT: return "BIT_NOT";

        /* Delimitadores */
        case TOK_LPAREN: return "LPAREN";
        case TOK_RPAREN: return "RPAREN";
        case TOK_LBRACE: return "LBRACE";
        case TOK_RBRACE: return "RBRACE";
        case TOK_LBRACKET: return "LBRACKET";
        case TOK_RBRACKET: return "RBRACKET";
        case TOK_COMMA: return "COMMA";
        case TOK_SEMICOLON: return "SEMICOLON";

        default: return "UNKNOWN";
    }
}