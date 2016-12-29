# include <ctype.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h> 
# include "../Parser_Code/symbol_table.c" 
# include "../Includes/preprocessor.h"



typedef struct bufstack BuffStack;

struct bufstack {
  BuffStack* prev; /*previous file*/
  int lineno; /* saved line number */
  char *filename; /* name of this file */
  FILE *file; /* current file */
}; 

static const int WRONG_INDEX = -1;


static BuffStack* curbs = NULL;
static int cf_lineno = 1; /*line number of the current file*/


#define BUFFER_SIZE 256
static char buffer[BUFFER_SIZE]; /*buffr for reading in data*/
//static char char_buffer; // for reading a character in

static char* curfilename; /* name of current input file */
static int newfile(char*  fn, FILE* old_file);
static void popfile(void);


/**
* This function is used for
* translation/mapping between a string of a data type
* and the data type value.
*/
static Data_Type
get_data_type(const int index)
{
  if(index >= 0 && index < NUMBER_OF_DATA_TYPES)
  {
     return DATA_TYPE_VALUES[index];
  }

   printf("\n%s: line %i: error index of data type out of range\n", curfilename, cf_lineno);

   return ERROR_DATA;
}


/**
* Function checks if a newly defined 
* data type has not been defined in for the same 
* function more than one time
**/
static int
already_exist(const Data_Type possible_val[], const Data_Type new_type, const int possible_val_index)
{
  int index;

  for(index = 0; index <= possible_val_index; index += 1)
  {
    if(possible_val[index] == new_type)
    {
      return FAILURE; /*repetition of the same data type*/
    }
  }
 
  return SUCCESS;
}


/**
* Function creates a data set
* for the function table that is later 
* used in the semantic step;
*/
static Data_Set*
create_data_set(const Data_Type possible_val[], const int possible_val_index)
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
 
 for(index = 1; index < possible_val_index; index += 1)
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
  
  for(i = 0; i < NUMBER_OF_DATA_TYPES; i += 1)
  {
    if(!strcmp(data_type, DATA_TYPES[i])) /*valid data type*/
    {
      return i;
    }
  }

  return WRONG_INDEX;
}


