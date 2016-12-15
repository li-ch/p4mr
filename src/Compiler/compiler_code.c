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

Symbol*
lookup(const char* const sym)
{
  Table_Node* cur = symbol_table[ symhash(sym) % NUM_SYMBOLS];
  Table_Node* prev;  

  while(cur != NULL) 
  { // traverse the linked list
     if((cur->m_entry)->m_name && !strcmp((cur->m_entry)->m_name, sym)) { return (cur->m_entry); }
     prev = cur;
     cur = cur->m_next; // move further
  }     
 
  // if this code is reached, means no symbol with the same string was found. 
  // create a new entry and append it to the hashed bin. 

    
    cur = malloc(sizeof(Table_Node));    
    if(!cur) 
    {
      yyerror("out of memory");
      exit(0);
    }

    cur->m_entry = malloc(sizeof(Symbol));
    if(!(cur->m_entry)) 
    {
      yyerror("out of memory");
      exit(0);
    }

    (cur->m_entry)->m_name = strdup(sym);  
    (cur->m_entry)->m_type = UINT_8;
    cur->m_next = NULL; // next node is NULL for termination of the linked list
    if(prev) 
    {  
      prev->m_next = cur; // update the previous node in the linked list
    }

    return (cur->m_entry); // return a reference to the newly created symbol
 
}

/*Next come the procedures to build the AST nodes and symlists. They all allocate a node
and then fill in the fields appropriately for the node type. An extended version of
treefree recursively walks an AST and frees all of the nodes in the tree.
*/



Ast*
newfunctype(Symbol* sym, Data_Type d_type, Func_Arg* args)
{

  Ast* ptr = malloc(sizeof(Ast)); /*create an AST node*/
  Func_Node* data = malloc(sizeof(Func_Node)); /*Create a Func_Npde*/

   if(!ptr || !data) 
   {
     yyerror("out of memory");
     exit(0);
   }

  ptr->m_type = FUNC_TYPE; /*function with type*/
  
  data->m_id = sym;
  data->m_id->m_type = d_type; // set a data type
  data->m_arg = args;

  (ptr->m_op).m_func = data; /*point to the newly created function node*/

  return ptr; /*returns a pointer to the created AST node*/

}


Ast* 
newfuncnotype(Symbol* sym, Func_Arg* args)
{
  return (newfunctype(sym, (args->m_id)->m_type, args));
}


Ast* 
newassign(Symbol* sym, Ast* exp)
{
 Ast* ptr = malloc(sizeof(Ast)); /*assignment node*/
 Ast* left = malloc(sizeof(Ast)); /*left branch of the assignment node -- symbol */
 Ast* right = malloc(sizeof(Ast)); /*right side of the assignment node -- expression */
 Assign_Node* data = malloc(sizeof(Assign_Node)); 
 

 if(!ptr || !data || !left || !right)
 {
   yyerror("out of memory");
   exit(0);
 }

  ptr->m_type = ASSIGN_TYPE; /* assignment type */

  if(exp -> m_type == FUNC_TYPE) /*means can assign the symbol to the expression*/
  {
    /*handle the left-hand side first*/
    left->m_type = SYMBOL_TYPE;
    (left->m_op).m_var = sym; 
    (left->m_op).m_var->m_type = (exp->m_op).m_func->m_id->m_type; // assign the data type to the variable taken from the function
    
    /*handle the right hand side first*/
    right->m_type = FUNC_TYPE;
    (right->m_op).m_func = (Func_Node*) exp;
    
  }
  else
  {
    /*there is a compilation error, don't need to assign anything except an error signal*/
    left->m_type = ERROR_TYPE;
    (left->m_op).m_var = NULL;
    (left->m_op).m_func = NULL;
    (left->m_op).m_assign = NULL;    

    right->m_type = ERROR_TYPE;
    (right->m_op).m_var = NULL;
    (right->m_op).m_func = NULL;
    (right->m_op).m_assign = NULL;
  }
  
  /*finally, set up the final pointers*/
  data->m_left = left;
  data->m_right = right;
  
  (ptr->m_op).m_assign = data;

  return ptr;

}



/* creates an argument node */
Func_Arg* 
newarglist(Symbol* pass_sym, Func_Arg* next_arg)
{
  Func_Arg* arg = malloc(sizeof(Func_Arg));
  
  if(!arg)
  {
   yyerror("out of memory");
   exit(0);
  }

  arg->m_id = pass_sym;
  arg->m_next = next_arg;

  return arg; 
}


/* free a symbol */
static void
free_symbol(Symbol* sym)
{
  free(sym->m_name);
  free(sym);
  sym = NULL;
}



