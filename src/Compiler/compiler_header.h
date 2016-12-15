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
typedef enum {
 UINT_8 = 1008,
 INT_8,
 UINT_16, 
 INT_16, 
 UINT_32, 
 INT_32, 
 UINT_64, 
 INT_64,
 PATH_STRING, /*IP string*/
 ERROR_DATA = -1
} Data_Type;


typedef enum {
 ASSIGN_TYPE, 
 FUNC_TYPE, 
 SYMBOL_TYPE, 
 ERROR_TYPE
} Node_Type; 


/* A table of symbols represents a structure
 * that keeps track of declared variables (lables) 
 * and functions.
 *
*/

typedef struct{
  char* m_name;  /* variable/function name */
  Data_Type m_type = ERROR_DATA; /*type of the variable*/
  unsigned int m_par_number; /*number of parameters a function takes -- for semantic checking phase*/
} Symbol;


typedef struct{
  Symbol* m_entry;
  Table_Node* m_next;
}Table_Node;


/*a fixed-size symbol table*/
void init_symbol_table();
#define NUM_SYMBOLS 3000
struct Table_Node* symbol_table[NUM_SYMBOLS];

Symbol* lookup(const char* const); /*for checking if a symbol has already been defined.*/


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

/*Assignment*/
typedef struct {
  
  Ast* m_left; 
  Ast* m_right;
} Assign_Node;


/*a list of funct arguments*/
typedef struct {
 
 Symbol* m_id; /*var idenftifier*/
 Func_Arg* m_next; /*next argument*/ 

} Func_Arg;

/*Function Call*/
typedef struct {
 
 Symbol* m_name; /*function name*/
 Func_Arg* m_arg; /*function arguments*/
 
} Func_Node;

typedef struct {
  Node_Type m_type; /*  node type */
  
  union{ 
       Symbol* m_var; /*a variable used within a tree*/
       Assign_Node* m_assign; /*assignmnet*/
       Func_Node* m_func; /*assignment*/
  } m_op; /*operation*/

} Ast;
 

/*structure for the top level of an AST*/
typedef struct {
  Ast* m_node;
  Tree* m_next;
} Tree;

/* build an AST */
Ast* newfunctype(Symbol* sym, Data_Type d_type, Func_Arg* args);
Ast* newfuncnotype(Symbol* sym, Func_Arg* args);
Ast* newassign(Symbol* sym, Ast* exp);


void treefree(Tree* root);

Func_Arg* newarglist(Symbol* sym, Func_Arg* next);
void print_tree(const Tree* const root) ;


/* interface to the compiler lexer */
extern int yylineno; /* from lexer */
extern FILE* yyin; /*from lexer*/
void yyerror(char* s, ...);

#endif



