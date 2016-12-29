/* recognize tokens for the compiler */
%option noyywrap yylineno
%{
# include "../Includes/compiler_header.h"
# include "../parser_code.tab.h"
%}

%x PATH_DEF VAR_STATE

%%


","  |
";"  |
"("  |
")"  |
"{"  |
"}"  { return yytext[0]; }
 
":=" { return ASSIGN ; }

"<" {BEGIN(VAR_STATE); return yytext[0];}


\" {BEGIN(PATH_DEF);}
<PATH_DEF>([-]+|[_a-zA-z]+) { yylval.symbol = malloc(sizeof(Symbol)); yylval.symbol->m_data = PATH_STRING; yylval.symbol->m_name = malloc(strlen(yytext) + sizeof(char)); strcpy(yylval.symbol->m_name, yytext); add_symbol(yylval.symbol); return NAME;}
<PATH_DEF>\"       {BEGIN(INITIAL);}
<PATH_DEF>[\n \t]+ {printf("%d: No line breaks in the data path\n", yylineno); return 0;}
<PATH_DEF><<EOF>>  {printf("%d: No line breaks in the data path\n", yylineno); return 0;}
<PATH_DEF>.        {printf("%d: No line breaks in the data path\n", yylineno); return 0;}


<VAR_STATE>[ \t]*"uint8_t"[ \t]*    { yylval.symbol = malloc(sizeof(Symbol)); yylval.symbol->m_data = UINT_8; yylval.symbol->m_name = malloc(strlen("uint8_t") + sizeof(char)); strcpy(yylval.symbol->m_name, "uint8_t"); return VAR_TYPE;}
<VAR_STATE>[ \t]*"int8_t"[ \t]*     { yylval.symbol = malloc(sizeof(Symbol)); yylval.symbol->m_data = INT_8;  yylval.symbol->m_name = malloc(strlen("int8_t") + sizeof(char)); strcpy(yylval.symbol->m_name, "int8_t"); return VAR_TYPE;}
<VAR_STATE>[ \t]*"uint16_t"[ \t]*   { yylval.symbol = malloc(sizeof(Symbol)); yylval.symbol->m_data = UINT_16; yylval.symbol->m_name = malloc(strlen("uint16_t") + sizeof(char)); strcpy(yylval.symbol->m_name, "uint16_t"); return VAR_TYPE;}
<VAR_STATE>[ \t]*"int16_t"[ \t]*    { yylval.symbol = malloc(sizeof(Symbol)); yylval.symbol->m_data = INT_16; yylval.symbol->m_name = malloc(strlen("int16_t") + sizeof(char)); strcpy(yylval.symbol->m_name, "int16_t"); return VAR_TYPE;}
<VAR_STATE>[ \t]*"uint32_t"[ \t]*   { yylval.symbol = malloc(sizeof(Symbol)); yylval.symbol->m_data = UINT_32; yylval.symbol->m_name = malloc(strlen("uint32_t") + sizeof(char)); strcpy(yylval.symbol->m_name, "uint32_t"); return VAR_TYPE;}
<VAR_STATE>[ \t]*"int32_t"[ \t]*    { yylval.symbol = malloc(sizeof(Symbol)); yylval.symbol->m_data = INT_32; yylval.symbol->m_name = malloc(strlen("int32_t") + sizeof(char)); strcpy(yylval.symbol->m_name, "int32_t"); return VAR_TYPE;}
<VAR_STATE>[ \t]*"uint64_t"[ \t]*   { yylval.symbol = malloc(sizeof(Symbol)); yylval.symbol->m_data = UINT_64; yylval.symbol->m_name = malloc(strlen("uint64_t") + sizeof(char)); strcpy(yylval.symbol->m_name, "uint64_t"); return VAR_TYPE;}
<VAR_STATE>[ \t]*"int64_t"[ \t]*    { yylval.symbol = malloc(sizeof(Symbol)); yylval.symbol->m_data = INT_64; yylval.symbol->m_name = malloc(strlen("int64_t") + sizeof(char)); strcpy(yylval.symbol->m_name, "int64_t");  return VAR_TYPE;} 

<VAR_STATE>[ \t]*">"[ \t]*          { BEGIN(INITIAL); return yytext[0]; }
<VAR_STATE>\n|.                     { printf("%d: Undetermined data type %s\n", yylineno, yytext); return 0;}
<VAR_STATE><<EOF>>                  { printf("%d: Undetermined data type %s\n", yylineno, yytext); return 0;}


[a-zA-Z][_a-zA-Z0-9]* { yylval.symbol = malloc(sizeof(Symbol)); yylval.symbol->m_name = malloc(sizeof(char) * strlen(yytext)); strcpy(yylval.symbol->m_name, yytext); return NAME; }

"//".*\n /*only one type of comments*/

[ \t] /* ignore whitespace */

\\\n { printf("c> "); } /* ignore line continuation */

\n /*ignore new lines*/


. {printf("Mystery character %s\n", yytext); /* yyerror("Mystery character %s\n", *yytext);*/ }
%%
