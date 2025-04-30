%{
#include <iostream>
#include <set>
#include "parser-push.h"

void yyerror(YYLTYPE* loc, const char* err);
int yylex();
std::set<std::string> symbols;

%}

%locations
%define parse.error verbose
%define api.value.type { std::string* }
%define api.pure full
%define api.push-pull push

/*
 * These are all of the terminals in our grammar, i.e. the syntactic
 * categories that can be recognized by the lexer.
 */

%token NEWLINE INTEGER IDENTIFIER
%token LPAREN RPAREN
%token PLUS MINUS TIMES DIVIDEDBY
%token ASSIGN

/*
 * Here, we're defining the precedence of the operators.  The ones that appear
 * later have higher precedence.  All of the operators are left-associative
 * except the "not" operator, which is right-associative.
 */
%left PLUS MINUS
%left TIMES DIVIDEDBY

%start input

%%

input
  : input assignmentStatement
  | assignmentStatement
  ;
    
assignmentStatement
  : IDENTIFIER ASSIGN expression NEWLINE {
      if (symbols.find(*$1) == symbols.end()) {
        symbols.insert(*$1);
        std::cout << "int " << *$1 << " = " << *$3 << ";" << std::endl;
      } else {
        std::cout << *$1 << " = " << *$3 << ";" << std::endl;  
      }
      delete $1; delete $3;
    }
  ;

/*
 * Symbol representing algebraic expressions.  For most forms of algebraic
 * expression, we generate a translated string that simply concatenates the
 * target language translations of the operands with the C++ translation of the operator.
 */
 expression
  : LPAREN expression RPAREN { $$ = new std::string("(" + *$2 + ")"); delete $2; }
  | expression PLUS expression { $$ = new std::string(*$1 + " + " + *$3); delete $1; delete $3; }
  | expression TIMES expression { $$ = new std::string(*$1 + " * " + *$3); delete $1; delete $3; }
  | expression MINUS expression { $$ = new std::string(*$1 + " - * " + *$3); delete $1; delete $3; }
  | expression DIVIDEDBY expression { $$ = new std::string(*$1 + " / " + *$3); delete $1; delete $3; }
  | INTEGER { $$ = $1; }
  | IDENTIFIER { $$ = $1; }
  ;

%%

int main(int argc, char **argv)
{
  yylex();
  return 0;
}

void yyerror(YYLTYPE* loc, const char* err) {
  std::cerr << "Error (line " << loc->first_line << "): " << err << std::endl;
}
