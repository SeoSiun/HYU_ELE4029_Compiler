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
#define MAX_STACK 10
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

// to save name, type, num, lineno
// use stack to solve overwrite problem
char* nameStack[MAX_STACK];
int numStack[MAX_STACK];
int linenoStack[MAX_STACK];
ExpType typeStack[MAX_STACK];

int nameIndex = 0;
int numIndex = 0;
int linenoIndex = 0;
int typeIndex = 0;

void pushName(char* name) { nameStack[nameIndex++] = name; }
void pushNum(int num) { numStack[numIndex++] = num; }
void pushLineno(int lineno) { linenoStack[linenoIndex++] = lineno; }
void pushType(ExpType type) { typeStack[typeIndex++] = type; }

char* popName() { return nameStack[--nameIndex]; }
int popNum() { return numStack[--numIndex]; }
int popLineno() { return linenoStack[--linenoIndex]; }
ExpType popType() { return typeStack[--typeIndex]; }

%}

%token IF ELSE WHILE RETURN INT VOID
%token ID NUM 
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI COMMA
%token ERROR 

/* to solve shift/reduce conflict */
%nonassoc RPAREN
%nonassoc ELSE

%% /* Grammar for C-MINUS */

program		: declaration_list
                 		{ savedTree = $1;} 
            		;
id		: ID
				{ 
				  pushName(copyString(tokenString));
				  pushLineno(lineno);
				}
			;
num		: NUM
				{ 
				  pushNum(atoi(tokenString));
				  pushLineno(lineno);
				}
			;
declaration_list	: declaration_list declaration
				 { YYSTYPE t = $1;
				   if (t != NULL){
				     while (t->sibling != NULL)
				        t = t->sibling;
				     t->sibling = $2;
				     $$ = $1; 
				   }
				   else $$ = $2;
				 }
            		| declaration  { $$ = $1; }
            		;
declaration		: var_declaration { $$ = $1; }
            		| fun_declaration { $$ = $1; }
            		;
var_declaration     	: type_specifier id SEMI
				 { $$ = newStmtNode(VarDeclK);
				   $$->type = popType();
				   $$->attr.name = popName();
				   $$->lineno = popLineno();
				 }
            		| type_specifier id LBRACE num RBRACE SEMI
				 { $$ = newStmtNode(VarArrDeclK);
				   $$->type = popType();
				   $$->attr.name = popName();
				   $$->lineno = popLineno();
				   
				   // add array size to child
				   $$->child[0] = newExpNode(ConstK);
				   $$->child[0]->attr.val = popNum();
				   $$->child[0]->lineno = popLineno();
				 }
            		;
type_specifier 	: INT { pushType(Int); }
			| VOID { pushType(Void); }
		    	;
fun_declaration 	: type_specifier id LPAREN params RPAREN compound_stmt
				 { $$ = newStmtNode(FuncK);
				   $$->child[0] = $4;
				   $$->child[1] = $6;
				   $$->attr.name = popName();
				   $$->type = popType();
				   $$->lineno = popLineno();
				 }
			;
params   		: param_list { $$ = $1; }
			 | VOID 
			 	{ $$ = newStmtNode(VoidParamK);
			 	  $$->lineno = lineno; 
			 	}
			 ;
param_list  		: param_list COMMA param
				 { YYSTYPE t = $1;
				   if (t != NULL){ 
				     while (t->sibling != NULL)
				        t = t->sibling;
				     t->sibling = $3;
				     $$ = $1; 
				   }
				   else $$ = $3;
				 }
		    	| param { $$ = $1; }
		    	;
param         		: type_specifier id
				 { $$ = newStmtNode(ParamK);
				   $$->attr.name = popName();
				   $$->type = popType();
				   $$->lineno = popLineno();
				 }
            		| type_specifier id LBRACE RBRACE
				 { $$ = newStmtNode(ParamArrK);
				   $$->attr.name = popName();
				   $$->type = popType();
				   $$->lineno = popLineno();
				 }
            		;
compound_stmt  	: LCURLY local_declarations statement_list RCURLY
				 { $$ = newStmtNode(CompoundK);
				   $$->child[0] = $2;
				   $$->child[1] = $3;
				 }
            		;
local_declarations      : local_declarations var_declaration
				 { YYSTYPE t = $1;
				   if (t != NULL){ 
				     while (t->sibling != NULL)
				        t = t->sibling;
				     t->sibling = $2;
				     $$ = $1; 
				   }
				   else $$ = $2;
				 }
            		| { $$ = NULL; }
            		;
