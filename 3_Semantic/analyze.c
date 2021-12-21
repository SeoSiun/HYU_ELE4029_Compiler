/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* counter for variable memory locations */
//static int location = 0;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}
char* scopeName = "global";

static void popStack(TreeNode * t)
{ if (t->nodekind == StmtK) 
  { if (t->kind.stmt == CompoundK) pop();
    else if (t->kind.stmt == FuncK) scopeName = t->scope->parent->name;
  }
}
    
int isFunction = 0;
/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ ScopeStack topOfStack = top();
  Scope scope = topOfStack->scope;
  BucketList l;

  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompoundK:
          if (!isFunction) t->scope = push(scopeName);
          isFunction = 0;
          break;
        case VarDeclK:
          // not in scope
          if (st_lookup_excluding_parent(scope, t->attr.name) == NULL)
          { // void varable
            if (t->type == Void)
            { fprintf(listing, "Error: Variable Type cannot be void at line %d (name : %s)\n", t->lineno, t->attr.name);
              Error = TRUE;
              st_insert(scope, t->attr.name, SemanticError, t->lineno, topOfStack->location++);
            }
            else st_insert(scope, t->attr.name, Int, t->lineno, topOfStack->location++);
          }
          // already defined
          else
          { fprintf(listing, "Error: redefined symbol '%s' at line %d\n", t->attr.name, t->lineno);
            Error = TRUE;
          }
          t->scope = scope;
          break;
        case VarArrDeclK:
          // not in scope
          if (st_lookup_excluding_parent(scope, t->attr.name) == NULL)
          { // void varable
            if (t->type == Void)
            { fprintf(listing, "Error: Variable Type cannot be void at line %d (name : %s)\n", t->lineno, t->attr.name);
              Error = TRUE;
              st_insert(scope, t->attr.name, SemanticError, t->lineno, topOfStack->location++);
            }
            else st_insert(scope, t->attr.name, IntArr, t->lineno, topOfStack->location++);
          }
          // already defined
          else
          { fprintf(listing, "Error: redefined symbol '%s' at line %d\n", t->attr.name, t->lineno);
            Error = TRUE;
          }
          t->scope = scope;
          break;
        case FuncK:
          // not in scope
          if (st_lookup_excluding_parent(scope, t->attr.name) == NULL)
          { // void funciton
            if (t->type == Void) st_insert(scope, t->attr.name, VoidFunc, t->lineno, topOfStack->location++);
            // int function
            else st_insert(scope, t->attr.name, IntFunc, t->lineno, topOfStack->location++);
          }
          // already defined
          else
          { fprintf(listing, "Error: redefined symbol '%s' at line %d\n", t->attr.name, t->lineno);
            Error = TRUE;
          }
          scopeName = t->attr.name;
          t->scope = push(scopeName);
          isFunction = 1;
          break;
        case ParamK:
          // not in scope
          l = st_lookup_excluding_parent(scope->parent, scope->name);
          if (st_lookup_excluding_parent(scope, t->attr.name) == NULL)
          { // void varable
            if (t->type == Void)
            { fprintf(listing, "Error: Parameter Type cannot be void at line %d (name : %s)\n", t->lineno, t->attr.name);
              Error = TRUE;
              st_insert(scope, t->attr.name, Void, t->lineno, topOfStack->location++);
              addParam(l, Void);
            }
            else
            { st_insert(scope, t->attr.name, Int, t->lineno, topOfStack->location++);
              addParam(l, Int);
            }
          }
          // already defined
          else
          { fprintf(listing, "Error: redefined symbol '%s' at line %d\n", t->attr.name, t->lineno);
            Error = TRUE;
          }
          t->scope = scope;
          break;
        case ParamArrK:
          l = st_lookup_excluding_parent(scope->parent, scope->name);
          // not in scope
          if (st_lookup_excluding_parent(scope, t->attr.name) == NULL)
          { // void varable
            if (t->type == Void)
            { fprintf(listing, "Error: Parameter Type cannot be void[] at line %d (name : %s)\n", t->lineno, t->attr.name);
              Error = TRUE;
              st_insert(scope, t->attr.name, SemanticError, t->lineno, topOfStack->location++);
            }
            else 
            { st_insert(scope, t->attr.name, IntArr, t->lineno, topOfStack->location++);
              addParam(l, IntArr);
            }
          }
          // already defined
          else
          { fprintf(listing, "Error: redefined symbol '%s' at line %d\n", t->attr.name, t->lineno);
            Error = TRUE;
          }
          t->scope = scope;
          break;
        default:
          t->scope = scope;
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case VarK:
        case CallK:
          // if already defined in scope, just add lineno
          if ((l = st_lookup(scope, t->attr.name)) != NULL)
          { addLineno(t->lineno, l);
            t->scope = scope;
          }
          break;
        default:
          t->scope = scope;
          break;
      }
      break;
    default:
      break;
  }
}

