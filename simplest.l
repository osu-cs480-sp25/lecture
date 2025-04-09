/* one flex option, no declarations, no named definitions */
%option noyywrap

%%

%%

/* main function */

int main()
{
    return yylex();
}