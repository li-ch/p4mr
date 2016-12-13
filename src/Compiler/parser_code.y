/* parser for AST */
%{
# include <stdio.h>
# include <stdlib.h>
# include "compiler_header.h"

struct tree_struct* root; /*global reference to the AST*/
%}

%union {
 struct ast* a;
 struct symbol* s; /* which symbol */
 struct fun_arg* sl;
 enum data_type m_type; /*variable type*/
 struct tree_struct* list; /*list of statements*/
}


/* declare tokens */
%token <s> NAME 
%token <m_type> VAR_TYPE
%token EOL ASSIGN 
/*%token IF ELSE*/


%type <a> stmt expr
%type <list> stmtlist
%type <sl> symlist


%start beg_compile


%%

/*beginning of the compiler rules*/

beg_compile: stmtlist {root = $1;}
    ;


stmtlist: stmt {$$->sub_tree = $1; $$->next = NULL;}
    | stmtlist stmt {$$->sub_tree = $2; $$->next = $1;}
    | %empty {$$ = newstmtlist();} 
    ;


stmt: NAME ASSIGN expr {$$ = newassign($1, $3);}
    | NAME '<' VAR_TYPE '>' '(' symlist ')' ';' {$$ = newfunctype($1, $3, $6);}
    ;


expr: NAME '<' VAR_TYPE '>' '(' symlist ')' ';' {$$ = newfunctype($1, $3, $6);}
    | NAME '(' symlist ')' ';' {$$ = newfuncnotype($1, $3);}



symlist: NAME {$$ = newarglist($1, NULL);}
       | NAME ',' symlist {$$ = newarglist($1, $3);}
       | %empty {$$ = NULL;}
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




