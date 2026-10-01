#ifndef SCANNER_H
#define SCANNER_H

typedef enum ScannerToken {
    TOK_EOF = 0,
    TOK_ERROR = 256,

    /* Tipos de datos */
    TOK_KW_MANA,
    TOK_KW_AURA,
    TOK_KW_HALO,
    TOK_KW_GLIFO,
    TOK_KW_PROFECIA,
    TOK_KW_AUGURIO,
    TOK_KW_VACIO,

    /* Estructuras de control */
    TOK_KW_PRESAGIO,
    TOK_KW_MAL_AUGURIO,
    TOK_KW_RITUAL,
    TOK_KW_CONJURO,
    TOK_KW_CANALIZAR,

    /* Control de flujo */
    TOK_KW_OTORGAR,
    TOK_KW_DESTERRAR,
    TOK_KW_TRASCENDER,

    /* Funciones */
    TOK_KW_HECHIZO,
    TOK_KW_INVOCAR,

    /* Declaraciones */
    TOK_KW_ORBE,
    TOK_KW_RUNA,

    /* Entrada y salida */
    TOK_KW_REVELAR,
    TOK_KW_CONSULTAR,

    /* Valores especiales */
    TOK_KW_FAVORABLE,
    TOK_KW_ADVERSO,
    TOK_KW_ABISMO,

    /* Identificadores y literales */
    TOK_IDENTIFIER,
    TOK_INT_LITERAL,
    TOK_FLOAT_LITERAL,
    TOK_STRING_LITERAL,
    TOK_CHAR_LITERAL,

    /* Incremento, decremento y asignaciones */
    TOK_INC,
    TOK_DEC,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
    TOK_MOD_ASSIGN,
    TOK_ASSIGN,

    /* Comparacion */
    TOK_EQ,
    TOK_NEQ,
    TOK_LT,
    TOK_LE,
    TOK_GT,
    TOK_GE,

    /* Operadores logicos */
    TOK_AND,
    TOK_OR,
    TOK_NOT,

    /* Operadores aritmeticos */
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_MOD,

    /* Operadores a nivel de bits */
    TOK_SHL,
    TOK_SHR,
    TOK_BIT_AND,
    TOK_BIT_OR,
    TOK_BIT_XOR,
    TOK_BIT_NOT,

    /* Delimitadores */
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_COMMA,
    TOK_SEMICOLON

} ScannerToken;

const char *scanner_token_name(int token);

#endif // SCANNER_H