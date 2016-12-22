/* Some code has been taken from:
 "flex & bison" by John R. Levine Copyright 2009 John Levine, 978-0-596-15597-1.
*/



/*
 * Declarations for the compiler. 
 * This file is referred by three steps of the compiler: pre-processor, bison, and lexer.
 * The pre-processor and lexer are combined into one flex file that uses two scanners.
*/

#ifndef COMPILER_TEST_H
#define COMPILER_TEST_H

#include <string.h>
#include "symbol_table.h"



typedef enum {
 ASSIGN_TYPE, 
 FUNC_TYPE, 
 SYMBOL_TYPE, 
 ERROR_TYPE
} Node_Type; 



/* 
*  A node of AST.
*  If only one child is used, 
*  it has to be the left one - the right one NULL.
*/


/*
* Since an AST needs to be built,  
* the AST might contain one of the following nodes:
*
* function;
* function argument (label);
* assignment; 
* ...
*/

/* typedefs go here */
typedef struct Assign_Node Assign_Node;
typedef struct Ast Ast;
typedef struct Func_Node Func_Node;
typedef struct Func_Arg Func_Arg; 
typedef struct Tree Tree;
typedef struct Program Program;

/*Assignment*/
struct Assign_Node {
  
  Ast* m_left; 
  Ast* m_right;
};


/*a list of funct arguments*/
struct Func_Arg {
 
 Symbol* m_id; /*var idenftifier*/
 Func_Arg* m_next; /*next argument*/ 

};

/*Function Call*/
struct Func_Node {
 
 Symbol* m_id; /*function identifier*/
 Func_Arg* m_arg; /*function arguments*/
 
};

struct Ast {
  Node_Type m_type; /*  node type */
  
  union{ 
       Symbol* m_var; /*a variable used within a tree*/
       Assign_Node* m_assign; /*assignmnet*/
       Func_Node* m_func; /*assignment*/
  } m_op; /*operation*/

};
 

/*structure for the top level of an AST*/
struct Tree {
  Ast* m_node;
  Tree* m_next;
};


/*the starting point*/
struct Program {
  char* m_title; /*program title*/
  Tree* m_begin;
};

/* build an AST */
Ast* newfunctype(const Symbol* const sym, Data_Type d_type, Func_Arg* args);
Ast* newfuncnotype(const Symbol* const sym, Func_Arg* args);
Ast* newassign(const Symbol* const sym, Ast* exp);
Program* new_program(Tree* begin);


void deallocate_tree(Program* root);

Func_Arg* newarglist(const Symbol* const sym, Func_Arg* next);
void print_program(const Program* const root) ;


/* interface to the compiler lexer */
extern int yylineno; /* from lexer */
extern FILE* yyin; /*from lexer*/


#endif