statement_list      	: statement_list statement
                 		{ YYSTYPE t = $1;
				   if (t != NULL){ 
				     while (t->sibling != NULL)
				        t = t->sibling;
				     t->sibling = $2;
				     $$ = $1; 
				   }
				   else $$ = $2;
				 }
            		| { $$ = NULL; }
            		;
statement		: expression_stmt { $$ = $1; }
			| compound_stmt { $$ = $1; }
			| selection_stmt { $$ = $1; }
			| iteration_stmt { $$ = $1; }
			| return_stmt { $$ = $1; }
			;
expression_stmt	: expression SEMI { $$ = $1;}
			| SEMI { $$ = NULL; }
			;
selection_stmt		: IF LPAREN expression RPAREN statement
				{ $$ = newStmtNode(IfK);
				  $$->child[0] = $3;
				  $$->child[1] = $5;
				}
			| IF LPAREN expression RPAREN statement ELSE statement
				{ $$ = newStmtNode(IfElseK);
				  $$->child[0] = $3;
				  $$->child[1] = $5;
				  $$->child[2] = $7;
				}
			;
iteration_stmt		: WHILE LPAREN expression RPAREN statement
				{ $$ = newStmtNode(WhileK);
				  $$->child[0] = $3;
				  $$->child[1] = $5;
				}
			;
return_stmt		: RETURN SEMI 
				{ $$ = newStmtNode(NonValueReturnK);
				  $$->lineno = lineno;
				}
			| RETURN expression SEMI
				{ $$ = newStmtNode(ReturnK);
				  $$->child[0] = $2;
				}
			;
expression		: var ASSIGN expression
				{ $$ = newExpNode(AssignK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				}
			| simple_expression { $$ = $1; }
			;
var			: id
				{ $$ = newExpNode(VarK);
				  $$->attr.name = popName();
				  $$->lineno = popLineno();
				}
			| id LBRACE expression RBRACE
				{ $$ = newExpNode(VarK);
				  $$->attr.name = popName();
				  $$->lineno = popLineno();
				  $$->child[0] = $3;
				}
			;
simple_expression	: additive_expression LE additive_expression
				{ $$ = newExpNode(OpK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				  $$->attr.op = LE;
				}
			| additive_expression LT additive_expression
				{ $$ = newExpNode(OpK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				  $$->attr.op = LT;
				}
			| additive_expression GT additive_expression
				{ $$ = newExpNode(OpK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				  $$->attr.op = GT;
				}
			| additive_expression GE additive_expression
				{ $$ = newExpNode(OpK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				  $$->attr.op = GE;
				}
			| additive_expression EQ additive_expression
				{ $$ = newExpNode(OpK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				  $$->attr.op = EQ;
				}
			| additive_expression NE additive_expression
				{ $$ = newExpNode(OpK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				  $$->attr.op = NE;
				}
			| additive_expression { $$ = $1; }
			;
additive_expression	: additive_expression PLUS term
				{ $$ = newExpNode(OpK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				  $$->attr.op = PLUS;
				}
			| additive_expression MINUS term
				{ $$ = newExpNode(OpK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				  $$->attr.op = MINUS;
				}
			| term { $$ = $1; }
			;
term			: term TIMES factor
				{ $$ = newExpNode(OpK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				  $$->attr.op = TIMES;
				}
			| term OVER factor
				{ $$ = newExpNode(OpK);
				  $$->child[0] = $1;
				  $$->child[1] = $3;
				  $$->attr.op = OVER;
				}
			| factor { $$ = $1; }
			;
factor			: LPAREN expression RPAREN { $$ = $2; }
			| var { $$ = $1; }
			| call { $$ = $1; }
			| num
				{ $$ = newExpNode(ConstK);
				  $$->attr.val = popNum();
				  $$->lineno = popLineno();
				}
			;
call			: id LPAREN args RPAREN
				{ $$ = newExpNode(CallK);
				  $$->child[0] = $3;
				  $$->attr.name = popName();
				  $$->lineno = popLineno();
				}
			;
args			: arg_list { $$ = $1; }
			| { $$ = NULL; }
			;
arg_list		: arg_list COMMA expression
				{ YYSTYPE t = $1;
				   if (t != NULL){ 
				     while (t->sibling != NULL)
				        t = t->sibling;
				     t->sibling = $3;
				     $$ = $1; 
				   }
				   else $$ = $3;
				 }
			| expression { $$ = $1; }	
			;
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