/**
* Function reads an include
* file and checks its syntax and 
* preprocess some symbol tables (function_table).
**/
static int
read_include_file(FILE* file)
{
  
  int param_number = 0; /*function paramter number*/
  int possible_val_index = 0;
  char char_buffer[1];
  Data_Type possible_val[NUMBER_OF_DATA_TYPES]; // stores data types of a function
  

 /* only for reading include files -- API header files */
 while(fscanf(file, "%c", char_buffer) != EOF) /*scan the entire file*/
 {
    if(char_buffer[0] == ' ' || char_buffer[0] == '\t') {while((fscanf(file, "%c", char_buffer) != EOF) && (char_buffer[0] == ' ' || char_buffer[0] == '\t')); /*ignore all white spaces*/}


    if(char_buffer[0] == '\n') { cf_lineno += 1; continue; }
    /*if not the above conditions, then only letters can be; otherwise, a syntax error*/ 
    
    /*comments are also allowed. C comments and C++ comments are allowed*/
    if(char_buffer[0] == '/') /*might be a comment*/
    {
      if(fscanf(file, "%c", char_buffer) == EOF) // read one more char and check which comment is being used
      {
        printf("'%s': line %i: Reading the file has stopped\n", curfilename, cf_lineno);
        return FAILURE;         
      }

      switch(char_buffer[0])
      {
        case '/' : {  
                      while((fscanf(file, "%c", char_buffer) != EOF) && (char_buffer[0] != '\n'));
                      cf_lineno += 1;
                      break;
                   } /*C++ comment*/
        case '*' : { 
                     int read_byte = 0;
                     char prev_char[1]; // for checking comments
                     prev_char[0] = '\0'; // initialize to NULL

                     while((read_byte = fscanf(file, "%c", char_buffer) != EOF) && (char_buffer[0] != '/'))
                     {
                        if(char_buffer[0] == '\n') { cf_lineno += 1; }
                        prev_char[0] = char_buffer[0]; // store the previous char                       
                     }
                     
                     /*check first EOF*/
                     if(read_byte == EOF || prev_char[0] != '*') // didn't find a slash or incorrect comment termination
                     {
                        printf("\n'%s': line %i: unterminated comment\n", curfilename, cf_lineno);
                        return FAILURE;
                     }


                     break; /*a correct comment. Break the switch statement*/
                   } /*C comment*/

        default : { printf("\n'%s': line %i: Inapprorpiate comment\n", curfilename, cf_lineno); return FAILURE; }
              

      }// switch

       continue; /*start over after comments have been handled*/
    }// if comments
   

    if(!isalpha(char_buffer[0]) && char_buffer[0] != '_'){ /*means a non-alphabetical character or not an underscore*/
       if(char_buffer[0] == '#')
       {
         printf("\n'%s': line %i: Recursive include is not supported\n", curfilename, cf_lineno);
         
       }
       else
       {
         printf("\n'%s': line %i: Wrong function identifier\n", curfilename, cf_lineno);
       }
 
       return FAILURE;
    }

    /*read until a non-alphabetical found found*/
    buffer[0] = char_buffer[0]; 
    unsigned int index = 1;
    while((fscanf(file, "%c", char_buffer) != EOF) && (isalpha(char_buffer[0]) || char_buffer[0] == '_'))
    {
      buffer[index] = char_buffer[0]; /*read character by character*/
      index += 1;
      if(index == (BUFFER_SIZE - 1)) {printf("\n'%s': line %i: too long function identifier\n", curfilename, cf_lineno); return FAILURE;}
    } 
           
     if(isspace(char_buffer[0])) 
     {
        if(char_buffer[0] == '\n') { cf_lineno += 1; }      

        while((fscanf(file, "%c", char_buffer) != EOF) && isspace(char_buffer[0])) /*ignore white spaces*/ 
        {
          if(char_buffer[0] == '\n') { cf_lineno += 1; }
        }
     }
    

     /*check syntax*/
     if(char_buffer[0] != '{') {printf("\n'%s': line %i: wrong data set specification\n", curfilename, cf_lineno); return FAILURE;} /*must be an opening brace*/

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
   

     /*following steps check the grammar and data types*/
     /* read until a closing brace is found */
     while((fscanf(file, "%c", char_buffer) != EOF) && char_buffer[0] != '}')
     {
       if(isspace(char_buffer[0])) /*ignore white spaces*/
       {
         if(char_buffer[0] == '\n') { cf_lineno += 1; }
         continue;
       }       
       
      if(char_buffer[0] == EOF)
      {
        printf("\n'%s': line %i: Unterminated function declaration\n", curfilename, cf_lineno);
        return FAILURE;
      }    


       /*must be data types that must start with a letter*/
       if(isalpha(char_buffer[0]))
       {
         unsigned int size = 1;
         buffer[0] = char_buffer[0];
         while((fscanf(file, "%c", char_buffer) != EOF) && (isalnum(char_buffer[0]) || char_buffer[0] == '_')) /*reads the data type*/
         {
           buffer[size] = char_buffer[0];
           size += 1;
           if(size == (BUFFER_SIZE-1)) {printf("\n'%s': line %i: too long data identifier\n", curfilename, cf_lineno); return FAILURE;} 
         }//  while
          
          buffer[size] = '\0'; /*C string*/
       }// if
       else
       { 
         /*syntax error*/
         free(func_identifier);
         printf("\n'%s': line %i: syntax error defining data types\n", curfilename, cf_lineno);
         return FAILURE;
       }// else
   
    
        /*check the read data type*/
        int data_type_index;
        if((data_type_index = is_valid_data_type(buffer)) == WRONG_INDEX)
        {
         /*incorrect data type*/
         free(func_identifier);
         printf("\n'%s': line %i: '%s' is undefined\n", curfilename, cf_lineno, buffer);
   
         return FAILURE;
        }
  

       /*the current char is checked for syntax error*/
       
       if(char_buffer[0] != ',' && char_buffer[0] != '}' && !isspace(char_buffer[0]))
       {
         /*syntax error*/
         free(func_identifier);
         printf("\n'%s': line %i: syntax error defining data types\n", curfilename, cf_lineno);
   
         return FAILURE;
       }
       
  
       const Data_Type temp_type = get_data_type(data_type_index); /*get a possible data type*/
      
       if(already_exist(possible_val, temp_type, possible_val_index) == FAILURE) /*don't allow repetition of the same data type*/
       {
         printf("\n'%s': line %i: no repeated data type allowed\n", curfilename, cf_lineno);
         return FAILURE; 
       }       

       possible_val[possible_val_index] = temp_type;
       possible_val_index += 1; /*increment index every for a possible next data type*/  
  

       if(isspace(char_buffer[0]))
       {
         if(char_buffer[0] == '\n') { cf_lineno += 1; }
         continue;
       }//if
       else // either as colon or a closing brace 
       {
         if(char_buffer[0] == ',') {continue;}
         
         break; /*must be a closing brace*/
       }// else   
       
     }// while
     
     /*the source code has been read until here. Now a pair of parentheses, a digit and a semicolon are expected*/
     while((fscanf(file, "%c", char_buffer) != EOF) && isspace(char_buffer[0])) /*ignore white spaces*/
     {
       if(char_buffer[0] == '\n') { cf_lineno += 1; }
     }
     
     /*handle parentheses*/
     if(char_buffer[0] != '(') 
     {
      /*syntax error*/
      free(func_identifier);
      printf("\n'%s': line %i: syntax error defining data types: missing '('\n", curfilename, cf_lineno);
      return FAILURE;
     }

     /*read until a digit is found*/
      while((fscanf(file, "%c", char_buffer) != EOF) && isspace(char_buffer[0])) /*ignore white spaces*/
     {
       if(char_buffer[0] == '\n') { cf_lineno += 1; }
     }


     if(!isdigit(char_buffer[0]) || (char_buffer[0] == EOF))
     {
      /*syntax error*/
      free(func_identifier);
      printf("\n'%s': line %i: syntax error defining parameter number\n", curfilename, cf_lineno);
      return FAILURE;
     }

     /*read digits until non-digit is found*/
     param_number = char_buffer[0] - '0';
     
     while((fscanf(file, "%c", char_buffer) != EOF) && isdigit(char_buffer[0]))
     {
        if(char_buffer[0] == EOF) // still need to check this condition sice EOF -1. Need to check if this condition id needed
        {
         /*syntax error*/
         free(func_identifier);
         printf("\n'%s': line %i: syntax error: incomplete function definition\n", curfilename, cf_lineno);
         return FAILURE;
        }
        param_number = param_number*10 + (char_buffer[0] - '0'); /*more than 9 parameters*/
     }


     
     if(isspace(char_buffer[0])) /*skip until a closing parenthesis is found*/
     {
                
       if(char_buffer[0] == '\n') { cf_lineno += 1; }
       /*read until a digit is found*/
       while((fscanf(file, "%c", char_buffer) != EOF) && isspace(char_buffer[0])) /*ignore white spaces*/
       {
         if(char_buffer[0] == '\n') { cf_lineno += 1; }
       }// while 
     }// if     
     
     
     /*handle parentheses*/
     if(char_buffer[0] != ')') 
     {
      /*syntax error*/
      free(func_identifier);
      printf("\n'%s': line %i: syntax error defining parameter number\n", curfilename, cf_lineno);
      return FAILURE;
     }

     /*what's left is a semi-colon*/
     while((fscanf(file, "%c", char_buffer) != EOF) && isspace(char_buffer[0]))
     {
       if(char_buffer[0] == '\n') { cf_lineno += 1; }
     }

     /*last step -- either a semicolon or a syntax error*/
     if(char_buffer[0] != ';')
     {
       /*syntax error*/
       free(func_identifier);
       printf("\n'%s': line %i: syntax error: missing ';'\n", curfilename, cf_lineno);
       return FAILURE;
     }

     /*can create a reference in the function symbol table*/
     Data_Set* av_data_types = create_data_set(possible_val, possible_val_index);
   
     add_function_API(func_identifier, av_data_types, param_number);
     
     /*reset values -- no need to delete the allocated memory since it is handled by other files*/
     param_number = 0;
     possible_val_index = 0;
        
     /*Function definition is done. Look for another function definition.*/
 }// while

   return SUCCESS; /*the entire file has been checked and no syntax erros have been found*/
}



