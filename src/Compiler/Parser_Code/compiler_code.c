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

  while(c = *char_ref)
  { 
    hash = hash*9 ^ c;
    char_ref += 1;
  }
  
  return hash;
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
  

  if(!pass_sym) /*no symbol passed*/
  {
     /*allocate memory for a new symbol and its name*/
     arg->m_id = malloc(sizeof(Symbol));
     arg->m_id->m_name = malloc(sizeof(char)); 
     *(arg->m_id->m_name) = '\0'; // null char
     arg->m_id->m_data = EMPTY_VAL;
     arg->m_next = NULL;     

     return arg;
  }

  const Symbol* const sym_ref = lookup(pass_sym->m_name);
 
  if(sym_ref)
  { 
     arg->m_id = pass_sym; /* just point at the passed symbol */
     arg->m_id->m_data = sym_ref->m_data; // retrieve the data type from the table
  }
  else
  { // no such symbol been defined before
    printf("\"%s\" such paramter does not exist\n", pass_sym->m_name);
    exit(0);
  }  

  /*a pointer to the next argument*/
  arg->m_next = next_arg; 
 
  return arg; 
}


/**
*
* Function takes an argument list
* and converts it into a param list -- 
* a list of JSON dependency objects.
* The function also deallocates the 
* passed argument list.
**/
static Param_Node*
convert_arguments(Func_Arg* args)
{
  Func_Arg* ptr = args->m_next;
  Func_Arg* prev;
  
  Param_Node* head = malloc(sizeof(Param_Node));
  Param_Node* list;
  if(!head)
  {
    yyerror("out of memory");
    exit(0);
  }  
   
  head->m_dep_label = args->m_id->m_name; /* name of a parameter is copied so the name is not freed later when the symbol is freed*/
  
  if(args->m_id->m_data == PATH_STRING)
  {
    head->m_dep_property = malloc(sizeof(char) + strlen("path_label"));
    if(!head->m_dep_property)
    { 
        yyerror("out of memory");
        exit(0);
    }

    strcpy(head->m_dep_property, "path_label");
  }
  else /*data label*/
  {
    head->m_dep_property = malloc(sizeof(char) + strlen("data_label"));
    if(!head->m_dep_property)
    { 
        yyerror("out of memory");
        exit(0);
    }

    strcpy(head->m_dep_property, "data_label");
  }


  list = head; /*point at the beginning of the list first*/

  /*delete the first structure*/
  free(args->m_id); /*symbol (do no free the string since it is pointed at by head now)*/
  free(args); /*argument*/
  

  /*the first node has been initiliazed,*/
  /*build the rest of the list*/
  while(ptr != NULL)
  {
    list->m_next = malloc(sizeof(Param_Node));
    if(!list->m_next)
    {
      yyerror("out of memory");
      exit(0);
    }  
  
    list = list->m_next; /*update the pointer*/
    list->m_dep_label = ptr->m_id->m_name;
  
    if(ptr->m_id->m_data == PATH_STRING)
    {
      list->m_dep_property = malloc(sizeof(char) + strlen("path_label"));
      if(!list->m_dep_property)
      { 
        yyerror("out of memory");
        exit(0);
      }

      strcpy(list->m_dep_property, "path_label");
    }
    else /*data label*/
    {
      list->m_dep_property = malloc(sizeof(char) + strlen("data_label"));
      if(!list->m_dep_property)
      { 
        yyerror("out of memory");
        exit(0);
      }
      strcpy(list->m_dep_property, "data_label");
    }

   /*update pointer*/
   prev = ptr;
   ptr = ptr->m_next;
   free(prev->m_id);
   free(prev);

  } // while

  list->m_next = NULL; /*finish building the list*/
  
  return head;
}


Func_JS* 
new_func_type(Symbol* func_name, Symbol* d_type, Func_Arg* args)
{

  /*first check for the function. If it has been defined and its prototype is correct*/
  if(correct_func(func_name->m_name, args) == FAILURE)
  {
    printf("***function '%s' has not been defined before***\n", func_name->m_name);
    exit(0);
  }  

  Func_JS* ptr = malloc(sizeof(Func_JS));
  
  if(!ptr)
  {
    yyerror("out of memory");
    exit(0);
  }

  /*copy name and types*/
  ptr->m_title = func_name->m_name;
  ptr->m_data_type = d_type->m_name;
  ptr->m_data = d_type->m_data; /*real data value is needed for later computation*/
  ptr->m_param = convert_arguments(args); /*returs a linked-list of dependencies*/

  /*delete symbols*/
  free(func_name);
  free(d_type);

  return ptr;
}


