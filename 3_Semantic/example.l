%{

#include <stdio.h>
#include <string>
#include "example.tab.h"
#define ERROR 13

using namespace std;

int mylineno = 1;

void yyerror(const char *msg);

%}

%%


[0-9]*             { yylval.s_val = new string(yytext); return NUMBER; }
"+"                { return OP_PL; }
"-"                { return OP_MI; }
"/"                { return OP_DIV; }
"*"                { return OP_MU; }
"^"                { return OP_POW; }
"("		   { return OP_PAR; }
")"		   { return CL_PAR; }
\n                 { mylineno++; return NEW_LINE; }
[ \t]              { /* ignore ws */ }

%%


void yyerror(const char *msg)
{
  printf("// ERROR : %s on line %d at %s\n", msg, mylineno, yytext);

} // yyerror



