%{
#include <stdio.h>
%}

%option yylineno
%option noyywrap

%%

c { /* This recognizes a "c" */ }

cat { /* Recognizes "cat" */ }

cat|dog {
  /* "cat" or "dog" */
  foo();
  printf("Recognized: %s\n", yytext);
}

ca*t {
  /* "ct" or "cat" or "caat" or "caaaaaaat" ... */
  foo();
  printf("Recognized this many characters: %d\n", yyleng);
}

ca+t { /* "cat" or "caat" or "caaaaaaat" ... */ }

ca?t { /* "ct" or "cat" */ }

c.t { /* "cat" or "cbt" or "cct" or "cdt" ... */ }

\n { /* newline */ }
\t { /* tab */ }
c\.t { /* "c.t" */ }

[cat] { /* "c" or "a" or "t" */ }
[a-z] { /* "a" or "b" ... */ }
[A-Z] { /* "A" or "B" ... */ }
[0-9] { /* "0" or "1" ... */ }
[a-zA-Z0-9] { /* Any alphanumeric character */ }
[a-zA-Z0-9]* { /* Any number of alphanumeric characters */ }

[^cat] { /* Any character except "c", "a", "t" */ }
[^0-9] { /* Any non number character */ }

^cat { /* "cat" only at the beginning of a line */ }
cat$ { /* "cat" only at the end of a line */ }
cat\n { /* "cat\n" */ }
cat\n\r { /* On windows */ }

".^$?" { /* Literal ".^$?" */ }

(cat)* { /* Any number of the string "cat" */ }

<<EOF>> {
  /* End of file */
  yyterminate();
}

. {
  fprintf(
    stderr,
    "Invalid character on line %d: %s\n",
    yylineno,
    yytext
  );
  yyterminate();
}

%%

void foo() {
  printf("foo()!\n");
}

int main() {
  yylex();
}