/* free an argument list */
static void 
free_func_arg(Func_Arg* list)
{

  if(!list) /*no arguments*/
  {
    return;
  }
  Func_Arg* temp;
  
  do /* walk the list and release memory */
  {
    temp = list->m_next;

    free_symbol(list->m_id); /*free symbol*/
    free(list); /*free argument node */
    list = temp; 
  }while(temp != NULL);

  /*walk through the list and release memory*/ 
}



/*release a function*/
static void
free_func(Func_Node* node)
{
 
 free_symbol(node->m_id); /*release char string*/
 free_func_arg(node->m_arg); /* release list of arguments */
 free(node); /*release memory allocated for the function node*/
 node = NULL;
}


static void 
free_assign(Assign_Node* node)
{
 /*release the left-hand side first*/
 if(!node->m_left || !node->m_right)
 {
   yyerror("Error with an assignment");
   exit(0);
 }

 if((node->m_left)->m_type == SYMBOL_TYPE)
 { 
   free_symbol((node->m_left->m_op).m_var); /* the node has a pointer to a symbol*/
   (node->m_left->m_op).m_var = NULL;
 } 
 
 if((node->m_left)->m_type == FUNC_TYPE)
 {
   free_func((node->m_right->m_op).m_func); /* must be a function node */
   (node->m_right->m_op).m_func = NULL;
 }
 
 /*finally, release the nodes themselves*/
 free(node->m_left);
 free(node->m_right);
 node->m_left = NULL;
 node->m_right = NULL;

 free(node);
 node = NULL;

}


/* free a tree of ASTs */
void
treefree(Tree* node)
{
  if(!node) return; 
  
  treefree(node->m_next); // recursively go to the end of the tree
  
  switch((node->m_node)->m_type) // check what's the node type (type of the statement branch)
  {
    case ASSIGN_TYPE : {free_assign((node->m_node->m_op).m_assign); break;}
    case FUNC_TYPE   : {free_func((node->m_node->m_op).m_func); break;}
    case SYMBOL_TYPE : {free_symbol((node->m_node->m_op).m_var); break;}
 
    default: { printf("internal error: free bad node %c\n", node->m_node->m_type); exit(0);}
 }
 
 /*free allocated memory for this node and set the pointers to NULL*/
 free(node->m_node);
 node->m_node = NULL;
 node->m_next = NULL;

 free(node); /* always free the node itself */
 node = NULL;
}


void yyerror(char *errmsg, ...)
 {
   fprintf(stderr, "%s\n", errmsg); 
 }


/* -************** Below code is for testing only ***************-*/


/*print at most two parameters*/
char*
print_func_args(const Func_Arg* const arg)
{
  char* arg_sym = (char*)malloc(sizeof(char)*2*8);
  const Func_Arg* ptr = arg;  
  unsigned int scalar = 0;  
  size_t j;

  // initialize to zero the arrays
  for(j = 0; j < 14; j++)
  {
    if(j == 6) {arg_sym[j] = ','; continue;}
    arg_sym[j] = '\0';
  }


  while(!ptr && scalar != 2)
  {
    strcpy((arg_sym + scalar*8), ptr->m_id->m_name);
    ptr = ptr->m_next; 
    scalar++;
  }

  return arg_sym;
}

void print_func(const Func_Node* const temp) 
{
  const Func_Node* const func = temp;
  char* args = print_func_args(func->m_arg);
  printf("%s < %d > (%s) ;\n", func->m_id->m_name, func->m_id->m_type, args);

  free(args); /* release memory */
}


/*print an assignment*/
void
print_assign(const Assign_Node* const temp)
{
   const Assign_Node* const assgn = temp;
   
   if(!assgn->m_left || !assgn->m_right)
   {
     yyerror("Error in one of the assignment nodes when printing.");
     exit(0);
   }

   if(assgn->m_left->m_type == SYMBOL_TYPE)
   {
     printf("%s = ", (assgn->m_left->m_op).m_var->m_name);
   }

   if(assgn->m_right->m_type == FUNC_TYPE) 
   {
     print_func((assgn->m_right->m_op).m_func); // print the function
   }
}


/* print the AST for testing */
void
print_tree (const Tree* const node)
{
  if(!node) return;
  
  printf("stmt_list -------------- ");
  switch(node->m_node->m_type) // check what's the node type (type of the statement branch)
  {
    case ASSIGN_TYPE : {print_assign((node->m_node->m_op).m_assign); break;}
    case FUNC_TYPE : {print_func((node->m_node->m_op).m_func); break;}
 
    default: { printf("internal error: free bad node while printing tree %c\n", node->m_node->m_type); exit(0);}
  }

  printf("\n"); // print a new line
  print_tree(node->m_next);
  
}

