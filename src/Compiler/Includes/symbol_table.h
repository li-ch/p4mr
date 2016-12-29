#ifndef P4_FRAMEWORK_SYMBOL_TABLE_H
#define P4_FRAMEWORK_SYMBOL_TABLE_H

#include "data_types.h"

/* boolean flags used for status checking */
const int SUCCESS = 1;
const int FAILURE = 0;
 

/* A table of symbols represents a structure
 * that keeps track of declared variables (labels).
 *
*/

typedef struct { 
  char* m_name;  /* label of a variable */
  Data_Type m_data; /*type of the variable*/
} Symbol;


typedef struct func_arg Func_Arg;
/*a list of funct arguments*/
struct func_arg {
 
 Symbol* m_id; /*var idenftifier*/
 Func_Arg* m_next; /*next argument*/ 

};


typedef struct symbol_table_node Table_Node;

struct symbol_table_node {
  Symbol* m_entry;
  Table_Node* m_next;
};



/*a fixed-size symbol table*/
#define NUM_LABELS 53
Table_Node* label_table[NUM_LABELS];


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
  int m_par_num; 
  Data_Set* m_data_set;
};

#define NUM_API_FUNC 19
Func_Tab_Node* function_table[NUM_API_FUNC];
  

const Symbol* const lookup(const char* const); /*for checking if a symbol has already been defined.*/
int add_function_API(char*, Data_Set*, const int); /* adds a function declaration/definition in the pre-processing stage*/
int add_label(const Symbol* const); /*add a new reference to the symbol table*/
int correct_func(const char* const, const Func_Arg* const); /* checks if a function has been defined */

void init_tables(); /*initialize all tables*/
void delete_tables(); /* delete the symbol and function tables */



void yyerror(char* s, ...);

#endif

