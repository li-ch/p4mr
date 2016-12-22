/*
 * helper functions for compiler
 */
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <math.h>
# include "../Includes/compiler_header.h"


/* symbol table */
/* hash a symbol */
static unsigned int
symhash(const char * const sym)
{
  unsigned int hash = 0;
  unsigned c;
  const char* char_ref = sym;  

  while(c = *char_ref++) hash = hash*9 ^ c;
  
  return hash;
}


static 
const Symbol* const
get_symbol(const char* const symbol)
{

  Table_Node* cur = symbol_table[(symhash(symbol) % NUM_SYMBOLS)];

  while(cur != NULL) 
  { // traverse the linked list
     printf("strings being compared: %s and %s\n", (cur->m_entry)->m_name, symbol);
     if((cur->m_entry)->m_name && !strcmp((cur->m_entry)->m_name, symbol)) { return (cur->m_entry); }
     cur = cur->m_next; // move further
  }     
 
  printf("get_symbol did not find the passed symbol\n");
  exit(0);
  return NULL;

}


/*Next come the procedures to build the AST nodes and symlists. They all allocate a node
and then fill in the fields appropriately for the node type. An extended version of
treefree recursively walks an AST and frees all of the nodes in the tree.
*/



Ast*
newfunctype(const Symbol* const sym, Data_Type d_type, Func_Arg* args)
{

  Ast* ptr = malloc(sizeof(Ast)); /*create an AST node*/
  Func_Node* data = malloc(sizeof(Func_Node)); /*Create a Func_Npde*/

   if(!ptr || !data) 
   {
     yyerror("out of memory");
     exit(0);
   }

  ptr->m_type = FUNC_TYPE; /*function type*/
  
  data->m_id = malloc(sizeof(Symbol)); // allocate a symbol
  if(!(data->m_id))
  {
    yyerror("out of memory");
    exit(0); 
  }  

  data->m_id->m_name = malloc(sizeof(char) * strlen(sym->m_name) + sizeof(char)); // allocate for string
  if(!(data->m_id->m_name))
  {
    yyerror("out of memory");
    exit(0); 
  } 
  
  strcpy(data->m_id->m_name, sym->m_name); // copy the string
  data->m_id->m_data = d_type; // set a data type
  data->m_arg = args;

  (ptr->m_op).m_func = data; /*point to the newly created function node*/

  return ptr; /*returns a pointer to the created AST node*/

}


Ast* 
newfuncnotype(const Symbol* const sym, Func_Arg* args)
{
  return (newfunctype(sym, (args->m_id)->m_data, args));
}


Ast* 
newassign(const Symbol* const sym, Ast* exp)
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
    (left->m_op).m_var = malloc(sizeof(Symbol)); // allocate memory for a symbol
    
    if(!((left->m_op).m_var))
    {
     yyerror("out of memory");
     exit(0);
    }


    (left->m_op).m_var->m_name = malloc(sizeof(char) * strlen(sym->m_name) + sizeof(char));
    if(!((left->m_op).m_var->m_name))
    {
     yyerror("out of memory");
     exit(0);
    }

    strcpy((left->m_op).m_var->m_name,  sym->m_name); /*copy the name*/
    

    /*handle the right hand side first*/
    right->m_type = FUNC_TYPE;
    (right->m_op).m_func = (exp->m_op).m_func; /*just copy the pointer value*/
    

    /* read type from the function */
    (left->m_op).m_var->m_data = (right->m_op).m_func->m_id->m_data; // assign the data type to the variable taken from the function
   
    /*add the newly assigned symbol to the table*/
    add_symbol((left->m_op).m_var);     

  }
  else
  {
    printf("Something wrong since exp -> m_type != FUNC_TYPE\n");
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
newarglist(const Symbol* const pass_sym, Func_Arg* next_arg)
{
  Func_Arg* arg = malloc(sizeof(Func_Arg));
  
  if(!arg)
  {
   yyerror("out of memory");
   exit(0);
  }
  
  /*allocate memory for a new symbol and its name*/
  arg->m_id = malloc(sizeof(Symbol));

  if(!pass_sym) /*no arguments passed*/
  {
     arg->m_id->m_name = malloc(sizeof(char)); 
     *(arg->m_id->m_name) = '\0'; // null char
     arg->m_id->m_data = EMPTY_VAL;
     arg->m_next = NULL;     

     return arg;
  }

  printf("newarglist: before calling the reference to the table\n");
  const Symbol* const sym_ref = get_symbol(pass_sym->m_name);
  
  printf("newarglist: get_symbol has been called\n");
 
  if(sym_ref)
  { 
     arg->m_id->m_name = malloc(sizeof(char) * strlen(pass_sym->m_name) + sizeof(char)); 
     strcpy(arg->m_id->m_name, pass_sym->m_name);
     arg->m_id->m_data = sym_ref->m_data; // retrieve the data type from the table
  }
  else
  { // no such symbol been defined before
    printf("newarglist: \"%s\" such paramter does not exists\n", pass_sym->m_name);
    exit(0);
  }  

  /*a pointer to the next argument*/
  arg->m_next = next_arg; 
    printf("newarglist: ends here\n");
 
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
 if(!(node->m_left) || !(node->m_right))
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
static void
treefree(Tree* node)
{

  if(!node || !(node->m_node)) 
  {
    return;
  } 
  
 
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


void 
deallocate_tree(Program* root) 
{

  if(!root)
  { 
    printf("There is no AST to delete.\n");
    return;
  }

  printf("\nDeleting the AST of \"%s\" program.\n\n", root->m_title);
  
  treefree(root->m_begin); /*deletes the entire tree*/

  printf("\nDone deleting the AST of \"%s\" program.\n\n", root->m_title);

  // handle the title of the root
  if(root->m_title)
  {
    free(root->m_title);
    root->m_title = NULL;
  }
  

  root->m_begin = NULL;  
  free(root);

  root = NULL;

  delete_tables(); /* delete symbol/API function tables */

  
}


Program*
new_program(Tree* begin)
{
  Program* root = malloc(sizeof(Program));
  root->m_begin = begin;
  
  return root;
}


void 
copy_prog(Program* root, Program* node)
{
  if(!root)
  {
    printf("No copying possible because root is NULL.\n");
    return;
  }

  /* copy pointers from the node and delete it */
  root->m_begin = node->m_begin;
  free(node);
  node = NULL; 
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


static 
void print_func(const Func_Node* const temp) 
{
  const Func_Node* const func = temp;
  char* args = print_func_args(func->m_arg);
  printf("%s < %d > (%s) ;\n", func->m_id->m_name, func->m_id->m_data, args);

  free(args); /* release memory */
}


/*print an assignment*/
static void
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
static void
print_tree (const Tree* const node)
{
  if(!node) return;
   
  print_tree(node->m_next); /*recursivle go to the end of the tree and then start printing from the end*/ 

  printf("stmt_list -------------- ");
  switch(node->m_node->m_type) // check what's the node type (type of the statement branch)
  {
    case ASSIGN_TYPE : {print_assign((node->m_node->m_op).m_assign); break;}
    case FUNC_TYPE : {print_func((node->m_node->m_op).m_func); break;}
 
    default: { printf("internal error: free bad node while printing tree %c\n", node->m_node->m_type); exit(0);}
  }

  printf("\n"); // print a new line
 

}

void 
print_program(const Program* const root)
{
  if(!root || !(root->m_title))
  {
    printf("Cannot print AST since it is empty.\n");
    return;
  }

  printf("Program title: \"%s\"\n\n", root->m_title);
  
  /*print the tree itself*/
  print_tree(root->m_begin);

}

