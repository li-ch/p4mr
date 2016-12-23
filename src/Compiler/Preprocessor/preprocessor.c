# include <stdio.h>
# include <stdlib.h>
# include <string.h> 
# include "../Includes/symbol_table.h"
# include "preprocessor.h"



typedef struct bufstack BuffStack;

struct bufstack {
  int lineno; /* saved line number */
  const char *filename; /* name of this file */
  FILE *file; /* current file */
}; 

static const int SUCCESS = 1;
static const int FAILURE = 0;



static BuffStack* curbs = NULL;
static int cf_lineno = 1; /*line number of the current file*/
static int include_file = 0; /*a global flag that identifies if the current file is an include file*/

#define BUFFER_SIZE 256
static char buffer[BUFFER_SIZE]; /*buffr for reading in data*/
//static char char_buffer; // for reading a character in

static const char* curfilename; /* name of current input file */
static int newfile(const char * const fn, FILE* old_file);
static int popfile(void);


/**
* This function is used for
* translation/mapping between a string of a data type
* and the data type value.
*/
static Data_Type
get_data_type(const char buffer[])
{
  unsigned int i;
  
  for(i = 0; ptr < NUMBER_OF_DATA_TYPES; i++)
  {
    if(!strcmp(buffer, DATA_TYPES[i])) /*valid data type*/
    {
      return DATA_TYPE_VALUES[i];
    }
  }

  return ERROR_TYPE;
}


/**
* Function initializes a data type array 
* with the type ERROR_DATA = 0 
*
**/
static void
clear_possible_val(Data_Type types[], const int limit)
{
  unsigned int i; 
  for(i = 0; i < limit; i++)
  {
    types[i] = ERROR_DATA; /* equiv to types[i] = 0;*/
  }
} 


/**
* Function creates a data set
* for the function table that is later 
* used in the semantic step;
*/
static Data_Set*
create_data_set(Data_Type possible_val[], const int possible_val_index)
{
 
 Data_Set* head; /*head of linked-list*/
 Data_Set* ptr;
 Data_Set* prev;
 
 head = malloc(sizeof(Data_Set));
   
 if(!head)
 {
   printf("\nSystem is out of memory\n");
   exit(1);
 }
 
 /*initialize the first node of the data_set linked list*/
 head->m_dtype = possible_val[0]; /*must take at least one type*/
 head->m_next = NULL;
 ptr = head;

 int index;
 
 for(index = 1; index < possible_val_index; index++)
 {
   prev = ptr;
   ptr = malloc(sizeof(Data_Set));
   
   if(!ptr)
   {
     printf("\nSystem is out of memory\n");
     exit(1);
   }
   
   ptr->m_dtype = possible_val[index];
   ptr->m_next = NULL;
   prev->m_next = ptr;  

 }

  return head;

}




/**
* Function checks if a typed data type
* is a valid one.
* 
* returns : SUCCESS - valid, FAILURE - invalid.
*
**/
static int
is_valid_data_type(const char* const data_type)
{
  
  unsigned int i;
  
  for(i = 0; ptr < NUMBER_OF_DATA_TYPES; i++)
  {
    if(!strcmp(data_type, DATA_TYPES[i])) /*valid data type*/
    {
      return SUCCESS;
    }
  }


  return FAILURE;
}


