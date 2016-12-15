/* parser for AST */
%{
# include <stdio.h>
# include <stdlib.h>
# include "compiler_header.h"

Tree* root; /*global reference to the AST*/
%}

%union {
 Ast* ast;
 Symbol* symbol; /* which symbol */
 Func_Arg* arg; /*argument list*/
 Data_Type d_type; /*type of a var/function*/
 Tree* statement; /*a node for a statement to build an AST*/
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


%start beg_compile


%%

/*beginning of the compiler rules*/

beg_compile: stmtlist { root = $1; root->m_node = $1->m_node; root->m_next = $1->m_next; }
    ;


stmtlist: stmt { $$->m_node = $1; $$->m_next = NULL; }
    | stmtlist stmt { $$->m_node = $2; $$->m_next = $1; } 
    ;


stmt: NAME ":=" expr { $$ = newassign($1, $3); }
    | func { $$ = $1; }
    ;


expr: func { $$ = $1; }
    | NAME '(' symlist ')' ';' { $$ = newfuncnotype($1, $3); }
    ;


func: NAME '<' VAR_TYPE '>' '(' symlist ')' ';' { $$ = newfunctype($1, $3, $6); }
    ;


symlist: NAME { $$ = newarglist($1, NULL); }
       | NAME ',' symlist { $$ = newarglist($1, $3); }
       | %empty { $$ = NULL; }
       ;


%%

int main(int argc, char** argv)
{

 if(argc < 2)
 {
   printf("Please input a source file for reading"); 
   return 1;
 } 

 FILE* file = fopen(argv[1], "r");
 if(!file) 
 {
  printf("No such file, %s, or it is not in the same directory", argv[1]); 
  return 1;
 } 
 yyin = file;

 /*parse the entire file*/
 do
 {
   yyparse();
 }while(!feof(yyin));

 /*print an AST first and then delete it */
 print_tree(root);
 treefree(root);

 return 0;
}




