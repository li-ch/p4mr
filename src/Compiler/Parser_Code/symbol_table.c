#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../Includes/symbol_table.h"
#include "../Includes/data_types.c"

/**
* This function initializes a predefined list of
* server functions which is static and should not
* vary much in the future.
*/
static void
init_server_functions()
{
  /*for future add reading from a text file to initialize this list*/
  /*read server functions from the Predefined_Functions directory.*/
  /*text file -- server_functions.txt*/
}

static void
init_label_table()
{
 
 unsigned int i;

 for(i = 0; i < NUM_LABELS; i += 1)
 {
   label_table[i] = NULL; // set pointers to NULL
 }

}


static void
init_function_table()
{
  unsigned int i;
  
  for(i = 0; i < NUM_API_FUNC; i += 1)
  {
    function_table[i] = NULL; // set pointers to NULL
  }

}


void
init_tables ()
{
  init_function_table();
  init_label_table();
 // init_server_functions();
}



/* hash the key for a bin index */
static unsigned int
symhash(const char * const sym)
{
  unsigned int hash = 0;
  unsigned c;
  const char* char_ref = sym;  

  while(c = *char_ref)
  { 
   hash = hash*9 ^ c;
   char_ref += 1;
  }

  return hash;
}


const Symbol* const
lookup(const char* const sym)
{
  Table_Node* cur = label_table[ symhash(sym) % NUM_LABELS];
  Table_Node* prev;  

  while(cur != NULL) 
  { // traverse the linked list
     if((cur->m_entry)->m_name && !strcmp((cur->m_entry)->m_name, sym)) { return (cur->m_entry); }
     prev = cur;
     cur = cur->m_next; // move further
  }     
 
  // if this code is reached, means no symbol with the same string was found. 
  // Returns NULL to notify that the symbol has not been defined before. 

    return NULL;
 
}