Func_JS* 
new_func_no_type(Symbol* func_name, Func_Arg* args)
{

  Symbol* sym = malloc(sizeof(Symbol)); /*convert a data type to a string*/
  
  if(!sym)
  {
    yyerror("out of memory");
    exit(0);
  }  
  
  const Symbol* const argument = lookup(args->m_id->m_name);
  if(!argument) { printf("\nLabel '%s' has not been defined before.\n", args->m_id->m_name); exit(0); }


  
  const Data_Type temp_type = argument->m_data; /* data type for comparison */
  sym->m_data = temp_type;

  int index;  

  for(index  = 0; index < NUMBER_OF_DATA_TYPES; index += 1)
  {
    if(DATA_TYPE_VALUES[index] == temp_type)
    {
      sym->m_name = malloc(sizeof(char) + strlen(DATA_TYPES[index]));
      
      if(!sym->m_name)
      {
        yyerror("out of memory");
        exit(0); 
      }

      strcpy(sym->m_name, DATA_TYPES[index]); /*copy the data type as a string for JSON*/
      break;
    }
  }  

  return (new_func_type(func_name, sym, args));
} 


static int
get_statement_index()
{
  static int global_index = 0;

  global_index += 1; /* increment every time the function is called === a new statement */

  return global_index;
}


Stmt_Node* 
new_statement(Symbol* label, Func_JS* func)
{

  /*first check if the label has not been defined before*/
  if(label && (label->m_data = func->m_data) && add_label(label) == FAILURE)
  {
    /*has been defined before*/
     printf("\nRe-definition of the label '%s'.\n", label->m_name);
     exit(0);
  }

  Stmt_Node* dep = malloc(sizeof(Stmt_Node));
  
   if(!dep || !func)
  {
   yyerror("out of memory or func is NULL");
   exit(0);
  }

  dep->m_index = get_statement_index(); 
  dep->m_data_type = func->m_data_type;
  dep->m_func = func->m_title;
  
  /* symbol might be NULL if the statement only contains a function */
  if(label)
  {
    dep->m_label = label->m_name;
    free(label); 
  }
  else{
    dep->m_label = malloc(sizeof(char) * 2);
    strcpy(dep->m_label, ""); /*copy empty string*/
  }

  dep->m_dep = func->m_param; /* copy dependencies by just reassigning pointers */

  /*since func won't be needed anymore, delete it*/  
  free(func);
  func = NULL;


  return dep; 
 
} 


/**
* Since bison allows to build a reverse
* linked-list, it is needed to find the end
* of the created list and return it as it actually
* points at the first statement.
*/
Dep_Node* 
get_begin(Dep_Node* const rev_begin)
{
 Dep_Node* iter = rev_begin;
 
 /* iterate through the entire list and return */
 /* a pointer to the last node */
 while(iter->m_next)
 {
   iter = iter->m_next;
 }

  return iter;
}

/**
* Deallocates the list of dependencies
* a statement has.
*
**/
static void
deall_dep(Param_Node* param)
{
 Param_Node* ptr = param;
 Param_Node* prev;

 while(ptr) /*free all params*/
 {
  prev = ptr;
  ptr = ptr->m_next;
  
  /*release memory of label and type/property*/
  free(prev->m_dep_label);
  free(prev->m_dep_property);

  /*delete the node itself*/
  free(prev);
 }

}

static void
delete_list(Dep_Node* dep)
{
  Dep_Node* ptr = dep;
  Dep_Node* prev;
 
  /*iterate through the list and delete one node at a time*/
  while(ptr)
  {
   prev = ptr;
   ptr = ptr->m_prev;
   /*delete all the items*/
   free(prev->m_stmt->m_func);
   free(prev->m_stmt->m_data_type);
   if(prev->m_stmt->m_label) { free(prev->m_stmt->m_label); /*label might be NULL*/ }
   
   deall_dep(prev->m_stmt->m_dep); /*delete all the dependencies (parameters)*/

   /*free the node itself*/
   free(prev->m_stmt);
   free(prev);   

  }
 
  /*done deleting the list*/
}

/**
* Deletes the created dependency 
* lined-list.
**/
void 
deallocate_dependency_list(Program* head)
{
  if(!head) { printf("\nThere is nothing to delete\n"); return; }

  printf("\nDeleteting the dependency linked list of the '%s' source code.\n", head->m_title);
  
  delete_list(head->m_begin); /*delete the list*/

  printf("\nDone deleting the dependency linked list of the '%s' source code.\n", head->m_title);
  
  /*delete the head itself*/
  free(head->m_title);
  free(head);
  head = NULL;

} 


