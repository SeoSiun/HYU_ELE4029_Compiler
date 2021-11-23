/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

%token IF ELSE WHILE RETURN INT VOID
%token ID NUM 
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI COMMA
%token ERROR 
/* EOF..? */

%% /* Grammar for TINY */

program		: declaration_list
                 		{ savedTree = $1;} 
            		;
declaration_list	: declaration_list declaration
				 { YYSTYPE t = $1;
				   if (t != NULL)
				   { while (t->sibling != NULL)
				        t = t->sibling;
				     t->sibling = $3;
				     $$ = $1; }
				     else $$ = $3;
				 }
            		| declaration  { $$ = $1; }
            		;
declaration		: var_declaration { $$ = $1; }
            		| fun_declaration { $$ = $1; }
            		;
var_declaration     	: type_specifier ID SEMI
				 { $$ = newStmtNode(IfKS);
				   $$->child[0] = $2;
				   $$->child[1] = $4;
				 }
            		| type_specifier ID LBRACE NUM RBRACE SEMI
				 { $$ = newStmtNode(IfK);
				   $$->child[0] = $2;
				   $$->child[1] = $4;
				   $$->child[2] = $6;
				 }
            		;
type_specifier 	: int | void
				 { $$ = newStmtNode(RepeatK);
				   $$->child[0] = $2;
				   $$->child[1] = $4;
				 }
		    	;
fun_declaration 	: type_specifier ID LPAREN params RPAREN compound_stmt
				 { $$ = newStmtNode(AssignK);
				   $$->child[0] = $4;
				   $$->attr.name = savedName;
				   $$->lineno = savedLineNo;
				 }
			;
params   		: param_list
				 { $$ = newStmtNode(ReadK);
				   $$->attr.name =
				     copyString(tokenString);
				 }
			 | void
			 ;
param_list  		: param_list COMMA param
				 { $$ = newStmtNode(WriteK);
				   $$->child[0] = $2;
				 }
		    	| param
		    	;
param         		: type_specifier ID
				 { $$ = newExpNode(OpK);
				   $$->child[0] = $1;
				   $$->child[1] = $3;
				   $$->attr.op = LT;
				 }
            		| type_specifier ID LBRACE RBRACE
				 { $$ = newExpNode(OpK);
				   $$->child[0] = $1;
				   $$->child[1] = $3;
				   $$->attr.op = EQ;
				 }
            		;
compound_stmt  	: LCURLY local_declarations statement_list RCURLY
				 { $$ = newExpNode(OpK);
				   $$->child[0] = $1;
				   $$->child[1] = $3;
				   $$->attr.op = PLUS;
				 }
            		;
local_delarations      : local_declarations var_declarations
				 { $$ = newExpNode(OpK);
				   $$->child[0] = $1;
				   $$->child[1] = $3;
				   $$->attr.op = TIMES;
				 }
            		| empty
				 { $$ = newExpNode(OpK);
				   $$->child[0] = $1;
				   $$->child[1] = $3;
				   $$->attr.op = OVER;
				 }
            		;
statement_list      	: statement_list statement
                 		{ $$ = $2; }
            		| empty
				 { $$ = newExpNode(ConstK);
				   $$->attr.val = atoi(tokenString);
				 }
            		;
statement		: expression_stmt
			| compound_stmt
			| selection_stmt
			| iteration_stmt
			| return_stmt
			;
expression_stmt	: expression SEMI
			| SEMI
			;
selection_stmt		: IF LPAREN

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

