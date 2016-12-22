# include <stdio.h>
# include <stdlib.h>
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
static char char_buffer[1]; // for reading a character in

static const char* curfilename; /* name of current input file */
static int newfile(const char * const fn, FILE* old_file);
static int popfile(void);


static int
read_include_file(FILE* file)
{
 /* only for reading include files -- API header files */
 while(fscan(file, "%c", char_buffer[0])) /*scan the entire file*/
 {
    if(char_buffer[0] == ' ' || char_buffer[0] == '\t') {while(fscan(file, "%c", char_buffer[0]) && (char_buffer[0] == ' ' || char_buffer[0] == '\t')); /*ignore all white spaces*/}

    if(char_buffer[0] == EOF){ return SUCCESS; }
    if(char_buffer[0] == '\n') { cf_lineno++; continue; }
    /*if not the above conditions, then only letters can be; otherwise, a syntax error*/ 
 }
}



static void
read_source_code(FILE* file)
{
  while(fscan(file, "%c", char_buffer[0])) // scan until the end of the file
  {

    if(char_buffer[0] == EOF){ return SUCCESS; }
    if(char_buffer[0] == '\n') { cf_lineno++; }
    if(char_buffer[0] == '#') 
    { // handle include
        /*check if the there is spaces until a quote is found*/
        while(fscan(file, "%c", char_buffer[0]) && (char_buffer[0] == ' ' || char_buffer[0] == '\t'));  /*skip all white spaces*/
        if(char_buffer[0] != '"')
        {
          printf("\n%s: %i: incoorrect include file\n", curfilename, cf_lineno);
          return FAILURE;
        }
        
        unsigned int size = 0; 
        /*read until another quotation mark is found*/
        while(fscan(file, "%c", char_buffer[0]) &&  char_buffer[0] != '"')
        {
          buffer[size] = char_buffer[0];
          size++;
          if(size == (BUFFER_SIZE-1)) {printf("\n%s: %i: too long filename\n", curfilename, cf_lineno); return FAILURE;}
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
  
  defining = 0; /*means an include has been handled*/

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





