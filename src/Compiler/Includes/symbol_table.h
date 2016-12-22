#ifndef P4_FRAMEWORK_SYMBOL_TABLE_H
#define P4_FRAMEWORK_SYMBOL_TABLE_H

#include "data_types.h"


/* A table of symbols represents a structure
 * that keeps track of declared variables (lables) 
 * and functions.
 *
*/

typedef struct { 
  char* m_name;  /* variable/function name */
  Data_Type m_data; /*type of the variable*/
} Symbol;


typedef struct symbol_table_node Table_Node;

struct symbol_table_node {
  Symbol* m_entry;
  Table_Node* m_next;
};



/*a fixed-size symbol table*/
void init_tables();
#define NUM_SYMBOLS 300
Table_Node* symbol_table[NUM_SYMBOLS];


/************************************ Below functions for function definitions *************************************/
typedef struct data_set Data_Set; 

/* this structure is used for storing the set of possible data type that a function supports */
struct data_set {
   Data_Type m_dtype;
   Data_Set* m_next;
}; 


typedef struct table_func_definition Func_Tab_Node;

struct table_func_definition {
  char* m_key;
  Func_Tab_Node* m_next;
  unsigned int m_par_num; 
  Data_Set* m_data_set;
};

#define NUM_API_FUNC 300
Func_Tab_Node* function_table[NUM_API_FUNC];
  

const Symbol* lookup(const char* const); /*for checking if a symbol has already been defined.*/
void add_function_API(char*, Data_Set*, const unsigned int); /* adds a function declaration/definition in the pre-processing stage*/
void add_symbol(const Symbol* const); /*add a new reference to the symbol table*/
void delete_tables(); /* delete the symbol and function tables */



void yyerror(char* s, ...);

#endif

