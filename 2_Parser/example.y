%{
#include <iostream>
#include <stack>
#include <string>
#include <stdio.h>
#include <stdlib.h>

  using namespace std;

  extern int yylex();
  extern void yyerror(const char*);
  extern int yyparse();

  string prog_string;

typedef union {
  int i_val;     /*don't need this for proj 1.*/
  std::string* s_val;
} YYSTYPE_template;

#define YYSTYPE YYSTYPE_template

 extern YYSTYPE yyval;

#define YYSTYPE_IS_DECLARED 1

%}

%union {
  std::string* s_val;
}

%token NUMBER
%token OP_PL
%token OP_MU
%token OP_DIV
%token OP_MI
%token OP_POW
%token OP_PAR
%token CL_PAR
%token NEW_LINE

%left OP_PL OP_MI
%left OP_MU OP_DIV
%left OP_POW

%type <s_val> line
%type <s_val> exp
%type <s_val> NUMBER

%%

program:	{ }
	| line program {}
	;

line: NEW_LINE 	{}
       | exp NEW_LINE      { cout << *$1 << endl; }
       ;

exp: exp OP_MU exp              { $$ = new string("* " + *$1 + " " + *$3); }
	| exp OP_MI exp              { $$ = new string("- " + *$1 + " " + *$3); }
	| exp OP_DIV exp              { $$ = new string("/ " + *$1 + " " + *$3); }
	| exp OP_PL exp              { $$ = new string("+ " + *$1 + " " + *$3); }
	| exp OP_POW exp              { $$ = new string("^ " + *$1 + " " + *$3); }
	| OP_PAR exp CL_PAR	    { $$ = $2; }
	| NUMBER		    { $$ = $1; }
       ;

%%

int main()
{
  prog_string = "";
  yyparse();
  cout<<prog_string;

  return 1;
}
