#ifndef P4_FRAMEWORK_SYMBOL_TABLE_H
#define P4_FRAMEWORK_SYMBOL_TABLE_H

#include "data_types.h"


/* A table of symbols represents a structure
 * that keeps track of declared variables (lables) 
 * and functions.
 *
*/

typedef struct{ 
  char* m_name;  /* variable/function name */
  Data_Type m_data; /*type of the variable*/
  unsigned int m_par_number; /*number of parameters a function takes -- for semantic checking phase*/
} Symbol;


typedef struct Table_Node Table_Node;

struct Table_Node{
  Symbol* m_entry;
  Table_Node* m_next;
};


/*a fixed-size symbol table*/
void init_symbol_table();
#define NUM_SYMBOLS 3000
Table_Node* symbol_table[NUM_SYMBOLS];

const Symbol* lookup(const char* const); /*for checking if a symbol has already been defined.*/
void add_symbol(const Symbol* const); /*add a new reference to the symbol table*/
void delete_symbol_table(); /* delete symbol table */

#endif

