/* recognize tokens for the compiler */
%option noyywrap yylineno
%{
# include "compiler_header.h"
# include "parser_code.tab.h"
%}

%x PATH_DEF VAR_STATE

%%


","  |
";"  |
"("  |
")"  |
"{"  |
"}"  { return yytext[0]; }
 
":=" { return ASSIGN; }

"<" {BEGIN(VAR_STATE); return yytext[0];}


\" {BEGIN(PATH_DEF);}
<PATH_DEF>.+[^" ]+ {yylval.s->d_type = PATH_STRING; yylval.s->name = yytext; return NAME;}
<PATH_DEF>\" {BEGIN(INITIAL);}
<PATH_DEF>[\n \t]+ {printf("%d: No line breaks in the data path\n", yylineno); return 0;}
<PATH_DEF><<EOF>> {printf("%d: No line breaks in the data path\n", yylineno); return 0;}


<VAR_STATE>[ \t]*"uint8_t"[ \t]* {yylval.m_type = UINT_8; return VAR_TYPE;}
<VAR_STATE>[ \t]*"int8_t"[ \t]* {yylval.m_type = INT_8; return VAR_TYPE;}
<VAR_STATE>[ \t]*"uint16_t"[ \t]* {yylval.m_type = UINT_16; return VAR_TYPE;}
<VAR_STATE>[ \t]*"int16_t"[ \t]* {yylval.m_type = INT_16; return VAR_TYPE;}
<VAR_STATE>[ \t]*"uint32_t"[ \t]* {yylval.m_type = UINT_32; return VAR_TYPE;}
<VAR_STATE>[ \t]*"int32_t"[ \t]* {yylval.m_type = INT_32; return VAR_TYPE;}
<VAR_STATE>[ \t]*"uint64_t"[ \t]* {yylval.m_type = UINT_64; return VAR_TYPE;}
<VAR_STATE>[ \t]*"int64_t"[ \t]*  {yylval.m_type = INT_64;  return VAR_TYPE;} 

<VAR_STATE>">" {BEGIN(INITIAL); return yytext[0];}
<VAR_STATE>\n+|.+ {printf("%d: Untermined data type\n", yylineno); return 0;}
<VAR_STATE><<EOF>> {printf("%d: Untermined data type\n", yylineno); return 0;}


[a-zA-Z][_a-zA-Z0-9]* { yylval.s->name = yytext; return NAME; }

"//".*\n /*only one type of comments*/

[ \t] /* ignore whitespace */

\\\n { printf("c> "); } /* ignore line continuation */

\n { return EOL; }

. { yyerror("Mystery character %c\n", *yytext); }
%%

/*{printf("%s:%d: Untermined data type\n",
curfilename, yylineno); return 0;}*/

