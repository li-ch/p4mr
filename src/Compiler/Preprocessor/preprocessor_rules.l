/* Some code has been taken from:
 "flex & bison" by John R. Levine Copyright 2009 John Levine, 978-0-596-15597-1.
*/

/* recognize tokens for the compiler */
%option noyywrap yylineno

%{
# include "../Includes/symbol_table.h"    
# include <string.h>    
# include "preprocessor_parser.tab.h"

typedef struct reference Reference;

struct reference {
  Reference* m_next_ref;
  const char* filename;
  int flags;  /*either definition or delcaration*/
  int lineno;
};


typedef struct bufstack BuffStack;

struct bufstack {
  BuffStack *prev; /* previous entry */
  YY_BUFFER_STATE bs; /* saved buffer */
  int lineno; /* saved line number */
  const char *filename; /* name of this file */
  FILE *file; /* current file */
}; 

BuffStack* curbs = NULL;

const char* curfilename; /* name of current input file */
static int newfile(const char * const fn);
static int popfile(void);

static unsigned int defining = 0; /*global flag that helps to handle includes*/


%}

%x INCLUDE_STATE
%x PARAMETER_SET_STATE
%x PAR_NUM_STATE

%%


^"#"[ \t]*include[ \t]*[\"] { if(defining) 
                                { 
                                  printf("\nRecursive include files are not supported\n"); yyterminate(); 
                                } 
                                printf("\n*******************Include has been found*******************\n");
                                defining = 1; BEGIN (INCLUDE_STATE); /* match the include statement up to a quote or a less sign*/ 
                              }


<INCLUDE_STATE>\"          { defining = 0; BEGIN (INITIAL); }
<INCLUDE_STATE>[^ \t\n\"]+   { /* handle filename up to the closing quote */
                                
                               int c;
                               while((c = input()) && (c != '"'));
                               if((c = input()) && (c != '\n')) 
                               { 
                                 fprintf(stderr, "\n%s : %i : Incorrect include\n", curfilename, (yylineno-1)); 
                                 yyterminate(); 
                               }                               

                               yylineno++; /*one include per line*/
                               
                               printf("\n***********Include has been successfully processed on file '%s' ***********\n", yytext);
                               BEGIN (INITIAL);
                               if(!newfile(strdup(yytext)))
                               {   
                                yyterminate(); /* no such file */
                               }
                               
                           }

<INCLUDE_STATE>.|\n        { fprintf(stderr, "\n%s: %4d bad include line\n", curfilename, yylineno); yyterminate(); }

<<EOF>> { if(!popfile()) { yyterminate(); }  /*pop the file stack at end of file, terminate if it's the outermost file*/}



"{"                                           { if (defining) /* means that an include file is being read */ 
                                                { 
                                                  BEGIN (PARAMETER_SET_STATE);
                                                  return yytext[0]; 
                                                }
                                              }

<PARAMETER_SET_STATE>[ \t]*"uint8_t"[ \t]*    { yylval.d_type = UINT_8; return VAR_TYPE;  }
<PARAMETER_SET_STATE>[ \t]*"int8_t"[ \t]*     { yylval.d_type = INT_8; return VAR_TYPE;   }
<PARAMETER_SET_STATE>[ \t]*"uint16_t"[ \t]*   { yylval.d_type = UINT_16; return VAR_TYPE; }
<PARAMETER_SET_STATE>[ \t]*"int16_t"[ \t]*    { yylval.d_type = INT_16; return VAR_TYPE;  }
<PARAMETER_SET_STATE>[ \t]*"uint32_t"[ \t]*   { yylval.d_type = UINT_32; return VAR_TYPE; }
<PARAMETER_SET_STATE>[ \t]*"int32_t"[ \t]*    { yylval.d_type = INT_32; return VAR_TYPE;  }
<PARAMETER_SET_STATE>[ \t]*"uint64_t"[ \t]*   { yylval.d_type = UINT_64; return VAR_TYPE; }
<PARAMETER_SET_STATE>[ \t]*"int64_t"[ \t]*    { yylval.d_type = INT_64;  return VAR_TYPE; } 

<PARAMETER_SET_STATE>","                      { return yytext[0]; } 

<PARAMETER_SET_STATE>"}"                      { BEGIN (INITIAL); return yytext[0];         } 
<PARAMETER_SET_STATE>\n                       {  yylineno++; /* ignore new lines */ }
<PARAMETER_SET_STATE>.                        { fprintf(stderr, "\n%s : %i : Bad type set\n", curfilename, yylineno); yyterminate(); }



";"                                           { if (defining) /* means that an include file is being read */ 
                                                { 
                                                  return yytext[0]; 
                                                } 
                                              } 



"("                                           { if (defining) /* means that an include file is being read (used for parameter number) */ 
                                                { 
                                                  BEGIN (PAR_NUM_STATE);
                                                  return yytext[0]; 
                                                }
                                              }


<PAR_NUM_STATE>[0-9]+                         { yylval.par_number = atoi(yytext); return PAR_NUMBER; } 
<PAR_NUM_STATE>")"                            { BEGIN (INITIAL); return yytext[0]; }
<PAR_NUM_STATE>\n                             { yylineno++; }
<PAR_NUM_STATE>.                              { fprintf(stderr, "\n%s : %i : Bad type set\n", curfilename, yylineno); yyterminate(); }    



                                              


([_a-zA-Z]+[_a-zA-Z0-9]*)                     { if (defining) /* means that an include file is being read (function identifier) */ 
                                                { 
                                                  yylval.symbol = strdup(yytext);
                                                  return NAME; 
                                                }
                                              }
                           



[ \t]+                                        {/*ignore white spaces*/}


\n { yylineno++; }

.                                             {/*ignore other characters*/}

%%


/*for checking if a new file is used*/
static int
newfile(const char* const filename)
{
  FILE* file = fopen(filename, "r");
  
  if(!file) // no such file
  {
    printf("\"%s\" does not exist\n", filename);
    return 0;
  }
  
  BuffStack* ptr = malloc(sizeof(BuffStack));
 
  if(!ptr) // not enough memory
  {
   printf("\nOut of memory in the pre-processing stage\n");
   exit(1);
  }
  
  /* remember state */
  if(curbs)
  {
     curbs->lineno = yylineno;
  }

   ptr->prev = curbs;

   /* set up current entry */
   ptr->bs = yy_create_buffer(file, YY_BUF_SIZE);
   ptr->file = file;
   ptr->filename = filename;

   yy_switch_to_buffer(ptr->bs);
   curbs = ptr;
   yylineno = 1;
   curfilename = filename;
 
   return 1;
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

  return 1;
}
