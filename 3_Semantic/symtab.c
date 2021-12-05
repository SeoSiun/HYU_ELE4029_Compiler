/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

 void addParam ( BucketList l, ExpType type )
 { if (!l->params)
   { l->params = (ParamList) malloc(sizeof(struct ParamListRec));
     l->params->type = type;
     l->params->next = NULL;
   }
   else
   { ParamList tmp = l->params;
     while (tmp->next) tmp = tmp->next;
     tmp->next = (ParamList) malloc(sizeof(struct ParamListRec));
     tmp->next->type = type;
     tmp->next->next = NULL;
   }
 }

Scope scopeList = NULL;
ScopeStack scopeStack = NULL;

Scope push ( char * scope )
{ if (!scopeStack)
  { scopeStack = (ScopeStack)malloc(sizeof(struct ScopeStackRec));
    memset(scopeStack, 0, sizeof(struct ScopeStackRec));
  } 
  Scope s = (Scope)malloc(sizeof(struct ScopeListRec));
  memset(s, 0, sizeof(struct ScopeListRec));
  
  s->name = scope;
  
  if (scopeList)
  { Scope iter = scopeList;
    while(iter->next) iter = iter->next;
    iter->next = s;
  }
  else scopeList = s;
  
  ScopeStack topOfStack = top();
  if (topOfStack) s->parent = topOfStack->scope;
  
  ScopeStack tmp = (ScopeStack)malloc(sizeof(struct ScopeStackRec));
  tmp->scope = s;
  tmp->next = scopeStack->next;
  tmp->location = 0;
  scopeStack->next = tmp;
  
  return s;
}

ScopeStack top()
{ if (!scopeStack)
  { scopeStack = (ScopeStack)malloc(sizeof(struct ScopeStackRec));
    memset(scopeStack, 0, sizeof(struct ScopeStackRec));
  } 
  return scopeStack->next;
}

void pop()
{ if (scopeStack && scopeStack->next) 
    scopeStack->next = scopeStack->next->next;
}

void addLineno( int lineno, BucketList l)
{ LineList t = l->lines;
  while (t->next != NULL) t = t->next;
  t->next = (LineList) malloc(sizeof(struct LineListRec));
  t->next->lineno = lineno;
  t->next->next = NULL;
}  

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( Scope scope, char * name, ExpType type, int lineno, int loc )
{ int h = hash(name);
  BucketList l =  scope->bucket[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    // save type info
    l->type = type;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->params = NULL;
    l->lines->next = NULL;
    l->next = scope->bucket[h];
    scope->bucket[h] = l; }
  else /* found in table, so just add line number */
    addLineno( lineno, l);
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
BucketList st_lookup ( Scope scope, char * name )
{ while (scope)
  { int h = hash(name);
    BucketList l =  scope->bucket[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;
    if (l == NULL) scope = scope->parent;
    else return l;
  }
  return NULL;
}

BucketList st_lookup_excluding_parent ( Scope scope, char * name )
{ int h = hash(name);
  BucketList l =  scope->bucket[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  return l;
}

// Void,Int,IntArr,VoidFunc,IntFunc,SemanticError
char* expType[] = {"Void", "Integer", "Integer Array", "Function", "Function"};
/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  Scope iter = scopeList;
  fprintf(listing,"Variable Name  Variable Type  Scope Name  Location   Line Numbers\n");
  fprintf(listing,"-------------  -------------  ----------  --------   ------------\n");
  
  while (iter)
  { for (i=0;i<SIZE;++i)
    { if (iter->bucket[i] != NULL)
      { BucketList l = iter->bucket[i];
        while (l != NULL)
        { LineList t = l->lines;
          fprintf(listing,"%-13s  ",l->name);
          fprintf(listing,"%-13s  ",expType[l->type]);
          fprintf(listing,"%-10s  ",iter->name);
          fprintf(listing,"%-8d  ",l->memloc);
          while (t != NULL)
          { fprintf(listing,"%4d ",t->lineno);
            t = t->next;
          }
          fprintf(listing,"\n");
          l = l->next;
        }
      }
    }
    iter = iter->next;
  }
} /* printSymTab */
