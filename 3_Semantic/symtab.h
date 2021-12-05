/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* SIZE is the size of the hash table */
#define SIZE 211


/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

typedef struct ParamListRec
   { ExpType type;
     struct ParamListRec * next;
   } * ParamList;
 
 /* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
   { char * name;
     // add type info
     ExpType type;
     LineList lines;
     ParamList params;
     int memloc ; /* memory location for variable */
     struct BucketListRec * next;
   } * BucketList;

// add scope info
typedef struct ScopeListRec
   { char * name;
     BucketList bucket[SIZE];
     struct ScopeListRec * parent;
     struct ScopeListRec * next;
   } * Scope;
   
typedef struct ScopeStackRec
   { Scope scope;
     struct ScopeStackRec * next;
     int location;
   } * ScopeStack;

Scope push( char * scope );
ScopeStack top();
void pop();
void addLineno( int lineno, BucketList l);
void addParam ( BucketList l, ExpType type );

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
 // add scope and type info
void st_insert( Scope scope, char * name, ExpType type, int lineno, int loc );

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
BucketList st_lookup ( Scope scope, char * name );
BucketList st_lookup_excluding_parent ( Scope scope, char * name ); 

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);

#endif