// initialize scope stack (insert input, output to global)
static void init()
{ Scope global = push(scopeName);
  ScopeStack topOfStack = top();
  
  st_insert(global, "input", IntFunc, 0, topOfStack->location++);
  st_insert(global, "output", VoidFunc, 0, topOfStack->location++);
  
  Scope output = push("output");
  topOfStack = top();
  st_insert(output, "value", Int, 0, topOfStack->location++);
  addParam(st_lookup(global,"output"),Int);
  pop();
}
  
/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ init();
  traverse(syntaxTree,insertNode,popStack);
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Error: Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ BucketList l;
  char* funcName;
  switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          // child's type must be int
          if ( t->child[0]->type != Int || t->child[1]->type != Int )
            typeError(t, "invalid expression");
          else t->type = Int;
          break;
        case ConstK:
          t->type = Int;
          break;
        case VarK:
          if ( (l = st_lookup(t->scope, t->attr.name))==NULL || l->lines->lineno > t->lineno )
          { fprintf(listing, "Error: Undeclared Variable '%s' at line %d\n", t->attr.name, t->lineno);
            Error = TRUE;
            t->type = SemanticError;
          }
          else
          { if (t->child[0] && t->child[0]->type != Int)
            { fprintf(listing, "Error: Invalid array indexing at line %d (name: '%s'). Indices should be integer\n", t->lineno, t->attr.name);
              Error = TRUE;
              t->type = SemanticError;
            }
            else if (!t->child[0]) t->type = l->type;
            else t->type = Int;
          }
          break;
        case AssignK:
          if ( (l = st_lookup(t->child[0]->scope, t->child[0]->attr.name))==NULL || l->lines->lineno > t->child[0]->lineno )
          { fprintf(listing, "Error: Undeclared Variable '%s' at line %d\n", t->attr.name, t->lineno);
            Error = TRUE;
            t->type = SemanticError;
          }
          else if ( t->child[0]->type != t->child[1]->type && t->child[0]->type != SemanticError && t->child[1]->type != SemanticError )
          { fprintf(listing,"Error: Assginment type error at line %d (name: '%s')\n", t->lineno, t->child[0]->attr.name);
            Error = TRUE;
            t->type = SemanticError;
          }
          else t->type = t->child[0]->type;
          break;
        case CallK:
          if ( (l = st_lookup(t->scope, t->attr.name))==NULL || l->lines->lineno > t->lineno )
          { fprintf(listing, "Error: Undeclared Function '%s' at line %d\n", t->attr.name, t->lineno);
            Error = TRUE;
          }
          else if ( l->type != IntFunc && l->type != VoidFunc ) 
          { fprintf(listing,"Error: invalid function call at line %d (name: '%s')\n",t->lineno, t->attr.name);
            Error = TRUE;
          }
          else
          { TreeNode * arg = t->child[0];
            ParamList param = l->params;

            while (arg && param)
            { if (arg->type != param->type) break;
              arg = arg->sibling;
              param = param->next;
            } 
            if (arg || param)
            { fprintf(listing,"Error: Parameter error at line %d: invalid function call (name: '%s')\n",t->lineno, t->attr.name);
              Error = TRUE;
            }
    
            if (l->type == IntFunc) t->type = Int;
            else t->type = Void;
          }
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
        case IfElseK:
        case WhileK:
          if (t->child[0]->type != Int) 
          { fprintf(listing,"Error: Invalid condition at line %d\n", t->lineno);
            Error = TRUE;
          }
          break;
        case ReturnK:
          funcName = t->scope->name;
          l = st_lookup(t->scope->parent,funcName);
          if (l->type == VoidFunc)
            typeError(t, "invalid return type");
          else if (l->type == IntFunc && (t->child[0] == NULL || t->child[0]->type != Int))
            typeError(t, "invalid return type");
          break;
        case NonValueReturnK:
          funcName = t->scope->name;
          l = st_lookup(t->scope->parent,funcName);
          if (l->type != VoidFunc)
            typeError(t, "invalid return type");
          break;
        default:
          break;
      }
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
