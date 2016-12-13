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
<PATH_DEF>.+[^" ]+ {yylval.s.d_type = PATH_STRING; yylval.s.name = yytext; return NAME;}
<PATH_DEF>\" {BEGIN(INITIAL);}
<PATH_DEF>[\n \t<<EOF>>]+ {printf("%s:%d: No line breaks in the data path\n", curfilename, yylineno); return 0;}


<VAR_STATE>[ \t]*("uint8_t"|"int8_t"|"uint16_t"|"int16_t"|"uint32_t"|"int32_t"|"uint64_t"|"int64_t")[ \t]*  {yylval.m_type = get_data_type(yytext);  return VAR_TYPE;} 

<VAR_STATE>">" {BEGIN(INITIAL); return yytext[0];}
<VAR_STATE> [.<<EOF>>\n]+ {printf("%s:%d: Untermined data type\n",
curfilename, yylineno); return 0;}



[_a-zA-Z][_a-zA-Z0-9]* { yylval.s.name = yytext; return NAME; }

"//".*\n /*only one type of comments*/

[ \t] /* ignore whitespace */

\\\n { printf("c> "); } /* ignore line continuation */

\n { return EOL; }

. { yyerror("Mystery character %c\n", *yytext); }
%%


enum data_type get_data_type(const char* match_text)
{
  if(match_text[0] == 'u') /*unsigned type*/
  {
    switch(match_text[4])
    {
      case '8' : {return UINT_8;}
      case '1' : {return UINT_16;}
      case '3' : {return UINT_32;}
      case '6' : {return UINT_64;}
      default : {return ERROR_TYPE;}
    }
  }
  else{ /* signed integers */
    if(match_text[0] == 'i')
    {
      switch(match_text[3])
      {
        case '8' : {return INT_8;}
        case '1' : {return INT_16;}
        case '3' : {return INT_32;}
        case '6' : {return INT_64;}
        default : {return ERROR_TYPE;}
      }
    }
  }
  
  return ERROR_TYPE;
}