static int
read_source_code(FILE* file)
{
  char char_buffer[1]; 
  while(fscanf(file, "%c", char_buffer) != EOF) // scan until the end of the file
  {
    if(char_buffer[0] == '\n') { cf_lineno += 1; }
    if(char_buffer[0] == '#') 
    { // handle include
        /*check if the there is spaces until a quote is found*/
        while((fscanf(file, "%c", char_buffer) != EOF) && (char_buffer[0] == ' ' || char_buffer[0] == '\t'));  /*skip all white spaces*/
        
        if(char_buffer[0] != 'i') 
        {
          printf("\n'%s': line %i: incorrect include file\n", curfilename, cf_lineno);
          return FAILURE;
        }

        buffer[0] = 'i';
        unsigned int size = 1;
        /*read the string which must be 'include'*/
        while((fscanf(file, "%c", char_buffer) != EOF) && isalpha(char_buffer[0])) 
        {
          buffer[size] = char_buffer[0];
          size += 1;
          if(size == (BUFFER_SIZE - 1)) {printf("\n'%s': line %i: wrong keyword 'include'\n", curfilename, cf_lineno); return FAILURE;}
        }
        
         buffer[size] = '\0'; /*C string*/
        /*check the read string*/
        if(strcmp(buffer, "include"))
        {
          printf("\n'%s': line %i: incorrect keyword 'include'\n", curfilename, cf_lineno);
          return FAILURE;
        }

        /*skip white spaces*/
        if(char_buffer[0] == ' ' || char_buffer[0] == '\t')
        {
          while((fscanf(file, "%c", char_buffer) != EOF) && (char_buffer[0] == ' ' || char_buffer[0] == '\t'));  /*skip all white spaces*/
        }


        if(char_buffer[0] != '"')
        {
          printf("\n'%s': line %i: incorrect include file\n", curfilename, cf_lineno);
          return FAILURE;
        }
        
        size = 0; /*defined a few lines above*/ 
        /*read until another quotation mark is found*/
        while((fscanf(file, "%c", char_buffer) != EOF) && (isalnum(char_buffer[0]) || ispunct(char_buffer[0]) && char_buffer[0] != '"'))
        {
          buffer[size] = char_buffer[0];
          size += 1;
          if(size == (BUFFER_SIZE - 1)) {printf("\n'%s': line %i: too long filename\n", curfilename, cf_lineno); return FAILURE;}
        } 
        
        buffer[size] = '\0'; /*create a C string*/
        
        /*skip white spaces*/
        if(char_buffer[0] == ' ' || char_buffer[0] == '\t')
        {
          while((fscanf(file, "%c", char_buffer) != EOF) && (char_buffer[0] == ' ' || char_buffer[0] == '\t'));  /*skip all white spaces*/
        }
         
        if(char_buffer[0] != '"')
        {
          printf("\n'%s': line %i: incorrect include file\n", curfilename, cf_lineno);
          return FAILURE;
        }
   
        if(newfile(buffer, file) == FAILURE) {return FAILURE;} // create a new structure and try if it is possible to read a new file
      
    }  
 
  } // while loop

   return SUCCESS;

}



