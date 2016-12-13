/* Some code has been taken from:
 "flex & bison" by John R. Levine Copyright 2009 John Levine, 978-0-596-15597-1.
*/



/*
 * Declarations for the compiler. 
 * This file is referred by three steps of the compiler: pre-processor, bison, and lexer.
 * The pre-processor and lexer are combined into one flex file that uses two scanners.
*/


enum data_type {
 UINT_8 = 1008,
 INT_8,
 UINT_16, 
 INT_16, 
 UINT_32, 
 INT_32, 
 UINT_64, 
 INT_64,
 PATH_STRING, /*IP string*/
 ERROR_TYPE = -1
};


enum node_type {
 ASSM_TYPE, /* assignment */
 FUNC_TYPE, /* function with data type */ 
}





/* A table of symbols represents a structure
 * that keeps track of declared variables (lables) 
 * and functions.
 *
*/

struct symbol{
  char* name;  /* variable/function name */
  enum data_type d_type; /*type of the variable*/
};

struct table_node {

  struct symbol* entry;
  struct table_node* next;
};


/*a fixed-size symbol table*/
void init_symbol_table();
#define NUM_SYMBOLS 3000
struct table_node* symbol_table[NUM_SYMBOLS];

struct symbol* lookup(const char* const); /*for checking if a symbol has already been defined.*/


/*list of symbols, for an argument list*/
struct func_arg {
  struct symbol* sym;
  struct func_arg* next_arg;
};



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


struct ast{
  enum node_type type; /*  node type */
};

// controls the structure of an AST
struct tree_struct {
  struct ast* sub_tree; // for parsing and placement -- represents a statement
  struct tree_struct* next; // next node (statement)
}; 


struct func_call { /* user-defined functions (APIs)*/
  enum node_type n_type; /* node type*/
  struct symbol* f_name; /*function name*/
  struct func_arg* sym_list; /* list of arguments */
};


struct asgn {
  enum node_type n_type; /*  node type */
  struct symbol* lhs; /*symbol*/
  struct ast* expr; /* expression */
};


/* build an AST */
struct ast* newfunctype(struct symbol* sym, enum data_type d_type, struct func_arg* args);
struct ast* newfuncnotype(struct symbol* sym, struct func_arg* args);
struct ast* newasgn(struct symbol* sym, struct ast *exp);
struct tree_struct* newstmtlist ();


void treefree(struct tree_struct* root);

struct func_arg* newarglist(struct symbol* sym, struct func_arg* next);
void print_tree(const struct tree_struct* const root) const;


/* interface to the compiler lexer */
extern int yylineno; /* from lexer */
void yyerror(char* s, ...);
enum data_type get_data_type(const char*);





