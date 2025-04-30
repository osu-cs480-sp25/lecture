/*
 * Lexer definition
 */

%{
#include <iostream>
#include "parser.h"
%}

%option noyywrap

%%

[ \t]*    /* Ignore whitespace. */

[a-z][0-9]? {
    yylval = new std::string(yytext); return IDENTIFIER;
}

[0-9]+ {
    yylval = new std::string(yytext); return INTEGER; 
}

"="     return ASSIGN;
"+"     return PLUS;
"-"     return MINUS;
"*"     return TIMES;
"/"     return DIVIDEDBY;

"("     return LPAREN;
")"     return RPAREN;

\n      { return NEWLINE; }
\r
.       { printf("Unexpected character:: %d\n", yytext[0]); return 0; }

%%
