%{
#include <iostream>
#include "parser-push.h"
%}

%option noyywrap
%option yylineno

%%

%{
  yypstate* pstate = yypstate_new();

  YYSTYPE yylval;
  YYLTYPE loc;

  #define PUSH_TOKEN(category, lexeme) do {                      \
    yylval = lexeme ? new std::string(lexeme) : NULL;            \
    loc.first_line = loc.last_line = yylineno;                   \
    int status = yypush_parse(pstate, category, &yylval, &loc);  \
    if (status != YYPUSH_MORE) {                                 \
      yypstate_delete(pstate);                                   \
      return status;                                             \
    }                                                            \
  } while (0)
%}

[ \t]*    /* Ignore whitespace. */

[a-z][0-9]? {
    PUSH_TOKEN(IDENTIFIER, yytext);
}

[0-9]+ {
    PUSH_TOKEN(INTEGER, yytext);
}

"="     PUSH_TOKEN(ASSIGN, NULL) ;
"+"     PUSH_TOKEN(PLUS, NULL);
"-"     PUSH_TOKEN(MINUS, NULL);
"*"     PUSH_TOKEN(TIMES, NULL);
"/"     PUSH_TOKEN(DIVIDEDBY, NULL);

"("     PUSH_TOKEN(LPAREN, NULL);
")"     PUSH_TOKEN(RPAREN, NULL);

\n      PUSH_TOKEN(NEWLINE, NULL);
\r
.       {
            printf("Unexpected character: %d\n", yytext);
            return 0; 
        }

<<EOF>>  {
    int s = yypush_parse(pstate, 0, NULL, NULL);
    yypstate_delete(pstate);
    return s;
}

%%
