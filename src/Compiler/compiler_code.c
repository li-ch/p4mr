/*
 * helper functions for compiler
 */
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include <math.h>
# include "compiler_header.h"


void
init_symbol_table ()
{
 unsigned int i;

 for(i = 0; i < NUM_SYMBOLS; i++)
 {
   symbol_table[i] = NULL; // set pointers to NULL
 }

}

/* symbol table */
/* hash a symbol */
static unsigned
symhash(const char * sym)
{
  unsigned int hash = 0;
  unsigned c;
  
  while(c = *sym++) hash = hash*9 ^ c;
  
  return hash;
}

struct symbol *
lookup(const char* const sym)
{
  struct table_node* cur = symbol_table[ symhash(sym) % NUM_SYMBOLS];
  struct table_node* prev;  

  while(cur != NULL) 
  { // traverse the linked list
     if((cur->entry)->name && !strcmp((cur->entry)->name, sym)) { return (cur->entry); }
     prev = cur;
     cur = cur->next; // move further
  }     
 
  // if this code is reached, means no symbol with the same string was found. 
  // create a new entry and append it to the hashed bin. 

    
    cur = malloc(sizeof(struct table_node));    
    if(!cur) 
    {
      yyerror("out of memory");
      exit(0);
    }

    cur->entry = malloc(sizeof(struct symbol));
    if(!(cur->entry)) 
    {
      yyerror("out of memory");
      exit(0);
    }

    (cur->entry)->name = strdup(sym);  
    (cur->entry)->d_type = UINT_8;
    cur->next = NULL; // next node is NULL for termination of the linked list
    if(prev) 
    {  
      prev->next = cur; // update the previous node in the linked list
    }

    return (cur->entry); // return a reference to the newly created symbol
 
}

/*Next come the procedures to build the AST nodes and symlists. They all allocate a node
and then fill in the fields appropriately for the node type. An extended version of
treefree recursively walks an AST and frees all of the nodes in the tree.
*/



struct ast*
newfunctype(struct symbol* sym, enum data_type d_type, struct func_arg* args)
{
  struct func_call* ptr = malloc(sizeof(struct func_call));

   if(!ptr) 
   {
     yyerror("out of memory");
     exit(0);
   }

  ptr->n_type = FUNC_TYPE; /*function with type*/
  sym->d_type = d_type;
  ptr->f_name = sym;
  ptr->sym_list = args;

  return (struct ast *)ptr;

}


struct ast* 
newfuncnotype(struct symbol* sym, struct func_arg* args)
{
  return (newfunctype(sym, (args->sym)->d_type, args));
}


struct ast* 
newasgn(struct symbol* sym, struct ast* exp)
{
 struct asgn* ptr = malloc(sizeof(struct asgn));
 
 if(!ptr)
 {
   yyerror("out of memory");
   exit(0);
 }

  ptr->n_type = ASSM_TYPE; /* assignment type */
  if(exp -> n_type == FUNC_TYPE)
  {
    struct func_call* det_type = (struct func_call* ) exp;
    sym->d_type = det_type->f_name->d_type; /*assign data type from the function call*/
  }
  else
  {
    sym->d_type = ERROR_TYPE;
  }
  
  ptr->lhs = sym;
  ptr->expr = exp;

  return (struct ast *)ptr;

}


/* create and return a new statement list */
struct tree_struct* 
newstmtlist ()
{
  struct tree_struct* stm = malloc(sizeof(struct tree_struct));
  
  if(!stm)
 {
   yyerror("out of memory");
   exit(0);
 }

  stm->sub_tree = NULL;
  stm->next = NULL;

  return stm;
}


/* creates an argument node */
struct func_arg* 
newarglist(struct symbol* pass_sym, struct func_arg* next_arg)
{
  struct func_arg* arg = malloc(sizeof(struct func_arg));
  
  if(!arg)
  {
   yyerror("out of memory");
   exit(0);
  }

  arg->sym = pass_sym;
  arg->next_arg = next_arg;

  return arg; 
}


/* free a symbol */
void
free_symbol(struct symbol* sym)
{
  free(sym->name);
  free(sym);
  sym = NULL;
}



/* free an argument list */
void free_func_arg(struct func_arg* list)
{

  if(!list) /*no arguments*/
  {
    return;
  }
  struct func_arg* temp;
  
  do /* walk the list and release memory */
  {
    temp = list->next_arg;

    free_symbol(list->sym); /*free symbol*/
    free(list); /*free argument node */
    list = temp; 
  }while(temp != NULL);

  /*walk through the list and release memory*/ 
}



/*release a function*/
void
free_func(struct ast* temp)
{
 struct func_call* func = (struct func_call*) temp;
 
 free_symbol(func->f_name); /*release char string*/
 free_func_arg(func->sym_list); /* release list of arguments */

 free(func);
 temp = NULL;
}


void 
free_assign(struct ast* temp)
{
 struct asgn* assignment = (struct asgn*) temp;
 free_symbol(assignment->lhs); /* delete function name */
 free_func(assignment->expr); /* delete the right hand side -- function */
 
 free(assignment); /* free the node itself */
 
 temp = NULL;
}


/* free a tree of ASTs */
void
treefree(struct tree_struct* node)
{
  if(!node) return; 
  
  treefree(node->next); // recursively go to the end of the tree
  
  switch(node->sub_tree->n_type) // check what's the node type (type of the statement branch)
  {
    case ASSM_TYPE : {free_assign(node->sub_tree); break;}
    case FUNC_TYPE : {free_func(node->sub_tree); break;}
 
    default: { printf("internal error: free bad node %c\n", node->sub_tree->n_type);}
 }

 free(node); /* always free the node itself */
 node = NULL;
}



/* -************** Below code is for testing only ***************-*/


/*print at most two parameters*/
char*
print_func_args(const struct func_arg* const arg)
{
  char* arg_sym = (char*)malloc(sizeof(char)*2*7);
  const struct func_arg* ptr = arg;  
  unsigned int scalar = 0;  

  // initialize to zero the arrays
  for(size_t j = 0; j < 14; j++)
  {
    if(j == 6) {arg_sym[j] = ','; continue;}
    arg_sym[j] = '\0';
  }


  while(!ptr && scalar != 2)
  {
    const size_t length = strlen(ptr->sym->name);
    const char* par = ptr->sym->name; 
    for(size_t i = 0; i < length; i++)
    {
      arg_sym[7*scalar* + i] = *(par+i); // read a char 
    }
    
    ptr = ptr->next_arg; 
    scalar++;
  }

  return arg_sym;
}

void print_func(const struct ast* const temp) 
{
  const struct func_call * const stm = (const struct func_call * const) temp;
  char* args = print_func_args(stm->sym_list);
  printf("%s < %d > (%s)", stm->f_name->name, stm->f_name->d_type, args);

  free(args); /* release memory */
}


/*print an assignment*/
void
print_assign(const struct ast* const temp)
{
   const struct asgn* const st = (const struct asgn* const) temp;
   
   printf("%s = ", st->lhs->name);
   print_func(st->expr); // print the function
}


/* print the AST for testing */
void
print_tree (const struct tree_struct* const node)
{
  if(!node) return;
  
  printf("stmt_list -------------- ");
  switch(node->sub_tree->n_type) // check what's the node type (type of the statement branch)
  {
    case ASSM_TYPE : {print_assign(node->sub_tree); break;}
    case FUNC_TYPE : {print_func(node->sub_tree); break;}
 
    default: { printf("internal error: free bad node %c\n", node->sub_tree->n_type);}
  }

  printf("\n"); // print a new line
  print_tree(node->next);
  
}

