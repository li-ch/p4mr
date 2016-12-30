/* parser for a dependency list */
%{
# include <stdio.h>
# include <stdlib.h>
# include "Includes/compiler_header.h"
//# include "Preprocessor/preprocessor_parser.tab.c" 


Program* root; /*global reference to the AST*/
%}


%union {
 Symbol* symbol; /* which symbol */
 Func_Arg* arg; /*argument list*/
 Dep_Node* depend;
 Stmt_Node* statement;
 Func_JS* function;
}


/* declare tokens */
%token <symbol> NAME 
%token <symbol> VAR_TYPE
%token EOL 
%token ASSIGN ":="


%type <statement> stmt 
%type <depend> stmtlist
%type <function> func expr
%type <arg> symlist


%start beg_compile


%%

/*beginning of the compiler rules*/

beg_compile: stmtlist { printf("beg_compile:\n"); root->m_begin = get_begin($1); }      
    ;


stmtlist: stmtlist stmt { printf("stmtlist: stmtlist stmt\n"); 
                          if($1) /*there are statements before this*/
                          {
                            /*such an order of assignments preserve the doubly-linked-list order*/
                            $$ = malloc(sizeof(Dep_Node));
                            $$->m_stmt = $2;
                            $$->m_next = $1; 
                            $$->m_prev = $1->m_prev;
                            $1->m_prev = $$;
                          }
                          else
                          {
                            /*first node, means that this has to be initialized as follows*/
                            $$ = malloc(sizeof(Dep_Node));
                            $$->m_next = NULL;
                            $$->m_prev = NULL;
                            $$->m_stmt = $2; /*point at the new statement*/
                          }
                        } 
        | %empty        { printf("stmtlist: empty\n"); $$ = NULL; }
        ;



stmt: NAME ":=" expr { printf("stmt: NAME := expr\n"); $$ = new_statement($1, $3); }
    | func           { printf("stmt: func\n"); $$ =  new_statement(NULL, $1); }
    ;



expr: NAME '<' VAR_TYPE '>' '(' symlist ')' ';' { printf("expr: NAME <VAR_TYPE> (symlist);\n"); $$ = new_func_type($1, $3, $6); }
    | func                                      { printf("expr: func\n"); $$ = $1; }
    ;



func: NAME '(' symlist ')' ';' { printf("func: NAME (symlist);\n"); $$ = new_func_no_type($1, $3); }
    ;


symlist: NAME             { printf("symlist: NAME\n"); $$ = newarglist($1, NULL); }
       | NAME ',' symlist { printf("symlist: NAME, symlist\n"); $$ = newarglist($1, $3); } 
       | %empty           { printf("symlist: empty\n"); $$ = newarglist(NULL, NULL); }
       ;


%%


int main(int argc, char** argv)
{

 if(argc < 2)
 {
   printf("Please input a source file for reading.\n"); 
   return 1;
 } 

 if(argc > 2)
 {
   printf("Too many arguments passed.\n");
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
 root->m_title = malloc(sizeof(char) +  strlen(argv[1]));
 strcpy(root->m_title, argv[1]);
 init_tables();
 /*initialization ends here*/

 printf("\n\n###### Parsing and building the dependency list for \"%s\" begins now... ######\n\n", argv[1]);

 int state;
 /*parse the entire file*/
 do
 { 
   state = yyparse();
 }while(!feof(yyin));

 if(state)
 {
   exit(0); /*means a syntax error was found*/
 }

 printf("\n###### Done! ######\n\n");
 fclose(file); /* close the source file */ 

 /*print the dependencies first and then delete them */
 display_dependency_list(root);
 deallocate_dependency_list(root);
 

 return 0;
}