/*for checking if a new file is used*/
static int
newfile(char* filename, FILE* old_file)
{
  FILE* file = fopen(filename, "r");
  
  if(!file) // no such file
  {
    printf("'%s' does not exist\n", filename);
    return FAILURE;
  }
  
  BuffStack* ptr = malloc(sizeof(BuffStack));
 
  if(!ptr) // not enough memory
  {
   printf("\nOut of memory in the pre-processing stage\n");
   exit(1);
  }
  
 
  /* save the current file state */
  curbs->filename = curfilename;
  curbs->lineno = cf_lineno;
  curbs->file = old_file;
  
  /*set up the new file*/
  ptr->file = file;
  ptr->prev = curbs;

  /*re-initialize global variables*/

  curbs = ptr;
  cf_lineno = 1;
  curfilename = malloc(sizeof(char) * strlen(filename) + sizeof(char));
  strcpy(curfilename, filename);
   
  const unsigned int status = read_include_file(file);

  popfile(); /*reading is done, pop the previous file*/

  return status;
}

  
static void
popfile(void)
{
  BuffStack* bs = curbs;
  BuffStack* prevbs;
  
  if(!bs) { return; } 
  
  /* switch back to previous */
  fclose(curbs->file); 
  prevbs = bs->prev;
  free(curfilename); /*delete the pointer to the current file*/ 
  
  if(!prevbs) { return;}

  curbs = prevbs;
  cf_lineno = curbs->lineno;
  curfilename = curbs->filename;
  
}



int 
preprocess_file(const char* const filename)
{
  /*check if it is possible to open the file*/
  FILE* file = fopen(filename, "r");

  if(!file)
  {
    printf("\n'%s' does not exist.\n", filename);

    return FAILURE;
  }

  curbs = malloc(sizeof(BuffStack));
  curfilename = malloc(sizeof(char)*strlen(filename) + sizeof(char)); 
  
  if(!curfilename || !curbs)
  {
     printf("\nCannot process file '%s' as there is no enough memory\n", filename);
     exit(1);
  }

  strcpy(curfilename, filename);
  
  const int status = read_source_code(file); /*return the status of reading*/  
  fclose(file); // close the file

  free(curbs->filename);
  free(curbs);

  return status; /*successfully included all the files*/
 
}


int
main(int argc, char** argv)
{

  if(argc != 2)
  {
    printf("\nInappropriate number of parameters. There is/are %i, but 1 is required\n", (argc-1));
    return 1;
  }

  return preprocess_file(argv[1]);
 
}