/**
* Function reads an include
* file and checks its syntax and 
* preprocess some symbol tables (function_table).
**/
static int
read_include_file(FILE* file)
{

  int par_number = 0; /*function paramter number*/
  int possible_val_index = 0;
  char char_buffer;
  Data_Type possible_val[NUMBER_OF_DATA_TYPES]; // stores data types of a function
  
  clear_possible_val(possible_val, NUMBER_OF_DATA_TYPES);  

 /* only for reading include files -- API header files */
 while(fscan(file, "%c", char_buffer)) /*scan the entire file*/
 {
    if(char_buffer == ' ' || char_buffer == '\t') {while(fscan(file, "%c", char_buffer) && (char_buffer == ' ' || char_buffer == '\t')); /*ignore all white spaces*/}

    if(char_buffer == EOF){ return SUCCESS; } /*need this case since white spaces are skipped -- empty files are allowed*/
    if(char_buffer == '\n') { cf_lineno++; continue; }
    /*if not the above conditions, then only letters can be; otherwise, a syntax error*/ 
    
    if(!isalpha(char_buffer) && char_buffer != '_'){ /*means a non-alphabetical character or not an underscore*/
       if(char_buffer == '#')
       {
         printf("\n%s: line %i: Recursive include is not supported\n", curfilename, cf_lineno);
         
       }
       else
       {
         printf("\n%s: line %i: Wrong function identifier\n", curfilename, cf_lineno);
       }
 
       return FAILURE;
    }

    /*read until a non-alphabetical found found*/
    buffer[0] = char_buffer; 
    unsigned int index = 1;
    while(fscan(file, "%c", char_buffer) && (isalpha(char_buffer) || char_buffer == '_'))
    {
      buffer[index] = char_buffer; /*read character by character*/
      index++;
      if(index == (BUFFER_SIZE - 1)) {printf("\n%s: %i: too long function identifer\n", curfilename, cf_lineno); return FAILURE;}
    } 
           
     if(isspace(char_buffer)) 
     {
        if(char_buffer == '\n') {cf_lineno++;}

        while(fscan(file, "%c", char_buffer) && isspace(char_buffer)) /*ignore white spaces*/ 
        {
          if(char_buffer == '\n') {cf_lineno++;}
        }
     }
    

     /*check syntax*/
     if(char_buffer != '{') {printf("\n%s: line %i: wrong data set specification\n", curfilename, cf_lineno); return FAILURE;} /*must be an opening brace*/

     buffer[index] = '\0'; /*C string*/

     /*the following keeps building */
     /*since a dynamic char array has to be passed, create it here and the buffer is used for other readings*/
     char* func_identifier = malloc(sizeof(char)*strlen(buffer) + sizeof(char)); /*allocate enough memory for the function identifier*/
     if(!func_identifier)
     {
        printf("\nSysmtem is out of memory.\n");
        exit(1);
     }

     strcpy(func_identifier, buffer); /*copy the function identifier*/
     /***/

     /*following steps check the grammar and data types*/
     /* read until a closing brace is found */
     while(fscan(file, "%c", char_buffer) && char_buffer != '}')
     {
       if(isspace(char_buffer)) /*ignore white spaces*/
       {
         if(char_buffer == '\n') {cf_lineno++;}
         continue;
       }       

       /*must be data types that must start with a letter*/
       if(isalpha(char_buffer))
       {
         unsigned int size = 1;
         buffer[0] = char_buffer;
         while(fscan(file, "%c", char_buffer) && (isalnum(char_buffer) || char_buffer == '_')) /*reads the data type*/
         {
           buffer[size] = char_buffer;
           size++;
           if(size == (BUFFER_SIZE-1)) {printf("\n%s: %i: too long data identifier\n", curfilename, cf_lineno); return FAILURE;} 
         }//  while
          
          buffer[size] = '\0'; /*C string*/
       }// if
       else
       { 
         /*syntax error*/
         free(func_identifier);
         printf("\n%s: line %i : syntax error defining data types\n", curfilename, cf_lineno);
         return FAILURE;
       }// else
   
        
       /*first, the current char is checked for syntax error*/
       
       if(!isspace(char_buffer) || char_buffer != ',' || char_buffer != '}' || !is_valid_data_type(buffer))
       {
         /*syntax error*/
         free(func_identifier);
         printf("\n%s: line %i : syntax error defining data types\n", curfilename, cf_lineno);
         return FAILURE;
       }
       
       possible_val[possible_val_index] = get_data_type(buffer); /*get a possible data type*/
       possible_val_index++; /*for next type if the currently checked function takes more*/

       if(isspace(char_buffer))
       {
         if(char_buffer == '\n') { cf_lineno++; }
         continue;
       }//if
       else // either as colon or a closing brace 
       {
         if(char_buffer == ',') {continue;}
         
         break; /*must be a closing brace*/
       }// else   
       
     }// while
     
     /*the source code has been read until here. Now a pair of parentheses, a digit and a semicolon are expected*/
     while(fscan(file, "%c", char_buffer) && isspace(char_buffer)) /*ignore white spaces*/
     {
       if(char_buffer == '\n') { cf_lineno++; }
     }
     
     /*handle parentheses*/
     if(char_buffer != '(') 
     {
      /*syntax error*/
      free(func_identifier);
      printf("\n%s: line %i : syntax error defining data types: missing '('\n", curfilename, cf_lineno);
      return FAILURE;
     }

     /*read until a digit is found*/
      while(fscan(file, "%c", char_buffer) && isspace(char_buffer)) /*ignore white spaces*/
     {
       if(char_buffer == '\n') { cf_lineno++; }
     }


     if(!isdigit(char_buffer) || (char_buffer == EOF))
     {
      /*syntax error*/
      free(func_identifier);
      printf("\n%s: line %i : syntax error defining parameter number\n", curfilename, cf_lineno);
      return FAILURE;
     }

     /*read digits until non-digit is found*/
     param_number = char_buffer - '0';
     
     while(fscan(file, "%c", char_buffer) && isdigit(char_buffer))
     {
        (char_buffer == EOF) // still need to check this condition sice EOF -1. Need to check if this condition id needed
        {
         /*syntax error*/
         free(func_identifier);
         printf("\n%s: line %i : syntax error: incomplete function definition\n", curfilename, cf_lineno);
         return FAILURE;
        }
        param_number = param_number*10 + (char_buffer - '0'); /*more than 9 parameters*/
     }


     
     if(isspace(char_buffer)) /*skip until a closing parenthesis is found*/
     {
                
       if(char_buffer == '\n') { cf_lineno++; }
       /*read until a digit is found*/
       while(fscan(file, "%c", char_buffer) && isspace(char_buffer)) /*ignore white spaces*/
       {
         if(char_buffer == '\n') { cf_lineno++; }
       }// while 
     }// if     
     
     
     /*handle parentheses*/
     if(char_buffer != ')') 
     {
      /*syntax error*/
      free(func_identifier);
      printf("\n%s: line %i : syntax error defining parameter number\n", curfilename, cf_lineno);
      return FAILURE;
     }

     /*what's left is a semi-colon*/
     while(fscan(file, "%c", char_buffer) && isspace(char_buffer))
     {
       if(char_buffer == '\n') {cf_lineno++;}
     }

     /*last step -- either a semicolon or a syntax error*/
     if(char_buffer != ';')
     {
       /*syntax error*/
       free(func_identifier);
       printf("\n%s: line %i : syntax error: a missing ';'\n", curfilename, cf_lineno);
       return FAILURE;
     }

     /*can create a reference in the function symbol table*/
     Data_Set* av_data_types = create_data_set(possible_val, possible_val_index);

     add_function_API(func_identifier, av_data_types, param_number);
     
     /*reset values -- no need to delete the allocated memory since it is handled by other files*/
     param_number = 0;
     possible_val_index = 0;
        
     /*Function definiiton is done. Look for another function definition.*/
 }// while

   return SUCCESS; /*the entire file has been checked and no syntax erros have been found*/
}



