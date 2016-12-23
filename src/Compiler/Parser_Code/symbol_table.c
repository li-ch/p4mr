#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../Includes/symbol_table.h"



static void
init_symbol_table()
{
 
 unsigned int i;

 for(i = 0; i < NUM_SYMBOLS; i++)
 {
   symbol_table[i] = NULL; // set pointers to NULL
 }

}


static void
init_function_table()
{
  unsigned int i;
  
  for(i = 0; i < NUM_API_FUNC; i++)
  {
    function_table[i] = NULL; // set pointers to NULL
  }

}



void
init_tables ()
{
  init_function_table();
  init_symbol_table();
}



/* hash the key for a bin index */
static unsigned int
symhash(const char * const sym)
{
  unsigned int hash = 0;
  unsigned c;
  const char* char_ref = sym;  

  while(c = *char_ref++) hash = hash*9 ^ c;
  
  return hash;
}


const Symbol* 
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
    (cur->m_entry)->m_data = UINT_8;
    cur->m_next = NULL; // next node is NULL for termination of the linked list
    if(prev) 
    {  
      prev->m_next = cur; // update the previous node in the linked list
    }

    return (cur->m_entry); // return a reference to the newly created symbol
 
}


void 
add_symbol(const Symbol* const sym)
{
  if(!sym)
  {
    yyerror("Cannot add a symbol since the pointer is NULL");
    return;
  } 

  const unsigned int tab_index = (symhash(sym->m_name) % NUM_SYMBOLS);
  Table_Node* cur = symbol_table[tab_index];
  Table_Node* prev = NULL;  

  while(cur != NULL) 
  { // traverse the linked list
     if((cur->m_entry)->m_name && !strcmp((cur->m_entry)->m_name, sym->m_name)) { return; /*no need to add the symbol*/ }
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
       symbol_table[tab_index] = cur;
    }
  
}


void 
add_function_API(char* func_identifier, Data_Set* type_set, const int num_par)
{
  if(!func_identifier)
  {
    yyerror("Cannot add a new API function since function identifier is NULL");
    return;
  } 

  const unsigned int tab_index = (symhash(func_identifier) % NUM_API_FUNC);
  Func_Tab_Node* cur = function_table[tab_index];
  Func_Tab_Node* prev = NULL;  

  while(cur != NULL) 
  { // traverse the linked list
     if(cur->m_key && !strcmp(cur->m_key, func_identifier) && cur->m_par_num == num_par)  
     { 
       printf("\nError: function \"%s\" is being re-defined\n", func_identifier);
       exit(1);
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
delete_symbol_table()
{
  unsigned int index;

  for(index = 0; index < NUM_SYMBOLS; index++) /*loop through the table*/
  {
   
    Table_Node* ptr = symbol_table[index];
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

  for(index = 0; index < NUM_API_FUNC; index++) /*loop through the table*/
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
  delete_symbol_table();
  delete_function_table();
}



void yyerror(char *errmsg, ...)
 {
   fprintf(stderr, "%s\n", errmsg); 
 }

