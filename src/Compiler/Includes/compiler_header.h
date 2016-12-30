/* Some code has been taken from:
 "flex & bison" by John R. Levine Copyright 2009 John Levine, 978-0-596-15597-1.
*/

 

/*
 * Declarations for the compiler. 
 * This file is referred by three steps of the compiler: pre-processor, bison, and lexer.
 * The pre-processor and lexer are combined into one flex file that uses two scanners.
*/

#ifndef P4_FRAMEWORK_COMPILER_H
#define P4_FRAMEWORK_COMPILER_H

#include <string.h>
#include "symbol_table.h"


/*
* Since an dependency list needs to be built,  
* some typedefs have to used. 
* 
*/

/* typedefs go here */

typedef struct program_node Program; 
typedef struct stmt_node Stmt_Node; /*a statement node*/
typedef struct dep_node Dep_Node; /* dependency tree node */
typedef struct param_node Param_Node; /* the structure that represents a dependency node -- array in a json file */
typedef struct func_temp Func_JS; /* function that stores some data relationships */


/** The below structures make up the interface between the source file and the JSON file
*   that is created and passed to the next stage of the compiler (placement).
**/
struct param_node {
  char* m_dep_label;  /* dependency label */
  char* m_dep_type;   /* dependency type */
  Param_Node* m_next; /* next param in the dependency list */   

};
 
struct stmt_node {
  int m_index;        /* statement index */
  char* m_data_type;  /* statement data type as a string */
  char* m_func;       /* function performed */
  char* m_label;      /* assignment label -- if not an assignment, NULL */
  Param_Node* m_dep;  /* dependency list */
};

struct func_temp {     
  char* m_title;        /* function name */
  char* m_data_type;   /* string that points to the data type this function works with */
  Param_Node* m_param; /* a list of paramters for a JSOn array */  
};


/*a node in the dependency linked-list*/
struct dep_node {
  Stmt_Node* m_stmt;
  Dep_Node* m_next;
};


/*the starting point*/
struct program_node { 
  char* m_title; /*program title*/
  Dep_Node* m_begin;
};


Func_Arg* newarglist(Symbol* sym, Func_Arg* next);

/* Build a dependency linked-list that is passed to the next stage (placement) */
Func_JS* new_func_type(Symbol* func_name, Symbol* d_type, Func_Arg* arg); /* creates a function structures that stores dat type, name and dependencies */
Func_JS* new_func_no_type(Symbol* func_name, Func_Arg* arg); /* same as above, but this function deducts function type from the params */

Stmt_Node* new_statement(Symbol* label, Func_JS* func); /* creates a JSON interface that stores all the information needed to parse this interface to a JSON object. Also, the returned strurcture can be viewed as a statement/node in a dependency graph. */

void deallocate_dependency_list(Program*); /* deletes the linked list */
void display_dependency_list(const Program* const); /* dsiplays the dependency list */

/* interface to the compiler lexer */
extern int yylineno; /* from lexer */
extern FILE* yyin; /*from lexer*/


#endif



