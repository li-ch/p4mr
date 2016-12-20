/* parser for AST */
%{
# include <stdio.h>
# include <stdlib.h>
# include "compiler_header.h"

Program* root; /*global reference to the AST*/
%}


%union {
 Ast* ast;
 Symbol symbol; /* which symbol */
 Func_Arg* arg; /*argument list*/
 Data_Type d_type; /*type of a var/function*/
 Tree* statement; /*a node for a statement to build an AST*/
 Program* program; /*the pointer that points to the root of the tree*/
}


/* declare tokens */
%token <symbol> NAME 
%token <d_type> VAR_TYPE
%token EOL 
%token ASSIGN ":="
/*%token IF ELSE*/


%type <ast> stmt expr func 
%type <statement> stmtlist 
%type <arg> symlist
%type <program> beg_compile


%start beg_compile


%%

/*beginning of the compiler rules*/

beg_compile: stmtlist { printf("beg_compile:\n"); root->m_begin = $1; /*$$ = new_program($1); copy_prog(root, $$);*/ }      
    ;


stmtlist: stmtlist stmt { printf("stmtlist: stmtlist stmt\n"); 
                          printf("stmtlist addresses %i, %i", $$, $1);
                          $$->m_node = $2; $$->m_next = $1;
                        } 
        | %empty { printf("stmtlist: empty\n"); $$ = malloc(sizeof(Tree)); $$->m_node = NULL; $$->m_next = NULL; printf("stmtlist: empty  address structure: %i", $$);}
        ;


stmt: NAME ":=" expr { printf("stmt: NAME := expr\n"); $$ = newassign(&$1, $3); printf("stmt: NAME = %s\n", $1.m_name); free($1.m_name); $1.m_name = NULL; }
    | func           { printf("stmt: func\n"); $$ = $1; }
    ;


expr: func { printf("expr: func\n"); $$ = $1; }
    | NAME '(' symlist ')' ';' { printf("expr: NAME (symlist);\n"); $$ = newfuncnotype(&$1, $3); free($1.m_name); $1.m_name = NULL; }
    ;


func: NAME '<' VAR_TYPE '>' '(' symlist ')' ';' { printf("func: NAME <VAR_TYPE> (symlist);\n"); $$ = newfunctype(&$1, $3, $6); free($1.m_name); $1.m_name = NULL; }
    ;


symlist: NAME {printf("symlist: NAME\n"); $$ = newarglist(&$1, NULL); free($1.m_name); $1.m_name = NULL; }
       | NAME ',' symlist {printf("symlist: NAME, symlist\n"); $$ = newarglist(&$1, $3); free($1.m_name); $1.m_name = NULL; }
       | %empty { printf("symlist: empty"); }
       ;


%%


int main(int argc, char** argv)
{

 if(argc < 2)
 {
   printf("Please input a source file for reading.\n"); 
   return 1;
 } 

 FILE* file = fopen(argv[1], "r");
 if(!file) 
 {
  printf("No such file found: \"%s\"\n", argv[1]); 
  return 1;
 } 
 yyin = file;
 
 /*initialize root for storing the tilte of the progrma*/
 root = malloc(sizeof(Program));
 root->m_title = malloc(sizeof(char) * strlen(argv[1]));
 strcpy(root->m_title, argv[1]);
 /*initialization ends here*/

 printf("\n\n###### Parsing and building of the AST for \"%s\" begins now... ######\n\n", argv[1]);

 /*parse the entire file*/
 do
 { 
   yyparse();
 }while(!feof(yyin));



 printf("\n###### Done! ######\n\n");
 fclose(file); /* close the source file */ 

 /*print an AST first and then delete it */
 //print_program(root);
 //deallocate_tree(root);
 

 return 0;
}