/**
* Helper function for
* writing a statement to a
* JSON file.
*/
static void
write_stmt(const Stmt_Node* const stmt, FILE* const js_file)
{

 /*create a JSON object*/
 fprintf(js_file, "\n\t{\n");
 
 /*JSON interface*/
 fprintf(js_file, "\t\t\"index\"   :    \"%i\",\n", stmt->m_index);
 fprintf(js_file, "\t\t\"type\"    :    \"%s\",\n", stmt->m_data_type);
 fprintf(js_file, "\t\t\"func\"    :    \"%s\",\n", stmt->m_func);
 fprintf(js_file, "\t\t\"label\"   :    \"%s\",\n", stmt->m_label);
 
 /*print a param array*/
 fprintf(js_file, "\t\t\"params\"  :     [\n");
 
 const Param_Node* params = stmt->m_dep;
 
 while(params)
 {
   fprintf(js_file, "\t\t\t{\n");
   fprintf(js_file, "\t\t\t\t\"label\"  :  \"%s\",\n", params->m_dep_label);
   fprintf(js_file, "\t\t\t\t\"property\"  :  \"%s\"\n", params->m_dep_property);
   
   /*this is used for nice printing*/
   if(params->m_next) /*there is more*/
   {
       fprintf(js_file, "\t\t\t},\n");
   }
   else /*no more params*/
   {
       fprintf(js_file, "\t\t\t}\n");
       break;
   }

   params = params->m_next; /*move to the next param*/
 }
   
  fprintf(js_file, "\t\t]\n");

}


/**
* This function takes a
* pointer to the program and
* creates a JSON file which 
* stores the dependency list
* of the program.
**/
void
create_JSON(const Program* const program)
{

  if(!program) { printf("\n[]\n"); return; } /* nothing to print */
  
  FILE* js_file = fopen("dependency_graph.json", "w+"); /*open a new file and delete the previous content*/
  if(!js_file)
  {
    printf("\n*** Cannot create a JSON file ***\n");
    exit(0);
  }

  const Dep_Node* ptr = program->m_begin;
  
  fprintf(js_file, "[");
  
  while(ptr)
  {
   
   write_stmt(ptr->m_stmt, js_file); /*print a statement*/  
   
   if(ptr->m_prev) 
   {
     fprintf(js_file, "\t},\n"); /*write a new line*/
   }
   else 
   {
     fprintf(js_file, "\t}\n"); /*write a new line*/ 
     break; /*done printing*/
   }

   ptr = ptr->m_prev; /*update the pointer*/
  }

  fprintf(js_file, "]\n");

  fclose(js_file); /*close the file*/
}

/* -************** Below code is for testing only ***************-*/


/**
* Prints the structure of a statement so that 
* it would be easy to visualise the dependency
* list.
*/

static void
print_stmt(const Stmt_Node* const stmt)
{
  /*print a structure as a JSON object*/
 printf("\n\t\t{\n");
 
 /*JSON interface*/
 printf("\t\t\t\"index\"   :    \"%i\",\n", stmt->m_index);
 printf("\t\t\t\"type\"    :    \"%s\",\n", stmt->m_data_type);
 printf("\t\t\t\"func\"    :    \"%s\",\n", stmt->m_func);
 printf("\t\t\t\"label\"   :    \"%s\",\n", stmt->m_label);
 
 /*print a param array*/
 printf("\t\t\t\"params\"  :     [\n");
 
 const Param_Node* params = stmt->m_dep;
 
 while(params)
 {
   printf("\t\t\t\t{\n");
   printf("\t\t\t\t\t\"label\"  :  \"%s\",\n", params->m_dep_label);
   printf("\t\t\t\t\t\"property\"  :  \"%s\"\n", params->m_dep_property);
   
   /*this is used for nice printing*/
   if(params->m_next) /*there is more*/
   {
       printf("\t\t\t\t},\n");
   }
   else /*no more params*/
   {
       printf("\t\t\t\t}\n");
       break;
   }

   params = params->m_next; /*move to the next param*/
 }
   
  printf("\t\t\t]\n");
  
}

/**
* Prints the structure
* of the dependency linked-list.
*/
void 
display_dependency_list(const Program* const head) 
{
  
  if(!head) { printf("\n[]\n"); return; } /* nothing to print */
  
  printf("\nDependency list for the '%s' program:\n", head->m_title);

  const Dep_Node* ptr = head->m_begin;
  
  printf("\n\t[");
  
  while(ptr)
  {
   
   print_stmt(ptr->m_stmt); /*print a statement*/  
   
   if(ptr->m_prev) 
   {
     printf("\t\t},\n"); /*print a new line*/
   }
   else 
   {
     printf("\t\t}\n"); /*print a new line*/ 
     break; /*done printing*/
   }

   ptr = ptr->m_prev; /*update the pointer*/
  }

  printf("\t]\n");
}