static int
read_source_code(FILE* file)
{
  while(fscan(file, "%c", char_buffer)) // scan until the end of the file
  {

    if(char_buffer == EOF){ return SUCCESS; }
    if(char_buffer == '\n') { cf_lineno++; }
    if(char_buffer == '#') 
    { // handle include
        /*check if the there is spaces until a quote is found*/
        while(fscan(file, "%c", char_buffer) && (char_buffer == ' ' || char_buffer == '\t'));  /*skip all white spaces*/
        if(char_buffer != '"')
        {
          printf("\n%s: %i: incoorrect include file\n", curfilename, cf_lineno);
          return FAILURE;
        }
        
        unsigned int size = 0; 
        /*read until another quotation mark is found*/
        while(fscan(file, "%c", char_buffer) &&  char_buffer != '"')
        {
          buffer[size] = char_buffer;
          size++;
          if(size == (BUFFER_SIZE - 1)) {printf("\n%s: %i: too long filename\n", curfilename, cf_lineno); return FAILURE;}
        } 
        
        buffer[size] = '\0'; /*create a C string*/
        if(newfile(buffer, file) == FAILURE) {return FAILURE;} // create a new structure and try if it is possible to read a new file
      
    }  
 
  } // while loop

}



/*for checking if a new file is used*/
static int
newfile(const char* const filename, FILE* old_file)
{
  FILE* file = fopen(filename, "r");
  
  if(!file) // no such file
  {
    printf("\"%s\" does not exist\n", filename);
    return FAILURE;
  }
  
  BuffStack* ptr = malloc(sizeof(BuffStack));
 
  if(!ptr) // not enough memory
  {
   printf("\nOut of memory in the pre-processing stage\n");
   exit(1);
  }
  
 
  /* save the current file state */
  ptr->file = old_file;
  ptr->filename = curfilename;
  ptr->lineno = cf_lineno;

  curbs = ptr;
  cf_lineno = 1;
  curfilename = filename;
  
  include_file = 1; /*set flag to notify that an include file is being read*/ 
   
  read_include_file(file);
  popfile(); /*reading is done, pop the file*/

  return SUCCESS;
}

  
static int
popfile(void)
{
  BuffStack* bs = curbs;
  BuffStack* prevbs;
  
  if(!bs) {return 0;}
  
  /* get rid of current entry */
  fclose(bs->file);
  yy_delete_buffer(bs->bs);
  
  
  /* switch back to previous */
  prevbs = bs->prev;
  
  free(bs);
  
  if(!prevbs) { return 0; }

  yy_switch_to_buffer(prevbs->bs);
  curbs = prevbs;
  yylineno = curbs->lineno;
  curfilename = curbs->filename;
  
  include_file = 0; /*means an include has been handled*/

  return SUCCESS;
}



int 
preprocess_file(const char* const filename)
{
  /*check if it is possible to open the file*/
  FILE* file = fopen(filename, "r");

  if(!file)
  {
    return FAILURE;
  }

  curfilename = filename; 

  const int status = read_source_code(file); /*return the status of reading*/
  
  fclose(file); // close the file

  return status; /*successfully included all the files*/
 
}





