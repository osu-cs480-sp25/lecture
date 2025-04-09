%{
#include <stdio.h>

int num_chars = 0;
int num_words = 0;
int num_lines = 0;
%}

%option noyywrap

%%

\n {
  num_lines++;
  num_chars++;
 }

[^ \t\n]+ {
  num_words++;
  num_chars += yyleng;
}

. {
  num_chars++;
}

%%

int main() {
  yylex();
  printf("%d characters\n", num_chars);
  printf("%d words\n", num_words);
  printf("%d lines\n", num_lines);
  return 0;
}