int 
add_label(const Symbol* const sym)
{
  if(!sym)
  {
    yyerror("Cannot add a symbol since the pointer is NULL");
    return FAILURE;
  } 

  const unsigned int tab_index = (symhash(sym->m_name) % NUM_LABELS);
  Table_Node* cur = label_table[tab_index];
  Table_Node* prev = NULL;  

  while(cur != NULL) 
  { // traverse the linked list
     if((cur->m_entry)->m_name && !strcmp((cur->m_entry)->m_name, sym->m_name)) { return FAILURE; /*the label exists, error*/ }
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

    // add a new entry to the Symbol table
    (cur->m_entry)->m_name = strdup(sym->m_name);  
    (cur->m_entry)->m_data = sym->m_data; /*data type*/
    cur->m_next = NULL; /* next node is NULL for termination of the linked list */
    

    if(prev) 
    {  
      prev->m_next = cur; /* update the previous node in the linked list */
    }
    else /*means the first node is being added*/
    {
       label_table[tab_index] = cur;
    }
  
   return SUCCESS;
}


int 
add_function_API(char* func_identifier, Data_Set* type_set, const int num_par)
{
  if(!func_identifier)
  {
    yyerror("Cannot add a new API function since function identifier is NULL");
    return FAILURE;
  } 

  const unsigned int tab_index = (symhash(func_identifier) % NUM_API_FUNC);
  Func_Tab_Node* cur = function_table[tab_index];
  Func_Tab_Node* prev = NULL;  

  while(cur != NULL) 
  { // traverse the linked list
     if(cur->m_key && !strcmp(cur->m_key, func_identifier) && cur->m_par_num == num_par)  
     { 
       return FAILURE;
     }
     prev = cur;
     cur = cur->m_next; // move further
  }     
 
  // if this code is reached, means no function with the same string was found. 
  // create a new entry and append it to the hashed bin. 

    
    cur = malloc(sizeof(Func_Tab_Node));    
    if(!cur) 
    {
      yyerror("out of memory");
      exit(0);
    }
 
    cur->m_key = func_identifier; /* just reassign  the identifier */
    cur->m_data_set = type_set;   /* just reassign the pointer */ 
    cur->m_par_num = num_par; 

    cur->m_next = NULL; /* next node is NULL for termination of the linked list */
    

    if(prev) 
    {  
      prev->m_next = cur; /* update the previous node in the linked list */
    }
    else /*means the first node is being added*/
    {
       function_table[tab_index] = cur;
    } 

    return SUCCESS;
}



static const Func_Tab_Node* const
get_function(const char* const f_title, const int par_num)
{
 
  const unsigned int tab_index = (symhash(f_title) % NUM_API_FUNC);
  const Func_Tab_Node* cur = function_table[tab_index];
   

  while(cur) 
  { // traverse the linked list
     if(cur->m_key && !strcmp(cur->m_key, f_title) && cur->m_par_num == par_num)  
     { 
       return cur; /* found the function */
     }
     cur = cur->m_next; // move further
  }   


 return NULL; /*function not found*/
}


/**
* This function is a simplified version of the 
* final function since this function only checks 
* two server functions, 'map' and 'store'.
* 
* In the future the function should check against all
* possible server functions stored in a text file (server_functions.txt)
* in the Predefined_Functions directory.
*/
static int
is_server_function(const char* const f_title, const Func_Arg* const args)
{

  if(!strcmp(f_title, "map") && args && !args->m_next && args->m_id->m_data == PATH_STRING) /* matches 'map' */
  {
    return SUCCESS;
  }

  if(!strcmp(f_title, "store") && args && args->m_next && args->m_next->m_id->m_data == PATH_STRING && !args->m_next->m_next) /* match 'store' */
  {
     return SUCCESS;
  }

  return FAILURE;
}
 

/*
* Functions does a very simple checking:
* checks if all arguments are of the same type.
*
* No - returns -1, yes - returns number of arguments
*/
static int
check_arguments(const Func_Arg* const args)
{
  if(!(args->m_next)){ return 1; } /* only one parameter */

  int num_par = 1;
  const Data_Type data_type = args->m_id->m_data; /*the first parameter determines data type*/
  const Func_Arg* check;  

  for(check = args->m_next;  check != NULL; check = check->m_next, num_par += 1)
  {
    if(check->m_id->m_data != data_type) /*not all parameters have the same type*/
    {
       return FAILURE;
    }
  }

   return num_par;
}


/**
* Does semantic checking.
* Checks if the parameters are of the
* same type, if the function has been
* defined before, if the protype matches.
*
*/
int 
correct_func(const char* const f_title, const Func_Arg* const args)
{ 
  if(!args)
  {
    return FAILURE; /*every function must have an argument*/
  }

  if(is_server_function(f_title, args) == SUCCESS) /*check if it is not a server function*/
  {
    return SUCCESS;
  }   


  /*not a server function*/
  /*arguments must be of the same type*/
  int par_number;
  if((par_number = check_arguments(args)) == FAILURE)
  {  
     return FAILURE; 
  }

  /*if the below code is being executed, it means*/
  /*that all parameters are of the same type. Par number is known. Look for function*/
  const Func_Tab_Node* const function = get_function(f_title, par_number); /*get a pointer to the function*/
  
  if(!function)
  {
    return FAILURE; /*no such function*/
  }

  /* check data types -- whether the function supports the passed type */
  const Data_Set* d_types = function->m_data_set;
  const Data_Type func_type = args->m_id->m_data; /*the first parameter determines the data type*/  

  while(d_types)
  {
    if(d_types->m_dtype == func_type)
    {
      return SUCCESS;
    }

    d_types = d_types->m_next; /*move further*/
  }

  return FAILURE; /*incorrect data type*/
}


/* free a symbol */
static void
free_symbol(Symbol* sym)
{
  free(sym->m_name);
  free(sym);
  sym = NULL;
}


/* Deletes the symbol table */
static void 
delete_label_table()
{
  unsigned int index;

  for(index = 0; index < NUM_LABELS; index += 1) /*loop through the table*/
  {
   
    Table_Node* ptr = label_table[index];
    Table_Node* next;
       
    /*loop through a linked-list and delete it*/
    while(ptr)
    {
      next = ptr->m_next;
      free_symbol(ptr->m_entry); /*delete entry*/
      free(ptr);
      ptr = next; /*update the pointer*/
          
    }// while
   
  }// for
  
}



static void
delete_data_set(Data_Set* data_set)
{
 Data_Set* next;

 while(!data_set) /* traverse the linked-list of acceptable data types */
 {
   next = data_set->m_next;
   free(data_set);
   data_set = next;
 }

}

static void 
delete_function_table()
{
  unsigned int index;

  for(index = 0; index < NUM_API_FUNC; index += 1) /*loop through the table*/
  {
   
    Func_Tab_Node* ptr = function_table[index];
    Func_Tab_Node* next;
       
    /*loop through a linked-list and delete it*/
    while(ptr)
    {
      next = ptr->m_next;
      free(ptr->m_key); /*delete the key*/
      delete_data_set(ptr->m_data_set); 
      ptr = next; /*update the pointer*/
          
    }// while
   
  }// for

}


void 
delete_tables()
{
  /*deletes the respective tables*/
  delete_label_table();
  delete_function_table();
}



void yyerror(char *errmsg, ...)
 {
   fprintf(stderr, "%s\n", errmsg); 
 }

