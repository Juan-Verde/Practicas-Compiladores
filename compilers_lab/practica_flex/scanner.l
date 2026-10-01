%option noyywrap

%{
#include "scanner.h"
%}

%x COMMENT

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INT         {DIGIT}+
FLOAT       {DIGIT}+"."{DIGIT}+
STRING      \"([^\\\n\"]|\\.)*\"
CHAR        \'([^\\\n\']|\\.)\'

%%

"mana"              { return TOK_KW_MANA; }
"aura"              { return TOK_KW_AURA; }
"halo"              { return TOK_KW_HALO; }
"glifo"             { return TOK_KW_GLIFO; }
"profecia"          { return TOK_KW_PROFECIA; }
"augurio"           { return TOK_KW_AUGURIO; }
"vacio"             { return TOK_KW_VACIO; }

"presagio"          { return TOK_KW_PRESAGIO; }
"mal_augurio"       { return TOK_KW_MAL_AUGURIO; }
"ritual"            { return TOK_KW_RITUAL; }
"conjuro"           { return TOK_KW_CONJURO; }
"canalizar"         { return TOK_KW_CANALIZAR; }

"otorgar"           { return TOK_KW_OTORGAR; }
"desterrar"         { return TOK_KW_DESTERRAR; }
"trascender"        { return TOK_KW_TRASCENDER; }

"hechizo"           { return TOK_KW_HECHIZO; }
"invocar"           { return TOK_KW_INVOCAR; }

"orbe"              { return TOK_KW_ORBE; }
"runa"              { return TOK_KW_RUNA; }

"revelar"           { return TOK_KW_REVELAR; }
"consultar"         { return TOK_KW_CONSULTAR; }

"favorable"         { return TOK_KW_FAVORABLE; }
"adverso"           { return TOK_KW_ADVERSO; }
"abismo"            { return TOK_KW_ABISMO; }

"++"                { return TOK_INC; }
"--"                { return TOK_DEC; }

"+="                { return TOK_PLUS_ASSIGN; }
"-="                { return TOK_MINUS_ASSIGN; }
"*="                { return TOK_MUL_ASSIGN; }
"/="                { return TOK_DIV_ASSIGN; }
"%="                { return TOK_MOD_ASSIGN; }

"=="                { return TOK_EQ; }
"!="                { return TOK_NEQ; }
"<="                { return TOK_LE; }
">="                { return TOK_GE; }

"<<"                { return TOK_SHL; }
">>"                { return TOK_SHR; }

"vinculo"           { return TOK_AND; }
"desvinculo"        { return TOK_OR; }
"negar"             { return TOK_NOT; }

"&&"                { return TOK_AND; }
"||"                { return TOK_OR; }
"!"                 { return TOK_NOT; }

"="                 { return TOK_ASSIGN; }

"<"                 { return TOK_LT; }
">"                 { return TOK_GT; }

"+"                 { return TOK_PLUS; }
"-"                 { return TOK_MINUS; }
"*"                 { return TOK_MUL; }
"/"                 { return TOK_DIV; }
"%"                 { return TOK_MOD; }

"&"                 { return TOK_BIT_AND; }
"|"                 { return TOK_BIT_OR; }
"^"                 { return TOK_BIT_XOR; }
"~"                 { return TOK_BIT_NOT; }

"("                 { return TOK_LPAREN; }
")"                 { return TOK_RPAREN; }
"{"                 { return TOK_LBRACE; }
"}"                 { return TOK_RBRACE; }
"["                 { return TOK_LBRACKET; }
"]"                 { return TOK_RBRACKET; }
","                 { return TOK_COMMA; }
";"                 { return TOK_SEMICOLON; }

{FLOAT}             { return TOK_FLOAT_LITERAL; }
{INT}               { return TOK_INT_LITERAL; }
{STRING}            { return TOK_STRING_LITERAL; }
{CHAR}              { return TOK_CHAR_LITERAL; }

{ID}                { return TOK_IDENTIFIER; }

"//"[^\n]*          { }

"/*"                { BEGIN(COMMENT); }
<COMMENT>"*/"       { BEGIN(INITIAL); }
<COMMENT>\n         { }
<COMMENT>.          { }

[ \t\r\n]+          { }

.                   { return TOK_ERROR; }

%%